# NetPass - StreetPass like it's 2014!

[![Translation status](https://weblate.sorunome.de/widget/netpass/app/svg-badge.svg)](https://weblate.sorunome.de/engage/netpass/)
[![GitLab Release](https://img.shields.io/gitlab/v/release/3ds-netpass/netpass)](https://gitlab.com/3ds-netpass/netpass/-/releases/permalink/latest/)


![QR Code](https://gitlab.com/Sorunome/3ds-streetpass/-/raw/release_builds/qr.png)

## Getting Netpass
#### Scan QR with your 3DS to install Netpass


## Introducing NetPass

In the current state of the world, the 3DS is, sadly, on decline. That makes getting StreetPasses harder and harder, due to fewer and fewer people taking their 3DS with them on a walk. This is where NetPass comes in!  

Unlike StreetPass, NetPass works over the internet. Upon opening NetPass, you can pick various locations to go to, i.e. the train station, or a town plaza. 

Upon entering a location, you get passes of others who are in the same location! And, while you are at the location, others who enter it can get passes with you. But beware! You can only switch locations once every 10 hours!


## Netpass Resources

- GBAtemp: [The Official Thread](https://gbatemp.net/threads/netpass-a-new-way-to-experience-streetpass.664005/)
- Discord: [The Netpass Community](https://netpass.cafe/discord)


## Translations
If you want to contribute to translations, we are doing that on [our Weblate](https://weblate.sorunome.de/projects/netpass)!


## Build Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started)

After installing devkitPro, you will need to download the following using [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman) or the devkitPro updater:

- 3ds-dev
- 3ds-curl

In other words, you'll need to run the following command in a Terminal/command prompt (with administrator/root privileges):

```bash
dkp-pacman -S 3ds-dev 3ds-curl
```

Furthermore, you need to have `ffmpeg` and `python` and `python-pyyaml` installed.

Be sure to run `make codegen` first.
```bash
make codegen
make
```


## Building Patches
If you want to build patches you have to manually dump their decrypted code from a 3ds and then place the file as `code.bin` into the respective patch folder. After that, you can run:
```bash
make patch
```

### Building `.CIA` (extra steps)
 - Makerom: You need the `makerom` executable in your `$PATH`
   You can get it precompiled on https://github.com/3DSGuy/Project_CTR/releases and then copy it to `$DEVKITPRO/tools/bin`
 - Bannertool: You need the `bannertool` executable in your `$PATH`
   You can get it precompiled on https://github.com/diasurgical/bannertool/releases

### Compiling everything

This project ships with a [Makefile](Makefile), which is meant to simplify the compilation process. If you're unfamiliar with them, you can find out more about GNU Make [here](https://www.gnu.org/software/make/).

```bash
make
```

To build `.cia` files, run

```bash
./build_release.sh
```


## Research Credits
 - [This gist](https://gist.github.com/wwylele/29a8caa6f5e5a7d88a00bedae90472ed) by wwylele, describing some cecd functionality
 - [This repo](https://github.com/NarcolepticK/CECDocs) by NarcolepticK documenting some more of the cecd sysmodule
 - [StreetPass 2](https://gbatemp.net/threads/streetpass-2-rise-from-the-ashes.526749/) for valuable data dumps
 - 3Dbrew and all its contributors, especially of the [cecd service](https://www.3dbrew.org/wiki/CECD_Services)


## License
The source code of this project is licensed under GPLv3 or later. See the included `LICENSE`.

Other assets, such as images and sounds, are licensed under CC-BY-NC. See the included `LICENSE-assets`.

