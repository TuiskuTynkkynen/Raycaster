#!/usr/bin/env bash

pushd ..

if [ ! -d ./Raycaster-Core/Dependencies/glad/include/ ]; then
    echo Generating glad...
    pushd ./Raycaster-Core/Dependencies/glad_generator

    python -m venv venv
    source ./venv/bin/activate

    python -m pip install -r requirements.txt
    python -m pip install -e .
    glad --api gl:compatibility=3.3 --out-path ../glad

    deactivate
    rm -rf ./venv
    popd
fi

./Vendor/Binaries/Premake/Linux/premake5 --cc=clang --file=Build.lua gmake

popd
