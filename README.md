# DDCToolbox
[![Build status](https://ci.appveyor.com/api/projects/status/7akte2nk20j6u9w1?svg=true)](https://ci.appveyor.com/project/ThePBone/ddctoolbox)
![GitHub](https://img.shields.io/github/license/ThePBone/DDCToolbox) ![GitHub release](https://img.shields.io/github/release/ThePBone/DDCToolbox)

Create and edit ViPER DDC files on Linux

## Features
 * Add/edit/remove calibration points
 * Save/load *.vdcprj files
 * Export as *.vdc
 * Live preview
 * Edit values directly in the table
 * Import VDCs (to *.vdcprj)
 * Import AutoEQ configurations (*ParametricEQ.txt)

 
## Screenshot

![Screenshot](https://github.com/ThePBone/DDCToolbox/blob/master/img/screenshot.png?raw=true)

## Installation
Primarily developed for Linux. It should work on other platforms too, but you need to compile it yourself.

  * [Windows](#windows)
  * [Ubuntu](#ubuntu-ppa)
  * [Debian](#debian)
  * [Manually/Portable (Linux)](#manuallyportable)
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

### Windows
Windows builds are automatically built and deployed when a new commit has been pushed.

You can download one of these builds [from my server](https://nightly.thebone.cf/ddctoolbox-win/?C=M;O=D).
If the server is down, you can also get one from the [AppVeyor cloud](https://ci.appveyor.com/project/ThePBone/ddctoolbox) (select one of the two jobs (32/64-bit) and go to the tab 'Artifacts')

**NOTE:** The current release triggers a SmartScreen warning, you can ignore it though [(Virustotal scan here)](https://www.virustotal.com/gui/file/f54e6a9502a4f09cf9aa8b136d8f2c9ae9f643f9940af7af40027cbadc3ec004/detection)

### Debian
Users of debian-based distros can use the DEB-packages that are attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

### Manually/Portable
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

## Contributors
  * VDC Importer (vdc2vdcprj) - [James Fung (@james34602)](https://github.com/james34602)
_____________
Based on ViPERs Toolbox 2.0 for Windows
