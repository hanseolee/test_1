///////////// ATMEL REFERENCE MODIFY FOR VFOS_UH ///////////////



#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>     //usleep 
#include <stdlib.h>     //system, strtoul 
#include <i2c-dev.h>
#include <type.h>
#include <atmel_touch.h>
#include <atmel_mxt540s.h>
#include <rs485.h>


#define TOKEN_SEP_COMMA				", \t\r\n"
#define TOKEN_SEP					" \t\r\n"


int dic_dev;
extern int flag_touch_test_result_ch1;
extern int flag_touch_test_result_ch2;

int atmel_touch_limit_parser(int id, char *m_name, struct atmel_touch_limit* limit)
{

    char string[500];
    FILE *fp;
    char *token = NULL;
    char file_name[50];
    char name_buf[30] = {0,};
    int parsing_en = 0;

	FUNC_BEGIN();

    sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_FILE);
    limit->id = id;

    if((fp=(fopen(file_name,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, file_name);
        return -1;
    }

    sprintf(name_buf,"[%s]",m_name);
    printf("%s : %s searching... \n",__func__,name_buf);

    while((fgets(string, 500, fp)) != NULL){
        token = strtok(string, TOKEN_SEP_COMMA);
        while(token != NULL){

            if(!parsing_en)
            {
                if(!strcmp(token,name_buf))
                {
                    printf("Found LIMIT DATA --> %s \n",name_buf);
                    printf("%s : %s limit parser START.. \n",__func__,name_buf);
                    parsing_en = 1;
                }
                break;
            }
            else
            {
                if(!strcmp(token,"END"))
                {
                    parsing_en = 0;
                    goto end;
                }
            }

            if(!strcmp(token, "FW_VER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->fw_ver = (unsigned short)strtoul(token,NULL,16);
                printf("FW_VER = 0x%X\n",limit->fw_ver);
            }
            if(!strcmp(token, "CONFIG_VER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->config_ver = (unsigned int)strtoul(token,NULL,16);
                printf("CONFIG_VER = 0x%X\n",limit->config_ver);
            }
            if(!strcmp(token, "MD_SET_CHARGE"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->md_set_charge = strtof(token,NULL);
                printf("Micro Defect Set Charge = %lf\n",limit->md_set_charge);
            }
            if(!strcmp(token, "MD_X_CENTER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->md_x_center = (double)strtof(token,NULL);
                printf("MD_X_CENTER = %lf  \n",limit->md_x_center);
            }
            if(!strcmp(token, "MD_Y_CENTER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->md_y_center = (double)strtof(token,NULL);
                printf("MD_Y_CENTER = %lf  \n",limit->md_y_center);
            }
            if(!strcmp(token, "MD_X_BORDER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->md_x_border = (double)strtof(token,NULL);
                printf("MD_X_BORDER = %lf  \n",limit->md_x_border);
            }
            if(!strcmp(token, "MD_Y_BORDER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->md_y_border = (double)strtof(token,NULL);
                printf("MD_Y_BORDER = %lf  \n",limit->md_y_border);
            }

            token = strtok(NULL, TOKEN_SEP_COMMA);
        }
    }

end:
    printf("END \n");
    fclose(fp);
	FUNC_END();
    return 0;
}


int atmel_touch_limit_table_parser(int id, char *m_name, struct atmel_touch_limit* limit)
{
    char string[500];
    FILE *fp;
    char *token = NULL;
    char file_name[50];
    char name_buf[30]={0,};
    int row = 0;
    unsigned int x = 0, y =0;
    unsigned int i = 0, j =0;
    int parsing_en = 0;

    unsigned int st_NodeDetection_MAX = 0,
				 st_NodeDetection_MIN = 0,
				 st_Delta_MAX = 0,
				 st_Delta_MIN = 0;

	FUNC_BEGIN();

    sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_TABLE_FILE);

    if(id != limit->id)
    {
        printf("%s : model id FAIL [%d/%d] \n",__func__,id, limit->id);
        printf("%s : plz, excute touch_table_parser func first\n",__func__);
        return -1;
        //return FAIL;
    }

    if((fp=(fopen(file_name,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, file_name);
        return -1;
    }

    sprintf(name_buf,"[%s]",m_name);
    printf("%s : %s searching... \n",__func__,name_buf);

    while((fgets(string, 500, fp)) != NULL){
        token = strtok(string, TOKEN_SEP_COMMA);
        while(token != NULL){

            if(!parsing_en)
            {
                if(!strcmp(token,name_buf))
                {
                    printf("Found LIMIT DATA --> %s \n",name_buf);
                    printf("%s : %s limit parser START.. \n",__func__,name_buf);
                    parsing_en = 1;
                }
                break;
            }
            else
            {
                if(!strcmp(token,"END"))
                {
                    parsing_en = 0;
                    goto end;
                }
            }

            if(!strcmp(token, "X"))
            {
                token = strtok(NULL, TOKEN_SEP_COMMA);
                x = (unsigned int)strtoul(token, NULL,10);
                if(!strcmp(strtok(NULL, TOKEN_SEP_COMMA),"Y"))
                {
                    token = strtok(NULL, TOKEN_SEP_COMMA);
                    y = (unsigned int)strtoul(token, NULL,10);
                }
            }

            if(st_NodeDetection_MAX)
            {
                limit->nodeDetection_MAX[0][0] = 1;
                if(!limit->nodeDetection_MAX[0][1] || !limit->nodeDetection_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=16; y=32;
						printf("Default X(%d),Y(%d) Set \n",x,y);
					}
                    limit->nodeDetection_MAX[0][1] = x;
                    limit->nodeDetection_MAX[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->nodeDetection_MAX[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->nodeDetection_MAX[0][2])) //2 :y
                                {
                                    st_NodeDetection_MAX = 0;
                                    row = 0;

                                    if(DEBUG_MODE)
                                    {
                                        printf("NodeDetection_MAX\n");
                                        printf("X=%d Y=%d \n",limit->nodeDetection_MAX[0][1],limit->nodeDetection_MAX[0][2]);
                                        for(i = 1; i<limit->nodeDetection_MAX[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->nodeDetection_MAX[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->nodeDetection_MAX[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->nodeDetection_MAX[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_NodeDetection_MIN)
            {
                limit->nodeDetection_MIN[0][0] = 1;
                if(!limit->nodeDetection_MIN[0][1] || !limit->nodeDetection_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
                    {
                        x=16; y=32;
                        printf("Default X(%d),Y(%d) Set \n",x,y);
                    } 
                    limit->nodeDetection_MIN[0][1] = x;
                    limit->nodeDetection_MIN[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->nodeDetection_MIN[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->nodeDetection_MIN[0][2])) //2 :y
                                {
                                    st_NodeDetection_MIN = 0;
                                    row = 0;

                                    if(DEBUG_MODE)
                                    {
                                        printf("NodeDetection_MIN\n");
                                        printf("X=%d Y=%d \n",limit->nodeDetection_MIN[0][1],limit->nodeDetection_MIN[0][2]);
                                        for(i = 1; i<limit->nodeDetection_MIN[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->nodeDetection_MIN[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->nodeDetection_MIN[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->nodeDetection_MIN[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_Delta_MAX)
            {
                limit->delta_MAX[0][0] = 1;
                if(!limit->delta_MAX[0][1] || !limit->delta_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
                    {
                        x=16; y=32;
                        printf("Default X(%d),Y(%d) Set \n",x,y);
                    } 
                    limit->delta_MAX[0][1] = x;
                    limit->delta_MAX[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->delta_MAX[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->delta_MAX[0][2])) //2 :y
                                {
                                    st_Delta_MAX = 0;
                                    row = 0;

                                    if(DEBUG_MODE)
                                    {
                                        printf("Delta_MAX\n");
                                        printf("X=%d Y=%d \n",limit->delta_MAX[0][1],limit->delta_MAX[0][2]);
                                        for(i = 1; i<limit->delta_MAX[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->delta_MAX[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->delta_MAX[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->delta_MAX[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_Delta_MIN)
            {
                limit->delta_MIN[0][0] = 1;
                if(!limit->delta_MIN[0][1] || !limit->delta_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
                    {
                        x=16; y=32;
                        printf("Default X(%d),Y(%d) Set \n",x,y);
                    } 
                    limit->delta_MIN[0][1] = x;
                    limit->delta_MIN[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->delta_MIN[0][1]+3; n++)
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->delta_MIN[0][2]))
                                {
                                    st_Delta_MIN = 0;
                                    row = 0;
                                    if(DEBUG_MODE)
                                    {
                                        printf("Delta_MIN\n");
                                        printf("X=%d Y=%d \n",limit->delta_MIN[0][1],limit->delta_MIN[0][2]);
                                        for(i = 1; i<limit->delta_MIN[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->delta_MIN[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->delta_MIN[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->delta_MIN[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }

            if(!strcmp(token, "NodeDetection_MAX"))
            {
				st_NodeDetection_MAX = 1;
                st_NodeDetection_MIN = 0;
                st_Delta_MAX = 0;
                st_Delta_MIN = 0;

                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "NodeDetection_MIN"))
            {

                st_NodeDetection_MAX = 0;
                st_NodeDetection_MIN = 1;
                st_Delta_MAX = 0;
                st_Delta_MIN = 0;

                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "Delta_MAX"))
            {
                st_NodeDetection_MAX = 0;
                st_NodeDetection_MIN = 0;
                st_Delta_MAX = 1;
                st_Delta_MIN = 0;

                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "Delta_MIN"))
            {
                st_NodeDetection_MAX = 0;
                st_NodeDetection_MIN = 0;
                st_Delta_MAX = 0;
                st_Delta_MIN = 1;

                row = 0;
                x = 0;
                y = 0;

            }

            token = strtok(NULL, TOKEN_SEP_COMMA);
        }
    }

end:
    printf("END \n");
    fclose(fp);
	FUNC_END();
    return 0;

}


char ch1_TDev[30];
char ch2_TDev[30];

extern int flag_touch_test_result_ch1;
extern int flag_touch_test_result_ch2;
int atmel_control(MODEL   id, struct atmel_touch_limit *t_limit)
{

	int i2c_err = 0;
    int i2c_dev;
    unsigned long funcs;
	int ch = 0;
	int tmp = 0;
	unsigned char uart_buf[30] ={0,};

	FUNC_BEGIN();
    for(ch = 1; ch<3; ch++)
    {
		memset(uart_buf,0,sizeof(uart_buf));
        if(ch == 1)
            i2c_dev = open(ch1_TDev, O_RDWR);
        else if(ch == 2)
            i2c_dev = open(ch2_TDev, O_RDWR);

        printf("\n============================  [C H %d] ============================\n",ch);

        if(i2c_dev >= 0)
        {
            if (ioctl(i2c_dev, I2C_FUNCS, &funcs) < 0) {
                fprintf(stderr, "Error: Could not get the adapter "
                "functionality matrix: %s\n", strerror(errno));
                i2c_err = 1;
            }

            if (ioctl(i2c_dev, I2C_SLAVE_FORCE, TC_SLAVE_ADDR) < 0) {
                fprintf(stderr, "Error: Could not set address \n");
                i2c_err = 1;
            }
        }
        else
            i2c_err = 1;

        if(i2c_err)
        {
            ////////////////////I2C FAIL////////////////
            close(i2c_dev);
            i2c_err = 0;
            uart_buf[4] |= 0xFF;
            uart_buf[5] |= 0xFF;
            uart_buf[6] |= 0xFF;
            uart_buf[7] |= 0xFF;
            printf("I2C FAIL \n ==================================================================\n");
        }
        else
        {
            printf("touch i2c init completed..\n");

			limit_data_match_v540(id,t_limit);
			i2c_dev_match_v540(I2C_MODE_GENERAL, i2c_dev);

            tmp = 0;

            ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &tmp);
            ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &tmp);
            ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);

            usleep(1000);
            tmp = 1;

            ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &tmp);
            ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &tmp);
            ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);

            usleep(25000);

			mxt_panel_test(id,uart_buf,I2C_MODE_GENERAL);	
            printf("==================================================================\n");

            tmp = 0;
            ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &tmp);
            ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &tmp);

            usleep(25000);

            close(i2c_dev);
            i2c_err = 0;
        }

        serial_packet_init(uart_buf, TOUCH, ch);

        printf("uart_buf : ");
        int i;
        for(i = 0; i < 30;i++)
            printf("0x%X, ",uart_buf[i]);
        printf("\n");

		if( 
			(uart_buf[4]&0xFF) | ((uart_buf[5]<<8)&(0xFF<<8)) | ((uart_buf[6]<<16)&(0xFF<<16)) | ((uart_buf[7]<<24)&(0xFF<<24))){
			if(ch == 1){
				flag_touch_test_result_ch1 = 2;
			}else{
				flag_touch_test_result_ch2 = 2;
			}
		}else{
			if(ch == 1){
				if(flag_touch_test_result_ch1 != 2){
					flag_touch_test_result_ch1 = 1;
				}
			}else{
				if(flag_touch_test_result_ch2 != 2){
					flag_touch_test_result_ch2 = 1;
				}
			}
		}
        serial_write_function(uart_buf);
	}

	FUNC_END();

	return 0;
} //main end


