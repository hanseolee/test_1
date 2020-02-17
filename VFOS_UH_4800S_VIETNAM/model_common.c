/*
 * model_common.c
 * This is for common functions for new models since STORM.
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
#include <stm_touch.h>
#include <mipi_con.h>
#include <mipi_dsi.h>
#include <model_common.h>

int is_display_on_for_next_prev = 0;
int flag_interlock = 1;		// 시작시 interlock 걸린상태로 수행, 단, /mnt/sd/interlock 파일을 읽어서 ENABLE인지 DISABLE인지 추가로 체크
int flag_judge = 0;
int flag_password = 0;

//int password[] = { OTP, PREV, CURRENT, OTP, FUNC, TOUCH, NEXT, PREV, FUNC, CURRENT };
int password[10] = { CURRENT, TOUCH, NEXT, CURRENT, OTP, TOUCH, PREV, NEXT, TOUCH, CURRENT };		// LWG 191104 CAN BE CHANGE USING password.tty
int pw_value[PW_LEN];
int pw_idx = -1;

/*
 * Name : send_vfos_display_version_to_uart
 * Description : Send VFOS display version information to UI AP through UART.
 * Parameters : 
 * 		vfos_version_info_t *info_p : version information.
 * 		int model_index
 * Return value :
 */

void send_vfos_display_version_to_uart(vfos_version_info_t *info_p, int model_index)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[4] = model_index;
	uart_buf[7] = info_p->display_version[7];
	uart_buf[8] = info_p->display_version[6];
	uart_buf[9] = info_p->display_version[5];
	uart_buf[10] = info_p->display_version[4];
	uart_buf[11] = info_p->display_version[3];
	uart_buf[12] = info_p->display_version[2];
	uart_buf[13] = info_p->display_version[1];
	uart_buf[14] = info_p->display_version[0];
	serial_packet_init(uart_buf, FUNC2,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_vfos_touch_version_to_uart
 * Description : Send VFOS touch version information to UI AP through UART.
 * Parameters : 
 * 		vfos_version_info_t *info_p : version information.
 * 		int model_index
 * Return value :
 */

void send_vfos_touch_version_to_uart(vfos_version_info_t *info_p, int model_id)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[VER_INFO_MODEL_ID_BUF_NUM] = model_id;
	uart_buf[VER_INFO_VFOS_VER_BUF_NUM] = VFOS_VER;
	uart_buf[VER_INFO_VFOS_REV_BUF_NUM] = VFOS_REV;
	uart_buf[VER_INFO_VFOS_REV_MINOR_BUF_NUM] = VFOS_REV_MINOR;
	uart_buf[VER_INFO_TOUCH_VER_BYTE_7_BUF_NUM] = info_p->touch_version[7];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_6_BUF_NUM] = info_p->touch_version[6];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_5_BUF_NUM] = info_p->touch_version[5];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_4_BUF_NUM] = info_p->touch_version[4];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_3_BUF_NUM] = info_p->touch_version[3];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_2_BUF_NUM] = info_p->touch_version[2];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_1_BUF_NUM] = info_p->touch_version[1];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_0_BUF_NUM] = info_p->touch_version[0];
	uart_buf[VER_INFO_SITE_VER_BUF_NUM] = info_p->site_version;

	serial_packet_init(uart_buf, FUNC,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_touch_test_result_to_uart
 * Description : Send TOUCH test result to UI AP through UART.
 * Parameters :
 * 		unsigned int touch_test_result : TOUCH test result.
 * 		int hf_test_on : HF_TEST ON/OFF.
 * 		int ch_num : channel number.
 * Return value :
 */
void send_touch_test_result_to_uart(unsigned int touch_test_result, int hf_test_on, int ch_num)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[4] = touch_test_result & 0xff;
	uart_buf[5] = (touch_test_result >> 8) & 0xff;
	uart_buf[6] = (touch_test_result >> 16) & 0xff;
	uart_buf[7] = (touch_test_result >> 24) & 0xff;
	uart_buf[10] = hf_test_on;
	serial_packet_init(uart_buf,TOUCH,ch_num+1);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_touch_test_error_to_uart
 * Description : Send TOUCH test error to UI AP through UART.
 * Parameters :
 * 		int hf_test_on : HF_TEST ON/OFF.
 * 		int ch_num : channel number.
 * Return value :
 */
void send_touch_test_error_to_uart(int hf_test_on, int ch_num)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[4] = 1 << 0;	/* TODO : need to set the define for synaptics touch, just add code to build */
	uart_buf[10] = hf_test_on;
	serial_packet_init(uart_buf,TOUCH,ch_num+1);
	serial_write_function(uart_buf);

	FUNC_END();
}
void send_pocb_status_to_uart_for_f2(unsigned char ch1_pocb_cur_status,unsigned char ch2_pocb_cur_status,unsigned char ch1_pocb_init_status,unsigned char ch2_pocb_init_status,unsigned int pocb_write_enable, unsigned char hz_status)
{
    unsigned char uart_buf[MAX_PACKET];

    FUNC_BEGIN();

    printf("        >>> SSW <<< [%s %d] %s CALL ====== 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X hz = %d\n", __FILE__, __LINE__, __FUNCTION__,ch1_pocb_cur_status,ch2_pocb_cur_status,ch1_pocb_init_status,ch2_pocb_init_status, pocb_write_enable, hz_status);

    /* uart command */
    memset(uart_buf, 0, MAX_PACKET);
    uart_buf[5] = ch1_pocb_cur_status;
    uart_buf[6] = ch2_pocb_cur_status; 
    uart_buf[7] = ch1_pocb_init_status;
    uart_buf[8] = ch2_pocb_init_status; 
    uart_buf[9] = pocb_write_enable; 
    uart_buf[10] = 1;  //POCB INFO
    uart_buf[11] = hz_status;
    serial_packet_init(uart_buf, NEXT,0x00);
    serial_write_function(uart_buf);

    FUNC_END();
}

void send_judge_status_to_uart(int flag_otp_test_result_ch1, int flag_otp_test_result_ch2, int flag_touch_test_result_ch1, int flag_touch_test_result_ch2, int flag_current_test_result_ch1, int flag_current_test_result_ch2){
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();
	
    printf("        >>> LWG <<< [%s %d] %s CALL ====== (%d %d) (%d %d) (%d %d)\n", __FILE__, __LINE__, __FUNCTION__,
					flag_otp_test_result_ch1, flag_otp_test_result_ch2,
					flag_touch_test_result_ch1, flag_touch_test_result_ch2,
					flag_current_test_result_ch1, flag_current_test_result_ch2);
    /* uart command */
    memset(uart_buf, 0, MAX_PACKET);
    uart_buf[5] = flag_otp_test_result_ch1;
    uart_buf[6] = flag_otp_test_result_ch2;
    uart_buf[7] = flag_touch_test_result_ch1; 
    uart_buf[8] = flag_touch_test_result_ch2; 
    uart_buf[9] = flag_current_test_result_ch1;
    uart_buf[10] = flag_current_test_result_ch2;
    serial_packet_init(uart_buf, JUDGE,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_pocb_status_to_uart
 * Description : Send POCB status information to UI AP through UART.
 * Parameters : 
 * 		unsigned char ch1_pocb_cur_status : current POCB status for CH1.
 * 		unsigned char ch2_pocb_cur_status : current POCB status for CH2.
 * 		unsigned char ch1_pocb_init_status : current POCB status for CH1.
 * 		unsigned char ch2_pocb_init_status : current POCB status for CH2.
 * 		unsigned int pocb_write_enable : Whether or not POCB write is enabled.
 * Return value :
 */
void send_pocb_status_to_uart(unsigned char ch1_pocb_cur_status,unsigned char ch2_pocb_cur_status,unsigned char ch1_pocb_init_status,unsigned char ch2_pocb_init_status,unsigned int pocb_write_enable)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

    printf("        >>> SSW <<< [%s %d] %s CALL ====== 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", __FILE__, __LINE__, __FUNCTION__,ch1_pocb_cur_status,ch2_pocb_cur_status,ch1_pocb_init_status,ch2_pocb_init_status, pocb_write_enable);
	
	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	uart_buf[5] = ch1_pocb_cur_status;
	uart_buf[6] = ch2_pocb_cur_status; 
	uart_buf[7] = ch1_pocb_init_status;
	uart_buf[8] = ch2_pocb_init_status; 
	uart_buf[9] = pocb_write_enable; 
	uart_buf[10] = 1;  //POCB INFO
	serial_packet_init(uart_buf, NEXT,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_function_start_info_to_uart
 * Description : Send information which new function is entered to UI AP through UART.
 * Parameters : 
 * 		int key_value : Key value to send.
 * Return value :
 */
void send_function_start_info_to_uart(int key_value)
{
	unsigned char uart_buf[MAX_PACKET];

	FUNC_BEGIN();

	/* uart command */
	memset(uart_buf, 0, MAX_PACKET);
	serial_packet_init(uart_buf, key_value,0x00);
	serial_write_function(uart_buf);

	FUNC_END();
}

/*
 * Name : send_otp_key_to_uart
 * Description : Send information which OTP key is entered to UI AP through UART.
 * Parameters : 
 * 		int channel_num : channel number.
 *		unsigned char otp_val : OTP value.
 *		int otp_max_write_time : Max time to write OTP. 
 *		unsigned char pocb_status : Status of POCB (ON/OFF/NO_READ)
 *		unsigned char additional_info : Additional information if needed.
 * Return value :
 */
void send_otp_key_to_uart(int channel_num, unsigned char otp_val, int otp_max_write_time, unsigned char pocb_status, unsigned char additional_info)
{
	FUNC_BEGIN();

	/* Call function which is used for all models */
	OTP_uart_write(channel_num,otp_val,otp_max_write_time,pocb_status,additional_info);

	FUNC_END();
}

/*
 * Name : send_reset_key_to_uart
 * Description : Send information which RESET key is entered to UI AP through UART.
 * Parameters : 
 * Return value :
 */
void send_reset_key_to_uart(void)
{
	FUNC_BEGIN();

	/* uart command */
	send_function_start_info_to_uart(RESET);
	sleep(4);

	FUNC_END();
}

/*
 ** Name : send_interlock_key_to_uart
 ** Description : Send information which RESET(INTERLOCK) key is entered to UI AP through UART.
 ** Parameters : 
 ** Return value :
 **/
void send_interlock_key_to_uart(void)
{
		FUNC_BEGIN();

		/* uart command */
		send_function_start_info_to_uart(INTERLOCK);

		FUNC_END();
}

/*
 ** Name : send_judge_key_to_uart
 ** Description : Send information which RESET(JUDGE) key is entered to UI AP through UART.
 ** Parameters : 
 ** Return value :
 **/
void send_judge_key_to_uart(void)
{
		FUNC_BEGIN();

		/* uart command */
		send_function_start_info_to_uart(JUDGE);

		FUNC_END();
}

/*
 ** Name : send_toggle_synaptics_to_uart
 ** Description : Send information which use_synaptics touch ui is entered to UI AP through UART.
 ** Parameters : 
 ** Return value :
 **/
void send_toggle_synaptics_to_uart(void)
{
		FUNC_BEGIN();

		/* uart command */
		send_function_start_info_to_uart(SYNAPTICS_TOUCH_UI);

		FUNC_END();
}

/*
 * Name : send_func_key_to_uart
 * Description : Send information which FUNC key is entered to UI AP through UART.
 * Parameters :
 * 		vfos_version_info_t *info_p.
 * 		int model_id : model id.
 * Return value :
 */
void send_func_key_to_uart(vfos_version_info_t *info_p, int model_id)
{
	FUNC_BEGIN();

	/* uart command */
	send_vfos_touch_version_to_uart(info_p,model_id);

	FUNC_END();
}

/*
 * Name : parsing_pattern_mode
 * Description : Parsing pattern mode from picture name.
 * Parameters :
 * 		char *picture_name_p
 * 		special_pattern_mode_t *pattern_mode_p
 * Return value :
 * 		error value
 */

extern int cur_model_id;
extern int next_model_id;

int parsing_pattern_start_end_mode(char *picture_name_p, special_pattern_mode_t *pattern_mode_p)
{
	int ret = 0;
    char pic_name[MAX_FILE_NAME_LENGTH];
    char *token;
	int pic_num = 0;
	special_pattern_mode_t mode;

	FUNC_BEGIN();

	memset(&mode,0,sizeof(special_pattern_mode_t));
	memset(pic_name, 0, sizeof(pic_name));
	strcpy(pic_name, picture_name_p);
	token = strtok(pic_name, TOKEN_SEP_UNDER_POINT);
	pic_num = strtoul(token,NULL,PARSING_BY_DECIMAL);

	/* POCB write enable is default */
	mode.pattern_mode = NORMAL_MODE;
	mode.pattern_mode |= POCB_WRITE_ENABLE_MODE;

	while(token != NULL)
	{
		if((!strcmp(token, STRING_START_CAP)) || (!strcmp(token, STRING_START_SMALL))){
			mode.pattern_mode |= START_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			break;
		}
        else if((!strcmp(token, STRING_END_CAP)) || (!strcmp(token, STRING_END_SMALL)))
        {
            mode.pattern_mode |= END_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			break;
        }
	    token = strtok(NULL, TOKEN_SEP_UNDER_POINT);
	}

	DPRINTF("Special_Pattern_mode=(0x%08x)\n", mode.pattern_mode);
	memcpy(pattern_mode_p,&mode,sizeof(special_pattern_mode_t));
	FUNC_END();

	return ret;
}




int parsing_pattern_mode(char *picture_name_p, special_pattern_mode_t *pattern_mode_p)
{
	int ret = 0;
    char pic_name[MAX_FILE_NAME_LENGTH];
    char *token;
	int pic_num = 0;
	special_pattern_mode_t mode;

	FUNC_BEGIN();

	memset(&mode,0,sizeof(special_pattern_mode_t));
	memset(pic_name, 0, sizeof(pic_name));
	strcpy(pic_name, picture_name_p);
	token = strtok(pic_name, TOKEN_SEP_UNDER_POINT);
	pic_num = strtoul(token,NULL,PARSING_BY_DECIMAL);

	/* POCB write enable is default */
	mode.pattern_mode = NORMAL_MODE;
	mode.pattern_mode |= POCB_WRITE_ENABLE_MODE;

	while(token != NULL)
	{
		if((!strcmp(token, STRING_SCAN_SMALL)) || (!strcmp(token, STRING_SCAN_CAP_SMALL))){
			mode.pattern_mode |= GRAY_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("GRAY_SCAN FILE\n");
			break;
		}
	    else if((!strcmp(token, STRING_FBRIGHTLINE_SMALL)) || (!strcmp(token, STRING_FBRIGHTLINE_CAP))){
			mode.pattern_mode |= POWER_BRIGHT_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("POWER BRIGHT LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_FBLACKLINE_SMALL)) || (!strcmp(token, STRING_FBLACKLINE_CAP))){
			mode.pattern_mode |= POWER_BLACK_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("POWER BLACK LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_FBRIGHTLINE50_SMALL)) || (!strcmp(token, STRING_FBRIGHTLINE50_CAP))){
			mode.pattern_mode |= LU_50P_POWER_BRIGHT_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("50Per LUMINANCE POWER BRIGHT LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_FBLACKLINE50_SMALL)) || (!strcmp(token, STRING_FBLACKLINE50_CAP))){
			mode.pattern_mode |= LU_50P_POWER_BLACK_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("50Per LUMINANCE POWER BLACK LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_BLACKPOINT_SMALL)) || (!strcmp(token, STRING_BLACKPOINT_CAP))){
			mode.pattern_mode |= BLACKPOINT_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("BLACK_POINT FILE\n");
	    }
	    else if((!strcmp(token, STRING_BRIGHTLINE_SMALL)) || (!strcmp(token, STRING_BRIGHTLINE_CAP))){
			mode.pattern_mode |= BRIGHT_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("BRIGHT LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_BLACKLINE_SMALL)) || (!strcmp(token, STRING_BLACKLINE_CAP))){
			mode.pattern_mode |= BLACK_LINE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("BLACK LINE FILE\n");
	    }
	    else if((!strcmp(token, STRING_VARIABLE_SMALL)) || (!strcmp(token, STRING_VARIABLE_CAP))){
			mode.pattern_mode |= VARIABLE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("VARIABLE FILE\n");
		}
		else if((!strcmp(token, STRING_EMCONPWM_SMALL)) || (!strcmp(token, STRING_EMCONPWM_CAP))){
			mode.pattern_mode |= EMCONTROL_IN_PWM_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("EM CONTROL..IN PWM FILE\n");
	    }
	    else if((!strcmp(token, STRING_EMCON_SMALL)) || (!strcmp(token, STRING_EMCON_CAP))){
			mode.pattern_mode |= EMCONTROL_NO_PWM_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("EM CONTROL..NOT PWM FILE\n");
	    }
	    else if((!strcmp(token, STRING_DIM_SMALL)) || (!strcmp(token, STRING_DIM_CAP_SMALL))){
			mode.pattern_mode |= DIMMING_MODE;
			mode.pattern_mode |= DBV_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("DIMMING FILE\n");
	    }
		else if((!strcmp(token,STRING_DBV10_SMALL)) || (!strcmp(token,STRING_DBV10_CAP))){
			mode.pattern_mode &= (~DIMMING_MODE);
			mode.pattern_mode |= DBV_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			mode.dbv_pattern_mode.dbv_level = 10;
			DPRINTF("DBV FILE\n");
		}
	    else if((!strcmp(token, STRING_AOD_SMALL)) || (!strcmp(token, STRING_AOD_CAP))){
			mode.pattern_mode |= AOD_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("AOD FILE\n");
	    }
	    else if((!strcmp(token, STRING_VR_SMALL)) || (!strcmp(token, STRING_VR_CAP))){
			mode.pattern_mode |= VR_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("VR FILE\n");
	    }
	    else if((!strcmp(token, STRING_SLEEP_SMALL)) || (!strcmp(token, STRING_SLEEP_CAP_SMALL))){
			mode.pattern_mode |= SLEEP_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("SLEEP FILE\n");
	    }
	    else if((!strcmp(token, STRING_DSC_SMALL)) || (!strcmp(token, STRING_DSC_CAP))){
			mode.pattern_mode |= DSC_ROLL_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			mode.dsc_pattern_mode.dsc_roll_pic_num = pic_num;
			sprintf(mode.dsc_pattern_mode.dsc_roll_name_string,"%s",strtok(NULL,"."));
			DPRINTF("dsc_roll_mode disc -> [%s] \n",mode.dsc_pattern_mode.dsc_roll_name_string); 
			DPRINTF("DSC ROLL FILE\n");
	    }
	    else if((!strcmp(token, STRING_BORDERTEST_SMALL)) || (!strcmp(token, STRING_BORDERTEST_CAP))){
			mode.pattern_mode |= BDTEST_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("BORDER TEST FILE\n");
	    }
	    else if((!strcmp(token, STRING_GRAD_SMALL)) || (!strcmp(token, STRING_GRAD_CAP)))
	    {
			mode.pattern_mode |= GRAD_MODE;
			mode.pattern_mode |= POCB_WRITE_ENABLE_MODE;
			DPRINTF("GRAD FILE\n");
	    }
		else if((!strcmp(token, STRING_RGB_SMALL)) || (!strcmp(token, STRING_RGB_CAP)))
		{
			/* parsing much deeper to get RGB or RED for Gradation file */
			mode.pattern_mode |= GRAD_RGB_MODE;
			mode.pattern_mode &= (~GRAD_RED_MODE);
			DPRINTF("GRAD RGB\n");
		}
		else if((!strcmp(token, STRING_RED_SMALL)) || (!strcmp(token, STRING_RED_CAP)))
		{
			/* parsing much deeper to get RGB or RED for Gradation file */
			mode.pattern_mode |= GRAD_RED_MODE;
			mode.pattern_mode &= (~GRAD_RGB_MODE);
			DPRINTF("GRAD RED\n");
		}
		else if((!strcmp(token, STRING_DBVVARIABLE_SMALL)) || (!strcmp(token, STRING_DBVVARIABLE_CAP)))
		{
			mode.pattern_mode |= DBV_VARIABLE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("DBV VARIABLE FILE\n");
		}
		else if((!strcmp(token, STRING_BLACKPOINT_VARIABLE_SMALL)) || (!strcmp(token, STRING_BLACKPOINT_VARIABLE_CAP)))
		{
			mode.pattern_mode |= BLACKPOINT_VARIABLE_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("BLACKPOINT VARIABLE FILE\n");
		}
		else if((!strcmp(token, STRING_DBV_4TH_NIT_SMALL)) || (!strcmp(token, STRING_DBV_4TH_NIT_CAP)))
		{
			mode.pattern_mode |= DBV_4NIT_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("DBV 4NIT FILE\n");
		}
        //181226 32bit -> 64bit
        else if((!strcmp(token, STRING_DBV_1ST_NIT_SMALL)) || (!strcmp(token, STRING_DBV_1ST_NIT_CAP)))
        {
            mode.pattern_mode |= DBV_1ST_NIT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}
            DPRINTF("DBV 1ST NIT FILE\n");
        }
        else if((!strcmp(token, STRING_DBV_2ND_NIT_SMALL)) || (!strcmp(token, STRING_DBV_2ND_NIT_CAP)))
        {
            mode.pattern_mode |= DBV_2ND_NIT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("DBV 2ND NIT FILE\n");
        }
        else if((!strcmp(token, STRING_DBV_3RD_NIT_SMALL)) || (!strcmp(token, STRING_DBV_3RD_NIT_CAP)))
        {
            mode.pattern_mode |= DBV_3RD_NIT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}
            DPRINTF("DBV 3RD NIT FILE\n");
        }
        //190104
	    else if((!strcmp(token, STRING_HBM_SMALL)) || (!strcmp(token, STRING_HBM_CAP)))
        {
            mode.pattern_mode |= HBM_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}
            DPRINTF("HBM FILE\n");
        }
        //190109 only storm model
        else if((!strcmp(token, STRING_WHITE_BOX_SMALL)) || (!strcmp(token, STRING_WHITE_BOX_CAP)))
        {
            mode.pattern_mode |= WHITE_BOX_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("WHITE_BOX FILE\n");
        }
        //190116 only dp076 model
        else if((!strcmp(token, STRING_DARKMURA_SMALL)) || (!strcmp(token, STRING_DARKMURA_CAP)))
        {
            mode.pattern_mode |= DARKMURA_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("DARKMURA FILE\n");
        }
		// 190322 only alpha model
        else if((!strcmp(token, STRING_ALP1_SMALL)) || (!strcmp(token, STRING_ALP1_CAP)))
        {
            mode.pattern_mode |= ALP1_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("ALP1 FILE\n");
        }
        else if((!strcmp(token, STRING_ALP2_SMALL)) || (!strcmp(token, STRING_ALP2_CAP)))
        {
            mode.pattern_mode |= ALP2_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("ALP2 FILE\n");
        }
		// 190325 only dp086 model
        else if((!strcmp(token, STRING_VOLTAGE_SMALL)) || (!strcmp(token, STRING_VOLTAGE_CAP)))
        {
            mode.pattern_mode |= VOLTAGE_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
            DPRINTF("VOLTAGE FILE\n");
        }
		// 190715 only f2 model..(f2_config.tty)
		// 190813 storm + dp116 use HL_L
		else if((!strcmp(token, STRING_HL_L_SMALL)) || (!strcmp(token, STRING_HL_L_CAP)))
		{
			mode.pattern_mode |= HL_L_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("HL_L FILE\n");
		}
		// 190726 only dp116 model..(dp116_config.tty)
		else if((!strcmp(token, STRING_VL_L_SMALL)) || (!strcmp(token, STRING_VL_L_CAP)))
		{
			mode.pattern_mode |= VL_L_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("VL_L FILE\n");
		}
		// 190902 only dp116 model..(dp116_config.tty)
		else if((!strcmp(token, STRING_L_EM_SMALL)) || (!strcmp(token, STRING_L_EM_CAP)))
		{
			mode.pattern_mode |= L_EM_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("L_EM FILE\n");
		}
		// 190902 only dp116 model..(dp116_config.tty)
		else if((!strcmp(token, STRING_R_EM_SMALL)) || (!strcmp(token, STRING_R_EM_CAP)))
		{
			mode.pattern_mode |= R_EM_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			DPRINTF("R_EM FILE\n");
		}
		// 191026 only dp150 model..(dp150_config.tty)
		else if((!strcmp(token, STRING_60HZ_SMALL)) || (!strcmp(token, STRING_60HZ_CAP)))
		{
			mode.pattern_mode |= HZ_60_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}

			DPRINTF("HZ_60 FILE\n");
		}
		// 191026 only dp150 model..(dp150_config.tty)
		else if((!strcmp(token, STRING_90HZ_SMALL)) || (!strcmp(token, STRING_90HZ_CAP)))
		{
			mode.pattern_mode |= HZ_90_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}

			DPRINTF("HZ_90 FILE\n");
		}
		// 191102 dp150 new DBV (180nit)
		else if((!strcmp(token, STRING_DBV_4RD_NIT_SMALL)) || (!strcmp(token, STRING_DBV_4RD_NIT_CAP)))
        {
            mode.pattern_mode |= DBV_4RD_NIT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}

            DPRINTF("DBV 4RD NIT FILE\n");
        }
		// 191102 dp150 new DBV (90nit)
		else if((!strcmp(token, STRING_DBV_5RD_NIT_SMALL)) || (!strcmp(token, STRING_DBV_5RD_NIT_CAP)))
        {
            mode.pattern_mode |= DBV_5RD_NIT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);
			printf("CURRRENT MODEL ID IS %d\n", cur_model_id);
			if(cur_model_id == DP150){
            	mode.pattern_mode |= (POCB_WRITE_ENABLE_MODE);
			}

            DPRINTF("DBV 5RD NIT FILE\n");
        }
		// 191120 dp150 source float
		else if((!strcmp(token, STRING_FLOAT_SMALL)) || (!strcmp(token, STRING_FLOAT_CAP)))
		{
            mode.pattern_mode |= FLOAT_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE);

            DPRINTF("DBV 5RD NIT FILE\n");
		}
		// 191127 dp150 hfr 90hz, 60hz
		else if((!strcmp(token, STRING_HFR_90HZ_CAP)) || (!strcmp(token, STRING_HFR_90HZ_SMALL)))
		{
			mode.pattern_mode |= HFR_90HZ_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE); 

            DPRINTF("HFR 90HZ FILE\n");
		}
		else if((!strcmp(token, STRING_HFR_60HZ_CAP)) || (!strcmp(token, STRING_HFR_60HZ_SMALL)))
		{
			mode.pattern_mode |= HFR_60HZ_MODE;
			mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE); 

            DPRINTF("HFR 60HZ FILE\n");
		}
        // 191210 dp150 dimming
        else if((!strcmp(token, STRING_DIMMING_60TO10_CAP)) || (!strcmp(token, STRING_DIMMING_60TO10_SMALL)))
        {
            mode.pattern_mode |= DIMMING_60TO10_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE); 

            DPRINTF("DIMMING 60TO10 FILE\n");
        }
        else if((!strcmp(token, STRING_DIMMING_10TO60_CAP)) || (!strcmp(token, STRING_DIMMING_10TO60_SMALL)))
        {
            mode.pattern_mode |= DIMMING_10TO60_MODE;
            mode.pattern_mode &= (~POCB_WRITE_ENABLE_MODE); 

            DPRINTF("DIMMING 10TO60 FILE\n");
        }


        /* parsing much deeper to get next mode(HT, HD, VL, VR) for Gradation file */
		if ((mode.pattern_mode & GRAD_MODE) == GRAD_MODE)
		{
			if ((mode.pattern_mode & GRAD_RGB_MODE) == GRAD_RGB_MODE)
			{
				if((!strcmp(token, STRING_HT_SMALL)) || (!strcmp(token, STRING_HT_CAP)))
				{
					DPRINTF("RGB HORIZONTAL TOP\n");
					mode.pattern_mode |= HORIZONTAL_TOP_MODE;
				}
				else if((!strcmp(token, STRING_HD_SMALL)) || (!strcmp(token, STRING_HD_CAP)))
				{
					DPRINTF("RGB HORIZONTAL DOWN\n");
					mode.pattern_mode |= HORIZONTAL_DOWN_MODE;
				}
				else if((!strcmp(token, STRING_VL_SMALL)) || (!strcmp(token, STRING_VL_CAP)))
				{
				    DPRINTF("RGB VERTICAL LEFT\n");
					mode.pattern_mode |= VERTICAL_LEFT_MODE;
				}
				else if((!strcmp(token, STRING_VR_SMALL)) || (!strcmp(token, STRING_VR_CAP)))
				{
				    DPRINTF("RGB VERTICAL RIGHT\n");
					mode.pattern_mode |= VERTICAL_RIGHT_MODE;
				}
			}
			else if ((mode.pattern_mode & GRAD_RED_MODE) == GRAD_RED_MODE)
			{
				if((!strcmp(token, STRING_HT_SMALL)) || (!strcmp(token, STRING_HT_CAP)))
				{
					DPRINTF("RED HORIZONTAL TOP\n");
					mode.pattern_mode |= HORIZONTAL_TOP_MODE;
				}
				else if((!strcmp(token, STRING_HD_SMALL)) || (!strcmp(token, STRING_HD_CAP)))
				{
					DPRINTF("RED HORIZONTAL DOWN\n");
					mode.pattern_mode |= HORIZONTAL_DOWN_MODE;
				}
				else if((!strcmp(token, STRING_VL_SMALL)) || (!strcmp(token, STRING_VL_CAP)))
				{
					DPRINTF("RED VERTICAL LEFT\n");
					mode.pattern_mode |= VERTICAL_LEFT_MODE;
				}
				else if((!strcmp(token, STRING_VR_SMALL)) || (!strcmp(token, STRING_VR_CAP)))
				{
					DPRINTF("RED VERTICAL RIGHT\n");
					mode.pattern_mode |= VERTICAL_RIGHT_MODE;
				}
			}
		}
	
	    token = strtok(NULL, TOKEN_SEP_UNDER_POINT);
//		DPRINTF("%s / ",token);
	}

	DPRINTF("Special_Pattern_mode=(0x%08x)\n", mode.pattern_mode);
	memcpy(pattern_mode_p,&mode,sizeof(special_pattern_mode_t));
	FUNC_END();

	return ret;
}

/*
 * Name : parsing_pattern_write_command
 * Description : Parsing mipi commands(register set) to write for special pattern code.
 * Parameters :
 * 		char *parsing_file_name_p : name of pattern code file
 * 		char *parsing_code_name_p : name of pattern
 * 		mipi_write_data_t *mipi_data_p : get mipi data to be written
 * Return value :
 * 		error value
 */
int parsing_pattern_write_command(char *parsing_file_name_p, char *parsing_code_name_p, mipi_write_data_t *mipi_data_p)
{
	int ret = 0;
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
 * Name : parsing_pattern_read_command
 * Description : Parsing mipi commands(register set) to write for special pattern code.
 * Parameters :
 * 		char *parsing_file_name_p : name of pattern code file
 * 		char *parsing_code_name_p : name of pattern
 * 		mipi_read_data_t *mipi_data_p : get mipi data to read
 * Return value :
 * 		error value
 */
int parsing_pattern_read_command(char *parsing_file_name_p, char *parsing_code_name_p, mipi_read_data_t *mipi_data_p)
{
	int ret = 0;
    FILE *fp;
    char *token = NULL;
    char string[MAX_PARSING_STRING_LENGTH];
	unsigned long data_value = 0;
	int found = 0;
	char parsing_file_name[MAX_PARSING_NAME_LENGTH];
	char parsing_code_name[MAX_PARSING_NAME_LENGTH];
	mipi_read_data_t mipi_data;

	FUNC_BEGIN();

	memset(&mipi_data,0,sizeof(mipi_read_data_t));
	memset(parsing_file_name,0,sizeof(parsing_file_name));
	memset(parsing_code_name,0,sizeof(parsing_code_name));

	strcpy(parsing_file_name,parsing_file_name_p);

	DPRINTF("Parsing File_name:(%s)\n",parsing_file_name);

    if((fp=(fopen(&parsing_file_name[0],"r"))) == 0 ){
        DERRPRINTF("cannot open %s\n", parsing_file_name);
        return FAIL;
    }

	strcpy(parsing_code_name,parsing_code_name_p);

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
					found = true;
					DPRINTF("Found %s\n", parsing_code_name);
				}
			}
			else if (found == true)
			{
				if(!strcmp(token,STRING_MIPI_READ_COMMAND))
				{
//					DPRINTF("Found %s\n", STRING_MIPI_READ_COMMAND);
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
	memcpy(mipi_data_p,&mipi_data,sizeof(mipi_read_data_t));

	fclose(fp);

	FUNC_END();

	return ret;
}







/*
 * Name : set_mipi_port
 * Description : Set MIPI port (A or B or BOTH).
 * Parameters : 
 * 		int mipi_port : mipi port to be set (DSI_PORT_SEL_A or DSI_PORT_SEL_B or DSi_PORT_SEL_BOTH).
 * Return value : error
 */
int set_mipi_port(int mipi_port)
{
	int ret = 0;

	FUNC_BEGIN();

	mipi_dev_open();

	/* set mipi port */
	mipi_port_set(mipi_port);

	mipi_dev_close();

	FUNC_END();

	return ret;
}

/*
 * Name : write_mipi_command
 * Description : Write several mipi commands.
 * Parameters : 
 * 		mipi_write_data_t *mipi_data_p : mipi data to be written.
 * Return value : error
 */
int write_mipi_command(mipi_write_data_t *mipi_data_p)
{
	int ret = 0;
	int write_cnt = 0;
	int reg_cnt = 0;
	int data_cnt = 0;
	unsigned int delay = 0;
//	int debug_cnt = 0;	/* for debug */

	FUNC_BEGIN();

	reg_cnt = mipi_data_p->reg_cnt;

	mipi_dev_open();

	for (write_cnt = 0;write_cnt < reg_cnt;write_cnt++)
	{
		data_cnt = mipi_data_p->data_cnt[write_cnt];

		/* write mipi data */
		
		if (mipi_data_p->data_buf[write_cnt][0] == DEFINE_OF_DELAY_COMMAND)
		{
			delay = 1000 * mipi_data_p->data_buf[write_cnt][1];	/* unit = mili seconds */
			printf("[LWG] delay is %u(us)\n", delay);
			usleep(delay);
#if	0
			printf("[Delay %d]\n", delay);
#endif
		}
		else if (mipi_data_p->data_buf[write_cnt][0] == DEFINE_OF_PPS_SET_COMMAND)
		{
			write_pps_0x0A();
#if	0	/* debug print */
			DPRINTF("PPS0x0A: PPS set ");
#endif
		}
		else
		{
			ret = mipi_write(mipi_data_p->data_buf[write_cnt][0],&mipi_data_p->data_buf[write_cnt][1],data_cnt - 1);
			if (ret < 0)
			{
				DERRPRINTF("mipi_write\n");
				FUNC_END();
				return FAIL;
			}
			else
			{
				/* debug print start */
#if	0
				DPRINTF("MIPI_WRITE: data_cnt=(%d) ", data_cnt);
				for (debug_cnt = 0;debug_cnt < data_cnt;debug_cnt++)
				{
					printf("[0x%02x] ",mipi_data_p->data_buf[write_cnt][debug_cnt]);
				}
				printf("\n");
#endif
				/* debug print end */
			}
		}
	}

	mipi_dev_close();

	FUNC_END();

	return ret;
}

/*
 * Name : read_mipi_command
 * Description : Read data from display module.
 * Parameters : 
 * 		mipi_read_data_t *mipi_data_p : mipi data to read.
 * 		unsigned char *read_data_p : return value with read data.
 * Return value : error
 */
int read_mipi_command(mipi_read_data_t *mipi_data_p, int read_len, unsigned char *read_data_p)
{
	int ret = 0;
	int read_cnt = 0;
	int reg_cnt = 0;
	unsigned int delay = 0;
//	int debug_cnt = 0;	/* for debug */

	FUNC_BEGIN();

	reg_cnt = mipi_data_p->reg_cnt;

	mipi_dev_open();

	for (read_cnt = 0;read_cnt < reg_cnt;read_cnt++)
	{
		/* read mipi data */
		if (mipi_data_p->data_buf[read_cnt][0] == DEFINE_OF_DELAY_COMMAND)
		{
			delay = 1000 * mipi_data_p->data_buf[read_cnt][1];	/* unit = mili seconds */
			usleep(delay);
#if	0
			printf("[Delay %d]\n", delay);
#endif
		}
		else
		{
			ret = mipi_read(mipi_data_p->data_buf[read_cnt][0],mipi_data_p->data_buf[read_cnt][1],read_len, read_data_p);
			if (ret < 0)
			{
				DERRPRINTF("mipi_read\n");
				FUNC_END();
				return FAIL;
			}
			else
			{
				/* debug print start */
#if	0
				DPRINTF("MIPI_READ: read_len=(%d) ", read_len);
				for (debug_cnt = 0;debug_cnt < read_len;debug_cnt++)
				{
					printf("[0x%02x] ",read_data_p[debug_cnt]);
				}
				printf("\n");
#endif
				/* debug print end */
			}
		}
	}

	mipi_dev_close();

	FUNC_END();

	return ret;
}

/*
 * Name : parsing_dsc_roll_test_file_name
 * Description : Parsing file names for DSC roll test.
 * Parameters :
 * 		char *dsc_roll_dir_p : Directory for dsc_roll test files.
 * 		int dsc_roll_file_num : Image number for dsc_roll test.
 * 		char *dsc_roll_file_string_p : File string for dsc_roll test to find the matched files in dsc_roll test directory.
 * 		char *file_name_list_p : Return file name list for dsc_roll test.
 * Return value :
 * 		int matched_file_cnt : return file number which is real file number to be used for dsc_roll test.
 */
int parsing_dsc_roll_test_file_name(char *dsc_roll_dir_p, int dsc_roll_file_num, char *dsc_roll_file_string_p, char (*file_name_list_p)[MAX_FILE_NAME_LENGTH])
{
	struct dirent **namelist;
	char dsc_roll_test_dir[MAX_FILE_NAME_LENGTH];
	int scan_file_num = 0;
	int file_cnt = 0;
	int matched_file_cnt = 0;

	FUNC_BEGIN();

	memset(&dsc_roll_test_dir[0],0,sizeof(dsc_roll_test_dir));

	sprintf(dsc_roll_test_dir,"%s/%s/%02d%s/",SD_CARD_DIR_PATH,dsc_roll_dir_p,dsc_roll_file_num,DSC_ROLL_TEST_DIR_STRING);
	scan_file_num = scandir(dsc_roll_test_dir,&namelist,0,alphasort);
	if (scan_file_num < 0)
	{
		DERRPRINTF("scandir\n");
		FUNC_END();
		return FAIL;
	}
	else
	{
		DPRINTF("DSC TEST DIR > %s <\n", dsc_roll_test_dir);
		for (file_cnt = 0;file_cnt < scan_file_num;file_cnt++)
		{
			if(strstr(namelist[file_cnt]->d_name,dsc_roll_file_string_p))
			{
				strcpy(file_name_list_p[matched_file_cnt++], namelist[file_cnt]->d_name);
				DPRINTF("%s \n",file_name_list_p[file_cnt]);
				free(namelist[file_cnt]);
			}
		}
		free(namelist);

		FUNC_END();
		return matched_file_cnt;	/* return file number which is real file number to be used for dsc_roll test */
	}

	FUNC_END();

	return 0;
}

/*
 * Name : get_measured_voltage
 * Description : Measure and Calculate voltage and return voltage value. (This function is copied from current.c, was unable to use original function in current.c because i2c file descriptor is global variable)
 * Parameters :
 *		int i2c_fd : i2c file descriptor.
 * Return value :
 * 		unsigned long : return measured voltage value.
 */
unsigned long get_measured_voltage(int i2c_fd)
{
	unsigned long measured_voltage = 0;
	unsigned long cal_voltage = 0;	/* calculated voltage */
	
	FUNC_BEGIN();
	measured_voltage = i2c_smbus_read_word_data(i2c_fd, INA219_REG_BUS);
	#ifdef NOSWAP   
	#else
	measured_voltage = ((measured_voltage & 0xFF00) >> 8 | (measured_voltage & 0xFF)  << 8);
	#endif
	measured_voltage = (measured_voltage >> 3)*4;
	
	cal_voltage = measured_voltage;
	
	FUNC_END();
	return cal_voltage;
}

/*
 * Name : calculate_value_and_write
 * Description : Calculate and write value by i2c. (This function is copied from current.c, was unable to use original function in current.c because i2c file descriptor is global variable)
 * Parameters :
 *		int i2c_fd : i2c file descriptor.
 * Return value :
 * 		int : return calculated value. (looks like not used actually)
 */
static int calculate_value_and_write(int i2c_fd)
{
	double min_lsb = 0;
	double max_pos_i = 0;
	short value =0;
	short cal_value =0;
	float r_shunt = 0.1;
	double current_lsb = 0;
	
	FUNC_BEGIN();
	max_pos_i = (float)(0.32/r_shunt);  //maxVshunt_Range/r_shunt
	
	min_lsb = (float)(max_pos_i/(float)32767);
	current_lsb = min_lsb + 0.000001;
	value = (short)(0.04096/(current_lsb*r_shunt)); //if Rshunt 0.1, cal_val = 4151
	
	#ifdef  NOSWAP
	cal_value = value;
	i2c_smbus_write_word_data(i2c_fd, INA219_REG_CAL, cal_value);
	#else
	cal_value = (((int)value & 0xFF00) >> 8 | ((int)value & 0xFF)  << 8);
	i2c_smbus_write_word_data(i2c_fd, INA219_REG_CAL, cal_value);
	#endif
	
	FUNC_END();
	return (int)cal_value;
}


#include <i2c-dev.h>
#define SLV_ADDR_VCC1						0x40
#define SLV_ADDR_VCC2						0x41
#define SLV_ADDR_VDDVDH						0x44
#define SLV_ADDR_VDDEL						0x45

/*
 * Name : get_measured_current
 * Description : Measure and Calculate current and return current value. (This function is copied from current.c, was unable to use original function in current.c because i2c file descriptor is global variable)
 * Parameters :
 *		int i2c_fd : i2c file descriptor.
 * Return value :
 * 		unsigned long : return measured current value.
 */
unsigned long get_measured_current(int i2c_fd)
{
	unsigned int half = 0;
	int i;
	short vl = 0;

	unsigned short conf = 0x3C1F;
	unsigned short conf_swap = (((int)conf & 0xFF00) >> 8 | ((int)conf & 0xFF)  << 8);
	short i_current = 0;
	short i_buf[SUM_COUNT]={0,};

	FUNC_BEGIN();
	printf("conf : 0x%x\n", conf);
	#ifdef NOSWAP
	i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf);
	#else
	i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf_swap);
	#endif

	// 1CH TTL(VDDD)
	{
		int i2c_fd_vddd = open(CURRENT_TEST_I2C_3_DEV, O_RDWR);
		if (ioctl(i2c_fd_vddd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);
	
		unsigned short conf_new = conf;
		unsigned short conf_swap_new = conf_swap;

		#ifdef NOSWAP
		i2c_smbus_write_word_data(i2c_fd_vddd, INA219_REG_CONF, conf_new);
		#else
		i2c_smbus_write_word_data(i2c_fd_vddd, INA219_REG_CONF, conf_swap_new);
		#endif
		usleep(30000);

		close(i2c_fd_vddd);
	}

	// 2CH TTL(VDDD)
	{
		int i2c_fd_vddd = open(CURRENT_TEST_I2C_4_DEV, O_RDWR);
		if (ioctl(i2c_fd_vddd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);

		unsigned short conf_new = conf;
		unsigned short conf_swap_new = conf_swap;
		
		#ifdef NOSWAP
		i2c_smbus_write_word_data(i2c_fd_vddd, INA219_REG_CONF, conf_new);
		#else
		i2c_smbus_write_word_data(i2c_fd_vddd, INA219_REG_CONF, conf_swap_new);
		#endif
		usleep(30000);

		close(i2c_fd_vddd);
	}
	
	usleep(600);
//	sleep(10);

	i_current = 0;
	
	for(i = 0; i < SUM_COUNT; i++)
	{
		calculate_value_and_write(i2c_fd);
		usleep(600);
//		usleep(600);
//		sleep(1);
		vl=i2c_smbus_read_word_data(i2c_fd,INA219_REG_CURRENT);
//		printf("0x%X ", vl);
//		if(i % 5 == 0)
//		    printf("\n");
		#ifdef NOSWAP
		#else
		vl = ((vl & 0xFF00) >> 8 | (vl & 0x00FF)  << 8);
		#endif
		if(vl < 0)
		    vl *= -1;

#if 0
		/* TEST */
		printf("0x%X ", vl);
		if(i % 5 == 0)
		    printf("\n");
#endif	
		i_buf[i] = vl;
		usleep(600);
	}
	bubble_short(SUM_COUNT,(unsigned short *)i_buf);
	
#ifdef DEBUG_MSG
	for(i=0; i< SUM_COUNT; i++)
	{
		printf("[B]buf_%d[%d]",i,i_buf[i]);
		if(i % 5 == 0)
		    printf("\n");
	}
#endif
	
	half = (SUM_COUNT / 2);
	i_current = i_buf[half];

#ifdef DEBUG_MSG
	DPRINTF("i_current = %d \n",i_current);
#endif

    if(i_current <= 0)  i_current = 0;

	FUNC_END();
    return i_current;
}

/*
 * Name : display_logo_for_vfos
 * Description : Display logo on selected display module.
 * Parameters :
 * 		int model_index
 * Return value : 
 */
void display_logo_for_vfos(int model_index)
{
	char comm[100] ={0,};

	FUNC_BEGIN();

	sprintf(comm,"%s %s/%d_logo.jpg %s", PIC_VIEW_COMMAND, LOGO_DIR_PATH, model_index, DECON_START_STOP_COMMAND);
	DPRINTF("command : %s \n", comm);
	system(comm);

	FUNC_END();
}

/*
 * Name : display_image_for_vfos
 * Description : Display image.
 * Parameters :
 * 		char image_dir
 * 		char *file_name_p
 * Return value : 
 */
void display_image_for_vfos(char *image_dir_p,char *file_name_p)
{
	char comm[100] ={0,};

	FUNC_BEGIN();

	sprintf(comm,"%s %s/%s/\"%s\" %s", PIC_VIEW_COMMAND, SD_CARD_DIR_PATH, image_dir_p, file_name_p, DECON_START_STOP_COMMAND);
	DPRINTF("command : %s \n", comm);
	system(comm);

	FUNC_END();
}

/*
 * Name : display_black_pattern_for_vfos
 * Description : Display black pattern.
 * Parameters :
 * Return value : 
 */
void display_black_pattern_for_vfos(void)
{
	char comm[100] ={0,};
	int pattern_num = 0;

	FUNC_BEGIN();

	pattern_num = BLACK_PATTERN_COMMAND_NUM;

	sprintf(comm,"%s %d %s", PATTERN_COMMAND, pattern_num, DECON_START_STOP_COMMAND);
	DPRINTF("command : %s \n", comm);
	system(comm);

	FUNC_END();
}

/*
 * Name : display_white_pattern_for_vfos
 * Description : Display white pattern.
 * Parameters :
 * Return value : 
 */
void display_white_pattern_for_vfos(void)
{
	char comm[100] ={0,};
	int pattern_num = 0;

	FUNC_BEGIN();

	pattern_num = WHITE_PATTERN_COMMAND_NUM;

	sprintf(comm,"%s %d %s", PATTERN_COMMAND, pattern_num, DECON_START_STOP_COMMAND);
	DPRINTF("command : %s \n", comm);
	system(comm);

	FUNC_END();
}

/*
 * Name : display_pattern_for_vfos
 * Description : Display specific pattern.
 * Parameters :
 * 		char *pattern_num_p : pattern number string to be displayed.
 * Return value : 
 */
void display_pattern_for_vfos(char *pattern_num_p)
{
	char comm[100] ={0,};

	FUNC_BEGIN();

	sprintf(comm,"%s %s %s", PATTERN_COMMAND, pattern_num_p, DECON_START_STOP_COMMAND);
	DPRINTF("command : %s \n", comm);
	system(comm);

	FUNC_END();
}

/*
 * Name : display_grad_rgb_ht_pattern_for_vfos
 * Description : Display GRAD_RGB_HT pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_rgb_ht_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RGB_HT_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_rgb_hd_pattern_for_vfos
 * Description : Display GRAD_RGB_HD pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_rgb_hd_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RGB_HD_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_rgb_vl_pattern_for_vfos
 * Description : Display GRAD_RGB_VL pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_rgb_vl_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RGB_VL_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_rgb_vr_pattern_for_vfos
 * Description : Display GRAD_RGB_VR pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_rgb_vr_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RGB_VR_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_red_ht_pattern_for_vfos
 * Description : Display GRAD_RED_HT pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_red_ht_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RED_HT_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_red_hd_pattern_for_vfos
 * Description : Display GRAD_RED_HD pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_red_hd_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RED_HD_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_red_vl_pattern_for_vfos
 * Description : Display GRAD_RED_VL pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_red_vl_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RED_VL_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : display_grad_red_vr_pattern_for_vfos
 * Description : Display GRAD_RED_VR pattern.
 * Parameters :
 * Return value : 
 */
void display_grad_red_vr_pattern_for_vfos(void)
{
	char pattern_num_str[MAX_PATTERN_NUM_STRING_NUM] = {0,};

	FUNC_BEGIN();

	strcpy(pattern_num_str,GRAD_RED_VR_PATTERN_COMMAND_NUM_STRING);
	display_pattern_for_vfos(pattern_num_str);

	FUNC_END();
}

/*
 * Name : start_decon_frame_update
 * Description : Start Frame update in DECON.
 * Parameters :
 * Return value : error (0: success, -1: fail)
 */
int start_decon_frame_update(void)
{
	int fd = 0;

	FUNC_BEGIN();

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		DERRPRINTF("Can not open the frame buffer device");
		FUNC_END();
		return -1;
	}

	if (ioctl(fd, DECON_IOC_FRAME_UPDATE_START, NULL)) {
		DERRPRINTF("Error-DECON_IOC_FRAME_UPDATE_START");
		close(fd);
		FUNC_END();
		return -1;
	}

	close(fd);

	FUNC_END();
	return 0;
}

/*
 * Name : stop_decon_frame_update
 * Description : Stop Frame update in DECON.
 * Parameters :
 * Return value : error
 */
int stop_decon_frame_update(void)
{
	int fd = 0;

	FUNC_BEGIN();

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		DERRPRINTF("Can not open the frame buffer device");
		FUNC_END();
		return -1;
	}

	if (ioctl(fd, DECON_IOC_FRAME_UPDATE_STOP, NULL)) {
		DERRPRINTF("Error-DECON_IOC_FRAME_UPDATE_STOP");
		close(fd);
		FUNC_END();
		return -1;
	}

	close(fd);

	FUNC_END();
	return 0;
}

int parsing_label_limit(char *parsing_file_name_p, char *parsing_code_name_p, unsigned char *read_buf)
{
	int ret = 0;
    FILE *fp;
    char *token = NULL;
    char string[MAX_PARSING_STRING_LENGTH];
	unsigned long data_value = 0;
	int found = 0;
	char parsing_file_name[MAX_PARSING_NAME_LENGTH];
	char parsing_code_name[MAX_PARSING_NAME_LENGTH];

	FUNC_BEGIN();

	memset(parsing_file_name,0,sizeof(parsing_file_name));
	memset(parsing_code_name,0,sizeof(parsing_code_name));

	strcpy(parsing_file_name,parsing_file_name_p);

	DPRINTF("Parsing File_name:(%s)\n",parsing_file_name);

    if((fp=(fopen(&parsing_file_name[0],"r"))) == 0 ){
        DERRPRINTF("cannot open %s\n", parsing_file_name);
        return FAIL;
    }

	strcpy(parsing_code_name,parsing_code_name_p);

	/* parsing */
	while((fgets(string, 500, fp)) != NULL){
        token = strtok(string, TOKEN_SEP);
        while(token != NULL){
			if (token[0] == COMMENT_START_CHARACTER)
			{
				break;
			}

			if (found == false)
			{
				if (!strcmp(token, parsing_code_name))
				{
					found = true;
					DPRINTF("Found %s\n", parsing_code_name);
				}
			}
			else if (found == true)
			{
				if(!strcmp(token,STRING_END_COMMAND))
				{
					DPRINTF("%s code end\n", parsing_code_name);
   					found = false;
					goto parsing_end;
				}
                else if(!strcmp(token,STRING_LABEL_LIMIT))
				{
					token = strtok(NULL, TOKEN_SEP);
                    int label_count = 0;
					while(token != NULL){
						if (token[0] == COMMENT_START_CHARACTER)
						{
							break;
						}
						data_value = (unsigned long)strtoul(token,NULL,16);
                        //DPRINTF("get data(0x%02x)\n", data_value);
                        read_buf[label_count] = (unsigned char)data_value;
                        label_count++;
						token = strtok(NULL, TOKEN_SEP);
					}
				}

			}
			token = strtok(NULL, TOKEN_SEP);
		}
    }
    
parsing_end:
	fclose(fp);
	FUNC_END();
	return ret;
}


