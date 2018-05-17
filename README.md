## Building V8

To build V8 for embedding we need to mix the instructions from:

- [Getting Started with Embedding](https://github.com/v8/v8/wiki/Getting-Started-with-Embedding)
- [Building from source](https://github.com/v8/v8/wiki/Building-from-Source) 

In short this is:

```
# Clone depot_tools
cd /usr/local/code/
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

# Add following to bashrc
export PATH=$PATH:/path/to/depot_tools

# Update 
gclient

# Create a new directory for v8
fetch v8
cd v8

# CRUCIAL - DOES NOT WORK WITH LATEST MASTER
# Switch to 5.8
# git checkout -b 5.8 -t origin/5.8-lkgr
git checkout -b 6.7 -t origin/6.7-lkgr

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
# Also make sure target build is x64
gn args out.gn/x64.release

# Compile
ninja -C out.gn/x64.release

# Run tests
tools/run-tests.py --gn
```

This will build V8 binaries suitable for embedding. The CMake config in this repo assumes the v8 directory is at
`/usr/local/code/v8`.