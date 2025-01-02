## I. Installing 3rd-parties

sView requires several 3rd-party components for building:
* C/C++ compiler (g++, MSVC 2010+)
* FFmpeg (https://www.ffmpeg.org)
* OpenAL soft (https://openal-soft.org/)
* FreeType
* libconfig++, Linux and Android (https://www.hyperrealm.com/libconfig/libconfig.html)
* libxrandr, libxpm, libfontconfig, Linux only
* zenity, Linux only (https://help.gnome.org/users/zenity/stable/)

On Debian/Ubuntu you might use the following command to install all dependencies at once:

~~~~~
sudo apt-get install \
  g++ cmake \
  zenity \
  libopenal-dev \
  libgl-dev \
  libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libswscale-dev \
  libconfig++-dev libconfig-dev \
  libfreetype-dev libfontconfig-dev \
  libxrandr-dev libxpm-dev \
  libopenvr-dev
~~~~~

The similar command for RPM-based distributives:

~~~~~
yum install gcc gcc-c++ \
  gtk+-devel \
  mesa-libGLU-devel glew-devel \
  openal-devel \
  libconfig-devel

rpm -Uvh http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm
yum install ffmpeg-devel
~~~~~

ALT Linux:
~~~~~
apt-get install \
  gcc8-c++ gcc-c++ libGLU-devel libXrandr-devel libfreetype-devel \
  libopenal-devel libconfig-devel libconfig-c++-devel libXpm-devel \
  libavcodec-devel libavdevice-devel libavformat-devel libavutil-devel libswscale-devel
~~~~~

On Windows and macOS please refer to official documentation for each project.
Notice that DevIL and FreeImage libraries are optional and are not required for building sView
(libraries are loaded dynamically if available).

## II. Makefile (Linux, macOS, Android)

sView project defines a single `Makefile` for building an application and all libraries.
There are no configuration scripts (like `./configure`), although building options can be adjusted by passing arguments to `make` or by modification of `Makefile` file.

When building sView on Debian-based distributive - all you need is to install dependencies via package manager and to execute traditional commands:

~~~~~
  make && make install
~~~~~

### Android

sView for Android platform is built in the same way as for Linux platforms - via UNIX `Makefile`.
sView does not provide `.mk` files - it uses UNIX Makefile for building native code.

`Makefile` coming with sView performs all building steps - from building native `.so` libraries, to compiling `.java` source code and generation of APK package.
Paths to NDK and 3rd-party libraries can be specified through command-line options to **make** or by editing `Makefile` itself.
The building can be run on a Linux host with compatible Java SDK, Android NDK and Android SDK versions.

### Qt Creator

*distribution/qmake/sView.pro* defines a project file for Qt Creator.
This is not a self-sustained solution, but rather a wrapper over existing UNIX `Makefile`, allowing to develop and build sView for macOS, Linux and Android targets within IDE.

## IV. CMake

sView comes within CMake scripts, compatible with Windows (Visual Studio 2015 and higher) and Linux.

## V. Building options

Several preprocessor directives control building options.
Notice that by default "include/stconfig.conf" file is used to override these options
(this file will be used only when ST_HAVE_STCONFIG is defined).

* `ST_HAVE_MONGOOSE` - should be defined to activate built-in web UI for remote Movie Player control
* `ST_DEBUG` - should be defined to activate debugging log output
* `ST_DEBUG_LOG_TO_FILE` - specifies file name or full path to duplicate debug log output

## VI. Distribution scripts

Several script were written to automate distribution routines.
All them were placed in "distribution" folder.

* `build.bat`, batch script for Windows. Performs re-building of x86 and x86_64 targets
  using Visual Studio and pack result binaries using InnoSetup script.
  Notice that all dependencies should be available (including InnoSetup).
* `buildDebSrc.sh`, bash script for Linux to pack sources into Debian source package.
* `buildMac.sh`, bash script to pack binaries from **bin/MAC_gcc** folder into DMG image.
  3rd-party libraries should be already located in "bin/MAC_gcc_DEBUG/sView.app/Contents/Frameworks/"
  folder with correct search path (refer to `bind_frameworks.sh` auxiliary script).

~~~~~
su
yum install rpm-build

mkdir -p ${HOME}/workspace/redhat/{RPMS,SRPMS,SPECS,SOURCES,BUILD}
echo "%_topdir ${HOME}/workspace/redhat" > ${HOME}/.rpmmacros

wget https://launchpad.net/~sview/+archive/stable/+files/sview_12.05-1%7Eprecise.tar.gz
mv sview_12.05-1~precise.tar.gz $HOME/workspace/redhat/SOURCES/sview_12.05-1.tar.gz
rpmbuild -ba distribution/sView.rpm.spec
~~~~~
