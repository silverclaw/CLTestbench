## CLTestbench

Provides a playground to experiment with OpenCL commands.

This should be useful for OpenCL and driver developers.


## Usage

CLTestbench provides a command-line interface to create, and manipulate, OpenCL primitives, such as buffers, programs, and kernels.  This should provide the means to quickly modify kernels and their inputs without having to rebuild a host application.

Images can also be created and loaded from PNG files, if support was compiled in.

### Example

~~~
# Load an OpenCL source from file.
(bench) prog = program(file("source.cl"))
# Compiles a kernel.
(bench) kern = kernel(prog, testKernel)
# Create an OpenCL buffer from a file.
(bench) arg0 = buffer(file("input_data.bin"))
# Create a blank OpenCL buffer of 1024 bytes.
(bench) arg1 = buffer(1024)
# Runs the compiled kernel with NDRange (1024, 1, 1)
(bench) run kern((1024, 1, 1), arg0, arg1)
# Wait for the kernel to finish execution.
(bench) wait
# Save the contents of the buffer into a file.
(bench) save arg1 output_data.bin
~~~

### Commands

When started, CLTestbench will show the prompt `(bench)`.  From here, the developer can use several commands:

* `help` will list the commands and can be used to query further details on specific commands.
* `load` will allow loading an OpenCL implementation.  CLTB will load the default system implementation automatically.
* `info` will display loaded platform and device information.
* `select` will choose which platform and device to use from the loaded implementation.
* `set` sets CLTB options.
* `script` will read a given filename and execute the commands line by line.
* `quit` will terminate the testbench session.

### Builtin functions

Objects can be constructed using the builtin functions:

* `program` will create an OpenCL program object.
* `kernel` will create an OpenCL kernel object.
* `file` will load the contents of a file into memory.

Data objects can also be constructed with literals, such as:

~~~
(bench) integers = int(3, 5, 3, 6, 1)
(bench) floats = float(3.0, -4)
~~~

## Building

CLTestbench is built using CMake.  You will need a C++17 capable toolchain.

Building has only ever been tested under Linux.  It is possible to support other operating systems, but that has not been added.


### Dependencies

The build configuration will automatically fetch the [Khronos OpenCL headers](https://github.com/KhronosGroup/OpenCL-Headers.git) and, if testing is enabled, [Catch2](https://github.com/catchorg/Catch2) on systems where it isn't already available.

You will need either GNU `readline` or BSD `editline`.  If both are available on the system, you can select which one is used through CMake `USE_LIBEDIT=ON` (`OFF` by default).

For PNG loading support, `libpng` must be available.  If not, CLTB will be compiled without support for loading PNG images.


## Testing

Tests have been written using [Catch2](https://github.com/catchorg/Catch2) and are placed inside the `test` subdirectory of the repository.

## CLIntercept

CLTestbench ships a utility library called CLIntercept which traces OpenCL calls on a host application and generates a script file that can later be used by CLTestbench to duplicate another program's execution.


## Licensing

The CLTestbench is licenced under the LGPLv3 or later, in hopes it could be useful to others.

Note that the CLTestbench can use `readline` which is licenced under the GPL.  If this is an issue, consider building against `libedit` instead (see `Building` above).


## Legal
OpenCL and the OpenCL logo are trademarks of Apple Inc.
