CROSS_COMPILE = aarch64-imx8mm-linux-gnu-
CPP = $(CROSS_COMPILE)g++
CC  = $(CROSS_COMPILE)gcc

TARGET	= simple-spitft

DIR		= . ./font ./LCD ./sys ./spi ./key ./iic #./touch
INC		= -I./font -I./LCD -I./sys -I./spi -I./iic -I./iic -I./key #-I./touch
CFLAGS	= -O3 -g -Wall# -std=gnu++11 
LDFLAGS += -lpthread -lm -lrt -ldl

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
	cp -r $(TARGET) $(INSTALLPATH) 

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)