#!/bin/bash

set -e

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
sudo apt-get update
sudo apt-get install -y clang-6.0

pushd /usr/bin
sudo ln -s clang-6.0 clang
sudo ln -s clang++-6.0 clang++
sudo ln -s llvm-6.0 llvm
popd


