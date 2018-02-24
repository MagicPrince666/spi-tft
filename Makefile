
CROSS_COMPILE = mipsel-openwrt-linux-uclibc-
CPP = $(CROSS_COMPILE)g++

#目标文件
TARGET	= 3.2tft

#所有的目录
DIR		= . ./font ./LCD ./sys ./touch ./pwm ./spi ./iic ./key ./picture ./mpu6050 ./text ./GUI ./music
INC		= -I./font -I./LCD -I./sys -I./touch -I./pwm -I./spi -I./iic -I./key -I./picture -I./mpu6050 -I./text -I./GUI -I./music
CFLAGS	= -O2 -g -Wall
LDFLAGS += -L./lib -lpthread -lm -lrt -ldl -liconv

OBJPATH	= ./obj

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:$(OBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CPP) -o $@ $(OBJPATH)/*.o $(LDFLAGS) 

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)
	