CC   = arm-none-eabi-gcc
MACH = cortex-m4
OPT  = -O2

CFLAGS  = -c -mcpu=$(MACH) -mthumb -std=gnu11 $(OPT) -g -Wall -Wextra
LDFLAGS = -T linker.ld -mcpu=$(MACH) -mthumb -specs=rdimon.specs -lc -lrdimon

OBJS = main.o startup.o

all: firmware.elf

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

firmware.elf: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.elf

gdbserver:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg