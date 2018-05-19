## Building V8

### Docs

To build V8 for embedding we need to mix the instructions from:

- [Getting Started with Embedding](https://github.com/v8/v8/wiki/Getting-Started-with-Embedding)
- [Building from source](https://github.com/v8/v8/wiki/Building-from-Source)
- [Version numbers](https://github.com/v8/v8/wiki/Version-numbers)

### Version

Note that the **version is crucial** and steps in the version numbers wiki should be followed all the way through.
This involves:

- Go to https://omahaproxy.appspot.com/ and find the V8 version related to the current stable chrome (scroll right)
- Having used `depot_tools` to check out the repo, you can check out `branch-heads/<minor-version>` replacing `<minor-version>` with the version above to "one decimal place"

For example:

- Stable Chrome V8 version = `6.6.346.32`
- `git checkout branch-heads/6.6`

Note, if changing version, make sure you check `samples/hello-world.cc` in the V8 repo to check for obvious changes.

### Clang

Also note that V8 version 6+ uses **`clang` and not `g++`**. This means you need to make
sure you have your machine set up to use clang and libc++, i.e.:

```
sudo apt-get install clang libc++-dev
```

And edit your `/etc/environment` to contain:

```
CXX=/usr/bin/clang++
CC=/usr/bin/clang
CPP=/usr/bin/clang
LINK=/usr/bin/clang++
CXX_host=/usr/bin/clang++
CC_host=/usr/bin/clang
CPP_host=/usr/bin/clang
LINK_host=/usr/bin/clang++
GYP_DEFINES="clang=1"
```

Note the crucial `-stdlib=libc++` argument when building the project.

And restart your machine.

### CLion

You need to set CLion to use clang for C/C++ compilation. To do this you need to edit
the "Toolchains" settings, changing both the C and C++ compilers.

### Build

The build steps are as follows:

```
# Clone depot_tools
cd /usr/local/code/
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

# Add following to bashrc
export PATH=$PATH:/usr/local/code/depot_tools

# Update 
gclient

# Create a new directory for v8
fetch v8
cd v8

# CRUCIAL - see notes on version numbers above
git checkout branch-heads/6.6

# Download deps
gclient sync

# Install build deps
./build/install-build-deps.sh

# Generate build files
rm -rf out.gn
./tools/dev/v8gen.py x64.release

# Edit the default build configuration and add two lines for embedding:
#
#   is_component_build = false
#   v8_static_library = true
#
# Also double check that the target build is "x64"
gn args out.gn/x64.release

# Compile
ninja -C out.gn/x64.release

# To check it's working you can compile the hello-world example
clang++
```

You should then be able to build this project.
