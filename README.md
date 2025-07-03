# nsfp

A command-line player of Nintendo NES/Famicom music files (.nsf/.nsfe)

Based on the Game Music Emu library.

**This is a fork of [nsfp](https://github.com/munshkr/nsfp) with some opinionated changes to make it work in the latest Ubuntu versions.**

**This branch includes changes to:**
- `cxxopts.h` to allow it compile (See [nsfp/issues/2](https://github.com/munshkr/nsfp/issues/2))
- main.cc to remove ncurses dependency and allow the keyboard to work again (it was ignored completely on latest versions of Ubuntu and Debian). It was fixed by an LLM so code may be messy or have some bugs.

## Usage


```
nsfp 0.1 - NSF/NSFE player
Usage:
  ./nsfp [OPTION...] INPUT

      --input arg  Input file
  -i, --info       Only show info (default: false)
  -t, --track arg  Start playing from track NUM (default: 0)
  -s, --single     Stop after playing current track (default: false)
  -h, --help       Print this message (default: false)
```

For example, the following plays only track 3

```
$ nsfp Kirby.nes -t 3 -s
nsfp 0.1 - Simple command-line player of NSF/NSFE files
Game:      Kirby's Adventure
Author:    Hirokazu Ando, Jun Ishikawa
Copyright: 1993 HAL Laboratory, Nintendo

Kirby's Adventure: 3/57  (2:30)
...
```

When running you can also use the following key to control the player:

* <kbd>left</kbd>: Play previous track
* <kbd>right</kbd>: Play next track
* <kbd>space</kbd>: Pause/resume playing
* <kbd>q</kbd>, <kbd>ctrl</kbd>+<kbd>c</kbd>: Exit


## Dependencies

On Debian/Ubuntu you need a recent GCC compiler, and the development files
forSDL 2.0 and Game Music Emu:

```
sudo apt-get install build-essential libsdl2-dev libgme-dev libncurses-dev
```


## Install

After cloning the repository, go to the directory and run the following to
compile and install the player:

```
make
sudo make install
```

## License

Source code is released under Apache 2.0 license. Please refer to
[LICENSE](LICENSE).
