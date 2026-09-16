# BinParser - A binary file parser
BinParse is a simple binary file parser written in C. It was originally created to help break down binary files like ISO images or compiled ELF binaries while studying their structure.

## Table of contents
- [Building and running](#building-and-running)
- [How it works](#how-it-works)
- [BPS file format explained](#bps-file-format-explained)
    - [Section structure](#section-structure)
    - [Attribute structure](#attribute-structure)
    - [Attribute types](#attribute-types)
- [Features completed](#features-completed)
- [Why BinParser was made](#why-binparser-was-made)

## Building and running
The project is written in plain C and uses CMake as the build system. To build the project, simply run the following in your favorite terminal:

```bash
cmake -B build -S .
cmake --build build
```

To run the parser:
```bash
./build/binparser [binary-file] [BPS file]
```

## How it works
BinParser uses a simple script file called the **Binary Parser Script** (or `.bps`) file. The script file contains two parts
1. Sections - define offsets in the binary file on where to parse
2. Attributes - are part of sections and hold the values you want to read

The parser reads through the BPS file and constructs an internal representation of the sections and attributes. It then reads the binary file specified and fills the attributes with their values.

A simple BPS file is shown below as an example. The script is used for reading the MBR partition table in the boot sector of a disc image and also reads the GPT partition header in the next sector:

```
[mbr]
skip:440:skip
signature:4:hex
reserved:2:hex

[mbr_table]
[partition_1]
drive_attr:1:bin
chs_start:3:hex
type:1:hex
chs_end:3:hex
lba_start:4:num
lba_end:4:num

[partition_header:0x200]
signature:8:str
revision:4:num
header_size:4:num
crc_checksum:4:hex
reserved:4:num
header_lba:8:num
alternate_gpt_lba:8:num
first_block:8:num
last_blocl:8:num
guid:16:hex
partition_entry:8:num
no_partitions:4:num
entry_size:4:num
crc_partitions:4:hex
```

## BPS file format explained
As mentioned above, a BPS file has two parts: **sections** and **attribute**

### Section structure
Sections can be defined in 2 ways:

1. `[section_name]` - define a section that follows immediately after the last section and it's attributes
2. `[section_name:offset]` - define a section at a fixed offset in the file. The offset can be in either decimal or hexadecimal format

Any section that lies outside of the file is automatically skipped and it's attributes are marked as `INVALID`.

### Attribute structure
Attributes have only one format:

`attribute_name:size:type`

- `attribute_name` - the name of the attribute
- `size` - the size of the attribute in bytes, can either be in decimal or hexadecimal
- `type` - the type of the attribute

An attribute must follow a section, and will be part of the last section declared, regardless of whitespaces or newlines added.

If an attribute is invalid (for instance having the wrong syntax) or lies outside of the file, it is marked as invalid.

### Attribute types
At the moment, there are 4 main attribute types supported
- `num` - represent numbers/integers. They **MUST** always have a size of either 1, 2, 4, or 8 bytes
- `str` - represent strings
- `hex` - represent hexadecimal values
- `bin` - represent binary values
- `skip` - used to skip parts of a section

## Features completed
- [x] - Parse different types (numbers, strings, raw bytes)
- [x] - Jump to different sections in a file
- [x] - Handle illegal sections and attributes
- [ ] - Handle command line arguments better
- [ ] - Save the output to a file instead of just stdout
- [ ] - Maybe write some unit tests (if I get the time...)
- [ ] - Allow relative offsets based on attribute values
- [ ] - Maybe use C++ instead of C to simplify the code with Vectors...

## Why BinParser was made
One of my hobby projects is an operating system/bootloader from scratch ([SteinerOS](https://github.com/BrickSigma/SteinerOS)) and a large part of learning how to develop it is understanding the structure of raw binary files and images, such as the ELF headers in an executable or the file table in an ISO 9660 image. 

Before I was mostly loading the images in a hex editor and looking at the raw hex values, which is tedious and extremely time consuming. Obviously other tools exist for parsing these files, such as Xorriso for reading the details of the ISO image, or objdump for parsing binary executables, however they didn't give me a true idea of where in the binary files certain parts are, which is necessary when building a driver or parser for these files.

That's why I created BinParse, both to make it easier to parse large binaries as well as for the fun of making a parser from scratch. It is by no means an efficient implementation, and their are probably a lot of bugs or possible syntax errors that could happen with the BPS files, but it works for now for what I need and I'll probably fix them later.