# c-

This project is a compiler for the C- language targeting the Tiny VM. This program utilizes flex and bison for scanning and parsing.

## Building

These instructions for Linux assume you already have GCC/G++, Flex, Bison, and Make.

```bash
git clone https://github.com/zachoooo/c-
cd c-
make
```

You should then be left with a program called `c-`. To use it, just type `./c- [sourceFile]` where `[sourceFile]` is the name of your C- program file. Use `./c- -h` to display the help page to find additional usage information. There is no install script so you will need to copy this to `/usr/local/bin` if you want it installed.

## Acknowledgements

The C- language was defined by Dr. Robert Heckendorn at the University of Idaho and can be found here: http://marvin.cs.uidaho.edu/Teaching/CS445/c-Grammar.pdf

This language definition may differ somewhat from what my compiler recognizes as the class may have changed and this document may be updated.

The TinyVM can be found in the test folder. The TinyVM was originally written by Kenneth C. Louden for the textbook "Compiler Construction: Principles and Practice" and was then heavily modified by Dr. Heckendorn for this class.