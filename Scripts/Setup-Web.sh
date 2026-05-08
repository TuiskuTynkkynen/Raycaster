#!/usr/bin/env bash

pushd ..

# Generate glad bindings
if [ ! -d ./Raycaster-Core/Dependencies/glad/include/ ]; then
    echo Generating glad...
    pushd ./Raycaster-Core/Dependencies/glad_generator

    python -m venv venv
    source ./venv/bin/activate

    python -m pip install -r requirements.txt
    python -m pip install -e .
    glad --api gl:compatibility=4.6 --out-path ../glad

    deactivate
    rm -rf ./venv
    popd
fi

# Compile libogg to a library
if [ ! -d ./Raycaster-Core/Dependencies/libogg/build/lib/ ]; then
    pushd ./Raycaster-Core/Dependencies/libogg

    ./autogen.sh
    emconfigure ./configure --prefix=$PWD/build
    emmake make
    emmake make install

    popd
fi

# Compile libopus to a library
if [ ! -d ./Raycaster-Core/Dependencies/libopus/build/lib/ ]; then
    pushd ./Raycaster-Core/Dependencies/libopus

    ./autogen.sh
    emconfigure ./configure --host=wasm32-unknown-emscripten --disable-shared --enable-static --prefix=$PWD/build 
    emmake make
    emmake make install

    popd
fi

./Vendor/Binaries/Premake/Linux/premake5 --os=emscripten --cc=clang --file=Build.lua gmake

popd
