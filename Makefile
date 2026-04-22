# (C) 2026, Cornell University
# All rights reserved.

QEMU        = qemu-system-riscv32
RISCV_CC    = riscv-none-elf-gcc
OBJDUMP     = riscv-none-elf-objdump
OBJCOPY     = riscv-none-elf-objcopy

LDFLAGS     = -nostdlib -lc -lgcc
CFLAGS      = -march=rv32ima_zicsr -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option -fno-builtin
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

all:
	@printf "$(YELLOW)-------- Compile Hello, World! --------$(END)\n"
	$(RISCV_CC) $(CFLAGS) -c hello.s -o hello_s.o
	$(RISCV_CC) $(CFLAGS) -c hello.c -o hello.o
	$(RISCV_CC) $(CFLAGS) hello.s hello.c -Thello.lds $(LDFLAGS) -o hello.elf
	$(OBJDUMP) $(DEBUG_FLAGS) hello.elf > hello.lst
	$(OBJDUMP) $(DEBUG_FLAGS) hello.o > hello_o.lst
	$(OBJDUMP) $(DEBUG_FLAGS) hello_s.o > hello_s.lst
	$(OBJCOPY) -O binary hello.elf hello.bin

qemu: all
	@printf "$(YELLOW)-------- Run Hello-World on QEMU --------$(END)\n"
	$(QEMU) -nographic -machine virt -smp 1 -m 4M -bios hello.bin

clean:
	@rm -f hello.bin hello.lst hello.elf hello_s.o hello.o hello_o.lst hello_s.lst

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
