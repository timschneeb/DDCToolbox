# DDCToolbox
Create and edit ViPER DDC files on Linux

## Features
 * Add/edit/remove calibration points
 * Save/load *.vdcprj files
 * Export as *.vdc
 * Live preview
 * Edit values directly in the table
 * Import AutoEQ configurations (*ParametricEQ.txt)

 
## Screenshot

![Screenshot](https://github.com/ThePBone/DDCToolbox/blob/master/img/screenshot.png?raw=true)

## Installation
Primarily developed for Linux. It should work on other platforms too, but you need to compile it yourself.
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
_____________
Based on ViPERs Toolbox 2.0 for Windows
