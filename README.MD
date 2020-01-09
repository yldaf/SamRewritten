# SamRewritten
A Steam Achievements Manager for Linux
---

<p align=center>
	<img src="assets/icon_256.png" alt="SamRewritten logo"/>
</p>

<p align=center>
	<img src="assets/screenshot.png" alt="SamRewritten screenshot"/>
</p>

<p align=center>
	<img src="assets/screenshot2.png" alt="SamRewritten screenshot"/>
</p>

# Introduction

SamRewritten is an open-source program that allows you to unlock and relock your Steam achievements. It is named after the famous Steam Achievement Manager written by Gibbed, and aims to implement its functionalities on Linux, and more!

It comes with third-party tools integration and a nice user interface.

SamRewritten is constantly under development, if you think you can give us a hand, feel free to reach us :)

# Building

To build this project, clone this repository on your PC, and run the `make` command within the repository folder that you downloaded.

Alternatively, you can download one of our releases in the AppImage format. Check the 'Release' tab of this repository to find them.
To build your own AppImage, build SamRewritten normally, then execute `build_appimage.sh` in the `package` folder, it will take care of everything for you.

For Arch users, there is also a PKGBUILD within the `package` folder if you wish to build it using `makepkg`.

If you're encountering issues building SamRewritten, make sure you have installed the required packages on your machine. You'll most likely need Steam, curl, yajl, GTK 3, and GLib.

* **Ubuntu 19.04** and above: `$ sudo apt install libgtkmm-3.0-dev libcurl4-gnutls-dev libyajl-dev`

* **Arch** and variants: `$ sudo pacman -S gtkmm3 glibc gnutls yajl`

* **openSUSE** Tumbleweed: `$ sudo zypper install gtkmm3 gcc-c++ gnutls libcurl-devel libyajl-devel-static`

To run SamRewritten, simply launch `$ ./bin/launch.sh`

# AUR Installation

You can install SamRewritten from the AUR by installing the `samrewritten-git` package.

Run SamRewritten by opening the corresponding desktop entry, or by using the `samrewritten` command.

# Command line options

I know you linux geeks love to use command line options! SamRewritten has a few, let us know if you want to see more!

```
Usage:
  samrewritten AppId [OPTION...]

      --apps        Get the list of your owned apps.
  -h, --help        Show CLI help.
  -a, --app arg     Set which AppId you want to use. Same as using positional
                    'AppId'
  -i, --idle        Set your Steam profile as 'ingame'. Ctrl+c to stop.
      --ls          Display stats (TODO) and achievements for selected app.
      --unlock arg  Unlock achievements for an AppId. Separate achievement
                    names by a comma.
      --lock arg    Lock achievements for an AppId. Separate achievement
                    names by a comma.
      --launch      Actually just launch the app.
```

---

## Third-party apps integration

SamRewritten uses JSON to communicate between the interface and the "backend logic". Communications are transmitted via unix sockets. If you plan to make an app using some of SamRewritten's features, feel free to ask for help, for features, or to contribute.

## Footnotes

This software is licensed under the GNU GPL v3 and comes with no warranty. Use it at your own risk. We believe you should not get any ban using it, but we do not take any responsability over your user experience
