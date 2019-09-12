# DDCToolbox
Create and edit ViPER DDC files on Linux

## Screenshot

![Screenshot](https://github.com/ThePBone/DDCToolbox/blob/master/img/screenshot.png?raw=true)

## Installation
Primarily developed for Linux. It should work on other platforms too, but you need to compile it yourself.
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
sudo cp DDCToolbox /usr/local/bin
sudo chmod 755 /usr/local/bin/DDCToolbox
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
Exec=DDCToolbox
Icon=/usr/share/pixmaps/ddc_toolbox.png
StartupNotify=false
Terminal=false
Type=Application
EOT
```
##### Download Icon
```bash
sudo wget -O /usr/share/pixmaps/ddc_toolbox.png https://raw.githubusercontent.com/ThePBone/DDCToolbox/master/img/icon.png -q --show-progress
```
