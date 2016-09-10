# Copyright (C) 2001-2003 by Simon Nieuviarts

# Files to compile
SUBDIRS= ./interruptions ./pci ./audio
FILES=$(wildcard *.S *.c) printf.c sprintf.c doprnt.c panic.c div64.c strtol.c strtoul.c string.c  $(foreach subdir, $(SUBDIRS), $(wildcard $(subdir)/*.S $(subdir)/*.c))
DIRS=. ../shared $(SUBDIRS)

# crt0.o must be the first object linked
OBJS=$(strip crt0.o $(filter-out crt0.o,$(notdir $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(FILES))))))

CC=gcc -m32
AS=gcc -m32
LD=ld -melf_i386

KFLAGS=-Wall -g -gstabs -pipe -std=c99 -nostdinc -fno-stack-protector
#KFLAGS=-Wall -O2 -fomit-frame-pointer -pipe -std=c99 -nostdinc

DEF=-D__KERNEL__
INC=$(patsubst %,-I%,$(DIRS))
CCL=$(CC) -Werror $(KFLAGS) $(DEF) $(INC)
ASL=$(AS) -DASSEMBLER $(DEF) $(INC)
DEPS=$(patsubst %.o,%.d,$(OBJS))

# Where to search for source files
vpath %.c $(DIRS)
vpath %.h $(DIRS)
vpath %.S $(DIRS)

# Targets that are not files
.PHONY: clean all user lwip kbd_linux 

# The default target
all: user kbd_linux lwip kernel.bin 

user:
	$(MAKE) -C ../user

lwip: 
	$(MAKE) -C lwip

kbd_linux:
	$(MAKE) -C kbd_linux

# Automatic rules to build dependency files
%.d: %.c
	@$(SHELL) -ec '$(CCL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

%.d: %.S
	@$(SHELL) -ec '$(ASL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

# Build and include dependency files
-include $(DEPS)

# Automatic rules to build the object files
%.o: %.S
	$(ASL) -c $< -o $@

%.o: %.c
	$(CCL) -c $< -o $@

%.s: %.c
	$(CCL) -S -c $< -o $@

# Wrap the user program in an object file. Note that although the user program
# is wrapped into the kernel, they are not linked together.
userdata.o: empty.o ../user/user.bin
	objcopy empty.o --add-section=.user=../user/user.bin --set-section-flags=.user=contents,alloc,load,data userdata.o

# A text screen for exception dump
task_dump_screen.o: empty.o task_dump_screen.txt
	objcopy empty.o --add-section=.task_dump_screen=task_dump_screen.txt --set-section-flags=.task_dump_screen=contents,alloc,load,data task_dump_screen.o

# Generate the kernel, ready to run
kernel.bin: kernel.lds $(OBJS) userdata.o kbd_linux/kbd_linux.o task_dump_screen.o lwip/lwip.o
	$(LD) -e entry -Tkernel.lds $(OBJS) userdata.o kbd_linux/kbd_linux.o task_dump_screen.o lwip/lwip.o -o $@

clean:
	$(MAKE) -C kbd_linux clean
	$(MAKE) -C lwip clean
	rm -f $(OBJS) $(DEPS) kernel.bin userdata.o task_dump_screen.o
