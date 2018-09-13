# PL0-Compiler


Program contains all source code for the scanner, parser, codGen, and virtual machine. 

The program must have a text file with the PL\0 source code at the same directory as this file, the source file must be named “sourceCode.txt”. 


To compile: 
```
gcc compiler.c
```


To run:
```
./a.out 
```

To view the compile process:

* “-l” flag will display the lexical scanner output and symbol rep
* “-a” flag will display the machine code generated from lexemes
* “-v” flag will display the stack trace of the VM

Note: These flags may be in any order

The program will generate and read necessary files at each step.
The only file it needs is the source code file.

Note: The VM output value will always be in register 0, thus any value left in register[0] will be outputted. 

The program will also generate a traceFile.txt that displays all the work at any step regardless of the (-l, -a, -v) flags.

