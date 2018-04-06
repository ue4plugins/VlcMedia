#!/bin/bash
set -x

# Control if ./Plugins/VlcMedia/ folder exists.
if [ ! -d "./Plugins/VlcMedia/" ]; then
  printf "./Plugins/VlcMedia/ folder doesn't exist.\nRun bash script from Project (GAME) folder\n"
  exit 1
fi

# clean-up and configure the VlcMedia plugin
installDir=$PWD
installDir+="/Plugins/VlcMedia/ThirdParty/vlc/Linux/x86_64-unknown-linux-gnu/"
rm -rf $installDir
mkdir -p $installDir
#rm -rf ./protobuf/
#rm -rf ./vlc/

# update Ubuntu and remove vlc
sudo apt purge vlc
sudo snap remove vlc
sudo add-apt-repository --remove ppa:videolan/master-daily
sudo apt update
sudo apt upgrade
sudo apt autoremove

# To build vlc from source, the following tools are needed
sudo apt install autoconf automake libtool curl make g++ unzip
sudo apt install git build-essential pkg-config libtool autopoint gettext
sudo apt install subversion yasm cvs cmake ragel
sudo apt install gperf default-jdk ant flex flex-old bison 
sudo apt install libcaca-dev libncursesw5-dev libasound2-dev lua5.2-dev libxcb-keysyms1-dev
sudo apt install lua5.2 openjdk-8-jdk ant nasm libxcb-composite0 libxcb-composite0-dev
sudo apt install libxcb-xv0 libxcb-xv0-dev wayland-protocols

# Protocol Buffers - Google's data interchange format
#git clone git://github.com/google/protobuf.git
cd protobuf
./autogen.sh
./configure
make clean
make
make check
sudo make install
make clean
sudo ldconfig # refresh shared library cache
cd ..

# install vlc and make it available to the VlcMedia plugin
#git clone git://git.videolan.org/vlc.git
cd vlc
#git checkout tags/4.0.0-dev -b 4.0.0-dev
./bootstrap
# VLC contribs system, included in the VLC source
cd contrib
mkdir native
cd native
../bootstrap

make # Create VLC contribs system code to Hotfix it
# Hotfix for `ERROR: libmp3lame >= 3.98.3 not found` problem.
# CREDIT # https://stackoverflow.com/a/46756012/4510033
sed -i -e 's/\(3.98.3..lame.lame.h.lame.set.VBR.quality..lmp3lame\)/\1 -lm/' ./ffmpeg/configure

make
cd ..
cd ..

./configure --disable-srt --disable-dca --disable-libass --disable-css --disable-upnp --disable-chromaprint --disable-freetype --prefix=$installDir
make clean
make
make install
make clean
cd ..

