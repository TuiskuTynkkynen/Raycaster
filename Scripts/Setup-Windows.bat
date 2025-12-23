@echo off

pushd ..

if not exist .\Raycaster-Core\Dependencies\glad\include\ (
    echo Generating glad...
    pushd .\Raycaster-Core\Dependencies\glad_generator

    python -m venv venv
    cmd /c ".\venv\Scripts\activate && python -m pip install -r requirements.txt && python -m pip install -e . && glad --api gl:compatibility=4.6 --out-path ..\glad && deactivate"

    rmdir /S /Q .\venv
    popd
)

Vendor\Binaries\Premake\Windows\premake5.exe --file=Build.lua vs2022

popd
pause
