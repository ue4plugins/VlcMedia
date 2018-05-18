#!/bin/bash
set -x

# https://askubuntu.com/questions/907873/how-to-set-clang-3-9-as-the-default-in-zesty

sudo apt update
sudo apt upgrade
sudo apt autoremove

sudo apt install clang clang-5.0

sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-5.0/bin/clang++ 100
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-5.0/bin/clang 100

sudo update-alternatives --config clang

clang --version

