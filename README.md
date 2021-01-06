<h1 align="center">
  DDCToolbox
  <br>
</h1>
<h4 align="center">Create and edit ViPER DDC files on Linux, Windows, macOS and Android</h4>
<p align="center">
  <a href="https://github.com/ThePBone/DDCToolbox/releases">
    <img alt="GitHub downloads count" src="https://img.shields.io/github/downloads/ThePBone/DDCToolbox/total?label=downloads%20%28windows%29">
  </a>
  <a href="https://github.com/ThePBone/DDCToolbox/releases">
  	<img alt="GitHub release (latest by date)" src="https://img.shields.io/github/v/release/thepbone/DDCToolbox">
  </a>
  <a href="https://github.com/ThePBone/DDCToolbox/blob/master/LICENSE">
      <img alt="License" src="https://img.shields.io/github/license/thepbone/DDCToolbox">
  </a>
  <a href="https://ci.appveyor.com/project/ThePBone/ddctoolbox">
    <img alt="AppVeyor" src="https://ci.appveyor.com/api/projects/status/7akte2nk20j6u9w1?svg=true">
  </a>
</p>
<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#contributors">Contributors</a> •
  <a href="#license">License</a> 
</p>


![Screenshot](https://github.com/ThePBone/DDCToolbox/blob/master/img/screenshot.png?raw=true)

## Features
 * Save/load VDC project files
 * Import/export VDCs
 * Direct AutoEQ integration
 * Undo/redo framework 
 * Edit values directly in the table
 * Interactive magnitude response, phase response and group delay plot
 * Various IIR filters (peaking, low/high pass/shelf, notch, all/band pass and unity gain)
 * Embed custom IIR filters
 * Stability check for filters

# Installation
Developed with Linux in mind. You can also find Windows and macOS installation instructions below.
  * [Windows](#windows)
  * [Android](#android)
  * [Arch (AUR)](#arch-aur)
  * [Ubuntu](#ubuntu-ppa)
  * [Debian](#debian-unmanaged-packages)
  * [Manually (Linux)](#manuallyportable-linux)
  * [Manually (macOS)](#manually-macos)

## Windows

### Stable releases
You can find stable windows packages attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

### Nightly builds
These windows builds are automatically built and deployed once a new commit has been pushed.

You can download one of these builds [from my server](https://nightly.timschneeberger.me/ddctoolbox-win).

If the server is down, you can also get one from the [AppVeyor cloud](https://ci.appveyor.com/project/ThePBone/ddctoolbox) (select one of the two jobs (32/64-bit) and go to the tab 'Artifacts')

## Android
There is an Android version of DDCToolbox with limited functionality available here: [ThePBone/DDCToolbox-Android](https://github.com/ThePBone/DDCToolbox-Android)

## Arch (AUR)
A git package is available in the [AUR](https://aur.archlinux.org/packages/ddctoolbox-git/).
```bash
yay -S ddctoolbox-git
```
![AUR version](https://img.shields.io/aur/version/ddctoolbox-git?label=aur-git)

## ~Ubuntu (PPA)~
**The Ubuntu PPA is DISCONTINUED. It will not receive new updates and is stuck on version 1.4! If you are interested in helping to package this app, please open an issue.**

**Qt 5.11 or later required.** Ubuntu 20.04 (or later releases) provide this version of Qt in their official repositories.

Add PPA Repo
```bash
curl -s --compressed "https://thepbone.github.io/PPA-Repository/KEY.gpg" | sudo apt-key add -
sudo curl -s --compressed -o /etc/apt/sources.list.d/thepbone_ppa.list "https://thepbone.github.io/PPA-Repository/thepbone_ppa.list"
sudo apt update
```
Install from PPA **(Not recommended)**
```bash
sudo apt install ddc-toolbox
```
[View PPA on GitHub](https://github.com/ThePBone/PPA-Repository)

I'll look into packaging this app as a snapcraft or flatpak instead. As soon as a proper alternative is available, I'll shutdown the PPA repository.

## Debian (unmanaged packages)
**Qt 5.11 or later required.** Debian 10 (or later releases) provide this version of Qt in their official repositories.

Since these packages are not downloaded by your package manager, you will not be notified about any updates.

### Stable releases
Users of debian-based distros can use the DEB-packages that are attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

### Nightly builds
You can download one of these DEB-builds [from my server](https://nightly.timschneeberger.me/ddctoolbox-debian).
They are automatically compiled and may contain bugs.

## Manually/Portable (Linux)
### Build from sources

**Requirements:**
 * Qt 5.11 or later

Install dependencies (Debian)

    sudo apt install qt5-qmake qtbase5-dev libgl1-mesa-dev

Install dependencies (Arch)

    sudo pacman -S qt5-base 

Clone this repository

    git clone https://github.com/ThePBone/DDCToolbox

Compile sources

    cd DDCToolbox
    qmake
    make

You should now be able to execute it:

    ./DDCToolbox

#### Optional: Installation and shortcut
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
## Manually (macOS)
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
  * and more


## License

This project is licensed under [GPLv3](LICENSE).

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

