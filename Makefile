CROSS_COMPILE = mipsel-openwrt-linux-uclibc-
CPP = $(CROSS_COMPILE)g++

TARGET	= SpiTft

DIR		= . ./font ./LCD ./sys ./touch ./pwm ./spi ./iic ./key ./picture ./mpu6050 ./text ./GUI ./music
INC		= -I./font -I./LCD -I./sys -I./touch -I./pwm -I./spi -I./iic -I./key -I./picture -I./mpu6050 -I./text -I./GUI -I./music
CFLAGS	= -O3 -g -Wall
LDFLAGS += -L./lib -lpthread -lm -lrt -ldl -liconv

OBJPATH	= ./obj
INSTALLPATH	= ./bin

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:$(OBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CPP) -o $@ $(OBJPATH)/*.o $(LDFLAGS) 

install:
	cp -r $(TARGET) gbk* test.* lib/libiconv.so $(INSTALLPATH) 
	mv  $(INSTALLPATH)/libiconv.so $(INSTALLPATH)/libiconv.so.2
clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)