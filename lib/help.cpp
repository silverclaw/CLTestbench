// This file is part of CLTestbench.

// CLTestbench is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CLTestbench is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.

#include <cassert>
#include <iostream>

#include "commands.hpp"
#include "driver.hpp"
#include "istringview.hpp"
#include "object.hpp"
#include "testbench.hpp"
#include "token.hpp"

using namespace CLTestbench;

namespace
{
void GenericHelp(std::ostream& out)
{
    out << "Available commands:\n"
           " * load FILENAME             - Loads the given OpenCL implementation library.\n"
           " * info lib|platform|device  - OpenCL implementation information.\n"
           " * select plaform|device N   - Selects a platform and device to use.\n"
           "                               Defaults to the first platform/device found (N is 0).\n"
           "                               Ommitting argument shows currently selected platform/device.\n"
           " * VAR = EXPRESSION          - Defines a variable constructed with an evaluated expression.\n"
           "                               Use 'help expression' to list supported expressions.\n"
           " * release VAR               - Clears a defined variable, releasing associated memory.\n"
           " * run                       - Runs a kernel with the given arguments.\n"
           " * list                      - Lists all defined variables.\n"
           " * save                      - Saves a data object to disk.\n"
           " * script                    - Runs commands from a script file.\n"
           " * flush                     - Flushes the queued commands.\n"
           " * wait                      - Blocks until all pending OpenCL commands finish.\n"
           " * clone                     - Clones a CL Object.\n"
           "Use 'help command' for command-specific information.  Use 'help expression' for a list of\n"
           "functions available for object construction.\n";
}

void HelpForExpressionProgram(std::ostream& out)
{
    out << "program(data [,ARGS])\n"
           "Compiles the given data as an OpenCL program.\n"
           "The resulting object can then be used for kernel() calls.\n"
           "Build arguments can be specified as a string.\n"
           "Example:\n"
           "    p = program(file(\"source.cl\"), \"-cl-fast-relaxed-math -cl-std=CL2.0\")\n"
           "If the program fails to compile, appropriate diagnostics will be printed.\n";
}

void HelpForExpressionBinary(std::ostream& out)
{
    out << "binary(data)\n"
           "Builds the given program binary.\n"
           "The resulting object can then be used for kernel() calls.\n"
           "Example:\n"
           "    p = binary(file(\"prog.bin\"), \"-cl-fast-relaxed-math -cl-std=CL2.0\")\n"
           "If the program fails to compile, appropriate diagnostics will be printed.\n";
}

void HelpForExpressionKernel(std::ostream& out)
{
    out << "kernel(PROGRAM, NAME)\n"
           "Retrieves an OpenCL kernel from the given program.\n"
           "Example:\n"
           "    p = program(file(\"source.cl\"), \"-cl-fast-relaxed-math -cl-std=CL2.0\")\n"
           "    k = kernel(p, \"entryPoint\")\n";
}

void HelpForExpressionBuffer(std::ostream& out)
{
    out << "buffer(DATA [, START[, LEN]])\n"
           "Creates an OpenCL buffer object from DATA.\n"
           "The size of the buffer is calculated from the given start and length offsets.\n"
           "By default, the entire data is used.\n"
           "The second form of the command:\n"
           "    buffer(SIZE)\n"
           "Will create a buffer with the given size in bytes.  The buffer will be uninitialised.\n"
           "In both forms of the command, the buffer will be created with read-write permissions.\n";
}

void HelpForExpressionImage(std::ostream& out)
{
    out << "image(data)\n"
           "image(data, width, height, format, type)\n"
           "Creates an OpenCL Image object from the given data.  If the given data object is\n"
           "not of image data, then additional parameters must be provided for the properties.\n"
           "When creating a data object from an image file (using the 'file' expression), if\n"
           "support for that format has been compiled in, the properties will be inferred.\n"
           "For example:\n"
           "    imagedata = file(\"file.png\")\n"
           "    imageobject = image(imagedata)\n"
           "Will create an OpenCL image object from 'file.png', with the relevant properties\n"
           "loaded from the file, assuming PNG support has been enabled.\n"
           "If there is image support available, or if the data buffer was created from raw data\n"
           "then the image properties must be manually specified in order:\n"
           "    width, height, format, type\n"
           "Specifying a HEIGHT value of 0 will create a 1D image (instead of 2D).\n"
           "Format must be one of:\n"
           " * CL_R    - Single-channel.\n"
           " * CL_RA   - Two-channel.\n"
           " * CL_RGB  - Three-channel.\n"
           " * CL_RBGA - Four-channel.\n"
           "The OpenCL standard supports more channel formats, but are not parsed by CLTestbench yet.\n"
           "Type must be one of:\n"
           " * {u}char  - 8-bit channel (unnormalised).\n"
           " * {u}short - 16-bit channel (unnormalised).\n"
           " * {u}int   - 32-bit channel (unnormalised).\n"
           " * half     - 16-bit float.\n"
           " * float    - 32-bit float.\n"
           "The type will be auto-completed by the Testbench.\n"
           "The type specified need not match the type used to create the data buffer.\n"
           "The OpenCL standard supports more channel types, but are not parsed by the Testbench yet.\n"
           "The loaded OpenCL implementation may have restrictions on the combination of image formats\n"
           "and types used, as well as image dimensions.  If the image creation fails, the error will\n"
           "be reported.\n"
           "If you provide image properties when DATA itself is already an image buffer, the properties\n"
           "will be used instead, overriding the properties of the loaded image.  The underlying data\n"
           "will not be modified.\n"
           "There is no 3D image support yet.\n";
}

void HelpForExpressionFile(std::ostream& out)
{
    out << "file(FILENAME [, START[, LEN]])\n"
           "Loads the contents of the given file into memory, optionally specifying read offset\n"
           "and data length.  Lenght, if provided, will be truncated to the file size, adjusted\n"
           "by the read offset.\n"
           "If appropriate support has been enabled, the contents of the file will be decoded\n"
           "after loading, such as when loading image files.\n"
           "Example:\n"
           "    data = file(\"data.bin\")\n"
           "    buffer = buffer(data)\n"
           "    imagedata = file(\"image.png\")\n"
           "    image = image(imagedata)\n";
}

void HelpForExpressionType(std::ostream& out)
{
    out << "TYPE(DATA[, DATA...])\n"
           "Creates a data object based on the given type.\n"
           "This object can then be used to initialise an OpenCL buffer,\n"
           "or set as a kernel argument.\n"
           "TYPE can be any of the following OpenCL C types:\n"
           " * Floating point: half, float double.\n"
           " * Integer types: char, short, int, long, and their unsigned variants.\n"
           "Examples:\n"
           "    fdata = float(1, 2, 3, -10)\n"
           "    uidata = uint(45, 0xFFFF)\n"
           "These data objects can be used to create buffers:\n"
           "    arg = buffer(fdata)\n"
           "Or used as kernel arguments:\n"
           "    run(kernel, (1, 1, 1), int(3))\n"
           "The underlying data will be copied to the OpenCL implementation.\n"
           "The size of an OpenCL buffer created this way will be inferred by the size\n"
           "of the data type specified.\n";
}

void HelpForExpressionClone(std::ostream& out)
{
    out << "clone(IDENTIFIER)\n"
           "Creates a clone of the given object.\n"
           "Kernel objects are cloned using clCloneKernel.\n"
           "Memory objects are created and their contents copied with clEnqueueCopyBuffer.\n"
           "Similarly, images are cloned using clEnqueueCopyImage.\n"
           "Program objects cannot be cloned, as the CL API provides no means to do this.\n"
           "CLTestbench objects are not clonable because they are read-only, thus a simple\n"
           "alias would normally be sufficient.  Support for cloning these objects might\n"
           "be added in the future if/when the need arises.\n";
}

void HelpForExpression(std::ostream& out, TokenStream& tokens)
{
    if (tokens) {
        Token subexpr = tokens.consume();
        IStringView command = tokens.getTokenText(subexpr);
        const std::initializer_list<std::string_view> commands {
            "program", "kernel", "buffer", "image", "file", "type", "clone", "binary"
        };

        switch (command.autocomplete(commands)) {
        case 0: HelpForExpressionProgram(out); return;
        case 1: HelpForExpressionKernel(out); return;
        case 2: HelpForExpressionBuffer(out); return;
        case 3: HelpForExpressionImage(out); return;
        case 4: HelpForExpressionFile(out); return;
        case 5: HelpForExpressionType(out); return;
        case 6: HelpForExpressionClone(out); return;
        case 7: HelpForExpressionBinary(out); return;
        case IStringView::ambiguous:
            out << "Ambiguous argument for help expression '" << command << "'\n";
            return;
        case std::string_view::npos:
            out << "Unknown expression command '" << command << "'.\n";
            return;
        }
    }

    out << "IDENTIFIER = EXPRESSION\n"
           "Evaluates EXPRESSION and defines an object named IDENTIFIER with the result.\n"
           "EXPRESSION may be a composite of various commands:\n"
           " * program(SOURCE [, OPTIONS])      - Creates a CL program from SOURCE with given OPTIONS.\n"
           " * kernel(PROGRAM, NAME)            - Creates kernel NAME from from PROGRAM.\n"
           " * buffer(DATA [, START[, LEN]])    - Creates a memory buffer from DATA, with specified offset and length.\n"
           " * buffer(SIZE)                     - Creates a memory buffer with specified SIZE.\n"
           " * image(data[, PROPERTIES])        - Creates an image object from the provided data buffer.\n"
           " * file(FILENAME [, START[, LEN]])  - Loads the contents of this file,\n"
           "                                      optionally specifying start offset and length.\n"
           " * CLTYPE(DATA[,DATA...])           - Creates a data object based on CLTYPE and assigns the contents.\n"
           "                                      The CLTYPE corresponds to the OpenCL C types:\n"
           "                                      Floating point: half, float double.\n"
           "                                      Integer types: char, short, int, long, and their unsigned variants.\n"
           "                                      Use 'help expression type' for more information.\n"
           " * IDENTIFIER                       - Evaluates to an already-existing object.\n"
           "                                      This will not clone the underlying object!\n"
           "                                      It simply creates an alias.\n"
           " * clone(IDENTIFIER)                - Creates a clone of an existing object.\n"
           "Use 'help expression X' for further information on 'X'.\n";
}

void HelpForSet(std::ostream& out)
{
    out << "Sets testbench options.  Use with 'set [OPTION {ON|OFF|ARG}]'.\n"
           "Available operands:\n"
           " * verbose    - Controls output of information output.  Default ON.\n"
           " * silent     - Disables ALL testbench output.  Default OFF.\n"
           " * caret      - Controls output of diagnostic caret.  Default ON.\n"
           " * block      - Sets driver commands as blocking. Default ON.\n"
           "                See IMPORTANT notes below!\n"
           "Accepted values for boolean arguments are (case insensitive):\n"
           " * ON         - 'true', 'yes', 'on', '1', 'y', t'\n"
           " * OFF        - 'false', 'no', 'off', '0', 'n', f'\n"
           "\n"
           "IMPORTANT!  Notes when using 'block OFF':\n"
           "The CLTestbench will not automatically hold references to any temporary\n"
           "objects used when creating commands while parsing expressions.\n"
           "This means that expressions like:\n"
           "  obj = buffer(int(1, 2, 3))\n"
           "when blocking commands is false, will likely cause use-after-free errors.\n"
           "This is because the temporary data object created by the expression\n"
           "  int(1, 2, 3)\n"
           "will be destroyed after the expression evaluator finishes.  However, the\n"
           "buffer creation expression will enqueue a WriteBuffer OpenCL command which\n"
           "will attempt to read from the now-released memory.\n"
           "To prevent this, use a named object instead:\n"
           " data = int(1, 2, 3)\n"
           " obj = buffer(data)\n"
           "\n"
           "Further to the above, the CLTestbench uses an in-order queue, so commands\n"
           "will always execute in the same order as they are submitted.\n"
           "Remember to use the 'wait' command to block until all commands are finished.\n"
           "Kernel enqueue commands *always* execute as non-blocking.  This is because\n"
           "the EnqueueNDRangeKernel OpenCL function does not have a blocking argument.\n"
           "This could be emulated with relative ease by the CLTestbench, but the 'block'\n"
           "option described here is meant to map directly to the 'blocking' argument of\n"
           "the OpenCL API calls.\n";
    return;
}

void HelpForSave(std::ostream& out)
{
    out << "save OBJECT FILE\nWrites a the given OBJECT to a file.\n"
           "If OBJECT is a CL Buffer, an implicit READ is performed.\n"
           "If OBJECT is a CL Image, an implicit READ is performed.\n"
           "If OBJECT is a CL Program, a program binary is generated.\n";
    return;
}

void HelpForRun(std::ostream& out)
{
    out << "run KERNEL((SIZE), [(LOCAL_SIZE),] ARGS...)\nRuns a kernel.'\n"
           " * KERNEL       must evaluate to a OpenCL Kernel object.\n"
           " * (SIZE)       must contain up to 3 constants, specifying the enqueue size.\n"
           " * (LOCAL_SIZE) if specified, must contain 3 constants for local size.\n"
           " * ARGS         must evaluate to CL Memory objects, given as kernel arguments.\n"
           "The arguments are bound to the kernel.  Subsequent runs of the same kernel with\n"
           "the same arguments may omit one or more of ARG.  Repeating a run of a kernel\n"
           "this way may cause user-after-free errors if the run command used temporary\n"
           "kernel argument objects; that is, they were constructed while evaluating the\n"
           "'run' command.  Any of these ARGS may also be constants.\n"
           "IMPORTANT:\n"
           "The kernel enqueue, if successful, will run in parallel with the Testbench.\n"
           "This should not be an issue because the Testbench uses an in-order queue,\n"
           "so any commands issues after will implicitly wait for the kernel to finish.\n"
           "Use the 'wait' Testbench command to block until execution finishes.\n";
    return;
}

void HelpForInfo(std::ostream& out)
{
    out << "Displays information on the current state of the testbench.\n"
           "Available operands:\n"
           " * platforms - Displays list of available platforms.\n"
           " * devices   - Displays list of available devices.\n"
           " * lib       - Displays driver library information.\n";
    return;
}

void HelpForScript(std::ostream& out)
{
    out << "script FILENAME\n"
           "Runs all commands on the given file line by line.\n"
           "Lines starting with the character '#' are ignored.\n"
           "Commands will be echoed prior to execution unless the\n"
           "line starts with the character '@', or echo has been\n"
           "disabled (see 'set echo').\n";
}

void HelpForBind(std::ostream& out)
{
    out << "bind KERNEL ARGNO OBJECT [OBJECT ...]\n"
           "Binds kernel arguments.\n"
           "The objects are set as sequential kernel arguments,\n"
           "starting at ARGNO.\n";
}
} // namespace

void Testbench::executeHelp(TokenStream& tokens)
{
    if (!tokens) {
        GenericHelp(*mOut);
        return;
    }

    Token next = tokens.consume();
    IStringView command = tokens.getTokenText(next);
    const std::initializer_list<std::string_view> commands{
        "help", "info", "set", "expression",
        "save", "run", "script", "bind"
    };

    switch (command.autocomplete(commands)) {
    case 0: // help
        // ignore
        break;
    case 1: HelpForInfo(*mOut); break;
    case 2: HelpForSet(*mOut); break;
    case 3: HelpForExpression(*mOut, tokens); break;
    case 4: HelpForSave(*mOut); break;
    case 5: HelpForRun(*mOut); break;
    case 6: HelpForScript(*mOut); break;
    case 7: HelpForBind(*mOut); break;
    case IStringView::ambiguous:
        *mOut << "Ambiguous argument for help '" << command << "'\n";
        break;
    case std::string_view::npos:
        *mErr << "Unknown command '" << command << "'.\n";
        break;
    }

    if (tokens.current().mType != Token::End) {
        *mOut << "Trailing tokens on help command ignored.\n";
    }
}
