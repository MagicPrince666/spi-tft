#ifndef __SYS_H
#define __SYS_H	

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>	

extern int gpio_mmap_fd;

int gpio_mmap(void);
int mt76x8_gpio_get_pin(int pin);
void mt76x8_gpio_set_pin_direction(int pin, int is_output);
void mt76x8_gpio_set_pin_value(int pin, int value);
	  		 
#endif  
	 
	 



