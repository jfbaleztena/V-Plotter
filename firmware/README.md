# VPlotter-Firmware

### COMPILATION

**In linux**, install avr toolchain refering to [this link](https://www.tonymitchell.ca/posts/use-vscode-with-avr-toolchain/).

Add the following alias to the .bashfile:

```
alias avr-flash='make flash && sudo minicom'
alias avr-check='make && avr-size -C -x VPlotter-Firmware.elf'
alias avr-clean='make clean && rm *.o'
```

Type: 

```
avr-flash
```
For compiling, flashing and opening the port defined in the Makefile

```
avr-check
```
For compiling, and checking the program and data memory usage.  
This is critical during development as the 328p is quite memory limited for the current project. 

```
avr-clean
```
Clean all the compilation dependant files. 
Critical during development as changes in some configurations, ie. adding new states, are not reflected in code until cleaning.