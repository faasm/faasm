#!/bin/bash

cd /tmp/

wget https://github.com/curl/curl/archive/curl-7_61_0.tar.gz
tar -xvf curl-7_61_0.tar.gz
cd curl-curl-7_61_0

CONFIG_FLAGS="--without-ssl --disable-ares --disable-cookies --disable-crypto-auth --disable-ipv6"
CONFIG_FLAGS="$CONFIG_FLAGS --disable-manual --disable-proxy --disable-verbose --disable-versioned-symbols"
CONFIG_FLAGS="$CONFIG_FLAGS --without-libidn --without-librtmp --without-ssl --without-zlib"

# Common
./buildconf

# Normal
#./configure $CONFIG_FLAGS
#make

# Emscripten
/usr/local/code/emsdk/emscripten/1.38.11/emconfigure ./configure $CONFIG_FLAGS
/usr/local/code/emsdk/emscripten/1.38.11/emmake make

# Output should now be at /tmp/curl-curl-7_61_0/lib/.libs/libcurl.so


