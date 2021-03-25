## Manual requirements installation for VlcMedia

Execute any of the given stepts from the root of your project directory.

**Unbutu 18.04**

```
sudo add-apt-repository ppa:videolan/master-daily
sudo apt-get update
sudo apt install vlc
sudo ln -sf /usr/lib/x86_64-linux-gnu/libvlc.so.5 /usr/lib/x86_64-linux-gnu/libvlc.so
sudo ln -sf /usr/lib/x86_64-linux-gnu/libvlccore.so.9 /usr/lib/x86_64-linux-gnu/libvlccore.so
mkdir -p ./Plugins/VlcMedia/ThirdParty/vlc/Linux/
ln -sf /usr/lib/x86_64-linux-gnu/ ./Plugins/VlcMedia/ThirdParty/vlc/Linux/x86_64-unknown-linux-gnu
```

_Ubuntu 18.04 is now done no further steps are required_

**Ubuntu 16.04**

`sudo apt install live-media yasm cvs gperf ragel autoconf automake cmake make pkgconf gcc unzip git subversion apache-ant flex gettext curl libtool libcaca lua52 xcb-util-keysyms qtmir-desktop qtbase5-private-dev`

**Arch**

`sudo pacman -S base-devel git pkg-config autoconf automake live-media yasm cvs gperf ragel cmake pkgconf gcc unzip git subversion ant flex gettext curl libtool libcaca lua52 xcb-util-keysyms`

`export CPATH=/usr/include/liveMedia/:$CPATH`

**General**

```
NUMCORES=$(nproc --all)
installDir=$PWD
installDir+="/Plugins/VlcMedia/ThirdParty/vlc/Linux/x86_64-unknown-linux-gnu/"
git clone git://github.com/google/protobuf.git
cd protobuf
./autogen.sh
./configure
make -j $NUMCORES
sudo make install
sudo ldconfig

git clone git://git.videolan.org/vlc.git
cd vlc
./bootstrap
cd contrib
mkdir native
cd native
../bootstrap
make -j $NUMCORES
cd ../../
./configure --disable-srt --disable-dca --disable-libass --disable-css --disable-upnp --disable-chromaprint --disable-freetype --prefix=$installDir
make -j $NUMCORES
make install
```

