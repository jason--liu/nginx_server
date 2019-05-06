
#CROSS_COMPILE = arm-linux-
#AS		= $(CROSS_COMPILE)as
#LD		= $(CROSS_COMPILE)ld
#CC		= $(CROSS_COMPILE)gcc
#CPP		= $(CC) -E
#AR		= $(CROSS_COMPILE)ar
#NM		= $(CROSS_COMPILE)nm
#
#STRIP		= $(CROSS_COMPILE)strip
#OBJCOPY		= $(CROSS_COMPILE)objcopy
#OBJDUMP		= $(CROSS_COMPILE)objdump
#
#export AS LD CC CPP AR NM
#export STRIP OBJCOPY OBJDUMP
CC = g++
LD = ld
export CC LD

# CFLAGS := -Wall -std=c++11
COMPILE_FLAGS = -Wall -Wextra
INCLUDE = -I$(shell pwd)/_include
#CFLAGS += -I _include/

LDFLAGS :=

# export CFLAGS LDFLAGS INCLUDE
export LDFLAGS INCLUDE

TOPDIR := $(shell pwd)
#TOPDIR := . 
export TOPDIR

TARGET := nginx


obj-y += app/
obj-y += signal/

all :
	ctags-exuberant -e -R .
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o


release : export CFLAGS := -O2 $(COMPILE_FLAGS)
release : all
debug : export  CFLAGS := -DDEBUG -g $(COMPILE_FLAGS)
debug : all

clean:
	rm TAGS
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
