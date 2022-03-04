# GordonFlashTool
Toolset for Gotek SFR1M44-U100 formatted usb flash drives.

## Features

- Store and extract fdd images.
- Compute checksum for each image.
- Tested to work (and compile) on ppc64 MacOS X 10.4.

## How to install on Fedora

Under root:

`dnf copr enable hackr/GordonFlashTool`

`dnf install gordon-flash-tool`

## How to compile

Simply type `make` for build without boot code.

For boot code to be compiled, you need `nasm` installed and type `make all-boot-code`.

Supported compilers are:

- At least `gcc-4.0` (work in progress for `gcc-3.3` support).
- `clang` (no old version tested yet).

## Preview
![gordon in shell](/docs/imgs/preview1.png)
