#!/bin/bash
#DDCToolbox binary needs to be in working directory

debname="ddctoolbox_"$1"_linux64"
echo $debname
mkdir $debname
mkdir $debname"/DEBIAN"
mkdir -p $debname"/usr/bin"
mkdir -p $debname"/usr/share/applications"
mkdir -p $debname"/usr/share/pixmaps"
cp "DDCToolbox" $debname"/usr/bin/ddctoolbox"

cp "img/icon.png" $debname"/usr/share/pixmaps/ddctoolbox.png"
cp "LICENSE" $debname"/DEBIAN"

cat <<EOT >> $debname"/usr/share/applications/ddctoolbox.desktop"
[Desktop Entry]
Name=DDC Toolbox
GenericName=DDC Editor
Comment=Create and edit DDCs on Linux
Keywords=editor
Categories=AudioVideo;Audio;Editor
Exec=ddctoolbox
Icon=/usr/share/pixmaps/ddctoolbox.png
StartupNotify=false
Terminal=false
Type=Application
EOT

cat <<EOT >> $debname"/DEBIAN/control"
Package: ddctoolbox
Version: $1
Section: sound
Priority: optional
Architecture: amd64
Depends: qtbase5-dev (>= 5.9.5), libqt5core5a (>= 5.9.5), libqt5widgets5 (>= 5.9.5), libqt5gui5 (>= 5.9.5), libqt5core5a (>= 5.9.5), libgl1-mesa-dev
Maintainer: ThePBone <tim.schneeberger@gmail.com>
Description: Create and edit DDCs on Linux
Homepage: https://github.com/ThePBone/DDCToolbox
EOT

dpkg-deb --build $debname
