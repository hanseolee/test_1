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
#include	<fts_lgd_09.h>
#include	<fts_lgd_15.h>
#include	<fts_lgd_18.h>
#include	<fts_lgd_08.h>
#include	<i2c-dev.h>
#include	<stm_touch.h>

int dicOpen;
int dic_dev;

extern int flag_touch_test_result_ch1;
extern int flag_touch_test_result_ch2;

int stm_touch_limit_parser(MODEL id, char *m_name, struct stm_touch_limit* limit)
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
		FUNC_END();
        return -1;
    }

    sprintf(name_buf,"[%s]",m_name);
    printf("%s : %s surching... \n",__func__,name_buf);


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
/////////////////////////////////////////////
            if(!strcmp(token, "PRODUCT_ID"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->product_id = (unsigned short)strtoul(token,NULL,16);
                printf("PRODUCT_ID = 0x%X\n",limit->product_id);
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
                limit->config_ver = (unsigned short)strtoul(token,NULL,16);
                printf("CONFIG_VER = 0x%X\n",limit->config_ver);
            }
            if(!strcmp(token, "RELEASE_VER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->release_ver = (unsigned short)strtoul(token,NULL,16);
                printf("RELEASE_VER = 0x%X\n",limit->release_ver);
            }
            if(!strcmp(token, "OTP_PARAM"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->otp_param = (unsigned short)strtoul(token,NULL,16);
                printf("OTP_PARAM = 0x%X\n",limit->otp_param);
            }

            if(!strcmp(token, "OTP_PARAM_4"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->otp_param1 = (unsigned short)strtoul(token,NULL,16);
                printf("OTP_PARAM_4 = 0x%X\n",limit->otp_param1);
            }

            if(!strcmp(token, "OTP_PARAM_5"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->otp_param2 = (unsigned short)strtoul(token,NULL,16);
                printf("OTP_PARAM_5 = 0x%X\n",limit->otp_param2);
            }
            if(!strcmp(token, "OTP_PARAM_6"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->otp_param3 = (unsigned short)strtoul(token,NULL,16);
                printf("OTP_PARAM_6 = 0x%X\n",limit->otp_param3);
            }
            if(!strcmp(token, "PAT_CM_REFERENCE_RAW"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_cm_reference_raw[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_cm_reference_raw[1] = (unsigned int)strtoul(token,NULL,10);
                printf("PAT_CM_REFERENCE_RAW = [L:%d][H:%d]\n",limit->pat_cm_reference_raw[0], limit->pat_cm_reference_raw[1]);
            }
            if(!strcmp(token, "PAT_SELF_RAW_TX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_self_raw_tx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_self_raw_tx[1] = (unsigned int)strtoul(token,NULL,10);
                printf("PAT_SELF_RAW_TX = [L:%d][H:%d]\n", limit->pat_self_raw_tx[0],limit->pat_self_raw_tx[1]);

            }
            if(!strcmp(token, "PAT_SELF_RAW_RX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_self_raw_rx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->pat_self_raw_rx[1] = (unsigned int)strtoul(token,NULL,10);

                printf("PAT_SELF_RAW_RX = [L:%d][H:%d]\n",limit->pat_self_raw_rx[0],limit->pat_self_raw_rx[1]);
            }
            if(!strcmp(token, "CM_REFERENCE_RAW"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->cm_reference_raw[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->cm_reference_raw[1] = (int)strtoul(token,NULL,10);
                printf("CM_REFERENCE_RAW = [L:%d][H:%d]\n",limit->cm_reference_raw[0], limit->cm_reference_raw[1]);
            }
            if(!strcmp(token, "CM_REFERENCE_GAP"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->cm_reference_gap = (int)strtoul(token,NULL,10);
                printf("REFERENCE_GAP = %d\n",limit->cm_reference_gap);
            }
            if(!strcmp(token, "CM_JITTER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->cm_jitter[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->cm_jitter[1] = (int)strtoul(token,NULL,10);
                printf("CM_JITTER = [L:%d][H:%d]\n", limit->cm_jitter[0], limit->cm_jitter[1]);
            }
            if(!strcmp(token, "TOTAL_CX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->total_cx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->total_cx[1] = (unsigned int)strtoul(token,NULL,10);
                printf("TOTAL_CX = [L:%d][H:%d]\n",limit->total_cx[0],limit->total_cx[1]);
            }
            if(!strcmp(token, "SELF_RAW_TX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->self_raw_tx[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->self_raw_tx[1] = (int)strtoul(token,NULL,10);
                printf("SELF_RAW_TX = [L:%d][H:%d]\n", limit->self_raw_tx[0],limit->self_raw_tx[1]);

            }
            if(!strcmp(token, "SELF_RAW_RX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->self_raw_rx[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->self_raw_rx[1] = (int)strtoul(token,NULL,10);

                printf("SELF_RAW_RX = [L:%d][H:%d]\n",limit->self_raw_rx[0],limit->self_raw_rx[1]);
            }
            if(!strcmp(token, "LP_SELF_RAW_TX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_self_raw_tx[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_self_raw_tx[1] = (int)strtoul(token,NULL,10);
                printf("LP_SELF_RAW_TX = [L:%d][H:%d]\n", limit->lp_self_raw_tx[0],limit->lp_self_raw_tx[1]);
            }
            if(!strcmp(token, "LP_SELF_RAW_RX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_self_raw_rx[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_self_raw_rx[1] = (int)strtoul(token,NULL,10);

                printf("LP_SELF_RAW_RX = [L:%d][H:%d]\n",limit->lp_self_raw_rx[0],limit->lp_self_raw_rx[1]);
            }
            if(!strcmp(token, "SELF_IX_TX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->self_ix_tx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->self_ix_tx[1] = (unsigned int)strtoul(token,NULL,10);

                printf("SELF_IX_TX = [L:%d][H:%d]\n",limit->self_ix_tx[0], limit->self_ix_tx[1]);
            }
            if(!strcmp(token, "SELF_IX_RX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->self_ix_rx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->self_ix_rx[1] = (unsigned int)strtoul(token,NULL,10);
                printf("SELF_IX_RX = [L:%d][H:%d]\n", limit->self_ix_rx[0], limit->self_ix_rx[1]);
            }
            if(!strcmp(token, "CX2_HF"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->cx2_hf[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->cx2_hf[1] = (unsigned int)strtoul(token,NULL,10);
                printf("CX2_HF = [L:%d][H:%d]\n", limit->cx2_hf[0], limit->cx2_hf[1]);
            }
            if(!strcmp(token, "LP_RAW"))  ////
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_raw[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->lp_raw[1] = (unsigned int)strtoul(token,NULL,10);

                printf("LP_RAW = [L:%d][H:%d]\n",limit->lp_raw[0],limit->lp_raw[1]);
            }
            if(!strcmp(token, "HF_GAP_RX"))  ////
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->hf_gap_rx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->hf_gap_rx[1] = (unsigned int)strtoul(token,NULL,10);

                printf("HF_GAP_RX = [L:%d][H:%d]\n",limit->hf_gap_rx[0],limit->hf_gap_rx[1]);
            }
            if(!strcmp(token, "HF_GAP_TX"))  ////
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->hf_gap_tx[0] = (unsigned int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->hf_gap_tx[1] = (unsigned int)strtoul(token,NULL,10);

                printf("HF_GAP_TX = [L:%d][H:%d]\n",limit->hf_gap_tx[0],limit->hf_gap_tx[1]);
            }
            if(!strcmp(token, "SS_JITTER"))  ////
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->ss_jitter[0] = (int)strtoul(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->ss_jitter[1] = (int)strtoul(token,NULL,10);
                printf("SS_JITTER = [L:%d][H:%d]\n", limit->ss_jitter[0], limit->ss_jitter[1]);
            }

            if(!strcmp(token, "HF_TEST"))
            {
                token = strtok(NULL, TOKEN_SEP);
				if(!strcmp("ON",token))
				{
					limit->hf_test_mode = 1;
					printf("HF TEST ON MODE \n");
				}
				else if(!strcmp("OFF",token))
				{
					limit->hf_test_mode = 0;
					printf("HF TEST OFF MODE \n");
				}
				else
				{
					limit->hf_test_mode = 1;
					printf("HF TEST ON MODE \n");
				}
            }
////////////////////////////////////////////
            token = strtok(NULL, TOKEN_SEP_COMMA);
        }
    }

end:
    printf("END \n");
    fclose(fp);
	FUNC_END();
    return 0;
}

int stm_touch_limit_table_parser(MODEL id, char *m_name, struct stm_touch_limit* limit)
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

    unsigned int st_totalCx_MAX = 0,
                st_totalCx_MIN = 0,
                st_totalCx_Gap_Rx_MAX = 0,
                st_totalCx_Gap_Rx_MIN = 0,
                st_totalCx_Gap_Tx_MAX = 0,
                st_totalCx_Gap_Tx_MIN = 0,
                st_hf_TotalCx_Gap_Rx_MAX = 0,
                st_hf_TotalCx_Gap_Rx_MIN = 0,
                st_hf_TotalCx_Gap_Tx_MAX = 0,
                st_hf_TotalCx_Gap_Tx_MIN = 0;

	FUNC_BEGIN();

    sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_TABLE_FILE);

	if(id != limit->id)
	{
		printf("%s : model id FAIL [%d/%d] \n",__func__,id, limit->id);
		printf("%s : plz, excute touch_table_parser func first\n",__func__);
		FUNC_END();
		return FAIL;
	}

    if((fp=(fopen(file_name,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, file_name);
		FUNC_END();
        return -1;
    }

	sprintf(name_buf,"[%s]",m_name);
	printf("%s : %s surching... \n",__func__,name_buf);

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

            if(st_totalCx_MAX)
            {
				limit->totalCx_MAX[0][0] = 1;
                if(!limit->totalCx_MAX[0][1] || !limit->totalCx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->totalCx_MAX[0][1] = x;
                    limit->totalCx_MAX[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->totalCx_MAX[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->totalCx_MAX[0][2])) //2 :y
                                {
                                    st_totalCx_MAX = 0;
                                    row = 0;

                                    if(DEBUG_MODE)
                                    {
                                        printf("totalCx_MAX\n");
                                        printf("X=%d Y=%d \n",limit->totalCx_MAX[0][1],limit->totalCx_MAX[0][2]);
                                        for(i = 1; i<limit->totalCx_MAX[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->totalCx_MAX[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->totalCx_MAX[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->totalCx_MAX[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_totalCx_MIN)
            {
				limit->totalCx_MIN[0][0] = 1;
                if(!limit->totalCx_MIN[0][1] || !limit->totalCx_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->totalCx_MIN[0][1] = x;
                    limit->totalCx_MIN[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->totalCx_MIN[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->totalCx_MIN[0][2])) //2 :y
                                {
                                    st_totalCx_MIN = 0;
                                    row = 0;

                                    if(DEBUG_MODE)
                                    {
                                        printf("totalCx_MIN\n");
                                        printf("X=%d Y=%d \n",limit->totalCx_MIN[0][1],limit->totalCx_MIN[0][2]);
                                        for(i = 1; i<limit->totalCx_MIN[0][2]+1 ;i++)
                                        {
                                            for(j = 1; j<limit->totalCx_MIN[0][1]+1 ;j++)
                                            {
                                                printf(" %04d",limit->totalCx_MIN[i][j]);
                                            }
                                            printf("\n");
                                        }
                                    }
                                }
                                break;
                            }
                            limit->totalCx_MIN[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_totalCx_Gap_Rx_MAX)
            {
				limit->totalCx_Gap_Rx_MAX[0][0] = 1;
                if(!limit->totalCx_Gap_Rx_MAX[0][1] || !limit->totalCx_Gap_Rx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->totalCx_Gap_Rx_MAX[0][1] = x;
                    limit->totalCx_Gap_Rx_MAX[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->totalCx_Gap_Rx_MAX[0][1]+3; n++) //1 : x
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->totalCx_Gap_Rx_MAX[0][2])) //2 :y
                                {
                                    st_totalCx_Gap_Rx_MAX = 0;
                                    row = 0;

									if(DEBUG_MODE)
									{
										printf("totalCx_Gap_Rx_MAX\n");
										printf("X=%d Y=%d \n",limit->totalCx_Gap_Rx_MAX[0][1],limit->totalCx_Gap_Rx_MAX[0][2]);
										for(i = 1; i<limit->totalCx_Gap_Rx_MAX[0][2]+1 ;i++)
										{
										    for(j = 1; j<limit->totalCx_Gap_Rx_MAX[0][1]+1 ;j++)
										    {
										        printf(" %04d",limit->totalCx_Gap_Rx_MAX[i][j]);
										    }
										    printf("\n");
									    }
									}
                                }
                                break;
                            }
                            limit->totalCx_Gap_Rx_MAX[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_totalCx_Gap_Rx_MIN)
            {
				limit->totalCx_Gap_Rx_MIN[0][0] = 1;
                if(!limit->totalCx_Gap_Rx_MIN[0][1] || !limit->totalCx_Gap_Rx_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->totalCx_Gap_Rx_MIN[0][1] = x;
                    limit->totalCx_Gap_Rx_MIN[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->totalCx_Gap_Rx_MIN[0][1]+3; n++)
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);
                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->totalCx_Gap_Rx_MIN[0][2]))
                                {
                                    st_totalCx_Gap_Rx_MIN = 0;
                                    row = 0;
									if(DEBUG_MODE)
									{
										printf("totalCx_Gap_Rx_MIN\n");
										printf("X=%d Y=%d \n",limit->totalCx_Gap_Rx_MIN[0][1],limit->totalCx_Gap_Rx_MIN[0][2]);
										for(i = 1; i<limit->totalCx_Gap_Rx_MIN[0][2]+1 ;i++)
										{
										    for(j = 1; j<limit->totalCx_Gap_Rx_MIN[0][1]+1 ;j++)
										    {
										        printf(" %04d",limit->totalCx_Gap_Rx_MIN[i][j]);
										    }
										    printf("\n");
									    }
									}
                                }
                                break;
                            }
                            limit->totalCx_Gap_Rx_MIN[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_totalCx_Gap_Tx_MAX)
            {
				limit->totalCx_Gap_Tx_MAX[0][0] = 1;
                if(!limit->totalCx_Gap_Tx_MAX[0][1] || !limit->totalCx_Gap_Tx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=32; y=15;
					}
                    limit->totalCx_Gap_Tx_MAX[0][1] = x;
                    limit->totalCx_Gap_Tx_MAX[0][2] = y;
                }
                else
                {
					row++;
					int n = 1;
					for(n = 1; n < limit->totalCx_Gap_Tx_MAX[0][1]+3; n++)
					{
					    token = strtok(NULL, TOKEN_SEP_COMMA);
					    if(!strcmp(token,"E"))
					    {
					        if((row == limit->totalCx_Gap_Tx_MAX[0][2]))
					        {
					            st_totalCx_Gap_Tx_MAX = 0;
					            row = 0;
								if(DEBUG_MODE)
								{
									printf("totalCx_Gap_Tx_MAX\n");
									printf("X=%d Y=%d \n",limit->totalCx_Gap_Tx_MAX[0][1],limit->totalCx_Gap_Tx_MAX[0][2]);
									for(i = 1; i<limit->totalCx_Gap_Tx_MAX[0][2]+1 ;i++)
									{
									    for(j = 1; j<limit->totalCx_Gap_Tx_MAX[0][1]+1 ;j++)
									    {
									        printf(" %04d",limit->totalCx_Gap_Tx_MAX[i][j]);
									    }
									    printf("\n");
									}
								}
					        }
					        break;
					    }
					    limit->totalCx_Gap_Tx_MAX[row][n] = (int)strtoul(token,NULL,10);
					}
                }
            }
            else if(st_totalCx_Gap_Tx_MIN)
            {
				limit->totalCx_Gap_Tx_MIN[0][0] = 1;
                if(!limit->totalCx_Gap_Tx_MIN[0][1] || !limit->totalCx_Gap_Tx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=32; y=15;
					}
                    limit->totalCx_Gap_Tx_MIN[0][1] = x;
                    limit->totalCx_Gap_Tx_MIN[0][2] = y;
                }
                else
                {
					row++;
					int n = 1;
					for(n = 1; n < limit->totalCx_Gap_Tx_MIN[0][1]+3; n++)
					{
					    token = strtok(NULL, TOKEN_SEP_COMMA);
					    if(!strcmp(token,"E"))
					    {
					        if((row == limit->totalCx_Gap_Tx_MIN[0][2]))
					        {
					            st_totalCx_Gap_Tx_MIN = 0;
					            row = 0;
								if(DEBUG_MODE)
								{
									printf("totalCx_Gap_Tx_MIN\n");
									printf("X=%d Y=%d \n",limit->totalCx_Gap_Tx_MIN[0][1],limit->totalCx_Gap_Tx_MIN[0][2]);
									for(i = 1; i<limit->totalCx_Gap_Tx_MIN[0][2]+1 ;i++)
									{
									    for(j = 1; j<limit->totalCx_Gap_Tx_MIN[0][1]+1 ;j++)
									    {
									        printf(" %04d",limit->totalCx_Gap_Tx_MIN[i][j]);
									    }
									    printf("\n");
								    }
								}
					        }
					        break;
					    }
					    limit->totalCx_Gap_Tx_MIN[row][n] = (int)strtoul(token,NULL,10);
					}
                }
            }
            else if(st_hf_TotalCx_Gap_Rx_MAX)
            {
				limit->hf_TotalCx_Gap_Rx_MAX[0][0] = 1;
                if(!limit->hf_TotalCx_Gap_Rx_MAX[0][1] || !limit->hf_TotalCx_Gap_Rx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->hf_TotalCx_Gap_Rx_MAX[0][1] = x;
                    limit->hf_TotalCx_Gap_Rx_MAX[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->hf_TotalCx_Gap_Rx_MAX[0][1]+3; n++)
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);

                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->hf_TotalCx_Gap_Rx_MAX[0][2]))
                                {
                                    st_hf_TotalCx_Gap_Rx_MAX = 0;
                                    row = 0;

									if(DEBUG_MODE)
									{
									    printf("hf_TotalCx_Gap_Rx_MAX\n");
									    printf("X=%d Y=%d \n",limit->hf_TotalCx_Gap_Rx_MAX[0][1],limit->hf_TotalCx_Gap_Rx_MAX[0][2]);
									    for(i = 1; i<limit->hf_TotalCx_Gap_Rx_MAX[0][2]+1 ;i++)
									    {
									        for(j = 1; j<limit->hf_TotalCx_Gap_Rx_MAX[0][1]+1 ;j++)
									        {
									            printf(" %04d",limit->hf_TotalCx_Gap_Rx_MAX[i][j]);
									        }
									        printf("\n");
									    }
									}
                                }
                                break;
                            }
                            limit->hf_TotalCx_Gap_Rx_MAX[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_hf_TotalCx_Gap_Rx_MIN)
            {
				limit->hf_TotalCx_Gap_Rx_MIN[0][0] = 1;
                if(!limit->hf_TotalCx_Gap_Rx_MIN[0][1] || !limit->hf_TotalCx_Gap_Rx_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=31; y=16;
					}
                    limit->hf_TotalCx_Gap_Rx_MIN[0][1] = x;
                    limit->hf_TotalCx_Gap_Rx_MIN[0][2] = y;
                }
                else
                {
                    if(!strcmp(token,"S"))
                    {
                        row++;
                        int n = 1;
                        for(n = 1; n < limit->hf_TotalCx_Gap_Rx_MIN[0][1]+3; n++)
                        {
                            token = strtok(NULL, TOKEN_SEP_COMMA);

                            if(!strcmp(token,"E"))
                            {
                                if((row == limit->hf_TotalCx_Gap_Rx_MIN[0][2]))
                                {
                                    st_hf_TotalCx_Gap_Rx_MIN = 0;
                                    row = 0;

									if(DEBUG_MODE)
									{
									    printf("hf_TotalCx_Gap_Rx_MIN\n");
									    printf("X=%d Y=%d \n",limit->hf_TotalCx_Gap_Rx_MIN[0][1],limit->hf_TotalCx_Gap_Rx_MIN[0][2]);
									    for(i = 1; i<limit->hf_TotalCx_Gap_Rx_MIN[0][2]+1 ;i++)
									    {
									        for(j = 1; j<limit->hf_TotalCx_Gap_Rx_MIN[0][1]+1 ;j++)
									        {
									            printf(" %04d",limit->hf_TotalCx_Gap_Rx_MIN[i][j]);
									        }
									        printf("\n");
									    }
									}
                                }
                                break;
                            }
                            limit->hf_TotalCx_Gap_Rx_MIN[row][n] = (int)strtoul(token,NULL,10);
                        }
                    }
                }
            }
            else if(st_hf_TotalCx_Gap_Tx_MAX)
            {
				limit->hf_TotalCx_Gap_Tx_MAX[0][0] = 1;
                if(!limit->hf_TotalCx_Gap_Tx_MAX[0][1] || !limit->hf_TotalCx_Gap_Tx_MAX[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=32; y=15;
					}
                    limit->hf_TotalCx_Gap_Tx_MAX[0][1] = x;
                    limit->hf_TotalCx_Gap_Tx_MAX[0][2] = y;
                }
                else
                {
					row++;
					int n = 1;
					for(n = 1; n < limit->hf_TotalCx_Gap_Tx_MAX[0][1]+3; n++)
					{
					    token = strtok(NULL, TOKEN_SEP_COMMA);
					
					    if(!strcmp(token,"E"))
					    {
					        if((row == limit->hf_TotalCx_Gap_Tx_MAX[0][2]))
					        {
					            st_hf_TotalCx_Gap_Tx_MAX = 0;
					            row = 0;
					
								if(DEBUG_MODE)
								{
									printf("hf_TotalCx_Gap_Tx_MAX\n");
									printf("X=%d Y=%d \n",limit->hf_TotalCx_Gap_Tx_MAX[0][1],limit->hf_TotalCx_Gap_Tx_MAX[0][2]);
									for(i = 1; i<limit->hf_TotalCx_Gap_Tx_MAX[0][2]+1 ;i++)
									{
									    for(j = 1; j<limit->hf_TotalCx_Gap_Tx_MAX[0][1]+1 ;j++)
									    {
									        printf(" %04d",limit->hf_TotalCx_Gap_Tx_MAX[i][j]);
									    }
									    printf("\n");
									}
								}
					        }
					        break;
					    }
					    limit->hf_TotalCx_Gap_Tx_MAX[row][n] = (int)strtoul(token,NULL,10);
					}
                }
            }
            else if(st_hf_TotalCx_Gap_Tx_MIN)
            {
				limit->hf_TotalCx_Gap_Tx_MIN[0][0] = 1;
                if(!limit->hf_TotalCx_Gap_Tx_MIN[0][1] || !limit->hf_TotalCx_Gap_Tx_MIN[0][2])
                {
                    if((x == 0) || (y == 0))
					{
                        x=32; y=15;
					}
                    limit->hf_TotalCx_Gap_Tx_MIN[0][1] = x;
                    limit->hf_TotalCx_Gap_Tx_MIN[0][2] = y;
                }
                else
                {
					row++;
					int n = 1;
					for(n = 1; n < limit->hf_TotalCx_Gap_Tx_MIN[0][1]+3; n++)
					{
					    token = strtok(NULL, TOKEN_SEP_COMMA);
					
					    if(!strcmp(token,"E"))
					    {
					        if((row == limit->hf_TotalCx_Gap_Tx_MIN[0][2]))
					        {
					            st_hf_TotalCx_Gap_Tx_MIN = 0;
					            row = 0;
					
								if(DEBUG_MODE)
								{
									printf("hf_TotalCx_Gap_Tx_MIN\n");
									printf("X=%d Y=%d \n",limit->hf_TotalCx_Gap_Tx_MIN[0][1],limit->hf_TotalCx_Gap_Tx_MIN[0][2]);
									for(i = 1; i<limit->hf_TotalCx_Gap_Tx_MIN[0][2]+1 ;i++)
									{
									    for(j = 1; j<limit->hf_TotalCx_Gap_Tx_MIN[0][1]+1 ;j++)
									    {
									        printf(" %04d",limit->hf_TotalCx_Gap_Tx_MIN[i][j]);
									    }
									    printf("\n");
									}
								}
					        }
					        break;
					    }
					    limit->hf_TotalCx_Gap_Tx_MIN[row][n] = (int)strtoul(token,NULL,10);
					}
                }
            }
            if(!strcmp(token, "TotalCx_MAX"))
            {
				st_totalCx_MAX = 1;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_MAX\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "TotalCx_MIN"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 1;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_MIN\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "TotalCx_Gap_Rx_MAX"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 1;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_Gap_Rx_MAX\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "TotalCx_Gap_Rx_MIN"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 1;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_Gap_Rx_MIN\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "TotalCx_Gap_Tx_MAX"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 1;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_Gap_Tx_MAX\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "TotalCx_Gap_Tx_MIN"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 1;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nTotalCx_Gap_Tx_MIN\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "HF_TotalCx_Gap_Rx_MAX"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 1;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nHF_TotalCx_Gap_Rx_MAX\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "HF_TotalCx_Gap_Rx_MIN"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 1;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nHF_TotalCx_Gap_Rx_MIN\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "HF_TotalCx_Gap_Tx_MAX"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 1;
                st_hf_TotalCx_Gap_Tx_MIN = 0;

				printf("\nHF_TotalCx_Gap_Tx_MAX\n");
                row = 0;
                x = 0;
                y = 0;
            }
            else if(!strcmp(token, "HF_TotalCx_Gap_Tx_MIN"))
            {
                st_totalCx_MAX = 0;
                st_totalCx_MIN = 0;
                st_totalCx_Gap_Rx_MAX = 0;
                st_totalCx_Gap_Rx_MIN = 0;
                st_totalCx_Gap_Tx_MAX = 0;
                st_totalCx_Gap_Tx_MIN = 0;
                st_hf_TotalCx_Gap_Rx_MAX = 0;
                st_hf_TotalCx_Gap_Rx_MIN = 0;
                st_hf_TotalCx_Gap_Tx_MAX = 0;
                st_hf_TotalCx_Gap_Tx_MIN = 1;

				printf("\nHF_TotalCx_Gap_Tx_MIN\n");
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
int stm_control(MODEL	id, struct stm_touch_limit *t_limit)
{
	int i2c_dev;
    unsigned long funcs;
	int ch = 0;
	int i2c_err = 0;
	int tmp = 0;
	unsigned char uart_buf[30] ={0,};
	int (*fts_panel_test) (int, unsigned char*) = NULL;
	void (*i2c_dev_match) (int) = NULL;
	unsigned int i2c_addr = 0;

	FUNC_BEGIN();
	if((id == JOAN) || (id== JOAN_REL)||(id == JOAN_MANUAL) || (id == MV)|| (id == MV_MANUAL)|| (id == JOAN_E5) || (id == MV_MQA) || (id == MV_DQA))
	{
		fts_panel_test = fts_panel_test_v9;
		limit_data_match_v9(id, t_limit);
		i2c_dev_match = i2c_dev_match_v9;
		i2c_addr = FTS_I2C_ADDR_V9;
	}
	else if(id == A1)
	{
        fts_panel_test = fts_panel_test_v15;
        limit_data_match_v15(id, t_limit);
        i2c_dev_match = i2c_dev_match_v15;
        i2c_addr = FTS_I2C_ADDR_V15;
	}
	else if((id == DP049))
	{
        fts_panel_test = fts_panel_test_v18;
        limit_data_match_v18(id, t_limit);
        i2c_dev_match = i2c_dev_match_v18;
        i2c_addr = FTS_I2C_ADDR_V18;
	}
	else if(id == B1)
	{
        fts_panel_test = fts_panel_test_v8;
        limit_data_match_v8(id, t_limit);
        i2c_dev_match = i2c_dev_match_v8;
        i2c_addr = FTS_I2C_ADDR_V8;
	}
	else
		printf("This model do not use Touch Test [id:%d]\n",id);

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

	        if (ioctl(i2c_dev, I2C_SLAVE_FORCE, i2c_addr>>1) < 0) {
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
            uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
            uart_buf[10] = t_limit->hf_test_mode;

			printf("I2C FAIL \n ==================================================================\n");
		}
		else
		{
	
			printf("touch i2c init completed..\n");
			i2c_dev_match(i2c_dev);
			//////////////////////////////////////////////////////////////

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
	
			/////////////////////////////////////////////////////////////

			usleep(25000);

			fts_panel_test(id, uart_buf);
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
			if(ch==1){
				flag_touch_test_result_ch1 = 2;
			}else{
				flag_touch_test_result_ch2 = 2;
			}
		}else{
			if(ch==1){
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
	///////////////////////////////////////////////////////////////
	}
	FUNC_END();
    return 0;
}

