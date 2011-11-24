
readelf -aW kernel.bin | grep Machine

objcopy -I elf32-i386 -O elf32-i386 -R .source -R .llvmir -R .amdil --alt-machine-code=<Machine> kernel.bin stripped_kernel.bin

#objcopy -I elf64-x86-64 -O elf64-x86-64 -R .source -R .llvmir -R .amdil --alt-machine-code=<Machine> kernel.bin stripped_kernel.bin


#<Machine> is the "machine" code obtain in the previous step. This code should be the full hexadecimal value reported by readelf, including the preceding 0x value.

#It is expected that objcopy will report that it does not supportthe particular alternative machine code you provide. It will treat the number as an absolute e_machine value instead. This is the desired effect and should not be interpreted as an error.
#The stripped_kernel.bin binary kernel fileis ready for distribution. Repeat these steps on all generated binary kernel files. 


