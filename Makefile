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

CFLAGS := -Wall -O2 -g #-finput-charset=GB2312 -fexec-charset=GB2312
CFLAGS += -I./font -I./LCD -I./sys -I./touch -I./pwm -I./spi -I./iic -I./key -I./picture -I./mpu6050 -I./text -I./GUI -I./music

LDFLAGS := -lpthread -lm -lrt -ldl -L./text/lib -liconv 

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := show_file


obj-y += main.o
obj-y += LCD/
obj-y += sys/
obj-y += touch/
obj-y += pwm/
obj-y += spi/
obj-y += iic/
obj-y += key/
obj-y += picture/
obj-y += mpu6050/
obj-y += text/
obj-y += GUI/
obj-y += music/


all : 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CPP) $(LDFLAGS) -o $(TARGET) built-in.o 

clean:
	rm -f $(shell find . -name "*.o")
	rm -f $(shell find . -name "*.d")
	rm -f $(TARGET)
	