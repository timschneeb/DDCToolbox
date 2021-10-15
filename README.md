<h1 align="center">
  <img alt="Icon" width="75" src="https://github.com/ThePBone/DDCToolbox/blob/master/res/img/icon.png?raw=true">
  <br>
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
  <a href="https://github.com/ThePBone/DDCToolbox/">
    <img alt="Windows build" src="https://img.shields.io/github/repo-size/thepbone/ddctoolbox">
  </a>
</p>
<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#contributors">Contributors</a> •
  <a href="#license">License</a> 
</p>

<p align="center">
    <img alt="Screenshot" src="https://github.com/ThePBone/DDCToolbox/blob/master/res/img/screenshot.png?raw=true">
</p>

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
 * Advanced peaking filter curve fitting (using [libMultivariateOpt](https://github.com/james34602/libMultivariateOpt))

# Installation
Developed with Linux in mind. You can also find Windows and macOS installation instructions below.

Recommended:
* **Windows**
  * [Windows (Stable)](#windows-stable)
  * [Windows (Nightly)](#windows-nightly)
* **Linux**
  * [Debian/Ubuntu (PPA)](#debianubuntu-ppa)
  * [Arch Linux (AUR)](#arch-aur)
* **macOS**
  * [macOS (OSX 10.13 and later, Intel x64 only)](#macos-intel)
* **Android**
  * [Android app (basic features only)](#android)

Other installation methods (not recommended):
* **Linux**
  * [Debian (Unmanaged/Nightly)](#debian-nightly)
  * [Build from sources (Linux)](#manuallyportable-linux)
* **macOS**
  * [Build from sources (macOS)](#manually-macos)

## Windows (Stable)
You can find stable windows packages attached on the [release page](https://github.com/ThePBone/DDCToolbox/releases).

## Windows (Nightly)
These windows builds are automatically built and deployed once a new commit has been pushed. They may contain bugs but are always cutting-edge.

You can download one of these builds [from my server](https://nightly.timschneeberger.me/ddctoolbox-win).

If the server is down, you can also get one from the GitHub artifact storage:
1. Visit the [actions section](https://github.com/ThePBone/DDCToolbox/actions?query=workflow%3A%22Windows+static+build%22)
2. Select the latest job at the top that finished successfully
3. On the next page, scroll all the way down to the artifacts section
4. Select your architecture and choose whether you want an installer or standalone executable.

**NOTE: You need a GitHub account to download these build artifacts, otherwise you'll get redirected to a 404 page.**

## Debian/Ubuntu (PPA)

Recommended system requirements:
* Distro based on Debian 11 or later **OR**
* Distro based on Ubuntu 20.04 or later

Add PPA Repo
```bash
sudo apt install -y curl
curl -s --compressed "https://thepbone.github.io/PPA-Repository/KEY.gpg" | sudo apt-key add -
sudo curl -s --compressed -o /etc/apt/sources.list.d/thepbone_ppa.list "https://thepbone.github.io/PPA-Repository/thepbone_ppa.list"
sudo apt update
```
Install from PPA

```bash
sudo apt install ddctoolbox
```
[View PPA on GitHub](https://github.com/ThePBone/PPA-Repository)

## Arch (AUR)
A git package is available in the [AUR](https://aur.archlinux.org/packages/ddctoolbox-git/).
```bash
yay -S ddctoolbox-git
```
![AUR version](https://img.shields.io/aur/version/ddctoolbox-git?label=aur-git)

## Android
There is an Android version of DDCToolbox with limited functionality available here: [ThePBone/DDCToolbox-Android](https://github.com/ThePBone/DDCToolbox-Android)

## macOS (Intel)
macOS support is currently in beta; only cloud-compiled nightly binaries are available. The next stable release will include stable versions of OSX binaries.
OSX 10.13 (High Sierra) or later Intel macOS devices only. If your device is older or an ARM-based device, try to [compile it from sources yourself](#manually-macos).

You can download one of nightly beta builds [from my server](https://nightly.timschneeberger.me/ddctoolbox-mac).

## Debian (Nightly)
**Qt 5.11 or later required.** Debian 10 (or later releases) provide this version of Qt in their official repositories.

Since these nightly (beta) packages are not downloaded by your package manager, you will not be notified about any updates. **If possible, please install this app from the stable [PPA](#debianubuntu-ppa) instead!**

You can download one of these nightly debian packages or precompiled binaries [from my server](https://nightly.timschneeberger.me/ddctoolbox-linux).
They are automatically compiled and may contain bugs. Dependencies are not included with precompiled standalone executables.

## Manually/Portable (Linux)
### Build from sources

**Requirements:**
 * Qt 5.11 or later

Install dependencies (Debian)

    sudo apt install qt5-qmake qtbase5-dev libgl1-mesa-dev

Install dependencies (Arch)

    sudo pacman -S qt5-base 

Clone this repository

    git clone --recurse-submodules https://github.com/ThePBone/DDCToolbox

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
    
    git clone --recurse-submodules https://github.com/ThePBone/DDCToolbox

Compile it

    cd DDCToolbox
    qmake
    make

Run it
    open ./DDCToolbox.app

## Contributors
* [James Fung (@james34602):](https://github.com/james34602)
  * Peaking filter curve fitting ([libMultivariateOpt](https://github.com/james34602/libMultivariateOpt))
  * VDC Importer
  * Group delay plot
  * Unity gain filter 
  * [and more](https://github.com/ThePBone/DDCToolbox/commits?author=james34602)


## License

This project is licensed under [GPLv3](LICENSE).

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

