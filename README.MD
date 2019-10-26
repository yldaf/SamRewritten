Steam Achievements Manager for Linux (SamRewritten)
===

[//]: # (**TODO TAKE A SCREENSHOT WHEN JUNK IS GONE**)

![SamRewritten logo](assets/icon.png)

# Introduction

SamRewritten is an open-source program that allows you to unlock and relock your Steam achievements. It is named after the famous Steam Achievements Manager written by Gibbed, and aims to implement its functionalities on Linux and more!

It comes with third-party tools integration and a nice user interface.

SamRewritten is constantly under development, if you think you can give us a hand, feel free to reach us :)

# Command line options

I know you linux geeks love to use command line options.
We have big ambitions for SamRewritten, and I believe many of them are related to these command-line options, so stay put, because that list will be growing soon!

* Prevent the window from showing and set your status to "In Game"
* -a \<appid\>
	* `./bin/launch.sh -a 10` will idle Counter Strike

# Building

To build this project, clone this repository on your PC, use `cd` to move into the project folder, and type `make` to build the program.

Alternatively, you can download one of our releases in the AppImage format. Check the 'Release' tab of this repository to find them.
To build your own AppImage, build SamRewritten normally, then execute `build_appimage.sh` in the `package` folder, it will take care of everything for you.

If you're encountering issues building SamRewritten, make sure you have installed the required packages on your machine. You'll most likely need Steam, yajl, GTK 3, and GLib.

* Ubuntu (and variants): `$ sudo apt install libgtk-3-dev libcurl4-gnutls-dev libyajl-dev`

* Arch (and variants): `$ sudo pacman -S gtk3 glibc gnutls yajl`

To run SamRewritten, simply launch `$ ./bin/launch.sh`

# AUR Installation

You can install SamRewritten by using an AUR and installing the `samrewritten-git` package.

Run SamRewritten by opening the corresponding desktop entry, or by using the `samrewritten` command.

### Footnotes

This software comes with no warranty. Use it at your own risk. We believe you should not get any ban using it, but we do not take any responsability over your user experience
