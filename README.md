## CLTestbench

Provides a playground to experiment with OpenCL commands.

This should be useful for OpenCL and driver developers.


## Usage

CLTestbench provides a command-line interface to create, and manipulate, OpenCL primitives, such as buffers, programs, and kernels.  This should provide the means to quickly modify kernels and their inputs without having to rebuild a host application.


### Commands

When started, CLTestbench will show the prompt `(bench)`.  From here, the developer can use several commands:

* `help` will list the commands and can be used to query further details on specific commands.
* `load` will allow loading an OpenCL implementation.  This must be done before OpenCL can be used.
* `into` will display loaded platform and device information.
* `select` will choose which platform and device to use from the loaded implementation.
* `quit` will terminate the testbench session.

(tbc)

### Builtin functions

Objects can be constructed using the builtin functions:

* `program` will create an OpenCL program object.
* `kernel` will create an OpenCL kernel object.
* `file` will load the contents of a file into memory.

### Example

~~~
(bench) load /usr/lib64/libOpenCL.so
(bench) prog = program(file("source.cl"))
(bench) kern = kernel(prog, testKernel)
~~~


## Building

CLTestbench is built using CMake.   You will need a C++17 capable toolchain.

Building has only ever been tested under Linux.  It is possible to support other operating systems, but that has not been added.


### Dependencies

The build configuration will automatically fetch the [Khronos OpenCL headers](https://github.com/KhronosGroup/OpenCL-Headers.git) and, if testing is enabled, [Catch2](https://github.com/catchorg/Catch2) on systems where it isn't already available.

You will need either GNU `readline` or BSD `editline`.  If both are available on the system, you can select which one is used through CMake `USE_LIBEDIT=ON` (`OFF` by default).


## Testing

Tests have been written using [Catch2](https://github.com/catchorg/Catch2) and are placed inside the `test` subdirectory of the repository.


## Licensing

The CLTestbench is licenced under the LGPLv3 or later, in hopes it could be useful to others.

Note that the CLTestbench can use `readline` which is licenced under the GPL.  If this is an issue, consider building against `libedit` instead (see `Building` above).


## Legal
OpenCL and the OpenCL logo are trademarks of Apple Inc.
