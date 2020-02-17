/*
 * model_dp086.c
 * This is for DP086 model.
 */
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
#include <i2c-dev.h>

#include <type.h>
#include <rs485.h>
#include <display.h>
#include <current.h>
#include <mipi_con.h>
#include <mipi_dsi.h>
#include <atmel_touch_dev_02.h>
#include <model_common.h>
#include <model_dp086.h>

//#include <synaptics_touch.h>
//#include <siw_touch.h>

/* main structure for model DP086 */
model_dp086_t	*model_dp086_p;
extern int is_display_on_for_next_prev;
extern int flag_interlock;
extern int flag_judge;
extern int flag_password;

extern int password[PW_LEN];
extern int pw_value[PW_LEN];
extern int pw_idx;

/* local function define */
void display_on_by_command_for_dp086(void);
void display_off_by_command_for_dp086(void);

/*
 * Name : init_variable_for_dp086
 * Description : Initialize variables to make initial condition.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value :
 */
void init_variable_for_dp086(model_dp086_t *dp086_p)
{
	int ch_cnt = 0;

	FUNC_BEGIN();

	for (ch_cnt = 0;ch_cnt < DP086_POCB_WRITE_CHANNEL_NUM;ch_cnt++)
	{
		dp086_p->pocb_status.pocb_init_status[ch_cnt] = POCB_STATUS_NO_READ;
		dp086_p->pocb_status.pocb_cur_status[ch_cnt] = POCB_STATUS_NO_READ;
	}
	dp086_p->pocb_status.flag_pocb_changed = POCB_STATUS_NO_CHANGE;
	dp086_p->cur_image_num = 0;

	FUNC_END();
}

/*
 * Name : put_info_for_dp086_thread
 * Description : Set information which are needed to set DP086 thread, it is called by main() function.
 * Parameters :
 * 		model_dp086_info_t *info_p
 * Return value :
 */
void put_info_for_dp086_thread(model_dp086_info_t *info_p)
{
	model_dp086_info_t *dp086_info_p = &model_dp086_p->model_dp086_info;

	FUNC_BEGIN();

	dp086_info_p->key_dev = info_p->key_dev;
	dp086_info_p->model_dp086_id = info_p->model_dp086_id;
	dp086_info_p->next_model_id = info_p->next_model_id;
	dp086_info_p->buf_index = info_p->buf_index;
//	dp086_info_p->image_directory = info_p->image_directory;
	memcpy(dp086_info_p->display_image_file_name,info_p->display_image_file_name,MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
	memcpy(&dp086_info_p->dp086_manager, &info_p->dp086_manager,sizeof(MODEL_MANAGER));
	memcpy(&dp086_info_p->version_info,&info_p->version_info,sizeof(vfos_version_info_t));

	/* Convert image directory character to string */
	strcpy(dp086_info_p->display_image_dir,&dp086_info_p->dp086_manager.dir);

	FUNC_END();
}

/*
 * Name : get_info_for_dp086_thread
 * Description : Get information which are needed to set DP086 thread.
 * Parameters :
 * Return value :
 */
int get_info_for_dp086_thread(void)
{
	FUNC_BEGIN();

	FUNC_END();

	return 0;
}

/*
 * Name : parsing_pattern_command_and_write_for_dp086
 * Description : Parsing pattern command from config file and write the commands through mipi.
 * Parameters : 
 * 		char *parsing_file_name_p : name of pattern code file
 *		char *parsing_code_name_p : name of parsing code
 * Return value : error value
 */
int parsing_pattern_command_and_write_for_dp086(char *parsing_file_name_p, char *parsing_code_name_p)
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[MAX_CODE_NAME_STRING_LENGTH];
	mipi_write_data_t *mipi_data = &model_dp086_p->g_mipi_write;

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);
	DPRINTF("code_file_name=(%s)\n", code_file_name);

	memset(code_name,0,sizeof(code_name));
	strcpy(code_name,parsing_code_name_p);
	DPRINTF("(%s) mode\n", code_name);

	/* parsing pattern command to get mipi data to be written */
	memset(mipi_data,0,sizeof(mipi_write_data_t));
	ret = parsing_pattern_write_command_for_dp086(code_file_name, code_name, mipi_data);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_write_command_for_dp086\n");
		return ret;
	}
	else
	{
		/* write mipi data if no error */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}
	}
	FUNC_END();

	return ret;
}

/*
 * Name : parsing_pattern_write_command_for_dp086
 * Description : Parsing mipi commands(register set) to write for special pattern code.
 * Parameters :
 * 		char *parsing_file_name_p : name of pattern code file
 * 		char *parsing_code_name_p : name of pattern
 * 		mipi_write_data_t *mipi_data_p : get mipi data to be written
 * Return value :
 * 		error value
 */

// Replaceable parsing_pattern_command_and_write_for_dp086();
int parsing_pattern_write_command_for_dp086(char *parsing_file_name_p, char *parsing_code_name_p, mipi_write_data_t *mipi_data_p)
{
	int ret = -1;
    FILE *fp;
    char *token = NULL;
    char string[MAX_PARSING_STRING_LENGTH];
	unsigned long data_value = 0;
	int found = 0;
	char parsing_file_name[MAX_PARSING_NAME_LENGTH];
	char parsing_code_name[MAX_PARSING_NAME_LENGTH];
	mipi_write_data_t mipi_data;

	FUNC_BEGIN();

	memset(&mipi_data,0,sizeof(mipi_write_data_t));
	memset(mipi_data_p,0,sizeof(mipi_write_data_t));
	memset(&parsing_file_name[0],0,sizeof(parsing_file_name));
	memset(&parsing_code_name[0],0,sizeof(parsing_code_name));

	strcpy(&parsing_file_name[0],parsing_file_name_p);
	
	DPRINTF("Parsing File_name:(%s)\n",&parsing_file_name[0]);
	
	if((fp=(fopen(&parsing_file_name[0],"r"))) == 0 ){
		DERRPRINTF("cannot open %s\n", &parsing_file_name[0]);
		return -1;
	} 
	
	strcpy(&parsing_code_name[0],parsing_code_name_p);
	
	/* parsing */
	while((fgets(string, 500, fp)) != NULL){
//		DPRINTF("Get string\n");
        token = strtok(string, TOKEN_SEP);
        while(token != NULL){
			if (token[0] == COMMENT_START_CHARACTER)
			{
//				DPRINTF("=============Found #=================\n");
				break;
			}

			if (found == false)
			{
				if (!strcmp(token, parsing_code_name))
				{
					ret = 0;
					found = true;
					DPRINTF("Found %s\n", parsing_code_name);
				}
			}
			else if (found == true)
			{
				if(!strcmp(token,STRING_MIPI_WRITE_COMMAND))
				{
//					DPRINTF("Found %s\n", STRING_MIPI_WRITE_COMMAND);
					token = strtok(NULL, TOKEN_SEP);
					while(token != NULL){
						if (token[0] == COMMENT_START_CHARACTER)
						{
//							DPRINTF("=============Found #=================\n");
							break;
						}
						data_value = (unsigned long)strtoul(token,NULL,16);
//						DPRINTF("get data(0x%02x)\n", data_value);
						mipi_data.data_buf[mipi_data.reg_cnt][mipi_data.data_cnt[mipi_data.reg_cnt]] = (unsigned char)data_value;
						mipi_data.data_cnt[mipi_data.reg_cnt]++;
						token = strtok(NULL, TOKEN_SEP);
					}
					mipi_data.reg_cnt++;
				}
				else if((strcmp(token,STRING_DELAY_COMMAND_CAP_SMALL) == 0) || (strcmp(token,STRING_DELAY_COMMAND_SMALL) == 0))
				{
//					DPRINTF("Found %s\n", token);
					mipi_data.data_buf[mipi_data.reg_cnt][mipi_data.data_cnt[mipi_data.reg_cnt]] = DEFINE_OF_DELAY_COMMAND;
					mipi_data.data_cnt[mipi_data.reg_cnt]++;
					token = strtok(NULL, TOKEN_SEP);
					data_value = (unsigned long)strtoul(token,NULL,10);
//					DPRINTF("get data(0x%02x)\n", data_value);
					mipi_data.data_buf[mipi_data.reg_cnt][mipi_data.data_cnt[mipi_data.reg_cnt]] = (unsigned char)data_value;
					mipi_data.data_cnt[mipi_data.reg_cnt]++;
					mipi_data.reg_cnt++;
				}
				else if(strcmp(token,STRING_PPS_SET_COMMAND) == 0)
				{
//					DPRINTF("Found %s\n", token);
					mipi_data.data_buf[mipi_data.reg_cnt][mipi_data.data_cnt[mipi_data.reg_cnt]] = DEFINE_OF_PPS_SET_COMMAND;
					mipi_data.data_cnt[mipi_data.reg_cnt]++;
					mipi_data.reg_cnt++;
				}
				else if(!strcmp(token,STRING_END_COMMAND))
				{
					DPRINTF("%s code end\n", parsing_code_name);
   					found = false;
					goto parsing_end;
				}
			}
			token = strtok(NULL, TOKEN_SEP);
		}
    }
parsing_end:
	memcpy(mipi_data_p,&mipi_data,sizeof(mipi_write_data_t));
	fclose(fp);
	FUNC_END();
	return ret;
}


/*
 * Name : gray_scan_thread_for_dp086
 * Description : Thread for gray scan.
 * Parameters :
 * 		void *arg : arguments for gray scan thread.
 * Return value : NULL
 */
void *gray_scan_thread_for_dp086(void *arg)
{
	int ret = 0;
	model_dp086_t *dp086_p = (model_dp086_t *)arg;
	pthread_mutex_t	*mutex_p = &dp086_p->gray_scan_thread_mutex;
	int thread_loop = 0;
	char code_name[20];
	int gray_value = 0;
	int positive_dir = true;
	mipi_write_data_t *mipi_data = &model_dp086_p->g_mipi_write;
	char code_file_name[MAX_FILE_NAME_LENGTH];

	FUNC_BEGIN();

	DPRINTF("######### Start gray_scan thread ###########\n");

	/* lock */
	pthread_mutex_lock(mutex_p);

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);
	DPRINTF("code_file_name=(%s)\n", code_file_name);
#if 1
	/* set BIST_IN */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",BIST_IN_CODE_NAME);

	/* parsing pattern command to get mipi data to be written */
	memset(mipi_data,0,sizeof(mipi_write_data_t));
	ret = parsing_pattern_write_command_for_dp086(code_file_name,code_name,mipi_data);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_write_command_for_dp086\n");
		system("/Data/reg_init /mnt/sd/initial/dp086_bist_on.tty");
		usleep(500);
	}
	else
	{
		/* write mipi data if no error */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command, system(dp086_bist_on.tty)\n");
		}
	}
	usleep(10000);	/* 10ms delay - need to check if it is needed */
#endif

	/* set BIST_START */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",BIST_START_CODE_NAME);

	/* parsing pattern command to get mipi data to be written */
	memset(mipi_data,0,sizeof(mipi_write_data_t));
	ret = parsing_pattern_write_command_for_dp086(code_file_name,code_name,mipi_data);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_write_command_for_dp086\n");
	}
	else
	{
		/* write mipi data if no error */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}
	}
	usleep(10000);	/* 10ms delay - need to check if it is needed */


	/* set flag */
	dp086_p->flag_run_gray_scan_thread = true;
	dp086_p->flag_finish_gray_scan_thread = false;

	/* set variable */
	thread_loop = true;
	positive_dir = true;	/* direction to go up */
	gray_value = 0;			/* clear gray_value to start */

	/* unlock */
	pthread_mutex_unlock(mutex_p);
	
	/* thread loop */
	while (thread_loop)
	{
		/* lock */
		pthread_mutex_lock(mutex_p);

		if (positive_dir == true)
		{
			gray_value += 4;
			if (gray_value >= DP086_GRAY_SCAN_MAX_VALUE*4)
			{
				gray_value = DP086_GRAY_SCAN_MAX_VALUE*4;
				positive_dir = false;
			}
		}
		else
		{
			gray_value -= 4;
			if (gray_value <= DP086_GRAY_SCAN_MIN_VALUE)
			{
				gray_value = DP086_GRAY_SCAN_MIN_VALUE;
				positive_dir = true;
			}
		}
		
        if(gray_value <= 255){
			mipi_data->data_buf[2][2] = (unsigned char)0x00;        
		}else if(gray_value > 255 && gray_value <= 510){
			mipi_data->data_buf[2][2] = (unsigned char)0x15;
		}else if(gray_value > 510 && gray_value <= 765){
			mipi_data->data_buf[2][2] = (unsigned char)0x2A;
		}else if(gray_value > 765){
			mipi_data->data_buf[2][2] = (unsigned char)0x3F;
		}else
			goto WRITE_SKIP;

		mipi_data->data_buf[3][2] = (unsigned char)(gray_value%256);
		mipi_data->data_buf[4][2] = (unsigned char)(gray_value%256);
		mipi_data->data_buf[5][2] = (unsigned char)(gray_value%256);
	
    	/* write mipi data */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}

		if (dp086_p->flag_run_gray_scan_thread == false)
		{
			thread_loop = false;
		}

WRITE_SKIP:
		/* unlock */
		pthread_mutex_unlock(mutex_p);
		
		usleep(10000);	/* 10ms delay for loop */
	}
	
	/* lock */
	pthread_mutex_lock(mutex_p);

	/* set BIST_OUT */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",BIST_OUT_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_command_and_write_for_dp086, system(dp086_bist_off) \n");
		system("/Data/reg_init /mnt/sd/initial/dp086_bist_off.tty");
		usleep(500);
	}
	
	usleep(10000);	/* 10ms delay - need to check if it is needed */

	/* set flag_finish_gray_scan_thread */
	dp086_p->flag_finish_gray_scan_thread = true;

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	DPRINTF("### Gray Scan thread finished! ###\n");
	FUNC_END();
	return NULL;
}

/*
 * Name : start_gray_scan_thread_for_dp086
 * Description : Start gray_scan thread.
 * Parameters :
 * Return value :
 * 		error value
 */
int start_gray_scan_thread_for_dp086(void)
{
	int ret = 0;

	FUNC_BEGIN();

	/* initialize mutex */
	ret = pthread_mutex_init(&model_dp086_p->gray_scan_thread_mutex,NULL);
	if (ret < 0)
	{
		DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	ret = pthread_create(&model_dp086_p->id_gray_scan_thread, NULL, gray_scan_thread_for_dp086, (void *)(model_dp086_p));
	if (ret < 0)
	{
		DERRPRINTF("pthread_create(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	FUNC_END();

	return ret;
}

/*
 * Name : wait_for_finish_gray_scan_thread_for_dp086
 * Description : wait for finishing gray_scan thread.
 * Parameters :
 * Return value :
 */
void wait_for_finish_gray_scan_thread_for_dp086(void)
{
	model_dp086_t *dp086_p = model_dp086_p;	/* no way to avoid using global variable */
	pthread_mutex_t	*mutex_p = &dp086_p->gray_scan_thread_mutex;
	int wait_loop = 0;
	unsigned int timeout_cnt = TIMEOUT_WAIT_FOR_FINISH_THREAD;

	FUNC_BEGIN();

	/* stop gray_scan thread */
	DPRINTF("### Trying to stop gray_scan thread ###\n");
	/* lock */
	pthread_mutex_lock(mutex_p);
	dp086_p->flag_run_gray_scan_thread = false;
	/* unlock */
	pthread_mutex_unlock(mutex_p);

	wait_loop = true;

	/* wait for finishing gray scan */
	while ((wait_loop == true) && (timeout_cnt > 0))
	{
		DPRINTF("### waiting for finishing gray_scan thread ###\n");
		usleep(100000);	/* 100ms delay for wait loop */
		/* lock */
		pthread_mutex_lock(mutex_p);
		if (dp086_p->flag_finish_gray_scan_thread == true)
		{
			wait_loop = false;
		}
		/* unlock */
		pthread_mutex_unlock(mutex_p);
		/* timeout count down */
		timeout_cnt--;
	}

	if (timeout_cnt > 0)
	{
		DPRINTF("### Confirm Gray Scan thread finished! ###\n");
	}
	else if ((timeout_cnt <= 0) && (wait_loop == true))
	{
		DPRINTF("### Timeout waiting for finishing Gray Scan thread ###\n");
	}

	FUNC_END();
}

/*
 * Name : dimming_thread_for_dp086
 * Description : Thread for dimming.
 * Parameters :
 * 		void *arg : arguments for dimming thread.
 * Return value : NULL
 */
void *dimming_thread_for_dp086(void *arg)
{
	int ret = 0;
	model_dp086_t *dp086_p = (model_dp086_t *)arg;
	pthread_mutex_t	*mutex_p = &dp086_p->dimming_thread_mutex;
	int thread_loop = 0;
	char code_name[20];
	int dbv_value = 0;
	int positive_dir = true;
	mipi_write_data_t *mipi_data = &model_dp086_p->g_mipi_write;
	char code_file_name[MAX_FILE_NAME_LENGTH];

	FUNC_BEGIN();

	DPRINTF("######### Start dimming thread ###########\n");

	/* lock */
	pthread_mutex_lock(mutex_p);

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);
	DPRINTF("code_file_name=(%s)\n", code_file_name);

	/* set DBV_IN */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",DBV_IN_CODE_NAME);

	/* parsing pattern command to get mipi data to be written */
	memset(mipi_data,0,sizeof(mipi_write_data_t));
	ret = parsing_pattern_write_command_for_dp086(code_file_name,code_name,mipi_data);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_write_command_for_dp086\n");
	}
	else
	{
		/* write mipi data if no error */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}
	}
	
	usleep(10000);	/* 10ms delay - need to check if it is needed */

	/* set flag */
	dp086_p->flag_run_dimming_thread = true;
	dp086_p->flag_finish_dimming_thread = false;

	/* set variable */
	thread_loop = true;
	positive_dir = false;				/* direction to down */
	dbv_value = DP086_DBV_MAX_VALUE;	/* set dbv_value to start */

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	/* thread loop */
	while (thread_loop)
	{
		/* lock */
		pthread_mutex_lock(mutex_p);

//		DPRINTF("=============================dbv_value=(0x%04x)=============================\n", dbv_value);
		if (positive_dir == true)
		{
			dbv_value+=4;
			if (dbv_value >= DP086_DBV_MAX_VALUE)
			{
				dbv_value = DP086_DBV_MAX_VALUE;
				positive_dir = false;
			}
		}
		else
		{
			dbv_value-=4;
			if (dbv_value <= DP086_DBV_MIN_VALUE)
			{
				dbv_value = DP086_DBV_MIN_VALUE;
				positive_dir = true;
			}
		}
		/* set dbv_value to mipi data */
		mipi_data->data_buf[mipi_data->reg_cnt - 1][2] = (unsigned char)((dbv_value >> 8) & DP086_DBV_HIGH_8BITS_FILTER);
		mipi_data->data_buf[mipi_data->reg_cnt - 1][3] = (unsigned char)((dbv_value >> 0) & 0xff);
		/* write mipi data */
		ret = write_mipi_command(mipi_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}

		if (dp086_p->flag_run_dimming_thread == false)
		{
			thread_loop = false;
		}

		/* unlock */
		pthread_mutex_unlock(mutex_p);

		usleep(1000);	/* 1ms delay for loop */
	}
	
	/* lock */
	pthread_mutex_lock(mutex_p);

	/* set DBV_OUT */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",DBV_OUT_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_command_and_write_for_dp086\n");
	}
	
	usleep(20000);	/* 20ms delay - need to check if it is needed */

	/* set flag_finish_dimming_thread */
	dp086_p->flag_finish_dimming_thread = true;

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	DPRINTF("### dimming thread finished! ###\n");

	FUNC_END();
	return NULL;
}

/*
 * Name : start_dimming_thread_for_dp086
 * Description : Start dimming thread.
 * Parameters :
 * Return value :
 * 		error value
 */
int start_dimming_thread_for_dp086(void)
{
	int ret = 0;

	FUNC_BEGIN();

	/* initialize mutex */
	ret = pthread_mutex_init(&model_dp086_p->dimming_thread_mutex,NULL);
	if (ret < 0)
	{
		DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	ret = pthread_create(&model_dp086_p->id_dimming_thread, NULL, dimming_thread_for_dp086, (void *)(model_dp086_p));
	if (ret < 0)
	{
		DERRPRINTF("pthread_create(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	FUNC_END();

	return ret;
}

/*
 * Name : wait_for_finish_dimming_thread_for_dp086
 * Description : wait for finishing dimming thread.
 * Parameters :
 * Return value :
 */
void wait_for_finish_dimming_thread_for_dp086(void)
{
	model_dp086_t *dp086_p = model_dp086_p;	/* no way to avoid using global variable */
	pthread_mutex_t	*mutex_p = &dp086_p->dimming_thread_mutex;
	int wait_loop = 0;
	unsigned int timeout_cnt = TIMEOUT_WAIT_FOR_FINISH_THREAD;

	FUNC_BEGIN();

	/* stop dimming thread */

	/* lock */
	pthread_mutex_lock(mutex_p);
	/* set flag */
	dp086_p->flag_run_dimming_thread = false;
	/* unlock */
	pthread_mutex_unlock(mutex_p);

	wait_loop = true;

	/* wait for finishing gray scan */
	while ((wait_loop == true) && (timeout_cnt > 0))
	{
		DPRINTF("### waiting for finishing Dimming thread ###\n");
		usleep(100000);	/* 100ms delay for wait loop */
		/* lock */
		pthread_mutex_lock(mutex_p);
		if (dp086_p->flag_finish_dimming_thread == true)
		{
			wait_loop = false;
		}
		/* unlock */
		pthread_mutex_unlock(mutex_p);
		/* timeout count down */
		timeout_cnt--;
	}

	if (timeout_cnt > 0)
	{
		DPRINTF("### Confirm Dimming thread finished! ###\n");
	}
	else if ((timeout_cnt <= 0) && (wait_loop == true))
	{
		DPRINTF("### Timeout waiting for finishing Dimming thread ###\n");
	}

	FUNC_END();
}

/*
 * Name : dsc_roll_thread_for_dp086
 * Description : Thread for dsc_roll.
 * Parameters :
 * 		void *arg : arguments for dsc_roll thread.
 * Return value : NULL
 */
void *dsc_roll_thread_for_dp086(void *arg)
{
	model_dp086_t *dp086_p = (model_dp086_t *)arg;
	model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;
	pthread_mutex_t	*mutex_p = &dp086_p->dsc_roll_thread_mutex;
	int thread_loop = 0;
	int dsc_roll_pic_num = 0;	/* picture number for dsc_roll test - used to get dsc_roll test directory name */
	char *dsc_roll_name_string_p;
	int dsc_roll_file_num = 0;	/* file number to be used for dsc_roll test */
	int dsc_roll_cnt = 0;
	char file_name_string[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	char dsc_roll_test_dir[MAX_FILE_NAME_LENGTH];

	FUNC_BEGIN();

	DPRINTF("######### Start dsc_roll thread ###########\n");

	/* lock */
	pthread_mutex_lock(mutex_p);

	/* set flag */
	dp086_p->flag_run_dsc_roll_thread = true;
	dp086_p->flag_finish_dsc_roll_thread = false;

	/* set variable */
	dsc_roll_pic_num = dp086_p->special_pattern_mode.dsc_pattern_mode.dsc_roll_pic_num;
	dsc_roll_name_string_p = dp086_p->special_pattern_mode.dsc_pattern_mode.dsc_roll_name_string;
	thread_loop = true;
	dsc_roll_cnt = 0;

	/* get dsc_roll file number and file names */
	memset(file_name_string,0,sizeof(file_name_string));
	dsc_roll_file_num = parsing_dsc_roll_test_file_name(dp086_info_p->display_image_dir,dsc_roll_pic_num,dsc_roll_name_string_p,file_name_string);
	/* dsc_roll test directory */
	memset(dsc_roll_test_dir,0,sizeof(dsc_roll_test_dir));
	sprintf(dsc_roll_test_dir,"%c/%02d%s",dp086_info_p->dp086_manager.dir,dsc_roll_pic_num,DSC_ROLL_TEST_DIR_STRING);
	DPRINTF("DSC_ROLL test dir=(%s)\n", dsc_roll_test_dir);

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	/* thread loop */
	while (thread_loop)
	{
		/* lock */
		pthread_mutex_lock(mutex_p);

		display_image_for_vfos(dsc_roll_test_dir,&file_name_string[dsc_roll_cnt][0]);
		dsc_roll_cnt = (dsc_roll_cnt + 1) % dsc_roll_file_num;

		if (dp086_p->flag_run_dsc_roll_thread == false)
		{
			thread_loop = false;
		}

		/* unlock */
		pthread_mutex_unlock(mutex_p);

		usleep(10000);	/* 10ms delay for loop */
	}

	/* lock */
	pthread_mutex_lock(mutex_p);

	/* set flag_finish_dsc_roll_thread */
	dp086_p->flag_finish_dsc_roll_thread = true;

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	DPRINTF("### DSC_ROLL thread finished! ###\n");

	FUNC_END();
	return NULL;
}

/*
 * Name : start_dsc_roll_thread_for_dp086
 * Description : Start dsc_roll thread.
 * Parameters :
 * Return value :
 * 		error value
 */
int start_dsc_roll_thread_for_dp086(void)
{
	int ret = 0;

	FUNC_BEGIN();

	/* initialize mutex */
	ret = pthread_mutex_init(&model_dp086_p->dsc_roll_thread_mutex,NULL);
	if (ret < 0)
	{
		DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	ret = pthread_create(&model_dp086_p->id_dsc_roll_thread, NULL, dsc_roll_thread_for_dp086, (void *)(model_dp086_p));
	if (ret < 0)
	{
		DERRPRINTF("pthread_create(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	FUNC_END();

	return ret;
}

/*
 * Name : wait_for_finish_dsc_roll_thread_for_dp086
 * Description : wait for finishing dsc_roll thread.
 * Parameters :
 * Return value :
 */
void wait_for_finish_dsc_roll_thread_for_dp086(void)
{
	model_dp086_t *dp086_p = model_dp086_p;	/* no way to avoid using global variable */
	pthread_mutex_t	*mutex_p = &dp086_p->dsc_roll_thread_mutex;
	int wait_loop = 0;
	unsigned int timeout_cnt = TIMEOUT_WAIT_FOR_FINISH_THREAD;

	FUNC_BEGIN();

	/* stop gray_scan thread */
	DPRINTF("### Trying to stop dsc_roll thread ###\n");
	/* lock */
	pthread_mutex_lock(mutex_p);
	/* set flag */
	dp086_p->flag_run_dsc_roll_thread = false;
	wait_loop = true;

	/* unlock */
	pthread_mutex_unlock(mutex_p);

	/* wait for finishing gray scan */
	while ((wait_loop == true) && (timeout_cnt > 0))
	{
		DPRINTF("### waiting for finishing dsc_roll thread ###\n");
		usleep(100000);	/* 100ms delay for wait loop */
		/* lock */
		pthread_mutex_lock(mutex_p);
		if (dp086_p->flag_finish_dsc_roll_thread == true)
		{
			wait_loop = false;
		}
		/* unlock */
		pthread_mutex_unlock(mutex_p);
		/* timeout count down */
		timeout_cnt--;
	}

	if (timeout_cnt > 0)
	{
		DPRINTF("### Confirm DSC Roll thread finished! ###\n");
	}
	else if ((timeout_cnt <= 0) && (wait_loop == true))
	{
		DPRINTF("### Timeout waiting for finishing DSC Roll thread ###\n");
	}

	FUNC_END();
}

/*
 * Name : control_special_pattern_mode_for_dp086
 * Description : Control and send mipi command for special pattern mode before/after switching new pattern mode.
 * Parameters : 
 * 		model_dp086_t *dp086_p :
 * 		int type_flag : SPECIAL_PATTERN_PREVIOUS_MODE or SPECIAL_PATTERN_CURRENT_MODE
 *		char *code_file_name_p : name of pattern code file
 *		unsigned int pattern_mode : previous pattern mode
 * Return value :
 */
//int control_special_pattern_mode_for_dp086(model_dp086_t *dp086_p, int type_flag, char *code_file_name_p, unsigned int pattern_mode)
int control_special_pattern_mode_for_dp086(model_dp086_t *dp086_p, int type_flag, char *code_file_name_p, unsigned long long pattern_mode)
{
	int ret = 0;
	int matched = 0;
	int need_to_display_black_pattern = 0;
	int need_to_display_on = 0;
	int need_to_display_off = 0;
	char code_name[20];
	int mipi_data_set = 0;
	mipi_write_data_t *mipi_data = &model_dp086_p->g_mipi_write;

	FUNC_BEGIN();

	memset(&code_name[0],0,sizeof(code_name));
	memset(mipi_data,0,sizeof(mipi_write_data_t));

	if (type_flag == SPECIAL_PATTERN_PREVIOUS_MODE)
	{
		if (pattern_mode & AOD_MODE)
		{
			sprintf(code_name,"%s",AOD_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & EMCONTROL_NO_PWM_MODE)
		{
			sprintf(code_name,"%s",EM_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & EMCONTROL_IN_PWM_MODE)
		{
			sprintf(code_name,"%s",EM_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & SLEEP_MODE)
		{
			sprintf(code_name,"%s",SLEEP_OUT_CODE_NAME);
			matched = true;
			need_to_display_on = true;
		}
		else if (pattern_mode & BRIGHT_LINE_MODE)
		{
			sprintf(code_name,"%s",BRIGHT_LINE_OFF_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & BLACK_LINE_MODE)
		{
			sprintf(code_name,"%s",BLACK_LINE_OFF_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & BLACKPOINT_MODE)
		{
			sprintf(code_name,"%s",BLACK_POINT_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & VARIABLE_MODE)
		{
			sprintf(code_name,"%s",VARIABLE_OFF_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & VR_MODE)
		{
			sprintf(code_name,"%s",VR_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & DBV_VARIABLE_MODE)
		{
			sprintf(code_name,"%s",DBV_VARIABLE_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & GRAY_MODE)	/* wait for finishing thread */
		{
			wait_for_finish_gray_scan_thread_for_dp086();
			matched = false;
		}
		else if (pattern_mode & DIMMING_MODE)	/* wait for finishing thread */
		{
			wait_for_finish_dimming_thread_for_dp086();
			matched = false;
		}
		else if (pattern_mode & DSC_ROLL_MODE)	/* wait for finishing thread */
		{
			wait_for_finish_dsc_roll_thread_for_dp086();
			matched = false;
		}
		//181211
		else if (pattern_mode & DBV_1ST_NIT_MODE)
		{
			sprintf(code_name,"%s",DBV_1ST_NIT_OFF_CODE_NAME);
			matched = true;
		}
        else if (pattern_mode & DBV_2ND_NIT_MODE)
		{
			sprintf(code_name,"%s",DBV_2ND_NIT_OFF_CODE_NAME);
			matched = true;
		}
        else if (pattern_mode & DBV_3RD_NIT_MODE)
		{
			sprintf(code_name,"%s",DBV_3RD_NIT_OFF_CODE_NAME);
			matched = true;
        }
        //190104
        else if (pattern_mode & HBM_MODE)
        {
            sprintf(code_name,"%s",HBM_OFF_CODE_NAME);
            matched = true;
        }
		//190325
        else if (pattern_mode & VOLTAGE_MODE)
        {
            sprintf(code_name,"%s",VOLTAGE_OFF_CODE_NAME);
            matched = true;
        }
		else
		{
			DPRINTF("No match of pattern mode\n");
			matched = false;
		}
	}
	else if (type_flag == SPECIAL_PATTERN_CURRENT_MODE)
	{
		if (pattern_mode & AOD_MODE)
		{
			sprintf(code_name,"%s",AOD_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & EMCONTROL_NO_PWM_MODE)
		{
			sprintf(code_name,"%s",EM_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & EMCONTROL_IN_PWM_MODE)
		{
			sprintf(code_name,"%s",EM_OFF_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & SLEEP_MODE)
		{
			sprintf(code_name,"%s",SLEEP_IN_CODE_NAME);
			matched = true;
//			need_to_display_off = true;	/* this will be set by command in dp086.config file for Sleep mode, but display on command has to be set in F/W */
		}
		else if (pattern_mode & BRIGHT_LINE_MODE)
		{
			sprintf(code_name,"%s",BRIGHT_LINE_ON_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & BLACK_LINE_MODE)
		{
			sprintf(code_name,"%s",BLACK_LINE_ON_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & BLACKPOINT_MODE)
		{
			sprintf(code_name,"%s",BLACK_POINT_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & VARIABLE_MODE)
		{
			sprintf(code_name,"%s",VARIABLE_ON_CODE_NAME);
			matched = true;
			need_to_display_black_pattern = true;
		}
		else if (pattern_mode & VR_MODE)
		{
			sprintf(code_name,"%s",VR_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & DBV_VARIABLE_MODE)
		{
			sprintf(code_name,"%s",DBV_VARIABLE_ON_CODE_NAME);
			matched = true;
		}
		else if (pattern_mode & GRAY_MODE)	/* new thread will run */
		{
			/* display black pattern as the last image is displayed after BIST_OUT, it should be avoided */
			display_black_pattern_for_vfos();
			usleep(50000);	/* 50ms delay to make smooth - TODO: need to adjust later */
			ret = start_gray_scan_thread_for_dp086();
			if (ret < 0)
			{
				DERRPRINTF("start_gray_scan_thread\n");
				FUNC_END();
				return ret;
			}
			matched = false;
		}
		else if (pattern_mode & DIMMING_MODE)	/* new thread will run */
		{
			ret = start_dimming_thread_for_dp086();
			if (ret < 0)
			{
				DERRPRINTF("start_dimming_thread\n");
				FUNC_END();
				return ret;
			}
			matched = false;
		}
		else if (pattern_mode & DSC_ROLL_MODE)	/* new thread will run */
		{
			ret = start_dsc_roll_thread_for_dp086();
			if (ret < 0)
			{
				DERRPRINTF("start_dsc_roll_thread\n");
				FUNC_END();
				return ret;
			}
			matched = false;
        }
        //181211
        else if (pattern_mode & DBV_1ST_NIT_MODE)
        {
            sprintf(code_name,"%s",DBV_1ST_NIT_ON_CODE_NAME);
            matched = true;
        }
        else if (pattern_mode & DBV_2ND_NIT_MODE)
        {
            sprintf(code_name,"%s",DBV_2ND_NIT_ON_CODE_NAME);
            matched = true;
        }
        else if (pattern_mode & DBV_3RD_NIT_MODE)
        {
            sprintf(code_name,"%s",DBV_3RD_NIT_ON_CODE_NAME);
            matched = true;
        }
        //190104
        else if (pattern_mode & HBM_MODE)
        {
            sprintf(code_name,"%s",HBM_ON_CODE_NAME);
            matched = true;
        }
		//190325
        else if (pattern_mode & VOLTAGE_MODE)
        {
            sprintf(code_name,"%s",VOLTAGE_ON_CODE_NAME);
            matched = true;
        }
		else
		{
			DPRINTF("No match of pattern mode\n");
			matched = false;
		}
	}
	else
	{
		DERRPRINTF("Invalid type(%d) for special pattern mode\n", type_flag);
		FUNC_END();
		return -1;
	}

	if (matched == true)
	{
		/* display black pattern if needed */
		if (need_to_display_black_pattern == true)
		{
			display_black_pattern_for_vfos();
		}
		/* set display off code if needed */
		if (need_to_display_off == true)
		{
			display_off_by_command_for_dp086();
		}
		/* display on flag as DISPLAY_ON code has to be set after display PTN */
		if (need_to_display_on == true)
		{
			dp086_p->flag_need_to_display_on = true;
		}

		/* parsing pattern command to get mipi data to be written */
		ret = parsing_pattern_write_command_for_dp086(code_file_name_p,&code_name[0],mipi_data);
		if (ret < 0)
		{
			DERRPRINTF(" parsing_pattern_write_command_for_dp086\n");
			mipi_data_set = false;
		}
		else
		{
			mipi_data_set = true;
		}
		
		/* write mipi data if no error */
		if (mipi_data_set == true)
		{
			ret = write_mipi_command(mipi_data);
			if (ret < 0)
			{
				DERRPRINTF(" write_mipi_command\n");
			}
		}
	}

	FUNC_END();

	return ret;
}

/*
 * Name : send_current_test_result_to_uart_for_dp086
 * Description : Send CURRENT test result to UI AP through UART.
 * Parameters :
 * 		current_test_result_t (*result_p)[] : CURRENT test result.
 * 		int number_of_pattern : The number of pattern.
 * 		int pattern_num : Pattern number of test result.
 * 		int ch_num : channel number
 * Return value :
 */
void send_current_test_result_to_uart_for_dp086(current_test_result_t (*result_p)[MAX_VOLT_NUM], int number_of_pattern, int pattern_num, int ch_num)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[4] = (result_p[ch_num][VCC1].current >> 8) & 0xff;
	uart_buf[5] = result_p[ch_num][VCC1].current & 0xff;
	uart_buf[6] = (result_p[ch_num][VCC2].current >> 8) & 0xff;
	uart_buf[7] = result_p[ch_num][VCC2].current & 0xff;
	uart_buf[8] = (result_p[ch_num][VDDVDH].current >> 8) & 0xff;
	uart_buf[9] = result_p[ch_num][VDDVDH].current & 0xff;
	uart_buf[10] = (result_p[ch_num][VDDEL].current >> 8) & 0xff;
	uart_buf[11] = result_p[ch_num][VDDEL].current & 0xff;
	uart_buf[12] = result_p[ch_num][VCC1].is_over_limit;
	uart_buf[13] = result_p[ch_num][VCC2].is_over_limit;
	uart_buf[14] = result_p[ch_num][VDDVDH].is_over_limit;
	uart_buf[15] = result_p[ch_num][VDDEL].is_over_limit;
	uart_buf[16] = pattern_num;
	uart_buf[17] = number_of_pattern; 
	serial_packet_init(uart_buf,CURRENT,ch_num+1);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_display_info_to_uart_for_dp086
 * Description : Send display image information which NEXT or PREV key is entered to UI AP through UART.
 * Parameters :
 * 		int key_value : key input value.
 * 		model_dp086_t *dp086_p : dp086 main structure.
 * Return value :
 */
void send_display_info_to_uart_for_dp086(int key_value, model_dp086_t *dp086_p)
{
	unsigned char uart_buf[MAX_PACKET];
	MODEL_MANAGER *manager_p = &dp086_p->model_dp086_info.dp086_manager;
	int image_count = manager_p->limit.display.image_count;

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[4] = dp086_p->cur_image_num - 1;
	uart_buf[5] = manager_p->dir;
	uart_buf[6] = image_count;
	uart_buf[7] = 0;	/* VOD count */
	uart_buf[10] = 0; //POCB INFO 
	serial_packet_init(uart_buf,key_value,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : read_key_input_for_dp086
 * Description : Read key input from Key input device.
 * Parameters :
 * Return value : key_value
 */
int read_key_input_for_dp086(void)
{
	int pushed_key = 0;
	KEY_EVENT   ev_key;
	model_dp086_info_t *dp086_info_p = &model_dp086_p->model_dp086_info;

	FUNC_BEGIN();

	if(read (dp086_info_p->key_dev, &ev_key, sizeof (ev_key)))
	{
	    if(ev_key.code != 0)
	    {
	        if(ev_key.value)
			{
				pushed_key = ev_key.code - 1;
				DPRINTF("####key_value=(%d)######\n", pushed_key);
				FUNC_END();
				return pushed_key;
			}
		}
	}

	FUNC_END();
	return 0;
}

/*
 * Name : display_module_on_for_dp086
 * Description : Initialize display panel module.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * 		int model_index
 * Return value : 
 */
void display_module_on_for_dp086(model_dp086_t *dp086_p, int model_index)
{
	char comm[100] ={0,};

	FUNC_BEGIN();

	sprintf(comm,"%s %s/register_data%d.tty",REG_INIT_COMMAND,INITIAL_CODE_DIR_PATH,model_index);
	DPRINTF("command : %s \n", comm);
	system(comm);

	dp086_p->flag_already_module_on = true;
	dp086_p->cur_image_num = 0;

	FUNC_END();
}

/*
 * Name : display_module_off_for_dp086
 * Description : Turn off display panel module.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * 		int model_index
 * Return value : 
 */
void display_module_off_for_dp086(model_dp086_t *dp086_p,int model_index)
{
	char comm[100] ={0,};

	FUNC_BEGIN();

	sprintf(comm,"%s %s/register_sleep_data%d.tty",REG_INIT_COMMAND,INITIAL_CODE_DIR_PATH,model_index);
	DPRINTF("command : %s \n", comm);
	system(comm);

	dp086_p->flag_already_module_on = false;
	dp086_p->cur_image_num = 0;

	FUNC_END();
}


void aod_on_by_command_for_dp086(void)
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[20];

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",AOD_ON_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_command_and_write_for_dp086\n");
	}

	FUNC_END();
}



/*
 * Name : display_on_by_command_for_dp086
 * Description : Display ON by command on DP086.
 * Parameters :
 * Return value : 
 */
void display_on_by_command_for_dp086(void)
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[20];

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",DISPLAY_ON_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_command_and_write_for_dp086\n");
	}

	FUNC_END();
}



/*
 * Name : display_off_by_command_for_dp086
 * Description : Display OFF by command on DP086.
 * Parameters :
 * Return value : 
 */
void display_off_by_command_for_dp086(void)
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[20];

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",DISPLAY_OFF_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_command_and_write_for_dp086\n");
	}

	FUNC_END();
}

/*
 * Name : otp_read_for_dp086
 * Description : Read OTP data from DP086.
 * Parameters :
 * 		unsigned char (*otp_value_p)[] : return value for OTP read.
 * Return value : error
 */
int otp_read_for_dp086(unsigned char (*otp_value_p)[DP086_OTP_READ_LENGTH])
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[20];
	mipi_write_data_t *mipi_write_data = &model_dp086_p->g_mipi_write;
	mipi_read_data_t *mipi_read_data = &model_dp086_p->g_mipi_read;
	unsigned char read_data[DP086_OTP_READ_LENGTH];
	int read_ch_num = 0;
	int read_ch_cnt = 0;
	int read_len = 0;

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	/* write code before reading OTP */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",OTP_READ_PRE_WRITE_CODE_NAME);

	memset(mipi_write_data,0,sizeof(mipi_write_data_t));
	ret = parsing_pattern_write_command_for_dp086(code_file_name,code_name,mipi_write_data);
	if (ret < 0)
	{
		DERRPRINTF("parsing_pattern_write_command_for_dp086\n");
	}
	else
	{
		/* write mipi data if no error */
		ret = write_mipi_command(mipi_write_data);
		if (ret < 0)
		{
			DERRPRINTF("write_mipi_command\n");
		}
	}
	/* read OTP */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",OTP_READ_CODE_NAME);

	memset(mipi_read_data,0,sizeof(mipi_read_data_t));
	ret = parsing_pattern_read_command(code_file_name,code_name,mipi_read_data);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_read_command\n");
		FUNC_END();
//		return ret;
	}
	else
	{
		read_ch_num = DP086_OTP_READ_CHANNEL_NUM;
		for (read_ch_cnt = 0;read_ch_cnt < read_ch_num;read_ch_cnt++)
		{
			memset(&read_data,0,sizeof(read_data));
			if (read_ch_cnt == 0)
			{
				ret = set_mipi_port(DSI_PORT_SEL_A);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
//					return ret;
				}
			}
			else if (read_ch_cnt == 1)
			{
				ret = set_mipi_port(DSI_PORT_SEL_B);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
				//	return ret;
				}
			}
			usleep(3000);	/* 3ms delay */
			read_len = DP086_OTP_READ_LENGTH;
			ret = read_mipi_command(mipi_read_data,read_len,read_data);
			if (ret < 0)
			{
				DERRPRINTF(" read_mipi_command\n");
				ret = set_mipi_port(DSI_PORT_SEL_BOTH);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
				//	return ret;
				}
				FUNC_END();
			//	return ret;
			}
			else
			{
				memcpy(&otp_value_p[read_ch_cnt][0],read_data,read_len);
			}
		}
		ret = set_mipi_port(DSI_PORT_SEL_BOTH);
		if (ret < 0)
		{
			DERRPRINTF("set_mipi_port\n");
			FUNC_END();
	//		return ret;
		}
	}

	FUNC_END();

	return 0;
}

/*
 * Name : pocb_read_for_dp086
 * Description : Read POCB data from DP086 to check whether POCB is enabled or not.
 * Parameters :
 * 		unsigned char (*pocb_status_p)[] : return value of POCB status.
 * Return value : error
 */
int pocb_read_for_dp086(unsigned char (*pocb_status_p)[DP086_POCB_READ_LENGTH])
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[MAX_CODE_NAME_STRING_LENGTH];
	mipi_read_data_t *mipi_data = &model_dp086_p->g_mipi_read;
	//unsigned char read_data[DP086_POCB_READ_LENGTH];
	unsigned char read_data[20];
	int read_ch_num = 0;
	int read_ch_cnt = 0;
	int read_len = 0;
	int pocb_status = 0;

	FUNC_BEGIN();

	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	/* write pre code before reading POCB */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",POCB_READ_PRE_WRITE_CODE_NAME);

	ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_command_and_write_for_dp086\n");
		FUNC_END();
		return ret;
	}

	/* read POCB */
	memset(code_name,0,sizeof(code_name));
	sprintf(code_name,"%s",POCB_READ_CODE_NAME);

	memset(mipi_data,0,sizeof(mipi_read_data_t));
	ret = parsing_pattern_read_command(code_file_name,code_name,mipi_data);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_read_command\n");
		FUNC_END();
		return ret;
	}
	else
	{
		read_ch_num = DP086_POCB_READ_CHANNEL_NUM;
		for (read_ch_cnt = 0;read_ch_cnt < read_ch_num;read_ch_cnt++)
		{
			memset(&read_data,0,sizeof(read_data));
			if (read_ch_cnt == 0)
			{
				ret = set_mipi_port(DSI_PORT_SEL_A);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
					return ret;
				}
			}
			else if (read_ch_cnt == 1)
			{
				ret = set_mipi_port(DSI_PORT_SEL_B);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
					return ret;
				}
			}
			usleep(3000);	/* 3ms delay */
			read_len = DP086_POCB_READ_LENGTH;
			ret = read_mipi_command(mipi_data,read_len,read_data);
			if (ret < 0)
			{
				DERRPRINTF(" read_mipi_command\n");
				pocb_status = POCB_STATUS_NO_READ;	/* default set */
				ret = set_mipi_port(DSI_PORT_SEL_BOTH);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
			//		return ret;
				}
				FUNC_END();
			//	return ret;
			}
			else
			{
				//if ((read_data[0] & DP086_POCB_ENABLE_BIT) == DP086_POCB_ENABLE_VALUE)
				if (!((read_data[0] & (1 << 2)) >>2) | ((read_data[0] & (1 << 7)) >>7)) 
				{
					pocb_status = POCB_STATUS_ON;
				}
				else
				{
					pocb_status = POCB_STATUS_OFF;
				}
				memcpy(&pocb_status_p[read_ch_cnt][0],&pocb_status,read_len);
			}
        }
        ret = set_mipi_port(DSI_PORT_SEL_BOTH);
        if (ret < 0)
        {
            DERRPRINTF("set_mipi_port\n");
            FUNC_END();
       //     return ret;
        }
    }

#if 1
    //190124 brightsource added LABEL
    memset(code_file_name,0,sizeof(code_file_name));
    sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

    memset(code_name,0,sizeof(code_name));
    sprintf(code_name,"%s",LABEL_READ_PRE_WRITE_CODE_NAME);

    ret = parsing_pattern_command_and_write_for_dp086(code_file_name,code_name);
    if (ret < 0)
    {
        DERRPRINTF(" parsing_pattern_command_and_write_for_dp086\n");
        FUNC_END();
        return ret;
    }
    /* read LABEL_LIMIT */
    unsigned char label_limit[5] = {0,};
    unsigned char *read_buf = label_limit;
    ret = parsing_label_limit(code_file_name,code_name,read_buf);
    if (ret < 0)
    {
        DERRPRINTF(" parsing_pattern_read_command\n");
        FUNC_END();
        return ret;
    }

    /* read LABEL */
    memset(code_name,0,sizeof(code_name));
    sprintf(code_name,"%s",LABEL_READ_CODE_NAME);

	memset(mipi_data,0,sizeof(mipi_read_data_t));
	ret = parsing_pattern_read_command(code_file_name,code_name,mipi_data);
    if (ret < 0)
    {
        DERRPRINTF(" parsing_pattern_read_command\n");
        FUNC_END();
        return ret;
    }
    else
    {
        read_ch_num = DP086_POCB_READ_CHANNEL_NUM;
        for (read_ch_cnt = 0;read_ch_cnt < read_ch_num;read_ch_cnt++)
        {
            memset(&read_data,0,sizeof(read_data));
            if (read_ch_cnt == 0)
            {
                ret = set_mipi_port(DSI_PORT_SEL_A);
                if (ret < 0)
                {
                    DERRPRINTF("set_mipi_port\n");
                    FUNC_END();
         //           return ret;
                }
            }
            else if (read_ch_cnt == 1)
            {
                ret = set_mipi_port(DSI_PORT_SEL_B);
                if (ret < 0)
                {
                    DERRPRINTF("set_mipi_port\n");
                    FUNC_END();
           //         return ret;
                }
            }
            usleep(3000);	/* 3ms delay */
            read_len = DP086_LABEL_MAX_LENGTH;
            ret = read_mipi_command(mipi_data,read_len,read_data);
            if (ret < 0)
            {
                DERRPRINTF(" read_mipi_command\n");
                pocb_status = POCB_STATUS_NO_READ;	/* default set */
                read_len = DP086_POCB_READ_LENGTH;
                memcpy(&pocb_status_p[read_ch_cnt][0],&pocb_status,read_len);
                ret = set_mipi_port(DSI_PORT_SEL_BOTH);
                if (ret < 0)
                {
                    DERRPRINTF("set_mipi_port\n");
                    FUNC_END();
             //       return ret;
                }
                FUNC_END();
             //   return ret;
            }
            else
            {
            printf("[MIPI            ] LABEL READ DATA ====== 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  \n",read_data[0],read_data[1],read_data[2],read_data[3],read_data[4]);
            printf("[dp086_config.tty] LABEL READ DATA ====== 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  \n",read_buf[0],read_buf[1],read_buf[2],read_buf[3],read_buf[4]);

                if(!strncmp(read_data, read_buf, 5))
                {   
                        pocb_status = pocb_status_p[read_ch_cnt][0] + 0 + 3;
                }
                else
                {   
                        pocb_status = pocb_status_p[read_ch_cnt][0] + 3 + 2;
                }
                printf("[uart sending data] ====== %d\n",pocb_status);
                read_len = DP086_POCB_READ_LENGTH;
                memcpy(&pocb_status_p[read_ch_cnt][0],&pocb_status,read_len);
            }
        }

        ret = set_mipi_port(DSI_PORT_SEL_BOTH);
        if (ret < 0)
        {
            DERRPRINTF("set_mipi_port\n");
            FUNC_END();
           // return ret;
        }
    }
#endif
    FUNC_END();
    return 0;
}

/*
 * Name : pocb_write_for_dp086
 * Description : Write POCB data to DP086 to set POCB on/off.
 * Parameters :
 * 		unsigned char (*pocb_status_p)[] : write value of POCB status.
 * Return value : error
 */
int pocb_write_for_dp086(unsigned char (*pocb_status_p)[DP086_POCB_WRITE_LENGTH])
{
	int ret = 0;
	char code_file_name[MAX_FILE_NAME_LENGTH];
	char code_name[20];
	mipi_write_data_t *mipi_data = &model_dp086_p->g_mipi_write;
	int write_ch_num = 0;
	int write_ch_cnt = 0;

	FUNC_BEGIN();

	/* set config file name */
	memset(code_file_name,0,sizeof(code_file_name));
	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);

	/* parse code and write command by mipi */
	write_ch_num = DP086_POCB_WRITE_CHANNEL_NUM;
	for (write_ch_cnt = 0;write_ch_cnt < write_ch_num;write_ch_cnt++)
	{
		/* mipi port set */
		if (write_ch_cnt == 0)
		{
			ret = set_mipi_port(DSI_PORT_SEL_A);
			if (ret < 0)
			{
				DERRPRINTF("set_mipi_port\n");
				FUNC_END();
				return ret;
			}
		}
		else if (write_ch_cnt == 1)
		{
			ret = set_mipi_port(DSI_PORT_SEL_B);
			if (ret < 0)
			{
				DERRPRINTF("set_mipi_port\n");
				FUNC_END();
				return ret;
			}
		}
		usleep(3000);	/* 3ms delay */

		/* set code name to parse */
		if (pocb_status_p[write_ch_cnt][0] == POCB_STATUS_ON)
		{
			memset(code_name,0,sizeof(code_name));
			sprintf(code_name,"%s",POCB_ON_WRITE_CODE_NAME);
		}
		else if (pocb_status_p[write_ch_cnt][0] == POCB_STATUS_OFF)
		{
			memset(code_name,0,sizeof(code_name));
			sprintf(code_name,"%s",POCB_OFF_WRITE_CODE_NAME);
		}
		else if (pocb_status_p[write_ch_cnt][0] == POCB_STATUS_NO_READ)
		{
			/* TODO - seems to be nothing to do, error should be return  */
			ret = set_mipi_port(DSI_PORT_SEL_BOTH);
			if (ret < 0)
			{
				DERRPRINTF("set_mipi_port\n");
				FUNC_END();
				return ret;
			}
			FUNC_END();
			return FAIL;
		}

		/* parsing */
		memset(mipi_data,0,sizeof(mipi_write_data_t));
		ret = parsing_pattern_write_command_for_dp086(code_file_name,code_name,mipi_data);
		if (ret < 0)
		{
			DERRPRINTF(" parsing_pattern_write_command_for_dp086\n");
			ret = set_mipi_port(DSI_PORT_SEL_BOTH);
			if (ret < 0)
			{
				DERRPRINTF("set_mipi_port\n");
				FUNC_END();
				return ret;
			}
			FUNC_END();
			return ret;
		}
		else
		{
			/* write mipi data if no error */
			ret = write_mipi_command(mipi_data);
			if (ret < 0)
			{
				DERRPRINTF(" write_mipi_command\n");
				ret = set_mipi_port(DSI_PORT_SEL_BOTH);
				if (ret < 0)
				{
					DERRPRINTF("set_mipi_port\n");
					FUNC_END();
					return ret;
				}
				FUNC_END();
				return ret;
			}
		}
	}

	ret = set_mipi_port(DSI_PORT_SEL_BOTH);
	if (ret < 0)
	{
		DERRPRINTF("set_mipi_port\n");
		FUNC_END();
		return ret;
	}
	FUNC_END();

	return 0;
}

/*
 * Name : init_pocb_status_for_dp086
 * Description : Set current POCB status to init POCB status.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void init_pocb_status_for_dp086(model_dp086_t *dp086_p)
{
	FUNC_BEGIN();

	/* set POCB current status to initial status */
	dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_1] = dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1];
	dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_2] = dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2];

	FUNC_END();
}

/*
 * Name : update_pocb_status_for_dp086
 * Description : Update(Write) POCB current status to display module.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void update_pocb_status_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	unsigned char pocb_write[DP086_POCB_WRITE_CHANNEL_NUM][DP086_POCB_WRITE_LENGTH];

	FUNC_BEGIN();

	/* write initial POCB status */
	memset(pocb_write,0,sizeof(pocb_write));
	pocb_write[POCB_CHANNEL_1][0] = dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_1];
	pocb_write[POCB_CHANNEL_2][0] = dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_2];
	ret = pocb_write_for_dp086(pocb_write);
	if (ret < 0)
	{
		DERRPRINTF("pocb_write\n");
	}

	FUNC_END();
}

/*
 * Name : reset_display_mode_for_dp086
 * Description : Reset display mode, for example special pattern mode will be off or a thread will be finished.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : error
 */
int reset_display_mode_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int type_flag = 0;
	char image_file_name[MAX_FILE_NAME_LENGTH];
	char code_file_name[MAX_FILE_NAME_LENGTH];

	FUNC_BEGIN();

	memset(&image_file_name[0],0,sizeof(image_file_name));
	memset(&code_file_name[0],0,sizeof(code_file_name));

	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);
	DPRINTF("code_file_name=(%s)\n", code_file_name);

	if (dp086_p->special_pattern_mode.pattern_mode != NORMAL_MODE)
	{
		/* control before displaying next image - mainly pattern off code or finishing thread */
		type_flag = SPECIAL_PATTERN_PREVIOUS_MODE;
		ret = control_special_pattern_mode_for_dp086(dp086_p,type_flag,code_file_name,dp086_p->special_pattern_mode.pattern_mode);
		if (ret < 0)
		{
			DERRPRINTF("control_special_pattern_mode\n");
			FUNC_END();
			return ret;
		}
		/* initialize of pattern_mode */
		dp086_p->special_pattern_mode.pattern_mode = NORMAL_MODE;
	}

	FUNC_END();

	return 0;
}

/*
 * Name : set_display_mode_for_dp086
 * Description : Set display mode, for example special pattern mode will be applied or a thread will be started.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : error
 */
int set_display_mode_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int type_flag = 0;
	char image_file_name[MAX_FILE_NAME_LENGTH];
	char code_file_name[MAX_FILE_NAME_LENGTH];

	FUNC_BEGIN();

	memset(&image_file_name[0],0,sizeof(image_file_name));
	memset(&code_file_name[0],0,sizeof(code_file_name));

	sprintf(code_file_name,"%s/%s/%s", SD_CARD_DIR_PATH, DP086_CONFIG_DIR_NAME, DP086_CONFIG_FILE_NAME);
	DPRINTF("code_file_name=(%s)\n", code_file_name);

	if (dp086_p->special_pattern_mode.pattern_mode != NORMAL_MODE)
	{
		/* control before displaying next image - mainly pattern off code or finishing thread */
		type_flag = SPECIAL_PATTERN_CURRENT_MODE;
		ret = control_special_pattern_mode_for_dp086(dp086_p,type_flag,code_file_name,dp086_p->special_pattern_mode.pattern_mode);
		if (ret < 0)
		{
			DERRPRINTF("control_special_pattern_mode\n");
			FUNC_END();
			return ret;
		}
	}

	FUNC_END();

	return 0;
}

/*
 * Name : display_current_test_image_for_dp086
 * Description : Display current test image.
 * Parameters :
 * 		model_dp086_t *dp086_p : 
 * 		int pattern_num : pattern number to display.
 * Return value : 
 */
void display_current_test_image_for_dp086(model_dp086_t *dp086_p, int pattern_num)
{
		char comm[100] ={0,};
		char current_test_dir[MAX_FILE_NAME_LENGTH];
		char current_file_name[MAX_FILE_NAME_LENGTH];
		model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;

		FUNC_BEGIN();

		/* set current test directory */
		memset(current_test_dir,0,sizeof(current_test_dir));
		sprintf(current_test_dir,"%c/%s",dp086_info_p->dp086_manager.dir,CURRENT_DIR_NAME);
		DPRINTF("CURRENT test dir=(%s)\n", current_test_dir);

		/* set current file name */
		memset(current_file_name,0,sizeof(current_file_name));
		sprintf(current_file_name,"%s%d%s",CURRENT_TEST_FILE_STRING,pattern_num+1,CURRENT_TEST_FILE_EXT);
		DPRINTF("CURRENT file name =(%s)\n", current_file_name);

		/* display current test file */
		sprintf(comm,"%s %s/%s/%s %s", PIC_VIEW_COMMAND, SD_CARD_DIR_PATH, current_test_dir, current_file_name, DECON_START_STOP_COMMAND);

#if 0	
		/* DP086 test2 mode is AOD */
		if(pattern_num == 1)
				aod_on_by_command_for_dp086();
#endif

#if 1		// LWG 190411 DBV overcurrent issue : if 1 --> if 0      
		/* 190108 DP086 current test1 mode is SCREEN MODE*/      
		if(pattern_num == 0)
		{     
				//system("/Data/reg_init /mnt/sd/initial/dp086_screen_on.tty");     
				system("/Data/reg_init /mnt/sd/initial/register_sleep_data9.tty");     
				usleep(200);     
				system("/Data/reg_init /mnt/sd/initial/dp086_screen_on.tty");     
				
				sleep(3);     
		}
		else if(pattern_num == 1)
		{     
				//system("/Data/reg_init /mnt/sd/initial/dp086_screen_off.tty");     
				system("/Data/reg_init /mnt/sd/initial/register_sleep_data9.tty");     
				usleep(201);     
				system("/Data/reg_init /mnt/sd/initial/register_data9.tty");     
				DPRINTF("command : %s \n", comm);
				system(comm);
		}
		else
		{
		 DPRINTF("command : %s \n", comm);
		 system(comm);
		}
}
#endif
/*
 * Name : get_current_test_result_for_dp086
 * Description : Get CURRENT test result.
 * Parameters :
 * 		model_dp086_t *dp086_p : 
 * 		int pattern_num : pattern number to display and test.
 * 		current_result_t (*result_p)[] : return CURRENT test result.
 * Return value : error
 */
int get_current_test_result_for_dp086(model_dp086_t *dp086_p, int pattern_num, current_test_result_t (*result_p)[MAX_VOLT_NUM])
{
	int ret = 0;
	int ch_cnt = 0;
	int index = 0;
	char *i2c_dev = NULL;
	unsigned long funcs = 0;	
	float temp_f = 0;
	float err_f = 0;
    short value = 0;
    short current = 0;
	int voltage = 0;
    unsigned int limit =0;
	int is_over_limit = 0;
	int i2c_fd = 0;
	int ina_slv = 0;
	short lgd_offset = 0;
	current_test_result_t current_test_result[DP086_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];
	MODEL_MANAGER *dp086_manager_p = &dp086_p->model_dp086_info.dp086_manager;
	struct current_limit *current_limit_p = &dp086_manager_p->limit.current;

	FUNC_BEGIN();

	memset(&current_test_result,0,sizeof(current_test_result_t));

	if (current_limit_p->volt_count > MAX_VOLT_NUM)
	{
		DERRPRINTF("Volt Count err[%d] \n", current_limit_p->volt_count);
		FUNC_END();
		return FAIL;
	}
	DPRINTF("Pattern Count [%d] Volt Count [%d] \n", current_limit_p->pattern_count,current_limit_p->volt_count);

	/* display image for CURRENT test */
	display_current_test_image_for_dp086(dp086_p,pattern_num);
	
	/* measure and get CURRENT test result */
	for (ch_cnt = 0;ch_cnt < DP086_CURRENT_TEST_CHANNEL_NUM;ch_cnt++)
	{
		/* set i2c device node */
		if (ch_cnt == VFOS_CHANNEL_1_NUM)
		{
			i2c_dev = CURRENT_TEST_I2C_1_DEV;
		}
		else if (ch_cnt == VFOS_CHANNEL_2_NUM)
		{
			i2c_dev = CURRENT_TEST_I2C_2_DEV;
		}
		else
		{
			DERRPRINTF("wrong channel [%d].. \n", ch_cnt);
			ret = FAIL;
			continue;
		}
	
		/* open i2c device */
		i2c_fd = open(i2c_dev, O_RDWR);
		if(i2c_fd < 0)
		{
			DERRPRINTF("[%s] I2C Device Open Failed..\n", i2c_dev);
			ret = FAIL;
			continue;
        }
#if 0
        if (ioctl(i2c_fd, I2C_FUNCS, &funcs) < 0) {
            DERRPRINTF("Could not get the adapter "
                    "functionality matrix: %s\n", strerror(errno));
            ret = FAIL;
            close(i2c_fd);
            continue;
        }
#endif
        /* measure */
        for(index = VCC1; index < current_limit_p->volt_count; index++)
        {
			if(index == VCC1)	/* VPNL - 3.0V */
			{
				DPRINTF("> VCC1 ------------ \n");
				err_f = 0.91;
				temp_f = 0;
			}
			else if(index == VCC2)	/* VDDI - 1.8V */
			{
				DPRINTF("> VCC2 ------------ \n");
				err_f = 0.84;
				temp_f = 0;
			}
			else if(index == VDDVDH)	/* DDVDH */
			{
				DPRINTF("> VDDVDH ------------ \n");
				err_f = 0.89;
				temp_f = 0;
			}
			else if(index == VDDEL)	/* ELVDD */
			{
				DPRINTF("> VDDEL ------------ \n");
				err_f = 0.98;
				temp_f = 0;
			}
			else
			{
				DERRPRINTF("Volt Index err[%d] \n",index);
				ret = FAIL;
				continue;
			}
	
			/* get i2c slave address - but why? */
			ina_slv = i2c_slv_addr(index);
			if (ioctl(i2c_fd, I2C_SLAVE_FORCE, ina_slv) < 0) 
			{
				DERRPRINTF("Error: Could not set address[reg:0x%X] \n",ina_slv);
				ret = FAIL;
				continue;
			}
	
			/* get voltage */
			voltage = get_measured_voltage(i2c_fd);
	
			DPRINTF("	V > %d\n",voltage);
	
			/* get current */
			value = get_measured_current(i2c_fd);
			temp_f = value * err_f;
			current = (short)temp_f;
				
			/* debug print */
#if	0	/* swchoi - comment not to print debug message */
			DPRINTF("0. [CH%d] [PW%d] ori current %d \n",ch_cnt,index,value);
			DPRINTF("1. [CH%d] [PW%d] float current %f \n",ch_cnt,index,temp_f);
			DPRINTF("2. [CH%d] [PW%d] modify current %d \n",ch_cnt,index,current);
			DPRINTF("3. [CH%d] [PW%d] err data %f \n",ch_cnt,index,err_f);
#endif	/* swchoi - end */
	
			/* apply LGD offset */
			if (pattern_num == DP086_CURRENT_PATTERN_WHITE)
			{
				if (index == VCC1)	/* VPNL */
				{
					lgd_offset = DP086_CURRENT_WHITE_VPNL_LGD_OFFSET;
				}
				else if (index == VCC2)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_WHITE_VDDI_LGD_OFFSET;
				}
				else if (index == VDDVDH)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_WHITE_VDDVDH_LGD_OFFSET;
				}
				else if (index == VDDEL)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_WHITE_VDDEL_LGD_OFFSET;
				}
			}
#if	1	/* swchoi - 40% PTN was removed for current test, so this code has to be removed - 20180827 */
			else if (pattern_num == DP086_CURRENT_PATTERN_40)
			{
				if (index == VCC1)	/* VPNL */
				{
					lgd_offset = DP086_CURRENT_40_VPNL_LGD_OFFSET;
				}
				else if (index == VCC2)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_40_VDDI_LGD_OFFSET;
				}
				else if (index == VDDVDH)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_40_VDDVDH_LGD_OFFSET;
				}
				else if (index == VDDEL)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_40_VDDEL_LGD_OFFSET;
				}
			}
#endif	/* swchoi - end */
			else if (pattern_num == DP086_CURRENT_PATTERN_SMILE)
			{
				if (index == VCC1)	/* VPNL */
				{
					lgd_offset = DP086_CURRENT_SMILE_VPNL_LGD_OFFSET;
				}
				else if (index == VCC2)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_SMILE_VDDI_LGD_OFFSET;
				}
				else if (index == VDDVDH)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_SMILE_VDDVDH_LGD_OFFSET;
				}
				else if (index == VDDEL)	/* VDDI */
				{
					lgd_offset = DP086_CURRENT_SMILE_VDDEL_LGD_OFFSET;
				}
			}

#if	0	/* swchoi - comment of debug print */
			DPRINTF("Ori_current=(%d),LGD offset=(%d)\n",current,lgd_offset);
#endif	/* swchoi - end */

			lgd_offset = 0;//not used offset

			current += lgd_offset;
			
            //if (current < 0)
			if (current < 10)//1.0mA under to ALL zero
				current = 0;

			DPRINTF("	C > %d\n",current);
	
			/* get limit */
			limit = current_limit_p->max_current[pattern_num][index];
			DPRINTF("	C Limit > %d\n",limit);
	
			/* judgement */
	        if(current > limit)
		    {
			    DPRINTF("[OCP] CH %2d, %d.%dmA Limit %d.%dmA \n",ch_cnt, current/10,current%10, limit/10, limit%10);
				is_over_limit = true;
			}
			else
			{
				is_over_limit = false;
			}

			/* set value */
			current_test_result[ch_cnt][index].voltage = voltage;
			current_test_result[ch_cnt][index].current = current;
			current_test_result[ch_cnt][index].is_over_limit = is_over_limit;
		}
	
		/* close i2c device */
		close(i2c_fd);
	}

	/* return CURRENT test result */
	memcpy(result_p, current_test_result,sizeof(current_test_result));

	FUNC_END();

	return ret;
}

int get_touch_test_result_for_dp086(int ch_num, unsigned int *test_result_p)
{
	int ret = 0;
	int err_ret = 0;

	FUNC_BEGIN();
	ret = atmel_init_i2c_set_slvAddr_depending_channel2(ch_num, 40, test_result_p);
	if (ret <= 0)
	{
		DERRPRINTF("init_i2c_set_slvAddr_depending_channel\n");
		err_ret = -1;
	}
	printf("------------------------------------------------------------ \n");
	ret = mxt_initialize();
	if (ret <= 0)
	{
		DERRPRINTF("SPI NOT CONNECTED. TEST ALL FAIL.\n");
		err_ret = -1;
		*test_result_p = 65535;//all fail
		goto ERR;
	}
	printf("------------------------------------------------------------ \n");
#if 1
	DTPRINTF("[TD01] SPI, INT_CHG_PIN, AVDD TEST\n");
	ret = initial_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD01] SPI, INT_CHG_PIN, AVDD TEST\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_SPI_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_SPI_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD02] FW_Ver_check\n");
	ret = FW_Ver_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD02] FW_Ver_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_FW_VER_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_FW_VER_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif

	config_restore();

#if 1
	DTPRINTF("[TD03] CONFIG_Ver_check\n");
	ret = CONFIG_Ver_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD03] CONFIG_Ver_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_CONFIG_VER_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_CONFIG_VER_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD26] PRODUCT_ID_check\n");
	ret = atmel_02_product_id_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD26] PRODUCT_ID_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_ATMEL_PRODUCT_ID_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_ATMEL_PRODUCT_ID_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD17] Pin_fault_check\n");
	ret = pin_fault_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD17] Pin_fault_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_PIN_FAULT_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_PIN_FAULT_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD06] Node_detection_check\n");
	ret = atmel_02_Node_detection_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD06] Node_detection_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_NODE_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_NODE_TEST);
	printf("\n----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 0
	DTPRINTF("[TD33] FORCE_touch_check\n");
	ret = force_touch_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD33] FORCE_touch_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_FORCE_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_FORCE_TEST);
	printf("----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD18] MICRO_defect_check\n");
	ret = micro_defect_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD18] MICRO_defect_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_MICRO_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_MICRO_TEST);
	printf("\n----------------------result = %d------------------------------ \n",*test_result_p);
#endif
#if 1
	DTPRINTF("[TD07] Delta_limit_check\n");
	ret = atmel_02_delta_limit_check();
	if (ret <= 0)
	{
		DERRPRINTF("[TD07] Delta_limit_check\n");
		err_ret = -1;
		*test_result_p |= (1 << TOUCH_DELTA_TEST);
	}else
		*test_result_p &= ~(1 << TOUCH_DELTA_TEST);
	printf("\n----------------------result = %d------------------------------ \n\n",*test_result_p);
#endif

	config_restore();
ERR :

	release_i2c_set();

	FUNC_END();

	return err_ret;
}

/*
 * Name : otp_key_action_for_dp086
 * Description : Key action for OTP key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void otp_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	unsigned char otp_value[DP086_OTP_READ_CHANNEL_NUM][DP086_OTP_READ_LENGTH];
	unsigned char pocb_status[DP086_POCB_READ_CHANNEL_NUM][DP086_POCB_READ_LENGTH];
	unsigned char additional_info = 0;
	unsigned char otp_write_num[DP086_OTP_READ_CHANNEL_NUM] = {0,0};
	int ch_cnt = 0;
	int debug_ch_cnt = 0;
	int debug_data_cnt = 0;

	FUNC_BEGIN();

	if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
	{
		ret = reset_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("reset_display_mode_for_dp086\n");
		}
	}

	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_black_pattern_for_vfos();
	}

	/* send uart command to indicate OTP function start */
	send_function_start_info_to_uart(OTP);

	/* get OTP value */
	memset(otp_value,0,sizeof(otp_value));
	ret = otp_read_for_dp086(otp_value);
	if (ret < 0)
	{
		DERRPRINTF("otp_read_for_dp086\n");
	}
	else
	{
		/* debug print to check OTP read value */
		DPRINTF("OTP_READ: ");
		for (debug_ch_cnt = 0;debug_ch_cnt < DP086_OTP_READ_CHANNEL_NUM;debug_ch_cnt++)
		{
			for (debug_data_cnt = 0;debug_data_cnt < DP086_OTP_READ_LENGTH;debug_data_cnt++)
			{
				printf("[0x%02x] ", otp_value[debug_ch_cnt][debug_data_cnt]);
			}
			printf("\n");
		}
		/* debug print end */

		/* get otp display write number */
		for (ch_cnt = 0;ch_cnt < DP086_OTP_READ_CHANNEL_NUM;ch_cnt++)
		{
			int remained_otp_cnt = 0;
			char result = 0xFF;

			result = otp_value[ch_cnt][DP086_OTP_OFFSET] << 3;
			if(result == 0xF8)
				remained_otp_cnt=5;
			else if(result == 0xD0)
				remained_otp_cnt=4;
			else if(result == 0xA8)
				remained_otp_cnt=3;
			else if(result == 0x80)
				remained_otp_cnt=2;
			else if(result == 0x60)
				remained_otp_cnt=1;
			else if(result == 0x40)
				remained_otp_cnt=0;
			else 
				remained_otp_cnt=6;
	//		remained_otp_cnt = GET_DP086_OTP_VALUE(otp_value[ch_cnt][DP086_OTP_OFFSET]);
			otp_write_num[ch_cnt] = DP086_OTP_MAX_WRITE_TIME - remained_otp_cnt;
			DPRINTF("otp_write_num[%d]=(%d)\n",ch_cnt,otp_write_num[ch_cnt]);
		}
	}

	/* get POCB value and check status */
	ret = pocb_read_for_dp086(pocb_status);
	if (ret < 0)
	{
		DERRPRINTF("pocb_read_for_dp086\n");
	}
	else
	{
		DPRINTF("POCB_STATUS: CH1=[%d],CH2=[%d]\n",pocb_status[POCB_CHANNEL_1][0],pocb_status[POCB_CHANNEL_2][0]);
		if (dp086_p->pocb_status.flag_pocb_changed == POCB_STATUS_NO_CHANGE)
		{
			for (ch_cnt = 0;ch_cnt < DP086_POCB_READ_CHANNEL_NUM;ch_cnt++)
			{
				dp086_p->pocb_status.pocb_init_status[ch_cnt] = pocb_status[ch_cnt][0];	/* set init status */
			}
		}
	}

	/* send OTP value and POCB status to UI by UART */
	for (ch_cnt = 0;ch_cnt < DP086_OTP_READ_CHANNEL_NUM;ch_cnt++)
	{
		send_otp_key_to_uart(ch_cnt + 1,otp_write_num[ch_cnt],DP086_OTP_MAX_WRITE_TIME,pocb_status[ch_cnt][0],additional_info);
	}

	dp086_p -> flag_otp_test_result_ch1 = 0;		// 재 테스트를 위한 초기화	
	dp086_p -> flag_otp_test_result_ch2 = 0;		// 재 테스트를 위한 초기화	
	// LWG : make for send_judge_status_to_uart, buf[4] is otp_write_num, buf[5] is otp_max_write_time
	for (ch_cnt = 0;ch_cnt < DP086_OTP_READ_CHANNEL_NUM;ch_cnt++)
	{
		if((otp_write_num[ch_cnt] > DP086_OTP_MAX_WRITE_TIME) || (otp_write_num[ch_cnt] == 0)){
			if(ch_cnt == 0){
				dp086_p -> flag_otp_test_result_ch1 = 2;		// OTP TEST FAIL
			}else{
				dp086_p -> flag_otp_test_result_ch2 = 2;		// OTP TEST FAIL
			}
		}
		//else if ((dp086_p -> flag_otp_test_result) != 2){
		else if ((otp_write_num[ch_cnt] <= DP086_OTP_MAX_WRITE_TIME) && (otp_write_num[ch_cnt] != 0)){
				if(ch_cnt == 0){
					if(dp086_p -> flag_otp_test_result_ch1 != 2){		// 1채널에서 실패한 적이 없어야 성공
							dp086_p -> flag_otp_test_result_ch1 = 1;		// OTP TEST SUCCESS
					}
				}else{
					if(dp086_p -> flag_otp_test_result_ch2 != 2){		// 1채널에서 실패한 적이 없어야 성공
							dp086_p -> flag_otp_test_result_ch2 = 1;		// OTP TEST SUCCESS
					}
				}
		}
	}
	display_module_off_for_dp086(dp086_p,model_index);
	FUNC_END();
}

/*
 * Name : touch_key_action_for_dp086
 * Description : Key action for TOUCH key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void touch_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	unsigned int result = 0;
	int ch_cnt = 0;
	int hf_test_on = 0;
    MODEL_MANAGER *dp086_manager_p = &dp086_p->model_dp086_info.dp086_manager;
	struct atmel_02_touch_limit *touch_limit_p;
	//struct synaptics_touch_limit *touch_limit_p;
	touch_limit_p = (struct atmel_02_touch_limit *)dp086_manager_p->limit.ptouch;

	FUNC_BEGIN();

	if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
	{
		ret = reset_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("reset_display_mode_for_dp086\n");
		}
	}

	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_black_pattern_for_vfos();
	}

	/* send uart command to indicate TOUCH function start */
	send_function_start_info_to_uart(TOUCH);

	/* init touch test */
	init_tch_power_set(1);
	atmel_02_init_limit_data(touch_limit_p);
	dp086_p -> flag_touch_test_result_ch1 = 0;
	dp086_p -> flag_touch_test_result_ch2 = 0;
	/* run touch test and send test result to UI */
	for(ch_cnt = 1; ch_cnt <3; ch_cnt++)
	{
		result = 0x0;	/* init test result - swchoi */

		ret = get_touch_test_result_for_dp086(ch_cnt,&result);
		if (ret < 0)
		{
			DERRPRINTF("get_touch_test_result_for_dp086(ch_num=(%d),test_result=0x%08x)\n",ch_cnt,result);
		}
		else
		{
			DPRINTF("#############ch_num=(%d),touch tset result=(0x%08x)#############\n",ch_cnt,result);	/* debug */
		}

		/* need uart send depending on the channel */
		hf_test_on = false;	/* hf_test is used for only JOAN model, so hf_test_on has to be false for DP086 */
		send_touch_test_result_to_uart(result,hf_test_on,ch_cnt-1);	/* channel number which put as paramter is different from other function, therefore ch_cnt should be decreased before put */
		if(result){
				if(ch_cnt == 1){
						dp086_p -> flag_touch_test_result_ch1 = 2;		// FAIL
				}else{
						dp086_p -> flag_touch_test_result_ch2 = 2;		// FAIL
				}
		//else if((dp086_p -> flag_touch_test_result) != 2){	// 이전에 실패한적이 없어야 한다.
		}else if(!result){
				if(ch_cnt == 1){
						if(dp086_p -> flag_touch_test_result_ch1 != 2){
								dp086_p -> flag_touch_test_result_ch1 = 1;		// PASS
						}
				}else{
						if(dp086_p -> flag_touch_test_result_ch2 != 2){
								dp086_p -> flag_touch_test_result_ch2 = 1;		// PASS
						}
				}
		}
	}
	usleep(30000);
	/* power off */
	init_tch_power_set(0);
	display_module_off_for_dp086(dp086_p,model_index);
	
	FUNC_END();
}

/*
 * Name : current_key_action_for_dp086
 * Description : Key action for CURRENT key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void current_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int pattern_cnt = 0;
	int ch_cnt = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	struct current_limit *current_limit_p = &dp086_p->model_dp086_info.dp086_manager.limit.current;

	FUNC_BEGIN();

	if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
	{
		ret = reset_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("reset_display_mode_for_dp086\n");
		}
	}

	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_black_pattern_for_vfos();
	}

	/* send uart command to indicate CURRENT function start */
	send_function_start_info_to_uart(CURRENT);

	// judge_interlock_current 수행전에 초기화
	dp086_p -> flag_current_test_result_ch1 = 0;
	dp086_p -> flag_current_test_result_ch2 = 0;

	/* send uart command to send CURRENT test result to UI */
	for (pattern_cnt = 0;pattern_cnt < current_limit_p->pattern_count;pattern_cnt++)
	{
		/* get current test result such as voltage and current */
		ret = get_current_test_result_for_dp086(dp086_p,pattern_cnt,dp086_p->current_test_result);
		if (ret < 0)
		{
			DERRPRINTF("get_current_test_result_for_dp086\n");
		}

		for (ch_cnt = 0;ch_cnt < DP086_CURRENT_TEST_CHANNEL_NUM;ch_cnt++)
		{
			send_current_test_result_to_uart_for_dp086(dp086_p->current_test_result,current_limit_p->pattern_count,pattern_cnt,ch_cnt);
		}

		// judge_interlock_current 시작 : '각 이미지' 별로 is_over_limit 가 누적되어어야 한다
		judge_interlock_current_for_dp086(dp086_p);

		usleep(500000);	/* 500ms delay - need to check if needed */
	}

	display_module_off_for_dp086(dp086_p,model_index);
	FUNC_END();
}

/*
 * Name : func_key_action_for_dp086
 * Description : Key action for FUNC key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void func_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;

	FUNC_BEGIN();

	if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
	{
		ret = reset_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("reset_display_mode_for_dp086\n");
		}
	}
	if (dp086_p->flag_already_module_on == true)
	{
		display_module_off_for_dp086(dp086_p,model_index);
		send_reset_key_to_uart();
	}

	FUNC_END();
}

/*
 * Name : next_key_action_for_dp086
 * Description : Key action for NEXT(TURN) key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void next_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;
	MODEL_MANAGER *manager_p = &dp086_info_p->dp086_manager;
	int image_count = manager_p->limit.display.image_count;
	char image_file_name[MAX_FILE_NAME_LENGTH];
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();
	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_black_pattern_for_vfos();
	}

	ret = reset_display_mode_for_dp086(dp086_p);
	if (ret < 0)
	{
		DERRPRINTF("reset_display_mode_for_dp086\n");
	}

	/* change current image number */
	dp086_p->cur_image_num = (dp086_p->cur_image_num + 1) % (image_count + 1);
	if (dp086_p->cur_image_num <= 0)
	{
		dp086_p->cur_image_num = 1;
	}

	/* parsing pattern mode from image file name */
	strcpy(&image_file_name[0],&dp086_info_p->display_image_file_name[dp086_p->cur_image_num - 1][0]);
	DPRINTF("cur_image_num=(%d),image_file_name=(%s)\n",dp086_p->cur_image_num,&image_file_name[0]);

	ret = parsing_pattern_mode(&image_file_name[0],&dp086_p->special_pattern_mode);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_mode\n");
	}
	else
	{
		DPRINTF("Pattern mode=(0x%x)\n", dp086_p->special_pattern_mode.pattern_mode);

		/* display white pattern for only dimming thread */
		if (((dp086_p->special_pattern_mode.pattern_mode & DIMMING_MODE) == DIMMING_MODE) && ((dp086_p->special_pattern_mode.pattern_mode & DBV_MODE) == DBV_MODE))
		{
			display_white_pattern_for_vfos();
			usleep(200000);	/* 200ms delay - need to check if the delay is OK */
		}
	
		/* set special pattern code */
		ret = set_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("set_display_mode_for_dp086\n");
		}
	}

	/* set POCB if POCB channel 1 & 2 are not NO_READ, and pattern allows to write POCB status */
	if ((dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1] != POCB_STATUS_NO_READ) && (dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2] != POCB_STATUS_NO_READ))
	{
		/* initialize POCB status */
		init_pocb_status_for_dp086(dp086_p);
		if ((dp086_p->special_pattern_mode.pattern_mode & POCB_WRITE_ENABLE_MODE) == POCB_WRITE_ENABLE_MODE)
		{
			/* write POCB status to display module */
			update_pocb_status_for_dp086(dp086_p);
		}
	}

	/* display image - except gray scan and dimming and gradation */
	if (((dp086_p->special_pattern_mode.pattern_mode & GRAY_MODE) != GRAY_MODE) && (((dp086_p->special_pattern_mode.pattern_mode & DIMMING_MODE) != DIMMING_MODE) && ((dp086_p->special_pattern_mode.pattern_mode & DBV_MODE) != DBV_MODE)) && ((dp086_p->special_pattern_mode.pattern_mode & GRAD_MODE) != GRAD_MODE))
	{
		display_image_for_vfos(dp086_info_p->display_image_dir,image_file_name);
	}
	else if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_MODE) == GRAD_MODE)	/* display pattern for Gradation display */
	{
		if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_RGB_MODE) == GRAD_RGB_MODE)
		{
			if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_TOP_MODE) == HORIZONTAL_TOP_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_HT_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_DOWN_MODE) == HORIZONTAL_DOWN_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_HD_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_LEFT_MODE) == VERTICAL_LEFT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_VL_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_RIGHT_MODE) == VERTICAL_RIGHT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_VR_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
		}
		else if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_RED_MODE) == GRAD_RED_MODE)
		{
			if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_TOP_MODE) == HORIZONTAL_TOP_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_HT_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_DOWN_MODE) == HORIZONTAL_DOWN_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_HD_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_LEFT_MODE) == VERTICAL_LEFT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_VL_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_RIGHT_MODE) == VERTICAL_RIGHT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_VR_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
		}
	}

	/* If display off code was set for SLEEP PTN, need to set display on code */
	if (dp086_p->flag_need_to_display_on == true)
	{
		usleep(100000);	/* 100ms delay between displaying image and display on command as LGD provided */
		display_on_by_command_for_dp086();
		dp086_p->flag_need_to_display_on = false;
	}

	is_display_on_for_next_prev = 1;		// LWG 190328 need for interlock
	FUNC_END();
}

/*
 * Name : prev_key_action_for_dp086
 * Description : Key action for PREV(RETURN) key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void prev_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;
	MODEL_MANAGER *manager_p = &dp086_info_p->dp086_manager;
	int image_count = manager_p->limit.display.image_count;
	char image_file_name[MAX_FILE_NAME_LENGTH];
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_black_pattern_for_vfos();
	}

	ret = reset_display_mode_for_dp086(dp086_p);
	if (ret < 0)
	{
		DERRPRINTF("reset_display_mode_for_dp086\n");
	}

	/* change current image number */
	dp086_p->cur_image_num--;
	if (dp086_p->cur_image_num <= 0)
	{
		dp086_p->cur_image_num = image_count;
	}

	/* parsing pattern mode from image file name */
	strcpy(&image_file_name[0],&dp086_info_p->display_image_file_name[dp086_p->cur_image_num - 1][0]);
	DPRINTF("cur_image_num=(%d),image_file_name=(%s)\n",dp086_p->cur_image_num,&image_file_name[0]);

	ret = parsing_pattern_mode(&image_file_name[0],&dp086_p->special_pattern_mode);
	if (ret < 0)
	{
		DERRPRINTF(" parsing_pattern_mode\n");
	}
	else
	{
		DPRINTF("Pattern mode=(0x%x)\n", dp086_p->special_pattern_mode.pattern_mode);

		/* display white pattern for only dimming thread */
		if (((dp086_p->special_pattern_mode.pattern_mode & DIMMING_MODE) == DIMMING_MODE) && ((dp086_p->special_pattern_mode.pattern_mode & DBV_MODE) == DBV_MODE))
		{
			display_white_pattern_for_vfos();
			usleep(200000);	/* 200ms delay - need to check if the delay is OK */
		}
	
		/* set special pattern code */
		ret = set_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("set_display_mode_for_dp086\n");
		}
	}

	/* set POCB if POCB channel 1 & 2 are not NO_READ, and pattern allows to write POCB status */
	if ((dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1] != POCB_STATUS_NO_READ) && (dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2] != POCB_STATUS_NO_READ))
	{
		/* initialize POCB status */
		init_pocb_status_for_dp086(dp086_p);
		if ((dp086_p->special_pattern_mode.pattern_mode & POCB_WRITE_ENABLE_MODE) == POCB_WRITE_ENABLE_MODE)
		{
			/* write POCB status to display module */
			update_pocb_status_for_dp086(dp086_p);
		}
	}

	/* display image - except gray scan and dimming and gradation */
	if (((dp086_p->special_pattern_mode.pattern_mode & GRAY_MODE) != GRAY_MODE) && (((dp086_p->special_pattern_mode.pattern_mode & DIMMING_MODE) != DIMMING_MODE) && ((dp086_p->special_pattern_mode.pattern_mode & DBV_MODE) != DBV_MODE)) && ((dp086_p->special_pattern_mode.pattern_mode & GRAD_MODE) != GRAD_MODE))
	{
		display_image_for_vfos(dp086_info_p->display_image_dir,image_file_name);
	}
	else if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_MODE) == GRAD_MODE)	/* display pattern for Gradation display */
	{
		if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_RGB_MODE) == GRAD_RGB_MODE)
		{
			if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_TOP_MODE) == HORIZONTAL_TOP_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_HT_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_DOWN_MODE) == HORIZONTAL_DOWN_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_HD_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_LEFT_MODE) == VERTICAL_LEFT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_VL_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_RIGHT_MODE) == VERTICAL_RIGHT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RGB_VR_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
		}
		else if ((dp086_p->special_pattern_mode.pattern_mode & GRAD_RED_MODE) == GRAD_RED_MODE)
		{
			if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_TOP_MODE) == HORIZONTAL_TOP_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_HT_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & HORIZONTAL_DOWN_MODE) == HORIZONTAL_DOWN_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_HD_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_LEFT_MODE) == VERTICAL_LEFT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_VL_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
			else if ((dp086_p->special_pattern_mode.pattern_mode & VERTICAL_RIGHT_MODE) == VERTICAL_RIGHT_MODE)
			{
				strcpy(pattern_num_str,GRAD_RED_VR_PATTERN_COMMAND_NUM_STRING);
				display_pattern_for_vfos(pattern_num_str);
			}
		}
	}

	/* If display off code was set for SLEEP PTN, need to set display on code */
	if (dp086_p->flag_need_to_display_on == true)
	{
		display_on_by_command_for_dp086();
		dp086_p->flag_need_to_display_on = false;
	}
	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	is_display_on_for_next_prev = 1;        // LWG 190328 need for interlock

	FUNC_END();
}

/*
 * Name : reset_key_action_for_dp086
 * Description : Key action for RESET key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void reset_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;

	FUNC_BEGIN();

	if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
	{
		ret = reset_display_mode_for_dp086(dp086_p);
		if (ret < 0)
		{
			DERRPRINTF("reset_display_mode_for_dp086\n");
		}
	}

	display_module_off_for_dp086(dp086_p,model_index);

	FUNC_END();
}

/*
 * Name : func2_key_action_for_dp086
 * Description : Key action for FUNC2(SET) key input.
 * Parameters :
 * 		model_dp086_t *dp086_p
 * Return value : 
 */
void func2_key_action_for_dp086(model_dp086_t *dp086_p)
{
	int ret = 0;
	int ch_cnt = 0;
	int model_index = dp086_p->model_dp086_info.buf_index + 1;
	unsigned char pocb_cur_status[DP086_POCB_WRITE_CHANNEL_NUM];
	unsigned char pocb_write[DP086_POCB_WRITE_CHANNEL_NUM][DP086_POCB_WRITE_LENGTH];

	FUNC_BEGIN();

	if (dp086_p->flag_need_to_init_module == true)
	{
		display_module_on_for_dp086(model_dp086_p,model_index);
		display_logo_for_vfos(model_index);
	}

	if (dp086_p->flag_need_to_pocb_write == true)
	{
		memcpy(pocb_cur_status,dp086_p->pocb_status.pocb_cur_status,sizeof(pocb_cur_status));
		for (ch_cnt = 0;ch_cnt < DP086_POCB_WRITE_CHANNEL_NUM;ch_cnt++)
		{
			/* XOR POCB status between POCB_STATUS_OFF and POCB_STATUS_ON */
			if (pocb_cur_status[ch_cnt] == POCB_STATUS_OFF)
			{
				pocb_cur_status[ch_cnt] = POCB_STATUS_ON;
			}
			else if (pocb_cur_status[ch_cnt] == POCB_STATUS_ON)
			{
				pocb_cur_status[ch_cnt] = POCB_STATUS_OFF;
			}
			else if (pocb_cur_status[ch_cnt] == POCB_STATUS_NO_READ)
			{
				pocb_cur_status[ch_cnt] = POCB_STATUS_OFF;
			}
			dp086_p->pocb_status.pocb_cur_status[ch_cnt] = pocb_cur_status[ch_cnt];
			DPRINTF("POCB current status(CH=%d) = (%d)\n", ch_cnt, pocb_cur_status[ch_cnt]);
		}

		/* write POCB */
		memset(pocb_write,0,sizeof(pocb_write));
		pocb_write[POCB_CHANNEL_1][0] = pocb_cur_status[POCB_CHANNEL_1];
		pocb_write[POCB_CHANNEL_2][0] = pocb_cur_status[POCB_CHANNEL_2];
		ret = pocb_write_for_dp086(pocb_write);
		if (ret < 0)
		{
			DERRPRINTF("pocb_write\n");
		}

		if (dp086_p->pocb_status.flag_pocb_changed == POCB_STATUS_NO_CHANGE)
		{
			dp086_p->pocb_status.flag_pocb_changed = POCB_STATUS_CHANGED;
		}
	}

	FUNC_END();
}

/*
 * Name : key_action_for_dp086
 * Description : Key action for each key input.
 * Parameters :
 * 		int key_value : input key value.
 * Return value : whether or not exit of thread
 */
int key_action_for_dp086(int key_value)
{
	int is_exit = 0;
	int pocb_write_enable = 0;
	model_dp086_t *dp086_p = model_dp086_p;
	model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;

	FUNC_BEGIN();

	DPRINTF("#######DP086_THREAD:key (%d) is pushed#########\n", key_value);
	//LWG 190329
#if 0
	if( flag_judge == 1){
		is_exit = false;
		dp086_p->last_key_value = key_value;
		return is_exit;
	}else 
#endif
	if(flag_password == 1){			// PASWORD INSERT
		// 1. RESET 키입력시 패스워드 체크
		if(key_value == RESET){
			int i;
			printf("\n PASSWORD CHECK\n");
			for(i=0;i<PW_LEN;i++){
				if(password[i] != pw_value[i]){
					printf("PASSWORD WRONG %d %d\n", password[i], pw_value[i]);
					flag_password = 0;
					pw_idx = -1;		// 초기화
					memset(pw_value, 0, PW_LEN); 
					return is_exit;		// 틀리면 나감
				}else{
					printf("PASSWORD RIGHT %d %d\n", password[i], pw_value[i]);
				}
			}
			flag_interlock = (flag_interlock == 0)?1:0;		// INTERLOCK 끔(토글)

		// 2. 맞으면 파일에다 변경내역 기록 (재부팅시 반영)
			{
				FILE *pFile =  fopen("/mnt/sd/initial/interlock.tty", "wt");	
				
				if(pFile != NULL){
					if(flag_interlock == 0){
						fwrite("DISABLE", sizeof(char), sizeof("DISABLE"), pFile);
						printf("INTERLOCK DISABLED : /mnt/sd/initial/interlock.tty\n");
					}else{
						fwrite("ENABLE", sizeof(char), sizeof("ENABLE"), pFile);
						printf("INTERLOCK ENABLED : /mnt/sd/initial/interlock.tty\n");
					}
					fclose(pFile);
				}
			}

			printf("PASSWORD OK : %d\n", flag_interlock);
			flag_password = 0;
			pw_idx = -1;		// 초기화
			//memset(pw_value, 0, PW_LEN);
			memset(pw_value, 0, PW_LEN*sizeof(int));
			{
				int i;
				printf("CHECK VALUE\n");
				for(i=0;i<PW_LEN;i++){
					printf("pw_value[%d] : %d\n", i, pw_value[i]);
				}
				printf("PW_LEN is %d\n",PW_LEN);
			}
			send_interlock_key_to_uart();

			system("sync");			// 반드시 필요
		
		// 3. 패스워드 길이 초과시 패스워드 맨끝에만 기록됨
		}else{
			pw_idx = (pw_idx < PW_LEN-1)? pw_idx+1 : pw_idx;
			
		// 4. RESET 키가 아니면 저장
			printf("key %d\n", key_value);	
			pw_value[pw_idx] = key_value;
			printf("pw_value[i] %d\n", pw_value[pw_idx]);
		}
		
		return is_exit;
	}

	if (key_value == OTP)
	{
		//if ((dp086_p -> flag_otp_test_result == 1) && (flag_interlock == 1))	return is_exit;		// 성공한 테스트는 재수행하지 않는다. 
		dp086_p->flag_need_to_init_module = true;	/* always initialize display module */

		otp_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_OTP_TEST_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;
	}
	else if (key_value == TOUCH)
	{
		//if ((dp086_p -> flag_touch_test_result == 1) && (flag_interlock == 1))    return is_exit;     // 성공한 테스트는 재수행하지 않는다.
		dp086_p->flag_need_to_init_module = true;	/* always initialize display module */

		touch_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_TOUCH_TEST_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;
	}
	else if (key_value == CURRENT)
	{
		//if ((dp086_p -> flag_current_test_result == 1) && (flag_interlock == 1))    return is_exit;     // 성공한 테스트는 재수행하지 않는다.
		dp086_p->flag_need_to_init_module = true;	/* always initialize display module */

		current_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_CURRENT_TEST_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;
	}
//	else if (key_value == FUNC)
	else if ((key_value == FUNC) && (flag_judge != 1))
	{
		dp086_p->flag_need_to_init_module = false;
		func_key_action_for_dp086(dp086_p);
		DPRINTF("#######DP086_THREAD:EXIT as (%d) is pushed#########\n", key_value);
		is_exit = true;
		dp086_p->last_key_value = key_value;
	}
	else if (key_value == NEXT)
	{
#if 1	
		// 세 테스트 통과하지 않고 NEXT 누르면(화상검사) interlock
		if(((dp086_p -> flag_otp_test_result_ch1 != 1) || (dp086_p -> flag_otp_test_result_ch2 != 1) 
			|| (dp086_p -> flag_touch_test_result_ch1 != 1) || (dp086_p -> flag_touch_test_result_ch2 != 1)
			|| (dp086_p -> flag_current_test_result_ch1 != 1) || (dp086_p -> flag_current_test_result_ch2 != 1))
			&& (flag_interlock)){
				printf("\n LWG judge\n");
				send_judge_status_to_uart(dp086_p -> flag_otp_test_result_ch1, dp086_p -> flag_otp_test_result_ch2,
								dp086_p -> flag_touch_test_result_ch1, dp086_p -> flag_touch_test_result_ch2,
								dp086_p -> flag_current_test_result_ch1, dp086_p -> flag_current_test_result_ch2);
				flag_judge = 1;
				return is_exit;
		// 모든 테스트를 통과하고 NEXT 누르면(화상검사) judge 플래그를 끈다. (재 테스트시 사용됨)
		}else if(((dp086_p -> flag_otp_test_result_ch1 == 1) && (dp086_p -> flag_otp_test_result_ch2 == 1) 
				&& (dp086_p -> flag_touch_test_result_ch1 == 1) && (dp086_p -> flag_touch_test_result_ch2 == 1)
				&& (dp086_p -> flag_current_test_result_ch1 == 1) && (dp086_p -> flag_current_test_result_ch2 == 1))
						&& (flag_interlock)){
				printf("\n LWG all pass\n");			
				flag_judge = 0;
		}
#endif
		if ((dp086_p->last_key_value == OTP) || (dp086_p->last_key_value == TOUCH) || (dp086_p->last_key_value == CURRENT)|| (dp086_p->last_key_value == FUNC)|| (dp086_p->last_key_value == RESET))
		{
			dp086_p->flag_need_to_init_module = true;
		}
		else	/* NEXT or PREV or FUNC2 */
		{
			dp086_p->flag_need_to_init_module = false;
		}

		next_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_DISPLAY_TEST_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;

		/* send display information to UI */
		send_display_info_to_uart_for_dp086(key_value,dp086_p);
		/* send POCB status to UI */
		if ((dp086_p->special_pattern_mode.pattern_mode & POCB_WRITE_ENABLE_MODE) == POCB_WRITE_ENABLE_MODE)
		{
			pocb_write_enable = true;
		}
		else
		{
			pocb_write_enable = false;
		}
		send_pocb_status_to_uart(dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_2],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2],pocb_write_enable);
	}
	else if (key_value == PREV)
	{
#if 1	
		// 세 테스트 통과하지 않고 PREV 누르면(화상검사) interlock
		if(((dp086_p -> flag_otp_test_result_ch1 != 1) || (dp086_p -> flag_otp_test_result_ch2 != 1) 
			|| (dp086_p -> flag_touch_test_result_ch1 != 1) || (dp086_p -> flag_touch_test_result_ch2 != 1)
			|| (dp086_p -> flag_current_test_result_ch1 != 1) || (dp086_p -> flag_current_test_result_ch2 != 1))
			&& (flag_interlock)){
				printf("\n LWG judge\n");
				send_judge_status_to_uart(dp086_p -> flag_otp_test_result_ch1, dp086_p -> flag_otp_test_result_ch2,
								dp086_p -> flag_touch_test_result_ch1, dp086_p -> flag_touch_test_result_ch2,
								dp086_p -> flag_current_test_result_ch1, dp086_p -> flag_current_test_result_ch2);
				flag_judge = 1;
				return is_exit;
		// 모든 테스트를 통과하고 NEXT 누르면(화상검사) judge 플래그를 끈다. (재 테스트시 사용됨)
		}else if(((dp086_p -> flag_otp_test_result_ch1 == 1) && (dp086_p -> flag_otp_test_result_ch2 == 1) 
				&& (dp086_p -> flag_touch_test_result_ch1 == 1) && (dp086_p -> flag_touch_test_result_ch2 == 1)
				&& (dp086_p -> flag_current_test_result_ch1 == 1) && (dp086_p -> flag_current_test_result_ch2 == 1))
						&& (flag_interlock)){
				printf("\n LWG all pass\n");			
				flag_judge = 0;
		}
#endif
		if ((dp086_p->last_key_value == OTP) || (dp086_p->last_key_value == TOUCH) || (dp086_p->last_key_value == CURRENT)|| (dp086_p->last_key_value == FUNC)|| (dp086_p->last_key_value == RESET))
		{
			dp086_p->flag_need_to_init_module = true;
		}
		else	/* NEXT or PREV or FUNC2 */
		{
			dp086_p->flag_need_to_init_module = false;
		}

		prev_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_DISPLAY_TEST_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;

		/* send display information to UI */
		send_display_info_to_uart_for_dp086(key_value,dp086_p);
		/* send POCB status to UI */
		if ((dp086_p->special_pattern_mode.pattern_mode & POCB_WRITE_ENABLE_MODE) == POCB_WRITE_ENABLE_MODE)
		{
			pocb_write_enable = true;
		}
		else
		{
			pocb_write_enable = false;
		}
		send_pocb_status_to_uart(dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_2],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2],pocb_write_enable);
	}
	else if (key_value == RESET)
	{
		printf("LWG %d %d, (%d %d) (%d %d) (%d %d)\n", 
						flag_interlock, flag_judge, 
						dp086_p -> flag_otp_test_result_ch1, dp086_p -> flag_otp_test_result_ch2,
						dp086_p -> flag_touch_test_result_ch1, dp086_p -> flag_touch_test_result_ch2, 
						dp086_p -> flag_current_test_result_ch1, dp086_p -> flag_current_test_result_ch2);
		if(flag_judge == 1){
			goto RESET;
		}else if((dp086_p->flag_need_to_init_module == false) && (dp086_p->flag_already_module_on == false) &&
			(is_display_on_for_next_prev == 0)){
				//printf("\n LWG interlock\n");
				//dp086_p->flag_interlock = ((dp086_p -> flag_display_test_result) == 0)?1:0;
				//flag_interlock = ((dp086_p -> flag_display_test_result) == 0)?1:0;
				//flag_interlock = (flag_interlock == 0)?1:0;
				//printf("LWG %d %d \n", flag_interlock, flag_judge);
				//send_interlock_key_to_uart();
			if(flag_password == 0){
				printf("\n ENTER PASSWORD\n");
				flag_password = 1;
			}
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
		// 디스플레이 켜져있는데 검사통과 못했는데, 인터락이 켜져있다면 judge
#if 0
		}else if(((dp086_p -> flag_otp_test_result != 1) || (dp086_p -> flag_touch_test_result != 1) ||
				(dp086_p -> flag_current_test_result != 1))
			//			&& (dp086_p -> flag_interlock)){
						&& (flag_interlock)){
				printf("\n LWG judge\n");
				//send_judge_key_to_uart();
				send_judge_status_to_uart(dp086_p -> flag_otp_test_result, dp086_p -> flag_touch_test_result,
								dp086_p -> flag_current_test_result);

				// 모든 키가 동작하지 않게 한다.
				flag_judge = 1;	
#endif	
		// 디스플레이 켜져있고 검사통과 했으면 reset
		}else{
RESET:
		// 검사통과 플래그 초기화
		dp086_p -> flag_otp_test_result_ch1 = 0;
		dp086_p -> flag_otp_test_result_ch2 = 0;
		dp086_p -> flag_touch_test_result_ch1 = 0;
		dp086_p -> flag_touch_test_result_ch2 = 0;
		dp086_p -> flag_current_test_result_ch1 = 0;
		dp086_p -> flag_current_test_result_ch2 = 0;
		flag_judge = 0;		

		is_display_on_for_next_prev = 0;
		dp086_p->flag_need_to_init_module = false;

		/* reset key action */
		reset_key_action_for_dp086(dp086_p);

		dp086_p->cur_test_mode = VFOS_RESET_MODE;
		is_exit = false;
		dp086_p->last_key_value = key_value;

		send_reset_key_to_uart();
		send_func_key_to_uart(&dp086_info_p->version_info,dp086_info_p->model_dp086_id);
		
		//flag_interlock = 1;		//	RESET need to interlock flag

		/* Initialize variables to make init condition */
		init_variable_for_dp086(dp086_p);
		}
	}
//	else if (key_value == FUNC2)
	else if ((key_value == FUNC2) && flag_judge != 1)
	{
		/* check if display module initialization is needed */
		if ((dp086_p->last_key_value == OTP) || (dp086_p->last_key_value == TOUCH) || (dp086_p->last_key_value == CURRENT) || (dp086_p->last_key_value == NEXT) || (dp086_p->last_key_value == PREV))
		{
			dp086_p->flag_need_to_init_module = false;
		}
		else	/* FUNC or RESET or FUNC2 */
		{
			if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
			{
				dp086_p->flag_need_to_init_module = false;
			}
			else
			{
				dp086_p->flag_need_to_init_module = true;
			}
		}

		/* check if POCB write is needed */
//		if ((dp086_p->last_key_value == NEXT) || (dp086_p->last_key_value == PREV))
		if (dp086_p->cur_test_mode == VFOS_DISPLAY_TEST_MODE)
		{
			if ((dp086_p->special_pattern_mode.pattern_mode & POCB_WRITE_ENABLE_MODE) == POCB_WRITE_ENABLE_MODE)
			{
				dp086_p->flag_need_to_pocb_write = true;
			}
			else
			{
				dp086_p->flag_need_to_pocb_write = false;
			}
		}
		else	/* OTP or TOUCH or CURRENT or FUNC or RESET */
		{
			dp086_p->flag_need_to_pocb_write = false;
		}

		/* run FUNC2 key action */
		func2_key_action_for_dp086(dp086_p);
		/* send info to UI */
		if (dp086_p->flag_need_to_init_module == true)
		{
			send_vfos_touch_version_to_uart(&dp086_p->model_dp086_info.version_info,dp086_p->model_dp086_info.model_dp086_id);
			send_vfos_display_version_to_uart(&dp086_p->model_dp086_info.version_info,dp086_p->model_dp086_info.buf_index+1);
		}
		if (dp086_p->flag_need_to_pocb_write == true)
		{
			pocb_write_enable = true;
			send_pocb_status_to_uart(dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_cur_status[POCB_CHANNEL_2],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_1],dp086_p->pocb_status.pocb_init_status[POCB_CHANNEL_2],pocb_write_enable);
		}
		is_exit = false;
		dp086_p->last_key_value = key_value;
	}
	else
	{
		DERRPRINTF("#######DP086_THREAD:Other key is pushed(key=%d)#########\n", key_value);
		is_exit = false;
		dp086_p->last_key_value = key_value;
	}

	FUNC_END();

	return is_exit;
}

/*
 * Name : dp086_thread
 * Description : Thread for model DP086.
 * Parameters :
 * 		void *arg : arguments for dp086_thread.
 * Return value : NULL
 */
void *dp086_thread(void *arg)
{
	int is_exit = 0;
	int thread_loop = 0;
	int key_value = 0;
	int ch_cnt = 0;
	model_dp086_t *dp086_p = model_dp086_p;
	model_dp086_info_t *dp086_info_p = &dp086_p->model_dp086_info;
//	unsigned char uart_buf[MAX_PACKET];
	pthread_mutex_t	*mutex_p = &dp086_p->dp086_thread_mutex;

	FUNC_BEGIN();

	pthread_mutex_lock(mutex_p);
	get_info_for_dp086_thread();

	DPRINTF("######### Start DP086 thread ###########\n");

	/* variable set as default value */
	thread_loop = true;
	dp086_p->cur_image_num = 0;
	for (ch_cnt = 0;ch_cnt < DP086_POCB_WRITE_CHANNEL_NUM;ch_cnt++)
	{
		dp086_p->pocb_status.pocb_init_status[ch_cnt] = POCB_STATUS_NO_READ;
		dp086_p->pocb_status.pocb_cur_status[ch_cnt] = POCB_STATUS_NO_READ;
	}
	dp086_p->pocb_status.flag_pocb_changed = POCB_STATUS_NO_CHANGE;
//goto DP086;

	/* start loop */
	while (thread_loop)
	{
		key_value = read_key_input_for_dp086();
		if (key_value > 0)
		{
//DP086:
//key_value = TOUCH;
			is_exit = key_action_for_dp086(key_value);
			if (is_exit == true)
			{
				thread_loop = false;
			}
		}
		usleep(50000);	/* 50ms delay for loop */
	}

	/* send information to UI */
	send_func_key_to_uart(&dp086_info_p->version_info,dp086_info_p->next_model_id);

	pthread_mutex_unlock(mutex_p);

	FUNC_END();
	return NULL;
}

/*
 * Name : init_model_dp086
 * Description : Initialize for model DP086, called by main() function.
 * Parameters :
 * Return value : error code
 */
int init_model_dp086(void)
{
	int ret = 0;

	FUNC_BEGIN();

	model_dp086_p = malloc(sizeof(model_dp086_t));
	if (model_dp086_p == NULL)
	{
		DERRPRINTF(" Allocate memory for model_dp086_p\n");
		FUNC_END();
		return -1;
	}

	/* initialize mutex */
	ret = pthread_mutex_init(&model_dp086_p->dp086_thread_mutex,NULL);
	if (ret < 0)
	{
		DERRPRINTF(" pthread_mutex_init(errno=%d)\n", errno);
		FUNC_END();
		return ret;
	}

	FUNC_END();
	return 0;
}

/*
 * Name : release_model_dp086
 * Description : Release for model DP086, called by main() function.
 * Parameters :
 * Return value : error code
 */
int release_model_dp086(void)
{
	FUNC_BEGIN();

	free(model_dp086_p);

	FUNC_END();
	return 0;
}

/*
 * Name : start_dp086_thread
 * Description : Create pthread and join, called by main() function.
 * Parameters :
 * Return value :
 */
void start_dp086_thread(void)
{
	int ret = 0;

	FUNC_BEGIN();

	ret = pthread_create(&model_dp086_p->id_dp086_thread, NULL, dp086_thread, (void *)(model_dp086_p));
	if (ret < 0)
	{
		DERRPRINTF(" pthread_create(errno=%d)\n", errno);
		FUNC_END();
	}
	else
	{
		pthread_join(model_dp086_p->id_dp086_thread,NULL);
	}

	FUNC_END();
}

/*
 * Name : get_last_key_value_for_dp086
 * Description : Return last key value, called by main() function.
 * Parameters :
 * Return value : latest key value
 */
int get_last_key_value_for_dp086(void)
{
	int last_key_value = 0;
	pthread_mutex_t	*mutex_p = &model_dp086_p->dp086_thread_mutex;

	FUNC_BEGIN();

	pthread_mutex_lock(mutex_p);
	last_key_value = model_dp086_p->last_key_value;
	pthread_mutex_unlock(mutex_p);

	FUNC_END();
	return last_key_value;
}

void judge_interlock_current_for_dp086(model_dp086_t *dp086_p){ 
	int ch_cnt;
	for(ch_cnt=0;ch_cnt<DP086_CURRENT_TEST_CHANNEL_NUM;ch_cnt++){
		if (
			((dp086_p->current_test_result)[ch_cnt][VCC1].is_over_limit & 0xff) |
			((dp086_p->current_test_result)[ch_cnt][VCC2].is_over_limit & 0xff) |
			((dp086_p->current_test_result)[ch_cnt][VDDVDH].is_over_limit & 0xff) |
			((dp086_p->current_test_result)[ch_cnt][VDDEL].is_over_limit & 0xff) |
			((dp086_p->current_test_result)[ch_cnt][TTL].is_over_limit & 0xff)
		){
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			if(ch_cnt == 0){
				printf("CH1 FAIL\n");
				dp086_p -> flag_current_test_result_ch1 = 2;		// FAIL
			}else{
				dp086_p -> flag_current_test_result_ch2 = 2;		// FAIL
			}
		//}else if((dp086_p -> flag_current_test_result) != 2){
		}else if(
			!((dp086_p->current_test_result)[ch_cnt][VCC1].is_over_limit & 0xff) &
			!((dp086_p->current_test_result)[ch_cnt][VCC2].is_over_limit & 0xff) &
			!((dp086_p->current_test_result)[ch_cnt][VDDVDH].is_over_limit & 0xff) &
			!((dp086_p->current_test_result)[ch_cnt][VDDEL].is_over_limit & 0xff) &
			!((dp086_p->current_test_result)[ch_cnt][TTL].is_over_limit & 0xff)
		){
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			if(ch_cnt == 0){
				if(dp086_p -> flag_current_test_result_ch1 != 2){
					printf("CH1 SUCCESS\n");
					dp086_p -> flag_current_test_result_ch1 = 1;		// SUCCESS
				}
			}else{
				if(dp086_p -> flag_current_test_result_ch2 != 2){
					dp086_p -> flag_current_test_result_ch2 = 1;		// SUCCESS
				}
			}
		}
	}
}
