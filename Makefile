CROSS_COMPILE = mipsel-openwrt-linux-uclibc-
CPP = $(CROSS_COMPILE)g++
CC  = $(CROSS_COMPILE)gcc

TARGET	= SpiTft

DIR		= . ./font ./LCD ./sys ./touch ./pwm ./spi ./iic ./key ./picture ./mpu6050 ./mpu6050/eMPL ./text ./GUI ./music ./video
INC		= -I./font -I./LCD -I./sys -I./touch -I./pwm -I./spi -I./iic -I./key -I./picture \
-I./mpu6050 -I./mpu6050/eMPL -I./text -I./GUI -I./music -I./video
CFLAGS	= -O3 -g -Wall
LDFLAGS += -L./lib -lpthread -lm -lrt -ldl -liconv

OBJPATH	= ./obj
INSTALLPATH	= ./bin

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))
CFILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))
COBJS	= $(patsubst %.c,%.o,$(CFILES))

all:$(OBJS) $(COBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CPP) -o $@ $(OBJPATH)/*.o $(LDFLAGS) 

install:
	cp -r $(TARGET) gbk* test.* lib/libiconv.so $(INSTALLPATH) 
	mv  $(INSTALLPATH)/libiconv.so $(INSTALLPATH)/libiconv.so.2

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)