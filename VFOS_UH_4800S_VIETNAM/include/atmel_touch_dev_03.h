#include <stdbool.h>
#include <stdio.h>  
#include <sys/types.h>  
#include <linux/types.h> 
#if 0
#include <sysfs.h> 
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <type.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <spi.h>
#include <i2c-dev.h>
#endif



void init_tch_power_set_for_dp116(int);               
bool init_i2c_set_slvAddr_depending_channel2_for_dp116(int, int, unsigned int *);                   
bool release_i2c_set_for_dp116(); 
