#!/bin/bash
#DEB build script for Travis-CI
#DDCToolbox binary needs to be in working directory

debname="ddctoolbox_"$base_version"-build"$TRAVIS_BUILD_NUMBER
echo $debname
mkdir $debname
mkdir $debname"/DEBIAN"
mkdir -p $debname"/usr/bin"
mkdir -p $debname"/usr/share/applications"
mkdir -p $debname"/usr/share/pixmaps"
cp "DDCToolbox" $debname"/usr/bin/ddc-toolbox"

cp "img/icon.png" $debname"/usr/share/pixmaps/ddc-toolbox.png"
cp "LICENSE" $debname"/DEBIAN"

cat <<EOT >> $debname"/usr/share/applications/ddc-toolbox.desktop"
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

cat <<EOT >> $debname"/DEBIAN/control"
Package: ddc-toolbox
Version: $base_version-$TRAVIS_BUILD_NUMBER
Section: sound
Priority: optional
Architecture: amd64
Depends: qtbase5-dev (>= 5.9.5), libqt5core5a (>= 5.9.5), libqt5widgets5 (>= 5.9.5), libqt5gui5 (>= 5.9.5), libqt5core5a (>= 5.9.5), libgl1-mesa-dev
Maintainer: ThePBone <tim.schneeberger@gmail.com>
Description: Create and edit DDCs on Linux
Homepage: https://github.com/ThePBone/DDCToolbox
EOT

dpkg-deb --build $debname
