# synacor-challenge
VM and Assembler for the Synacor Challenge: https://challenge.synacor.com/

This has been tested only under Linux. Check out the repository and then
type 'make'. Two binaries will be produced:

* vm - The Synacor virtual machine
* synasm - The Synacor assembler

The VM can accept input by redirecting from a file. An example is the
"play.txt" file which is included - this plays the Synacor challenge as
far as I got (which is everything except the last code).

The VM takes a command line parameter which is the name of the .bin
file to execute. If this is omitted, then "challenge.bin" is assumed.

The assembler was done very quickly so the code could be improved!
It takes a syntax that's close to ARM assembler as that's what I'm
used to. It's a simple dual-pass assembler that does one pass to
resolve symbols and then another pass to produce the actual code.

