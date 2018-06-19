## Nodejs

[Install instructions](https://nodejs.org/en/download/package-manager/) contain the following for v10:

```
curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
sudo apt-get install -y nodejs
```

## Emscripten

[Install instructions](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). Basically:

```
cd /usr/local/code
git clone https://github.com/juj/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest

# Run this before executing any emsdk stuff
source /usr/local/code/emsdk/emsdk_set_env.sh
```

## Compiling

Once Emscripten is installed, you need to run the following to compile things:

