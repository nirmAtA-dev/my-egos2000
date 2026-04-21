# Learning OS internals from ***EGOS-2000***

## ***BASIC-SETUP***
- If you are in linux and shell is bash, then write a line at the end where you add below command
> export EGOS=/home/yourusername/path/to/egos
- This will help you to navigate egos folder using
> cd $EGOS
- clone my repository, it has riscv toolchain as well hence just need to add it in path in bashrc
> export PATH=$PATH:$EGOS/xpack-riscv-none-elf-gcc-14.2.0-3/bin
- go to the folder and run make
> cd $EGOS/egos-2000 && make
- we need qemu's riscv emulator, already added in folder, to run this os
> export PATH=$PATH:$EGOS/xpack-qemu-riscv-8.2.2-1/bin
- now to to egos path and run 
> cd $EGOS/egos-2000 && make qemu
- close qemu using
> Ctrl + A and then x
