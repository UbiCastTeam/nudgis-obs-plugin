# Nudgis OBS Studio

## Description

The Nudgis OBS Studio plugin makes it easy to interface [Open Broadcaster Software](https://obsproject.com) with the [Nudgis platform from Ubicast](https://www.ubicast.eu).

The functionality of the plugin covers the following actions:
- performing live streams
- uploading recordings

The plugin is available for the following platforms:
- Windows 7/8/10/11 64 bits
- macOS (arm64 and x86)
- Linux: Ubuntu 64 22.04 (Jammy Jellyfish) bits and [Arch Linux](https://aur.archlinux.org/packages/nudgis-obs-plugin)  (Flatpack installation is also possible - see [Flatpack Installation](#flatpack))

See the release notes for the minimal version of each OS

## Usage

Check out [our tutorial](https://help.ubicast.tv/permalink/v1263f480e913w8vr71t/iframe/).

<a target="_blank" title="Nudgis OBS Plugin Tutorial" href="https://help.ubicast.tv/permalink/v1263f480e913w8vr71t/iframe/"><img src="https://help.ubicast.tv/public/videos/v1263f48137336b4p4tr3zew63db9a/thumb_link.png" alt="Nudgis OBS Plugin Tutorial"/></a>

## Installation

OS-specific packages are available in the [releases github page](https://github.com/UbiCastTeam/nudgis-obs-plugin/releases). 

An [AUR package](https://aur.archlinux.org/packages/nudgis-obs-plugin) is available for Arch Linux.

### Windows

Check out [our tutorial](https://help.ubicast.tv/permalink/v1263f5564abdmnmj38p/iframe/).

<a target="_blank" title="Installing the OBS Nudgis plugin on Windows" href="https://help.ubicast.tv/permalink/v1263f5564abdmnmj38p/iframe/"><img src="https://help.ubicast.tv/public/videos/v1263f55698ddjr0tg91rzxa77tacn/thumb_link.png" alt="Installing the OBS Nudgis plugin on Windows"/></a>

Double click the .exe and follow the instructions. A "Windows protected your PC" popup will appear; click "More info" and confirm to allow installation; follow the wizard, and confirm you want to install the plugin into the same folder as OBS (important).

> :warning: Microsoft Defender may tag some versions of the installer as malware (it depends on the release and Windows Defender database). If this happens follow these [instructions](docs/windows_av_bypass.md) until the Microsoft Defender DB is updated.

### macOS

Download the universal .pkg package from the [releases github page](https://github.com/UbiCastTeam/nudgis-obs-plugin/releases).

The universal package can be installed on both x86 and arm64 machines.

After download follow these steps:

* open the Download folder, then `right click` on the file and choose `Open`

![macos_0](docs/img/macos/macos_0.png)

* As the plugin is not signed you will have to confirm that you want to open it by clicking `Open`

![macos_1](docs/img/macos/macos_1.png)

* Click `Continue` on the installer until the `Installation Type` step, at this step you *must* click `Change Install Location` and then choose `Install for me only`

![macos_3](docs/img/macos/macos_3.png)
![macos_4](docs/img/macos/macos_4.png)

* If everything went well you will see this

![macos_5](docs/img/macos/macos_5.png)

> :warning:  Depending on the level of security you may also have to allow the installation of apps downloaded for elsewhere than App store, this can be done by opening the `System Preferences` and tick the `App Store and identified developers` box in `Security and Privacy` tab:

![macos_sec](docs/img/macos/macos_security.png)

(you will also have to explicitly allow the app in this window)

### Linux

#### Ubuntu 

##### Install OBS

With Ubuntu, OBS can be installed by different means:

* Ubuntu packages (with `apt install obs-studio`)
	
	```
	sudo apt install obs-studio
	```
	
* OBS offical ppa https://obsproject.com/wiki/install-instructions#linux

	```
	sudo apt install software-properties-common
	sudo add-apt-repository ppa:obsproject/obs-studio
	sudo apt update
	sudo apt install ffmpeg obs-studio
	```

* flatpak https://flathub.org/apps/details/com.obsproject.Studio

	```
	flatpak install flathub com.obsproject.Studio
	```

##### Install the corresponding Nudgis OBS Studio plugin

Depending on the Ubuntu version and OBS installation method, the version of Nudgis OBS plugin will vary and may need some adjustement. 

Check the release note and download the .deb package from the [releases github page](https://github.com/UbiCastTeam/nudgis-obs-plugin/releases).

**Nudgis OBS compatibility table (as of 16 Jan 2023)**

|                   | Ubuntu OBS package               | OBS PPA                          | flatpak                          |
| ----------------- | -------------------------------- | -------------------------------- | -------------------------------- |
| Focal 20.04 (LTS) | OBS-25/QT5, Not supported        | OBS-29/QT5, [v1.0.0 (OBS 27)][1] | OBS-29/QT6, [v1.0.0 (OBS 28)][2] |
| Jammy 22.04 (LTS) | OBS-27/QT5, [v1.0.0 (OBS 27)][1] | OBS-29/QT6, [v1.0.0 (OBS 28)][2] | OBS-29/QT6, [v1.0.0 (OBS 28)][2] |
| Kinetic 22.10     | OBS-28/QT5, [v1.0.0 (OBS 27)][1] | OBS-29/QT6, [v1.0.0 (OBS 28)][2] | OBS-29/QT6, [v1.0.0 (OBS 28)][2] |

[1]: https://github.com/UbiCastTeam/nudgis-obs-plugin/releases/download/v1.0.0-rc2/nudgis-obs-plugin-1.0.0-Linux.deb
[2]: https://github.com/UbiCastTeam/nudgis-obs-plugin/releases/download/v1.0.0_OBS-28/nudgis-obs-plugin-1.0.0-linux-x86_64.deb

After you download the right DEB package install it (and do some adjustement)

###### Ubuntu package and OBS ppa:

For [v1.0.0 (OBS 27)][1]:

```
sudo apt install ./nudgis-obs-plugin-1.0.0-Linux.deb
# Multiarch Ubuntu distribution needs some adjustements for this versrion of plugin:
sudo cp /usr/lib/obs-plugins/nudgis-obs-plugin.so /usr/lib/x86_64-linux-gnu/obs-plugins/
```

For [v1.0.0 (OBS 28)][2]:

```
sudo apt install ./nudgis-obs-plugin-1.0.0-linux-x86_64.deb
```

######  Flatpack

As of 16/01/2023, flatpak OBS is based on org.kde.Platform v6.4 which uses QT6 so the right DEB is [v1.0.0 (OBS 28)][2], but this DEB is meant for local install, for flatpak some extra step is needed:

1. create a tmp folder and extract the deb file inside:

	```
	mkdir /tmp/obs-nudgis-flatpack; cd /tmp/obs-nudgis-flatpack
	dpkg-deb -x /path/to/nudgis-obs-plugin-1.0.0-linux-x86_64.deb .
	```

* then copy the files to the right place

	```
	mkdir ~/.var/app/com.obsproject.Studio/config/obs-studio/plugins/nudgis-obs-plugin/{bin/64bit,data}
	cp usr/lib/x86_64-linux-gnu/obs-plugins/nudgis-obs-plugin.so ~/.var/app/com.obsproject.Studio/config/obs-studio/plugins/nudgis-obs-plugin/bin/64bit
	cp -r usr/share/obs/obs-plugins/nudgis-obs-plugin/locale ~/.var/app/com.obsproject.Studio/config/obs-studio/plugins/nudgis-obs-plugin/data
	```
	
#### Archlinux

Nudgis OBS plugin is available from AUR https://aur.archlinux.org/packages/nudgis-obs-plugin, use you prefered AUR packet manager to install it, for instance with `yay`:

```
yay -S nudgis-obs-plugin
```

## Uninstallation

### Windows

The plugin can be uninstalled like any other installed software in the "Add or remove programs" menu.

### macOS

To remove this plugin open the terminal and type:

```
rm -rf "$HOME/Library/Application Support/obs-studio/plugins/nudgis-obs-plugin.plugin/"
pkgutil --volume ~ --forget eu.ubicast.nudgis-obs-plugin
```

### Ubuntu

```
sudo apt remove nudgis-obs-plugin
```

## Development

### CI artifacts description

Deployment binaries are generated by the github/actions (Artifacts) CI, and are accessible from the [Actions section of the project repository](https://github.com/UbiCastTeam/nudgis-obs-plugin/actions).  

The file name of an Artifacts respects the following format:

\<name_of_plugin>-\<version_of_plugin>-\<platform>\

For example
<pre>
 nudgis-obs-plugin-1.0.0-linux-x86_64.deb
</pre>

Below is the listing of Artifact names, containing the deployment binaries for each platform for version 1.0.0 of the Nudgis OBS Studio plugin compiled for OBS >= 28:

| Platform | Artifact Name                                        | Note                                                                                                                              |
| -------- | ---------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| Linux    | nudgis-obs-plugin-1.0.0-linux-x86_64.deb             | Debian .deb package, may be used for other Linux platforms                                                                        |
| MacOS    | nudgis-obs-plugin-1.0.0-macos-arm64.pkg              | MacOS ARM machine (M1)                                                                                                            |
| MacOS    | nudgis-obs-plugin-1.0.0-macos-universal.pkg          | Universal MacOS package (intel x86 and ARM based)                                                                                 |
| MacOS    | nudgis-obs-plugin-1.0.0-macos-x86_64.pkg             | MacOS intel x86                                                                                                                   |
| Windows  | nudgis-obs-plugin-1.0.0-windows-x64-Installer.exe    | Universal 64 bits windows package                                                                                                 |
### Setting up the development environment on Ubuntu 20.04

#### OBS compilation and installation in debug mode

> :information_source: update the version to the current plugin and OBS version

Activation of deb-src repositories in /etc/apt/sources.list
<pre>
sudo perl -pi -0e 's/^(deb .*\n)# (deb-src)/$1$2/gm' /etc/apt/sources.list
sudo apt update
</pre>

Fetching sources, config
<pre>
sudo apt build-dep obs-studio
sudo apt install git wget libwayland-dev libxkbcommon-dev libxcb-composite0-dev libpci-dev qtbase5-private-dev
git clone https://github.com/obsproject/obs-studio.git
cd obs-studio
git checkout 27.2.4 -b 27.2.4
git submodule init
git submodule update
export CI_LINUX_CEF_VERSION=$(cat .github/workflows/main.yml | sed -En "s/[ ]+LINUX_CEF_BUILD_VERSION: '([0-9]+)'/\1/p")
wget https://cdn-fastly.obsproject.com/downloads/cef_binary_${CI_LINUX_CEF_VERSION}_linux64.tar.bz2
tar -xvaf cef_binary_${CI_LINUX_CEF_VERSION}_linux64.tar.bz2
cmake -B build . -DCMAKE_BUILD_TYPE=Debug -DENABLE_PIPEWIRE=FALSE -DCEF_ROOT_DIR=${PWD}/cef_binary_${CI_LINUX_CEF_VERSION}_linux64 -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g3' -DCMAKE_C_FLAGS_DEBUG='-O0 -g3'
</pre>

Compiling and Installing
<pre>
cmake --build build
sudo cmake --install build
sudo ldconfig
</pre>

#### nudgis-obs-plugin compilation and installation in debug mode

<pre>
git clone git@github.com:UbiCastTeam/nudgis-obs-plugin.git
cd nudgis-obs-plugin
cmake -B build . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g3' -DCMAKE_C_FLAGS_DEBUG='-O0 -g3'
cmake --build build
sudo cmake --install build
</pre>

#### Starting a GDB debug session

Installing gdb
<pre>
sudo apt install gdb
</pre>

Starting a debug session - with a breakpoint on the plugin initialization function
<pre>
gdb -ex 'set breakpoint pending on' -ex 'b src/nudgis-plugin.cpp:obs_module_load' -ex r obs
</pre>

Launching a debug session
<pre>
gdb obs
</pre>
