/*
 * Copyright 2018 Damián Silvani
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file has been patched by an LLM to remove ncurses and fix issue
 * about keyboard not responding in the latest Ubuntu or Debian versions
 * using termios
 */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <SDL2/SDL.h>

#include "cxxopts.h"
#include "player.h"

#define PRINTF(...) printf(__VA_ARGS__)
using namespace std;

// Terminal raw mode manager
struct TerminalRawMode {
  termios original;
  TerminalRawMode() {
    tcgetattr(STDIN_FILENO, &original);
    termios raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  }
  ~TerminalRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
  }
};

void start_track(Player* player, int track, bool dry_run = false) {
  if (auto err = player->start_track(track, dry_run)) {
    PRINTF("Player error: %s\n", err);
    exit(1);
  }

  long seconds = player->track_info().length / 1000;
  const char* game = player->track_info().game;
  if (!*game) {
    game = strrchr(player->filename().c_str(), '\\');
    if (!game)
      game = strrchr(player->filename().c_str(), '/');
    if (!game)
      game = player->filename().c_str();
    else
      game++;
  }

  auto& info = player->track_info();

  if (strcmp(info.game, "") != 0)
    PRINTF("Game:      %s\n", info.game);
  if (strcmp(info.author, "") != 0)
    PRINTF("Author:    %s\n", info.author);
  if (strcmp(info.copyright, "") != 0)
    PRINTF("Copyright: %s\n", info.copyright);
  if (strcmp(info.comment, "") != 0)
    PRINTF("Comment:   %s\n", info.comment);
  if (strcmp(info.dumper, "") != 0)
    PRINTF("Dumper:    %s\n", info.dumper);

  char title[512];
  sprintf(title, "%s: %d/%d %s (%ld:%02ld)", game, track + 1,
          player->track_count(), info.song, seconds / 60, seconds % 60);
  PRINTF("%s\n\n", title);
}

int main(int argc, const char* argv[]) {
  try {
    cxxopts::Options options(argv[0], "nsfp 0.1 - NSF/NSFE player");

    options.positional_help("INPUT").show_positional_help();

    options.add_options()
      ("input", "Input file", cxxopts::value<string>())
      ("i,info", "Only show info")
      ("t,track", "Start playing from a specific track",
        cxxopts::value<int>()->default_value("1"))
      ("s,single", "Stop after playing current track")
      ("h,help", "Print this message");

    options.parse_positional({"input"});
    auto result = options.parse(argc, argv);

    if (result.count("help") || !result.count("input")) {
      cout << options.help({""}) << endl;
      return result.count("help") ? 0 : 1;
    }

    const string input = result["input"].as<string>();
    bool show_info = result["info"].as<bool>();
    int track = result["track"].as<int>() - 1;
    bool single = result["single"].as<bool>();

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      cerr << "Failed to initialize SDL" << endl;
      return 1;
    }
    atexit(SDL_Quit);

    Player* player = new Player;
    if (!player) {
      cerr << "Out of memory" << endl;
      return 1;
    }

    if (auto err = player->init()) {
      cerr << "Player error: " << err << endl;
      return 1;
    }

    if (auto err = player->load_file(input)) {
      cerr << "Player error: " << err << endl;
      return 1;
    }

    if (track < 0 || track >= player->track_count()) {
      cerr << "Invalid track number. Must be between 1 and "
           << player->track_count() << endl;
      return 1;
    }

    TerminalRawMode terminal_mode;

    bool running = true;
    bool playing = true;

    start_track(player, track, show_info);
    if (show_info) return 0;

    while (running) {
      char ch;
      if (read(STDIN_FILENO, &ch, 1) > 0) {
        if (ch == 'q') {
          running = false;
        } else if (ch == ' ') {
          player->pause(playing);
          PRINTF(playing ? "[Paused]\n" : "[Playing]\n");
          playing = !playing;
        } else if (ch == '\033') {
          // Escape sequence: possible arrow key
          char seq[2];
          int n1 = read(STDIN_FILENO, &seq[0], 1);
          int n2 = read(STDIN_FILENO, &seq[1], 1);

          if (n1 == 1 && n2 == 1 && seq[0] == '[') {
            if (seq[1] == 'C') { // Right arrow
              if (track < player->track_count() - 1) {
                track++;
                start_track(player, track);
              }
            } else if (seq[1] == 'D') { // Left arrow
              if (track > 0) {
                track--;
                start_track(player, track);
              }
            }
          } else {
            // ESC alone
            running = false;
          }
        }
      }

      SDL_Delay(100);

      if (player->track_ended()) {
        if (single || track == player->track_count() - 1) {
          running = false;
        } else {
          track++;
          start_track(player, track);
        }
      }
    }

    delete player;
    return 0;

  } catch (const cxxopts::OptionException& e) {
    cerr << "Error parsing options: " << e.what() << endl;
    return 1;
  }
}
