CROSS_COMPILE = mipsel-openwrt-linux-uclibc-
#export PATH=$PATH:/Volumes/OpenWrt/openwrt_widora/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin
#export STAGING_DIR=/Volumes/OpenWrt/openwrt_widora/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$STAGING_DIR
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -O2 -g
CFLAGS += -I./font -I./LCD -I./sys -I./touch -I./pwm -I./spi -I./key -I./picture

LDFLAGS := -lpthread -lm -lrt -ldl

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := show_file


obj-y += main.o
obj-y += LCD/
obj-y += font/
obj-y += sys/
obj-y += touch/
obj-y += pwm/
obj-y += spi/
obj-y += key/
obj-y += picture/


all : 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CPP) $(LDFLAGS) -o $(TARGET) built-in.o 


clean:
	rm -f $(shell find . -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find . -name "*.o")
	rm -f $(shell find . -name "*.d")
	rm -f $(TARGET)
	