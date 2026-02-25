# godot-orbbec

orbbec SDK 2 wrapper GDExtension for godot 4.5+

Exposes a `OrbbecPointCloud` node which can list network devices, connect to one of them and relay frames of depth data with the `point_cloud_frame` signal.

## Current status

Works on linux and windows, currently tested with orbbec femto mega.

## Building
Note : if you want the OrbbecPointCloud object to work from the editor, you will need to build the debug version.

### linux
```
git submodule --recursive --init
./build.sh # BUILD_RELEASE=1 ./build.sh for release build
```

Note: if you have already built a release version, you will have to delete your the build folder to do a debug build.

### windows
```
git submodule --recursive --init
```

and then, in an admin cmd/powershell prompt
```
:: set BUILD_RELEASE=1 :: if you want to build release
build_windows.cmd
```
