# BinParser - A binary file parser
BinParse is a simple binary file parser written in C++. It was originally created to help break down binary files like ISO images or compiled ELF binaries while studying their structure.

## Table of contents
- [Building and running](#building-and-running)
- [How it works](#how-it-works)
- [Example script](#example-script)
- [BPS file format explained](#bps-file-format-explained)
    - [Section structure](#section-structure)
    - [Attribute structure](#attribute-structure)
    - [Attribute types](#attribute-types)
- [Features completed](#features-completed)
- [Why BinParser was made](#why-binparser-was-made)

## Building and running
The project is written in C++ and uses CMake as the build system. To build the project, simply run the following in your favorite terminal:

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

The parser reads through the BPS file and constructs an internal representation of the sections and attributes. It then reads the binary file specified and fills the attributes with their values. The format of how sections and attributes are declared is described further [below](#bps-file-format-explained).


## Example script
A simple BPS file is shown below as an example. The script is used for reading the following sections in an El-Torito formated ISO image:
- MBR partition table in the boot sector
- GPT header in the next partition (LBA 1)
- Alternative GPT header pointed by the first one

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

[gpt_header:0x200]
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

[alt_gpt_header:gpt_header.alternate_gpt_lba * 512]
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

When run using `binparser image.iso file.bps`, the following output is printed to stdout:

```
mbr: 0x00000000
    0000: 440 : skip = skipped
    01b8: 4 : signature = e3 a1 55 02 
    01bc: 2 : reserved = 00 00 

mbr_table: 0x000001be

partition_1: 0x000001be
    0000: 1 : drive_attr = 10000000
    0001: 3 : chs_start = 00 01 00 
    0004: 1 : type = 00 
    0005: 3 : chs_end = 3f 20 01 
    0008: 4 : lba_start = 0
    000c: 4 : lba_end = 4096

gpt_header: 0x00000200
    0000: 8 : signature = EFI PART
    0008: 4 : revision = 65536
    000c: 4 : header_size = 92
    0010: 4 : crc_checksum = ad 06 18 5f 
    0014: 4 : reserved = 0
    0018: 8 : header_lba = 1
    0020: 8 : alternate_gpt_lba = 4095
    0028: 8 : first_block = 64
    0030: 8 : last_blocl = 4032
    0038: 16 : guid = ea 9d 9c 95 41 b5 1d 4f 8a 89 ff e8 0a c5 14 b0 
    0048: 8 : partition_entry = 2
    0050: 4 : no_partitions = 248
    0054: 4 : entry_size = 128
    0058: 4 : crc_partitions = 49 9a f7 66 

alt_gpt_header: 0x001ffe00
    0000: 8 : signature = EFI PART
    0008: 4 : revision = 65536
    000c: 4 : header_size = 92
    0010: 4 : crc_checksum = 2d c1 5e 51 
    0014: 4 : reserved = 0
    0018: 8 : header_lba = 4095
    0020: 8 : alternate_gpt_lba = 1
    0028: 8 : first_block = 64
    0030: 8 : last_blocl = 4032
    0038: 16 : guid = ea 9d 9c 95 41 b5 1d 4f 8a 89 ff e8 0a c5 14 b0 
    0048: 8 : partition_entry = 4033
    0050: 4 : no_partitions = 248
    0054: 4 : entry_size = 128
    0058: 4 : crc_partitions = 49 9a f7 66
```

The sections are printed in the format:
```
section_name: offset
```
Where the offset is in hexadecimal notation. The attributes are printed like so as well:
```
offset: size : attribute_name = value
```
- `offset` - offset of the attribute relative to the section, printed in hexadecimal
- `size` - size of the attribute in bytes
- `attribute_name` - the name of the attribute
- `value` - the value stored in the attribute

## BPS file format explained
As mentioned above, a BPS file has two parts: **sections** and **attribute**

### Section structure
Sections can be defined in 4 ways:

1. `[section_name]` - define a section that follows immediately after the last section and it's attributes
2. `[section_name:offset]` - define a section at a fixed offset in the file. The offset can be in either decimal or hexadecimal format
3. `[section_name:section.attribute]` - defines a section at an offset pointed by another attribute in another section
4. `[section_name:section.attribute * scale]` - similar to `3`, but the offset can be scaled by a `scale` factor.

Any section that lies outside of the file is automatically skipped and it's attributes are marked as `invalid`.

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
- [x] - Allow relative offsets based on attribute values
- [ ] - Handle command line arguments better
- [ ] - Save the output to a file instead of just stdout
- [ ] - Maybe write some unit tests (if I get the time...)

## Why BinParser was made
One of my hobby projects is an operating system/bootloader from scratch ([SteinerOS](https://github.com/BrickSigma/SteinerOS)) and a large part of learning how to develop it is understanding the structure of raw binary files and images, such as the ELF headers in an executable or the file table in an ISO 9660 image. 

Before I was mostly loading the images in a hex editor and looking at the raw hex values, which is tedious and extremely time consuming. Obviously other tools exist for parsing these files, such as Xorriso for reading the details of the ISO image, or objdump for parsing binary executables, however they didn't give me a true idea of where in the binary files certain parts are, which is necessary when building a driver or parser for these files.

That's why I created BinParse, both to make it easier to parse large binaries as well as for the fun of making a parser from scratch. It is by no means an efficient implementation, and their are probably a lot of bugs or possible syntax errors that could happen with the BPS files, but it works for now for what I need and I'll probably fix them later.