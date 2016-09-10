# Copyright (C) 2001-2003 by Simon Nieuviarts

# Files to compile
SUBDIRS= ./src/api ./src/core ./src/core/arch ./src/core/ipv4 ./src/core/snmp ./src/include ./src/include/ipv4 ./src/netif ./src/netif/ppp

FILES=$(wildcard *.S *.c) $(foreach subdir, $(SUBDIRS), $(wildcard $(subdir)/*.S $(subdir)/*.c))
DIRS=. $(SUBDIRS)

EXT_DIRS=../ ../../shared

OBJS=$(notdir $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(FILES))))

CC=gcc -m32
AS=gcc -m32
LD=ld -melf_i386 -r

KFLAGS=-Wall -g -gstabs -pipe -std=c99 -nostdinc -fno-stack-protector
#KFLAGS=-Wall -O2 -fomit-frame-pointer -pipe -std=c99 -nostdinc

DEF=-D__KERNEL__
INC=$(patsubst %,-I%,$(DIRS)) $(patsubst %,-I%,$(EXT_DIRS))
CCL=$(CC) -Werror $(KFLAGS) $(DEF) $(INC)
ASL=$(AS) -DASSEMBLER $(DEF) $(INC)
DEPS=$(patsubst %.o,%.d,$(OBJS))

vpath %.c $(DIRS)
vpath %.h $(DIRS)
vpath %.S $(DIRS)

.PHONY: clean all

all: lwip.o

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

lwip.o: $(OBJS)
	$(LD) $(OBJS) -o $@

clean:
	rm -f $(OBJS) $(DEPS) lwip.o
