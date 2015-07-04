# Introduction #

Thanks to the existence of the Mingw and Wine, one no longer needs to build the Windows binaries on Windows.

Here are the steps needed to cross-compile the Xchecklist:
  * Install g++-mingw-w64, gcc-mingw-w64 and wine (package names that are used on Ubuntu, other distros might differ).
  * Download the "Microsoft Windows SDK for Windows 7 and .NET Framework 4 (ISO)" it is needed for Speech api
  * Extract from the ISO the headers/lib files.
  * Using the provided scripts compile the Xchecklist for both 32 and 64 bit Linux and Windows.


# Details #

## Install Mingw and Wine ##
Mingw installation is easily done using your distro's package managing program (Synaptic, apt-get, yum, ...) - just install Mingw packages for 32 and 64 bit Windows cross-compilation, along with the Wine.

## Microsoft Windows SDK for Windows 7 and .NET Framework 4 (ISO) ##
Download the above mentioned ISO (GRMSDK\_EN\_DVD.iso, ~500MB) either using this [link](http://www.microsoft.com/en-us/download/details.aspx?id=8442) or just google the heading and it should lead you to the file.

Mount the ISO (Disk image mounter comes handy here) and start the wine's explorer (command 'wine explorer').

In the explorer, navigate through disk Z: to the place where the ISO is mounted and there to the Setup/WinSDKBuild directory (/media/user/UDF\ Volume/Setup/WinSDKBuild/ on my machine). There open the file WinSDKBuild\_x86.msi and wait for the headers/libs to get installed.

Now in Linux navigate to your Wine prefix (~/.wine by default) and in drive\_c/Program Files (x86)/Microsoft SDKs/Windows/v7.1 you will find the headers and library files. Copy the needed files to the WinSDK directory (at the same level where the Xplane SDK directory is) according to this:

```
WinSDK/
├── Include
│   ├── sapi51.h
│   ├── sapi.h
└── Lib
    ├── sapi.lib
    └── x64
        └── sapi.lib
```

## Compile ##
Now you can just run the build4linux script and when it is done, the resulting code is in the release directory.