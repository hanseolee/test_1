#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //mempcpy 
#include <sys/ioctl.h>  //ioctl 
#include <fcntl.h>      //open() O_RDWR 
#include <unistd.h>     //close()
#include <mipi_con.h>
#include <type.h>
#include <rs485.h>
#include <mipi_dsi.h>
#include <pthread.h>
#include <display.h>

//#define OTPD    15
#define OTPD    20

#define PACKET_DELAY    1000
//#define PACKEY_DEBUG    0
int PACKEY_DEBUG = 0;
//int PACKEY_DEBUG = 1;			// mipi 패킷 디버그할때 필요 (ex : OTP, CRC ...)

int st_is_remain_code_after_next_pt = 0;  //maybe it only use in Retern(PREV) Command (not NEXT command).



///make

#define _IOCTL_CHANGE_HS_CLK            0x1025

///ori

#define _IOCTL_MIPI_CON_SET_DSIM_OFF                            0x1004
#define _IOCTL_MIPI_CON_CLOCK_CHANGE                            0x1000
#define _IOCTL_MIPI_CON_SET_LCD                                 0x1001
#define _IOCTL_MIPI_CON_SET_LCD_PRE                             0x1002
#define _IOCTL_MIPI_CON_SET_LCD_AFTER                           0x1003
#define _IOCTL_MIPI_CON_SET_REGISTER_SHORT                      0x1005
#define _IOCTL_MIPI_CON_SET_REGISTER_LONG                       0x1006
#define _IOCTL_MIPI_CON_SET_I80                                 0x1007
#define _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_NO_PARM        0x1008
#define _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM         0x1009
#define _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM         0x100A
#define _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM          0x100B
#define _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM           0x100C
#define _IOCTL_MIPI_CON_SET_REGISTER_GENERIC_READ_NO_PARM       0x100D
#define _IOCTL_MIPI_CON_SET_REGISTER_GENERIC_READ_1_PARM        0x100E
#define _IOCTL_MIPI_CON_SET_REGISTER_GENERIC_READ_2_PARM        0x100F
#define _IOCTL_MIPI_CON_SET_REGISTER_DCS_READ_NO_PARM           0x1010
#define _IOCTL_MIPI_CON_SET_REGISTER_COMMAND_OFF                0x1011
#define _IOCTL_MIPI_CON_SET_REGISTER_COMMAND_ON                 0x1012
#define _IOCTL_MIPI_CON_SET_REGISTER_COMMAND_SHUT_DOWN          0x1013
#define _IOCTL_MIPI_CON_SET_REGISTER_COMMAND_TURN_ON            0x1014
#define _IOCTL_MIPI_CON_SET_REGISTER_NULL_PACKET                0x1015
#define _IOCTL_MIPI_CON_SET_REGISTER_BLANKING_PACKET            0x1016
#define _IOCTL_MIPI_CON_SET_REGISTER_READ_DATA                  0x1017
#define _IOCTL_DSI_PORT_SEL                                     0x1018
#define _IOCTL_MIPI_CON_NO_KERNEL_MSG_MIPI_WRITE     0x1019 //khl

#define _IOCTL_MIPI_CON_SET_TRANSFER_LP                         0x4001
#define _IOCTL_MIPI_CON_SET_TRANSFER_HS                         0x4002
#define _IOCTL_MIPI_CON_SET_TRANSFER_HS_CMD                     0x4003
#define _IOCTL_MIPI_CON_SET_TRANSFER_LP_DATA                    0x4004
#define _IOCTL_MIPI_CON_SET_MIPI_ENABLE                         0x4005
#define _IOCTL_MIPI_CON_SET_MIPI_DISABLE                        0x4006



#define _IOCTL_PWM_DUTY          0x1110
#define _IOCTL_PWM_PERIOD           0x1112

#define _IOCTL_LED_ONOFF_CONTROL    0x1306
#define _IOCTL_LCD_VCC1_CONTROL  0x1201
#define _IOCTL_LCD_VCC2_CONTROL  0x1202

#define _IOCTL_FB_CON_RELEASE       0x1000
#define _IOCTL_FB_CON_INIT      0x1001

#define _IOCTL_MIPI_CON_33V_ON          0x1020
#define _IOCTL_MIPI_CON_33V_OFF         0x1021
#define _IOCTL_MIPI_CON_18V_ON          0x1022
#define _IOCTL_MIPI_CON_18V_OFF         0x1023

#define _IOCTL_LED1_CONTROL         0x1115
#define _IOCTL_LED2_CONTROL         0x1116
#define _IOCTL_LED3_CONTROL         0x1119
#define _IOCTL_LED4_CONTROL         0x111A

#define _IOCTL_CH1_TE_GET               0x1004
#define _IOCTL_CH2_TE_GET               0x1005

#define READ_REPAET_CNT 1
#define BASIC_DELAY 1


int mipi_dev = 0, decon_dev = 0;
int dic_dev;
int dicOpen;

short_packet short_command;
long_packet long_command;
read_packet read_command;
static struct mipi_conf all_config;
int mipi_write(char PacketType, unsigned char *reg_buf, int len);
int mipi_write_delay(char PacketType, unsigned char *reg_buf, int len);

void joan_power_bright_line_mode_onoff(int id, int model_index,int *onoff);
void joan_power_black_line_mode_onoff(int id, int model_index, int *onoff);
void joan_luminance_50per_power_bright_line_mode_onoff(int id, int model_index, int *onoff);
void joan_luminance_50per_power_black_line_mode_onoff(int id, int model_index, int *onoff);


void m_delay(int msCount)
{
    int i;
    for(i=0; i<200; i++)
        usleep(msCount);
}

int mipi_dev_open()
{
	FUNC_BEGIN();
    if ((mipi_dev = open("/dev/mipicon", O_RDWR )) < 0){
        perror("open faile /dev/mipicon\n");
		FUNC_END();
        return -1;
    }

	FUNC_END();
    return 0;
}

int mipi_dev_close()
{
	FUNC_BEGIN();
    close(mipi_dev);

	FUNC_END();
    return 0;
}

int decon_dev_open()
{
	FUNC_BEGIN();
    if ((decon_dev = open("/dev/fb_con", O_RDWR )) < 0){
        perror("open faile /dev/fb_con\n");
		FUNC_END();
        return -1;
    }
	FUNC_END();
	return 0;
}

int decon_dev_close()
{
	FUNC_BEGIN();
    close(decon_dev);
	FUNC_END();
    return 0;
}

void configure_dsi_read(void)
{
    int tmp;

	FUNC_BEGIN();
    memset(&all_config,0,sizeof(struct mipi_conf));

    all_config.lcd_info.mode = DECON_VIDEO_MODE;
    all_config.lcd_info.xres = 1440;
    all_config.lcd_info.yres = 2880;
    all_config.lcd_info.dsi_lanes = 4;
    //all_config.lcd_info.hs_clk = 800;
    all_config.lcd_info.hs_clk = 880;
    all_config.lcd_info.cmd_tr_mode = 0;
    all_config.lcd_info.esc_clk = 20;
    all_config.lcd_info.dsc_enabled = 1;
    all_config.lcd_info.dsc_cnt = 1;
    all_config.lcd_info.dsc_slice_num = 2;

    tmp = DSI_PORT_SEL_BOTH;
    ioctl(mipi_dev, _IOCTL_DSI_PORT_SEL, &tmp);

    ioctl(mipi_dev,  _IOCTL_MIPI_CON_SET_LCD_PRE, &all_config);
    ioctl(mipi_dev,  _IOCTL_MIPI_CON_SET_LCD_AFTER, NULL);

    printf("**CHANGE to VIDEO MODE for MIPI READ.....**\n");
	FUNC_END();
}

int	decon_fb_init_release(int init)
{
	FUNC_BEGIN();
	if(init)
	{
		ioctl(decon_dev, _IOCTL_FB_CON_INIT, NULL);
		printf("%s : FB_CON_INIT \n",__func__);
	}
	else
	{
		ioctl(decon_dev, _IOCTL_FB_CON_RELEASE, NULL);
		printf("%s : FB_CON_RELESE \n",__func__);
	}

	FUNC_END();
	return 0;
}

void write_pps_0x0A(void)
{
    FILE *fp;
    char string[80];
    char tokensep[]=" \t\n\r";
    char *token;
    int idx = -1;
    int tmp;
    int i;

	FUNC_BEGIN();
	memset(&long_command, 0, sizeof(long_command));



	printf("\nPPS Packet (0x0A) MODE...\n\n");
    long_command.packet_type = 0x0A;
    long_command.count = 88;

    if ((fp = (fopen("/Data/pps.tty", "r"))) == 0 )
    {
        printf("Write PPS : Cannot Open %s\n", "/Data/pps.tty");
        exit(1);
    }

    while ((fgets(string, 80, fp)) != NULL)
    {
        token = strtok(string, tokensep);
#ifdef  PPS_PRINT
        printf("[%s]", token);
#endif

        while (token != NULL)
        {
            if (!strcmp(token, "dsc_version"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 1] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t\t%s\n", token);
#endif
            }


            else if (!strcmp(token, "dsc_pps_id"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 2] = tmp;
                long_command.data[idx + 3] = 0x00;
#ifdef  PPS_PRINT
                printf("\t\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token,"dsc_bpc"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 4] += (tmp << 4);
#ifdef  PPS_PRINT
                printf("\t\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_linebuf_depth"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 4] += tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token,"dsc_initial_xmit_delay"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 17] = (tmp >> 8);
                long_command.data[idx + 18] = tmp;
#ifdef  PPS_PRINT
                printf("\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_initial_scale_value"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 21] = (tmp >> 8);
                long_command.data[idx + 22] = tmp;
#ifdef  PPS_PRINT
                printf("\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_first_line_bpg_offset"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 27] = 0x00;
                long_command.data[idx + 28] = tmp;
#ifdef  PPS_PRINT
                printf("\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_initial_offset"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 33] = (tmp >> 8);
                long_command.data[idx + 34] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_block_pred_enable"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 5] += (tmp << 5);
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_convert_rgb"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 5] += (tmp << 4);
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_enable_422"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 5] += (tmp << 3);
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_vbr_enable"))
            {
                token= strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 5] += (tmp << 2);
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_bpp"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 16);
                long_command.data[idx + 5] += (tmp << 2);
                long_command.data[idx + 6] += tmp;
#ifdef  PPS_PRINT
                printf("\t\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_pic_height"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 7] = (tmp >> 8);
                long_command.data[idx + 8] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_pic_width"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 9] = (tmp >> 8);
                long_command.data[idx + 10] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_slice_height"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
//              tmp = 720;  ////
                long_command.data[idx + 11] = (tmp >> 8);
                long_command.data[idx + 12] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_slice_width"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 13] = (tmp >> 8);
                long_command.data[idx + 14] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_chunk_size"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 15] = (tmp >> 8);
                long_command.data[idx + 16] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_initial_dec_delay"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 19] = (tmp >> 8);
                long_command.data[idx + 20] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_scale_increment_interval"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 23] = (tmp >> 8);
                long_command.data[idx + 24] = tmp;
#ifdef  PPS_PRINT
                printf("\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_scale_decrement_interval"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 25] = (tmp >> 8);
                long_command.data[idx + 26] = tmp;
#ifdef  PPS_PRINT
                printf("\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_nfl_bpg_offset"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 29] = (tmp >> 8);
                long_command.data[idx + 30] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_slice_bpg_offset"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
//              tmp = 28;
                long_command.data[idx + 31] = (tmp >> 8);
                long_command.data[idx + 32] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }
            else if (!strcmp(token, "dsc_final_offset"))
            {
                token = strtok(NULL, tokensep);
                tmp = strtoul(token, NULL, 10);
                long_command.data[idx + 35] = (tmp >> 8);
                long_command.data[idx + 36] = tmp;
#ifdef  PPS_PRINT
                printf("\t\t%s\n", token);
#endif
            }

            token=strtok(NULL, tokensep);
        }
    }
/*------------------------------------------------------*/
/*---             Set Default Value                  ---*/
/*------------------------------------------------------*/

    idx += 37;

    // index = 37:40
    long_command.data[idx++] = 0x03;
    long_command.data[idx++] = 0x0C;
    long_command.data[idx++] = 0x20;
    long_command.data[idx++] = 0x00;

    // index = 41:50
    long_command.data[idx++] = 0x06;
    long_command.data[idx++] = 0x0B;
    long_command.data[idx++] = 0x0B;
    long_command.data[idx++] = 0x33;
    long_command.data[idx++] = 0x0E;
    long_command.data[idx++] = 0x1C;
    long_command.data[idx++] = 0x2A;
    long_command.data[idx++] = 0x38;
    long_command.data[idx++] = 0x46;
    long_command.data[idx++] = 0x54;

    // index = 51:60
    long_command.data[idx++] = 0x62;
    long_command.data[idx++] = 0x69;
    long_command.data[idx++] = 0x70;
    long_command.data[idx++] = 0x77;
    long_command.data[idx++] = 0x79;
    long_command.data[idx++] = 0x7B;
    long_command.data[idx++] = 0x7D;
    long_command.data[idx++] = 0x7E;
    long_command.data[idx++] = 0x01;
    long_command.data[idx++] = 0x02;

    // index = 61:70
    long_command.data[idx++] = 0x01;
    long_command.data[idx++] = 0x00;
    long_command.data[idx++] = 0x09;
    long_command.data[idx++] = 0x40;
    long_command.data[idx++] = 0x09;
    long_command.data[idx++] = 0xBE;
    long_command.data[idx++] = 0x19;
    long_command.data[idx++] = 0xFC;
    long_command.data[idx++] = 0x19;
    long_command.data[idx++] = 0xFA;

    // index = 71:80
    long_command.data[idx++] = 0x19;
    long_command.data[idx++] = 0xF8;
    long_command.data[idx++] = 0x1A;
    long_command.data[idx++] = 0x38;
    long_command.data[idx++] = 0x1A;
    long_command.data[idx++] = 0x78;
    long_command.data[idx++] = 0x1A;
    long_command.data[idx++] = 0xB6;
    long_command.data[idx++] = 0x2A;
    long_command.data[idx++] = 0xF6;

    // index = 81:88
    long_command.data[idx++] = 0x2B;
    long_command.data[idx++] = 0x34;
    long_command.data[idx++] = 0x2B;
    long_command.data[idx++] = 0x74;
    long_command.data[idx++] = 0x3B;
    long_command.data[idx++] = 0x74;
    long_command.data[idx++] = 0x6B;
    long_command.data[idx++] = 0xF4;

    ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);

    if(PACKEY_DEBUG)
    {
        printf("[W] : 0x%02x", long_command.packet_type);
        for(i=0; i<long_command.count; i++)
            printf(",0x%02x", long_command.data[i]);
        printf("\n");
    }

    memset(&long_command, 0, sizeof(long_command));

	fclose(fp);

    printf("PPS_PKT DONE.......\n");
	FUNC_END();
}



///////////////////////////////////////////////

unsigned char found_reg_init_data(int id, int model_index, unsigned char reg, int data_index) //180104
{
    char init_path[300] = "/mnt/sd/initial/register_data";
    char string[500];
    FILE *fp;
    char *token = NULL;
    unsigned char tmp = 0;
    int index_st = 0;
    int check_st = 0;
	int data_count = 0;

	FUNC_BEGIN();

    sprintf(init_path,"%s%d.tty",init_path,model_index);
    printf("%s [MODEL:%d][index:%d] searching INDEX 0x%X  datanum %d \n",init_path, id, model_index, reg,data_index);

    if((fp=(fopen(init_path,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, init_path);
		FUNC_END();
        return FAIL;
    }

   while((fgets(string, 500, fp)) != NULL){
        token = strtok(string, TOKEN_SEP);
        if(token != NULL){
            if(!strcmp(token,"INDEX"))
            {
				index_st = 1;
            }
            else if(!strcmp(token,"DATA"))
            {

                if(index_st)
                {
					index_st = 0;
                    tmp = 0;
                    token = strtok(NULL, TOKEN_SEP);
                    tmp = (unsigned char)strtoul(token,NULL,16);
                    if(tmp == reg)
                        check_st = 1;
                }
                else if(check_st)
                {
					data_count++;

					if(data_count == data_index)
					{
						check_st = 0;
                    	tmp = 0;
                    	token = strtok(NULL, TOKEN_SEP);
                    	tmp = (unsigned char)strtoul(token,NULL,16);
                    	printf("found data 0x%X [reg:0x%X / index:%d] \n",tmp,reg,data_index);
						FUNC_END();
                    	return tmp;
					}
                }
            }
        }
    }

    printf("%s : DATA FOUND FAIL...[0x%X]\n",__func__,reg);
	FUNC_END();
    return 0;
}



void found_initial_dbv(int id, int model_index, unsigned char *reg_dbv1, unsigned char *reg_dbv2)
{
    /* DBV register in initial code*/
	FUNC_BEGIN();

	switch(id)
	{
		case	DP049	:
		    *reg_dbv1 = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
		    *reg_dbv2 = found_reg_init_data(id,model_index, 0x51,2); //modify 180920

	        if(!(*reg_dbv1))
				*reg_dbv1 = 0x03; 

            if(!(*reg_dbv2))
            	*reg_dbv2 = 0xE1; 
			break;

        case    AKATSUKI   :
            *reg_dbv1 = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
            *reg_dbv2 = found_reg_init_data(id,model_index, 0x51,2); //modify 180920

            if(!(*reg_dbv1))
                *reg_dbv1 = 0x02;

            if(!(*reg_dbv2))
                *reg_dbv2 = 0xC3;
        break;
        case    B1   :  //modify 180226
            *reg_dbv1 = found_reg_init_data(id,model_index, 0x51,1);
            *reg_dbv2 = found_reg_init_data(id,model_index, 0x51,2);

            if(!(*reg_dbv1))
                *reg_dbv1 = 0x03;

            if(!(*reg_dbv2))
                *reg_dbv2 = 0xFF;
			break;
		default :
			printf("%s:This model id not collect... \n",__func__);
			break;
	}
	FUNC_END();
}

void joan_sleep_in_out(int id, int model_index, int mode, int in)
{
    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
	printf("JOAN[0x%X] SLEEP IN/OUT [I?%d]\n",mode,in);
    if(!in)
    {
        usleep(50000);

		PacketType = 0x05;
		reg_buf[0] = 0x11;
		mipi_write(PacketType, reg_buf, 1);

		usleep(60000);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE2;
		reg_buf[1] = 0x20;
		reg_buf[2] = 0x0D;
		reg_buf[3] = 0x08;
		reg_buf[4] = 0xA8;
		reg_buf[5] = 0x0A;
		reg_buf[6] = 0xAA;
		reg_buf[7] = 0x04;
		reg_buf[8] = 0xA4;
		reg_buf[9] = 0x80;
		reg_buf[10] = 0x80;
		reg_buf[11] = 0x80;
		reg_buf[12] = 0x5C;
		reg_buf[13] = 0x5C;
		reg_buf[14] = 0x5C;
		mipi_write(PacketType, reg_buf, 15);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =		0x39;
		reg_buf[0] =	0xE7;
		reg_buf[1] =	0x00;
		reg_buf[2] =	0x0D;
		reg_buf[3] =	0x76;
		reg_buf[4] =	0x1F;
		reg_buf[5] =	0x00;
		reg_buf[6] =	0x0d;
		reg_buf[7] =	0x4A;
		reg_buf[8] =	0x44;
		reg_buf[9] =	0x0D;
		reg_buf[10] =	0x76;
		reg_buf[11] =	0x25;
		reg_buf[12] =	0x00;
		reg_buf[13] =	0x0D;
		reg_buf[14] =	0x0D;
		reg_buf[15] =	0x0D;
		reg_buf[16] =	0x0D;
		reg_buf[17] =	0x4A;
		reg_buf[18] =	0x00;
		mipi_write(PacketType, reg_buf, 19);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x15;
		reg_buf[0] =	0x51;
		reg_buf[1] =	0xEF;
		mipi_write(PacketType, reg_buf, 2);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =    0x15;
		reg_buf[0] =    0x53;
		reg_buf[1] =    0x00;
		mipi_write(PacketType, reg_buf, 2);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =    0x15;
		reg_buf[0] =    0x55;
		reg_buf[1] =    0x0C;
		mipi_write(PacketType, reg_buf, 2);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x39;
		reg_buf[0] =	0xE8;
		reg_buf[1] =	0x08;
		reg_buf[2] =	0x90;
		reg_buf[3] =	0x10;
		reg_buf[4] =	0x25;
		mipi_write(PacketType, reg_buf, 5);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x39;
		reg_buf[0] =	0xFB;
		reg_buf[1] =	0x03;
		reg_buf[2] =	0x77;
		mipi_write(PacketType, reg_buf, 3);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x39;
		reg_buf[0] =	0xED;
		reg_buf[1] =	0x13;
		reg_buf[2] =	0x00;
		reg_buf[3] =	0x07;
		reg_buf[4] =	0x00;
		reg_buf[5] =	0x13;
		mipi_write(PacketType, reg_buf, 6);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x39;
		reg_buf[0] =	0xB7;
		reg_buf[1] =	0x03;
		reg_buf[2] =	0x06;
		reg_buf[3] =	0x26;
		reg_buf[4] =	0x00;
		reg_buf[5] =	0x88;
		reg_buf[6] =	0x08;
		reg_buf[7] =	0x80;
		reg_buf[8] =	0x88;
		reg_buf[9] =	0x88;
		reg_buf[10] =	0x88;
		reg_buf[11] =	0x20;
		mipi_write(PacketType, reg_buf, 12);

		usleep(90000);

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType =	0x39;
		reg_buf[0] =	0xE7;
		reg_buf[1] =	0x00;
		reg_buf[2] =	0x0D;
		reg_buf[3] =	0x76;
		reg_buf[4] =	0x1F;
		reg_buf[5] =	0x00;
		reg_buf[6] =	0x0d;
		reg_buf[7] =	0x0d;
		reg_buf[8] =	0x44;
		reg_buf[9] =	0x0D;
		reg_buf[10] =	0x76;
		reg_buf[11] =	0x25;
		reg_buf[12] =	0x00;
		reg_buf[13] =	0x0D;
		reg_buf[14] =	0x0D;
		reg_buf[15] =	0x0D;
		reg_buf[16] =	0x0D;
		reg_buf[17] =	0x4A;
		reg_buf[18] =	0x00;
		mipi_write(PacketType, reg_buf, 19);
    }
    else
    {
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
		usleep(2000);
    }
	FUNC_END();
}

void mv_sleep_in_out(int id, int model_index, int in)
{
    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv = 0xFF;

	FUNC_BEGIN();
    printf("MV SLEEP IN/OUT [I?%d]\n",in);
    if(!in)
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFF;

        usleep(50000);
            PacketType = 0x05;
            reg_buf[0] = 0x11;
            mipi_write(PacketType, reg_buf, 1);
            usleep(60000);

			PacketType = 0x39;
			reg_buf[0] = 0xB0;
			reg_buf[1] = 0xA5;
			reg_buf[2] = 0x00;
			mipi_write(PacketType, reg_buf, 3);

			memset(reg_buf,0,sizeof(reg_buf));
            PacketType =	0x39;
			reg_buf[0] =	0xE7;
			reg_buf[1] =	0x00;
			reg_buf[2] =	0x0D;
			reg_buf[3] =	0x76;
			reg_buf[4] =	0x23;
			reg_buf[5] =	0x00;
			reg_buf[6] =	0x00;
			reg_buf[7] =	0x23;
			reg_buf[8] =	0x44;
			reg_buf[9] =	0x0D;
			reg_buf[10] =	0x76;
			reg_buf[11] =	0x0D;
			reg_buf[12] =	0x0D;
			reg_buf[13] =	0x00;
			reg_buf[14] =	0x0D;
			reg_buf[15] =	0x0D;
			reg_buf[16] =	0x0D;
			reg_buf[17] =	0x4A;
			reg_buf[18] =	0x00;
			mipi_write(PacketType, reg_buf, 19);

			memset(reg_buf,0,sizeof(reg_buf));
			PacketType =	0x15;
			reg_buf[0] =	0x51;
			reg_buf[1] =	reg_dbv;
			mipi_write(PacketType, reg_buf, 2);

			memset(reg_buf,0,sizeof(reg_buf));
            PacketType =    0x15;
            reg_buf[0] =    0x53;
            reg_buf[1] =    0x00;
            mipi_write(PacketType, reg_buf, 2);

			memset(reg_buf,0,sizeof(reg_buf));
            PacketType =    0x15;
            reg_buf[0] =    0x55;
            reg_buf[1] =    0x0C;
            mipi_write(PacketType, reg_buf, 2);

			memset(reg_buf,0,sizeof(reg_buf));
			PacketType =	0x39;
			reg_buf[0] =	0xFB;
			reg_buf[1] =	0x03;
			reg_buf[2] =	0x77;
            mipi_write(PacketType, reg_buf, 3);

			memset(reg_buf,0,sizeof(reg_buf));
			PacketType =	0x39;
			reg_buf[0] =	0xED;
			reg_buf[1] =	0x13;
			reg_buf[2] =	0x00;
			reg_buf[3] =	0x07;
			reg_buf[4] =	0x00;
			reg_buf[5] =	0x00;
            mipi_write(PacketType, reg_buf, 6);

			memset(reg_buf,0,sizeof(reg_buf));
			PacketType =	0x39;
			reg_buf[0] =	0xE2;
			reg_buf[1] =	0x20;
			reg_buf[2] =	0x0D;
			reg_buf[3] =	0x08;
			reg_buf[4] =	0xA8;
			reg_buf[5] =	0x0A;
			reg_buf[6] =	0x04;
			reg_buf[7] =	0x44;
			reg_buf[8] =	0x80;
			reg_buf[9] =	0x80;
			reg_buf[10] =	0x80;
			reg_buf[11] =	0x5C;
			reg_buf[12] =	0x5C;
			reg_buf[13] =	0x5C;
            mipi_write(PacketType, reg_buf, 14);
    }
    else
    {
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
		usleep(2000);
    }
	FUNC_END();
}

void a1_sleep_in_out(int id,int model_index, int in)
{

	printf("A1 SLEEP IN/OUT [I?%d]\n",in);

    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv = 0xFE;

	FUNC_BEGIN();
    if(!in)
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFE;

        usleep(50000);
		PacketType = 0x05;
		reg_buf[0] = 0x11;
		mipi_write(PacketType, reg_buf, 1);
		usleep(120000);
		
		
		PacketType = 0x39;
		reg_buf[0] = 0x7F;
		reg_buf[1] = 0x5A;
		reg_buf[2] = 0x5A;
		mipi_write(PacketType, reg_buf, 3);
		
		PacketType = 0x15;
		reg_buf[0] = 0x02;
		reg_buf[1] = 0x01;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x15;
		reg_buf[0] = 0x35;
		reg_buf[1] = 0x01;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x15;
		reg_buf[0] = 0x36;
		reg_buf[1] = 0x02;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x15;
		reg_buf[0] = 0x51;
		reg_buf[1] = reg_dbv;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x15;
		reg_buf[0] = 0x53;
		reg_buf[1] = 0x20;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x15;
		reg_buf[0] = 0x57;
		reg_buf[1] = 0x24;
		mipi_write(PacketType, reg_buf, 2);
		
		write_pps_0x0A();
		
		system("/Data/Pattern 11 B1");
		
		usleep(100000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x29;
		mipi_write(PacketType, reg_buf, 1);
    }
    else
    {
		PacketType = 0x05;
		reg_buf[0] = 0x28;
		mipi_write(PacketType, reg_buf, 1);
		
		usleep(100000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
		usleep(2000);
    }

	FUNC_END();
}

void dp049_sleep_in_out(int id,int model_index, int in)
{
    printf("DP049 SLEEP IN/OUT [I?%d]\n",in);

    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;
    //int i;

	FUNC_BEGIN();
    if(in)
    {
		PacketType = 0x05;
		reg_buf[0] = 0x28;
		mipi_write(PacketType, reg_buf, 1);
		
		usleep(120000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x34;
		mipi_write(PacketType, reg_buf, 1);
		
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
    }
    else
    {
		PacketType = 0x15;
		reg_buf[0] = 0x35;
		reg_buf[1] = 0x00;
		mipi_write(PacketType, reg_buf, 2);
		
		PacketType = 0x05;
		reg_buf[0] = 0x11;
		mipi_write(PacketType, reg_buf, 1);
		
		usleep(200000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x29;
		mipi_write(PacketType, reg_buf, 1);
    }
	FUNC_END();
}

void akatsuki_sleep_in_out(int id,int model_index, int in)
{
    printf("AKATSUKI SLEEP IN/OUT [I?%d]\n",in);

    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;
    //int i;

	FUNC_BEGIN();
    if(in)
    {
		PacketType = 0x05;
		reg_buf[0] = 0x28;
		mipi_write(PacketType, reg_buf, 1);
		
		usleep(120000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
		usleep(120000);
    }
    else
    {
		PacketType = 0x05;
		reg_buf[0] = 0x11;
		mipi_write(PacketType, reg_buf, 1);
		
		usleep(200000);
		
		PacketType = 0x05;
		reg_buf[0] = 0x29;
		mipi_write(PacketType, reg_buf, 1);
		usleep(120000);
    }
	FUNC_END();
}



void mv_mqa_sleep_in_out(int id, int model_index, int in)
{
    printf("MV_MQA SLEEP IN/OUT [I?%d]\n",in);

    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;
	int i;

	FUNC_BEGIN();
	if(!in)
	{
	    usleep(50000);
  
	    for(i=0; i<7; i++)
	    {
	        PacketType = 0x05;
	        reg_buf[0] = 0x11;
	        mipi_write(PacketType, reg_buf, 1);
	        usleep(2000);
	    }
	}
	else
	{
	    for(i=0; i<7; i++)
	    {
	        PacketType = 0x05;
	        reg_buf[0] = 0x10;
	        mipi_write(PacketType, reg_buf, 1);
	        usleep(2000);
	    }
	}
	FUNC_END();
}


void b1_sleep_in_out(int id, int model_index, int in, int if_prev)
{
    printf("B1 SLEEP IN/OUT [I?%d]\n",in);

    unsigned char reg_buf[30];
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(!in)
    {
        PacketType = 0x05;
        reg_buf[0] = 0x11;
        mipi_write(PacketType, reg_buf, 1);
		usleep(5000);
		if(if_prev)
		{
			usleep(130000);
			PacketType = 0x05;
			reg_buf[0] = 0x29;
			mipi_write(PacketType, reg_buf, 1);
			usleep(50000);
			printf("B1 Sleep Code dependence Retern key OFF\n"); 
		}
		else
			printf("B1 Sleep(+DP OFF) Code Normal OFF\n"); 
    }
    else
    {
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        usleep(100000);
        PacketType = 0x05;
        reg_buf[0] = 0x10;
        mipi_write(PacketType, reg_buf, 1);
        usleep(100000);

		printf("B1 Sleep(+DP OFF) Code Normal ON\n"); 
    }
	FUNC_END();
}

int sleep_control(int id, int model_index, int in, int if_prev)
{

	FUNC_BEGIN();
	printf("%s : sleep [%d] \n",__func__, in);
    switch(id)
    {

        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
            joan_sleep_in_out(id, model_index,0, in);
			break;
        case    MV :
        case    MV_MANUAL :
            mv_sleep_in_out(id, model_index, in);
            break;
        case    A1 :
            a1_sleep_in_out(id, model_index, in);
            break;
        case    JOAN_E5 :
			joan_sleep_in_out(id, model_index,1,in);
            break;
		case	DP049	:
            dp049_sleep_in_out(id, model_index,in);
            break;

		case	AKATSUKI	:
			akatsuki_sleep_in_out(id, model_index,in);
			break;
        case    MV_MQA   :
        case    MV_DQA   :
            mv_mqa_sleep_in_out(id,model_index, in);
            break;

        case    B1   :
            b1_sleep_in_out(id,model_index, in,if_prev);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
			FUNC_END();
            return FAIL;
    }
	FUNC_END();
    return PASS;
}


/////////////////////////////////////////

////////////////////////////////////////

void mipi_port_set(int dsi)
{
	int tmp = 0;

	FUNC_BEGIN();
	if(dsi == DSI_PORT_SEL_A){
		tmp = DSI_PORT_SEL_A;
		ioctl(mipi_dev, _IOCTL_DSI_PORT_SEL, &tmp);	
		printf("mipi_port_set : A [%d]\n",tmp);
	}
	else if(dsi == DSI_PORT_SEL_B){
		tmp = DSI_PORT_SEL_B;
		ioctl(mipi_dev, _IOCTL_DSI_PORT_SEL, &tmp);	
		printf("mipi_port_set : B [%d]\n",tmp);
	}
	else if(dsi == DSI_PORT_SEL_BOTH){
        tmp = DSI_PORT_SEL_BOTH;
        ioctl(mipi_dev, _IOCTL_DSI_PORT_SEL, &tmp); 
        printf("mipi_port_set : BOTH [%d]\n",tmp);
	}
	else
        printf("%s : mipi_port_set data err\n",__func__);

	FUNC_END();
}


int mipi_write_delay(char PacketType, unsigned char *reg_buf, int len)
{
    int i;

	FUNC_BEGIN();
    memset(&short_command, 0, sizeof(short_command));
    memset(&long_command, 0, sizeof(long_command));

    switch(PacketType)
    {
        case 0x03:  //-Generic Short Write, No parameter

            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : 0x%02x,", PacketType);
            }
            short_command.packet_type = PacketType;
            //?
            break;

        case 0x13:   //-Generic Short Write, 1 parameter
            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : 0x%02x", PacketType);
                printf(" 0x%02x,0x%02x \n", reg_buf[0], reg_buf[1]);

            }
            short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = reg_buf[1];
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);


            if(PACKEY_DEBUG)
                printf("0x%02x,0x%02x",reg_buf[0], 0x00);

            break;
        case 0x23:   //-Generic Short Write, 2 parameter
            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : %#04x, 0x%02x, 0x%02x \n", PacketType, reg_buf[0], reg_buf[1]);
            }
            short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = reg_buf[1];
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????


            break;

        case 0x05:  //-DCS_WRITE_NO_PARM
            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : 0x%02x", PacketType);
                printf(" 0x%02x,0x%02x \n", reg_buf[0], 0x00);
            }
            short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = 0x00;
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);

            break;
        case 0x15:  //-DCS_WRITE_1_PARM
            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : %#04x, 0x%02x, 0x%02x \n", PacketType, reg_buf[0], reg_buf[1]);
            }
            short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = reg_buf[1];
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);


            break;
        case 0x29:
        case 0x39:
        case 0x2C:
        case 0x3C:
        case 0x3e:
            if(PACKEY_DEBUG)
            {

                printf("[W] : 0x%02x", PacketType);

                for (i=0; i<len; i++){
                    printf(",0x%02x", reg_buf[i]);
                }
            printf("\n");
            }

            long_command.packet_type = PacketType;
            memcpy(long_command.data, reg_buf, len);
            long_command.count = len;

            if (len == 1)
            {
                long_command.data[1] = 0x00;
                long_command.count = 2;
            }

            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);

            break;
    }
    usleep(12000);
	FUNC_END();
    return 0;
}


int mipi_write(char PacketType, unsigned char *reg_buf, int len)
{
	int i;
	int ret = 0;

	FUNC_BEGIN();
    memset(&short_command, 0, sizeof(short_command));
    memset(&long_command, 0, sizeof(long_command));

    switch(PacketType)
    {
		case 0x03:  //-Generic Short Write, No parameter

			if(PACKEY_DEBUG)
			{
			    printf("\n");
			    printf("[W] : 0x%02x,", PacketType);
			}
			short_command.packet_type = PacketType;
			//?
			break;

		case 0x13:   //-Generic Short Write, 1 parameter
			if(PACKEY_DEBUG)
			{
			    printf("\n");
	            printf("[W] : 0x%02x", PacketType);
		        printf(" 0x%02x,0x%02x \n", reg_buf[0], reg_buf[1]);

			}
			short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = reg_buf[1];
            ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM, &short_command);
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_1_PARM\n");
				FUNC_END();
				return ret;
			}
            if(PACKEY_DEBUG)
                printf("0x%02x,0x%02x",reg_buf[0], 0x00);

			break;

        case 0x23:   //-Generic Short Write, 2 parameter
			if(PACKEY_DEBUG)
			{
				printf("\n");
                printf("[W] : %#04x, 0x%02x, 0x%02x \n", PacketType, reg_buf[0], reg_buf[1]);
            }
            short_command.packet_type = PacketType;
			short_command.reg = reg_buf[0];
			short_command.data = reg_buf[1];
			ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM, &short_command);  //?????????
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_WRITE_SHORT_2_PARM\n");
				FUNC_END();
				return ret;
			}
            break;

        case 0x05:  //-DCS_WRITE_NO_PARM
		case 0x37:
            if(PACKEY_DEBUG)
            {
                printf("\n");
                printf("[W] : 0x%02x", PacketType);
                printf(" 0x%02x,0x%02x \n", reg_buf[0], 0x00);
            }
            short_command.packet_type = PacketType;
            short_command.reg = reg_buf[0];
            short_command.data = 0x00;
            ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM, &short_command);
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_NO_PARM\n");
				FUNC_END();
				return ret;
			}

            break;

        case 0x15:  //-DCS_WRITE_1_PARM
            if(PACKEY_DEBUG)
            {
				printf("\n");
				printf("[W] : %#04x, 0x%02x, 0x%02x \n", PacketType, reg_buf[0], reg_buf[1]);
            }
			short_command.packet_type = PacketType;
			short_command.reg = reg_buf[0];
            short_command.data = reg_buf[1];
            ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM, &short_command);
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_DCS_WRITE_1_PARM\n");
				FUNC_END();
				return ret;
			}
            break;

        case 0x29:
        case 0x39:
        case 0x2C:
        case 0x3C:
        case 0x3e:
            if(PACKEY_DEBUG)
            {

	            printf("[W] : 0x%02x", PacketType);

		        for (i=0; i<len; i++){
			        printf(",0x%02x", reg_buf[i]);
				}
            printf("\n");
			}

            long_command.packet_type = PacketType;
			memcpy(long_command.data, reg_buf, len);
			long_command.count = len;

            if (len == 1)
            {
                long_command.data[1] = 0x00;
                long_command.count = 2;
            }

          	ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_LONG, &long_command);            
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_LONG\n");
				FUNC_END();
				return ret;
			}
			break;
	}
	FUNC_END();
	return 0;
}

int mipi_read(char PacketType, unsigned char RdReg, int len, unsigned char *RdData ){

	int i;
	int ret = PASS;
	
	FUNC_BEGIN();
    switch(PacketType)
    {

        //-Read Command
        case 0x04:
        case 0x14:
        case 0x24:
        case 0x06:

			read_command.packet_type = PacketType;
            read_command.reg = RdReg;
            read_command.len = len;
            ret = ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_GENERIC_READ_1_PARM, &read_command);
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_GENERIC_READ_1_PARM\n");
				FUNC_END();
				return ret;
			}
            ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_REGISTER_READ_DATA, RdData);
			if (ret < 0)
			{
				printf("ERR:_IOCTL_MIPI_CON_SET_REGISTER_READ_DATA\n");
				FUNC_END();
				return ret;
			}

			
			if(PACKEY_DEBUG)			
			{
				printf("\n");
				printf("[R] : 0x%02x,", RdReg);

	            for (i = 0; i < len; i++)
					printf(",0x%02x", RdData[i] & 0xFF);
				printf(" ");
			}

			break;

        default :
            printf("Error unkonwn 'Packet Type(MIPI Read)'\n");
            break;
        }

	FUNC_END();
    return ret;

}

void kernel_msg_in_mipi_write(int param)
{
	int no_kmsg = !param;
	FUNC_BEGIN();
	ioctl(mipi_dev,_IOCTL_MIPI_CON_NO_KERNEL_MSG_MIPI_WRITE, &no_kmsg);

	printf("mipi write kernel msg [%d] \n",param);
	FUNC_END();
}

int mipi_hs_clk_change(unsigned int hs_clk)
{
	FUNC_BEGIN();
	ioctl(mipi_dev, _IOCTL_CHANGE_HS_CLK, &hs_clk);
	printf("modify HS_CLK --> %u \n",hs_clk);
	FUNC_END();

	return 0;
}


////////////////////////////////////////////////////////////////////
//DISPLAY Func..//
///////////////////////////////////////////////////////////////////

void dp049_pocb_write(int	ch, unsigned char *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;

	FUNC_BEGIN();
    //MODE = 1 : PSM MODE / MODE  = 0 : NOT PSM MODE
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x5A;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);
        usleep(50000);

		printf("[DP049 POCB ON] \n");
	}
	else
	{
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x5A;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);
        usleep(50000);
		printf("[DP049 POCB OFF] \n");
	}
	mipi_port_set(DSI_PORT_SEL_BOTH); 
	FUNC_END();
}

void mv_joan_pocb_write(int   ch, unsigned char *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;

	FUNC_BEGIN();
        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x43;
        mipi_write(PacketType, reg_buf, 3);
        m_delay(BASIC_DELAY+OTPD);


    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xDF;
        reg_buf[1] = 0x42;
        reg_buf[2] = 0x70;
        mipi_write(PacketType, reg_buf, 3);
		m_delay(BASIC_DELAY+OTPD);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xA5;
        reg_buf[2] = 0x00;
        mipi_write(PacketType, reg_buf, 3);

        usleep(50000);

        printf("[MV_JOAN POCB ON] \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xDF;
        reg_buf[1] = 0x4A;
        reg_buf[2] = 0x70;
        mipi_write(PacketType, reg_buf, 3);
		m_delay(BASIC_DELAY+OTPD);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xA5;
        reg_buf[2] = 0x00;
        mipi_write(PacketType, reg_buf, 3);

        printf("[MV_JOAN POCB OFF] \n");
    }
    mipi_port_set(DSI_PORT_SEL_BOTH);
	FUNC_END();
}

void a1_pocb_write(int   ch, unsigned char *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;

	FUNC_BEGIN();
//need check

    memset(reg_buf, 0, sizeof(reg_buf));
    PacketType = 0x39;
	reg_buf[0] = 0xF0;
    reg_buf[1] = 0x5A;
    reg_buf[2] = 0x5A;
    mipi_write(PacketType, reg_buf, 3);
    usleep(1000);

    memset(reg_buf, 0, sizeof(reg_buf));
    PacketType = 0x39;
    reg_buf[0] = 0xF1;
    reg_buf[1] = 0x5A;
    reg_buf[2] = 0x5A;
    mipi_write(PacketType, reg_buf, 3);
    usleep(1000);

    memset(reg_buf, 0, sizeof(reg_buf));
    PacketType = 0x39;
    reg_buf[0] = 0xF2;
    reg_buf[1] = 0x5A;
    reg_buf[2] = 0x5A;
    mipi_write(PacketType, reg_buf, 3);
    usleep(1000);


    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x5A;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);
        usleep(50000);

        printf("[A1 POCB ON] \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x5A;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);
        usleep(50000);
        printf("[A1 POCB OFF] \n");
    }
    mipi_port_set(DSI_PORT_SEL_BOTH);
	FUNC_END();
}


void b1_pocb_write(int   ch, unsigned char *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;

	FUNC_BEGIN();
//need check

    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xEA;
        reg_buf[1] = 0x40;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xCA;
        mipi_write(PacketType, reg_buf, 2);


        //printf("[B1 POCB ON(IC Rev 0.3 : write[0xEA:0x10])] \n");
        printf("[B1 POCB ON(DVT : write[0xEA:0x40])] \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xEA;
        reg_buf[1] = 0xC0;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xCA;
        mipi_write(PacketType, reg_buf, 2);


        //printf("[B1 POCB OFF(IC Rev 0.3 : write[0xEA:0x90])] \n");
        printf("[B1 POCB OFF(DVT : write[0xEA:0xC0])] \n");
    }
    mipi_port_set(DSI_PORT_SEL_BOTH);
	FUNC_END();
}

int	pocb_write_control(int id,int ch, unsigned char *onoff)
{
	FUNC_BEGIN();
	//modify khl 180227
    if(!ch)
    {
        mipi_port_set(DSI_PORT_SEL_BOTH);
        printf(">> POCB write BOTH [%d] \n",(int)*onoff);
    }
    else if(ch == 1)
    {
        mipi_port_set(DSI_PORT_SEL_A);
        printf(">> POCB write A port [%d] \n",(int)*onoff);
    }
    else if(ch == 2)
    {
        mipi_port_set(DSI_PORT_SEL_B);
        printf(">> POCB write B port [%d] \n",(int)*onoff);
    }
    else
    {
        mipi_port_set(DSI_PORT_SEL_BOTH);
        printf(">> POCB write BOTH [%d] \n",(int)*onoff);
    }

	switch(id)
	{
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    MV :
        case    MV_MANUAL :
        case    JOAN_E5 :
        case    MV_MQA   :
        case    MV_DQA   :
			mv_joan_pocb_write(ch,onoff);
            break;
        case    A1 :
			a1_pocb_write(ch,onoff);
            break;
		case	DP049	:
		case	AKATSUKI	:
			dp049_pocb_write(ch,onoff);
			break;
        case    B1   :

			b1_pocb_write(ch,onoff);
			break;

		default	:
			printf("This Model is not use function of POCB Write [ID:%d/ ONOFF:%d]\n",id,*onoff);
			FUNC_END();
			return	FAIL;
	}

	FUNC_END();
	return PASS;
}


////////////////////////////////////////////////////////////////
//JOAN_MV
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int joan_dsc_err_flag_check(int id, int model_index, unsigned char *ch1_crc, unsigned char *ch2_crc)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int DataLen = 3;
    unsigned char RdReg;
    int ret = 0;
    int ret2 = 0;
	int ch = 0;
	unsigned char *crc;
	unsigned short ioctl_cmd = 0;
	unsigned int err_count = 0;
	char cmd[300] = {0,};

	FUNC_BEGIN();

    mipi_dev_open();
    decon_dev_open();
printf("dic_dev_open [%d] \n",dicOpen);
	

	configure_dsi_read();	

	memset(reg_buf,0,sizeof(reg_buf));
    /* MCS ACCESS */
    PacketType = 0x39;
    reg_buf[0] = 0xB0;
    reg_buf[1] = 0xA5;
    reg_buf[2] = 0x00;
    mipi_write(PacketType, reg_buf, 3);
    m_delay(BASIC_DELAY);

	memset(reg_buf,0,sizeof(reg_buf));
    /* TE CONTROL */
    PacketType = 0x39;
    reg_buf[0] = 0xED;
    reg_buf[1] = 0x13;
    reg_buf[2] = 0x00;
    reg_buf[3] = 0x06;  //06h:Pulse-Type, 07h:Flag-Type
    reg_buf[4] = 0x00;
    reg_buf[5] = 0x00;
    mipi_write(PacketType, reg_buf, 6);
    m_delay(BASIC_DELAY);

	for(ch = 1; ch < 3; ch++)
	{
		err_count = 100;

		if(ch == 1)
		{
			mipi_port_set(DSI_PORT_SEL_A);	
			crc = ch1_crc;
			ioctl_cmd = _IOCTL_CH1_TE_GET;
		}
		else
		{
			mipi_port_set(DSI_PORT_SEL_B);	
			crc = ch2_crc;
			ioctl_cmd = _IOCTL_CH2_TE_GET;
		}

		m_delay(3);

		while(1)
		{
			usleep(1000);

		    ioctl(dic_dev,ioctl_cmd,&ret2);  //TE Interrupt By Polling

			if(!err_count)
			{
                printf("\n");
                printf("**************************************************************\n");
				printf("Last....\n");
                printf("**************************************************************\n");
                memset(reg_buf, 0, sizeof(reg_buf));
                PacketType = 0x06;
                RdReg = 0x9F;
                ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
                if(ret < 0)
                {
					memset(reg_buf, 0, sizeof(reg_buf));
                    printf("\n");
                    printf("**************************************************************\n");
                    printf("MIPI READ FAIL[CH%d]... \n",ch);
                    printf("**************************************************************\n");
                    break;
                }
                m_delay(BASIC_DELAY);
                printf("\n");
                printf("**************************************************************\n");
                printf("ERROR FLAG[CH%d] : 0x%02X \n",ch,reg_buf[0]);
                printf("**************************************************************\n");
				break;
			}

	        if(ret2>0)
	        {
				memset(reg_buf,0,sizeof(reg_buf));
				printf("\n");
				printf("TE READ![%d] \n",ret2);
	            memset(reg_buf, 0, sizeof(reg_buf));
	            PacketType = 0x06;
	            RdReg = 0x9F;
	            ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);

	            if(ret < 0)
				{
                    printf("\n");
                    printf("**************************************************************\n");
					printf("MIPI READ FAIL[CH%d]... \n",ch);
                    printf("**************************************************************\n");
	                break;
				}
	            m_delay(BASIC_DELAY);

			        printf("\n");
					printf("**************************************************************\n");
			        printf("ERROR FLAG[CH%d] : 0x%02X \n",ch,reg_buf[0]);
			        printf("**************************************************************\n");
	            break;
	        }
			else
			{
				err_count--;
			}
	    }
		*crc = reg_buf[0];
		ret2 = 0;
		ret = 0;
		memset(reg_buf,0,sizeof(reg_buf));
	}

	mipi_port_set(DSI_PORT_SEL_BOTH);
	m_delay(3);
	
    mipi_dev_close();
		decon_dev_close();
	if((id != A1)&&(id != DP049) &&(id != AKATSUKI) )
	{
		sprintf(cmd,"%s%d%s",RECOVERY_CMD1,model_index,RECOVERY_CMD2);
		printf("%s\n",cmd);
		system(cmd);
	}
	FUNC_END();
    return 0;

}

void mv_mqa_vr_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    int i;

	FUNC_BEGIN();
    if(*onoff)
    {
        for(i=0; i<5; i++)
        {
            PacketType = 0x15;
            reg_buf[0] = 0x53;
            reg_buf[1] = 0x08;
            mipi_write(PacketType, reg_buf, 2);
            usleep(50000);
        }
        printf("MV MQR VR ON \n");
    }
    else
    {
        // Added by iamozzi. 2017.09.19.
        system("/Data/Pattern 11");
        usleep(3000);
        for(i=0; i<5; i++)
        {
            PacketType = 0x15;
            reg_buf[0] = 0x53;
            reg_buf[1] = 0x00;
            mipi_write(PacketType, reg_buf, 2);
            usleep(50000);
        }

        // Added by iamozzi. 2017.09.19.
        system("/Data/Pattern 11");
        usleep(3000);

        printf("MV MQR VR OFF \n");

    }
    usleep(100000);
    printf("VR WRITE \n");
	FUNC_END();
}

void joan_vr_mode_onoff(int id, int model_index, int *onoff, int mode)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
	//MODE = 1 : PSM MODE / MODE  = 0 : NOT PSM MODE
    if(*onoff)
    {
		memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x0B;
        reg_buf[17] = 0x68;
        reg_buf[18] = 0x00;
        reg_buf[19] = 0x0B;
        reg_buf[20] = 0x68;
        reg_buf[21] = 0x00;
        reg_buf[22] = 0x0B;
        reg_buf[23] = 0x68;
        reg_buf[24] = 0x00;
        reg_buf[25] = 0x0B;
        reg_buf[26] = 0x68;
        reg_buf[27] = 0x00;
        reg_buf[28] = 0x0B;
        reg_buf[29] = 0x68;
        reg_buf[30] = 0x00;
        mipi_write(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write(PacketType, reg_buf, 19);

        usleep(20000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x53;
        reg_buf[1] = 0x08;
		mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x45;
        reg_buf[2] = 0xC5;
        reg_buf[3] = 0x04;
        reg_buf[4] = 0x88;
        reg_buf[5] = 0xD3;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0A;
        reg_buf[8] = 0x0B;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0A;
        reg_buf[11] = 0x0B;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x05;
        reg_buf[14] = 0x0B;
        mipi_write(PacketType, reg_buf, 15);

		memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCA;
        reg_buf[1] = 0x00;
		if(!mode)
			reg_buf[2] = 0x06;
		else
		    reg_buf[2] = 0x36;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x06;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x06;
        reg_buf[7] = 0x11;
		mipi_write(PacketType, reg_buf, 8);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x9F;
        mipi_write(PacketType, reg_buf, 2);

        usleep(20000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
		if(!mode)
		{
            reg_buf[1] = 0xB0;
            reg_buf[2] = 0x04;
            reg_buf[3] = 0x5B;
            reg_buf[4] = 0xB0;
            reg_buf[5] = 0x04;
            reg_buf[6] = 0x5C;
            reg_buf[7] = 0xB0;
            reg_buf[8] = 0x04;
            reg_buf[9] = 0x5C;
            reg_buf[10] = 0xB0;
            reg_buf[11] = 0x04;
            reg_buf[12] = 0x5C;
            reg_buf[13] = 0xB0;
            reg_buf[14] = 0x04;
            reg_buf[15] = 0x5C;
            reg_buf[16] = 0xB0;
            reg_buf[17] = 0x04;
            reg_buf[18] = 0x5C;
            reg_buf[19] = 0xB0;
            reg_buf[20] = 0x04;
            reg_buf[21] = 0x5C;
            reg_buf[22] = 0xB0;
            reg_buf[23] = 0x04;
            reg_buf[24] = 0x5C;
            reg_buf[25] = 0xB0;
            reg_buf[26] = 0x04;
            reg_buf[27] = 0x5C;
            reg_buf[28] = 0xB0;
            reg_buf[29] = 0x04;
            reg_buf[30] = 0x5C;
		}
		else
		{
			reg_buf[1] = 0x20;
			reg_buf[2] = 0x04;
			reg_buf[3] = 0xD3;
			reg_buf[4] = 0x20;
			reg_buf[5] = 0x04;
			reg_buf[6] = 0xD3;
			reg_buf[7] = 0x20;
			reg_buf[8] = 0x04;
			reg_buf[9] = 0xD3;
			reg_buf[10] = 0x20;
			reg_buf[11] = 0x04;
			reg_buf[12] = 0xD3;
			reg_buf[13] = 0x20;
			reg_buf[14] = 0x04;
			reg_buf[15] = 0xD3;
			reg_buf[16] = 0x20;
			reg_buf[17] = 0x04;
			reg_buf[18] = 0xD3;
			reg_buf[19] = 0x20;
			reg_buf[20] = 0x04;
			reg_buf[21] = 0xD3;
			reg_buf[22] = 0x11;
			reg_buf[23] = 0xB5;
			reg_buf[24] = 0x24;
			reg_buf[25] = 0x02;
			reg_buf[26] = 0x9D;
			reg_buf[27] = 0x3B;
			reg_buf[28] = 0x02;
			reg_buf[29] = 0xC2;
			reg_buf[30] = 0x16;
		}
        mipi_write(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCC;
		if(!mode)
		{
            reg_buf[1] = 0xB0;
            reg_buf[2] = 0x04;
            reg_buf[3] = 0x5C;
            reg_buf[4] = 0xB0;
            reg_buf[5] = 0x04;
            reg_buf[6] = 0x5C;
            reg_buf[7] = 0xB0;
            reg_buf[8] = 0x04;
            reg_buf[9] = 0x5C;
            reg_buf[10] = 0x37;
            reg_buf[11] = 0xF4;
            reg_buf[12] = 0x6A;
            reg_buf[13] = 0x37;
            reg_buf[14] = 0xF4;
            reg_buf[15] = 0x6A;
            reg_buf[16] = 0x55;
            reg_buf[17] = 0x12;
            reg_buf[18] = 0x13;
		}
		else
		{
			reg_buf[1] = 0x0B;
			reg_buf[2] = 0x04;
			reg_buf[3] = 0x5C;
			reg_buf[4] = 0xB0;
			reg_buf[5] = 0x04;
			reg_buf[6] = 0x5C;
			reg_buf[7] = 0xB0;
			reg_buf[8] = 0x04;
			reg_buf[9] = 0x5C;
			reg_buf[10] = 0x29;
			reg_buf[11] = 0x18;
			reg_buf[12] = 0x46;
			reg_buf[13] = 0x29;
			reg_buf[14] = 0x18;
			reg_buf[15] = 0x46;
			reg_buf[16] = 0x55;
			reg_buf[17] = 0x12;
			reg_buf[18] = 0x13;
		}
		
		mipi_write(PacketType, reg_buf, 19);

        printf("JOAN VR ON \n");
    }
    else
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xEF;

		memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x0B;
        reg_buf[17] = 0x68;
        reg_buf[18] = 0x00;
        reg_buf[19] = 0x0B;
        reg_buf[20] = 0x68;
        reg_buf[21] = 0x00;
        reg_buf[22] = 0x0B;
        reg_buf[23] = 0x68;
        reg_buf[24] = 0x00;
        reg_buf[25] = 0x0B;
        reg_buf[26] = 0x68;
        reg_buf[27] = 0x00;
        reg_buf[28] = 0x0B;
        reg_buf[29] = 0x68;
        reg_buf[30] = 0x00;
        mipi_write(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write(PacketType, reg_buf, 19);

        usleep(20000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x53;
        reg_buf[1] = 0x00;
		mipi_write(PacketType, reg_buf, 2);

		memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCA;
        reg_buf[1] = 0x00;
        if(!mode)
            reg_buf[2] = 0x06;
        else
	        reg_buf[2] = 0x36;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x06;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x06;
        reg_buf[7] = 0x10;
		mipi_write(PacketType, reg_buf, 8);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x45;
        reg_buf[2] = 0xC5;
        reg_buf[3] = 0x04;
        reg_buf[4] = 0x88;
        reg_buf[5] = 0xD1;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0A;
        reg_buf[8] = 0x0B;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0A;
        reg_buf[11] = 0x0B;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x05;
        reg_buf[14] = 0x0B;
        mipi_write(PacketType, reg_buf, 15);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write(PacketType, reg_buf, 2);

        usleep(20000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x04;
        reg_buf[3] = 0xD3;
        reg_buf[4] = 0x20;
        reg_buf[5] = 0x04;
        reg_buf[6] = 0xD3;
        reg_buf[7] = 0x20;
        reg_buf[8] = 0x04;
        reg_buf[9] = 0xD3;
        reg_buf[10] = 0x20;
        reg_buf[11] = 0x04;
        reg_buf[12] = 0xD3;
        reg_buf[13] = 0x20;
        reg_buf[14] = 0x04;
        reg_buf[15] = 0xD3;
        reg_buf[16] = 0x20;
        reg_buf[17] = 0x04;
        reg_buf[18] = 0xD3;
        reg_buf[19] = 0x20;
        reg_buf[20] = 0x04;
        reg_buf[21] = 0xD3;
        reg_buf[22] = 0x11;
        reg_buf[23] = 0xB5;
        reg_buf[24] = 0x24;
        reg_buf[25] = 0x02;
        reg_buf[26] = 0x9D;
        reg_buf[27] = 0x3B;
        reg_buf[28] = 0x02;
        reg_buf[29] = 0xC2;
        reg_buf[30] = 0x16;
        mipi_write(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x04;
        reg_buf[3] = 0x5C;
        reg_buf[4] = 0xB0;
        reg_buf[5] = 0x04;
        reg_buf[6] = 0x5C;
        reg_buf[7] = 0xB0;
        reg_buf[8] = 0x04;
        reg_buf[9] = 0x5C;
        reg_buf[10] = 0x29;
        reg_buf[11] = 0x18;
        reg_buf[12] = 0x46;
        reg_buf[13] = 0x29;
        reg_buf[14] = 0x18;
        reg_buf[15] = 0x46;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write(PacketType, reg_buf, 19);

        printf("JOAN VR OFF \n");
    }
	usleep(100000);
    printf("VR WRITE \n");
	FUNC_END();
}

void mv_vr_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
	int i;

	FUNC_BEGIN();
    if(*onoff)
    {
        for(i=0; i<5; i++)
        {
            PacketType = 0x15;
            reg_buf[0] = 0x53;
            reg_buf[1] = 0x08;
            mipi_write(PacketType, reg_buf, 2);
            usleep(50000);
        }
        printf("MV VR ON \n");
    }
    else
    {

        // Added by iamozzi. 2017.09.19.
        //system("/Data/Pattern 11");
        system("/Data/Pattern 11 B1"); //180605
        usleep(3000);
        for(i=0; i<5; i++)
        {
            PacketType = 0x15;
            reg_buf[0] = 0x53;
            reg_buf[1] = 0x00;
            mipi_write(PacketType, reg_buf, 2);
            usleep(50000);
        }

        // Added by iamozzi. 2017.09.19.
        //system("/Data/Pattern 11");
        system("/Data/Pattern 11 B1"); //180605
        usleep(3000);

        printf("MV VR OFF \n");

    }
	usleep(100000);
    printf("VR WRITE \n");
	FUNC_END();
}
void a1_vr_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xFE;

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);


        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);


        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB0;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);


    }
    else
    {

        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFE;

        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);


        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);


        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB3;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write(PacketType, reg_buf, 2);
    }
	FUNC_END();
}

void joan_emcon_mode_onoff(int id, int *onoff, int mode)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
		usleep(800000);
        memset(reg_buf,0,sizeof(reg_buf));

		PacketType = 0x39;
        reg_buf[0] = 0xCA;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x06;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x06;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x16;
        reg_buf[7] = 0x10;
        mipi_write_delay(PacketType, reg_buf, 8);
        mipi_write_delay(PacketType, reg_buf, 8);


        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x0B;
        reg_buf[17] = 0x68;
        reg_buf[18] = 0x00;
        reg_buf[19] = 0x0B;
        reg_buf[20] = 0x68;
        reg_buf[21] = 0x00;
        reg_buf[22] = 0x0B;
        reg_buf[23] = 0x68;
        reg_buf[24] = 0x00;
        reg_buf[25] = 0x0B;
        reg_buf[26] = 0x68;
        reg_buf[27] = 0x00;
        reg_buf[28] = 0x0B;
        reg_buf[29] = 0x68;
        reg_buf[30] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 31);
        mipi_write_delay(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write_delay(PacketType, reg_buf, 19);
        mipi_write_delay(PacketType, reg_buf, 19);

		printf("JOAN EM CONTROL : EM OFF (PWM[%d])\n",mode);
	}
	else
	{
		if(mode)
		{
			memset(reg_buf,0,sizeof(reg_buf));

			PacketType = 0x39;
			reg_buf[0] = 0xCA;
			reg_buf[1] = 0x00;
			reg_buf[2] = 0x36;
			reg_buf[3] = 0x00;
			reg_buf[4] = 0x06;
			reg_buf[5] = 0x00;
			reg_buf[6] = 0x16;
			reg_buf[7] = 0x10;
			mipi_write_delay(PacketType, reg_buf, 15);
	        mipi_write_delay(PacketType, reg_buf, 15);

			memset(reg_buf,0,sizeof(reg_buf));

			PacketType = 0x39;
			reg_buf[0] = 0xCB;
			reg_buf[1] = 0x20;
			reg_buf[2] = 0x04;
			reg_buf[3] = 0xD3;
			reg_buf[4] = 0x20;
			reg_buf[5] = 0x04;
			reg_buf[6] = 0xD3;
			reg_buf[7] = 0x20;
			reg_buf[8] = 0x04;
			reg_buf[9] = 0xD3;
			reg_buf[10] = 0x20;
			reg_buf[11] = 0x04;
			reg_buf[12] = 0xD3;
			reg_buf[13] = 0x20;
			reg_buf[14] = 0x04;
			reg_buf[15] = 0xD3;
			reg_buf[16] = 0x20;
			reg_buf[17] = 0x04;
			reg_buf[18] = 0xD3;
			reg_buf[19] = 0x20;
			reg_buf[20] = 0x04;
			reg_buf[21] = 0xD3;
			reg_buf[22] = 0x11;
			reg_buf[23] = 0xB5;
			reg_buf[24] = 0x24;
			reg_buf[25] = 0x02;
			reg_buf[26] = 0x9D;
			reg_buf[27] = 0x3B;
			reg_buf[28] = 0x02;
			reg_buf[29] = 0xC2;
			reg_buf[30] = 0x16;
			mipi_write_delay(PacketType, reg_buf, 31);
			mipi_write_delay(PacketType, reg_buf, 31);

			memset(reg_buf,0,sizeof(reg_buf));

			PacketType = 0x39;
			reg_buf[0] = 0xCC;
			reg_buf[1] = 0xB0;
			reg_buf[2] = 0x04;
			reg_buf[3] = 0x5C;
			reg_buf[4] = 0xB0;
			reg_buf[5] = 0x04;
			reg_buf[6] = 0x5C;
			reg_buf[7] = 0xB0;
			reg_buf[8] = 0x04;
			reg_buf[9] = 0x5C;
			reg_buf[10] = 0x29;
			reg_buf[11] = 0x18;
			reg_buf[12] = 0x1E;
			reg_buf[13] = 0x29;
			reg_buf[14] = 0x18;
			reg_buf[15] = 0x1E;
			reg_buf[16] = 0x55;
			reg_buf[17] = 0x12;
			reg_buf[18] = 0x13;
	        mipi_write_delay(PacketType, reg_buf, 19);
		    mipi_write_delay(PacketType, reg_buf, 19);

			printf("JOAN EM CONTROL : EM ON (PWM[%d])\n",mode);
		}
		else
		{
            memset(reg_buf,0,sizeof(reg_buf));

            PacketType = 0x39;
            reg_buf[0] = 0xCA;
            reg_buf[1] = 0x00;
            reg_buf[2] = 0x06;
            reg_buf[3] = 0x00;
            reg_buf[4] = 0x06;
            reg_buf[5] = 0x00;
            reg_buf[6] = 0x16;
            reg_buf[7] = 0x10;
	        mipi_write_delay(PacketType, reg_buf, 8);
		    mipi_write_delay(PacketType, reg_buf, 8);

            memset(reg_buf,0,sizeof(reg_buf));

            PacketType = 0x39;
            reg_buf[0] = 0xCB;
            reg_buf[1] = 0xB0;
            reg_buf[2] = 0x04;
            reg_buf[3] = 0x5B;
            reg_buf[4] = 0xB0;
            reg_buf[5] = 0x04;
            reg_buf[6] = 0x5C;
            reg_buf[7] = 0xB0;
            reg_buf[8] = 0x04;
            reg_buf[9] = 0x5C;
            reg_buf[10] = 0xB0;
            reg_buf[11] = 0x04;
            reg_buf[12] = 0x5C;
            reg_buf[13] = 0xB0;
            reg_buf[14] = 0x04;
            reg_buf[15] = 0x5C;
            reg_buf[16] = 0xB0;
            reg_buf[17] = 0x04;
            reg_buf[18] = 0x5C;
            reg_buf[19] = 0xB0;
            reg_buf[20] = 0x04;
            reg_buf[21] = 0x5C;
            reg_buf[22] = 0xB0;
            reg_buf[23] = 0x04;
            reg_buf[24] = 0x5C;
            reg_buf[25] = 0xB0;
            reg_buf[26] = 0x04;
            reg_buf[27] = 0x5C;
            reg_buf[28] = 0xB0;
            reg_buf[29] = 0x04;
            reg_buf[30] = 0x5C;
	        mipi_write_delay(PacketType, reg_buf, 31);
		    mipi_write_delay(PacketType, reg_buf, 31);

            PacketType = 0x39;
            reg_buf[0] = 0xCC;
            reg_buf[1] = 0xB0;
            reg_buf[2] = 0x04;
            reg_buf[3] = 0x5C;
            reg_buf[4] = 0xB0;
            reg_buf[5] = 0x04;
            reg_buf[6] = 0x5C;
            reg_buf[7] = 0xB0;
            reg_buf[8] = 0x04;
            reg_buf[9] = 0x5C;
            reg_buf[10] = 0x29;
            reg_buf[11] = 0x18;
            reg_buf[12] = 0x1E;
            reg_buf[13] = 0x29;
            reg_buf[14] = 0x18;
            reg_buf[15] = 0x1E;
            reg_buf[16] = 0x55;
            reg_buf[17] = 0x12;
            reg_buf[18] = 0x13;
	        mipi_write_delay(PacketType, reg_buf, 15);
		    mipi_write_delay(PacketType, reg_buf, 15);

			printf("JOAN EM CONTROL : EM ON (PWM[%d])\n",mode);
		}
	}
	FUNC_END();
}


void akatsuki_vr_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x15;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB0;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0x55;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x01;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("AKATSUKI VR ON \n");
    }
    else
    {
        found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x15;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB3;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv1;
        reg_buf[2] = reg_dbv2;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x15;
        reg_buf[0] = 0x55;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("AKATSUKI VR OFF \n");
    }
	FUNC_END();
}

void b1_vr_mode_onoff(int id, int model_index, int *onoff, char* pic_cmd)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

		usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x55;
        reg_buf[1] = 0x04;
        reg_buf[2] = 0x70;
        reg_buf[3] = 0xDB;
        reg_buf[4] = 0x04;
        reg_buf[5] = 0x70;
        reg_buf[6] = 0xDB;
        mipi_write(PacketType, reg_buf, 7);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x01;
        reg_buf[2] = 0xFF;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x53;
        reg_buf[1] = 0x2C;
        reg_buf[2] = 0x30;
        mipi_write(PacketType, reg_buf, 3);

		system(pic_cmd);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

		usleep(100000);

        printf("B1 VR ON \n");
    }
    else
    {
		found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

		usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x53;
        reg_buf[1] = 0x0C;
        reg_buf[2] = 0x30;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv1;
        reg_buf[2] = reg_dbv2;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x55;
        //reg_buf[1] = 0x34;
        reg_buf[1] = 0x04; //test -180515
        reg_buf[2] = 0x61;
        //reg_buf[3] = 0xCB;
        reg_buf[3] = 0xDB; //test -180515
        reg_buf[4] = 0x04;
        reg_buf[5] = 0x70;
        reg_buf[6] = 0xDB;
        mipi_write(PacketType, reg_buf, 7);

        st_is_remain_code_after_next_pt |= 1<< B1_VR_OFF;
        printf("%s: remain_code_after_next_pt is set [0x%X]\n",__func__,st_is_remain_code_after_next_pt);

        printf("B1 VR OFF \n");
    }
	FUNC_END();
}

void mv_emcon_mode_onoff(int id, int *onoff, int mode)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        usleep(800000);
        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xA5;
        reg_buf[2] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 3);
        mipi_write_delay(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCA;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x06;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x06;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x16;
        reg_buf[7] = 0x10;
        mipi_write_delay(PacketType, reg_buf, 8);
        mipi_write_delay(PacketType, reg_buf, 8);

        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x0B;
        reg_buf[17] = 0x68;
        reg_buf[18] = 0x00;
        reg_buf[19] = 0x0B;
        reg_buf[20] = 0x68;
        reg_buf[21] = 0x00;
        reg_buf[22] = 0x0B;
        reg_buf[23] = 0x68;
        reg_buf[24] = 0x00;
        reg_buf[25] = 0x0B;
        reg_buf[26] = 0x68;
        reg_buf[27] = 0x00;
        reg_buf[28] = 0x0B;
        reg_buf[29] = 0x68;
        reg_buf[30] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 31);
        mipi_write_delay(PacketType, reg_buf, 31);

		memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x0B;
        reg_buf[2] = 0x68;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x68;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0B;
        reg_buf[8] = 0x68;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x0B;
        reg_buf[11] = 0x68;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x0B;
        reg_buf[14] = 0x68;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write_delay(PacketType, reg_buf, 19);
        mipi_write_delay(PacketType, reg_buf, 19);

		usleep(40000);

		printf("MV EM CONTROL : EM OFF \n");
	}
	else
	{
        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCA;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x46;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x46;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x16;
        reg_buf[7] = 0x10;
        mipi_write_delay(PacketType, reg_buf, 8);
        mipi_write_delay(PacketType, reg_buf, 8);

        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x04;
        reg_buf[3] = 0xD6;
        reg_buf[4] = 0x20;
        reg_buf[5] = 0x04;
        reg_buf[6] = 0xD6;
        reg_buf[7] = 0x20;
        reg_buf[8] = 0x04;
        reg_buf[9] = 0xD6;
        reg_buf[10] = 0x20;
        reg_buf[11] = 0x04;
        reg_buf[12] = 0xD6;
        reg_buf[13] = 0x20;
        reg_buf[14] = 0x04;
        reg_buf[15] = 0xD6;
        reg_buf[16] = 0x10;
        reg_buf[17] = 0xF8;
        reg_buf[18] = 0xE1;
        reg_buf[19] = 0x10;
        reg_buf[20] = 0xF8;
        reg_buf[21] = 0xE1;
        reg_buf[22] = 0x02;
        reg_buf[23] = 0x73;
        reg_buf[24] = 0x66;
        reg_buf[25] = 0x02;
        reg_buf[26] = 0x73;
        reg_buf[27] = 0x66;
        reg_buf[28] = 0x02;
        reg_buf[29] = 0x73;
        reg_buf[30] = 0x66;
        mipi_write_delay(PacketType, reg_buf, 31);
        mipi_write_delay(PacketType, reg_buf, 31);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCC;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x04;
        reg_buf[3] = 0xD6;
        reg_buf[4] = 0x20;
        reg_buf[5] = 0x04;
        reg_buf[6] = 0xD6;
        reg_buf[7] = 0x02;
        reg_buf[8] = 0xBC;
        reg_buf[9] = 0x1D;
        reg_buf[10] = 0xB0;
        reg_buf[11] = 0x0F;
        reg_buf[12] = 0x57;
        reg_buf[13] = 0x37;
        reg_buf[14] = 0xFB;
        reg_buf[15] = 0x6B;
        reg_buf[16] = 0x55;
        reg_buf[17] = 0x12;
        reg_buf[18] = 0x13;
        mipi_write_delay(PacketType, reg_buf, 19);
        mipi_write_delay(PacketType, reg_buf, 19);

        usleep(84000);

		printf("MV EM CONTROL : EM ON \n");
	}
	FUNC_END();
}

void a1_emcon_mode_onoff(int id, int *onoff, int mode)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        usleep(800000);
        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB3;
        reg_buf[2] = 0xFF;
        reg_buf[3] = 0xC0;
        reg_buf[4] = 0x0B;
        reg_buf[5] = 0x32;
        reg_buf[6] = 0xCC;
        reg_buf[7] = 0x73;
        reg_buf[8] = 0x1C;
        reg_buf[9] = 0xC0;
        reg_buf[10] = 0x33;
        reg_buf[11] = 0x0C;
        reg_buf[12] = 0xC0;
        reg_buf[13] = 0xA3;
        reg_buf[14] = 0x32;
        reg_buf[15] = 0x33;
        reg_buf[16] = 0x0D;
        reg_buf[17] = 0x60;
        mipi_write_delay(PacketType, reg_buf, 18);
        mipi_write_delay(PacketType, reg_buf, 18);

        printf("MV EM CONTROL : EM OFF \n");
	}
	else
	{
        memset(reg_buf,0,sizeof(reg_buf));

        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0xB3;
        reg_buf[2] = 0xFF;
        reg_buf[3] = 0xFF;
        reg_buf[4] = 0xFB;
        reg_buf[5] = 0x32;
        reg_buf[6] = 0xCC;
        reg_buf[7] = 0x73;
        reg_buf[8] = 0x1C;
        reg_buf[9] = 0xC0;
        reg_buf[10] = 0x33;
        reg_buf[11] = 0x0C;
        reg_buf[12] = 0xC0;
        reg_buf[13] = 0xA3;
        reg_buf[14] = 0x32;
        reg_buf[15] = 0x33;
        reg_buf[16] = 0x0D;
        reg_buf[17] = 0x60;
        mipi_write_delay(PacketType, reg_buf, 18);
        mipi_write_delay(PacketType, reg_buf, 18);

        printf("A1 EM CONTROL : EM ON \n");
	}
	FUNC_END();
}

void dp049_emcon_mode_onoff(int id,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x02;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0x40;
        reg_buf[2] = 0x0F;
        mipi_write(PacketType, reg_buf, 3);

        printf("DP049 EMOFF ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x02;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xBF;
        reg_buf[1] = 0x7F;
        reg_buf[2] = 0xDF;
        mipi_write(PacketType, reg_buf, 3);

        printf("DP049 EMOFF OFF \n");
    }
	FUNC_END();
}

void b1_emcon_mode_onoff(int id,int *onoff, char* pic_cmd)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x00;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x00;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x00;
        reg_buf[10] = 0x00;
        reg_buf[11] = 0x00;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x00;
        reg_buf[14] = 0x00;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x00;

        mipi_write(PacketType, reg_buf, 19);

		usleep(50000);
	}
	else
	{
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x2D;
        reg_buf[4] = 0x20;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x2D;
        reg_buf[7] = 0x20;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x2D;
        reg_buf[10] = 0x20;
        reg_buf[11] = 0x00;
        reg_buf[12] = 0x2D;
        reg_buf[13] = 0x20;
        reg_buf[14] = 0x00;
        reg_buf[15] = 0x2D;
        reg_buf[16] = 0x10;
        reg_buf[17] = 0xBD;
        reg_buf[18] = 0x6F;

		mipi_write(PacketType, reg_buf, 19);
	}
	FUNC_END();
}

void mv_aod_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv = 0xFF;

	FUNC_BEGIN();
    if(*onoff)
    {

        PacketType = 0x05;
        reg_buf[0] = 0x22;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x15;
        reg_buf[0] = 0xE0;
        reg_buf[1] = 0x1A;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        usleep(52000);

        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x33;
        reg_buf[2] = 0x04;
        mipi_write_delay(PacketType, reg_buf, 3);
        mipi_write_delay(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x03;
        mipi_write_delay(PacketType, reg_buf, 3);
        mipi_write_delay(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xE5;
        reg_buf[1] = 0x04;
        reg_buf[2] = 0x06;
        reg_buf[3] = 0x03;
        reg_buf[4] = 0x03;
        reg_buf[5] = 0x56;
        reg_buf[6] = 0x61;
        mipi_write_delay(PacketType, reg_buf, 7);
        mipi_write_delay(PacketType, reg_buf, 7);

        PacketType = 0x39;
        reg_buf[0] = 0xE7;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x76;
        reg_buf[4] = 0x23;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x0D;
        reg_buf[8] = 0x44;
        mipi_write_delay(PacketType, reg_buf, 9);
        mipi_write_delay(PacketType, reg_buf, 9);

        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x10;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        usleep(84000);

        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x30;
        reg_buf[2] = 0x04;
        mipi_write_delay(PacketType, reg_buf, 3);
        mipi_write_delay(PacketType, reg_buf, 3);

        PacketType = 0x05;
        reg_buf[0] = 0x13;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x15;
        reg_buf[0] = 0x5E;
        reg_buf[1] = 0x10;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xFF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

		printf("MV AOD ON \n");
	}
	else
	{
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFF;

        //system("/Data/Pattern 11");
		if((id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5))
			system("/Data/Pattern 11 B1");
		else
	        system("/Data/Pattern 11"); //180605
        usleep(3000);

        PacketType = 0x15;
        reg_buf[0] = 0x5E;
        reg_buf[1] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        usleep(18000);

        PacketType = 0x05;
        reg_buf[0] = 0x22;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        mipi_write_delay(PacketType, reg_buf, 3);
        mipi_write_delay(PacketType, reg_buf, 3);

        usleep(64000);

        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x05;
        reg_buf[0] = 0x13;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write_delay(PacketType, reg_buf, 1);
        mipi_write_delay(PacketType, reg_buf, 1);

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        //reg_buf[1] = 0xFF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

		printf("MV AOD OFF \n");
	}

	printf("AOD WRITE \n");
	FUNC_END();
}

void joan_aod_mode_onoff(int id,int model_index, int *onoff,int mode)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
	//mode = 1 : PSM MODE / mode = 0 : NOT PSM MODE
	printf("JOAN MODE : %d \n mode = 2 : JOAN_E5 PSM MODE / mode = 1 : PSM MODE / mode = 0 : NOT PSM MODE \n",mode);

    if(*onoff)
    {

		//system("/Data/Pattern 11"); //need black.. in PSM mode
        system("/Data/Pattern 11 B1"); //180605

#if	1	/* swchoi - add new code before entering AOD code by LGD request - 20180709 */
		if(mode == 3)	/* only for JOAN_E5 and only for new AOD image in JOAN_E5 */
		{
			memset(reg_buf,0,sizeof(reg_buf));
			
			PacketType = 0x15;
			reg_buf[0] = 0x51;
			reg_buf[1] = 0x38;
			mipi_write_delay(PacketType, reg_buf, 2);
			printf("\nNew AOD code is set for EMMA(JOAN_E5)\n");
		}
#endif	/* swchoi - end */
		
		memset(reg_buf,0,sizeof(reg_buf));
		
		PacketType = 0x39;
		reg_buf[0] = 0xB0;
		reg_buf[1] = 0xA5;
		reg_buf[2] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 3);
		
		memset(reg_buf,0,sizeof(reg_buf));
		
		PacketType = 0x39;
		reg_buf[0] = 0xD2;
		reg_buf[1] = 0x1A;
		reg_buf[2] = 0xBC;
		reg_buf[3] = 0x02;
		reg_buf[4] = 0xBC;
		reg_buf[5] = 0x01;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x06;
		reg_buf[8] = 0xFF;
		reg_buf[9] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 10);
		
		memset(reg_buf,0,sizeof(reg_buf));
		
		PacketType = 0x39;
		reg_buf[0] = 0xCA;
		reg_buf[1] = 0x00;
		if(!mode)
			reg_buf[2] = 0x06;
		else
		    reg_buf[2] = 0x36;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x06;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x16;
		reg_buf[7] = 0x10;
		mipi_write_delay(PacketType, reg_buf, 8);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCB;
		reg_buf[1] = 0x0B;
		reg_buf[2] = 0x68;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x0B;
		reg_buf[5] = 0x68;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x0B;
		reg_buf[8] = 0x68;
		reg_buf[9] = 0x00;
		reg_buf[10] = 0x0B;
		reg_buf[11] = 0x68;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x0B;
		reg_buf[14] = 0x68;
		reg_buf[15] = 0x00;
		reg_buf[16] = 0x0B;
		reg_buf[17] = 0x68;
		reg_buf[18] = 0x00;
		reg_buf[19] = 0x0B;
		reg_buf[20] = 0x68;
		reg_buf[21] = 0x00;
		reg_buf[22] = 0x0B;
		reg_buf[23] = 0x68;
		reg_buf[24] = 0x00;
		reg_buf[25] = 0x0B;
		reg_buf[26] = 0x68;
		reg_buf[27] = 0x00;
		reg_buf[28] = 0x0B;
		reg_buf[29] = 0x68;
		reg_buf[30] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 31);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCC;
		reg_buf[1] = 0x0B;
		reg_buf[2] = 0x68;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x0B;
		reg_buf[5] = 0x68;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x0B;
		reg_buf[8] = 0x68;
		reg_buf[9] = 0x00;
		reg_buf[10] = 0x0B;
		reg_buf[11] = 0x68;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x0B;
		reg_buf[14] = 0x68;
		reg_buf[15] = 0x00;
		reg_buf[16] = 0x55;
		reg_buf[17] = 0x12;
		reg_buf[18] = 0x13;
		mipi_write_delay(PacketType, reg_buf, 19);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE5;
		reg_buf[1] = 0x04;
		reg_buf[2] = 0x04;
		reg_buf[3] = 0x03;
		reg_buf[4] = 0x03;
		reg_buf[5] = 0x56;
		reg_buf[6] = 0x21;
		reg_buf[7] = 0x33;
		reg_buf[8] = 0x0A;
		mipi_write_delay(PacketType, reg_buf, 9);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE7;
		reg_buf[1] = 0x00;
		reg_buf[2] = 0x0D;
		reg_buf[3] = 0x76;
		reg_buf[4] = 0x1F;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x0D;
		reg_buf[7] = 0x0D;
		reg_buf[8] = 0x44;
		reg_buf[9] = 0x0D;
		reg_buf[10] = 0x76;
		reg_buf[11] = 0x25;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x0D;
		reg_buf[14] = 0x0D;
		reg_buf[15] = 0x0D;
		reg_buf[16] = 0x0D;
		reg_buf[17] = 0x4A;
		reg_buf[18] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 19);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE2;
		reg_buf[1] = 0x20;
		reg_buf[2] = 0x03;
		reg_buf[3] = 0x08;
		reg_buf[4] = 0xA8;
		reg_buf[5] = 0x0A;
		reg_buf[6] = 0xAA;
		reg_buf[7] = 0x04;
		reg_buf[8] = 0xA4;
		reg_buf[9] = 0x80;
		reg_buf[10] = 0x80;
		reg_buf[11] = 0x80;
		reg_buf[12] = 0x5C;
		reg_buf[13] = 0x5C;
		reg_buf[14] = 0x5C;
		mipi_write_delay(PacketType, reg_buf, 15);
		
		if(mode == 2)
		{
		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x39;
		    reg_buf[0] = 0xE1;
		    reg_buf[1] = 0x00;
		    reg_buf[2] = 0x00;
		    reg_buf[3] = 0x00;
		    reg_buf[4] = 0x28;
		    reg_buf[5] = 0x28;
		    reg_buf[6] = 0x40;
		    reg_buf[7] = 0x17;
		    reg_buf[8] = 0x17;
		    reg_buf[9] = 0x01;
		    reg_buf[10] = 0x0C;
		    reg_buf[11] = 0x4F;
		    mipi_write_delay(PacketType, reg_buf, 12);
		}
		
		usleep(70000);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x34;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0x30;
		reg_buf[1] = 0x00;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x0B;
		reg_buf[4] = 0x3F;
		mipi_write_delay(PacketType, reg_buf, 5);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x31;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x5E;
		reg_buf[1] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 2);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x12;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE0;
		reg_buf[1] = 0x18;
		reg_buf[2] = 0x10;
		reg_buf[3] = 0x11;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x20;
		reg_buf[7] = 0x00;
		reg_buf[8] = 0x00;
		reg_buf[9] = 0x20;
		reg_buf[10] = 0x20;
		mipi_write_delay(PacketType, reg_buf, 11);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x39;
		mipi_write(PacketType, reg_buf, 1);
		usleep(85000);
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x35;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCA;
		reg_buf[1] = 0x00;
		if(!mode)
		    reg_buf[2] = 0x06;
		else
			reg_buf[2] = 0x36;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x06;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x16;
		reg_buf[7] = 0x10;
		mipi_write_delay(PacketType, reg_buf, 8);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCB;

		if(!mode)
		{
		    reg_buf[1] = 0xB0;
		    reg_buf[2] = 0x04;
		    reg_buf[3] = 0x5B;
		    reg_buf[4] = 0xB0;
		    reg_buf[5] = 0x04;
		    reg_buf[6] = 0x5C;
		    reg_buf[7] = 0xB0;
		    reg_buf[8] = 0x04;
		    reg_buf[9] = 0x5C;
		    reg_buf[10] = 0xB0;
		    reg_buf[11] = 0x04;
		    reg_buf[12] = 0x5C;
		    reg_buf[13] = 0xB0;
		    reg_buf[14] = 0x04;
		    reg_buf[15] = 0x5C;
		    reg_buf[16] = 0xB0;
		    reg_buf[17] = 0x04;
		    reg_buf[18] = 0x5C;
		    reg_buf[19] = 0xB0;
		    reg_buf[20] = 0x04;
		    reg_buf[21] = 0x5C;
		    reg_buf[22] = 0xB0;
		    reg_buf[23] = 0x04;
		    reg_buf[24] = 0x5C;
		    reg_buf[25] = 0xB0;
		    reg_buf[26] = 0x04;
		    reg_buf[27] = 0x5C;
		    reg_buf[28] = 0xB0;
		    reg_buf[29] = 0x04;
		    reg_buf[30] = 0x5C;
		}
		else
		{
		    reg_buf[1] = 0x20;
		    reg_buf[2] = 0x04;
		    reg_buf[3] = 0xD3;
		    reg_buf[4] = 0x20;
		    reg_buf[5] = 0x04;
		    reg_buf[6] = 0xD3;
		    reg_buf[7] = 0x20;
		    reg_buf[8] = 0x04;
		    reg_buf[9] = 0xD3;
		    reg_buf[10] = 0x20;
		    reg_buf[11] = 0x04;
		    reg_buf[12] = 0xD3;
		    reg_buf[13] = 0x20;
		    reg_buf[14] = 0x04;
		    reg_buf[15] = 0xD3;
		    reg_buf[16] = 0x20;
		    reg_buf[17] = 0x04;
		    reg_buf[18] = 0xD3;
		    reg_buf[19] = 0x20;
		    reg_buf[20] = 0x04;
		    reg_buf[21] = 0xD3;
		    reg_buf[22] = 0x11;
		    reg_buf[23] = 0xB5;
		    reg_buf[24] = 0x24;
		    reg_buf[25] = 0x02;
		    reg_buf[26] = 0x9D;
		    reg_buf[27] = 0x3B;
		    reg_buf[28] = 0x02;
		    reg_buf[29] = 0xC2;
		    reg_buf[30] = 0x16;
		}
		mipi_write_delay(PacketType, reg_buf, 31);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCC;
		reg_buf[1] = 0xB0;
		reg_buf[2] = 0x04;
		reg_buf[3] = 0x5C;
		reg_buf[4] = 0xB0;
		reg_buf[5] = 0x04;
		reg_buf[6] = 0x5C;
		reg_buf[7] = 0xB0;
		reg_buf[8] = 0x04;
		reg_buf[9] = 0x5C;
		reg_buf[10] = 0x29;
		reg_buf[11] = 0x18;
		reg_buf[12] = 0x1E;
		reg_buf[13] = 0x29;
		reg_buf[14] = 0x18;
		reg_buf[15] = 0x1E;
		reg_buf[16] = 0x55;
		reg_buf[17] = 0x12;
		reg_buf[18] = 0x13;
		mipi_write_delay(PacketType, reg_buf, 19);
		
		if(mode == 2)
		{
			usleep(20000);
			memset(reg_buf,0,sizeof(reg_buf));
			PacketType = 0x39;
			reg_buf[0] = 0xE1;
			reg_buf[1] = 0x00;
			reg_buf[2] = 0x00;
			reg_buf[3] = 0x00;
			reg_buf[4] = 0x28;
			reg_buf[5] = 0x28;
			reg_buf[6] = 0x40;
			reg_buf[7] = 0x17;
			reg_buf[8] = 0x17;
			reg_buf[9] = 0x00;
			reg_buf[10] = 0x0C;
			reg_buf[11] = 0x4F;
			mipi_write_delay(PacketType, reg_buf, 12);
		}
		
		usleep(100000);
		
        printf("JOAN AOD ON \n");
	}
	else
	{
       	reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
		if(!reg_dbv) reg_dbv = 0xEF;

		usleep(50000);
		memset(reg_buf,0,sizeof(reg_buf));
		
		PacketType = 0x39;
		reg_buf[0] = 0xCA;
		reg_buf[1] = 0x00;
		if(!mode)
			reg_buf[2] = 0x06;
		else
			reg_buf[2] = 0x36;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x06;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x16;
		reg_buf[7] = 0x10;
		mipi_write_delay(PacketType, reg_buf, 8);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCB;
		reg_buf[1] = 0x0B;
		reg_buf[2] = 0x68;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x0B;
		reg_buf[5] = 0x68;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x0B;
		reg_buf[8] = 0x68;
		reg_buf[9] = 0x00;
		reg_buf[10] = 0x0B;
		reg_buf[11] = 0x68;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x0B;
		reg_buf[14] = 0x68;
		reg_buf[15] = 0x00;
		reg_buf[16] = 0x0B;
		reg_buf[17] = 0x68;
		reg_buf[18] = 0x00;
		reg_buf[19] = 0x0B;
		reg_buf[20] = 0x68;
		reg_buf[21] = 0x00;
		reg_buf[22] = 0x0B;
		reg_buf[23] = 0x68;
		reg_buf[24] = 0x00;
		reg_buf[25] = 0x0B;
		reg_buf[26] = 0x68;
		reg_buf[27] = 0x00;
		reg_buf[28] = 0x0B;
		reg_buf[29] = 0x68;
		reg_buf[30] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 31);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCC;
		reg_buf[1] = 0x0B;
		reg_buf[2] = 0x68;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x0B;
		reg_buf[5] = 0x68;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x0B;
		reg_buf[8] = 0x68;
		reg_buf[9] = 0x00;
		reg_buf[10] = 0x0B;
		reg_buf[11] = 0x68;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x0B;
		reg_buf[14] = 0x68;
		reg_buf[15] = 0x00;
		reg_buf[16] = 0x55;
		reg_buf[17] = 0x12;
		reg_buf[18] = 0x13;
		mipi_write_delay(PacketType, reg_buf, 19);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE5;
		reg_buf[1] = 0x04;
		reg_buf[2] = 0x04;
		reg_buf[3] = 0x03;
		reg_buf[4] = 0x03;
		reg_buf[5] = 0x56;
		reg_buf[6] = 0x21;
		reg_buf[7] = 0x33;
		reg_buf[8] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 9);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE2;
		reg_buf[1] = 0x20;
		reg_buf[2] = 0x0D;
		reg_buf[3] = 0x08;
		reg_buf[4] = 0xA8;
		reg_buf[5] = 0x0A;
		reg_buf[6] = 0xAA;
		reg_buf[7] = 0x04;
		reg_buf[8] = 0xA4;
		reg_buf[9] = 0x80;
		reg_buf[10] = 0x80;
		reg_buf[11] = 0x80;
		reg_buf[12] = 0x5C;
		reg_buf[13] = 0x5C;
		reg_buf[14] = 0x5C;
		mipi_write_delay(PacketType, reg_buf, 15);
		usleep(40000);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x34;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x38;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x31;
		reg_buf[1] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 2);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x51;
		reg_buf[1] = 0x03;
		mipi_write_delay(PacketType, reg_buf, 2);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0x5E;
		reg_buf[1] = 0x00;
		mipi_write_delay(PacketType, reg_buf, 2);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x13;
		mipi_write_delay(PacketType, reg_buf, 1);
		usleep(85000);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x05;
		reg_buf[0] = 0x35;
		mipi_write_delay(PacketType, reg_buf, 1);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCA;
		reg_buf[1] = 0x00;
		if(!mode)
			reg_buf[2] = 0x06;
		else
			reg_buf[2] = 0x36;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x06;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x16;
		reg_buf[7] = 0x10;
		mipi_write_delay(PacketType, reg_buf, 8);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCB;
		if(!mode)
		{
		    reg_buf[1] = 0xB0;
		    reg_buf[2] = 0x04;
		    reg_buf[3] = 0x5B;
		    reg_buf[4] = 0xB0;
		    reg_buf[5] = 0x04;
		    reg_buf[6] = 0x5C;
		    reg_buf[7] = 0xB0;
		    reg_buf[8] = 0x04;
		    reg_buf[9] = 0x5C;
		    reg_buf[10] = 0xB0;
		    reg_buf[11] = 0x04;
		    reg_buf[12] = 0x5C;
		    reg_buf[13] = 0xB0;
		    reg_buf[14] = 0x04;
		    reg_buf[15] = 0x5C;
		    reg_buf[16] = 0xB0;
		    reg_buf[17] = 0x04;
		    reg_buf[18] = 0x5C;
		    reg_buf[19] = 0xB0;
		    reg_buf[20] = 0x04;
		    reg_buf[21] = 0x5C;
		    reg_buf[22] = 0xB0;
		    reg_buf[23] = 0x04;
		    reg_buf[24] = 0x5C;
		    reg_buf[25] = 0xB0;
		    reg_buf[26] = 0x04;
		    reg_buf[27] = 0x5C;
		    reg_buf[28] = 0xB0;
		    reg_buf[29] = 0x04;
		    reg_buf[30] = 0x5C;
		}
		else
		{
		    reg_buf[1] = 0x20;
		    reg_buf[2] = 0x04;
		    reg_buf[3] = 0xD3;
		    reg_buf[4] = 0x20;
		    reg_buf[5] = 0x04;
		    reg_buf[6] = 0xD3;
		    reg_buf[7] = 0x20;
		    reg_buf[8] = 0x04;
		    reg_buf[9] = 0xD3;
		    reg_buf[10] = 0x20;
		    reg_buf[11] = 0x04;
		    reg_buf[12] = 0xD3;
		    reg_buf[13] = 0x20;
		    reg_buf[14] = 0x04;
		    reg_buf[15] = 0xD3;
		    reg_buf[16] = 0x20;
		    reg_buf[17] = 0x04;
		    reg_buf[18] = 0xD3;
		    reg_buf[19] = 0x20;
		    reg_buf[20] = 0x04;
		    reg_buf[21] = 0xD3;
		    reg_buf[22] = 0x11;
		    reg_buf[23] = 0xB5;
		    reg_buf[24] = 0x24;
		    reg_buf[25] = 0x02;
		    reg_buf[26] = 0x9D;
		    reg_buf[27] = 0x3B;
		    reg_buf[28] = 0x02;
		    reg_buf[29] = 0xC2;
		    reg_buf[30] = 0x16;
		}
		    mipi_write_delay(PacketType, reg_buf, 31);
		
		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x39;
		    reg_buf[0] = 0xCC;
		    reg_buf[1] = 0xB0;
		    reg_buf[2] = 0x04;
		    reg_buf[3] = 0x5C;
		    reg_buf[4] = 0xB0;
		    reg_buf[5] = 0x04;
		    reg_buf[6] = 0x5C;
		    reg_buf[7] = 0xB0;
		    reg_buf[8] = 0x04;
		    reg_buf[9] = 0x5C;
		    reg_buf[10] = 0x29;
		    reg_buf[11] = 0x18;
		    reg_buf[12] = 0x1E;
		    reg_buf[13] = 0x29;
		    reg_buf[14] = 0x18;
		    reg_buf[15] = 0x1E;
		    reg_buf[16] = 0x55;
		    reg_buf[17] = 0x12;
		    reg_buf[18] = 0x13;
		    mipi_write_delay(PacketType, reg_buf, 19);
		    usleep(200000);
		
		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x15;
		    reg_buf[0] = 0x51;
		    reg_buf[1] = reg_dbv;
		    mipi_write_delay(PacketType, reg_buf, 2);
		
		    usleep(100000);

        printf("JOAN AOD OFF \n");
	}
	printf("AOD WRITE \n");
	FUNC_END();
}

void a1_aod_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
	char cmd[300] = {0,};

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        usleep(10000);

        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x0A;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x8C;
        reg_buf[2] = 0x80;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0xB2;
        reg_buf[1] = 0x65;
        mipi_write(PacketType, reg_buf, 2);


        PacketType = 0x15;
        reg_buf[0] = 0x57;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0x5A;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x05;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0xE3;
        reg_buf[1] = 0x03;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x15;
        reg_buf[0] = 0xC0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

		usleep(50000);

        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

///hs_clk modify 580
		mipi_hs_clk_change(580);	

		usleep(10000);

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x03;
        mipi_write(PacketType, reg_buf, 2);

		usleep(10000);

        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x07;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xB2;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x00;
        mipi_write(PacketType, reg_buf, 4);

        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);
    }
    else
    {
        system("/Data/Pattern 11 B1");
        usleep(3000);

        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x0A;
        mipi_write(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x92;
        reg_buf[2] = 0x40;
        mipi_write(PacketType, reg_buf, 3);

        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

		usleep(50000);

        PacketType = 0x05;
        reg_buf[0] = 0x10;
        mipi_write(PacketType, reg_buf, 1);

		usleep(100000);

		sprintf(cmd, "/Data/reg_init /mnt/sd/initial/register_data%d.tty",model_index);
		system(cmd);
    }
	FUNC_END();
}

void dp049_aod_mode_onoff(int id,int model_index,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
	unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x02;	//180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);
    }
    else
    {
		found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
		reg_buf[1] = reg_dbv1;
		reg_buf[2] = reg_dbv2;
        mipi_write(PacketType, reg_buf, 3);

        printf("DP049 AOD OFF \n");
    }
	FUNC_END();
}

void akatsuki_aod_mode_onoff(int id,int model_index,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

////////// /* hyelim add - 180226*/

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x03;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

		usleep(50000);	//180405

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

		usleep(1000000);	//180405

        printf("AKATSUKI AOD ON \n");
    }
    else
    {
        found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x7F;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;  //180129
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x55;	//180405
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xFA;
        reg_buf[1] = 0x00;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x13;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x0F;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xE1;
        reg_buf[1] = 0xC0;
        mipi_write(PacketType, reg_buf, 2);

		usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;	//180405
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv1;
        reg_buf[2] = reg_dbv2;
        mipi_write(PacketType, reg_buf, 3);

		usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("AKATSUKI AOD OFF \n");
    }
	FUNC_END();
}

void b1_aod_mode_onoff(int id,int model_index,int *onoff,char *pic_cmd)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
    unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x22;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x30;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0x6F;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x31;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x04;
        reg_buf[4] = 0x37;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB4;
        reg_buf[1] = 0x2A;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x25;
        reg_buf[4] = 0x22;
        reg_buf[5] = 0x41;
        reg_buf[6] = 0x41;
        reg_buf[7] = 0x41;
        reg_buf[8] = 0x0A;
        reg_buf[9] = 0x10;
        reg_buf[10] = 0x50;
        reg_buf[11] = 0x11;
        reg_buf[12] = 0x03;
        mipi_write(PacketType, reg_buf, 13);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x80;
        reg_buf[2] = 0x5C;
        reg_buf[3] = 0x07;
        reg_buf[4] = 0x08;
        reg_buf[5] = 0x34;
        mipi_write(PacketType, reg_buf, 6);

		//usleep(60000);
		usleep(100000); //test-180515

//test - 180515
#if	0
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);
#endif
// end - 180515

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x03;
        reg_buf[2] = 0xFF;
        mipi_write(PacketType, reg_buf, 3);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x12;
        mipi_write(PacketType, reg_buf, 1);

		system(pic_cmd);	/* swchoi - add for AOD with DECON frame start/stop patch */

		usleep(50000);
        printf("B1 AOD ON \n");

	}
	else
	{
		found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x22;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB4;
        reg_buf[1] = 0x2A;
        reg_buf[2] = 0x02;
        reg_buf[3] = 0x25;
        reg_buf[4] = 0x22;
        reg_buf[5] = 0x41;
        reg_buf[6] = 0x41;
        reg_buf[7] = 0x41;
        reg_buf[8] = 0x0A;
        reg_buf[9] = 0x10;
        reg_buf[10] = 0x50;
        reg_buf[11] = 0x11;
        reg_buf[12] = 0x00;
        mipi_write(PacketType, reg_buf, 13);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x80;
        reg_buf[2] = 0x5C;
        reg_buf[3] = 0x07;
        reg_buf[4] = 0x1A;
        reg_buf[5] = 0x34;
        mipi_write(PacketType, reg_buf, 6);

        //usleep(60000);
        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv1;
        reg_buf[2] = reg_dbv2;
        mipi_write(PacketType, reg_buf, 3);
	
		st_is_remain_code_after_next_pt |= 1<< B1_AOD_OFF;	

//test - 180515

#if	0
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1); 
#endif

#if	0
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x13;
        mipi_write(PacketType, reg_buf, 1);
   
        usleep(50000);
#endif
//end - 180515

		printf("%s: remain_code_after_next_pt is set [0x%X]\n",__func__,st_is_remain_code_after_next_pt);
		
        printf("B1 AOD OFF \n");
	}
	FUNC_END();
}

void joan_variable_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xFE;
    int i = 0;

	FUNC_BEGIN();
    if(*onoff)
    {

		PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xAF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x07;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0d;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x14;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x32;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);
        printf("JOAN/MV VARIABLE MODE ON [%d]\n",i);
    }
    else
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFE;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0d;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x30;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        printf("JOAN VARIABLE MODE OFF [%d]\n",i);
    }
	FUNC_END();
}

void mv_variable_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xFF;
    int i = 0;

	FUNC_BEGIN();
    if(*onoff)
	{
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x88;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x07;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0d;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x14;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x32;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);
        printf("JOAN/MV VARIABLE MODE ON [%d]\n",i);
	}
	else
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0d;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE4;
        reg_buf[1] = 0x30;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        printf("JOAN/MV VARIABLE MODE OFF [%d]\n",i);
	}
	FUNC_END();
}


void a1_variable_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xFE;
	
	FUNC_BEGIN();

    if(*onoff)
    {
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xFF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x44;
        reg_buf[2] = 0xFF;
        reg_buf[3] = 0x2C;
        reg_buf[4] = 0xD1;
        mipi_write_delay(PacketType, reg_buf, 5);
	}
	else
	{
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFE;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x44;
        reg_buf[2] = 0xFF;
        reg_buf[3] = 0x2C;
        reg_buf[4] = 0xB9;
        mipi_write_delay(PacketType, reg_buf, 5);
	}
	FUNC_END();
}

void dp049_variable_mode_onoff(int id, int model_index,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x14;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xC1;
        reg_buf[1] = 0xA0;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xC0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE1;
        reg_buf[1] = 0x1C;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x01;
        reg_buf[4] = 0x1C;
        reg_buf[5] = 0x48;
        reg_buf[6] = 0x48;
        reg_buf[7] = 0xF0;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x03;
        reg_buf[10] = 0x03;
        reg_buf[11] = 0x06;
        reg_buf[12] = 0x44;
        reg_buf[13] = 0x44;
        reg_buf[14] = 0x01;
        reg_buf[15] = 0x14;
        reg_buf[16] = 0xE0;
        mipi_write(PacketType, reg_buf, 17);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x07;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01; //180129
        mipi_write(PacketType, reg_buf, 2);

/////180129
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0xE8;
        reg_buf[2] = 0x4A;
        reg_buf[3] = 0x5F;
        mipi_write(PacketType, reg_buf, 4);
        usleep(10000);
//////
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("DP049 VARIABLE ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x14;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xC1;
        reg_buf[1] = 0x50;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xC0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE1;
        reg_buf[1] = 0x1C;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x18;
        reg_buf[5] = 0x12;
        reg_buf[6] = 0x12;
        reg_buf[7] = 0x00;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x03;
        reg_buf[10] = 0x03;
        reg_buf[11] = 0x06;
        reg_buf[12] = 0x44;
        reg_buf[13] = 0x44;
        reg_buf[14] = 0x01;
        reg_buf[15] = 0x14;
        reg_buf[16] = 0xC0;
        mipi_write(PacketType, reg_buf, 17);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;		//180129
        mipi_write(PacketType, reg_buf, 2);

///////////////180129
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x28;
        reg_buf[2] = 0x39;
        reg_buf[3] = 0xDF;
        mipi_write(PacketType, reg_buf, 4);
        usleep(10000);
/////////////////
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("DP049 VARIABLE OFF \n");
    }
	FUNC_END();
}

void b1_variable_mode_onoff(int id, int model_index,int *onoff, char *pic_cmd, int if_prev)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;
	unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0x00;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x00;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x00;
        reg_buf[10]= 0x00;
        reg_buf[11]= 0x00;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x00;
        reg_buf[14] = 0x00;
        reg_buf[15] = 0x00;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x00;
        mipi_write(PacketType, reg_buf, 19);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xB6;
        reg_buf[1] = 0xFF;
        reg_buf[2] = 0x80;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0xE0;
        reg_buf[5] = 0x20;
        reg_buf[6] = 0x00;
        reg_buf[7] = 0x50;
        reg_buf[8] = 0x20;

        reg_buf[9] = 0x80;//
/*
        reg_buf[10] = 0x23;//
        reg_buf[11] = 0x1D;//
*/
        reg_buf[10] = 0x1C;//180420
        reg_buf[11] = 0x1C;//180420

        mipi_write(PacketType, reg_buf, 12);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x8A;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x82;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x9E; //
        reg_buf[23] = 0x1E;//
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC3;
        reg_buf[1] = 0x10;
        reg_buf[2] = 0x15;
        reg_buf[3] = 0x15;
        reg_buf[4] = 0xB9;
        reg_buf[5] = 0xAD;
        reg_buf[6] = 0x1D;
        //reg_buf[7] = 0xB0; //
        reg_buf[7] = 0xBF; //180420
        reg_buf[8] = 0x10;
        reg_buf[9] = 0xB7;
        reg_buf[10] = 0x17;
        reg_buf[11] = 0x22;
        reg_buf[12] = 0x02;
        reg_buf[13] = 0xC8;
        reg_buf[14] = 0x20;
        reg_buf[15] = 0x83;
        reg_buf[16] = 0x0C;
        reg_buf[17] = 0x07;
        mipi_write(PacketType, reg_buf, 18);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x80;
        reg_buf[2] = 0x5C;
        reg_buf[3] = 0x07;
        reg_buf[4] = 0x1A;
        reg_buf[5] = 0x34;
        reg_buf[6] = 0x5A;
        reg_buf[7] = 0x4F;
        reg_buf[8] = 0x5A;
        reg_buf[9] = 0x33;
        reg_buf[10] = 0x19;
        reg_buf[11] = 0x60;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x00;
        reg_buf[14] = 0x00;
        mipi_write(PacketType, reg_buf, 15);

		usleep(130000);
/*
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x01;
        reg_buf[2] = 0xFF;
        mipi_write(PacketType, reg_buf, 3);
*/
		system(pic_cmd);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

		usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE6;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x2D;
        reg_buf[4] = 0x20;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0x2D;
        reg_buf[7] = 0x20;
        reg_buf[8] = 0x00;
        reg_buf[9] = 0x2D;
        reg_buf[10] = 0x20;
        reg_buf[11] = 0x00;
        reg_buf[12] = 0x2D;
        reg_buf[13] = 0x20;
        reg_buf[14] = 0x00;
        reg_buf[15] = 0x2D;
        reg_buf[16] = 0x10;
        reg_buf[17] = 0xBD;
        reg_buf[18] = 0x6F;

        mipi_write(PacketType, reg_buf, 19);

// add 180319
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xCA;
        mipi_write(PacketType, reg_buf, 2);
/// end

        printf("B1 VARIABLE ON \n");
    }
    else
    {
		found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x15;
		reg_buf[0] = 0xB0;
		reg_buf[1] = 0xAC;
		mipi_write(PacketType, reg_buf, 2);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xE6;
		reg_buf[1] = 0x00;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x00;
		reg_buf[8] = 0x00;
		reg_buf[9] = 0x00;
		reg_buf[10]= 0x00;
		reg_buf[11]= 0x00;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x00;
		reg_buf[14] = 0x00;
		reg_buf[15] = 0x00;
		reg_buf[16] = 0x00;
		reg_buf[17] = 0x00;
		reg_buf[18] = 0x00;
		mipi_write(PacketType, reg_buf, 19);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xB6;
		reg_buf[1] = 0x60;
		reg_buf[2] = 0x30;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x50;
		reg_buf[5] = 0x20;
		reg_buf[6] = 0x00;
		reg_buf[7] = 0x50;
		reg_buf[8] = 0x20;
		reg_buf[9] = 0xB0;
		reg_buf[10] = 0x23;
		reg_buf[11] = 0x1D;
		mipi_write(PacketType, reg_buf, 12);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xC2;
		reg_buf[1] = 0x70;
		reg_buf[2] = 0x8A;
		reg_buf[3] = 0x0A;
		reg_buf[4] = 0x0A;
		reg_buf[5] = 0x24;
		reg_buf[6] = 0x8C;
		reg_buf[7] = 0x4C;
		reg_buf[8] = 0x24;
		reg_buf[9] = 0x8C;
		reg_buf[10] = 0x4C;
		reg_buf[11] = 0x0A;
		reg_buf[12] = 0xED;
		reg_buf[13] = 0xAD;
		reg_buf[14] = 0x54;
		reg_buf[15] = 0x22;
		reg_buf[16] = 0x00;
		reg_buf[17] = 0x00;
		reg_buf[18] = 0x64;
		reg_buf[19] = 0x24;
		reg_buf[20] = 0xCB;
		reg_buf[21] = 0x64;
/*
		reg_buf[22] = 0x97;
		reg_buf[23] = 0x17;
*/
        reg_buf[22] = 0x9E;//180420
        reg_buf[23] = 0x12;//180420

		reg_buf[24] = 0x64;
		mipi_write(PacketType, reg_buf, 25);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xC3;
		reg_buf[1] = 0x10;
		reg_buf[2] = 0x15;
		reg_buf[3] = 0x1F;
		reg_buf[4] = 0xB9;
		reg_buf[5] = 0xAD;
		reg_buf[6] = 0x1D;
		reg_buf[7] = 0xB0;
		reg_buf[8] = 0x10;
		reg_buf[9] = 0xB7;
		reg_buf[10] = 0x17;
		reg_buf[11] = 0x22;
		reg_buf[12] = 0x02;
		reg_buf[13] = 0xC8;
		reg_buf[14] = 0x20;
		reg_buf[15] = 0x83;
		reg_buf[16] = 0x0C;
		reg_buf[17] = 0x07;
		mipi_write(PacketType, reg_buf, 18);
		
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xCB;
		reg_buf[1] = 0x80;
		reg_buf[2] = 0x5C;
		reg_buf[3] = 0x07;
		reg_buf[4] = 0x1A;
		reg_buf[5] = 0x34;
		reg_buf[6] = 0x54;
		reg_buf[7] = 0x4F;
		reg_buf[8] = 0x5A;
		reg_buf[9] = 0x33;
		reg_buf[10] = 0x19;
		reg_buf[11] = 0x60;
		reg_buf[12] = 0x00;
		reg_buf[13] = 0x00;
		reg_buf[14] = 0x00;
		mipi_write(PacketType, reg_buf, 15);
		
		usleep(100000);
		/*
		memset(reg_buf,0,sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0x51;
		reg_buf[1] = reg_dbv1;
		reg_buf[2] = reg_dbv2;
		mipi_write(PacketType, reg_buf, 3);
		*/
		st_is_remain_code_after_next_pt |= 1<< B1_VARIABLE_OFF;
		printf("%s: remain_code_after_next_pt is set [0x%X]\n",__func__,st_is_remain_code_after_next_pt);

        printf("B1 VARIABLE OFF \n");
    }
	FUNC_END();
}

void joan_blackpoint_mode_onoff(int id, int model_index, int *onoff) //for E5 (normal joan is E2)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    int i;

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x07;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x04;

        mipi_write_delay(PacketType, reg_buf, 4);
        mipi_write_delay(PacketType, reg_buf, 4);

        printf("JOAN BLACKPOINT ON [%d]\n",i);
		usleep(84000);
    }
    else
    {
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x00;

        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;

        mipi_write_delay(PacketType, reg_buf, 4);
        mipi_write_delay(PacketType, reg_buf, 4);

        printf("JOAN BLACKPOINT ON [%d]\n",i);
        usleep(84000);
    }
    printf("BLACKPOINT WRITE \n");
	FUNC_END();
}

void mv_blackpoint_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    int i;

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x07;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x23;
        reg_buf[4] = 0x23;
        reg_buf[5] = 0x23;
        reg_buf[6] = 0x25;
        reg_buf[7] = 0x29;
        reg_buf[8] = 0x2C;
        reg_buf[9] = 0x30;
        reg_buf[10] = 0x31;
        reg_buf[11] = 0x31;
        reg_buf[12] = 0x2F;
        reg_buf[13] = 0x2B;
        reg_buf[14] = 0x30;
        reg_buf[15] = 0x2F;
        reg_buf[16] = 0x1E;
        reg_buf[17] = 0x30;
        mipi_write_delay(PacketType, reg_buf, 18);
        mipi_write_delay(PacketType, reg_buf, 18);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x04;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);
        printf("MV BLACKPOINT ON [%d]\n",i);
		usleep(84000);
    }
    else
    {
        PacketType = 0x39;
        reg_buf[0] = 0xE9;
        reg_buf[1] = 0x00;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x23;
        reg_buf[4] = 0x23;
        reg_buf[5] = 0x23;
        reg_buf[6] = 0x25;
        reg_buf[7] = 0x29;
        reg_buf[8] = 0x2C;
        reg_buf[9] = 0x30;
        reg_buf[10] = 0x31;
        reg_buf[11] = 0x31;
        reg_buf[12] = 0x2F;
        reg_buf[13] = 0x2B;
        reg_buf[14] = 0x30;
        reg_buf[15] = 0x2F;
        reg_buf[16] = 0x1E;
        reg_buf[17] = 0x30;
        mipi_write_delay(PacketType, reg_buf, 18);
        mipi_write_delay(PacketType, reg_buf, 18);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);
        printf("MV BLACKPOINT OFF [%d]\n",i);
		usleep(84000);
    }
    printf("BLACKPOINT WRITE \n");
	FUNC_END();
}


void dp049_blackpoint_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x07;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x25;
        reg_buf[2] = 0xB9;
        mipi_write(PacketType, reg_buf, 3);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("DP049 BLACKPOINT ON \n");
    }
    else
    {

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x09;
        reg_buf[4] = 0x69;
        mipi_write(PacketType, reg_buf, 5);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x39;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        usleep(100000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x28;
        mipi_write(PacketType, reg_buf, 1);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x06;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x23;
        reg_buf[2] = 0x00;
        reg_buf[3] = 0x12;
        reg_buf[4] = 0x65;
        mipi_write(PacketType, reg_buf, 5);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x28;
        reg_buf[2] = 0x39;
        mipi_write(PacketType, reg_buf, 3);

        usleep(10000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x38;
        mipi_write(PacketType, reg_buf, 1);

        usleep(50000);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x05;
        reg_buf[0] = 0x29;
        mipi_write(PacketType, reg_buf, 1);

        printf("DP049 BLACKPOINT OFF \n");
    }
	FUNC_END();
}

void b1_blackpoint_mode_onoff(int id,int model_index, int *onoff,char* pic_cmd, int if_prev) //yac hui jum..sin loe sung hui jum..
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
		printf("if_prev = %d \n",if_prev);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x80;
        reg_buf[2] = 0x5C;
        reg_buf[3] = 0x07;
        reg_buf[4] = 0x1A;
        reg_buf[5] = 0x34;
        reg_buf[6] = 0x5A;
        reg_buf[7] = 0x4F;
        reg_buf[8] = 0x5A;
        reg_buf[9] = 0x33;
        reg_buf[10] = 0x19;
        reg_buf[11] = 0x60;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x00;
        reg_buf[14] = 0x00;
        mipi_write(PacketType, reg_buf, 15);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x85;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x8C;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

		system(pic_cmd);

        printf("B1 BLACKPOINT ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xCB;
        reg_buf[1] = 0x80;
        reg_buf[2] = 0x5C;
        reg_buf[3] = 0x07;
        reg_buf[4] = 0x1A;
        reg_buf[5] = 0x34;
        reg_buf[6] = 0x54;
        reg_buf[7] = 0x4F;
        reg_buf[8] = 0x5A;
        reg_buf[9] = 0x33;
        reg_buf[10] = 0x19;
        reg_buf[11] = 0x60;
        reg_buf[12] = 0x00;
        reg_buf[13] = 0x00;
        reg_buf[14] = 0x00;
        mipi_write(PacketType, reg_buf, 15);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x8A;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x8C;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        if(!if_prev)
        {
            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x15;
            reg_buf[0] = 0xB0;
            reg_buf[1] = 0xCA;
            mipi_write(PacketType, reg_buf, 2);
        }

        printf("B1 BLACKPOINT OFF \n");
    }
	FUNC_END();
}

void joan_bright_line_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
	unsigned char reg_dbv = 0xFF;
    
	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x75;
		mipi_write_delay(PacketType, reg_buf, 2);
		mipi_write_delay(PacketType, reg_buf, 2);
		
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x14;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);
        printf("JOAN BRIGHT LINE ON\n");
    }
    else
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
	    if(!reg_dbv) reg_dbv = 0xFF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);
        printf("JOAN BRIGHT LINE OFF\n");
    }
	FUNC_END();
}

void dp049_bright_line_mode_onoff(int id,int model_index,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

/////////180129
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0xE0;
        reg_buf[2] = 0xCA;
        reg_buf[3] = 0x5F;

        mipi_write(PacketType, reg_buf, 4);
//////////
        printf("DP049 BRIGHT_LINE ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x28;
        reg_buf[2] = 0x39;
        reg_buf[3] = 0xDF;
        mipi_write(PacketType, reg_buf, 4);

        printf("DP049 BRIGHT_LINE OFF \n");
    }
	FUNC_END();
}

void b1_bright_line_mode_onoff(int id,int model_index,int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x80;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x82;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC3;
        reg_buf[1] = 0x10;
        reg_buf[2] = 0x15;
        reg_buf[3] = 0x15;
        reg_buf[4] = 0xB9;
        reg_buf[5] = 0xAD;
        reg_buf[6] = 0x1D;
        reg_buf[7] = 0xBF;
        reg_buf[8] = 0x10;
        reg_buf[9] = 0xB7;
        reg_buf[10] = 0x17;
        reg_buf[11] = 0x22;
        reg_buf[12] = 0x02;
        reg_buf[13] = 0xC8;
        reg_buf[14] = 0x20;
        reg_buf[15] = 0x83;
        reg_buf[16] = 0x0C;
        reg_buf[17] = 0x07;
        mipi_write(PacketType, reg_buf, 18);

        printf("B1 BRIGHT_LINE ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x8A;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x8C;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC3;
        reg_buf[1] = 0x10;
        reg_buf[2] = 0x15;
        reg_buf[3] = 0x1F;
        reg_buf[4] = 0xB9;
        reg_buf[5] = 0xAD;
        reg_buf[6] = 0x1D;
        reg_buf[7] = 0xB0;
        reg_buf[8] = 0x10;
        reg_buf[9] = 0xB7;
        reg_buf[10] = 0x17;
        reg_buf[11] = 0x22;
        reg_buf[12] = 0x02;
        reg_buf[13] = 0xC8;
        reg_buf[14] = 0x20;
        reg_buf[15] = 0x83;
        reg_buf[16] = 0x0C;
        reg_buf[17] = 0x07;
        mipi_write(PacketType, reg_buf, 18);

        printf("B1 BRIGHT_LINE OFF \n");
    }
	FUNC_END();
}

void joan_black_line_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
	unsigned char reg_dbv = 0xFF;

	FUNC_BEGIN();
    if(*onoff)
    {
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x75;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x51;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x00;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);
        printf("JOAN BLACK LINE ON\n");
    }
    else
    {
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xFF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        mipi_write_delay(PacketType, reg_buf, 6);
        mipi_write_delay(PacketType, reg_buf, 6);
        printf("JOAN BLACK LINE OFF\n");
    }
	FUNC_END();
}

void joan_power_bright_line_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xEF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x14;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN POWER BRIGHT LINE ON\n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xEF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN POWER BRIGHT LINE OFF\n");
    }
	FUNC_END();
}

void joan_power_black_line_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xEF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x51;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN POWER BLACK LINE ON\n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xEF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN POWER BLACK LINE OFF\n");
    }
	FUNC_END();
}

void joan_luminance_50per_power_bright_line_mode_onoff(int id, int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0x75;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x00;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x14;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN 50PER LUMINANCE POWER BRIGHT LINE ON\n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xEF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN 50PER LUMINANCE POWER BRIGHT LINE OFF\n");
    }
	FUNC_END();
}

void joan_luminance_50per_power_black_line_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x15;
    unsigned char reg_dbv = 0xEF;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = 0xEF;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x51;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x00;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN 50PER LUMINANCE POWER BLACK LINE ON\n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        if(!reg_dbv) reg_dbv = 0xEF;

        PacketType = 0x15;
        reg_buf[0] = 0x51;
        reg_buf[1] = reg_dbv;
        mipi_write_delay(PacketType, reg_buf, 2);
        mipi_write_delay(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x20;
        reg_buf[2] = 0x0D;
        reg_buf[3] = 0x08;
        reg_buf[4] = 0xA8;
        reg_buf[5] = 0x0A;
        reg_buf[6] = 0xAA;
        reg_buf[7] = 0x04;
        reg_buf[8] = 0xA4;
        reg_buf[9] = 0x80;
        reg_buf[10] = 0x80;
        reg_buf[11] = 0x80;
        reg_buf[12] = 0x5C;
        reg_buf[13] = 0x5C;
        reg_buf[14] = 0x5C;
        mipi_write_delay(PacketType, reg_buf, 15);
        mipi_write_delay(PacketType, reg_buf, 15);

        printf("JOAN 50PER LUMINANCE POWER BLACK LINE OFF\n");
    }
	FUNC_END();
}

void dp049_black_line_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);
////////////////////180129
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0xEF;
        reg_buf[2] = 0x90;
        reg_buf[3] = 0x9F;
        mipi_write(PacketType, reg_buf, 4);
/////////////////////////

        printf("DP049 BLACK_LINE ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0x01;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xE2;
        reg_buf[1] = 0x28;
        reg_buf[2] = 0x39;
        reg_buf[3] = 0xDF;
        mipi_write(PacketType, reg_buf, 4);
       printf("DP049 BLACK_LINE OFF \n");
    }
	FUNC_END();
}

void b1_black_line_mode_onoff(int id,int model_index, int *onoff)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x05;

	FUNC_BEGIN();
    if(*onoff)
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x93;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x96;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC3;
        reg_buf[1] = 0x10;
        reg_buf[2] = 0x15;
        reg_buf[3] = 0x15;
        reg_buf[4] = 0xB9;
        reg_buf[5] = 0xAD;
        reg_buf[6] = 0x1D;
        reg_buf[7] = 0xBF;
        reg_buf[8] = 0x10;
        reg_buf[9] = 0xB7;
        reg_buf[10] = 0x17;
        reg_buf[11] = 0x22;
        reg_buf[12] = 0x02;
        reg_buf[13] = 0xC8;
        reg_buf[14] = 0x20;
        reg_buf[15] = 0x83;
        reg_buf[16] = 0x0C;
        reg_buf[17] = 0x07;
        mipi_write(PacketType, reg_buf, 18);

        printf("B1 BLACK_LINE ON \n");
    }
    else
    {
        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x15;
        reg_buf[0] = 0xB0;
        reg_buf[1] = 0xAC;
        mipi_write(PacketType, reg_buf, 2);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC2;
        reg_buf[1] = 0x70;
        reg_buf[2] = 0x8A;
        reg_buf[3] = 0x0A;
        reg_buf[4] = 0x0A;
        reg_buf[5] = 0x24;
        reg_buf[6] = 0x8C;
        reg_buf[7] = 0x4C;
        reg_buf[8] = 0x24;
        reg_buf[9] = 0x8C;
        reg_buf[10] = 0x4C;
        reg_buf[11] = 0x0A;
        reg_buf[12] = 0xED;
        reg_buf[13] = 0xAD;
        reg_buf[14] = 0x54;
        reg_buf[15] = 0x22;
        reg_buf[16] = 0x00;
        reg_buf[17] = 0x00;
        reg_buf[18] = 0x64;
        reg_buf[19] = 0x24;
        reg_buf[20] = 0xCB;
        reg_buf[21] = 0x64;
        reg_buf[22] = 0x97;
        reg_buf[23] = 0x17;
        reg_buf[24] = 0x64;
        mipi_write(PacketType, reg_buf, 25);

        memset(reg_buf,0,sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xC3;
        reg_buf[1] = 0x10;
        reg_buf[2] = 0x15;
        reg_buf[3] = 0x1F;
        reg_buf[4] = 0xB9;
        reg_buf[5] = 0xAD;
        reg_buf[6] = 0x1D;
        reg_buf[7] = 0xB0;
        reg_buf[8] = 0x10;
        reg_buf[9] = 0xB7;
        reg_buf[10] = 0x17;
        reg_buf[11] = 0x22;
        reg_buf[12] = 0x02;
        reg_buf[13] = 0xC8;
        reg_buf[14] = 0x20;
        reg_buf[15] = 0x83;
        reg_buf[16] = 0x0C;
        reg_buf[17] = 0x07;
        mipi_write(PacketType, reg_buf, 18);

        printf("B1 BLACK_LINE OFF \n");
    }
	FUNC_END();
}

void b1_border_test_mode_onoff(int id,int model_index, int *onoff)
{
	FUNC_BEGIN();
    if(*onoff)
    {
		printf("B1 Border Test On \n");
	}
	else
	{
		printf("B1 Border Test Off \n");
	}
	FUNC_END();
}

unsigned short joan_dbv_control(int mode,int id,int model_index, int *dimming_mode) // 180104
{
	int i=0;
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x15;
	memset(reg_buf,0,sizeof(reg_buf));
	unsigned char reg_dbv = 0;
/*
	pthread_mutex_t *roll_mutex;

	FUNC_BEGIN();
	if(mode == 1)
	{
		if(receive_roll_thread_mutex(roll_mutex) == false)
		{
			printf("Mutex Null \n");
		}
	}
*/
	switch(mode)
	{
        case    0 :
        	reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
			if(!reg_dbv) reg_dbv = 0xFF; //need modify.............180122
			//system("/Data/Pattern 11");
			if((id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5))
	        	system("/Data/Pattern 11 B1"); //180605
			else
	        	system("/Data/Pattern 11"); //180605
            PacketType = 0x15;
            reg_buf[0] = 0x51;
            reg_buf[1] = reg_dbv;
            mipi_write(PacketType, reg_buf, 2);
// joan / mv..only
            {

				PacketType = 0x15;
				reg_buf[0] = 0x53;
				reg_buf[1] = 0x00;
				mipi_write(PacketType, reg_buf, 2);
			}
            printf("DBV MODE OFF[0x%X]\n",reg_dbv);
			usleep(50000);

            break;

		case	1 :

            printf(" -- DBV_CONTROL\n");

            for(i = 0xFF; i > 0x00; i--)
            {
                PacketType = 0x15;
                reg_buf[0] = 0x51;
                reg_buf[1] = i;
                mipi_write(PacketType, reg_buf, 2);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*dimming_mode)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);
////////////////////////////////////////// need modify...180122
		            reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        		    if(!reg_dbv) reg_dbv = 0xFF;
                    PacketType = 0x15; 
                    reg_buf[0] = 0x51;
                    reg_buf[1] = i;
                    mipi_write(PacketType, reg_buf, 2);
////////////////////////////////////////// need modify...180122
					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
                usleep(5000);
            }
            usleep(50000);

			printf(" ++ DBV_CONTROL\n");
			for(i = 0x00; i < 0xFF; i++)
			{
				PacketType = 0x15; 
				reg_buf[0] = 0x51; 
				reg_buf[1] = i; 
				mipi_write(PacketType, reg_buf, 2);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*dimming_mode)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

////////////////////////////////////////// need modify...180122
		            reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        		    if(!reg_dbv) reg_dbv = 0xFF;
	                PacketType = 0x15; 
    	            reg_buf[0] = 0x51; 
    	            reg_buf[1] = i; 
    	            mipi_write(PacketType, reg_buf, 2);
////////////////////////////////////////// need modify...180122
					FUNC_END();
		            return 0;
				}
                /* lock */
                //pthread_mutex_lock(roll_mutex);
				usleep(5000);
			}
			usleep(50000);
			break;
		default : 
			printf("%s : not correct mode... \n",__func__);
			break;
	}

	FUNC_END();
	return	0;
}

unsigned short a1_dbv_control(int mode,int id,int model_index, int *dimming_mode)
{
    int i=0;
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x15;
    memset(reg_buf,0,sizeof(reg_buf));
    unsigned char reg_dbv = 0;
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
    switch(mode)
    {
        case    0 :
        	reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
            if(!reg_dbv) reg_dbv = 0xFF;
            system("/Data/Pattern 11 B1");
            PacketType = 0x15;
            reg_buf[0] = 0x51;
            reg_buf[1] = reg_dbv;
            mipi_write(PacketType, reg_buf, 2);
            printf("DBV MODE OFF[0x%X]\n",reg_dbv);
            usleep(50000);

            break;

        case    1 :

            printf(" -- DBV_CONTROL\n");

            for(i = 0xFE; i > 0x04; i--)
            {
                PacketType = 0x15;
                reg_buf[0] = 0x51;
                reg_buf[1] = i;
                mipi_write(PacketType, reg_buf, 2);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*dimming_mode)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);
////////////////////////////////////////// need modify...180122
		            reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        		    if(!reg_dbv) reg_dbv = 0xFF;
                    PacketType = 0x15; 
                    reg_buf[0] = 0x51;
                    reg_buf[1] = i;
                    mipi_write(PacketType, reg_buf, 2);
////////////////////////////////////////// need modify...180122
					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
                usleep(5000);
            }
            usleep(50000);

            printf(" ++ DBV_CONTROL\n");
            for(i = 0x04; i < 0xFE; i++)
            {
                PacketType = 0x15;
                reg_buf[0] = 0x51;
                reg_buf[1] = i;
                mipi_write(PacketType, reg_buf, 2);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*dimming_mode)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

////////////////////////////////////////// need modify...180122
		            reg_dbv = found_reg_init_data(id,model_index, 0x51,1); //modify 180920
        		    if(!reg_dbv) reg_dbv = 0xFF;
                    PacketType = 0x15; 
                    reg_buf[0] = 0x51;
                    reg_buf[1] = i;
                    mipi_write(PacketType, reg_buf, 2);
////////////////////////////////////////// need modify...180122
					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
                usleep(5000);
            }
            usleep(50000);
            break;
        default :
            printf("%s : not correct mode... \n",__func__);
            break;
    }

	FUNC_END();
    return  0;
}

int tmp_count = 0;
unsigned short dp049_dbv_control(int mode,int id,int model_index, unsigned char reg_dbv1, unsigned char reg_dbv2, int *dimming_mode) // 180117
{
	int i=0,x=0;
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x15;
	memset(reg_buf,0,sizeof(reg_buf));
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
	switch(mode)
	{
        case    0 :
			found_initial_dbv(id, model_index, &reg_dbv1, &reg_dbv2); //180120

			if ((id == B1) || (id == AKATSUKI))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11");
			}
            PacketType = 0x39;
            reg_buf[0] = 0x51;
            reg_buf[1] = reg_dbv1;
			reg_buf[2] = reg_dbv2;
            mipi_write(PacketType, reg_buf, 3);
            printf("DBV MODE OFF[0x%X][0x%X]\n",reg_dbv1,reg_dbv2);
			usleep(50000);
            break;

		case	1 :
            printf(" -- DBV_CONTROL\n");

            PacketType = 0x39;
            reg_buf[0] = 0x51;

			for(x = 0xFF; x > -1; x--)
			{
            	for(i = 0xFF; i > 0x00; i--)
            	{
					/* unlock */
	                //pthread_mutex_unlock(roll_mutex);

		            if(!*dimming_mode)
			        {
				        /* lock */
					    //pthread_mutex_lock(roll_mutex);

					    reg_buf[1] = reg_dbv1;
	                    reg_buf[2] = reg_dbv2;
						mipi_write(PacketType, reg_buf, 3);
			            if(DEBUG_MODE)
        		        printf("\n");

						FUNC_END();
              	        return 0;
                    }
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

					if(x > reg_dbv1)
					{
						continue;
					}
					else if(x == reg_dbv1)
					{
                        if(i > reg_dbv2)
                            continue;
					}
            	    reg_buf[1] = x;
            	    reg_buf[2] = i;
            	    mipi_write(PacketType, reg_buf, 3);
            	    usleep(1000);
            	}
			}

			if(DEBUG_MODE)
				printf("\n");

			printf(" ++ DBV_CONTROL\n");
			for(x = 0x00; x < 0xFF; x++)
			{
				for(i = 0x00; i < 0xFF; i++)
				{
                    /* unlock */
                    //pthread_mutex_unlock(roll_mutex);

                    if(!*dimming_mode)
                    {
                        /* lock */
                        //pthread_mutex_lock(roll_mutex);

////////////////////////////////////////// need modify...180122
                        reg_buf[1] = reg_dbv1;
                        reg_buf[2] = reg_dbv2;
                        mipi_write(PacketType, reg_buf, 3);
////////////////////////////////////////// need modify...180122

			            if(DEBUG_MODE)
            		    printf("\n");

						FUNC_END();
                        return 0;
                    }
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    if(x > reg_dbv1)
                    {
                        continue;
                    }
                    else if(x == reg_dbv1)
                    {   
                        if(i > reg_dbv2)
                            continue;
                    }
////////////////////////////////////////// need modify...180122
					reg_buf[1] = x; 
					reg_buf[2] = i; 
					mipi_write(PacketType, reg_buf, 3);
////////////////////////////////////////// need modify...180122
					usleep(1000);
				}
			}
            if(DEBUG_MODE)
                printf("\n");

			break;

		case	2:  // dbv modify mode
			if ((id == B1) || (id == AKATSUKI))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11");
			}
			usleep(500);
            PacketType = 0x39;
            reg_buf[0] = 0x51;
            reg_buf[1] = reg_dbv1;
            reg_buf[2] = reg_dbv2;
            mipi_write(PacketType, reg_buf, 3);
            printf("DBV MODE MODIFY[0x%X][0x%X]\n",reg_dbv1,reg_dbv2);
            usleep(50000);
		break;

		default : 
			printf("%s : not correct mode... \n",__func__);
			break;
	}

	FUNC_END();
	return	0;
}

int joan_bist_control(int id,int model_index, int mode,int *gray_scan)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int i = 0;
	memset(reg_buf,0,sizeof(reg_buf));
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
	switch(mode)
	{

		case	0:
		    printf("JOAN FULL GRAY SCAN OUT \n");
			sleep_control(id,model_index, 1,0);
			//system("/Data/Pattern 11");
			if((id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5))
	        	system("/Data/Pattern 11 B1"); //180605
			else
	        	system("/Data/Pattern 11"); //180605
		    reg_buf[0] = 0xEF; //bist time
		    reg_buf[1] = 0x00; //disable
		    mipi_write(PacketType, reg_buf, 9);

            PacketType = 0x15;
            reg_buf[0] = 0x53;
            reg_buf[1] = 0x00;
            mipi_write(PacketType, reg_buf, 2);

			sleep_control(id,model_index, 0,0);
			usleep(50000);
			break;

		case	1:

			//printf(" ++ JOAN FULL GRAY SCAN\n");
			reg_buf[0] = 0xEF; //bist time
			reg_buf[1] = 0x03; //if MANUAL 
			reg_buf[2] = 0x00; //User RGB pattern
			reg_buf[3] = 0x3F;
			reg_buf[7] = 0x00;
			reg_buf[8] = 0x00;

			printf(" ++ JOAN FULL GRAY SCAN\n");

            reg_buf[3] = 0x00;
            for(i = 0; i < 0xFF; i++)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
			    /* unlock */
			    //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
				    /* lock */
				    //pthread_mutex_lock(roll_mutex);

					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

            }

            reg_buf[3] = 0x15;
            for(i = 0; i < 0xFF; i++)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);
                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

            }

            reg_buf[3] = 0x2A;
            for(i = 0; i < 0xFF; i++)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
            }

            reg_buf[3] = 0x3F;
            for(i = 0; i < 0xFF; i++)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

					FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
            }

			printf(" -- JOAN FULL GRAY SCAN\n");

			reg_buf[3] = 0x3F;
			for(i = 0xFF; i > -1; i--)
			{
			    reg_buf[4] = i;
			    reg_buf[5] = i;
			    reg_buf[6] = i;
			    mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

			    if(!*gray_scan)
			    {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

					FUNC_END();
			        return 0;
			    }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

			}

            reg_buf[3] = 0x2A;
            for(i = 0xFF; i > -1; i--)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

            }

            reg_buf[3] = 0x15;
            for(i = 0xFF; i > -1; i--)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

            }

            reg_buf[3] = 0x00;
            for(i = 0xFF; i > -1; i--)
            {
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                mipi_write(PacketType, reg_buf, 9);

                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

            }

			usleep(10000);
			printf("[FULL GRAY SCAN DONE]\n");
			FUNC_END();
		    return 0;
	
			break;

		case	2:

			printf("JOAN FULL WHITE\n");
		    reg_buf[0] = 0xEF; //bist time
			reg_buf[1] = 0x03; //if MANUAL 
			reg_buf[2] = 0x00; //User RGB pattern

		    reg_buf[4] = 0xFF;
		    reg_buf[5] = 0xFF;
		    reg_buf[6] = 0xFF;
		    mipi_write(PacketType, reg_buf, 9);

			break;

        default :
            printf("%s : not correct mode... \n",__func__);
            break;
	}
	FUNC_END();
	return	0;
}

int a1_bist_control(int mode,int *gray_scan)
{
    unsigned char reg_buf[300] = {0,};
    unsigned char PacketType = 0x39;
    unsigned char enable = 1;
    unsigned char manual = 1;
    unsigned char bist_mode =0; //1 : MANUAL_MODE.. 0: AUTO_MODE(AUTO SLIDE)
    int i = 0;
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
    bist_mode |=(manual<<1) | enable;

    switch(mode)
    {

        case    0:
            printf("A1 FULL GRAY SCAN OUT \n");

            bist_mode = 0x01;
            reg_buf[0] = 0xFD; //bist time
            reg_buf[1] = 0; //if MANUAL 
            reg_buf[2] = 0;
            reg_buf[3] = 0;
            reg_buf[4] = 0;
            mipi_write(PacketType, reg_buf, 5);

            break;

        case    1:

            reg_buf[0] = 0xFD; //bist time
            reg_buf[1] = 0x01; //enable

            for(i = 0x00; i < 0xFF; i++)
            {
                reg_buf[2] = i;
                reg_buf[3] = i;
                reg_buf[4] = i;
                mipi_write(PacketType, reg_buf, 5);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
				usleep(6000);
            }
			printf("UPEND\n");

            for(i = 0xFF; i>0; i--)
            {
                reg_buf[2] = i;
                reg_buf[3] = i;
                reg_buf[4] = i;
                mipi_write(PacketType, reg_buf, 5);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

				usleep(6000);
            }
			printf("DOWNEND\n");

            break;

        case    2:

            reg_buf[0] = 0xFD; //bist time
            reg_buf[1] = bist_mode; //if MANUAL 
            reg_buf[2] = 0xFF;
            reg_buf[3] = 0xFF;
            reg_buf[4] = 0xFF;

            mipi_write(PacketType, reg_buf, 5);

            break;

        default :
            printf("%s : not correct mode... \n",__func__);
            break;
    }

	FUNC_END();
    return  0;
}

int dp049_bist_control(int mode,int *gray_scan)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int i = 0;
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
    switch(mode)
    {
        case    0:
            printf("DP049 FULL GRAY SCAN OUT \n");
            system("/Data/Pattern 11");
            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x39;
            reg_buf[0] = 0xFC;
            reg_buf[1] = 0x00;
            mipi_write(PacketType, reg_buf, 2);

            break;

        case    1:


            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x39;
            reg_buf[0] = 0xFC;
            reg_buf[1] = 0x21;
            reg_buf[2] = 0x00;

		    for(i = 0; i < 0xFF; i++)
			{
				reg_buf[3] = i;
				reg_buf[4] = i;
				reg_buf[5] = i;
				reg_buf[6] = i;
				reg_buf[7] = i;
				reg_buf[8] = i;
				reg_buf[9] = i;
				reg_buf[10] = i;
				mipi_write(PacketType, reg_buf, 11);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);
                usleep(6000);
			}
			printf("UPEND\n");

            for(i = 0xFF; i > 0; i--)
            {

                reg_buf[3] = i;
                reg_buf[4] = i;
                reg_buf[5] = i;
                reg_buf[6] = i;
                reg_buf[7] = i;
                reg_buf[8] = i;
                reg_buf[9] = i;
                reg_buf[10] = i;
                mipi_write(PacketType, reg_buf, 11);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

                usleep(6000);
            }
			printf("DOWNEND\n");

            break;

        case    2:
            reg_buf[0] = 0xFC;
            reg_buf[1] = 0x00;
            reg_buf[2] = 0xFF;
            reg_buf[3] = 0xFF;
            reg_buf[4] = 0xFF;
            reg_buf[5] = 0xFF;
            reg_buf[6] = 0xFF;
            reg_buf[7] = 0xFF;
            reg_buf[8] = 0xFF;
            reg_buf[9] = 0xFF;
            reg_buf[10] = 0xFF;

            mipi_write(PacketType, reg_buf, 11);

            break;

        default :
            printf("%s : not correct mode... \n",__func__);
            break;
    }

	FUNC_END();
    return  0;
}

int b1_bist_control(int mode,int *gray_scan)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int rgb = 0;
    int sub_rgb = 0;
/*
    pthread_mutex_t *roll_mutex;

    FUNC_BEGIN();
    if(mode == 1)
    {
        if(receive_roll_thread_mutex(roll_mutex) == false)
        {
            printf("Mutex Null \n");
        }
    }
*/
    memset(reg_buf,0,sizeof(reg_buf));

    switch(mode)
    {
        case    0:
			memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x05;
		    reg_buf[0] = 0x28;
		    mipi_write(PacketType, reg_buf, 1);

			usleep(50000);

		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x39;
		    reg_buf[0] = 0xFA;
		    reg_buf[1] = 0x00;
		    reg_buf[2] = 0x00;
		    reg_buf[3] = 0x3F;
		    reg_buf[4] = 0xFF;
		    reg_buf[5] = 0x00;
			reg_buf[6] = 0x00;
			reg_buf[7] = 0x13;
		    reg_buf[8] = 0x01;

		    mipi_write(PacketType, reg_buf, 9);

		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x05;
		    reg_buf[0] = 0x29;
		    mipi_write(PacketType, reg_buf, 1);

			usleep(50000);

		    printf("B1 BIST OUT \n");

			break;

		case	1 :

	        for(rgb = 0; rgb <0xFF; rgb++)
	        {
	            sub_rgb = (rgb>>6)&0x03; // sub_rgb = 0 0 0 0 0 0 [rgb7] [rgb6]
	            sub_rgb = ((sub_rgb <<4) | (sub_rgb <<2) | (sub_rgb));
		        //printf("sub_rgb = 0x%X(rgb = 0x%X) \n",sub_rgb,rgb);

		        memset(reg_buf,0,sizeof(reg_buf));
		        PacketType = 0x39;
		        reg_buf[0] = 0xFA;
		        reg_buf[1] = 0x03;
		        reg_buf[2] = 0x00;
		        reg_buf[3] = sub_rgb;
		        reg_buf[4] = rgb<<2;
		        reg_buf[5] = rgb<<2;
		        reg_buf[6] = rgb<<2;
		        reg_buf[7] = 0x18;
		        reg_buf[8] = 0x01;
		        mipi_write(PacketType, reg_buf, 9);
                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);


		        usleep(10000);
		    }
		    printf("UP\n");

			for(rgb = 0xFF; rgb > 0; rgb--)
			{
	            sub_rgb = (rgb>>6)&0x03; // sub_rgb = 0 0 0 0 0 0 [rgb7] [rgb6]
	            sub_rgb = ((sub_rgb <<4) | (sub_rgb <<2) | (sub_rgb));
	            //printf("sub_rgb = 0x%X(rgb = 0x%X) \n",sub_rgb,rgb);
	
	            memset(reg_buf,0,sizeof(reg_buf));
	            PacketType = 0x39;
	            reg_buf[0] = 0xFA;
	            reg_buf[1] = 0x03;
	            reg_buf[2] = 0x00;
	            reg_buf[3] = sub_rgb;
	            reg_buf[4] = rgb<<2;
	            reg_buf[5] = rgb<<2;
	            reg_buf[6] = rgb<<2;
	            reg_buf[7] = 0x18;
	            reg_buf[8] = 0x01;
	            mipi_write(PacketType, reg_buf, 9);

                /* unlock */
                //pthread_mutex_unlock(roll_mutex);

                if(!*gray_scan)
                {
                    /* lock */
                    //pthread_mutex_lock(roll_mutex);

                    FUNC_END();
                    return 0;
                }
                /* lock */
                //pthread_mutex_lock(roll_mutex);

	            usleep(10000);

	        }
		    printf("DOWN \n");

		break;

		case	2 : // BIST IN
		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x05;
		    reg_buf[0] = 0x28;
		    mipi_write(PacketType, reg_buf, 1);

			usleep(50000);

		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x15;
		    reg_buf[0] = 0xB0;
		    reg_buf[1] = 0xAC;
		    mipi_write(PacketType, reg_buf, 2);

		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x39;
		    reg_buf[0] = 0xFA;
		    reg_buf[1] = 0x03;
		    reg_buf[2] = 0x00;
		    reg_buf[3] = 0x00;
		    reg_buf[4] = 0x00;
		    reg_buf[5] = 0x00;
		    reg_buf[6] = 0x00;
		    reg_buf[7] = 0x18;
		    reg_buf[8] = 0x01;

		    mipi_write(PacketType, reg_buf, 9);

		    memset(reg_buf,0,sizeof(reg_buf));
		    PacketType = 0x05;
		    reg_buf[0] = 0x29;
		    mipi_write(PacketType, reg_buf, 1);

			usleep(50000);

			printf("B1 BIST IN : GRAYSCAN \n");
		
			break;

		default : 
			printf("%s: MODE ERR \n",__func__);	
			break;
	}
	FUNC_END();
	return 0;
}

int dbv_control(int mode, int id,int model_index, unsigned char reg_dbv1, unsigned char reg_dbv2,int *dimming_mode, char *pic_cmd,int if_prev)
{
	int ret = PASS;
 		// mode 0 : original dbv / mode 1 : dimming / mode 2 : special dbv

	FUNC_BEGIN();
	switch(id)
	{

		case	JOAN :
		case	JOAN_REL :
		case	JOAN_MANUAL :
		case	MV :
		case	MV_MANUAL :
		case	MV_MQA :
		case	MV_DQA :
			joan_dbv_control(mode,id,model_index,dimming_mode);			
			break;
        case    A1 :
			a1_dbv_control(mode,id,model_index,dimming_mode);			
			break;
		case	JOAN_E5 :
			joan_dbv_control(mode,id,model_index,dimming_mode);			
			break;

		case	DP049	:
		case    AKATSUKI    :
		case    B1    :
			dp049_dbv_control(mode,id,model_index,reg_dbv1,reg_dbv2,dimming_mode);
			break;
		default :	
			printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
	}
	if(mode == 2)
	    system(pic_cmd);

	FUNC_END();
    return ret;
}

int bist_control(int mode, int id, int model_index, int *gray_scan ,int spdf)
{
	//mode 0 : bist out / mode 1 : gray scan  / mode 2 : bist in

	FUNC_BEGIN();
    switch(id)
    {

        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    MV :
        case    MV_MANUAL :
        case    JOAN_E5 :
        case    MV_MQA :
        case    MV_DQA :
            joan_bist_control(id,model_index,mode,gray_scan);

            break;
        case    A1 :
            a1_bist_control(mode,gray_scan);
            break;
		case	DP049 :
		case    AKATSUKI    :
			dp049_bist_control(mode, gray_scan);
			break;
		case	B1 :
			b1_bist_control(mode, gray_scan);
			break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
			FUNC_END();
            return FAIL;

    }

	FUNC_END();
    return PASS;
}

int aod_control(int id,int model_index, int *onoff, char mode, char *pic_cmd ,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
	printf("%s: id = %d \n",__func__,id);
    switch(id)
    {

        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
			if(mode == 0xEC) //no pwm..(pwm:0xED, E5:0xF0)
			{
				joan_aod_mode_onoff(id,model_index,onoff,0);
				printf("JOAN E2 NO PWM MODE \n");
			}
			else
			{
				joan_aod_mode_onoff(id,model_index,onoff,1);
				printf("JOAN E2 PWM MODE \n");
			}
			system(pic_cmd);
			break;
        case    JOAN_E5 :
			if (mode == 2)
			{
            	joan_aod_mode_onoff(id,model_index,onoff,3);	/* set mode to 3 for new AOD code */
			}
			else
			{
            	joan_aod_mode_onoff(id,model_index,onoff,2);
			}
			system(pic_cmd);
			printf("JOAN E5 PWM MODE \n");
			break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
            mv_aod_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
            break;
        case    A1 :
            a1_aod_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
            break;
		case	DP049:
			dp049_aod_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
		break;
		
		case	AKATSUKI	:
            akatsuki_aod_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
        break;

        case    B1    :
	        b1_aod_mode_onoff(id,model_index,onoff,pic_cmd);
        break;

        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

	FUNC_END();
    return ret;
}

int vr_control(int id, int model_index, int *onoff, char mode, char *pic_cmd ,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
            if(mode == 0xEC) //no pwm..(pwm:0xED, E5:0xF0)
			{
                joan_vr_mode_onoff(id,model_index,onoff,0);
                printf("JOAN E2 NO PWM MODE \n");
            }
            else
			{
                joan_vr_mode_onoff(id,model_index, onoff,1);
                printf("JOAN E2 PWM MODE \n");
            }
			system(pic_cmd);
            break;
        case    JOAN_E5 :
            joan_vr_mode_onoff(id,model_index,onoff,1);
            printf("JOAN E5 PWM MODE \n");
			system(pic_cmd);
			break;
        case    MV :
        case    MV_MANUAL :
            mv_vr_mode_onoff(id,model_index, onoff);
			system(pic_cmd);
            break;
        case    A1 :
            a1_vr_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
            break;
        case    MV_MQA :
        case    MV_DQA :
            mv_mqa_vr_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
            break;
        case    DP049 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        case    AKATSUKI :
            akatsuki_vr_mode_onoff(id,model_index,onoff);
			system(pic_cmd);
            break;
		case B1	:
            b1_vr_mode_onoff(id,model_index,onoff,pic_cmd);
			break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

    system(pic_cmd);

	FUNC_END();
    return ret;
}

int emcon_control(int id, int *onoff, int mode, char *pic_cmd ,int if_prev) //emoff test
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
            if(mode == 0xEC) //no pwm..(pwm:0xED, E5:0xF0)
            {
                joan_emcon_mode_onoff(id,onoff,0);
                printf("JOAN E2 NO PWM MODE \n");
            }
            else
            {
                joan_emcon_mode_onoff(id,onoff,1);
                printf("JOAN E2 PWM MODE \n");
            }
            break;
        case    MV_MQA  :
        case    MV_DQA  :
            joan_emcon_mode_onoff(id,onoff,1);

            printf("MV_MQA / DQA MODE \n");
            break;
        case    JOAN_E5 :
			joan_emcon_mode_onoff(id, onoff, 1);
            printf("JOAN E2 PWM MODE \n");
            break;
        case    MV :
        case    MV_MANUAL :
			mv_emcon_mode_onoff(id, onoff, mode);
            break;
        case    A1 :
			a1_emcon_mode_onoff(id, onoff, mode);
			printf("A1 NO emcon function.. \n");
            break;
		case	DP049 :
		case	AKATSUKI	:
			dp049_emcon_mode_onoff(id, onoff);
			break;
        case    B1    :
	        b1_emcon_mode_onoff(id, onoff,pic_cmd);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
			break;
    }
	printf("EMCON [%d] \n",*onoff);
	system(pic_cmd);

	FUNC_END();
    return ret;
}

int variable_control(int id,int model_index, int *onoff, char *pic_cmd,int if_prev) //yark am sun test
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_variable_mode_onoff(id,model_index, onoff);
    		system(pic_cmd);
			break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
            joan_variable_mode_onoff(id,model_index, onoff);
    		system(pic_cmd);
            break;
        case    A1 :
            a1_variable_mode_onoff(id,model_index, onoff);
    		system(pic_cmd);
            break;
        case    DP049 :
		case AKATSUKI	:
            dp049_variable_mode_onoff(id,model_index, onoff);
    		system(pic_cmd);
            break;
        case B1   :
	        b1_variable_mode_onoff(id,model_index, onoff,pic_cmd,if_prev);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

	FUNC_END();
    return ret;
}

int blackpoint_control(int id, int model_index, int *onoff, char *pic_cmd ,int if_prev) //black  jum test
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_blackpoint_mode_onoff(id, model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
            mv_blackpoint_mode_onoff(id,model_index, onoff);
            break;
        case    A1 :
			printf("A1 NO blackpoint function.. \n");
            break;
        case    DP049 :
		case	AKATSUKI :
			if (id == AKATSUKI)
			{
				system("/Data/Pattern 11 B1");
			}
            dp049_blackpoint_mode_onoff(id,model_index, onoff);
            break;
        case    B1 :
            b1_blackpoint_mode_onoff(id,model_index, onoff,pic_cmd,if_prev);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

    system(pic_cmd);

	FUNC_END();
    return ret;
}

int bright_line_control(int id, int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
			joan_bright_line_mode_onoff(id,model_index,onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        case    DP049 :
		case	AKATSUKI :
            dp049_bright_line_mode_onoff(id,model_index,onoff);
            break;
        case    B1 :
            b1_bright_line_mode_onoff(id,model_index,onoff);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

    system(pic_cmd);

	FUNC_END();
    return ret;
}

int black_line_control(int id, int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_black_line_mode_onoff(id,model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        case    DP049 :
		case	AKATSUKI :
            dp049_black_line_mode_onoff(id,model_index, onoff);
            break;
        case    B1 :
            b1_black_line_mode_onoff(id,model_index, onoff);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }

    system(pic_cmd);

	FUNC_END();
    return ret;
}


int power_bright_line_control(int id,int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_power_bright_line_mode_onoff(id,model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }
    system(pic_cmd);

	FUNC_END();
    return ret;
}

int power_black_line_control(int id,int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_power_black_line_mode_onoff(id,model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;

        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }
    system(pic_cmd);

	FUNC_END();
    return ret;
}

int luminance_50per_power_bright_line_control(int id,int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_luminance_50per_power_bright_line_mode_onoff(id,model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }
    system(pic_cmd);

	FUNC_END();
    return ret;
}

int luminance_50per_power_black_line_control(int id,int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
            joan_luminance_50per_power_black_line_mode_onoff(id,model_index, onoff);
            break;
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
            printf("THIE MODEL haven't function for bright line function.. \n");
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }
    system(pic_cmd);

	FUNC_END();
    return ret;
}

int border_test_control(int id, int model_index, int *onoff, char *pic_cmd,int if_prev)
{
	int ret = PASS;

	FUNC_BEGIN();
    switch(id)
    {
        case    JOAN :
        case    JOAN_REL :
        case    JOAN_MANUAL :
        case    JOAN_E5 :
        case    MV :
        case    MV_MANUAL :
        case    MV_MQA :
        case    MV_DQA :
        case    A1 :
        case    DP049 :
        case    AKATSUKI :
            printf("THIE MODEL haven't function for border test function.. \n");
            break;
        case    B1 :
            b1_border_test_mode_onoff(id,model_index, onoff);
            break;
        default :
            printf("%s: MODEL ID ERR \n",__func__);
            ret  = FAIL;
            break;
    }
    system(pic_cmd);

	FUNC_END();
    return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

////////////////////////////////////////////////////////////////////
//OTP Func..//
///////////////////////////////////////////////////////////////////
int mv_joan_pocb_check(int id, unsigned char *ch1_pocb, unsigned char *ch2_pocb)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int ret_a = 0;
    int ret_b = 0;
    int DataLen = 3;
    unsigned char RdReg;
    int otp_flag = 0x0;
    int ch = 0;
    int ret;

	FUNC_BEGIN();
    memset(reg_buf, 0, sizeof(reg_buf));

    for(ch=1; ch<3; ch++)
    {
        /* Select Activate DSI Port */
        if(ch == 1)
            mipi_port_set(DSI_PORT_SEL_A);
        else if(ch == 2)
            mipi_port_set(DSI_PORT_SEL_B);
        else
        {
            printf("%s : CH SET FAIL[%d] \n",__func__,ch);
			FUNC_END();
            return FAIL;
        }
        m_delay(3);

	    PacketType = 0x39;
	    reg_buf[0] = 0xB0;
	    reg_buf[1] = 0x20;
	    reg_buf[2] = 0x43;
	    mipi_write(PacketType, reg_buf, 3);
	    m_delay(BASIC_DELAY+OTPD);


        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xDF;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
		if (ret < 0)
		{
			DERRPRINTF("mipi_read\n");
		}
		printf("CH%d POCB1 : 0x%X POCB2 : 0x%X \n",ch, reg_buf[0],reg_buf[1]);
        otp_flag = reg_buf[0];

		if(ch == 1)
		{
		    printf("POCB CH1 Result : ");
		
		    if (otp_flag & 0x08)
		    {
		        printf(":DISABLE\n");
				ret_a = 0;
		    }
		    else
			{
		        printf(":ENABLE\n");
				ret_a = 1;
			}
		}
		else if(ch == 2)
		{
		    printf("POCB CH2 Result : ");
		
		    if (otp_flag & 0x08)
		    {
		        printf(":DISABLE\n");
		    }
		    else
			{
		        printf(":ENABLE\n");
		        ret_b++;
			}
		}
		m_delay(BASIC_DELAY);
    }
	*ch1_pocb = ret_a;
	*ch2_pocb = ret_b;

	FUNC_END();
    return 0;
}


int mv_joan_otp_check(int id, unsigned char *ch1_otp, unsigned char *ch2_otp)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
	unsigned char base_block = 0x05;
	unsigned char RdReg;
	unsigned char DataLen = 3;
	int i = 0,ch = 0;
	int otp_flag[2][5] ={{0,},};
	int ret = 0;
	int mipi_fail = 0;

	FUNC_BEGIN();
	/* Write Packet Both Channel (DSI A/B) */
	for(ch = 1; ch < 3; ch++)
	{
		mipi_fail = 0;
		mipi_port_set(DSI_PORT_SEL_A+ch-1);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* SLEEP IN */
		PacketType = 0x05;
		reg_buf[0] = 0x10;
		mipi_write(PacketType, reg_buf, 1);
		usleep(5000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* MCS ACCESS */
		PacketType = 0x39;
		reg_buf[0] = 0xB0;
		reg_buf[1] = 0xA5;
		reg_buf[2] = 0x00;
		mipi_write(PacketType, reg_buf, 3);
		usleep(1000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* OTPCTL1 (0xF0) : OTP Block Select */
		PacketType = 0x39;
		reg_buf[0] = 0xF0;
		reg_buf[1] = 0x03;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x10;
		mipi_write(PacketType, reg_buf, 5);
		usleep(1000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* OTPCTL2 (0xF1) : OTP Initial */
		PacketType = 0x39;
		reg_buf[0] = 0xF1;
		reg_buf[1] = 0x40;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		mipi_write(PacketType, reg_buf, 6);
		usleep(1000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* OTPCTL2 (0xF1) */
		PacketType = 0x39;
		reg_buf[0] = 0xF1;
		reg_buf[1] = 0x41;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		mipi_write(PacketType, reg_buf, 6);
		usleep(1000);
		
		for(i=0; i<5; i++)
		{
		
			if(mipi_fail)
				continue;
		
		    memset(reg_buf, 0, sizeof(reg_buf));
		    /* OTPCTL2 (0xF1) : Select OTP Address */
		    PacketType = 0x39;
		    reg_buf[0] = 0xF1;
		    reg_buf[1] = 0x41;
		    reg_buf[2] = 0x00;
		    reg_buf[3] = 0x00;
		    reg_buf[4] = base_block + i;
		    reg_buf[5] = 0x00;
		    mipi_write(PacketType, reg_buf, 6);
			usleep(1000);
		
		    memset(reg_buf, 0, sizeof(reg_buf));
		    /* OTPCTL2 (0xF1) : MCS OTP Block*/
		    PacketType = 0x39;
		    reg_buf[0] = 0xF1;
		    reg_buf[1] = 0xC1;
		    mipi_write(PacketType, reg_buf, 2);
			usleep(1000);
		
		    memset(reg_buf, 0, sizeof(reg_buf));
		    PacketType = 0x39;
		    reg_buf[0] = 0xF1;
		    reg_buf[1] = 0x41;
		    mipi_write(PacketType, reg_buf, 2);
			usleep(1000);
		
			/* OTPCTRL3 (0xF2) */
			memset(reg_buf, 0, sizeof(reg_buf));
			PacketType = 0x06;
			RdReg = 0xF2;
			ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
			usleep(1000);
			if(ret <0)
			{
				printf("MIPI READ FAIL \n");
				mipi_fail = 1;
				continue;
			}
			
			otp_flag[ch-1][i] = reg_buf[0];
		}
		
		memset(reg_buf, 0, sizeof(reg_buf));
		    /* OTPCTL2 (0xF1) : OTP Initial */
		PacketType = 0x39;
		reg_buf[0] = 0xF1;
		reg_buf[1] = 0x41;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		mipi_write(PacketType, reg_buf, 6);
		usleep(1000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		/* OTPCTL2 (0xF1) */
		PacketType = 0x39;
		reg_buf[0] = 0xF1;
		reg_buf[1] = 0x00;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		reg_buf[5] = 0x00;
		mipi_write(PacketType, reg_buf, 6);
		usleep(1000);
		
		memset(reg_buf, 0, sizeof(reg_buf));
		PacketType = 0x39;
		reg_buf[0] = 0xF0;
		reg_buf[1] = 0x02;
		reg_buf[2] = 0x00;
		reg_buf[3] = 0x00;
		reg_buf[4] = 0x00;
		mipi_write(PacketType, reg_buf, 5);
		usleep(1000);
	}

	for(ch = 0; ch < 2; ch++)
	{
		printf("CH%d OTP -> ",ch+1);
		if((otp_flag[ch][0] == 0xAC) && (otp_flag[ch][1] == 0x00) && (otp_flag[ch][2] == 0x00) && (otp_flag[ch][3] == 0x00) && (otp_flag[ch][4] == 0x00))
		{
			if(ch ==0)
				*ch1_otp = 1;
			else
				*ch2_otp = 1;
			printf("1 write \n");
		}
		else if((otp_flag[ch][0] == 0xFF) && (otp_flag[ch][1] == 0xAC) && (otp_flag[ch][2] == 0x00) && (otp_flag[ch][3] == 0x00) && (otp_flag[ch][4] == 0x00))
        {
            if(ch ==0)
                *ch1_otp = 2;
            else
                *ch2_otp = 2;
            printf("2 write \n");
        }
		else if((otp_flag[ch][0] == 0xFF) && (otp_flag[ch][1] == 0xFF) && (otp_flag[ch][2] == 0xAC) && (otp_flag[ch][3] == 0x00) && (otp_flag[ch][4] == 0x00))
        {
            if(ch ==0)
                *ch1_otp = 3;
            else
                *ch2_otp = 3;
            printf("3 write \n");
        }
		else if((otp_flag[ch][0] == 0xFF) && (otp_flag[ch][1] == 0xFF) && (otp_flag[ch][2] == 0xFF) && (otp_flag[ch][3] == 0xAC) && (otp_flag[ch][4] == 0x00))
        {
            if(ch ==0)
                *ch1_otp = 4;
            else
                *ch2_otp = 4;
            printf("4 write \n");
        }
		else if((otp_flag[ch][0] == 0xFF) && (otp_flag[ch][1] == 0xFF) && (otp_flag[ch][2] == 0xFF) && (otp_flag[ch][3] == 0xFF) && (otp_flag[ch][4] == 0xAC))
        {
            if(ch ==0)
                *ch1_otp = 5;
            else
                *ch2_otp = 5;
            printf("5 write \n");
        }
		else if((otp_flag[ch][0] == 0x00) && (otp_flag[ch][1] == 0x00) && (otp_flag[ch][2] == 0x00) && (otp_flag[ch][3] == 0x00) && (otp_flag[ch][4] == 0x00))
        {
            if(ch ==0)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;
            printf("NO PROGRAM!! \n");
        }
		else
        {
            if(ch ==0)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;
            printf("OTP ERR \n");
        }

		if(DEBUG_MODE)
		{
			printf("CH%d raw DATA -> [%X][%X][%X][%X][%X] \n",ch+1,otp_flag[ch][0],otp_flag[ch][1],otp_flag[ch][2],otp_flag[ch][3],otp_flag[ch][4]);        
		}
	}

    printf("END \n");

	FUNC_END();
	return PASS;
}

int a1_otp_check(int id, unsigned char *ch1_otp, unsigned char *ch2_otp)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 3;
    int ch = 0;
    int ret = 0;
//    int mipi_fail = 0;

	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */
    for(ch = 1; ch < 3; ch++)
    {
//		mipi_fail = 0;
		mipi_port_set(DSI_PORT_SEL_A+ch-1);

	    memset(reg_buf, 0, sizeof(reg_buf));
		PacketType = 0x39;
	    reg_buf[0] = 0xF0;
	    reg_buf[1] = 0x5A;
	    reg_buf[2] = 0x5A;
	    mipi_write(PacketType, reg_buf, 3);
		usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);
        usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);
        usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xD9;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf); //Datalen originally..2
        usleep(1000);

		if(reg_buf[1] == 0x01)
		{
			if(ch == 1)
				*ch1_otp = 1;
			else
				*ch2_otp = 1;
			printf("CH %d OTP :  1 write.. \n",ch);
		}
		else if(reg_buf[1] == 0x03)
		{
            if(ch == 1)
                *ch1_otp = 2;
            else
                *ch2_otp = 2;
			printf("CH %d OTP :  2 write.. \n",ch);
		}
		else if(reg_buf[1] == 0x07)
		{
            if(ch == 1)
                *ch1_otp = 3;
            else
                *ch2_otp = 3;
			printf("CH %d OTP :  3 write.. \n",ch);
		}
		else if(reg_buf[1] == 0x00)
		{
            if(ch == 1)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;
			printf("CH %d OTP :  0 write.. \n",ch);
		}
		else
		{
            if(ch == 1)
                *ch1_otp = 4;
            else
                *ch2_otp = 4;
			printf("CH %d OTP :  over write..[0x%X] \n",ch,reg_buf[1]);
		}
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
//            mipi_fail = 1;
            if(ch == 1)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;

            continue;
        }
		if(DEBUG_MODE)
			printf("A1 OTP TEST [1]0x%X, [2]0x%X, [3]0x%X \n",reg_buf[0],reg_buf[1] ,reg_buf[2]);
	}

	FUNC_END();
	return	PASS;
}

int a1_pocb_check(int id, unsigned char *ch1_pocb, unsigned char *ch2_pocb)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 3;
    int ch = 0;
    int ret = 0;
//    int mipi_fail = 0;
	
	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */
    for(ch = 1; ch < 3; ch++)
    {
//        mipi_fail = 0;
        mipi_port_set(DSI_PORT_SEL_A+ch-1);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF0;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);
        usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF1;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);
        usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x39;
        reg_buf[0] = 0xF2;
        reg_buf[1] = 0x5A;
        reg_buf[2] = 0x5A;
        mipi_write(PacketType, reg_buf, 3);
        usleep(1000);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0x5A;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf); //Datalen originally..1
        usleep(1000);

        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
//            mipi_fail = 1;
            continue;
        }

        printf("A1 POCB TEST [1]0x%X, [2]0x%X, [3]0x%X \n",reg_buf[0],reg_buf[1] ,reg_buf[2] );

        if(reg_buf[0] == 0x00)
        {
            if(ch == 1)
                *ch1_pocb = 0;
            else
                *ch2_pocb = 0;
			printf("CH %d POCB : DISABLE\n",ch);
        }
        else if(reg_buf[0] == 0x01)
        {
            if(ch == 1)
                *ch1_pocb = 1;
            else
                *ch2_pocb = 1;
			printf("CH %d POCB : ENABLE\n",ch);
        }
		else
		{
			if(reg_buf[0] & 0x01)
			{
				if(ch == 1)
					*ch1_pocb = 1;
				else
					*ch2_pocb = 1;
				printf("CH %d POCB : MAYBE..ENABLE [0x%X] \n",ch,reg_buf[0]);
			}
			else
            {
                if(ch == 1)
                    *ch1_pocb = 0;
                else
                    *ch2_pocb = 0;
				printf("CH %d POCB : MAYBE..DISABLE [0x%X] \n",ch,reg_buf[0]);
            }
		}
    }

	FUNC_END();
	return PASS;
}

int dp049_otp_check(int id, unsigned char *ch1_otp, unsigned char *ch2_otp)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 3;
	int otp_flag[2] = {0,};
    int ch = 0;
    int ret = 0;
//    int mipi_fail = 0;

	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */
    for(ch = 1; ch < 3; ch++)
    {
//        mipi_fail = 0;
        mipi_port_set(DSI_PORT_SEL_A+ch-1);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xD9;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        usleep(1000000);
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
//            mipi_fail = 1;
        }

        otp_flag[ch-1]= reg_buf[0];
		printf("Data : 0x%X \n",otp_flag[ch-1]);
	}

    for(ch = 0; ch < 2; ch++)
    {
        printf("CH%d OTP -> ",ch+1);
        if(otp_flag[ch] == 0x70)
        {
            if(ch ==0)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;
			printf("NO PROGRAM!! \n");
        }
		else if(otp_flag[ch] == 0x50)
        {
            if(ch ==0)
                *ch1_otp = 1;
            else
                *ch2_otp = 1;
            printf("1 write \n");
        }
        else if(otp_flag[ch] == 0x30)
        {
            if(ch ==0)
                *ch1_otp = 2;
            else
                *ch2_otp = 2;
            printf("2 write \n");
        }
        else if(otp_flag[ch] == 0x10)
        {
            if(ch ==0)
                *ch1_otp = 3;
            else
                *ch2_otp = 3;
            printf("3 write \n");
        }
		else
        {
            if(ch ==0)
                *ch1_otp = 0;
            else
                *ch2_otp = 0;
			printf("OTP ERR!! \n");
        }
	}

	FUNC_END();
	return 0;
}

int dp049_pocb_check(int id, unsigned char *ch1_pocb, unsigned char *ch2_pocb)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 3;
    int ch = 0;
    int otp_flag[2] ={0,};
    int ret = 0;
//    int mipi_fail = 0;

	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */
    for(ch = 1; ch < 3; ch++)
    {
//        mipi_fail = 0;
        mipi_port_set(DSI_PORT_SEL_A+ch-1);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0x5A;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        usleep(1000000);
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
//            mipi_fail = 1;
        }

        otp_flag[ch-1]= reg_buf[0];
		printf("Data : 0x%X \n",otp_flag[ch-1]);
	}

    for(ch = 0; ch < 2; ch++)
    {
        printf("CH%d POCB -> ",ch+1);
        if(otp_flag[ch] == 0x00)
        {
            if(ch ==0)
                *ch1_pocb = 0;
            else
                *ch2_pocb = 0;
            printf("POCB DISABLE \n");
        }
        else if(otp_flag[ch] == 0x01)
        {
            if(ch ==0)
                *ch1_pocb = 1;
            else
                *ch2_pocb = 1;
            printf("POCB ENABLE \n");
        }
	}
	
	FUNC_END();
	return 0;
}

int b1_otp_check(int id, unsigned char *ch1_otp, unsigned char *ch2_otp)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 7;
    int otp_flag[2] = {0,};
    int ch = 0;
    int ret = 0;

	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */

	mipi_port_set(DSI_PORT_SEL_BOTH);

	memset(reg_buf, 0, sizeof(reg_buf));
	PacketType = 0x15;
	reg_buf[0] = 0xB0;
	reg_buf[1] = 0xAC;
	mipi_write(PacketType, reg_buf, 2);
	
    for(ch = 1; ch < 3; ch++)
    {
        mipi_port_set(DSI_PORT_SEL_A+ch-1);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xF5;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        usleep(400000);
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
        }

        otp_flag[ch-1]= reg_buf[4];
        printf("Data : 0x%X \n",otp_flag[ch-1]);
    }

	*ch1_otp = otp_flag[0];
	*ch2_otp = otp_flag[1];

	FUNC_END();
	return 0;
}

int b1_id_check(int id, unsigned char *ch1_id, unsigned char *ch2_id)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 7;
    int otp_flag[2] = {0,};
    int ch = 0;
    int ret = 0;

	FUNC_BEGIN();
    for(ch = 1; ch < 3; ch++)
    {
        mipi_port_set(DSI_PORT_SEL_A+ch-1);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xDB;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        usleep(400000);
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
        }

		if(reg_buf[0] == 0x28) // B1 Biel Type
			otp_flag[ch-1] = 1;
		else if(reg_buf[0] == 0x29) // B1 Lens Type
			otp_flag[ch-1] = 2;
		else // Data error
        	otp_flag[ch-1]= reg_buf[0];
			
        printf("Data : 0x%X(ID:0x%X) \n",reg_buf[0],otp_flag[ch-1]);
    }

    *ch1_id = otp_flag[0];
    *ch2_id = otp_flag[1];

	FUNC_END();
    return 0;
}

int b1_pocb_check(int id, unsigned char *ch1_pocb, unsigned char *ch2_pocb)
{
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    unsigned char RdReg;
    unsigned char DataLen = 3;
    int ch = 0;
    int ret = 0;
//    int mipi_fail = 0;

	FUNC_BEGIN();
    /* Write Packet Both Channel (DSI A/B) */

	printf("B1 POCB 0.3Ver [7b = 0 is enable]\n");
    for(ch = 0; ch < 2; ch++)
    {
//		mipi_fail = 0;

		mipi_port_set(DSI_PORT_SEL_A+ch);

        memset(reg_buf, 0, sizeof(reg_buf));
        PacketType = 0x06;
        RdReg = 0xEA;
		DataLen = 3;
        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        usleep(400000);
        if(ret <0)
        {
            printf("MIPI READ FAIL \n");
//            mipi_fail = 1;
        }

        printf("CH%d POCB -> ",ch+1);
	
        if(reg_buf[0] & 0x80)
        {
            printf("Data : 0x%X \n",reg_buf[0]);
            printf("POCB DISABLE \n");

            if(ch ==0)
                *ch1_pocb = 0;
            else
                *ch2_pocb = 0;

        }
        else if(!(reg_buf[0] & 0x80))
        {
			printf("Data : 0x%X \n",reg_buf[0]);
    	    printf("POCB ENABLE \n");

        	memset(reg_buf, 0, sizeof(reg_buf));
        	PacketType = 0x06;
        	RdReg = 0xCC;
        	DataLen = 6;
        	ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
        	usleep(400000);

	        if(ret <0)
	        {
	            printf("MIPI READ FAIL \n");
//	            mipi_fail = 1;
	        }
	
	        if((reg_buf[5] & 0x01))
	        {
	            printf("CH%d POCB -> ",ch+1);
	            printf(",But POCB Flash Read NG \n");
	
	            if(ch ==0)
	               *ch1_pocb = 2;
	             else
	               *ch2_pocb = 2;
				printf("Data : 0x%X \n",reg_buf[5]);
			}
			else
			{
    	        if(ch ==0)
    	            *ch1_pocb = 1;
    	        else
    	            *ch2_pocb = 1;
    	    }
		}
    }

	FUNC_END();
    return 0;
}

void OTP_uart_write(char ch,char ret, char max, unsigned char crc_ret, unsigned char arg1)
{
    unsigned char uart_buf[30] = {0,};

	FUNC_BEGIN();
    /* uart command */
    memset(uart_buf, 0, 30);
    uart_buf[4] = ret;
    uart_buf[5] = max;
	uart_buf[6] = crc_ret;
	uart_buf[7] = arg1;
	
    serial_packet_init(uart_buf, OTP, ch);
    serial_write_function(uart_buf);
	FUNC_END();
}

extern int flag_otp_test_result_ch1;
extern int flag_otp_test_result_ch2;

int otp_func(int  id,int model_index, unsigned char *ch1_pocb, unsigned char *ch2_pocb)
{
	/* initializing Model is needed after doing this function.. */

	int ret = PASS;
	unsigned char ch1_otp = 0, ch2_otp = 0;
	unsigned char ch1_temp = 0, ch2_temp = 0;
	char max = 0;
	char str[300]={0,};

	FUNC_BEGIN();
	if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL)||(id == MV)|| (id == MV_MANUAL) || (id == JOAN_E5) || (id == MV_MQA) || (id == MV_DQA))
	{
#ifdef VFOS_SITE_GUMI
		mipi_dev_open();
		decon_dev_open();
/*
		usleep(5000);
		decon_fb_init_release(0);
		usleep(5000);
		configure_dsi_read();*/
#else  //VIETNAM
        sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
        system(str);

		usleep(100000);
        mipi_dev_open();
        decon_dev_open();
#endif

		printf(">> OTP Read.. \n");
		ret = mv_joan_otp_check(id, &ch1_otp, &ch2_otp);
		usleep(100000);

		if((*ch1_pocb == 2) || (*ch2_pocb == 2))
		{
            printf(">> POCB Read.. \n");
            ret = mv_joan_pocb_check(id, ch1_pocb, ch2_pocb);
		}
		else
		{
            printf(">> already, POCB Read.. \n");
            printf("CH1 POCB [%d] \n CH2 POCB [%d] \n",*ch1_pocb,*ch2_pocb);
		}
		
		max = 5;
	}
	else if(id == A1)
	{
		mipi_dev_open();
		decon_dev_open();

//need debuging....
#if	0	/* swchoi - comment to use new way */
        usleep(5000);
        decon_fb_init_release(0);
        usleep(5000);
		system("/Data/Pattern 11"); //need debuging
        configure_dsi_read();
#else	/* swchoi - add to use new way */
		system("/Data/Pattern 11 B1"); //need debuging
#endif	/* swchoi - end */

		printf(">> OTP Read.. \n");
        ret = a1_otp_check(id, &ch1_otp, &ch2_otp);
		usleep(100000);

        if((*ch1_pocb == 2) || (*ch2_pocb == 2))
        {
            printf(">> POCB Read.. \n");
            ret = a1_pocb_check(id, ch1_pocb, ch2_pocb);
        }
        else
        {
            printf(">> already, POCB Read.. \n");
            printf("CH1 POCB [%d] \n CH2 POCB [%d] \n",*ch1_pocb,*ch2_pocb);
        }

        max = 3;
	}
	else if((id == DP049) || (id == AKATSUKI))
	{
		if(id == DP049)
		{
			sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);//test
			system(str);
		}

		usleep(100000);

	    mipi_dev_open();
	    decon_dev_open();

		printf(">> OTP Read.. \n");
        ret = dp049_otp_check(id, &ch1_otp, &ch2_otp);

        if((*ch1_pocb == 2) || (*ch2_pocb == 2))
        {
            printf(">> POCB Read.. \n");
            ret = dp049_pocb_check(id, ch1_pocb, ch2_pocb);
        }
        else
        {
            printf(">> already, POCB Read.. \n");
            printf("CH1 POCB [%d] \n CH2 POCB [%d] \n",*ch1_pocb,*ch2_pocb);
        }

        max = 3;
	}
	else if(id == B1)
	{
#if	0	/* swchoi - comment as actually it is not used */
        sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
//        system(str);
#endif	/* swchoi - end */
		usleep(100000);

        mipi_dev_open();
        decon_dev_open();

		printf(">> ID Read.. \n");
        ret = b1_id_check(id, &ch1_temp, &ch2_temp);
		printf(">> OTP Read.. \n");
        ret = b1_otp_check(id, &ch1_otp, &ch2_otp);

        if((*ch1_pocb == 2) || (*ch2_pocb == 2))
        {
            printf(">> POCB Read.. \n");
            ret = b1_pocb_check(id, ch1_pocb, ch2_pocb);
        }
        else
        {
            printf(">> already, POCB Read.. \n");
            printf("CH1 POCB [%d] \n CH2 POCB [%d] \n",*ch1_pocb,*ch2_pocb);
        }

        max = 5;
	}
	else
	{
		printf("%s : wrong MODEL ID[%d] \n",__func__,id);
		ret = FAIL;
	}

	if(ret != FAIL)
	{
		OTP_uart_write(1,ch1_otp,max,*ch1_pocb,ch1_temp);
		// LWG 190401
		if((ch1_otp > max) || (ch1_otp == 0)){
			flag_otp_test_result_ch1 = 2;
			ret = FAIL;
		}else{
			flag_otp_test_result_ch1 = 1;
			ret = PASS;
		}
		usleep(5000);
		OTP_uart_write(2,ch2_otp,max,*ch2_pocb,ch2_temp);
		if((ch2_otp > max) || (ch2_otp == 0)){
			flag_otp_test_result_ch2 = 2;
			ret = FAIL;
		}else if (ret != FAIL){		// ch1 must pass
			flag_otp_test_result_ch2 = 1;
			ret = PASS;
		}
	}
	mipi_dev_close();
	decon_dev_close();

	FUNC_END();
	return ret;
}

void temp_mipi_video_reset_for_b1(void)
{
	ioctl(decon_dev, _IOCTL_FB_CON_INIT, NULL);
	ioctl(mipi_dev,  _IOCTL_MIPI_CON_SET_LCD_AFTER, NULL); //hyelim
	
	ioctl(decon_dev, _IOCTL_FB_CON_RELEASE, NULL);
	ioctl(mipi_dev, _IOCTL_MIPI_CON_SET_DSIM_OFF, NULL);
}

