
# We don't need an IDE. We are the IDE.

IDE does some magic for us. Unfortunately, the magic does not work outside of Hogwartz (Windows OS). So we have to carefully recreate magic energy by ourselves.

## Embedded world

To build anything for embedded devices we need the following:
 - Compiler, capable of running on our system and producing binaries for embedded hardware
 - Standard library (if any) for the embedded hardware
 - Vendor provided libraries (Hardware abstraction layer and handy utils)
 - Linker script
 - Loader program
 - Build system to put it all together

IDE does nothing else other than providing all these services. 

### Compiler

We will use `arm-none-eabi-gcc` compiler. Unlike some other compilers (e.g. `rustc`) single compiled version of GCC can produce binaries for only a single architecture. **See example**:

> Your computer most likely has GCC installed. If you run `gcc -v` (not `--version`) you will see something like:
> ```
> Using built-in specs.
> COLLECT_GCC=gcc
> COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-linux-gnu/14.2.1/lto-wrapper
> Target: x86_64-pc-linux-gnu
> Configured with: /build/gcc/src/gcc/configure --enable-languages=ada,c,c++,d,fortran,go,lto,m2,objc,obj-c++,rust --enable-bootstrap --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=https://gitlab.archlinux.org/archlinux/packaging/packages/gcc/-/issues --with-build-config=bootstrap-lto --with-linker-hash-style=gnu --with-system-zlib --enable-__cxa_atexit --enable-cet=auto --enable-checking=release --enable-clocale=gnu --enable-default-pie --enable-default-ssp --enable-gnu-indirect-function --enable-gnu-unique-object --enable-libstdcxx-backtrace --enable-link-serialization=1 --enable-linker-build-id --enable-lto --enable-multilib --enable-plugin --enable-shared --enable-threads=posix --disable-libssp --disable-libstdcxx-pch --disable-werror
> Thread model: posix
> Supported LTO compression algorithms: zlib zstd
> gcc version 14.2.1 20240910 (GCC) 
> ```

> [!NOTE] 
> See `Target` line. With your default GCC compiler you can build only for `x86-64` machines, not for `ARM`.

 To build binaries for a concrete architecture, you need to compile GCC with support for that architecture. Likely for us, many Linux distros provide compiled versions of GCC for most architectures, including `arm-none-eabi`. This scary name means nothing more than: `arm` architecture, `none` vendor, `eabi` os (meaning no OS).

> [!NOTE]
> Operational system defines which ABI (Application Binary Interface) a compiler should use. 
> You can think of an OS in the target triplet as an ABI chooser.
> Here we specify `eabi` which stands for `embedded-abi`.

### Libc the Standard Library

Vendor libraries often require `libc`. Considering that we are targeting STM32 ARM SoC, we need `libc` for embedded arm devices. [`newlib`](https://www.sourceware.org/newlib/) is a `libc` implementation for various embedded devices, *including ARM*. We will use it too.

### Vendor libraries

Our vendor is ST Microelectronics™. [`Drivers`](./Drivers/) contains almost all libraries provided for `stm32f4xx` SoC (some libraries have special licensing policy hence not provided here). *Libraries gently taken from [here](https://github.com/STMicroelectronics/STM32CubeF4).*

### Linker script

Linker script maps our code to address space of an STM32 SoC. Linker scripts lays out interrupt vectors table, setups entry point, defines where RAM and FLASH are. It combines all the code scattered across compiled object files into single big `.text` section. Same with `.data` and some other sections. Linker script provides symbols for important addresses, such as maximum RAM address.

Linker scripts uses complex syntax. Linker script's official reference can be found [**here**](https://sourceware.org/binutils/docs/ld.html).

Linker script is configured for SDK1.1M development board. Sources: https://github.com/lmtspbru/SDK-1.1M.

### Loader

For loader program we will use `stlink`.

### Build system

To put it all together we use [Meson](https://mesonbuild.com) - user friendly build system. This repo has everything configured for you. Here we will explore a little what does Meson do to arrange the build. 

## Meson build system

[Meson](https://mesonbuild.com) is a user friendly build system generator. That means, meson does not *build* your files, but generates `ninja` build system to build your files. Meson makes cross-compilation easy!

> *Cross-compilation*: the code will run on a machine other than the one it was compiled on

Meson does cross-compilation via `cross-files`. This repo contains such file: [`cross_gcc.build`](./cross_gcc.build). The file setups which compiler meson should use. It also configures additional CFLAGS and linker flags. 

> [!TIP]
> If you are wondering why cross file specify `host_machine` and not `target_machine`, here is why (take from [here](https://mesonbuild.com/Cross-compilation.html)):
> - *build machine* is the computer doing the actual compiling.
> - *host machine* is the machine on which the compiled binary will run.
> - *target machine* is the machine on which the compiled binary's output will run, only meaningful if the compiled binary produces machine-specific output.

Meson build system declares which source files (both `.h` and `.c`) participate in the build. Our own source files resides in [`Core`](./Core) directory. We include vendor libraries into our own build rather then building them as a subproject or a dependency. This is because we [**`configure`**](./Core/Inc/stm32f4xx_hal_conf.h) vendor libraries for features we need. This reduces compile times and size of a produced binary.

Unfortunately, meson does not support automatic discovery of new source files. ***So if you create new file, you have to manually add it to [`Core/meson.build`](./Core/meson.build)***.

We setup `CFLAGS` to further optimize code size: `-ffunction-sections`, `-fdata-sections` and `-Wl,--gc-sections`. These options make linker exclude unused functions and (global) variables from final binary. 


# Enough words! Tell me how to build

## Dependencies

Repo has many things configured for you. You have to install compiler and standard library. 

For archlinux:
```
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib
```

> [!NOTE]
> Package name is <code>arm-none-eabi-<b>newlib</b></code>, not `...-libc`!

---

For Ubuntu:
```
sudo apt install gcc-arm-none-eabi
sudo apt install libnewlib-arm-none-eabi
```
may be required:
```
sudo apt install binutils-arm-none-eabi
```

---

For MacOS:
```
brew install gcc-arm-embedded
```

These are the only external dependencies.

## Build

Meson generates `ninja` build system in `build` directory with settings from `cross_gcc.build` file. 
```
meson setup --cross-file cross_gcc.build build
```

To do actual compilation:
```
cd build
ninja
```
or shorthand:
```
ninja -C build
```
or if you prefer to not call `ninja` directly:
```
meson compile -C build
```

# `newlib` (the libc)

Authors distribute `newlib` in source-only form. Compiled version *may* be provided by a package manager. If it isn't, you have to build it manually or download from [ARM Developers Portal](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).

`newlib` is a part of gcc project. It is a dedicated git repo though. This means that when you clone `https://sourceware.org/git/newlib-cygwin.git` you end up with the following directory structure:
```shell
./newlib-cygwin         # newlib project's root directory.
                        # Don't be scared of the name. This simply means, 
                        # that newlib support not only gcc but also cygwin.
./newlib-cygwin/newlib  # Actual newlib sources
```

## Build `newlib`

`newlib` utilizes `autoconf` build system. Building is done in two steps:

 - ***configure*** - discovers compilers and configures build environment
 - ***build*** - does the actual build

### *Configure everything* approach
> [!IMPORTANT]
> Both steps must be done in `newlib`'s root project directory: `./newlib-cygwin`.
> 
> ***NOT in `./newlib-cygwin/newlib`***.


Here is an example with `git clone` included:
```shell
git clone https://sourceware.org/git/newlib-cygwin.git
cd newlib-cygwin
mkdir build
cd build
CFLAGS="-fno-builtin -nostdlib -nostartfiles" ../configure --target=arm-eabi-none
```

### Build only for single architecture

If you try to configure `newlib` by `./newlib-cygwin/newlib/configure`, it won't work. This is because newlib checks C PreProcessor *sanity* (`newlib` uses `CPP` shorthand for C PreProcessor. So don't confuse with C Plus Plus). Autoconf performs the check by trying to preprocess simple program. The program, though, includes `assert.h` and `limits.h`. If we have only the `arm-none-eabi-gcc` and no `newlib` *installed*, the compiler fails the check. The reason is that the compiler can not find these headers, because (Surprise!) they are part of `newlib`, which is not installed on the system yet.

I could not find a way to disable these checks without removing `AC_PREPROC_IFELSE` macros used by multiple `m4` files which in turn are used by `configure.ac`. The `./newlib-cygwin/configure` also performs some fixups and disables some more `autoconf` options that otherwise cause configuration to fail.

The configuration succeeds if `newlib` is already installed.


