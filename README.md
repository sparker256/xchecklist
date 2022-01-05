Xchecklist for X-Plane 11, 10 and 9
=========================

[![Build Status](https://travis-ci.org/sparker256/xchecklist.svg)](https://travis-ci.org/sparker256/xchecklist)

![Alt text](docs/Xchecklist_GUI.jpg?raw=true "Xchecklist_GUI")

![Alt text](docs/Xchecklist_Widget.jpg?raw=true "Xchecklist_Widget")


Alow you to have a interactive checklist in your 2d or VR cockpit

## Building Xchecklist from source

### On windows

1.  Install required software using [Chocolatey](https://chocolatey.org/) using admin command prompt:

    ```
    choco install git cmake
    choco install mingw --version 8.1.0
    ```

    You can also install the same programs manually if you prefer.

2.  Checkout and configure the project:

    ```
    git clone https://github.com/sparker256/xchecklist.git
    cd xchecklist
    cmake -G "MinGW Makefiles" -S .\src -B .\build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    ```

3.  Build the project and copy the plugin DLL into the appropriate directory:

    ```
    cmake --build .\build
    cp .\build\win.xpl .\Xchecklist\64\
    ```

### On Ubuntu:

1. Install required software:

   ```
   sudo apt-get install -y --no-install-recommends build-essential cmake git freeglut3-dev libudev-dev libopenal-dev libspeechd-dev

   ```

2. Checkout and configure the project:

   ```
   git clone https://github.com/sparker256/xchecklist.git
   cd xchecklist
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   cp ./build/lin.xpl ./Xchecklist/64/
   ```

### On OS X:

1. Install XCode, Git, CMake (Homebrew can be convenient for this).

2. Checkout and configure the project:

   ```
   git clone https://github.com/sparker256/xchecklist-3.git
   cd xchecklist
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   cp ./build/mac.xpl ./Xchecklist/64/
   ```
