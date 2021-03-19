# Contribution guidelines

Thank you for expressing interest in contributing to the development and/or
maintenance of the Crust firmware. This document contains guidelines and
procedures for contributing to the project.

## Code of conduct

By contributing to Crust firmware, you are entering into an agreement to uphold
the project's [Code of Conduct][conduct]. Please review that document.

## Issue tracking

Bugs and features are tracked through the GitHub [issues page][issues]. When
filing issues, please follow the provided issue template.

### Labels

Labels are used to indicate the lifecycle of all Crust firmware issues. Labels
are prefixed with letters corresponding to the chart below.

| Prefix           | Description
|------------------|-----------------------------------------------------------
| Area (A)         | Indicates the area of the project impacted.
| Difficulty (D)   | Indicates the expected difficulty of an issue.
| Resolution (R)   | Indicates the resolution of an issue.
| Status (S)       | Indicates the status of an issue.
| Type (T)         | Indicates the type of an issue.

Additionally, the labels "good first issue" and "help wanted" may be applied to
an issue or pull request. An issue labeled with "good first issue" is an issue
that is recommended for new contributors. An issue labeled with "help wanted"
is an issue that project maintainers request community help resolving.

## Submitting a patch

The Crust firmware project accepts patches through the use of GitHub pull
requests. It is recommended that pull requests correspond to an issue on the
Crust firmware [issues page][issues]. Pull requests submitted _without_ a
corresponding issue will require more review to understand the intent behind
the code change.

### Commits

All contributions to this project must be released under the open-source
license used by the project. To help ensure this, all commits you make must be
accompanied by your real name and a `Signed-off-by` tag asserting authorship
and permission to contribute, per the [Developer Certificate of Origin][dco].
This tag can be added automatically by git using `git commit -s`.

Each change submitted in a pull request that addresses a different scope of the
project should be contained within its own commit. For example, a pull request
adding support for a new hardware component could include the following four
commits:

1. The first commit should add the headers and source files for the generic
   driver class, if one is being added. (e.g. `drivers/<class>/<class>.c` and
   `include/drivers/<class>.h`).
2. The second commit should add the driver and its header (e.g.
   `drivers/<class>/<driver>.c` and `include/drivers/<class>/<driver>.h`).
3. The third commit may use the device from common code (e.g. in
   `common/system.c` or `common/scpi_cmds.c`).
4. The fourth commit would enable the device in Kconfig on the appropriate
   boards and platforms.

Changes addressing feedback from project maintainers should be squashed into
existing commits rather than being included at the end of a pull request as a
new commit.

### Continuous integration testing

All pull requests are built and tested with Travis CI upon submission. To
ensure that your changes will pass the CI build, the following process should
be used:

- Verify the source code style by running `make check-format`. (This requires
  installing [Uncrustify][uncrustify]. See below.)
- Build the firmware using several `defconfig`s for various platforms.

## Architecture

This firmware is designed to be flexible yet extremely lightweight. It borrows
heavily from ideas in both Linux and ATF for its layout and driver model. The
code is divided into directories based on major function:

- `configs`: These files contain configuration for each board supported by this
  firmware. They determine which subdirectory of `platform` is used and which
  devices are enabled.
- `common`: Files in this directory contain the main logic of the firmware,
  which involves power state sequencing and handling SCPI commands.
- `drivers`: This directory contains a subdirectory for each class of drivers.
  - `drivers/<class>`: These directories contain all of the drivers of a class,
    as well as generic dispatch code (in `drivers/<class>/<class>.c`).
- `include`: This directory contains headers for code in the rest of the
  firmware, as well as the definitions for a freestanding C implementation.
- `lib`: This directory contains standalone utility code.
- `platform`: This directory contains a subdirectory for each supported
  platform, which refers to a family of SoCs with a similar programming
  interface.
  - `platform/<platform>/include`: This directory contains headers with
    platform-specific macro definitions, such as memory addresses of devices
    and register layouts that change between platforms.
- `scripts`: These files are used to assist in building and linking the
  firmware, but contain no code themselves.
- `tools`: Each file here is a self-contained program intended to be run on the
  host machine (in Linux userspace on the ARM cores) to gather information,
  manipulate hardware, load firmware, or for other reasons.

## Formatting

Crust firmware uses Uncrustify to format all source code files. Running `make
format` will format all project files according to a predefined style guide.

The following are _general_ formatting guidelines:
- Lines should be no longer than 79 characters plus the trailing newline
  character.
- Use tabs for indentation and spaces for alignment for all C source code and
  header files.
- Trim all trailing whitespace.
- Where multiple words must be combined into one, use hyphens in strings,
  filenames, and documentation; however, use underscores in function, variable
  names and header guards.

### Writing headers

- Header files act as APIs and are reserved for functions that will be
  exposed to the rest of the project. Macros, `struct` definitions, and
  function declarations that are only utilized by a driver or module belong in
  the corresponding implementation file, rather than the header file.
- Public macros, function prototypes, and declarations should be documented
  according to [Doxygen JavaDoc-style comment blocks][doxygen].
- Functions defined in assembly should also be declared in a header, unless
  they are not intended to be called directly (e.g. `__udivsi3`).
- Ideally, the header should be named the same as the driver or source file,
  except that it should be placed inside the include hierarchy.
- The header guard should be the same as the path you would need to put in your
  `#include` directive, but with the following changes:
  - All letters should be uppercase.
  - All hyphens, slashes, and periods should be replaced by underscores.

### Assembly

Assembly files are preprocessed, so they should use the file extension `.S`
(and this is the only extension the build system looks for). They should not
have the same path as a C file (excluding the extension), as the build system
cannot disambiguate the two.

Assembly should be formatted as follows:
- Local control-flow labels should be numbered, not named.
- Local data labels should be named descriptively.
- Global data and functions should use the `data` and `func`/`endfunc` macros
  from `macros.S`.
- Indent the instruction by one tab.
- Indent the first operand with one tab.
- Separate following operands with a single space.
- Use `#` to start comments.
- Align comments at 40 columns with tabs.

[conduct]: docs/code_of_conduct.md
[dco]: https://developercertificate.org
[doxygen]: https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html
[issues]: https://github.com/crust-firmware/crust/issues
[uncrustify]: https://github.com/uncrustify/uncrustify
