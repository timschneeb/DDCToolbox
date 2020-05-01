# DDCToolbox
[![Build status](https://ci.appveyor.com/api/projects/status/7akte2nk20j6u9w1?svg=true)](https://ci.appveyor.com/project/ThePBone/ddctoolbox)
![GitHub](https://img.shields.io/github/license/ThePBone/DDCToolbox) ![GitHub release](https://img.shields.io/github/release/ThePBone/DDCToolbox)

Create and edit ViPER DDC files on Linux, Windows and macOS

## Features
 * Save/load *.vdcprj files
 * Export as *.vdc
 * Import VDCs (to *.vdcprj)
 * Direct AutoEQ integration
 * Proper undo/redo framework 
 * Edit values directly in the table
 * Interactive magnitude response, phase response and group delay plot
 * Various IIR filter types (peaking, low/high pass/shelf, notch, all/band pass and unity gain)
 * Embed custom IIR filters
 * Stability check for filters

## Screenshot

![Screenshot](https://github.com/ThePBone/DDCToolbox/blob/master/img/screenshot.png?raw=true)

## Installation
Developed with Linux in mind. You can also find Windows and macOS installation instructions below.
  * [Windows](#windows)
  * [Arch (AUR)](#arch-aur)
  * [Ubuntu](#ubuntu-ppa)
  * [Debian](#debian)
  * [Android](#android)
  * [Manually (Linux)](#manuallyportable-linux)
  * [Manually (macOS)](#manually-macos)
### Ubuntu (PPA)
Add PPA Repo
```bash
curl -s --compressed "https://thepbone.github.io/PPA-Repository/KEY.gpg" | sudo apt-key add -
sudo curl -s --compressed -o /etc/apt/sources.list.d/thepbone_ppa.list "https://thepbone.github.io/PPA-Repository/thepbone_ppa.list"
sudo apt update
```
Install from PPA
```bash
sudo apt install ddc-toolbox
```
[View PPA on GitHub](https://github.com/ThePBone/PPA-Repository)

### Arch (AUR)
A git package is available in the [AUR](https://aur.archlinux.org/packages/ddctoolbox-git/).
```bash
yay -S ddctoolbox-git
```

### Windows

#### Stable
You can find stable windows packages attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

#### Nightly
These windows builds are automatically built and deployed once a new commit has been pushed.

You can download one of these builds [from my server](https://nightly.thebone.cf/ddctoolbox-win/?C=M;O=D).

If the server is down, you can also get one from the [AppVeyor cloud](https://ci.appveyor.com/project/ThePBone/ddctoolbox) (select one of the two jobs (32/64-bit) and go to the tab 'Artifacts')

**NOTE:** The current release triggers a SmartScreen warning, you can ignore it though [(Virustotal scan here)](https://www.virustotal.com/gui/file/f54e6a9502a4f09cf9aa8b136d8f2c9ae9f643f9940af7af40027cbadc3ec004/detection)

#### Note when compiling from sources

If you want to use the new AutoEQ integration, you'll need OpenSSL DLLs along your exe-File (Windows-only obviously). I'll provide those libraries with every (stable) release on Windows. However, if you need/want to compile this app by yourself, you'll have to add them manually. If you're version target is Qt 5.13 and higher (which is recommended) you can just copy libssl\*.dll and libcrypto\*.dll from the root of this repo into the working directory of DDCToolbox.

### Debian
Users of debian-based distros can use the DEB-packages that are attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

### Android
There is an Android version of DDCToolbox with limited functionality available here: [ThePBone/DDCToolbox-Android](https://github.com/ThePBone/DDCToolbox-Android)

### Manually/Portable (Linux)
#### Build from sources
_(You can find precompiled binaries [here](https://github.com/ThePBone/DDCToolbox/releases))_

 Install dependencies

    sudo apt install qt5-qmake qtbase5-dev libgl1-mesa-dev

Clone this repository

    git clone https://github.com/ThePBone/DDCToolbox

Compile sources

    cd DDCToolbox
    qmake
    make

You should now be able to execute it:

    ./DDCToolbox


#### Optional: Manual Install
Copy to /usr/local/bin
```bash
sudo cp DDCToolbox /usr/local/bin/ddc-toolbox
sudo chmod 755 /usr/local/bin/ddc-toolbox
```
Create Menu Entry
```bash
sudo cat <<EOT >> /usr/share/applications/ddc_toolbox.desktop
[Desktop Entry]
Name=DDC Toolbox
GenericName=DDC Editor
Comment=Create and edit DDCs on Linux
Keywords=editor
Categories=AudioVideo;Audio;Editor
Exec=ddc-toolbox
Icon=/usr/share/pixmaps/ddc-toolbox.png
StartupNotify=false
Terminal=false
Type=Application
EOT
```
Download Icon
```bash
sudo wget -O /usr/share/pixmaps/ddc-toolbox.png https://raw.githubusercontent.com/ThePBone/DDCToolbox/master/img/icon.png -q --show-progress
```
### Manually (macOS)
Note: these installation steps are untested but should still work.

Install Homebrew

    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

Install QT
    
    brew install qt5

Symlink QT

    brew link qt5 --force

Clone the repo
    
    git clone https://github.com/ThePBone/DDCToolbox

Compile it

    cd DDCToolbox
    qmake
    make

Run it
    
    open ./DDCToolbox.app

## Contributors
* [James Fung (@james34602):](https://github.com/james34602)
  * VDC Importer
  * Group delay plot
  * Unity gain filter 
_____________
Based on ViPERs Toolbox 2.0 for Windows
