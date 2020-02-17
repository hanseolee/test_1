#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <linux/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <type.h>
#include <rs485.h>
#include	<i2c-dev.h>
#include	<stm_touch.h>
#include	<atmel_touch.h>
#include <touch.h>

char ch1_TDev[30] = "/dev/i2c-13";
char ch2_TDev[30] = "/dev/i2c-9";

int touch_func(MODEL id,int model_index, void *t_limit)
{
    int ret = PASS;

	FUNC_BEGIN();
	switch(id)
	{
		case	JOAN :
		case	JOAN_REL :
		case	JOAN_MANUAL :
		case	JOAN_E5 :
			printf("TOUCH STMicro... \n");
	        module_initialize(id, model_index,"JOAN",OFF,0);
	        ret = stm_control(id, (struct stm_touch_limit *)t_limit);
			break;
		case	MV :
		case	MV_MANUAL :
		case	MV_MQA :
		case	MV_DQA :
			printf("TOUCH STMicro... \n");
	        module_initialize(id,model_index,"MV",OFF,0);
	        ret = stm_control(id, (struct stm_touch_limit *)t_limit);
			break;

		case	A1 :
			printf("TOUCH STMicro... \n");
	        module_initialize(id, model_index,"A1",OFF,0);
	        ret = stm_control(id, (struct stm_touch_limit *)t_limit);
			break;

		case	DP049 :
			printf("TOUCH STMicro... \n");
			module_initialize(id, model_index,"DP049",OFF,0);
			ret = stm_control(id, (struct stm_touch_limit *)t_limit);
			break;

        case    AKATSUKI :
			printf("TOUCH Atmel... \n");
            module_initialize(id, model_index,"AKATSUKI",OFF,0);
            ret = atmel_control(id, (struct atmel_touch_limit *)t_limit);
            break;

		case	B1 :
			printf("TOUCH STMicro... \n");
			module_initialize(id, model_index,"B1",OFF,0);
			ret = stm_control(id, (struct stm_touch_limit *)t_limit);
			break;

        case    STORM :
            printf("TOUCH Synaptics... \n");
            module_initialize(id, model_index,"STORM",OFF,0);
            //ret = synaptics_control(id, (struct synaptics_touch_limit *)t_limit);
            break;

		default	:
	        printf("%s : wrong MODEL ID[%d] \n",__func__,id);
	        ret = FAIL;
			break;
	}

	FUNC_END();
	return ret;
}
