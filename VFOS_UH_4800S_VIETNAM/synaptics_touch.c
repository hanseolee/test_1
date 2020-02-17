
#if 1
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
#endif

#include <type.h>
#include <rs485.h>
#include <i2c-dev.h>
#include <model_common.h>
#include <model_storm.h>
#include <model_dp076.h>
#include <synaptics_touch.h>

#define _IOCTL_CH1_TE_GET               0x1004
#define _IOCTL_CH2_TE_GET               0x1005

#if 1 //temp
const char OperationStatus[11][35] =
{
    "Success",
    "Device Not In Bootloader Mode",//0x01
    "Invalid Partition",
    "Invalid Command",
    "Invalid Block Offset",
    "Invalid Transfer",
    "Not Erased",
    "Flash Programming Key Incorrect",
    "Bad Partition Table",
    "Checksum Failed",
    "Flash Hardware Failure"
};

const char Dev_ConfigStatus[4][20] =
{
    "Default",
    "Temporary",
    "Fixed",
    "Reserved"
};

const char BL7_CommandString[7][20] =
{
    "Idle",
    "Enter Bootloader",
    "Read",
    "Write",
    "Erase",
    "Erase Application",
    "Sensor ID",
};
const char PartitionName[PartitionCount][30] =
{
    "Reserved",
    "Bootloader",
    "Device Config",
    "Flash Config",
    "Manufacturing Block",
    "Customer Serialization",
    "Global Parameters",
    "Core Code",
    "Core Config",
    "Guest Code",
    "Display Config",
    "External Touch AFE"
};
#endif

#define	R_OUT	-4
#define	TX_RESULT	0
#define	RX_RESULT	1
#define OTP_WRITE_RESULT   0
#define OTP_COUNT_RESULT   1

#define	ONE_BLOCK_TOTAL_BYTE		16

#define	OTP_ONE_PART_TOTAL_BLOCK		3
#define	OTP_TOTAL_PART	6				//OTP memory range is "3 block / 1 part", total 6 part(18 block)
#define	OTP_TOTAL_PART_TOTAL_BLOCK	OTP_ONE_PART_TOTAL_BLOCK*OTP_TOTAL_PART

#define	PRODUCT_ID_TOTAL_PART				1
#define	PRODUCT_ID_ONE_PART_TOTAL_BLOCK		1 //PRODUCT ID memory range is "1 block / 1 part", total 1part(1block)
#define	PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK		PRODUCT_ID_ONE_PART_TOTAL_BLOCK*PRODUCT_ID_TOTAL_PART



static int snt_dev;

//////////////////////////////////////////////////////////////
struct PDT pdt;
PartitionTask p;
struct F34_Query g_F34Query;
bool rescanF54 = false;
/////////////////////////////////////////////////////////////
PartitionInfo   partition_list[LIST_MAX]; //100 nono..check upper line
int list_count = 0;

static uint8_t Data[TRX_MAX * TRX_MAX * 4];
static int16_t Image1[TRX_MAX][TRX_MAX];
static float ImagepF[TRX_MAX][TRX_MAX];
static float ImageFrame[TRX_MAX][TRX_MAX];
uint8_t TRX_Short[TRX_BITMAP_LENGTH] = {0,};
static float RXROE[TRX_MAX];
static float TXROE[TRX_MAX];
static uint8_t GpioOpenData[1];
///////////////////////////////////////////////////////////
double NoiseDeltaMin[TRX_MAX][TRX_MAX];
double NoiseDeltaMax[TRX_MAX][TRX_MAX];

/* Side Touch test */
float SideTouchRawCapMaxLimit[3][TRX_MAX];
float SideTouchRawCapMinLimit[3][TRX_MAX];
int SideTouchNoiseMaxLimit;
int SideTouchNoiseMinLimit;

static uint8_t err_array[TRX_BITMAP_LENGTH];
static uint8_t ExtendRT26_pin[4];

//static const int32_t DEFAULT_TIMEOUT = 2000; // In counts
static const int32_t DEFAULT_TIMEOUT = 300; // In counts
static bool bRepate_times = false;
static bool result_rt26 = true; // for Extended TRX short test //for case eRT_ExtendedTRexShort

struct synaptics_touch_limit l_limit;
int32_t *p32data_ReadeHybridRawCap;
int HybridTXShort();
int ExtendHybridTXShort2();
int ReadGpioOpenReport();
//////////////////////////////////////////

int synaptics_touch_limit_parser(MODEL id, char *m_name, struct synaptics_touch_limit* limit)
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
///////////////////////////////////////////// I don't Know, 10(decimal number) or 16(hexadecimal)
            if(!strcmp(token, "CONFIG"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->config = strtoul(token,NULL,16);
				if(DEBUG_MODE)
					printf("[CONFIG] Configuration TEST = 0x%lX\n",limit->config);
            }
            if(!strcmp(token, "FW_VERSION"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->fw_version = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[FW_VERSION] F/W version TEST = 0x%lX\n",limit->fw_version);
            }
            if(!strcmp(token, "NOISE_TEST"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->noise_test_min = (int)strtol(token,NULL,10);
                token = strtok(NULL, TOKEN_SEP);
                limit->noise_test_max = (int)strtol(token,NULL,10);
				if(DEBUG_MODE)
				{
					printf("[NOISE_TEST] Noise TEST MIN = %d\n",limit->noise_test_min);
					printf("[NOISE_TEST] Noise TEST MAX = %d\n",limit->noise_test_max);
				}
            }
#if	0	/* swchoi - comment as touch table will be used */
            if(!strcmp(token, "HYBRID_RAW_CAP_TX"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_tx_min = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_TX] Hybrid Raw Cap TX MIN= %ld\n",limit->hybrid_raw_cap_tx_min);
				
                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_tx_max = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_TX] Hybrid Raw Cap TX MAX= %ld\n",limit->hybrid_raw_cap_tx_max);

            }
            if(!strcmp(token, "HYBRID_RAW_CAP_RX_00"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_rx_00_min = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_RX_00] Hybrid Raw Cap RX00 MIN = %ld\n",limit->hybrid_raw_cap_rx_00_min);

                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_rx_00_max = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_RX_00] Hybrid Raw Cap RX00 MAX = %ld\n",limit->hybrid_raw_cap_rx_00_max);

            }
            if(!strcmp(token, "HYBRID_RAW_CAP_RX_OTHER"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_rx_other_min = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_RX_OTHER] Hybrid Raw Cap RX01~33 MIN = %ld\n",limit->hybrid_raw_cap_rx_other_min);

                token = strtok(NULL, TOKEN_SEP);
                limit->hybrid_raw_cap_rx_other_max = strtoul(token,NULL,10);
				if(DEBUG_MODE)
					printf("[HYBRID_RAW_CAP_RX_OTHER] Hybrid Raw Cap RX01~33 MAX = %ld\n",limit->hybrid_raw_cap_rx_other_max);

            }
#endif	/* swchoi - end */
/*
            if(!strcmp(token, "EXTENDED_HIGH_REGI"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->extended_high_regi_min = strtof(token,NULL);
                token = strtok(NULL, TOKEN_SEP);
                limit->extended_high_regi_max = strtof(token,NULL);
				if(DEBUG_MODE)
				{
					printf("[EXTENDED_HIGH_REGI] Extended High Registance MIN= %f\n",limit->extended_high_regi_min);
					printf("[EXTENDED_HIGH_REGI] Extended High Registance MAX= %f\n",limit->extended_high_regi_max);
				}
            }
*/
			/* side touch test */
            if(!strcmp(token, "SIDE_TOUCH_RAW_CAP"))
            {
				int cnt = 0;
				int trx_cnt = 0;
                token = strtok(NULL, TOKEN_SEP);
				for (cnt = 0;cnt < 3;cnt++)
				{
					for (trx_cnt = 0;trx_cnt < TRX_MAX;trx_cnt++)
					{
                		limit->side_touch_raw_cap_min[cnt][trx_cnt] = strtof(token,NULL);
						if(DEBUG_MODE)
							printf("[SIDE_TOUCH_RAW_CAP] Side Touch Raw Cap MIN = %f\n",limit->side_touch_raw_cap_min[cnt][trx_cnt]);
					}
				}

                token = strtok(NULL, TOKEN_SEP);
				for (cnt = 0;cnt < 3;cnt++)
				{
					for (trx_cnt = 0;trx_cnt < TRX_MAX;trx_cnt++)
					{
		                limit->side_touch_raw_cap_max[cnt][trx_cnt] = strtof(token,NULL);
						if(DEBUG_MODE)
							printf("[SIDE_TOUCH_RAW_CAP] Side Touch Raw Cap MAX = %f\n",limit->side_touch_raw_cap_max[cnt][trx_cnt]);
					}
				}
            }
            if(!strcmp(token, "SIDE_TOUCH_NOISE"))
            {
                token = strtok(NULL, TOKEN_SEP);
                limit->side_touch_noise_min = strtol(token,NULL,10);
				if(DEBUG_MODE)
					printf("[SIDE_TOUCH_NOISE] SIDE TOUCH NOISE MIN = %d\n",limit->side_touch_noise_min);

                token = strtok(NULL, TOKEN_SEP);
                limit->side_touch_noise_max = strtol(token,NULL,10);
				if(DEBUG_MODE)
					printf("[SIDE_TOUCH_NOISE] SIDE TOUCH NOISE MAX = %d\n",limit->side_touch_noise_max);
            }
            if(!strcmp(token, "DEVICE_PACKAGE"))
            {
                token = strtok(NULL, TOKEN_SEP"S");
                limit->device_package = strtoul(token,NULL,10);
                if(DEBUG_MODE)
                    printf("[DEVICE_PACKAGE] Device Package = S%ld\n",limit->device_package);
            }
            if(!strcmp(token, "PROJECT_ID"))
            {
                token = strtok(NULL, TOKEN_SEP);
                printf("read : %s",token);
                sprintf(limit->project_id,"%s",token);
                limit->project_id_len = strlen(limit->project_id);
                if(DEBUG_MODE)
                    printf("[PROJECT_ID] Project ID = %s [len:%d]\n",limit->project_id,limit->project_id_len);

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

int synaptics_touch_limit_table_parser(MODEL id, char *m_name, struct synaptics_touch_limit* limit)
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

    unsigned int st_full_raw_capacitance_max = 0;
    unsigned int st_full_raw_capacitance_min = 0;
    unsigned int st_extended_high_regi_max = 0;
    unsigned int st_extended_high_regi_min = 0;
    unsigned int st_hybrid_raw_cap_rx_max = 0;
    unsigned int st_hybrid_raw_cap_rx_min = 0;
    unsigned int st_hybrid_raw_cap_tx_max = 0;
    unsigned int st_hybrid_raw_cap_tx_min = 0;
	

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

				if(st_full_raw_capacitance_max)
				{
					limit->full_raw_cap_MAX[0][0] = 1;
				    if(!limit->full_raw_cap_MAX[0][1] || !limit->full_raw_cap_MAX[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=34; y=17;
						}
				        limit->full_raw_cap_MAX[0][1] = x;
				        limit->full_raw_cap_MAX[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->full_raw_cap_MAX[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->full_raw_cap_MAX[0][2])) //2 :y
				                    {
				                        st_full_raw_capacitance_max = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("full_raw_cap_MAX\n");
	                                        printf("X=%d Y=%d \n",(int)limit->full_raw_cap_MAX[0][1],(int)limit->full_raw_cap_MAX[0][2]);
	                                        for(i = 1; i<(int)limit->full_raw_cap_MAX[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->full_raw_cap_MAX[0][1]+1 ;j++)
	                                            {
	                                                printf(" %0.4f",limit->full_raw_cap_MAX[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->full_raw_cap_MAX[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

	            if(st_full_raw_capacitance_min)
	            {
					limit->full_raw_cap_MIN[0][0] = 1;
	                if(!limit->full_raw_cap_MIN[0][1] || !limit->full_raw_cap_MIN[0][2])
	                {
	                    if((x == 0) || (y == 0))
						{
	                        x=34; y=17;
						}
	                    limit->full_raw_cap_MIN[0][1] = x;
	                    limit->full_raw_cap_MIN[0][2] = y;
	                }
	                else
	                {
	                    if(!strcmp(token,"S"))
	                    {
	                        row++;
	                        int n = 1;
	                        for(n = 1; n < (int)limit->full_raw_cap_MIN[0][1]+3; n++) //1 : x
			                {
		                        token = strtok(NULL, TOKEN_SEP_COMMA);
			                    if(!strcmp(token,"E"))
			                    {
			                        if((row == limit->full_raw_cap_MIN[0][2])) //2 :y
			                        {
		                                st_full_raw_capacitance_min = 0;
		                                row = 0;

		                                if(DEBUG_MODE)
		                                {
		                                    printf("full_raw_cap_MIN\n");
		                                    printf("X=%d Y=%d \n",(int)limit->full_raw_cap_MIN[0][1],(int)limit->full_raw_cap_MIN[0][2]);
		                                    for(i = 1; i<limit->full_raw_cap_MIN[0][2]+1 ;i++)
		                                    {
		                                        for(j = 1; j<limit->full_raw_cap_MIN[0][1]+1 ;j++)
		                                        {
	                                                printf(" %0.4f",limit->full_raw_cap_MIN[i][j]);
		                                        }
		                                        printf("\n");
		                                    }
		                                }
		                            }
		                            break;
		                        }
		                        limit->full_raw_cap_MIN[row][n] = strtof(token,NULL);
		                    }
		                }
		            }
		        }

				if(st_extended_high_regi_max)
				{
					limit->extended_high_regi_MAX[0][0] = 1;
				    if(!limit->extended_high_regi_MAX[0][1] || !limit->extended_high_regi_MAX[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=34; y=17;
						}
				        limit->extended_high_regi_MAX[0][1] = x;
				        limit->extended_high_regi_MAX[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->extended_high_regi_MAX[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->extended_high_regi_MAX[0][2])) //2 :y
				                    {
				                        st_extended_high_regi_max = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_extended_high_regi_MAX\n");
	                                        printf("X=%d Y=%d \n",(int)limit->extended_high_regi_MAX[0][1],(int)limit->extended_high_regi_MAX[0][2]);
	                                        for(i = 1; i<(int)limit->extended_high_regi_MAX[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->extended_high_regi_MAX[0][1]+1 ;j++)
	                                            {
	                                                printf(" %0.4f",limit->extended_high_regi_MAX[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->extended_high_regi_MAX[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(st_extended_high_regi_min)
				{
					limit->extended_high_regi_MIN[0][0] = 1;
				    if(!limit->extended_high_regi_MIN[0][1] || !limit->extended_high_regi_MIN[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=34; y=17;
						}
				        limit->extended_high_regi_MIN[0][1] = x;
				        limit->extended_high_regi_MIN[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->extended_high_regi_MIN[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->extended_high_regi_MIN[0][2])) //2 :y
				                    {
				                        st_extended_high_regi_min = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_extended_high_regi_MIN\n");
	                                        printf("X=%d Y=%d \n",(int)limit->extended_high_regi_MIN[0][1],(int)limit->extended_high_regi_MIN[0][2]);
	                                        for(i = 1; i<(int)limit->extended_high_regi_MIN[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->extended_high_regi_MIN[0][1]+1 ;j++)
	                                            {
	                                                printf(" %0.4f",limit->extended_high_regi_MIN[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->extended_high_regi_MIN[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(st_hybrid_raw_cap_rx_min)
				{
					limit->hybrid_raw_cap_rx_MIN[0][0] = 1;
				    if(!limit->hybrid_raw_cap_rx_MIN[0][1] || !limit->hybrid_raw_cap_rx_MIN[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=34; y=17;
						}
				        limit->hybrid_raw_cap_rx_MIN[0][1] = x;
				        limit->hybrid_raw_cap_rx_MIN[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->hybrid_raw_cap_rx_MIN[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->hybrid_raw_cap_rx_MIN[0][2])) //2 :y
				                    {
				                        st_hybrid_raw_cap_rx_min = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_hybrid_raw_cap_rx_min\n");
	                                        printf("X=%d Y=%d \n",(int)limit->hybrid_raw_cap_rx_MIN[0][1],(int)limit->hybrid_raw_cap_rx_MIN[0][2]);
	                                        for(i = 1; i<(int)limit->hybrid_raw_cap_rx_MIN[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->hybrid_raw_cap_rx_MIN[0][1]+1 ;j++)
	                                            {
	                                                printf(" %ld",limit->hybrid_raw_cap_rx_MIN[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->hybrid_raw_cap_rx_MIN[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(st_hybrid_raw_cap_rx_max)
				{
					limit->hybrid_raw_cap_rx_MAX[0][0] = 1;
				    if(!limit->hybrid_raw_cap_rx_MAX[0][1] || !limit->hybrid_raw_cap_rx_MAX[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=34; y=17;
						}
				        limit->hybrid_raw_cap_rx_MAX[0][1] = x;
				        limit->hybrid_raw_cap_rx_MAX[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->hybrid_raw_cap_rx_MAX[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->hybrid_raw_cap_rx_MAX[0][2])) //2 :y
				                    {
				                        st_hybrid_raw_cap_rx_max = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_hybrid_raw_cap_rx_max\n");
	                                        printf("X=%d Y=%d \n",(int)limit->hybrid_raw_cap_rx_MAX[0][1],(int)limit->hybrid_raw_cap_rx_MAX[0][2]);
	                                        for(i = 1; i<(int)limit->hybrid_raw_cap_rx_MAX[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->hybrid_raw_cap_rx_MAX[0][1]+1 ;j++)
	                                            {
	                                                printf(" %ld",limit->hybrid_raw_cap_rx_MAX[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->hybrid_raw_cap_rx_MAX[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(st_hybrid_raw_cap_tx_min)
				{
					limit->hybrid_raw_cap_tx_MIN[0][0] = 1;
				    if(!limit->hybrid_raw_cap_tx_MIN[0][1] || !limit->hybrid_raw_cap_tx_MIN[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=17; y=17;
						}
				        limit->hybrid_raw_cap_tx_MIN[0][1] = x;
				        limit->hybrid_raw_cap_tx_MIN[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->hybrid_raw_cap_tx_MIN[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->hybrid_raw_cap_tx_MIN[0][2])) //2 :y
				                    {
				                        st_hybrid_raw_cap_tx_min = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_hybrid_raw_cap_tx_min\n");
	                                        printf("X=%d Y=%d \n",(int)limit->hybrid_raw_cap_tx_MIN[0][1],(int)limit->hybrid_raw_cap_tx_MIN[0][2]);
	                                        for(i = 1; i<(int)limit->hybrid_raw_cap_tx_MIN[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->hybrid_raw_cap_tx_MIN[0][1]+1 ;j++)
	                                            {
	                                                printf(" %ld",limit->hybrid_raw_cap_tx_MIN[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->hybrid_raw_cap_tx_MIN[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(st_hybrid_raw_cap_tx_max)
				{
					limit->hybrid_raw_cap_tx_MAX[0][0] = 1;
				    if(!limit->hybrid_raw_cap_tx_MAX[0][1] || !limit->hybrid_raw_cap_tx_MAX[0][2])
				    {
				        if((x == 0) || (y == 0))
						{
				            x=17; y=17;
						}
				        limit->hybrid_raw_cap_tx_MAX[0][1] = x;
				        limit->hybrid_raw_cap_tx_MAX[0][2] = y;
				    }
				    else
				    {
				        if(!strcmp(token,"S"))
				        {
				            row++;
				            int n = 1;
				            for(n = 1; n < limit->hybrid_raw_cap_tx_MAX[0][1]+3; n++) //1 : x
				            {
				                token = strtok(NULL, TOKEN_SEP_COMMA);
				                if(!strcmp(token,"E"))
				                {
				                    if((row == limit->hybrid_raw_cap_tx_MAX[0][2])) //2 :y
				                    {
				                        st_hybrid_raw_cap_tx_max = 0;
				                        row = 0;
	
	                                    if(DEBUG_MODE)
	                                    {
	                                        printf("st_hybrid_raw_cap_tx_max\n");
	                                        printf("X=%d Y=%d \n",(int)limit->hybrid_raw_cap_tx_MAX[0][1],(int)limit->hybrid_raw_cap_tx_MAX[0][2]);
	                                        for(i = 1; i<(int)limit->hybrid_raw_cap_tx_MAX[0][2]+1 ;i++)
	                                        {
	                                            for(j = 1; j<(int)limit->hybrid_raw_cap_tx_MAX[0][1]+1 ;j++)
	                                            {
	                                                printf(" %ld",limit->hybrid_raw_cap_tx_MAX[i][j]);
	                                            }
	                                            printf("\n");
	                                        }
	                                    }
	                                }
	                                break;
	                            }
	                            limit->hybrid_raw_cap_tx_MAX[row][n] = strtof(token,NULL);
	                        }
						}
					}
				}

				if(!strcmp(token, "Full_Raw_Capacitance_MAX"))
	            {
					st_full_raw_capacitance_max = 1;
					st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;
					printf("\nFull_Raw_Capacitance Max\n");
		            row = 0;
		            x = 0;
		            y = 0;
		        }
		        else if(!strcmp(token, "Full_Raw_Capacitance_MIN"))
		        {
		            st_full_raw_capacitance_max = 0;
		            st_full_raw_capacitance_min = 1;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;

		            printf("\nFull_Raw_Capacitance Min\n");
		            row = 0;
		            x = 0;
		            y = 0;
		        }
                else if(!strcmp(token, "Extended_High_Regi_MAX"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 1;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;
                    printf("\nExtended_High_Regi_MAX\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
                else if(!strcmp(token, "Extended_High_Regi_MIN"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 1;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;
                    printf("\nExtended_High_Regi_MIN\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
                else if(!strcmp(token, "Hybrid_Raw_Cap_RX_MAX"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 1;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;
                    printf("\nHybrid_Raw_Cap_RX_MAX\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
                else if(!strcmp(token, "Hybrid_Raw_Cap_RX_MIN"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 1;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 0;
                    printf("\nHybrid_Raw_Cap_RX_MIN\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
                else if(!strcmp(token, "Hybrid_Raw_Cap_TX_MAX"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 1;
                    st_hybrid_raw_cap_tx_min = 0;
                    printf("\nHybrid_Raw_Cap_TX_MAX\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
                else if(!strcmp(token, "Hybrid_Raw_Cap_TX_MIN"))
                {
                    st_full_raw_capacitance_max = 0;
                    st_full_raw_capacitance_min = 0;
                    st_extended_high_regi_max = 0;
                    st_extended_high_regi_min = 0;
                    st_hybrid_raw_cap_rx_max = 0;
                    st_hybrid_raw_cap_rx_min = 0;
                    st_hybrid_raw_cap_tx_max = 0;
                    st_hybrid_raw_cap_tx_min = 1;
                    printf("\nHybrid_Raw_Cap_TX_MIN\n");
                    row = 0;
                    x = 0;
                    y = 0;
                }
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

void init_limit_data(struct synaptics_touch_limit *limit)
{

	int i,j = 0;
    FUNC_BEGIN();

	l_limit.id = limit->id;
	l_limit.fw_version = limit->fw_version;
	l_limit.config = limit->config;
	l_limit.noise_test_min = limit->noise_test_min;
	l_limit.noise_test_max = limit->noise_test_max;
#if	0	/* swchoi - comment as touch table will be used */
	l_limit.hybrid_raw_cap_tx_min = limit->hybrid_raw_cap_tx_min;
	l_limit.hybrid_raw_cap_tx_max = limit->hybrid_raw_cap_tx_max;
	l_limit.hybrid_raw_cap_rx_00_min = limit->hybrid_raw_cap_rx_00_min;
	l_limit.hybrid_raw_cap_rx_00_max = limit->hybrid_raw_cap_rx_00_max;
	l_limit.hybrid_raw_cap_rx_other_min = limit->hybrid_raw_cap_rx_other_min;
	l_limit.hybrid_raw_cap_rx_other_max = limit->hybrid_raw_cap_rx_other_max;
#endif	/* swchoi - end */
/*
	l_limit.extended_high_regi_min = limit->extended_high_regi_min;
	l_limit.extended_high_regi_max = limit->extended_high_regi_max;
*/
	l_limit.device_package = limit->device_package; // 180516
	sprintf(l_limit.project_id,"%s", limit->project_id); // 180516
    l_limit.project_id_len = limit->project_id_len; // 180516

	/* Full Raw Cap */
	memcpy(l_limit.full_raw_cap_MAX,limit->full_raw_cap_MAX,300*300);
	memcpy(l_limit.full_raw_cap_MIN,limit->full_raw_cap_MIN,300*300);
	/* Extended High Regi */
	memcpy(l_limit.extended_high_regi_MIN,limit->extended_high_regi_MIN,300*300);
	memcpy(l_limit.extended_high_regi_MAX,limit->extended_high_regi_MAX,300*300);
	/* Hybrid Raw Cap RX/TX */
	memcpy(l_limit.hybrid_raw_cap_rx_MIN,limit->hybrid_raw_cap_rx_MIN,300*300);
	memcpy(l_limit.hybrid_raw_cap_rx_MAX,limit->hybrid_raw_cap_rx_MAX,300*300);
	memcpy(l_limit.hybrid_raw_cap_tx_MIN,limit->hybrid_raw_cap_tx_MIN,300*300);
	memcpy(l_limit.hybrid_raw_cap_tx_MAX,limit->hybrid_raw_cap_tx_MAX,300*300);

	/* side touch test */
	memcpy(l_limit.side_touch_raw_cap_max,limit->side_touch_raw_cap_max,sizeof(l_limit.side_touch_raw_cap_max));
	memcpy(l_limit.side_touch_raw_cap_min,limit->side_touch_raw_cap_min,sizeof(l_limit.side_touch_raw_cap_min));
	l_limit.side_touch_noise_max = limit->side_touch_noise_max;
	l_limit.side_touch_noise_min = limit->side_touch_noise_min;

	if(DEBUG_MODE)
	{
		printf("==========LIMIT========== \n");
		printf("> ID : %d \n",l_limit.id);
		printf("> FW_VERSION : %ld \n",l_limit.fw_version);
		printf("> CONFIG : %ld \n",l_limit.config);
		printf("> NOISE_TEST MIN: %d \n",l_limit.noise_test_min);
		printf("> NOISE_TEST MAX: %d \n",l_limit.noise_test_max);
#if	0	/* swchoi - comment as touch table will be used */
		printf("> HYBRID RAW CAP TX MIN: %ld \n",l_limit.hybrid_raw_cap_tx_min);
		printf("> HYBRID RAW CAP TX MAX: %ld \n",l_limit.hybrid_raw_cap_tx_max);
		printf("> HYBRID RAW CAP RX 00 MIN: %ld \n",l_limit.hybrid_raw_cap_rx_00_min);
		printf("> HYBRID RAW CAP RX 00 MAX: %ld \n",l_limit.hybrid_raw_cap_rx_00_max);
		printf("> HYBRID RAW CAP RX OTHER MIN: %ld \n",l_limit.hybrid_raw_cap_rx_other_min);
		printf("> HYBRID RAW CAP RX OTHER MAX: %ld \n",l_limit.hybrid_raw_cap_rx_other_max);
#endif	/* swchoi - end */
		//printf("> EXTENDED HIGH REGI MIN: %f \n",l_limit.extended_high_regi_min);
		//printf("> EXTENDED HIGH REGI MAX: %f \n",l_limit.extended_high_regi_max);
		printf("> SIDE TOUCH RAW CAP MIN: %f \n",l_limit.side_touch_raw_cap_min[0][0]);
		printf("> SIDE TOUCH RAW CAP MAX: %f \n",l_limit.side_touch_raw_cap_max[0][0]);
		printf("> SIDE TOUCH NOISE MIN: %d \n",l_limit.side_touch_noise_min);
		printf("> SIDE TOUCH NOISE MAX: %d \n",l_limit.side_touch_noise_max);
		printf("> DEVICE PACKAGE : %ld \n",l_limit.device_package);
        printf("> PROJECT ID : %s [len:%d]\n",l_limit.project_id,l_limit.project_id_len);

		printf("-------------------\n");
		printf("> FULL RAW CAP MAX [X: %d / Y : %d]\n",(int)l_limit.full_raw_cap_MAX[0][1],(int)l_limit.full_raw_cap_MAX[0][2]);
		printf("-------------------\n");
		for( i = 0; i < (int)l_limit.full_raw_cap_MAX[0][2] ; i++) //y
		{
			for(j = 0; j < (int)l_limit.full_raw_cap_MAX[0][1] ; j++) //x
			{
				if(j == 0)
					printf("[%d] | ",i+1);
				printf("%0.4f, ",l_limit.full_raw_cap_MAX[i+1][j+1]);
			}
			printf("\n");		
		}

		printf("-------------------\n");
		printf("> FULL RAW CAP MIN [X: %d / Y : %d]\n",(int)l_limit.full_raw_cap_MIN[0][1],(int)l_limit.full_raw_cap_MIN[0][2]);
		printf("-------------------\n");
		for( i = 0; i < (int)l_limit.full_raw_cap_MIN[0][2]  ;i++) //y
		{
		    for(j = 0; j < (int)l_limit.full_raw_cap_MIN[0][1]  ;j++) //x
		    {
		        if(j == 0)
		            printf("[%d] | ",i+1);
		        printf("%0.4f, ",l_limit.full_raw_cap_MIN[i+1][j+1]);
	
	        }
			printf("\n");		 
	    }

        printf("-------------------\n");
        printf("> EXTENDED HIGH REGISTANCE MAX [X: %d / Y : %d]\n",(int)l_limit.extended_high_regi_MAX[0][1],(int)l_limit.extended_high_regi_MAX[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.extended_high_regi_MAX[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.extended_high_regi_MAX[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%0.4f, ",l_limit.extended_high_regi_MAX[i+1][j+1]);
            }
            printf("\n");
        }

        printf("-------------------\n");
        printf("> EXTENDED HIGH REGISTANCE MIN [X: %d / Y : %d]\n",(int)l_limit.extended_high_regi_MIN[0][1],(int)l_limit.extended_high_regi_MIN[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.extended_high_regi_MIN[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.extended_high_regi_MIN[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%0.4f, ",l_limit.extended_high_regi_MIN[i+1][j+1]);
            }
            printf("\n");
        }

        printf("-------------------\n");
        printf("> HYBRID RAW CAP RX MIN [X: %d / Y : %d]\n",(int)l_limit.hybrid_raw_cap_rx_MIN[0][1],(int)l_limit.hybrid_raw_cap_rx_MIN[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.hybrid_raw_cap_rx_MIN[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.hybrid_raw_cap_rx_MIN[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%ld, ",l_limit.hybrid_raw_cap_rx_MIN[i+1][j+1]);
            }
            printf("\n");
        }

        printf("-------------------\n");
        printf("> HYBRID RAW CAP RX MAX [X: %d / Y : %d]\n",(int)l_limit.hybrid_raw_cap_rx_MAX[0][1],(int)l_limit.hybrid_raw_cap_rx_MAX[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.hybrid_raw_cap_rx_MAX[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.hybrid_raw_cap_rx_MAX[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%ld, ",l_limit.hybrid_raw_cap_rx_MAX[i+1][j+1]);
            }
            printf("\n");
        }

        printf("-------------------\n");
        printf("> HYBRID RAW CAP TX MIN [X: %d / Y : %d]\n",(int)l_limit.hybrid_raw_cap_tx_MIN[0][1],(int)l_limit.hybrid_raw_cap_tx_MIN[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.hybrid_raw_cap_tx_MIN[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.hybrid_raw_cap_tx_MIN[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%ld, ",l_limit.hybrid_raw_cap_tx_MIN[i+1][j+1]);
            }
            printf("\n");
        }

        printf("-------------------\n");
        printf("> HYBRID RAW CAP TX MAX [X: %d / Y : %d]\n",(int)l_limit.hybrid_raw_cap_tx_MAX[0][1],(int)l_limit.hybrid_raw_cap_tx_MAX[0][2]);
        printf("-------------------\n");
        for( i = 0; i < (int)l_limit.hybrid_raw_cap_tx_MAX[0][2] ; i++) //y
        {
            for(j = 0; j < (int)l_limit.hybrid_raw_cap_tx_MAX[0][1] ; j++) //x
            {
                if(j == 0)
                    printf("[%d] | ",i+1);
                printf("%ld, ",l_limit.hybrid_raw_cap_tx_MAX[i+1][j+1]);
            }
            printf("\n");
        }

		printf("-------------------\n");
		printf("========================= \n");
	}

    FUNC_END();

}

/////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// I2C Functions
//////////////////////////////////////////////////////////////////////////////
struct i2c_data{
    unsigned char addr;
    unsigned short reg;
    unsigned char *data;
    int len;
};

struct i2c_read_data{
    unsigned char addr;
    unsigned short reg;
    unsigned char data[4096];
};

int i2c_general_read_2BAddr(unsigned short addr, unsigned char *buf, int size)
{

    int rc;
    unsigned char temp_buf[2];
    temp_buf[0] = addr & 0xFF;
    temp_buf[1] = (addr >> 8) & 0xFF;

    FUNC_BEGIN();

    rc = write(snt_dev, temp_buf ,2);
    usleep(500);
    rc = read(snt_dev,buf,size);

	if(rc < 0)
	{
		FUNC_END();
		return	false;
	}

    FUNC_END();
    return true;

}

int i2c_general_write_2BAddr(unsigned short addr, unsigned char *data, int len)
{
    int ret;
    int i;
    unsigned char i2c_send_buf[255];

    FUNC_BEGIN();
    i2c_send_buf[0] = addr & 0xFF;
    i2c_send_buf[1] = (addr >> 8) & 0xFF;

    for(i=2; i<len+2; i++)
        i2c_send_buf[i] = data[i-2];

    ret = write(snt_dev, i2c_send_buf, len+2);

    memset(i2c_send_buf,0,len+2);

    if(ret < 0)
    {
        FUNC_END();
        return false;
    }

    FUNC_END();
    return true;
}

int i2c_general_read_1BAddr(unsigned char addr, unsigned char *buf, int size)
{

    int rc;
    unsigned char temp_buf[2];
    //int i = 0;

    temp_buf[0] = addr & 0xFF;

    FUNC_BEGIN();

    rc = write(snt_dev, temp_buf ,1);
    usleep(500);
    rc = read(snt_dev,buf,size);
#if	0	/* debug print */
    printf("\n - Read(%d) [ADDR:0x%X] [SIZE:%d] - ",rc,addr,size);
    for(i = 0; i < size; i++)
    {
        if(!(i % 5))
            printf("\n");
        printf("[%d:0x%X] ",i,*(buf+i));
    }
    printf("\n");
#endif

    if(rc < 0)
    {
        printf("Read Fail.. \n");
        FUNC_END();
        return false;
    }

    FUNC_END();
    return true;

}

int i2c_general_write_1BAddr(unsigned char addr, unsigned char *data, int len)
{
    int ret;
    int i;
    unsigned char i2c_send_buf[255];

    FUNC_BEGIN();
    i2c_send_buf[0] = addr & 0xFF;

    for(i=1; i<len+1; i++)
        i2c_send_buf[i] = data[i-1];

    ret = write(snt_dev, i2c_send_buf, len+1);

#if	0	/* debug print */
    printf("\n - Write(%d) [ADDR:0x%X] [SIZE:%d] - ",ret,addr,len);
    for(i = 0; i < len; i++)
    {
        if(!(i % 5))
            printf("\n");
        printf("[%d:0x%X] ",i,*(data+i));
    }
    printf("\n");
#endif

    memset(i2c_send_buf,0,len+1);

    if(ret < 0)
    {
        FUNC_END();
        return false;
    }

    FUNC_END();
    return true;

}

int rmi_set_page(unsigned char page)
{
    int rc;

    FUNC_BEGIN();
    rc = i2c_general_write_1BAddr((unsigned char)(RMI_PAGE_SELECT_REGISTER&0xFF), &page, 1);

    if (rc == false)
    {
        printf("%s : set page failed: %d\n",__func__,rc);
        FUNC_END();
        return false;
    }
    FUNC_END();
    return true;
}
/////////////////////////////////////////////////

/*
 * Name : SW_Reset
 * Description : S/W reset to write data to specific address.
 * Parameters :
 * Return value : error
 */
int SW_Reset(void)
{
	uint8_t data = 1;

	FUNC_BEGIN();

#if	1	/* confirmed by Synaptics FAE that Reset should be used for each test, need to check a delay time - 20180516 */
	data = 1;
	if(!rmi_set_page((unsigned char)(((pdt.g_F01Descriptor.CommandBase)>> 8) &0xFF)))
		return  I2C_ERR;
	if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F01Descriptor.CommandBase)&0xFF), &data, 1))
		return  I2C_ERR;

	usleep(50000);	/* 50ms delay - need to check if the value is OK */
#endif

	FUNC_END();

	return true;
}

const char * GetPartitionName(PartitionID id)
{
    FUNC_BEGIN();
    FUNC_END();
    return PartitionName[id];
}
int16_t WaitForATTN(uint16_t interruptStatusAddress, FunctionInterrupt func, uint32_t timeout)
{
    uint8_t status;
    uint32_t duration = 20;
    uint32_t retry = timeout / duration;
    uint32_t times = 0;

    //#ifdef POLLING //modify khl
    FUNC_BEGIN();
  do {
    status = 0x00;
    if(!i2c_general_read_1BAddr((unsigned char)((interruptStatusAddress)&0xFF), &status, 1))
		return	I2C_ERR;
    //if (status)
    //  printf("status = %x,  ", status);
    if ((status & (uint8_t)func) == (uint8_t)func)
      break;
    //Sleep(duration); //need debug khl ... msleep? usleep?
    usleep(duration*1000);
    times++;
  } while (times < retry);

    if (times == retry){
        printf("\nTimed out polling for attention interrupt...\n");
        FUNC_END();
        return false;
    }

/* //maybe that don't need.. modify khl
#else 
    if (Line_WaitForAttention(timeout) == EErrorTimeout){
        printf("\nTimed out waiting for attention interrupt...\n");
        return false;
    }
    //ReadRMI(interruptStatusAddress, &status, 1);
    ReadRMI(interruptStatusAddress, &status, 1);
    //ReadRMI(interruptStatusAddress, &status, 1);
  //if ((status & (uint8_t)func) != (uint8_t)func)
    //return false;
#endif
*/

    FUNC_END();
    return true;
}
//////////////////////////////////

uint8_t BitCount(uint8_t value)
{
    FUNC_BEGIN();
    // Lookup table for fast calculation of bits set in 8-bit unsigned char.
    const static uint8_t BitsTable[] = {
        //  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        //  ==============================================
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
    };
    FUNC_END();
    return BitsTable[value >> 4] + BitsTable[value & 0x0F];
}






void setErr_Array(uint8_t trx, bool err)
{

  uint8_t byte = trx / 8;
  uint8_t bit =  trx % 8;
    FUNC_BEGIN();
  if (err)
    err_array[byte] = (err_array[byte] | (0x01 << bit));
  else
    err_array[byte] = (err_array[byte] - (0x01 << bit));
    FUNC_END();


}


//void GetMaxRx(float value[TRX_MAX][TRX_MAX], uint8_t len, float max[TRX_MAX])
void GetMaxRx(float**value, uint8_t len, float*max)
{
    //uint8_t j, i =0;
    uint16_t j, i =0;
	float tray[TRX_MAX][TRX_MAX];
    FUNC_BEGIN();

	memcpy(tray,value,sizeof(float)*TRX_MAX*TRX_MAX);

  for(j = 0; j < pdt.RxChannelCount; j++)
  {
    for(i = 0; i < pdt.TxChannelCount; i++)
    {
      if (max[j] < tray[i][j])
        max[j] = tray[i][j];
    }
  }
    FUNC_END();
}

//float GetMinRx(float value[TRX_MAX][TRX_MAX], uint8_t rx, bool isbutton)
float GetMinRx(float **value, uint8_t rx, bool isbutton)
{
  float min = 0xffff;
  uint8_t txcount;
    //uint8_t i = 0;
    uint16_t i = 0;
    float tray[TRX_MAX][TRX_MAX];
    FUNC_BEGIN();

    memcpy(tray,value,sizeof(float)*TRX_MAX*TRX_MAX);

  (pdt.ButtonShared == true)? (txcount = pdt.TxChannelCount) : (txcount = pdt._2DTxCount);
  if (isbutton)
  {
    min = tray[pdt.TxChannelCount - 1][rx];
  }
  else
  {
    for(i = 0; i < txcount; i++)
    {
      if (min > tray[i][rx])
        min = tray[i][rx];
    }
  }
    FUNC_END();

  return min;
}


uint8_t GetLogicalPin(uint8_t p_pin)
{
    uint8_t i = 0;
    FUNC_BEGIN();
  for(i = 0; i < pdt.RxChannelCount; i++)
  {
    if (pdt.RxPhysical[i] == p_pin)
	{
		FUNC_END();
		return i;
	}
  }
    FUNC_END();
  return 0xff;
}


uint8_t GetF12CtrlOffset(uint8_t reg, uint8_t *buffer)
{
    uint8_t quo = (reg / 8) + 1;
    uint8_t rem = (reg % 8);
    uint8_t offset = 0, mask = 0;
    int i = 0;

    FUNC_BEGIN();
    for (i = 0; i < rem; i++)
        mask += (0x01 << i);

    buffer++;
    for (i = 1; i <= quo; i++, buffer++)
    {
        uint8_t _mask = (i == quo) ? mask : 0xff;
        _mask &= *buffer;
        offset += BitCount(_mask);
    }
    FUNC_END();
    return offset;
}

int16_t GetF12CtrlSubpacketSize(uint8_t offset)
{
    uint8_t sizeofQuery6, _offset = 0;
    uint8_t size = 0;
    FUNC_BEGIN();

    if(!rmi_set_page((unsigned char)(((pdt.g_F12Descriptor.QueryBase)>> 8) &0xFF)))
		return	I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F12Descriptor.QueryBase + 5)&0xFF), &sizeofQuery6, 1))
		return	I2C_ERR;

    uint8_t *buffer = (uint8_t*)calloc(sizeofQuery6, sizeof(uint8_t));

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F12Descriptor.QueryBase + 6)&0xFF), &buffer[0], sizeofQuery6))
		return	I2C_ERR;
    uint8_t i = 0;

    for (i = 0; i < sizeofQuery6; i++)
    {
        size = buffer[i];
        if (_offset == offset)
        {
            size = buffer[i];
            break;
        }
        else
        {
            do{
                i++;
            } while (buffer[i] & 0x80);
            _offset++;
        }
    }

    free(buffer);
    FUNC_END();
    return (int16_t)size;
}

///////////////////////////////
int	ReadF01Information()
{
    uint8_t buffer[20];
    FUNC_BEGIN();

    if(!rmi_set_page((unsigned char)(((pdt.g_F01Descriptor.QueryBase)>> 8) &0xFF)))
		return I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F01Descriptor.QueryBase + 11)&0xFF), &buffer[0], 10))
		return I2C_ERR;
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F01Descriptor.QueryBase + 17)&0xFF), &buffer[0], 4))
		return I2C_ERR;

    pdt.F01_interrupt = pdt.g_F01Descriptor.DataBase + 1;
    //#ifndef POLLING
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase + 1)&0xFF),&buffer[0],1))
		return I2C_ERR;

    buffer[0] |= 0x08;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase + 1)&0xFF),&buffer[0],1))
		return I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.F01_interrupt)&0xFF),&buffer[0],1))
		return I2C_ERR;
	//#endif
    FUNC_END();
	return	true;
}
int ReadF12Information()
{
    // F$12 information
    FUNC_BEGIN();
    if (pdt.g_F12Descriptor.ID == 0x12)
    {
        uint8_t buffer[16];
        uint8_t CtrlPresence = 0;
        if(!rmi_set_page((unsigned char)(((pdt.g_F12Descriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F12Descriptor.QueryBase + 5)&0xFF),&buffer[0], 2))
			return I2C_ERR;
        uint8_t ctrl8Offset = BitCount(buffer[1]);
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F12Descriptor.ControlBase + ctrl8Offset)&0xFF),&buffer[0], 14))
			return I2C_ERR;

        pdt._2DRxCount = buffer[12];
        pdt._2DTxCount = buffer[13];
        memset(buffer, 0, sizeof(buffer[0]));

        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F12Descriptor.QueryBase + 4)&0xFF),&CtrlPresence,1))
			return I2C_ERR;
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F12Descriptor.QueryBase + 5)&0xFF),&buffer[0],CtrlPresence))
			return I2C_ERR;

        pdt.bHaveF12Ctrl20 = (buffer[3] && 0x40);
        pdt.bHaveF12Ctrl27 = (buffer[4] && 0x40);

        if (pdt.bHaveF12Ctrl20)
        {
            pdt.F12Ctrl20Offset = GetF12CtrlOffset(20, buffer);
            if((pdt.F12Ctrl20Size = (uint8_t)GetF12CtrlSubpacketSize(pdt.F12Ctrl20Offset)) <0)
				return I2C_ERR;
        }
        if (pdt.bHaveF12Ctrl27)
        {
            pdt.F12Ctrl27Offset = GetF12CtrlOffset(27, buffer);
            if((pdt.F12Ctrl27Size = (uint8_t)GetF12CtrlSubpacketSize(pdt.F12Ctrl27Offset)) < 0)
				return	I2C_ERR;
        }


    }
    FUNC_END();
    return  true;

}

int ReadF1AInformation()
{
    int32_t i = 0;
    // F$1A information
    FUNC_BEGIN();
    if (pdt.g_F1ADescriptor.ID == 0x1A)
    {
        if(!rmi_set_page((unsigned char)(((pdt.g_F1ADescriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;

        uint8_t buffer[1];
        int32_t k = 0;

        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F1ADescriptor.ControlBase + 1)&0xFF),&buffer[0],1))
			return I2C_ERR;

        pdt.ButtonCount = BitCount(buffer[0]);

        for ( i = 0; i < pdt.ButtonCount; i++){

            if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F1ADescriptor.ControlBase + 3 + k)&0xFF),&pdt.ButtonTx[i],1))
				return I2C_ERR;
            if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F1ADescriptor.ControlBase + 3 + k + 1)&0xFF),&pdt.ButtonRx[i],1))
				return I2C_ERR;
            k = k + 2;
        }
    }
    FUNC_END();
    return  true;

}

int ReadF21Information()
{
    uint8_t i = 0;
    FUNC_BEGIN();
  if (pdt.g_F21Descriptor.ID == 0x21)
  {
    if(!rmi_set_page((unsigned char)(((pdt.g_F21Descriptor.QueryBase)>> 8) &0xFF)))
		return I2C_ERR;
    uint8_t buffer[16];
    uint8_t offset = 0, force_cnt = 0;
    bool isTx = true;
    //Query
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F21Descriptor.QueryBase + 2)&0xFF),&buffer[0], 3))
		return I2C_ERR;
    uint8_t q11Offset = BitCount(buffer[1]) + (BitCount(buffer[2] & 0x0f)) - 1;

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F21Descriptor.QueryBase + q11Offset)&0xFF),&buffer[0], 5))
		return I2C_ERR;

    pdt.ForceTRxMAXCount = buffer[2];
    isTx = (buffer[3] > 0) ? true : false;

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F21Descriptor.QueryBase + 5)&0xFF),&buffer[0], 4))
		return I2C_ERR;


    offset = BitCount(buffer[1] & 0x1f);
    if (offset > 0) offset--;
    pdt.F21Ctrl04Offset = offset;

    offset = BitCount(buffer[1]) + BitCount(buffer[2]) + BitCount(buffer[3]);
    if (offset > 0) offset--;
    pdt.F21Ctrl23Offset = offset;



    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F21Descriptor.ControlBase + pdt.F21Ctrl04Offset)&0xFF),&buffer[0], pdt.ForceTRxMAXCount))
		return I2C_ERR;

      if (isTx)
        memcpy(&pdt.ForceTx[0], &buffer[0], pdt.ForceTRxMAXCount);
      else
        memcpy(&pdt.ForceRx[0], &buffer[0], pdt.ForceTRxMAXCount);

    for (i = 0; i < pdt.ForceTRxMAXCount; i++)
    {
      if (buffer[i] != 0xff)
      {
        force_cnt++;
      }
    }
    pdt.ForcePhysicalCount = force_cnt;

  }
    FUNC_END();
    return  true;

}

int ReadF34Information()
{
    uint8_t offset = 0;
    uint8_t data[22];
    FUNC_BEGIN();
    if (pdt.g_F34Descriptor.ID == 0x34){
        if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;
        (pdt.g_F34Descriptor.Version >= 0x02) ? (pdt.bBoodloaderVersion6 = true) : (pdt.bBoodloaderVersion6 = false);
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F34Descriptor.QueryBase + 1)&0xFF),&data[0], 22))
			return I2C_ERR;

        g_F34Query.FlashKey[0] = data[offset]; offset++; //query1
        g_F34Query.FlashKey[1] = data[offset]; offset++;
        g_F34Query.BootloaderFWID = (data[offset + 3] << 24) | (data[offset + 2] << 16) |
            (data[offset + 1] << 8) | data[offset]; offset = offset + 4;//query 2
        g_F34Query.MWS = data[offset]; offset++;//query3
        g_F34Query.BlockSize = ((data[offset + 1] << 8) | data[offset]);  offset = offset + 2;
        g_F34Query.FlashPageSize = (data[offset + 1] << 8) | data[offset]; offset = offset + 2;
        g_F34Query.AdjPartitionSize = (data[offset + 1] << 8) | data[offset]; offset = offset + 2;//query 4
        g_F34Query.FlashConfLen = (data[offset + 1] << 8) | data[offset]; offset = offset + 2;//query 5
        g_F34Query.PayloadLen = (data[offset + 1] << 8) | data[offset]; offset = offset + 2;//query 6
    }
    FUNC_END();
    return  true;

};

int ReadF54Information()
{
    // F$54 information
    FUNC_BEGIN();
    if (pdt.g_F54Descriptor.ID == 0x54)
    {
        uint8_t offset = 0;
        uint8_t query_offset = 0;
        uint8_t data_offset = 0;
        uint8_t buffer[60];
        uint8_t i_buffer[30];
        if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;

        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.QueryBase)&0xFF), &buffer[0], 60))
			return I2C_ERR;

        pdt.RxChannelCount = buffer[0];
        pdt.TxChannelCount = buffer[1];
		DPRINTF("##################pdt.RxChannelCount=(%d),pdt.TxChannelCount=(%d)################\n",pdt.RxChannelCount,pdt.TxChannelCount);	/* debug */
        pdt.TouchControllerFamily = buffer[5];

        offset++; //Ctrl 00
        if (pdt.TouchControllerFamily == 0x0 || pdt.TouchControllerFamily == 0x01
            || pdt.TouchControllerFamily == 0x04) offset++; //Ctrl 01
        offset += 2; //Ctrl 02
        pdt.bHavePixelTouchThresholdTuning = ((buffer[6] & 0x01) == 0x01);
        if (pdt.bHavePixelTouchThresholdTuning) offset++; //Ctrl 03;
        if (pdt.TouchControllerFamily == 0x0 || pdt.TouchControllerFamily == 0x01) offset += 3; //Ctrl 04/05/06
        if (pdt.TouchControllerFamily == 0x01){
            pdt.F54Ctrl07Offset = offset;
            offset++; //Ctrl 07;
            pdt.bHaveF54Ctrl07 = true;

        }
        if (pdt.TouchControllerFamily == 0x0 || pdt.TouchControllerFamily == 0x01)
        {
            pdt.F54Ctrl08Offset = offset;
            offset += 2; //Ctrl 08
        }
        if (pdt.TouchControllerFamily == 0x0 || pdt.TouchControllerFamily == 0x01) offset++; //Ctrl 09
        if (pdt.TouchControllerFamily == 0x4)
            pdt.bHaveF54Ctrl228 = true; //HFW-1043
        pdt.bHaveF54Data04 = ((buffer[7] & 0x04) == 0x04);
        pdt.bHaveF54Data0601 = ((buffer[7] & 0x02) == 0x02);
        pdt.bHaveF54Data0602 = pdt.bHaveF54Data0601;
        pdt.bHaveF54Data0701 = (((buffer[7] & 0x20) == 0x20) || ((buffer[7] & 0x40) == 0x40));
        pdt.bHaveF54Data0702 = pdt.bHaveF54Data0701;
        pdt.bHaveInterferenceMetric = ((buffer[7] & 0x02) == 0x02);
        if (pdt.bHaveInterferenceMetric) offset++; // Ctrl 10

        pdt.bHaveCtrl11 = ((buffer[7] & 0x10) == 0x10);
        if (pdt.bHaveCtrl11) offset += 2; //Ctrl 11
        pdt.bHaveRelaxationControl = ((buffer[7] & 0x80) == 0x80);
        if (pdt.bHaveRelaxationControl) offset += 2; //Ctrl 12/13
        pdt.bHaveSensorAssignment = ((buffer[7] & 0x01) == 0x01);
        if (pdt.bHaveSensorAssignment) offset++; //Ctrl 14
        if (pdt.bHaveSensorAssignment) offset += pdt.RxChannelCount; //Ctrl 15
        if (pdt.bHaveSensorAssignment) offset += pdt.TxChannelCount; //Ctrl 16
        pdt.bHaveSenseFrequencyControl = ((buffer[7] & 0x04) == 0x04);

        if (pdt.bHaveSenseFrequencyControl) pdt.NumberOfSensingFrequencies = (buffer[13] & 0x0F);
        if (pdt.bHaveSenseFrequencyControl) offset += (3 * pdt.NumberOfSensingFrequencies); //Ctrl 17/18/19
        pdt.F54Ctrl20Offset = offset;
        offset++; //Ctrl 20
        if (pdt.bHaveSenseFrequencyControl) offset += 2; //Ctrl 21
        pdt.bHaveFirmwareNoiseMitigation = ((buffer[7] & 0x08) == 0x08);
        if (pdt.bHaveFirmwareNoiseMitigation) offset++; //Ctrl 22
        if (pdt.bHaveFirmwareNoiseMitigation) offset += 2; //Ctrl 23
        if (pdt.bHaveFirmwareNoiseMitigation) offset += 2; //Ctrl 24
        if (pdt.bHaveFirmwareNoiseMitigation) offset++; //Ctrl 25
        if (pdt.bHaveFirmwareNoiseMitigation) offset++; //Ctrl 26
        pdt.bHaveIIRFilter = ((buffer[9] & 0x02) == 0x02);
        if (pdt.bHaveIIRFilter) offset++; //Ctrl 27
        if (pdt.bHaveFirmwareNoiseMitigation) offset += 2; //Ctrl 28
        pdt.bHaveCmnRemoval = ((buffer[9] & 0x04) == 0x04);
        pdt.bHaveCmnMaximum = ((buffer[9] & 0x08) == 0x08);
        if (pdt.bHaveCmnRemoval) offset++; //Ctrl 29
        if (pdt.bHaveCmnMaximum) offset++; //Ctrl 30
        pdt.bHaveTouchHysteresis = ((buffer[9] & 0x10) == 0x10);
        if (pdt.bHaveTouchHysteresis) offset++; //Ctrl 31
        pdt.bHaveEdgeCompensation = ((buffer[9] & 0x20) == 0x20);
        if (pdt.bHaveEdgeCompensation) offset += 2; //Ctrl 32
        if (pdt.bHaveEdgeCompensation) offset += 2; //Ctrl 33
        if (pdt.bHaveEdgeCompensation) offset += 2; //Ctrl 34
        if (pdt.bHaveEdgeCompensation) offset += 2; //Ctrl 35
        pdt.CurveCompensationMode = (buffer[8] & 0x03);
        if (pdt.CurveCompensationMode == 0x02) {
            offset += (int32_t)pdt.RxChannelCount;
        }
        else if (pdt.CurveCompensationMode == 0x01) {
            offset += (pdt.RxChannelCount > pdt.TxChannelCount ? pdt.RxChannelCount : pdt.TxChannelCount);
        } //Ctrl 36
        if (pdt.CurveCompensationMode == 0x02) offset += pdt.TxChannelCount; //Ctrl 37
        pdt.bHavePerFrequencyNoiseControl = ((buffer[9] & 0x40) == 0x40);
        if (pdt.bHavePerFrequencyNoiseControl) offset += (3 * pdt.NumberOfSensingFrequencies); //Ctrl 38/39/40
        pdt.bHaveSignalClarity = ((buffer[10] & 0x04) == 0x04);
        if (pdt.bHaveSignalClarity){
            pdt.F54Ctrl41Offset = offset; //Ctrl 41
            offset++;
            pdt.bSignalClarityOn = true;
        }
        else pdt.bSignalClarityOn = false;
        pdt.bHaveF54Data0901 = ((buffer[10] & 0x02) == 0x02);
        pdt.bHaveF54Data0902 = pdt.bHaveF54Data0901;
        pdt.bHaveF54Data10 = (((buffer[10] & 0x02) == 0x02) || ((buffer[11] & 0x40) == 0x40));
        pdt.bHaveF54Data13 = ((buffer[10] & 0x02) == 0x02);
        pdt.bHaveF54Data0801 = ((buffer[10] & 0x08) == 0x08);
        pdt.bHaveF54Data0802 = pdt.bHaveF54Data0801;
        pdt.bHaveF54Data11 = ((buffer[10] & 0x40) == 0x40);
        pdt.bHaveF54Data12 = ((buffer[10] & 0x80) == 0x80);

        pdt.bHaveMultiMetricStateMachine = ((buffer[10] & 0x02) == 0x02);
        pdt.bHaveVarianceMetric = ((buffer[10] & 0x08) == 0x08);
        if (pdt.bHaveVarianceMetric) offset += 2; //Ctr 42
        if (pdt.bHaveMultiMetricStateMachine) offset += 2; //Ctrl 43
        if (pdt.bHaveMultiMetricStateMachine) offset += 11; //Ctrl 44/45/46/47/48/49/50/51/52/53/54
        pdt.bHave0DRelaxationControl = ((buffer[10] & 0x10) == 0x10);
        pdt.bHave0DAcquisitionControl = ((buffer[10] & 0x20) == 0x20);
        if (pdt.bHave0DRelaxationControl) offset += 2; //Ctrl 55/56
        if (pdt.bHave0DAcquisitionControl){
            pdt.F54Ctrl57Offset = offset;
            offset++; //Ctrl 57;
            pdt.bHaveF54Ctrl57 = true;
        }
        if (pdt.bHave0DAcquisitionControl) offset += 1; //Ctrl 58
        pdt.bHaveSlewMetric = ((buffer[10] & 0x80) == 0x80);
        pdt.bHaveHBlank = ((buffer[11] & 0x01) == 0x01);
        pdt.bHaveVBlank = ((buffer[11] & 0x02) == 0x02);
        pdt.bHaveLongHBlank = ((buffer[11] & 0x04) == 0x04);
        pdt.bHaveNoiseMitigation2 = ((buffer[11] & 0x20) == 0x20);
        pdt.bHaveSlewOption = ((buffer[12] & 0x02) == 0x02);
        if (pdt.bHaveHBlank) offset += 1; //Ctrl 59
        if (pdt.bHaveHBlank || pdt.bHaveVBlank || pdt.bHaveLongHBlank) offset += 3; //Ctrl 60/61/62
        if (pdt.bHaveSlewMetric || pdt.bHaveHBlank || pdt.bHaveVBlank || pdt.bHaveLongHBlank || pdt.bHaveNoiseMitigation2 || pdt.bHaveSlewOption) offset += 1; //Ctrl 63
        if (pdt.bHaveHBlank) offset += 28; //Ctrl 64/65/66/67
        else if (pdt.bHaveVBlank || pdt.bHaveLongHBlank) offset += 4; //Ctrl 64/65/66/67
        if (pdt.bIsTDDIHIC)
            offset += 6; //Ctrl 70/71/72/73
        else if (pdt.bHaveHBlank || pdt.bHaveVBlank || pdt.bHaveLongHBlank) offset += 8; //Ctrl 68/69/70/71/72/73
        if (pdt.bHaveSlewMetric) offset += 2; //Ctrl 74
        pdt.bHaveEnhancedStretch = ((buffer[9] & 0x80) == 0x80);
        if (pdt.bHaveEnhancedStretch) offset += pdt.NumberOfSensingFrequencies; //Ctrl 75
        pdt.bHaveStartupFastRelaxation = ((buffer[11] & 0x08) == 0x08);
        if (pdt.bHaveStartupFastRelaxation) offset += 1; //Ctrl 76
        pdt.bHaveESDControl = ((buffer[11] & 0x10) == 0x10);
        if (pdt.bHaveESDControl) offset += 2; //Ctrl 77/78
        if (pdt.bHaveNoiseMitigation2) offset += 5; //Ctrl 79/80/81/82/83
        pdt.bHaveEnergyRatioRelaxation = ((buffer[11] & 0x80) == 0x80);
        if (pdt.bHaveEnergyRatioRelaxation) offset += 2; //Ctrl 84/85
        pdt.bHaveF54Query13 = ((buffer[12] & 0x08) == 0x08);


        if (pdt.bHaveSenseFrequencyControl) {
            query_offset = 13;
            pdt.NumberOfSensingFrequencies = (buffer[13] & 0x0F);
        }
        else query_offset = 12;


        if (pdt.bHaveF54Query13) query_offset++;
        pdt.bHaveF54Data14 = pdt.bHaveF54CIDIM = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Data16 = ((buffer[query_offset] & 0x40) == 0x40);
        pdt.bHaveF54Data15 = ((buffer[query_offset] & 0x80) == 0x80);

        pdt.bHaveF54Ctrl86 = (pdt.bHaveF54Query13 && ((buffer[query_offset] & 0x01) == 0x01));
        pdt.bHaveF54Ctrl87 = (pdt.bHaveF54Query13 && ((buffer[query_offset] & 0x02) == 0x02));
        pdt.bHaveF54Ctrl88 = ((buffer[12] & 0x40) == 0x40);
        if (pdt.bHaveF54Ctrl86)
        {
            pdt.F54Ctrl86Offset = offset;
            offset += 1; //Ctrl 86
        }
        if (pdt.bHaveF54Ctrl87) offset += 1; //Ctrl 87
        if (pdt.bHaveF54Ctrl88){
            pdt.F54Ctrl88Offset = offset;
            offset++; //Ctrl 88;
        }
        pdt.bHaveF54Ctrl89 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Ctrl89 = (pdt.bHaveF54Ctrl89 | ((buffer[query_offset] & 0x40) == 0x40));
        pdt.bHaveF54Ctrl89 = (pdt.bHaveF54Ctrl89 | ((buffer[query_offset] & 0x80) == 0x80));
        if (pdt.bHaveF54Ctrl89)
        {
            pdt.F54Ctrl89Offset = offset;
            offset++;
        }
        pdt.bHaveF54Query15 = ((buffer[12] & 0x80) == 0x80);
        if (pdt.bHaveF54Query15) query_offset++;
        pdt.bHaveF54Ctrl90 = (pdt.bHaveF54Query15 && ((buffer[query_offset] & 0x01) == 0x01));
        if (pdt.bHaveF54Ctrl90) offset++; //offset = 1b
        pdt.bHaveF54Query16 = ((buffer[query_offset] & 0x8) == 0x8);
        pdt.bHaveF54Query20 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Query21 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Query22 = ((buffer[query_offset] & 0x40) == 0x40);
        pdt.bHaveF54Query25 = ((buffer[query_offset] & 0x80) == 0x80);
        if (pdt.bHaveF54Query16) query_offset++; //query_offset = 15
        pdt.bHaveF54Data17 = ((buffer[query_offset] & 0x02) == 0x02);
        pdt.bHaveF54Query17 = ((buffer[query_offset] & 0x01) == 0x01);
        pdt.bHaveF54Ctrl92 = ((buffer[query_offset] & 0x4) == 0x4);
        pdt.bHaveF54Ctrl93 = ((buffer[query_offset] & 0x8) == 0x8);
        pdt.bHaveF54Ctrl94 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Query18 = pdt.bHaveF54Ctrl94;
        pdt.bHaveF54Ctrl95 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Query19 = pdt.bHaveF54Ctrl95;
        pdt.bHaveF54Ctrl99 = ((buffer[query_offset] & 0x40) == 0x40);
        pdt.bHaveF54Ctrl100 = ((buffer[query_offset] & 0x80) == 0x80);
        if (pdt.bHaveF54Query17) query_offset++; //query_offset = 16
        if (pdt.bHaveF54Query18) query_offset++; //query_offset = 17
        if (pdt.bHaveF54Query19) query_offset++; //query_offset = 18
        if (pdt.bHaveF54Query20) query_offset++; //query_offset = 19
        if (pdt.bHaveF54Query21) query_offset++; //query_offset = 20
        pdt.bHaveF54Data19 = ((buffer[query_offset] & 0x40) == 0x40);
        pdt.bHaveF54Ctrl91 = ((buffer[query_offset] & 0x4) == 0x4);
        pdt.bHaveF54Ctrl96 = ((buffer[query_offset] & 0x8) == 0x8);
        pdt.bHaveF54Ctrl97 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Ctrl98 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Query24 = pdt.bHaveF54Data18 = ((buffer[query_offset] & 0x80) == 0x80);
        if (pdt.bHaveF54Query22) query_offset++; //query_offset = 21
        pdt.bHaveF54Ctrl101 = ((buffer[query_offset] & 0x2) == 0x2);
        pdt.bHaveF54Query23 = ((buffer[query_offset] & 0x8) == 0x8);
        pdt.bHaveF54Query26 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Ctrl103 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Ctrl104 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Ctrl105 = ((buffer[query_offset] & 0x40) == 0x40);
        pdt.bHaveF54Query28 = ((buffer[query_offset] & 0x80) == 0x80);
        if (pdt.bHaveF54Query23) {
            query_offset++; //query_offset = 22
            pdt.bHaveF54Ctrl102 = ((buffer[query_offset] & 0x01) == 0x01);

        }
        else
            pdt.bHaveF54Ctrl102 = false;

        if (pdt.bHaveF54Ctrl91){ pdt.F54Ctrl91Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl92) offset++;
        if (pdt.bHaveF54Ctrl93) offset++;
        if (pdt.bHaveF54Ctrl94) offset++;
        if (pdt.bHaveF54Ctrl95) { pdt.F54Ctrl95Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl96) { pdt.F54Ctrl96Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl97) { pdt.F54Ctrl97Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl98) { pdt.F54Ctrl98Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl99) { pdt.F54Ctrl99Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl100) offset++;
        if (pdt.bHaveF54Ctrl101) offset++;
        // for incell 
        if (pdt.bHaveF54Ctrl102){

            uint8_t tsvd_select;
            uint8_t i_offset = 0;
            pdt.F54Ctrl102Offset = offset;
            offset++;
            bool HasCtrl102Sub1 = (bool)(buffer[query_offset] & 0x02);
            bool HasCtrl102Sub2 = (bool)(buffer[query_offset] & 0x04);
            bool HasCtrl102Sub4 = (bool)(buffer[query_offset] & 0x08);
            bool HasCtrl102Sub5 = (bool)(buffer[query_offset] & 0x010);
            bool HasCtrl102Sub9 = (bool)(buffer[query_offset] & 0x020);
            bool HasCtrl102Sub10 = (buffer[query_offset] & 0x40);
            bool HasCtrl102Sub11 = (buffer[query_offset] & 0x80);
            bool HasCtrl102Sub12 = false;

            if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl102Offset)&0xFF), &i_buffer[0], 27))
				return I2C_ERR;

            tsvd_select = i_buffer[0] & 0x03;
            pdt.tsvd = i_buffer[1 + tsvd_select];
            i_offset = i_offset + 4;
            pdt.tshd = i_buffer[i_offset];

            if (HasCtrl102Sub1) {
                i_offset = i_offset + 2;
                pdt.tsstb = i_buffer[i_offset];
            }

            if (HasCtrl102Sub2) {
                pdt.tsfrq = i_buffer[i_offset + 2];
                pdt.tsfst = i_buffer[i_offset + 3];
                i_offset = i_offset + 3;
            }
            //Ctrl102Sub3
            pdt.exvcom_pin_type = (i_buffer[i_offset + 1] & 0x01); // 0 = GPIO, 1 = TRX
            pdt.exvcom1 = i_buffer[i_offset + 2];
            i_offset = i_offset + 2;

            if (HasCtrl102Sub4) {
                pdt.exvcom_sel = (i_buffer[i_offset + 1] & 0x03);
                pdt.exvcom2 = i_buffer[i_offset + 2];
                i_offset = i_offset + 4;
            }

            if (HasCtrl102Sub5) {
                pdt.enable_guard = (i_buffer[i_offset + 1] & 0x01);
                pdt.guard_ring = i_buffer[i_offset + 2];
                i_offset = i_offset + 2;
            }
            //Ctrl102Sub6, 7, 8
            i_offset = i_offset + 5;

            if (HasCtrl102Sub9) i_offset++;

            if (HasCtrl102Sub10){
                pdt.exvcom_sel = i_buffer[i_offset + 2];
                i_offset = i_offset + 2;
            }

            if (HasCtrl102Sub11) i_offset++;

            if (pdt.bHaveF54Query25)
                HasCtrl102Sub12 = (i_buffer[query_offset + 1] & 0x02);
            if (HasCtrl102Sub12){
                pdt.enable_verf = ((i_buffer[i_offset + 1]) & 0x01);
                pdt.verf = (i_buffer[i_offset + 2]);
            }

        }
        if (pdt.bHaveF54Query24) query_offset++;
        if (pdt.bHaveF54Query25) query_offset++;
        pdt.bHaveF54Ctrl106 = ((buffer[query_offset] & 0x01) == 0x01);
        pdt.bHaveF54Ctrl107 = ((buffer[query_offset] & 0x04) == 0x04);
        pdt.bHaveF54Ctrl108 = ((buffer[query_offset] & 0x08) == 0x08);
        pdt.bHaveF54Ctrl109 = ((buffer[query_offset] & 0x10) == 0x10);
        pdt.bHaveF54Data20 = ((buffer[query_offset] & 0x20) == 0x20);
        pdt.bHaveF54Query27 = ((buffer[query_offset] & 0x80) == 0x80);
        if (pdt.bHaveF54Query26) query_offset++;
        if (pdt.bHaveF54Query27) {
            query_offset++;
            pdt.bHaveF54Ctrl110 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Data21 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl111 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl112 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl113 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Data22 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl114 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query29 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query28) query_offset++;
        if (pdt.bHaveF54Query29) {
            query_offset++;
            pdt.bHaveF54Ctrl115 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl116 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Data23 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl117 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query30 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query30) {
            query_offset++;
            pdt.bHaveF54Ctrl118 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl119 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl120 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl121 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl122 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Query31 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl123 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl124 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query32 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query31) query_offset++;
        if (pdt.bHaveF54Query32) {
            query_offset++;
            pdt.bHaveF54Ctrl125 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl126 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl127 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Query33 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Data24 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Query34 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query35 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query33) {
            query_offset++;
            pdt.bHaveF54Ctrl128 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl129 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl130 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl131 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl132 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl133 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl134 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query36 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query34) query_offset++;
        if (pdt.bHaveF54Query35) {
            query_offset++;
            pdt.bHaveF54Data25 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl135 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl136 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl137 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl138 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl139 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Data26 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Ctrl140 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query36) {
            query_offset++;
            pdt.bHaveF54Ctrl141 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl142 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Query37 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl143 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl144 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl145 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl146 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query38 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query37) query_offset++;
        if (pdt.bHaveF54Query38) {
            query_offset++;
            pdt.bHaveF54Ctrl147 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl148 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl149 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl151 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl152 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl153 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query39 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query39) {
            query_offset++;
            pdt.bHaveF54Ctrl154 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl155 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl156 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl160 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl157 = pdt.bHaveF54Ctrl158 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl159 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl161 = pdt.bHaveF54Ctrl162 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query40 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query40) {
            query_offset++;
            pdt.bHaveF54Ctrl169 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl163 = pdt.bHaveF54Query41 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl164 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl165 = pdt.bHaveF54Query42 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl166 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl167 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl168 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query43 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query41) query_offset++;
        if (pdt.bHaveF54Query42) query_offset++;
        if (pdt.bHaveF54Query43) {
            query_offset++;
            pdt.bHaveF54Ctrl170 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl171 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl172 = pdt.bHaveF54Query44 = pdt.bHaveF54Query45 =
                ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl173 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl174 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl175 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query46 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query44) query_offset++;
        if (pdt.bHaveF54Query45) query_offset++;
        if (pdt.bHaveF54Query46) {
            query_offset++;
            pdt.bHaveF54Ctrl176 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl177 = pdt.bHaveF54Ctrl178 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl179 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl180 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl181 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query47 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query47) {
            query_offset++;
            pdt.bHaveF54Ctrl182 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl183 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Query48 = pdt.bHaveF54Ctrl184 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl185 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl186 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl187 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query49 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query48) query_offset++;
        if (pdt.bHaveF54Query49) {
            query_offset++;
            pdt.bHaveF54Ctrl188 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Data31 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl189 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl190 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query50 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query50) {
            query_offset++;
            pdt.bHaveF54Ctrl191 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl192 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl193 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Query52 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl194 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl195 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query51 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query51) {
            query_offset++;
            pdt.bHaveF54Ctrl196 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl197 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Query54 = pdt.bHaveF54Query53 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl198 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl199 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query55 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query52) {
            query_offset++;
        }
        if (pdt.bHaveF54Query53) {
            query_offset++;
        }
        if (pdt.bHaveF54Query54) {
            query_offset++;
        }
        if (pdt.bHaveF54Query55) {
            query_offset++;
            pdt.bHaveF54Query56 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl200 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl202 = pdt.bHaveF54Ctrl201 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl203 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl204 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query57 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query56)   query_offset++;
        if (pdt.bHaveF54Query57) {
            query_offset++;
            pdt.bHaveF54Ctrl205 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl206 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl207 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl208 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl209 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl210 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query58 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query58) {
            query_offset++;
            pdt.bHaveF54Query59 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Query60 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl211 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl212 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl213 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query61 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query59) query_offset++;
        if (pdt.bHaveF54Query60) query_offset++;
        if (pdt.bHaveF54Query61) {
            query_offset++;
            pdt.bHaveF54Ctrl214 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl215 = pdt.bHaveF54Query62 = pdt.bHaveF54Query63 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl216 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl217 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl218 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl219 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query64 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query62) query_offset++;
        if (pdt.bHaveF54Query63) query_offset++;
        if (pdt.bHaveF54Query64) {
            query_offset++;
            pdt.bHaveF54Ctrl220 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl221 = ((buffer[query_offset] & 0x04) == 0x04);
            pdt.bHaveF54Ctrl222 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl224 = pdt.bHaveF54Ctrl226 = pdt.bHaveF54Ctrl227 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query65 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query65) {
            query_offset++;
            pdt.bHaveF54Ctrl225 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl229 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl230 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl231 = pdt.bHaveF54Query66 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Ctrl232 = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query67 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query66) query_offset++;
        if (pdt.bHaveF54Query67) {
            query_offset++;
            pdt.bHaveF54Ctrl233 = ((buffer[query_offset] & 0x08) == 0x08);
            pdt.bHaveF54Ctrl234 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bHaveF54Ctrl235 = pdt.bHaveF54Ctrl236 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Query68 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query68) {
            query_offset++;
            pdt.bHaveF54Ctrl237 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl238 = ((buffer[query_offset] & 0x02) == 0x02);
            pdt.bHaveF54Ctrl239 = ((buffer[query_offset] & 0x10) == 0x10);
            pdt.bIsTDDIHIC = ((buffer[query_offset] & 0x40) == 0x40);
            pdt.bHaveF54Query69 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query69) {
            query_offset++;
            pdt.bHaveF54Ctrl240 = ((buffer[query_offset] & 0x01) == 0x01);
            pdt.bHaveF54Ctrl241 = ((buffer[query_offset] & 0x20) == 0x20);
            pdt.bHaveF54Query70 = ((buffer[query_offset] & 0x80) == 0x80);
        }
        if (pdt.bHaveF54Query70) {
            query_offset++;
            pdt.bHaveF54Ctrl242 = ((buffer[query_offset] & 0x04) == 0x04);
      		pdt.bHaveF54Query71 = ((buffer[query_offset] & 0x80) == 0x80);
        }
		if (pdt.bHaveF54Query71) {
			query_offset++;
			pdt.bHasInCellSideTouchLeft = ((buffer[query_offset] & 0x04) == 0x04);
			pdt.bHasInCellSideTouchRight = ((buffer[query_offset] & 0x08) == 0x08);
			pdt.bHasInCellSideTouchTop = ((buffer[query_offset] & 0x10) == 0x10);
			pdt.bHasInCellSideTouchBottom = ((buffer[query_offset] & 0x20) == 0x20);
		}
        if (pdt.bIsTDDIHIC)
        {
            pdt.bHaveF54Ctrl243 = true;
            pdt.bHaveF54Ctrl244 = true;
            pdt.bHaveF54Ctrl245 = true;
            if (pdt.bHaveF54Ctrl190) pdt.bHaveF54Ctrl246 = true;
        }
        //-----------------------------------------------------------from Ctrl 103

        //offset = offset - 1;
        if (pdt.bHaveF54Ctrl103) offset++;
        if (pdt.bHaveF54Ctrl104) offset++;
        if (pdt.bHaveF54Ctrl105) offset++;
        if (pdt.bHaveF54Ctrl106) offset++;
        if (pdt.bHaveF54Ctrl107) offset++;
        if (pdt.bHaveF54Ctrl108) offset++;
        if (pdt.bHaveF54Ctrl109) offset++;
        if (pdt.bHaveF54Ctrl110) offset++;
        if (pdt.bHaveF54Ctrl111) offset++;
        if (pdt.bHaveF54Ctrl112) offset++;
        if (pdt.bHaveF54Ctrl113) offset++;
        if (pdt.bHaveF54Ctrl114) offset++;
        if (pdt.bHaveF54Ctrl115) offset++;
        if (pdt.bHaveF54Ctrl116) offset++;
        if (pdt.bHaveF54Ctrl117) offset++;
        if (pdt.bHaveF54Ctrl118) offset++;
        if (pdt.bHaveF54Ctrl119) offset++;
        if (pdt.bHaveF54Ctrl120) offset++;
        if (pdt.bHaveF54Ctrl121) offset++;
        if (pdt.bHaveF54Ctrl122) offset++;
        if (pdt.bHaveF54Ctrl123) offset++;
        if (pdt.bHaveF54Ctrl124) offset++;
        if (pdt.bHaveF54Ctrl125) offset++;
        if (pdt.bHaveF54Ctrl126) offset++;
        if (pdt.bHaveF54Ctrl127) offset++;
        if (pdt.bHaveF54Ctrl128) offset++;
        if (pdt.bHaveF54Ctrl129) offset++;
        if (pdt.bHaveF54Ctrl130) offset++;
        if (pdt.bHaveF54Ctrl131) offset++;
        if (pdt.bHaveF54Ctrl132) { pdt.F54Ctrl132Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl133) offset++;
        if (pdt.bHaveF54Ctrl134) offset++;
        if (pdt.bHaveF54Ctrl135) offset++;
        if (pdt.bHaveF54Ctrl136) offset++;
        if (pdt.bHaveF54Ctrl137) offset++;
        if (pdt.bHaveF54Ctrl138) offset++;
        if (pdt.bHaveF54Ctrl139) offset++;
        if (pdt.bHaveF54Ctrl140) offset++;
        if (pdt.bHaveF54Ctrl141) offset++;
        if (pdt.bHaveF54Ctrl142) offset++;
        if (pdt.bHaveF54Ctrl143) offset++;
        if (pdt.bHaveF54Ctrl144) offset++;
        if (pdt.bHaveF54Ctrl145) offset++;
        if (pdt.bHaveF54Ctrl146) { pdt.F54Ctrl146Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl147) { pdt.F54Ctrl147Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl148) offset++;
        if (pdt.bHaveF54Ctrl149) { pdt.F54Ctrl149Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl150) offset++;
        if (pdt.bHaveF54Ctrl151) offset++;
        if (pdt.bHaveF54Ctrl152) offset++;
        if (pdt.bHaveF54Ctrl153) offset++;
        if (pdt.bHaveF54Ctrl154) offset++;
        if (pdt.bHaveF54Ctrl155) offset++;
        if (pdt.bHaveF54Ctrl156) offset++;
        if (pdt.bHaveF54Ctrl157) offset++;
        if (pdt.bHaveF54Ctrl158) offset++;
        if (pdt.bHaveF54Ctrl159) offset++;
        if (pdt.bHaveF54Ctrl160) offset++;
        if (pdt.bHaveF54Ctrl161) offset++;
        if (pdt.bHaveF54Ctrl162) offset++;
        if (pdt.bHaveF54Ctrl163) offset++;
        if (pdt.bHaveF54Ctrl164) offset++;
        if (pdt.bHaveF54Ctrl165) offset++;
        if (pdt.bHaveF54Ctrl166) offset++;
        if (pdt.bHaveF54Ctrl167) offset++;
        if (pdt.bHaveF54Ctrl168) offset++;
        if (pdt.bHaveF54Ctrl169) offset++;
        if (pdt.bHaveF54Ctrl170) offset++;
        if (pdt.bHaveF54Ctrl171) offset++;
        if (pdt.bHaveF54Ctrl172) offset++;
        if (pdt.bHaveF54Ctrl173) offset++;
        if (pdt.bHaveF54Ctrl174) offset++;
        if (pdt.bHaveF54Ctrl175) offset++;
        if (pdt.bHaveF54Ctrl176) offset++;
        if (pdt.bHaveF54Ctrl177) offset++;
        if (pdt.bHaveF54Ctrl178) offset++;
        if (pdt.bHaveF54Ctrl179) offset++;
        if (pdt.bHaveF54Ctrl180) offset++;
        if (pdt.bHaveF54Ctrl181) offset++;
        if (pdt.bHaveF54Ctrl182) { pdt.F54Ctrl182Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl183) offset++;
        if (pdt.bHaveF54Ctrl184) offset++;
        if (pdt.bHaveF54Ctrl185) offset++;
        if (pdt.bHaveF54Ctrl186) { pdt.F54Ctrl186Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl187) offset++;
        if (pdt.bHaveF54Ctrl188) { pdt.F54Ctrl188Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl189) { pdt.F54Ctrl189Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl190) { pdt.F54Ctrl190Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl191) offset++;
        if (pdt.bHaveF54Ctrl192) offset++;
        if (pdt.bHaveF54Ctrl193) offset++;
        if (pdt.bHaveF54Ctrl194) offset++;
        if (pdt.bHaveF54Ctrl195) offset++;
        if (pdt.bHaveF54Ctrl196) offset++;
        if (pdt.bHaveF54Ctrl197) offset++;
        if (pdt.bHaveF54Ctrl198) offset++;
        if (pdt.bHaveF54Ctrl199) offset++;
        if (pdt.bHaveF54Ctrl200) offset++;
        if (pdt.bHaveF54Ctrl201) offset++;
        if (pdt.bHaveF54Ctrl202) offset++;
        if (pdt.bHaveF54Ctrl203) offset++;
        if (pdt.bHaveF54Ctrl204) offset++;
        if (pdt.bHaveF54Ctrl205) offset++;
        if (pdt.bHaveF54Ctrl206) offset++;
        if (pdt.bHaveF54Ctrl207) offset++;
        if (pdt.bHaveF54Ctrl208) offset++;
        if (pdt.bHaveF54Ctrl209) offset++;
        if (pdt.bHaveF54Ctrl210) offset++;
        if (pdt.bHaveF54Ctrl211) offset++;
        if (pdt.bHaveF54Ctrl212) offset++;
        if (pdt.bHaveF54Ctrl213) offset++;
        if (pdt.bHaveF54Ctrl214) offset++;
        if (pdt.bHaveF54Ctrl215) { pdt.F54Ctrl215Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl216) offset++;
        if (pdt.bHaveF54Ctrl217) offset++;
        if (pdt.bHaveF54Ctrl218) offset++;
        if (pdt.bHaveF54Ctrl219) offset++;
        if (pdt.bHaveF54Ctrl220) offset++;
        if (pdt.bHaveF54Ctrl221) offset++;
        if (pdt.bHaveF54Ctrl222) offset++;
        if (pdt.bHaveF54Ctrl223) offset++;
        if (pdt.bHaveF54Ctrl224) offset++;
        if (pdt.bHaveF54Ctrl225) { pdt.F54Ctrl225Offset = offset; offset++; }
        if (pdt.bHaveF54Ctrl226) offset++;
        if (pdt.bHaveF54Ctrl227) offset++;
        if (pdt.bHaveF54Ctrl228) offset++;
        if (pdt.bHaveF54Ctrl229) offset++;
        if (pdt.bHaveF54Ctrl230) offset++;
        if (pdt.bHaveF54Ctrl231) offset++;
        if (pdt.bHaveF54Ctrl232) offset++;
        if (pdt.bHaveF54Ctrl233) offset++;
        if (pdt.bHaveF54Ctrl234) offset++;
        if (pdt.bHaveF54Ctrl235) offset++;
        if (pdt.bHaveF54Ctrl236) offset++;
        if (pdt.bHaveF54Ctrl237) offset++;
        if (pdt.bHaveF54Ctrl238) offset++;
        if (pdt.bHaveF54Ctrl239) offset++;
        if (pdt.bHaveF54Ctrl240) offset++;
        if (pdt.bHaveF54Ctrl241) offset++;
        if (pdt.bHaveF54Ctrl242) offset++;
        if (pdt.bHaveF54Ctrl243) offset++;
        if (pdt.bHaveF54Ctrl244) offset++;
        if (pdt.bHaveF54Ctrl245) offset++;
        if (pdt.bHaveF54Ctrl246) { pdt.F54Ctrl246Offset = offset; offset++; }

        //-----------------------------------------------------------Data register
        pdt.bHaveF54Data00 = true; //Report type
        pdt.bHaveF54Data01 = true; //Report index
        pdt.bHaveF54Data02 = true; //Report index
        pdt.bHaveF54Data03 = true; //Report data

        if (pdt.bHaveF54Data01) data_offset++;
        if (pdt.bHaveF54Data02) data_offset++;
        if (pdt.bHaveF54Data03) data_offset++;
        if (pdt.bHaveF54Data04)
        {
            data_offset++;
            pdt.F54Data04Offset = data_offset;
        }
        if (pdt.bHaveF54Data05) data_offset++;
        if (pdt.bHaveF54Data0601)
        {
            data_offset++;
            pdt.F54Data06Offset = data_offset;
        }
        if (pdt.bHaveF54Data0602) data_offset++;
        if (pdt.bHaveF54Data0701)
        {
            data_offset++;
            pdt.F54Data07Offset = data_offset;
        }
        if (pdt.bHaveF54Data0702) data_offset++;
        if (pdt.bHaveF54Data0801) data_offset++;
        if (pdt.bHaveF54Data0802) data_offset++;
        if (pdt.bHaveF54Data0901) data_offset++;
        if (pdt.bHaveF54Data0902) data_offset++;
        if (pdt.bHaveF54Data10)
        {
            data_offset++;
            pdt.F54Data10Offset = data_offset;
        }
        if (pdt.bHaveF54Data11) data_offset++;
        if (pdt.bHaveF54Data12) data_offset++;
        if (pdt.bHaveF54Data13) data_offset++;
        if (pdt.bHaveF54Data14)
        {
            data_offset++;
            pdt.F54Data14Offset = data_offset;
        }
        if (pdt.bHaveF54Data15) data_offset++;
        if (pdt.bHaveF54Data16) data_offset++;
        if (pdt.bHaveF54Data17)
        {
            data_offset++;
            pdt.F54Data17Offset = data_offset;
        }
        if (pdt.bHaveF54Data18) data_offset++;
        if (pdt.bHaveF54Data19) data_offset++;
        if (pdt.bHaveF54Data20) data_offset++;
        if (pdt.bHaveF54Data21) data_offset++;
        if (pdt.bHaveF54Data22) data_offset++;
        if (pdt.bHaveF54Data23) data_offset++;
        if (pdt.bHaveF54Data24)
        {
            data_offset++;
            pdt.F54Data24Offset = data_offset;
        }
        if (pdt.bHaveF54Data25) data_offset++;
        if (pdt.bHaveF54Data26) data_offset++;
        if (pdt.bHaveF54Data27) data_offset++;
        if (pdt.bHaveF54Data28) data_offset++;
        if (pdt.bHaveF54Data29) data_offset++;
        if (pdt.bHaveF54Data30) data_offset++;
        if (pdt.bHaveF54Data31) {
            data_offset++;
            pdt.F54Data31Offset = data_offset;
        }
    }
    FUNC_END();
    return  true;

}

int ReadF55Information()
{
    FUNC_BEGIN();
    uint32_t i = 0;
    uint8_t j = 0;

    // F$55 information
    if (pdt.g_F55Descriptor.ID == 0x55)
    {
        uint8_t query_offset = 0;
        uint8_t offset = 0;
        uint8_t buffer[64];

        if(!rmi_set_page((unsigned char)(((pdt.g_F55Descriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.QueryBase)&0xFF), &buffer[0], 3))
			return I2C_ERR;


        pdt.maxRxCount = buffer[0];
        pdt.maxTxCount = buffer[1];
		DPRINTF("##################pdt.maxRxCount=(%d),pdt.maxTxCount=(%d)################\n",pdt.maxRxCount,pdt.maxTxCount);	/* debug */

        pdt.RxChannelCount = 0;
        pdt.TxChannelCount = 0;
        if ((buffer[2] & 0x01)){

            if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.ControlBase + 1)&0xFF), &buffer[0], pdt.maxRxCount))
				return I2C_ERR;
            for (i = 0; i < pdt.maxRxCount; i++){
                if (buffer[i] != 0xFF){
                    pdt.RxChannelCount++;
                    pdt.TRxPhysical[i] = buffer[i];
                    pdt.RxPhysical[i] = buffer[i];
                }
                else{
                    break;
                }
            }

            if ((pdt.bIncellDevice) && (!pdt.bHaveF54Ctrl189)){
                offset = pdt.RxChannelCount;
                if (pdt.tsvd != 0xff) {
                    pdt.TRxPhysical[offset] = pdt.tsvd;
                    offset++;
                }
                if (pdt.tshd != 0xff) {
                    pdt.TRxPhysical[offset] = pdt.tshd;
                    offset++;
                }
                if (pdt.tsstb != 0xff) {
                    pdt.TRxPhysical[offset] = pdt.tsstb;
                    offset++;
                }
                if (pdt.tsfst != 0xff) {
                    pdt.TRxPhysical[offset] = pdt.tsfst;
                    offset++;
                }
                if (pdt.exvcom_pin_type){
                    pdt.TRxPhysical[offset] = pdt.exvcom1;
                    pdt.TRxPhysical[offset + 1] = pdt.exvcom2;
                    offset = offset + 2;
                }
                if (pdt.enable_guard){
                    pdt.TRxPhysical[offset] = pdt.guard_ring;
                    offset++;
                }
                if (pdt.enable_verf){
                    pdt.TRxPhysical[offset] = pdt.verf;
                    offset++;
                }
                for (i = offset; i < (TRX_MAPPING_MAX); i++){
                    pdt.TRxPhysical[i] = 0xFF;
                }
                if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.ControlBase + 2)&0xFF), &buffer[0], pdt.maxTxCount))
					return I2C_ERR;
                for (i = 0; i < pdt.maxTxCount; i++){
                    if (buffer[i] != 0xFF){
                        pdt.TxChannelCount++;
                    }
                    else    break;
                }
            }
            else{

                if (pdt.bHaveF54Ctrl189)
                {
					if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.QueryBase)>> 8) &0xFF)))
						return I2C_ERR;
                    //for incell, ignore first two bytes.
                    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl189Offset)&0xFF), &buffer[0], pdt.maxTxCount + 2))
						return I2C_ERR;
                    for (i = 0; i < pdt.maxTxCount; i++)
                        buffer[i] = buffer[i + 2];
                }
                else
                {
					if(!rmi_set_page((unsigned char)(((pdt.g_F55Descriptor.QueryBase)>> 8) &0xFF)))
						return I2C_ERR;
                    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.ControlBase + 2)&0xFF), &buffer[0], pdt.maxTxCount))
						return I2C_ERR;
                }

                for (i = 0; i < pdt.maxTxCount; i++){
                    if (buffer[i] != 0xFF){
                        pdt.TRxPhysical[pdt.RxChannelCount + i] = buffer[i];
                        pdt.TxPhysical[i] = buffer[i];
                        pdt.TxChannelCount++;
                        if (pdt.TxPhysicalMax < buffer[i])
                            pdt.TxPhysicalMax = buffer[i]; //keep the max physical number of Tx
                    }
                    else
                    {
                        if (pdt.ButtonCount > 0) { //ctrl189 didn't include button's tx
                            bool shared = false;
                            for (j = 0; j < i; j++)
                            {
                                if (pdt.TxPhysical[j] == pdt.ButtonTx[0])
                                {
                                    shared = true;
                                    break;
                                }
                            }
                            if (!shared)
                            {
                                pdt.TxChannelCount++;
                                pdt.TRxPhysical[pdt.RxChannelCount + i] = pdt.TxPhysical[i] = pdt.ButtonTx[0];

                            }

                        }
                        break;
                    }
                }

                pdt.TxPhysicalMax = (pdt.TxPhysicalMax / 8) + 1;
                for (i = (pdt.RxChannelCount + pdt.TxChannelCount); i < (TRX_MAPPING_MAX); i++){
                    pdt.TRxPhysical[i] = 0xFF;
                }
                for (i = pdt.RxChannelCount; i < TRX_MAPPING_MAX; i++) {
                    pdt.RxPhysical[i] = 0xFF;
                }
                for (i = pdt.TxChannelCount; i < TRX_MAPPING_MAX; i++) {
                    pdt.TxPhysical[i] = 0xFF;
                }

            }
        }
        for (i = 0; i < TRX_MAPPING_MAX; i++)
        {
            if (pdt.TRxPhysical[i] == 0xFF)
                break;
            uint8_t byte = pdt.TRxPhysical[i] / 8;
            uint8_t bit = pdt.TRxPhysical[i] % 8;
            pdt.TRxPhysical_bit[byte] = (pdt.TRxPhysical_bit[byte] | (0x01 << bit));
        }
        pdt.MaxArrayLength = pdt.RxChannelCount * pdt.TxChannelCount * 2;
        if ((pdt.TxChannelCount == pdt._2DTxCount) && pdt.ButtonCount > 0){
            pdt.ButtonShared = true;
        }


        offset = 2;
        if(!rmi_set_page((unsigned char)(((pdt.g_F55Descriptor.QueryBase)>> 8) &0xFF)))
			return I2C_ERR;
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.QueryBase)&0xFF), &buffer[0], 34))
			return I2C_ERR;

        if ((buffer[offset] & 0x01) == 0x01)
            pdt.bHaveF55Ctrl00 = pdt.bHaveF55Ctrl01 = pdt.bHaveF55Ctrl02 = true;
        if ((buffer[offset] & 0x02) == 0x02) pdt.bHaveF55Ctrl03 = true;

        uint8_t compensationMode = ((buffer[offset] & 0xc) >> 2);
        if (compensationMode == 1) pdt.bHaveF55Ctrl04 = true;
        if (compensationMode == 2) pdt.bHaveF55Ctrl04 = pdt.bHaveF55Ctrl05 = true;
        if (compensationMode == 3) pdt.bHaveF55Ctrl11 = pdt.bHaveF55Query06 = pdt.bHaveF55Query07 = true;
        if ((buffer[offset] & 0x10) == 0x10) pdt.bHaveF55Ctrl06 = true;
        if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl07 = true;
        if ((buffer[offset] & 0x40) == 0x40) pdt.bHaveF55Query03 = true;
        if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query05 = true;
        if (pdt.bHaveF55Query03)
        {
            offset++;
            if ((buffer[offset] & 0x01) == 0x01) pdt.bHaveF55Ctrl08 = true;
            if ((buffer[offset] & 0x02) == 0x02) pdt.bHaveF55Ctrl09 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query09 = true;
        }
        if (pdt.bHaveF55Ctrl09) offset++; //represent bHaveF55Query04
        if (pdt.bHaveF55Query05)
        {
            offset++;
            if ((buffer[offset] & 0x01) == 0x01) pdt.bHaveF55Ctrl10 = true;
            if ((buffer[offset] & 0x02) == 0x02) pdt.bHaveF55Ctrl12 = true;
            if ((buffer[offset] & 0x08) == 0x08) pdt.bHaveF55Ctrl13 = true; //Ctrl13: bonding pad active guard
            if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl14 = true;
            if ((buffer[offset] & 0x40) == 0x40)
                pdt.bHaveF55Query10 = pdt.bHaveF55Query11 = pdt.bHaveF55Query12 = pdt.bHaveF55Query13 =
                pdt.bHaveF55Query14 = pdt.bHaveF55Query15 = pdt.bHaveF55Query16 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query17 = true;
        }
        if (pdt.bHaveF55Query06) offset++;
        if (pdt.bHaveF55Query07) offset++;
        if (pdt.bHaveF55Ctrl08) offset++; //represent bHaveF55Query08
        if (pdt.bHaveF55Query09) offset++;
        if (pdt.bHaveF55Query10)
        {
            offset += 7; //query 10 ~ 16
            pdt.bHaveF55Ctrl15 = true;
        }
        if (pdt.bHaveF55Query17)
        {
            offset++;
            if ((buffer[offset] & 0x02) == 0x02) pdt.bHaveF55Ctrl16 = true;
            if ((buffer[offset] & 0x04) == 0x04) pdt.bHaveF55Ctrl18 = pdt.bHaveF55Ctrl19 = true;
            if ((buffer[offset] & 0x08) == 0x08) pdt.bHaveF55Ctrl17 = true;
            if ((buffer[offset] & 0x10) == 0x10) pdt.bHaveF55Ctrl20 = true;
            if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl21 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query18 = true;
        }
        if (pdt.bHaveF55Query18)
        {
            offset++;
            if ((buffer[offset] & 0x04) == 0x04) pdt.bHaveF55Query19 = true;
            if ((buffer[offset] & 0x08) == 0x08) pdt.bHaveF55Ctrl25 = true;
            if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl27 = pdt.bHaveF55Query20 = true;
            if ((buffer[offset] & 0x40) == 0x40) pdt.bHaveF55Ctrl28 = pdt.bHaveF55Query21 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query22 = true;
        }
        if (pdt.bHaveF55Query19) offset++;
        if (pdt.bHaveF55Query20) offset++;
        if (pdt.bHaveF55Query21) offset++;
        if (pdt.bHaveF55Query22)
        {
            offset++;
            if ((buffer[offset] & 0x01) == 0x01) pdt.bHaveF55Ctrl29 = true;
            if ((buffer[offset] & 0x02) == 0x02) pdt.bHaveF55Query23 = true;
            if ((buffer[offset] & 0x08) == 0x08) pdt.bHaveF55Ctrl30 = true;
            if ((buffer[offset] & 0x10) == 0x10) pdt.bHaveF55Ctrl31 = true;
            if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl32 = true;
            if ((buffer[offset] & 0x40) == 0x40)
                pdt.bHaveF55Query24 = pdt.bHaveF55Query25 = pdt.bHaveF55Query26 = pdt.bHaveF55Query27 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query28 = true;
        }
        if (pdt.bHaveF55Query23) offset++;
        if (pdt.bHaveF55Query24) offset += 4;
        if (pdt.bHaveF55Query28)
        {
            offset++;
            if ((buffer[offset] & 0x20) == 0x20) pdt.bHaveF55Ctrl37 = true;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query30 = true;
        }
        if (pdt.bHaveF55Query29) offset++;
        if (pdt.bHaveF55Query30)
        {
            offset++;
            if ((buffer[offset] & 0x80) == 0x80) pdt.bHaveF55Query33 = true;
        }
        if (pdt.bHaveF55Query31) offset++;
        if (pdt.bHaveF55Query32) offset++;
        if (pdt.bHaveF55Query33)
        {
            offset++;
            pdt.bHaveF54Ctrl223 = ((buffer[query_offset] & 0x02) == 0x02);
            if (pdt.bHaveF54Ctrl223) rescanF54 = true;
            if ((buffer[offset] & 0x04) == 0x04) pdt.bHaveF55Ctrl45 = pdt.bHaveF55Ctrl46 = true;
        }

        //control register
        offset = 0;
        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.ControlBase)&0xFF), &buffer[0], 64))
			return I2C_ERR;


        if (pdt.bHaveF55Ctrl00) offset += 2;
        if (pdt.bHaveF55Ctrl03) offset++;
        if (pdt.bHaveF55Ctrl04) offset++;
        if (pdt.bHaveF55Ctrl05) offset++;
        if (pdt.bHaveF55Ctrl06) offset++;
        if (pdt.bHaveF55Ctrl07) offset++;
        if (pdt.bHaveF55Ctrl08) offset++;
        if (pdt.bHaveF55Ctrl09) offset++;
        if (pdt.bHaveF55Ctrl10) offset++;
        if (pdt.bHaveF55Ctrl11) offset++;
        if (pdt.bHaveF55Ctrl12) offset++;
        if (pdt.bHaveF55Ctrl13) offset++;
        if (pdt.bHaveF55Ctrl14) //guard pin
        {
            offset++;
            uint8_t temp[10];
            pdt.F55Ctrl14Offset = offset;

            if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F55Descriptor.ControlBase + pdt.F55Ctrl14Offset)&0xFF), &temp[0], 10))
				return I2C_ERR;
            for (i = 0; i < 4; i++)
            {
                uint8_t _offset = 2 + i;
                pdt.GuardRx[i] = temp[_offset];
                pdt.GuardTx[i] = temp[_offset + 4];
                if (pdt.GuardRx[i] != 0xFF)
                {
                    uint8_t byte = pdt.GuardRx[i] / 8;
                    uint8_t bit = pdt.GuardRx[i] % 8;
                    pdt.TRxPhysical_bit[byte] = (pdt.TRxPhysical_bit[byte] | (0x01 << bit));
                }
                if (pdt.GuardTx[i] != 0xFF)
                {
                    uint8_t byte = pdt.GuardTx[i] / 8;
                    uint8_t bit = pdt.GuardTx[i] % 8;
                    pdt.TRxPhysical_bit[byte] = (pdt.TRxPhysical_bit[byte] | (0x01 << bit));
                }
            }
        }
        if (pdt.bHaveF55Ctrl15) offset++;
        if (pdt.bHaveF55Ctrl16) offset++;
        if (pdt.bHaveF55Ctrl17) offset++;
        if (pdt.bHaveF55Ctrl18) offset++;
        if (pdt.bHaveF55Ctrl19) offset++;
        if (pdt.bHaveF55Ctrl20) offset++;
        if (pdt.bHaveF55Ctrl21) offset++;
        if (pdt.bHaveF55Ctrl22) offset++;
        if (pdt.bHaveF55Ctrl23) offset++;
        if (pdt.bHaveF55Ctrl24) offset++;
        if (pdt.bHaveF55Ctrl25) offset++;
        if (pdt.bHaveF55Ctrl26) offset++;
        if (pdt.bHaveF55Ctrl27) offset++;
        if (pdt.bHaveF55Ctrl28) offset++;
        if (pdt.bHaveF55Ctrl29) offset++;
        if (pdt.bHaveF55Ctrl30) offset++;
        if (pdt.bHaveF55Ctrl31) offset++;
        if (pdt.bHaveF55Ctrl32) offset++;
        if (pdt.bHaveF55Ctrl33) offset++;
        if (pdt.bHaveF55Ctrl34) offset++;
        if (pdt.bHaveF55Ctrl35) offset++;
        if (pdt.bHaveF55Ctrl36) offset++;
        if (pdt.bHaveF55Ctrl37) offset++;
        if (pdt.bHaveF55Ctrl38) offset++;
        if (pdt.bHaveF55Ctrl39) offset++;
        if (pdt.bHaveF55Ctrl40) offset++;
        if (pdt.bHaveF55Ctrl41) offset++;
        if (pdt.bHaveF55Ctrl42) offset++;
        if (pdt.bHaveF55Ctrl43) offset++;
        if (pdt.bHaveF55Ctrl44) offset++;
    if (pdt.bHaveF55Ctrl45) offset++;


     if (pdt.g_F21Descriptor.ID == 0x21)
    {
      for (i = 0; i < pdt.ForceTRxMAXCount; i++)
      {
        if (pdt.ForceTx[i] != 0xFF)
        {
          uint8_t byte = pdt.ForceTx[i] / 8;
          uint8_t bit = pdt.ForceTx[i] % 8;
          pdt.TRxPhysical_bit[byte] = (pdt.TRxPhysical_bit[byte] | (0x01 << bit));
        }
        if (pdt.ForceRx[i] != 0xFF)
        {
          uint8_t byte = pdt.ForceRx[i] / 8;
          uint8_t bit = pdt.ForceRx[i] % 8;
          pdt.TRxPhysical_bit[byte] = (pdt.TRxPhysical_bit[byte] | (0x01 << bit));
        }
      }
    }

		DPRINTF("##################pdt.RxChannelCount=(%d),pdt.TxChannelCount=(%d)################\n",pdt.RxChannelCount,pdt.TxChannelCount);	/* debug */
    }
    FUNC_END();
	return true;
}

void ReadF11Information()
{
    FUNC_BEGIN();
    if (pdt.g_F11Descriptor.ID == 0x11){
        if (pdt.ButtonCount > 0) {
            pdt._2DRxCount = pdt.RxChannelCount - 1;
            pdt._2DTxCount = pdt.TxChannelCount - pdt.ButtonCount;
        }
        else
        {
            pdt._2DRxCount = pdt.RxChannelCount;
            pdt._2DTxCount = pdt.TxChannelCount;
        }
    }
    FUNC_END();


}

////////////////////////////////////////////////
//bool CheckF34Command()
int CheckF34Command()
{
    unsigned char data;
    FUNC_BEGIN();

    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
		return	I2C_ERR;
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F34Descriptor.DataBase + 4)), &data, 1))
        return  I2C_ERR;

	if(DEBUG_MODE)
		printf("(%s) \n",BL7_CommandString[data]);
    if (data)
    {
        printf("Reflash Command: [%s] is not completed.\n", BL7_CommandString[data]);
    }
    FUNC_END();
    return (data == 0) ? true : false;
}

int CheckF34Status(PartitionTask *partition)
{
    bool bootloader_mode;
    bool eReturn = true;
    unsigned char data[2], dev_status, o_status, BLmode, o_string;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
		return	I2C_ERR;
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F34Descriptor.DataBase)), &data[0], 2))
		return	I2C_ERR;

    o_status = 0x1f & data[0];
    dev_status = 0x30 & data[0];
    BLmode = 0x80 & data[0];
    (BLmode == 0x80) ? (bootloader_mode = true) : (bootloader_mode = false);

    if (o_status)
    {
        eReturn = false;
        (o_status == 0x1f) ? (o_string = 10) : (o_string = o_status);
        printf("Run command: %s, Partition: %s Operation status: %x (%s),",BL7_CommandString[partition->command], GetPartitionName(partition->ID), o_status, OperationStatus[o_string]);
        printf("Devicecfg status = %s, ", Dev_ConfigStatus[dev_status]);

    }
    FUNC_END();
    return eReturn;
}
bool CheckF34Status_no_param()
{
    unsigned char data[2], dev_status, o_status, BLmode, o_string;
    bool eReturn = true;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F34Descriptor.DataBase)), &data[0], 2))
        return  I2C_ERR;

    o_status = 0x1f & data[0];
    dev_status = 0x30 & data[0];
    BLmode = 0x80 & data[0];

    if (o_status || BLmode)
    {
        (o_status == 0x1f) ? (o_string = 10):(o_string = o_status);
        (BLmode == 0x80) ? 
            printf("Device in Bootloader Mode.\n") : printf("Device in UI mode.\n");
        printf("Device Configuration status = %s\n", Dev_ConfigStatus[dev_status]);
        eReturn = false;
    }

    FUNC_END();
    return eReturn;

}

//bool CheckDevice(PartitionTask *partition)
int CheckDevice(PartitionTask *partition)
{
    //bool eReturn;
    int eReturn;
    FUNC_BEGIN();
    eReturn = CheckF34Status(partition);
    //if (!eReturn) throw eReturn;
    if ((!eReturn) || (eReturn == I2C_ERR)){
        FUNC_END();
        return eReturn;
    }
    eReturn = CheckF34Command();

		

    //if (!eReturn) throw eReturn;
    //if (!eReturn) return eReturn;

    FUNC_END();
    return eReturn;
}


//////////////
PartitionInfo * FindPartition(PartitionID id)
{
    int i =0;
    FUNC_BEGIN();
    for(i = 0; i < LIST_MAX ; i++)
    {
        if(partition_list[i].ID == id)
        {
            printf("%s: [ID:%d] partition_list index = %d \n",__func__,id, i);
            FUNC_END();
            return &partition_list[i];
        }
    }

    FUNC_END();
    return  NULL;
}

unsigned short FindPartitionLength(PartitionID id)
{

    FUNC_BEGIN();
    PartitionInfo * t = FindPartition(id);
    FUNC_END();
    if (t != NULL)
        return t->Length;
    else
        return 0;
}
///////////////////////////////////////////

void setExtendRT26result()
{
  uint8_t _byte, _bit;
    uint8_t i = 0;
    uint32_t j = 0;
  result_rt26 = true;
    FUNC_BEGIN();
  ///set ExtendRT26_pin to default value
  ExtendRT26_pin[0] = 0;
  ExtendRT26_pin[1] = 1;
  ExtendRT26_pin[2] = 32;
  ExtendRT26_pin[3] = 33;

  ///err_array should keep the data of RT26
  //for(uint8_t i = 0; i < TRX_BITMAP_LENGTH; i++)
  //  err_array[i] = 0;
  for(i = 0; i < 4; i++)
  {
    bool in_use = false;
    for (j = 0; j < TRX_MAPPING_MAX; j++){
      if (ExtendRT26_pin[i] == pdt.TRxPhysical[j])
      {
        in_use = true;
        break;
      }
    }
    ///in case the four pins are assigned as guard pin
    for (j = 0; j < 4; j++)
    {
      if ((ExtendRT26_pin[i] == pdt.GuardTx[j]) || (ExtendRT26_pin[i] == pdt.GuardRx[j]))
      {
        in_use = true;
        break;
      }
    }
    if (!in_use)
    {
      ExtendRT26_pin[i] = 0xFF;
      continue;
    }

    if(ExtendRT26_pin[i] != 0)
    {
      _byte = ExtendRT26_pin[i] / 8;
      _bit = ExtendRT26_pin[i] % 8;
    }
    else
      _byte = _bit = 0;

    err_array[_byte] |= (0x01 << _bit);
  }

  for (i = 0 ; i < TRX_BITMAP_LENGTH; i++) {
    if (err_array[i] != 0)
      result_rt26 &= false; //for case eRT_ExtendedTRexShort
    //printf("\t:TRX[%d] \n", err_array[i]);
  }
    FUNC_END();
}


// Compare Report type #4 data against test limits
int CompareHighResistance()
{
    int ret = true;
    int32_t i,j = 0;

	/* 20180510 - Only surface spec is used until now - Synaptics confirmed */

    FUNC_BEGIN();
/* //NOTEST... but this sequence can use maybe.. 
  for( i = 0; i < pdt.RxChannelCount; i++) {

    //if (RXROE[i] > HighResistanceLimit[0]) {
    if (RXROE[i] < l_limit.extended_high_regi_min) {
		printf("\tNOTEST LOW Failed:RXROE [%d]: %3.3f\n",i, RXROE[i]);
		//ret = false;
    }
    else if (RXROE[i] > l_limit.extended_high_regi_max) {
        printf("\tNOTEST HIGH Failed:RXROE [%d]: %3.3f\n",i, RXROE[i]);
        //ret = false;
    }
  }
  for( i = 0; i < pdt.TxChannelCount; i++) {
    //if (TXROE[i] > HighResistanceLimit[1]) {
    if (TXROE[i] < l_limit.extended_high_regi_min) {
		printf("\tNOTEST LOW Failed:TXROE [%d]: %3.3f\n",i, TXROE[i]);
		//ret = false;
    }
    else if (TXROE[i] > l_limit.extended_high_regi_max) {
        printf("\tNOTEST HIGH Failed:TXROE [%d]: %3.3f\n",i, TXROE[i]);
        //ret = false;
    }

  }
*/
  for (i = 0; i < pdt.TxChannelCount; i++) {
    for (j = 0; j < pdt.RxChannelCount; j++) {
      //if (ImagepF[i][j] < HighResistanceLimit[2]) {
      if (ImagepF[i][j] < l_limit.extended_high_regi_MIN[i+1][j+1]) {
		printf("\tLOW Failed: Surface, Tx [%d] Rx [%d]: %3.3f (MIN LMT:%3.3f)\n",i, j, ImagepF[i][j],l_limit.extended_high_regi_MIN[i+1][j+1]);
        ret = false;
      }
      else if (ImagepF[i][j] > l_limit.extended_high_regi_MAX[i+1][j+1]) {
        printf("\tHIGH Failed: Surface, Tx [%d] Rx [%d]: %3.3f (MAX LMT:%3.3f)\n",i, j, ImagepF[i][j],l_limit.extended_high_regi_MAX[i+1][j+1]);
        ret = false;
      }
    }
  }
    FUNC_END();
    return  ret;
}


//void CompareHybridRawCap(int32_t * p32data)
int CompareHybridRawCap(int32_t * p32data)
{
    int ret = (((true << RX_RESULT)|(true << TX_RESULT)) & 0x03);
    int32_t i;

    FUNC_BEGIN();

#if	0	/* swchoi - comment as touch table will be used */
  for (i = 0; i < (int32_t)pdt.RxChannelCount; i++,
    p32data++) {

        if(i == 0) //need debug khl
        {
            //RX00 
            if (*p32data >= l_limit.hybrid_raw_cap_rx_00_max)
            {
				ret &= ~(1 << RX_RESULT);
				printf("MAX Fail ->   Failed (D:%06d / LMT:%06ld) \n", *p32data, l_limit.hybrid_raw_cap_rx_00_max);
            }
            else if(*p32data <= l_limit.hybrid_raw_cap_rx_00_min)
            {
				ret &= ~(1 << RX_RESULT);
				printf("MIN Fail ->   Failed :RX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_rx_00_min);
            }

        }
        else
        {
            //RX01~33
            if (*p32data >= l_limit.hybrid_raw_cap_rx_other_max)
            {
				ret &= ~(1 << RX_RESULT);
			    printf("MAX Fail ->   Failed :RX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_rx_other_max);
            }
            else if(*p32data <= l_limit.hybrid_raw_cap_rx_other_min)
            {
				ret &= ~(1 << RX_RESULT);
				printf("MIN Fail ->   Failed :RX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_rx_other_min);
            }
        }
    }
    printf("\n");

  for (i = 0; i < (int32_t)pdt.TxChannelCount; i++,
    p32data++) {

      if (*p32data >= l_limit.hybrid_raw_cap_tx_max)
      {
        ret &= ~(1 << TX_RESULT); 
		printf("MAX Fail ->   Failed :TX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_tx_max);
      }
      else if (*p32data <= l_limit.hybrid_raw_cap_tx_min)
      {
		ret &= ~(1 << TX_RESULT);
		printf("MIN Fail ->   Failed :TX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_tx_min);
      }
  }
  printf("\n");
#endif	/* swchoi - end */

#if	1	/* swchoi - add as touch table should be used */
  for (i = 0; i < (int32_t)pdt.RxChannelCount; i++, p32data++) {
	if (*p32data >= l_limit.hybrid_raw_cap_rx_MAX[1][i+1])
	{
		ret &= ~(1 << RX_RESULT);
		printf("MAX Fail ->   Failed (D:%06d / LMT:%06ld) \n", *p32data, l_limit.hybrid_raw_cap_rx_MAX[1][i+1]);
	}
	else if(*p32data <= l_limit.hybrid_raw_cap_rx_MIN[1][i+1])
	{
		ret &= ~(1 << RX_RESULT);
		printf("MIN Fail ->   Failed :RX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_rx_MIN[1][i+1]);
	}
  }
    printf("\n");
  for (i = 0; i < (int32_t)pdt.TxChannelCount; i++, p32data++) {
      if (*p32data >= l_limit.hybrid_raw_cap_tx_MAX[1][i+1])
      {
        ret &= ~(1 << TX_RESULT); 
		printf("MAX Fail ->   Failed :TX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_tx_MAX[1][i+1]);
      }
      else if (*p32data <= l_limit.hybrid_raw_cap_tx_MIN[1][i+1])
      {
		ret &= ~(1 << TX_RESULT);
		printf("MIN Fail ->   Failed :TX[%2d] (D:%06d / LMT:%06ld) \n",i, *p32data, l_limit.hybrid_raw_cap_tx_MIN[1][i+1]);
      }
  }
  printf("\n");
#endif	/* swchoi - end */

    FUNC_END();
    return ret;

}




//void ReadeHybridRawCap()
int ReadeHybridRawCap()
{
  uint8_t count = (pdt.TxChannelCount + pdt.RxChannelCount + pdt.ForcePhysicalCount) * 4;
  uint16_t k = 0;
    int32_t i = 0;
    //int ret = (((true << RX_RESULT)|(true << TX_RESULT)) & 0x03);
    int ret = true;
  //int32_t *p32data;
    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)), &Data[0], count))
        return  I2C_ERR;


  //printf("Hybrid Sensing Raw Cap:\n");
  p32data_ReadeHybridRawCap = (int32_t *)&Data[k];
	if(DEBUG_MODE)
		printf("Rx(%d): ",(int32_t)pdt.RxChannelCount);
  for (i = 0; i < (int32_t)pdt.RxChannelCount; i++) {
	if(DEBUG_MODE)
		printf("%d ", *p32data_ReadeHybridRawCap);
    p32data_ReadeHybridRawCap++;
  }
	if(DEBUG_MODE)
		printf("\n");

	if(DEBUG_MODE)
		 printf("Tx(%d): ",(int32_t)pdt.TxChannelCount);
  for (i = 0; i < (int32_t)pdt.TxChannelCount; i++) {
	if(DEBUG_MODE)
		printf("%d ", *p32data_ReadeHybridRawCap);
    p32data_ReadeHybridRawCap++;
  }
	if(DEBUG_MODE)
		printf("\n");

  if (pdt.g_F21Descriptor.ID == 0x21)
  {
		if(DEBUG_MODE)
		    printf("NOTEST.. Force(%d) : ",(int32_t)pdt.ForcePhysicalCount);
		for (i = 0; i < (int32_t)pdt.ForcePhysicalCount; i++) {
			if(DEBUG_MODE)
				printf("%d ", *p32data_ReadeHybridRawCap);
			p32data_ReadeHybridRawCap++;
		}
		if(DEBUG_MODE)
			printf("\n");
  }
  p32data_ReadeHybridRawCap = (int32_t *)&Data[k];

  //ret = CompareHybridRawCap(p32data);
	//need free p32data_ReadeHybridRawCap
    FUNC_END();
    return ret;

}


//void ReadHighResistanceReport()
int ReadHighResistanceReport()
{
  int32_t k = 6;
    int8_t i,j = 0;
    int ret = true;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)));
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)), &Data[0], (2 * ( 3+ ((pdt.TxChannelCount + 1) * (pdt.RxChannelCount+1))))))


	if(DEBUG_MODE)
		printf("\nHigh Resistance Test:\n");
/*
  int16_t maxRx = (int16_t)Data[0] | ((int16_t)Data[1] << 8);
  int16_t maxTx = (int16_t)Data[2] | ((int16_t)Data[3] << 8);
  int16_t min = (int16_t)Data[4] | ((int16_t)Data[5] << 8);

  double maxRxpF = maxRx / 1000.0;
  double maxTxpF = maxTx / 1000.0;
  double minpF = min / 1000.0;
*/
	if(DEBUG_MODE)
		printf("NOTEST.. RxROE:\n");
	for(i = 0; i < pdt.RxChannelCount; i++) {
		RXROE[i] = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
		RXROE[i] = RXROE[i]/1000;
		if(DEBUG_MODE)
		    printf(" %3.3f,", RXROE[i]);
		k = k+2;
	}

	if(DEBUG_MODE)
		printf("\nNOTEST.. TxROE:\n");
	for(i = 0; i < pdt.TxChannelCount; i++) {
		TXROE[i] = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
		TXROE[i] = TXROE[i]/1000;
		if(DEBUG_MODE)
		    printf(" %3.3f,", TXROE[i]);
	    k = k+2;
	}
	if(DEBUG_MODE)
		printf("\nSurface:\n");

	 for (i = 0; i < pdt.TxChannelCount; i++){
		if(DEBUG_MODE)
			printf("Tx[%d]: ", i);
		for (j = 0; j < pdt.RxChannelCount; j++){
			Image1[i][j] = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
			ImagepF[i][j] = (float)(Image1[i][j] / 1000.0);
			if(DEBUG_MODE)
				printf(" %3.3f,", ImagepF[i][j]);
	      //printf("Image Data [%d][%d] = %d\n", i, j, Image1[i][j]);
			k = k + 2;
		}
		if(DEBUG_MODE)
			printf("\n");
	}
	ret = CompareHighResistance();

    FUNC_END();
    return ret;
  //Reset Device
  ////Reset();
}




// Compare Report type #26 data against test limits
//void CompareTRexShortTestReport()
int CompareTRexShortTestReport()
{
  int8_t k = 0;
    int32_t i = 0;
  int8_t j = 0;
    int ret = true;

    FUNC_BEGIN();
  //get only Tx and Rx pin value
  for (i = 0; i < TRX_BITMAP_LENGTH; i++){
    Data[i] &= pdt.TRxPhysical_bit[i];
    if (Data[i] != TRX_Short[i]){
      err_array[i] = Data[i] ^ TRX_Short[i];
      ret = false;
    }
  }

  for (i = 0 ; i < TRX_BITMAP_LENGTH; i++) {
    for(j = 0; j < 8; j++) {
      k = 0x01 << j;
      if (err_array[i] & k)
      {
        printf("\tFailed :TRX[%d] \n", ((i * 8) + j ));
        ret = false;
      }

    }
  }
  setExtendRT26result();
    FUNC_END();

    return ret;
}


//////////////////////////////////////////
bool ParseFlashConfig(PartitionTask *p_FC)
{
    bool eReturn = true;
    //PartitionInfo *t;
    struct sPartition
    {
        unsigned char id[2];
        unsigned char length[2];
        unsigned char start_addr[2];
        unsigned char prop[2];
    };

    unsigned char *ori_data;
    ori_data = p_FC->data;
    int count = (p_FC->TransferLen *  g_F34Query.BlockSize) / sizeof(struct sPartition);
    p_FC->data += 2;
    int i = 0;

    struct sPartition *p;
    FUNC_BEGIN();
    //for (sPartition *p = (sPartition *)(p_FC->data); i < count; p++, i++)
    for (p = (struct sPartition *)(p_FC->data); i < count; p++, i++)
    {
        unsigned short id = p->id[0] | (p->id[1] << 8);
        unsigned short length = (short)(p->length[0] | (p->length[1] << 8));
        unsigned short start_addr = p->start_addr[0] | (p->start_addr[1] << 8);
        unsigned short prop = p->prop[0] | (p->prop[1] << 8);

        if (id != 0)
        {
			if(DEBUG_MODE)
				printf("partition list Set [ID:0x%X] \n",(PartitionID)id);
            partition_list[list_count].ID = (PartitionID)id;
            partition_list[list_count].Length = length;
            partition_list[list_count].StartPhysicalBlockAddress = start_addr;
            partition_list[list_count].Properties = prop;
            list_count++;
        }
    }

    p_FC->data = ori_data;

    FUNC_END();
    return eReturn;
}


//bool BL7_ReadPartition(PartitionTask* partition)
int BL7_ReadPartition(PartitionTask* partition)
{
    int eReturn = true;
    unsigned int transation_count, remain_block;
    unsigned int offset = 0, read_len, p_len; //block
    int ret;
    unsigned int i;

    FUNC_BEGIN();
    switch (partition->ID)
    {
    case FlashConfig:
        p_len = g_F34Query.FlashConfLen;
        break;
    default:
        p_len = FindPartitionLength(partition->ID);
        break;
    }

    if (p_len == 0)
    {
        printf("%s does not exist in partition table.\n", GetPartitionName(partition->ID));
        eReturn = false;
        FUNC_END();
        return eReturn;
    }
    partition->size = p_len * g_F34Query.BlockSize;
    partition->data = (unsigned char *)malloc(p_len * g_F34Query.BlockSize);  //need test khl
    partition->command = m_Read;
    partition->BlockOffset = 0;
    partition->TransferLen = 0;

    remain_block = (p_len % g_F34Query.PayloadLen);
    transation_count = (p_len / g_F34Query.PayloadLen);
    if (remain_block > 0)
        transation_count++;

    eReturn = CheckF34Command();

    if (eReturn<= 0)
    {
        printf("%s : CheckF34Command FAIL \n",__func__);
        goto last;
    }
    // set Partition ID and Block offset
    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
    {
        eReturn = I2C_ERR;
        goto last;
    }

    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F34Descriptor.DataBase + 1)&0xFF), (unsigned char *)&partition->ID, 1))
    {
        eReturn = I2C_ERR;
        goto last;
    }

    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F34Descriptor.DataBase + 2)&0xFF), (unsigned char *)&partition->BlockOffset, 2))
    {
        eReturn = I2C_ERR;
        goto last;
    }

    for (i = 0; i < transation_count; i++)
    {
        if ((i == (transation_count -1)) && (remain_block > 0))
            partition->TransferLen = remain_block;
        else
            partition->TransferLen = g_F34Query.PayloadLen;
        //4.    Set Transfer Length
        if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F34Descriptor.DataBase + 3)&0xFF), (unsigned char *)&partition->TransferLen, 2))
		{
			eReturn = I2C_ERR;
			goto last;
	    }
        //5.    Set ‘Command’ to ‘Read’

        if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F34Descriptor.DataBase + 4)&0xFF), (unsigned char *)&partition->command, 1))
		{
			eReturn = I2C_ERR;
			goto last;
		}


        //6.    Wait for interrupt
        if(!rmi_set_page((unsigned char)(((pdt.F01_interrupt)>> 8) &0xFF)))
        {
            eReturn = I2C_ERR;
            goto last;
        }
        WaitForATTN(pdt.F01_interrupt, eF34, TIMEOUT_READ); //TIMEOUT_READ...?
        if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
        {
            eReturn = I2C_ERR;
            goto last;
        }
        read_len = partition->TransferLen * g_F34Query.BlockSize;

        if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F34Descriptor.DataBase + 5)&0xFF), &(partition->data[offset]), read_len))
        {
            eReturn = I2C_ERR;
            goto last;
        }
        offset += read_len;
    }

    ret = CheckDevice(partition);
    if(ret<=0)
    {
        printf("%s : CheckDevice Fail \n",__func__);
        if (partition->ID == FlashConfig)
        {
            printf("Read CoreConfig failed.\n");
        }

        eReturn = ret;
        goto last;
    }

    printf("Read %s successfully\n", GetPartitionName(partition->ID));
	if(DEBUG_MODE)
	{
		if (partition->ID != FlashConfig)
		{
			for (i = 0; i < offset; i++)
			{
			  printf("[%3x]%02x ",i,*(partition->data + i));
			  if (i % 16 == 15)
			    printf("\n");
			}	
		}
	}
	return eReturn;

last :
    free(partition->data);
    partition->data = NULL;
    printf("%s : free(partition->data) \n",__func__);
    FUNC_END();
    return eReturn;
}

bool ParsePartitionTable()
{
    bool ret = false;
    FUNC_BEGIN();
    //PartitionTask *partition;
    PartitionTask partition;
    partition.ID = FlashConfig;
    partition.command = m_Read;
    //if (BL7_ReadPartition(partition))
    if ((ret = BL7_ReadPartition(&partition)) > 0)
        ret = ParseFlashConfig(&partition);
        //ret = ParseFlashConfig(partition);
    else
	{
		//ret = false;
        printf("Read Flash Config failed.");
	}

    if(partition.data != NULL)
    {
        printf("%s : free(partition->data) \n",__func__);
        free(partition.data);
        partition.data = NULL;
    }
    FUNC_END();
    return ret;
}









////////////////////////////////////////////////



//void DisableCBC()
int DisableCBC()
{
  // Turn off CBC.
  uint8_t data = 0;
  uint16_t addr;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.ControlBase)>> 8) &0xFF)))
        return  I2C_ERR;

  if (pdt.bHaveF54Ctrl07) {
    addr = pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl07Offset;
    if(!i2c_general_read_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

    // data = data & 0xEF;
    data = 0;
    if(!i2c_general_write_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

  }
  else if (pdt.bHaveF54Ctrl88){
    addr = pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl88Offset;

    if(!i2c_general_read_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

    data = data & 0xDF;
    if(!i2c_general_write_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;


  }
  // Turn off CBC2
  if (pdt.bHaveF54Ctrl149)
  {
    addr = pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl149Offset;
    if(!i2c_general_read_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

    data = data & 0xFE;
    if(!i2c_general_write_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

  }
  // Turn off 0D CBC.
  if (pdt.bHaveF54Ctrl57){
    addr = pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl57Offset;
    if(!i2c_general_read_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

    data = data & 0xEF;
    //data = 0;
    if(!i2c_general_write_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

  }
    FUNC_END();
    return true;
}


//void DisableCDM(bool value = true)
int DisableCDM()
{
  uint16_t addr;
  uint8_t data = 0;
    bool value = true;
    FUNC_BEGIN();
  if(pdt.bHaveSignalClarity) 
  {
    addr = pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl41Offset;

    if(!rmi_set_page((unsigned char)(((addr)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;

    data = data | (uint8_t)value;
    //data = 1;
    if(!i2c_general_write_1BAddr((unsigned char)((addr)&0xFF), &data, 1))
        return  I2C_ERR;
  }
    FUNC_END();
    return true;
}

//bool ForceUpdate()
int ForceUpdate()
{
  uint8_t data = 0;
  uint16_t count = 0;
    FUNC_BEGIN();
  // Apply ForceUpdate.
  /* ReadRMI(pdt.g_F54Descriptor.CommandBase, &data, 1);
  data = data | 0x04;*/
  data = 4;
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.CommandBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;

  // Wait complete
  do {
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;
    //Sleep(20);
    usleep(20*1000);
    count++;
  } while (data != 0x00 && (count < DEFAULT_TIMEOUT));

  if(count >= DEFAULT_TIMEOUT) {
    printf("Timeout -- ForceUpdate can not complete\n");
    //Reset();
	SW_Reset();
    FUNC_END();
    return false;
  }
    FUNC_END();
  return true;

}

//bool ForceCal()
int ForceCal()
{
  uint8_t data = 0;
  uint16_t count = 0;
  // Apply ForceCal.
    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.CommandBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;

  data = data | 0x02;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;
  // Wait complete
  count = 0;
  do {
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;
    //Sleep(20);
    usleep(20*1000);
    count++;
  } while (data != 0x00 && (count < DEFAULT_TIMEOUT));
  printf(" \n");// without this line, this function will return false in release mode
  if(count >= DEFAULT_TIMEOUT){
    printf("Timeout -- ForceCal can not complete\n");
    //Reset();
	SW_Reset();
    FUNC_END();
    return false;
  }
    FUNC_END();
  return true;
}


////////////////////////////////////////////////
int SetReportIndex()
{
  uint8_t data[2] = {0,0};
    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + 1)&0xFF),&data[0], 2))
        return I2C_ERR;
    FUNC_END();
    return true;
}

int ReadNoiseReport()
{
  int32_t k = 0;
  int32_t i,j = 0;
  //set FIFO index
  unsigned char fifoIndex[2] = {0, 0};

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + 1)&0xFF), &fifoIndex[0], sizeof(fifoIndex)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], pdt.MaxArrayLength))
        return  I2C_ERR;

#if	0	/* debug print */
    printf("Noise Delta : \n"); //hyelim
#endif	

  for (i = 0; i < pdt.TxChannelCount; i++) {
    for (j = 0; j < pdt.RxChannelCount; j++) {
      Image1[i][j] = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
      ImagepF[i][j] = Image1[i][j];


#if	0	/* debug print */
//hyelim
        if(j == 0)
            printf("[Y%d] ",i);
        printf("(H,%0.3f/L,%0.3f)",NoiseDeltaMax[i][j], NoiseDeltaMin[i][j]);
//
#endif	

      if (ImagepF[i][j] < NoiseDeltaMin[i][j])
      {
        NoiseDeltaMin[i][j] = ImagepF[i][j];
      }
      if (ImagepF[i][j] > NoiseDeltaMax[i][j])
      {
        NoiseDeltaMax[i][j] = ImagepF[i][j];
      }
#if	0	/* debug print */
//hyelim
        printf("%0.3f(H,%0.3f/L,%0.3f), ",ImagepF[i][j],NoiseDeltaMax[i][j], NoiseDeltaMin[i][j]);
//
#endif	
      k = k + 2;
    }
#if	0	/* debug print */
    printf("\n"); //hyelim
#endif	
  }


    FUNC_END();
    return true;
}

//void ReadRawImage() 
int ReadRawImage()
{
  int32_t k = 0;
    int32_t i,j = 0;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], pdt.MaxArrayLength))
        return  I2C_ERR;
  //ReadRMI(pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFEST, &Data[0], pdt.MaxArrayLength);

  for (i = 0; i < pdt.TxChannelCount; i++){
    //printf("Tx[%d]: ", i);
    for (j = 0; j < pdt.RxChannelCount; j++){
      Image1[i][j] = ((int16_t)Data[k] | ((int16_t)Data[k + 1] << 8));
      ImageFrame[i][j] = (float)(Image1[i][j] / 1000.0);
      //printf("% 3.4f,", ImageFrame[i][j]);
      k = k + 2;
    }
    //printf("\n");
  }
    FUNC_END();
    return true;
}



//void ReadImageReport()
int ReadImageReport()
{
  int32_t k = 0;
    int32_t i,j = 0;
    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], pdt.MaxArrayLength))
        return  I2C_ERR;

  printf("Image Data : \n");

  for (i = 0; i < pdt.TxChannelCount; i++){
    printf("Tx[%d]: ", i);
    for (j = 0; j < pdt.RxChannelCount; j++){
      Image1[i][j] = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
      ImagepF[i][j] = (float)(Image1[i][j] / 1000.0);
      printf("%3.3f,", ImagepF[i][j]);
      //printf("Image Data [%d][%d] = %d\n", i, j, Image1[i][j]);
      k = k + 2;
    }
    printf("\n");
  }
  //Reset Device
  //Reset();
    FUNC_END();
    return  true;
}

//void ReadTRexShortReport()
int ReadTRexShortReport()
{
    int32_t i = 0;
    int ret = 0;

    FUNC_BEGIN();
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], TRX_BITMAP_LENGTH))
        return  I2C_ERR;

  //ReadRMI(pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFEST, &Data[0], TRX_BITMAP_LENGTH);

  for (i = 0; i < TRX_BITMAP_LENGTH; i++)   {
    printf("\tByte [%d]= %#x ", i, Data[i]);
  }
  printf("\n");
  //if(e_inputRT == eRT_RT130)
    //return;

  ret = CompareTRexShortTestReport();
  //Reset Device
  //Reset();
    FUNC_END();
    return ret;
}





/////////
//bool TestPreparation()
int TestPreparation()
{
	int ret = 0;
    FUNC_BEGIN();
  if(bRepate_times == true)
    return true;
  if(DisableCBC() == I2C_ERR)
		return I2C_ERR;
	
  // Turn off SignalClarity. ForceUpdate is required for the change to be effective
  if(DisableCDM() == I2C_ERR)
		return	I2C_ERR;

  ret = ForceUpdate();
	if(ret <=0)
		return	ret;
  if(ForceCal()==I2C_ERR)
		return	I2C_ERR;
  //if(!enterincelltestmode())
  //    return false;

  bRepate_times = true;
    FUNC_END();
  return true;
}


int CompareNoiseReport()
{
    int32_t i,j = 0;
    int ret = true;
    FUNC_BEGIN();
	if(DEBUG_MODE)
		printf("Noise Test Data : \n");
	for (i = 0; i < pdt.TxChannelCount; i++){
		if(DEBUG_MODE)
			printf("Tx[%2d]: ", i);
		for (j = 0; j < pdt.RxChannelCount; j++){
			ImagepF[i][j] = (float)(NoiseDeltaMax[i][j] - NoiseDeltaMin[i][j]);
		      //int temp = ImagepF[i][j];
			if(DEBUG_MODE)
				printf("%3d,", (int)ImagepF[i][j]);
		}	
		if(DEBUG_MODE)
			printf("\n");
		
	}

  printf("\n -  R E S U L T  - \n");

  //Compare 0D area
  for (i = 1; i <= pdt.ButtonCount; i++){
//    if ((ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount] < NoiseLimitLow[pdt.TxChannelCount - i][pdt._2DRxCount])
//      || (ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount] > NoiseLimitHigh[pdt.TxChannelCount - i][pdt._2DRxCount])){
      if (ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount] > (l_limit.noise_test_max)){ // l_limit.noise_test is MAX limit
        printf("\tHIGH Failed: Button area: TxChannel [%d] RxChannel[%d] (D:%f /MAX LMT:%d)\n",pdt.TxChannelCount-i, pdt._2DRxCount,ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount],l_limit.noise_test_max);
        //g_result = false;
        ret = false;
    }
    else if (ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount] < (l_limit.noise_test_min)){ 
        printf("\tLOW Failed: Button area: TxChannel [%d] RxChannel[%d] (D:%f /MIN LMT:%d)\n",pdt.TxChannelCount-i, pdt._2DRxCount,ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount],l_limit.noise_test_min);
        //g_result = false;
        ret = false;
    }

  }
  //Compare 2D area
  for (i = 0; i < pdt._2DTxCount; i++){
    for (j = 0; j < pdt._2DRxCount; j++){
      //if ((ImagepF[i][j] < NoiseLimitLow[i][j]) || (ImagepF[i][j] > NoiseLimitHigh[i][j])) {
      if(ImagepF[i][j] > (l_limit.noise_test_max)) { // l_limit.noise_test is MAX limit
        printf("\tHIGH Failed: 2D area: Tx [%d] Rx [%d] (D:%f / MAX LMT:%d)\n",i, j,ImagepF[i][j],l_limit.noise_test_max);
        //g_result = false;
        ret = false;
      }
      if(ImagepF[i][j] < (l_limit.noise_test_min)) {
        printf("\tLOW Failed: 2D area: Tx [%d] Rx [%d] (D:%f / MIN LMT:%d)\n",i, j,ImagepF[i][j],l_limit.noise_test_min);
        //g_result = false;
        ret = false;
      }
    }
  }
    FUNC_END();
    return ret;
}



// Compare Report type #20 data against test limits
//int CompareImageReport(double LowerLimit[TRX_MAX][TRX_MAX], double UpperLimit[TRX_MAX][TRX_MAX]  )
int CompareImageReport()
{
    int32_t i, j =0;
    int ret = true;
    FUNC_BEGIN();
  printf("\n -  R E S U L T  - \n");

  //printf("Full Raw Capacitance TEST Data : \n");
	if(DEBUG_MODE)
		printf("pdt.TxChannelCount : %d / pdt._2DRxCount : %d / pdt._2DTxCount : %d \n",pdt.TxChannelCount,pdt._2DRxCount,pdt._2DTxCount);
    printf("compare 0D area...(%d)>> \n",pdt.ButtonCount);
	if(DEBUG_MODE)
	{
	  //Compare 0D area
		for (i = 1; i <= pdt.ButtonCount; i++)
		{
		    printf("%0.4f, ",ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount]);
		    if((i%10) == 9)
		        printf("\n");
		}
		printf(" \n << \n");
	}

    for (i = 1; i <= pdt.ButtonCount; i++)
    {

		if (ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount] < l_limit.full_raw_cap_MIN[pdt.TxChannelCount - i+1][pdt._2DRxCount+1])
		{
			printf("MIN Fail.. \n");
			printf("\tFailed: Button area: Tx [%d] Rx[%d] (D:%0.4f / LMT:%0.4f)\n",pdt.TxChannelCount-i, pdt._2DRxCount,ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount], l_limit.full_raw_cap_MIN[pdt.TxChannelCount - i+1][pdt._2DRxCount+1]);
			ret = false;
		}
		else if(ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount ] > l_limit.full_raw_cap_MAX[pdt.TxChannelCount - i+1][pdt._2DRxCount+1])
		{
			printf("MAX Fail.. \n");
			printf("\tFailed: Button area: Tx [%d] Rx[%d] (D:%0.4f / LMT:%0.4f)\n",pdt.TxChannelCount-i, pdt._2DRxCount,ImagepF[pdt.TxChannelCount - i][pdt._2DRxCount], l_limit.full_raw_cap_MAX[pdt.TxChannelCount - i+1][pdt._2DRxCount+1]);
			ret = false;
		}
	}

    printf(" \n << \n");
    printf("compare 2D area... >> \n");
	if(DEBUG_MODE)
	{
		//Compare 2D area
		for (i = 0; i < pdt._2DTxCount; i++){
			for (j = 0; j < pdt._2DRxCount; j++){
				if(j == 0)
	            printf("[Y%d] ",i);
				printf("%0.4f, ",ImagepF[i][j]);
			}
		    printf("\n");
		}

	    printf(" \n << \n");
	}

  //Compare 2D area
  for (i = 0; i < pdt._2DTxCount; i++){
    for (j = 0; j < pdt._2DRxCount; j++){

      if (ImagepF[i][j] < l_limit.full_raw_cap_MIN[i+1][j+1])
        {
            printf("MIN Fail -> ");
            printf("  Failed: 2D area: Tx [%d] Rx [%d] (D:%0.4f / LMT:%0.4f) \n",i, j,ImagepF[i][j], l_limit.full_raw_cap_MIN[i+1][j+1] );
            ret = false;
        }
      else if (ImagepF[i][j] > l_limit.full_raw_cap_MAX[i+1][j+1])
        {
            printf("MAX Fail -> ");
            printf("  Failed: 2D area: Tx [%d] Rx [%d] (D:%0.4f / LMT:%0.4f) \n",i, j, ImagepF[i][j], l_limit.full_raw_cap_MAX[i+1][j+1] );
            ret = false;
        }


      }
    }

    printf(" \n << \n");
    FUNC_END();
    return  ret;
}



//int ReadReport_eRT_Normalized16BitImageReport()
int ReadReport(EReportType input)
{
  uint8_t data = 0;
  int32_t count = 0;
    int ret = 0;

    FUNC_BEGIN();

  if(SetReportIndex() < 0)
    return I2C_ERR;
  //configure device

    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.CommandBase)>> 8) &0xFF)))
        return I2C_ERR;


    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF),&data, 1))
        return I2C_ERR;


  if (data & 0x01)
  {
    printf("Getreport = 1\n");
    FUNC_END();
    return false;
  }

  // write report type
  data = (uint8_t)(input);
  if(input == eRT_AbsOpen || input == eRT_AbsShort)
    data = eRT_AbsOpenShort;

    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase)&0xFF),&data, 1))
        return  I2C_ERR;

  // issue 'GetImage' command
  data = 0x01;
	if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF),&data, 1))
        return I2C_ERR;

  do {

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.CommandBase)&0xFF),&data, 1))
        return I2C_ERR;

    data = data & 0x01;
    //Sleep(20);
    usleep(20*1000); //usleep?? msleep?? need test ..khl
    count++;
  } while (data != 0x00 && (count < 30));
  if (count == 30)
  {
    printf("Timeout -- Not supported Report Type (GetImage command didn't clean)\n");
    //Reset(); /////////// need debug khl // need dll
	SW_Reset();
    FUNC_END();
    return R_OUT;
  }


  switch (input){

      case eRT_Normalized16BitImageReport:
        ret = ReadNoiseReport();
        break;

      case eRT_FullRawCapacitance:
      case eRT_AmpAbsSensingRawCap:
        ret = ReadImageReport();
        break;
      case eRT_HybridSensingRawCap:
        ret = ReadeHybridRawCap();
        break;

      case eRT_TRexShort:
      //case eRT_RT130:
      case eRT_RT131:
        ret = ReadTRexShortReport();
        break;

      case eRT_RawImageRT3:
      case eRT_RawImageRT100:
        ret = ReadRawImage();
        break;

      case eRT_HighResistance:
        ret = ReadHighResistanceReport();
        break;

	  case eRT_GpioOpenTest:
		ret = ReadGpioOpenReport();
		break;
	
	  default:
		DERRPRINTF("Invalid type(%d)\n", input);
		return false;
		break;
    }

    FUNC_END();
    return ret;

}






/////////////////////////////////////////////////
int ScanPDT()
{
    uint8_t buffer[6];
    //uint8_t buffer[12];
    uint8_t page = 0x0;
    uint8_t pageAddress = 0x0;

    FUNC_BEGIN();

    rescanF54 = false;
    memset(&pdt.g_F01Descriptor, 0, sizeof(SFunctionDescriptor));
    memset(&pdt.g_F12Descriptor, 0, sizeof(SFunctionDescriptor));
    memset(&pdt.g_F1ADescriptor, 0, sizeof(SFunctionDescriptor));
  memset(&pdt.g_F21Descriptor, 0, sizeof(SFunctionDescriptor));
    memset(&pdt.g_F34Descriptor, 0, sizeof(SFunctionDescriptor));
    memset(&pdt.g_F54Descriptor, 0, sizeof(SFunctionDescriptor));
    memset(&pdt.g_F55Descriptor, 0, sizeof(SFunctionDescriptor));



    // Scan Page Description Table (pdt) to find all RMI functions presented by this device.
    // The Table starts at $00EE. This and every sixth register (decrementing) is a function number
    // except when this "function number" is $00, meaning end of pdt.
    // In an actual use case this scan might be done only once on first run or before compile.
    //for (uint8_t page = 0x0; page < 5; page++) {
    for (page = 0x0; page < 5; page++) {
        if(!rmi_set_page(page))
			return I2C_ERR;
	
        for (pageAddress = 0xe9; pageAddress > 0xc0; pageAddress -= 6) {

            if(!i2c_general_read_1BAddr(((unsigned char)(pageAddress&0xFF)), buffer, sizeof(buffer)))
				return I2C_ERR;

			if(DEBUG_MODE)
				printf("PAGE[0x%X] : ADDR[0x%X] : Data[0:0x%X] / Data[1:0x%X] / Data[2:0x%X] / Data[3:0x%X] / Data[4:0x%X] / Data[5:0x%X] \n", page, pageAddress,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
            usleep(50000);

            if (buffer[5] == 0x01){
                pdt.g_F01Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F01Descriptor.CommandBase =(uint16_t)((page << 8) | buffer[1]);
                pdt.g_F01Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F01Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F01Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F01Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F01Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F01Descriptor.QueryBase [0x%X] \n",pdt.g_F01Descriptor.QueryBase);
					printf("pdt.g_F01Descriptor.CommandBase [0x%X] \n",pdt.g_F01Descriptor.CommandBase);
					printf("pdt.g_F01Descriptor.ControlBase [0x%X] \n",pdt.g_F01Descriptor.ControlBase);
					printf("pdt.g_F01Descriptor.DataBase [0x%X] \n",pdt.g_F01Descriptor.DataBase);
					printf("pdt.g_F01Descriptor.Version [0x%X] \n",pdt.g_F01Descriptor.Version);
					printf("pdt.g_F01Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F01Descriptor.InterruptSourceCount);
					printf("pdt.g_F01Descriptor.ID [0x%X] \n",pdt.g_F01Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}
            }
            else if (buffer[5] == 0x11){
                pdt.g_F11Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F11Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F11Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F11Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F11Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F11Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F11Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F11Descriptor.QueryBase [0x%X] \n",pdt.g_F11Descriptor.QueryBase);
					printf("pdt.g_F11Descriptor.CommandBase [0x%X] \n",pdt.g_F11Descriptor.CommandBase);
					printf("pdt.g_F11Descriptor.ControlBase [0x%X] \n",pdt.g_F11Descriptor.ControlBase);
					printf("pdt.g_F11Descriptor.DataBase [0x%X] \n",pdt.g_F11Descriptor.DataBase);
					printf("pdt.g_F11Descriptor.Version [0x%X] \n",pdt.g_F11Descriptor.Version);
					printf("pdt.g_F11Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F11Descriptor.InterruptSourceCount);
					printf("pdt.g_F11Descriptor.ID [0x%X] \n",pdt.g_F11Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x12){
                pdt.g_F12Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F12Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F12Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F12Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F12Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F12Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F12Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F12Descriptor.QueryBase [0x%X] \n",pdt.g_F12Descriptor.QueryBase);
					printf("pdt.g_F12Descriptor.CommandBase [0x%X] \n",pdt.g_F12Descriptor.CommandBase);
					printf("pdt.g_F12Descriptor.ControlBase [0x%X] \n",pdt.g_F12Descriptor.ControlBase);
					printf("pdt.g_F12Descriptor.DataBase [0x%X] \n",pdt.g_F12Descriptor.DataBase);
					printf("pdt.g_F12Descriptor.Version [0x%X] \n",pdt.g_F12Descriptor.Version);
					printf("pdt.g_F12Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F12Descriptor.InterruptSourceCount);
					printf("pdt.g_F12Descriptor.ID [0x%X] \n",pdt.g_F12Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x1A){
                pdt.g_F1ADescriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F1ADescriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F1ADescriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F1ADescriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F1ADescriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F1ADescriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F1ADescriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F1ADescriptor.QueryBase [0x%X] \n",pdt.g_F1ADescriptor.QueryBase);
					printf("pdt.g_F1ADescriptor.CommandBase [0x%X] \n",pdt.g_F1ADescriptor.CommandBase);
					printf("pdt.g_F1ADescriptor.ControlBase [0x%X] \n",pdt.g_F1ADescriptor.ControlBase);
					printf("pdt.g_F1ADescriptor.DataBase [0x%X] \n",pdt.g_F1ADescriptor.DataBase);
					printf("pdt.g_F1ADescriptor.Version [0x%X] \n",pdt.g_F1ADescriptor.Version);
					printf("pdt.g_F01Aescriptor.InterruptSourceCount [0x%X] \n",pdt.g_F1ADescriptor.InterruptSourceCount);
					printf("pdt.g_F1ADescriptor.ID [0x%X] \n",pdt.g_F1ADescriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
			else if (buffer[5] == 0x21){
			    pdt.g_F21Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
			    pdt.g_F21Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
			    pdt.g_F21Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
			    pdt.g_F21Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
			    pdt.g_F21Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
			    pdt.g_F21Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
			    pdt.g_F21Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F21Descriptor.QueryBase [0x%X] \n",pdt.g_F21Descriptor.QueryBase);
					printf("pdt.g_F21Descriptor.CommandBase [0x%X] \n",pdt.g_F21Descriptor.CommandBase);
					printf("pdt.g_F21Descriptor.ControlBase [0x%X] \n",pdt.g_F21Descriptor.ControlBase);
					printf("pdt.g_F21Descriptor.DataBase [0x%X] \n",pdt.g_F21Descriptor.DataBase);
					printf("pdt.g_F21Descriptor.Version [0x%X] \n",pdt.g_F21Descriptor.Version);
					printf("pdt.g_F21Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F21Descriptor.InterruptSourceCount);
					printf("pdt.g_F21Descriptor.ID [0x%X] \n",pdt.g_F21Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x34){

                pdt.g_F34Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F34Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F34Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F34Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F34Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F34Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F34Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F34Descriptor.QueryBase [0x%X] \n",pdt.g_F34Descriptor.QueryBase);
					printf("pdt.g_F34Descriptor.CommandBase [0x%X] \n",pdt.g_F34Descriptor.CommandBase);
					printf("pdt.g_F34Descriptor.ControlBase [0x%X] \n",pdt.g_F34Descriptor.ControlBase);
					printf("pdt.g_F34Descriptor.DataBase [0x%X] \n",pdt.g_F34Descriptor.DataBase);
					printf("pdt.g_F34Descriptor.Version [0x%X] \n",pdt.g_F34Descriptor.Version);
					printf("pdt.g_F34Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F34Descriptor.InterruptSourceCount);
					printf("pdt.g_F34Descriptor.ID [0x%X] \n",pdt.g_F34Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x54){
                pdt.g_F54Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F54Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F54Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F54Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F54Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F54Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F54Descriptor.ID = (uint16_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F54Descriptor.QueryBase [0x%X] \n",pdt.g_F54Descriptor.QueryBase);
					printf("pdt.g_F54Descriptor.CommandBase [0x%X] \n",pdt.g_F54Descriptor.CommandBase);
					printf("pdt.g_F54Descriptor.ControlBase [0x%X] \n",pdt.g_F54Descriptor.ControlBase);
					printf("pdt.g_F54Descriptor.DataBase [0x%X] \n",pdt.g_F54Descriptor.DataBase);
					printf("pdt.g_F54Descriptor.Version [0x%X] \n",pdt.g_F54Descriptor.Version);
					printf("pdt.g_F54Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F54Descriptor.InterruptSourceCount);
					printf("pdt.g_F54Descriptor.ID [0x%X] \n",pdt.g_F54Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x55){
                pdt.g_F55Descriptor.QueryBase = (uint16_t)((page << 8) | buffer[0]);
                pdt.g_F55Descriptor.CommandBase = (uint16_t)((page << 8) | buffer[1]);
                pdt.g_F55Descriptor.ControlBase = (uint16_t)((page << 8) | buffer[2]);
                pdt.g_F55Descriptor.DataBase = (uint16_t)((page << 8) | buffer[3]);
                pdt.g_F55Descriptor.Version = (uint8_t)((buffer[4] >> 5) & 0x03);
                pdt.g_F55Descriptor.InterruptSourceCount = (uint8_t)(buffer[4] & 0x07);
                pdt.g_F55Descriptor.ID = (uint8_t)(buffer[5]);
				if(DEBUG_MODE)
				{
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
					printf("pdt.g_F55Descriptor.QueryBase [0x%X] \n",pdt.g_F55Descriptor.QueryBase);
					printf("pdt.g_F55Descriptor.CommandBase [0x%X] \n",pdt.g_F55Descriptor.CommandBase);
					printf("pdt.g_F55Descriptor.ControlBase [0x%X] \n",pdt.g_F55Descriptor.ControlBase);
					printf("pdt.g_F55Descriptor.DataBase [0x%X] \n",pdt.g_F55Descriptor.DataBase);
					printf("pdt.g_F55Descriptor.Version [0x%X] \n",pdt.g_F55Descriptor.Version);
					printf("pdt.g_F55Descriptor.InterruptSourceCount [0x%X] \n",pdt.g_F55Descriptor.InterruptSourceCount);
					printf("pdt.g_F55Descriptor.ID [0x%X] \n",pdt.g_F55Descriptor.ID);
					printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				}

            }
            else if (buffer[5] == 0x00) {
                // no function in this page, go to next page
                break;
            }

        }
    }

    if (pdt.g_F01Descriptor.ID != 0x01) {
        printf("F$01 is not found\n");
        FUNC_END();
        return false;
    }
        printf("F$01 is found\n");
    if (pdt.g_F54Descriptor.ID != 0x54){
        printf("F$54 is not found\n");
        FUNC_END();
        return false;
    }
    printf("F$54 is found\n");


    ReadF01Information();
    ReadF12Information();
    ReadF1AInformation();
	  ReadF21Information();
    ReadF34Information();
    ReadF54Information();
    if (pdt.bIsTDDIHIC) ReadF54Information();
    ReadF55Information();
    if (rescanF54) ReadF54Information();
    ReadF11Information();

/*  
#if COMMUNICATION_DEFINE == 0
    UT_PDT();
#endif
*/
    FUNC_END();
    return true;
}

/*
 * Name : FW_Version_test
 * Description : Firmware version test.
 * Parameters :
 * Return value : error
 */
int FW_Version_test(void)
{
	int result = false;

	FUNC_BEGIN();
	if(g_F34Query.BootloaderFWID == l_limit.fw_version)	/* PASS */
	{
		printf("PASS [Rd : 0x%lX / Limit : 0x%lX]\n",(long)g_F34Query.BootloaderFWID,l_limit.fw_version);
		result = true;
	}
	else	/* FAIL */
	{
		printf("FAIL [Rd : 0x%lX / Limit : 0x%lX]\n",(long)g_F34Query.BootloaderFWID,l_limit.fw_version);
		result = false;
	}

	FUNC_END();

	return result;
}


//void Attention_test()
int Attention_test(int ch_num)
{
  uint32_t ret = false;
  int err_ret = 0;
  //uint8_t data = 1, count = 0;
  uint8_t data = 1;
  int pin = 0;
  uint32_t count = 0;
	unsigned int cmd = 0;

	printf("Attention Waiting Start.. \n");
	if(ch_num == 1)
		cmd = _IOCTL_CH1_TE_GET;
	else
		cmd = _IOCTL_CH2_TE_GET;

  //clear interrupt first
    if(!rmi_set_page((unsigned char)(((pdt.F01_interrupt)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.F01_interrupt)&0xFF), &data, 1))
        return  I2C_ERR;

  //reset
  data = 1;
    if(!rmi_set_page((unsigned char)(((pdt.g_F01Descriptor.CommandBase)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F01Descriptor.CommandBase)&0xFF), &data, 1))
        return  I2C_ERR;

	usleep(100000);	/* 100ms delay - based on reference code */

	/* read interrupt status */
    if(!rmi_set_page((unsigned char)(((pdt.F01_interrupt)>> 8) &0xFF)))
        return  I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.F01_interrupt)&0xFF), &data, 1))
        return  I2C_ERR;

	pin = 0;
  do {
    count++;
	err_ret = ioctl(dic_dev, cmd,&pin);
	if (err_ret < 0)
	{
		printf("ERR: ioctl-_IOCTL_CHX_TE_GET(ch_num:%d)\n",ch_num);
	}
	printf("Attention_test:pin=(%d)\n",pin);

	usleep(50000);	/* 50ms delay */
  } while ((pin == 0) && (count < 6)); // wait for Attention for 300 ms.
	printf("Attention Waiting END.. (count: %d)\n",count);

  if(count >= 6)
  {
    ret = false;
  }
	else
		ret = true;

	return ret;
}


//void ConfigID_test()
int ConfigID_test()
{
    int i = i;
	long config_res = 0;
	int result = false;

    FUNC_BEGIN();
  if (!pdt.bBoodloaderVersion6)
  {
    printf("Not support bootloader 6 and below yet\n");
    return false;
  }

  //uint8_t config_id[32], CRC[4], data[16];
  uint8_t config_id[32], CRC[4];

  p.ID = CoreConfig;
  p.command = m_Read;

    if((result = ParsePartitionTable())<=0)
		return	result;
    if((result = BL7_ReadPartition(&p))<=0)
		return	result;


  CRC[0] = p.data[p.size -1];
  CRC[1] = p.data[p.size -2];
  CRC[2] = p.data[p.size -3];
  CRC[3] = p.data[p.size -4];

    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.ControlBase)>> 8) &0xFF)))
		return	I2C_ERR;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F34Descriptor.ControlBase)&0xFF), &config_id[0], 32))
		return	I2C_ERR;

	if(DEBUG_MODE)
	{
		printf("Config ID in hex\n");

		for(i = 0; i < 32; i++)
		{
			printf("[%2d] 0x%2x, ",i, config_id[i]);
			if ((i % 10) == 9)
				printf("\n");
		}
	}

	printf("\n -  R E S U L T  - \n");

	config_res = ((config_id[0] << 8*3) | (config_id[1] << 8*2) | (config_id[2] << 8) | (config_id[3]));
	if(config_res == l_limit.config)
	{
		printf("PASS [Rd : 0x%lX / Limit : 0x%lX]\n",config_res,l_limit.config);
		result = true;
	}
	else
	{
		printf("FAIL [Rd : 0x%lX / Limit : 0x%lX]\n",config_res,l_limit.config);
		result = false;
	}
	
	if(DEBUG_MODE)
	{
		printf("\nCRC in hex\n");

		for(i = 0; i < 4; i++)
			printf("[%d] 0x%2x, ",i, CRC[i]);
		printf("\n");
	}
  if (p.data != NULL)
  {
    printf("%s : free(p.data) \n",__func__);
    free(p.data);
    p.data = NULL;
  }
    FUNC_END();
	return	result;
}

int ExtendedTRXShortRT100Test()
{
  uint8_t data = 0;
  float baseline_image0[TRX_MAX][TRX_MAX];
  float baseline_image1[TRX_MAX][TRX_MAX];
  float delta[TRX_MAX][TRX_MAX];
  uint8_t buffer[TRX_MAPPING_MAX] = {0};
  bool do_remapping = false;
  float limit1 = 0.2;
  float limit2 = 2;
    int ret = true;
    int ret_sub = true;

    int i, j, ii, jj = 0;
    uint8_t rxIndex = 0;
    float minrx = 0;
    FUNC_BEGIN();
  memset(buffer, 0, TRX_MAPPING_MAX);
  //ExtendRT26_pin[0] = 0;
  //ExtendRT26_pin[1] = 1;
  //ExtendRT26_pin[2] = 32;
  //ExtendRT26_pin[3] = 33;
  // if the four pins in TX need to remapping as RX
  // force pin need to remapping
  ReadReport(eRT_TRexShort);
/*
  if (!Remapping(&do_remapping)) //if return data is true.. p.data need free or don't free..if it is not true.. don't need free.
    return false;
    //goto Exit;
*/
  for (i = 0; i < 4; i++)
  {
    if (ExtendRT26_pin[i] == 0xff)
      data++;
  }
  // check 0, 1, 32, 33 and force pin
  if ((data == 4) && !do_remapping)
    {
        ret = false;
        goto Exit;
    }

  // 6. Set NoScan bit  
  // data will not refresh automatically. 
  //ReadRMI(pdt.g_F54Descriptor.ControlBase, &data, 1);
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.ControlBase)>> 8) &0xFF)))
    {
        ret = I2C_ERR;
        goto Exit;
    }

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase)&0xFF), &data, 1))
    {
        ret = I2C_ERR;
        goto Exit;
    }

  data = (data | 0x02);
  //WriteRMI(pdt.g_F54Descriptor.ControlBase, &data, 1);
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase)&0xFF), &data, 1))
    {
        ret = I2C_ERR;
        goto Exit;
    }


  // 7. Save all local CBC settings
  // 8. Turn off CDM
  //DisableCDM();
  // 9. RefCaps set to -4.824 to +4.824pF
  // b. REF_HI_TRANS_CAP = 6, REF_LO_TRANS_CAP = 6
  /* uint8_t ctrl91[5];
  ReadRMI(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl91Offset, ctrl91, sizeof(ctrl91));
  ctrl91[0] = 6;
  ctrl91[1] = 6;*/
  // c. RCVR_FB_CAP = 7
  //ctrl91[2] = 7;
  //ctrl91[3] = 7;

  // 10.    Set Trans Gain to unity.
  /*ctrl91[4] = 0;
  WriteRMI(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl91Offset, ctrl91, sizeof(ctrl91));*/

  // 11. set all local CBC Rxs to 0
  //WriteRMI(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl96Offset , &buffer[0], pdt.RxChannelCount);
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl96Offset)&0xFF), &buffer[0], pdt.RxChannelCount))
    {
        ret = I2C_ERR;
        goto Exit;
    }


  //Sleep(200);
  // 12. force update
  //if (ForceUpdate()<=0){ ret = false;  goto Exit;}
//	usleep(200000); //test khl
  ret_sub = ForceUpdate();
	if(ret_sub <=0)
	{	
		ret = ret_sub;
		goto Exit;

	}

  // 13. read baseline raw image
  //Sleep(200); // set from 20 to 200 for sony case
  usleep(200*1000); // set from 20 to 200 for sony case
  ReadReport(eRT_RawImageRT100); //ImageRT100

  memcpy(baseline_image0[0], ImageFrame[0], (TRX_MAX * TRX_MAX * 4));

  for (i = 0; i < 4; i++)
  {
    bool ret2 = true, isbutton = false;
    float maxRX[TRX_MAX] = {0.0};
    if (ExtendRT26_pin[i] == 0xFF)
      continue;
    uint8_t logical_pin = GetLogicalPin(ExtendRT26_pin[i]);

    // 14. set local CBC to 8pf(2D) 3.5pf(0D)
    for(j = 0; j < pdt.ButtonCount; j++)
    {
      if(ExtendRT26_pin[i] == pdt.ButtonRx[j])
        isbutton = true;
    }
    if(isbutton)
      buffer[logical_pin] = 0x06; //button
    else
      buffer[logical_pin] = 0x0f; //EXTENDED_TRX_SHORT_CBC;

    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl96Offset)&0xFF), &buffer[0], pdt.RxChannelCount))
    {
        ret = I2C_ERR;
        goto Exit;
    }

//      return  I2C_ERR;
    //WriteRMI(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl96Offset , &buffer[0], pdt.RxChannelCount);
    buffer[logical_pin] = 0;

    // 15. force update
    //if(ForceUpdate()<=0){ ret = false;  goto Exit;} //goto Exit;
    ret_sub = ForceUpdate();
    if(ret_sub <=0)
    { 
        ret = ret_sub;
        goto Exit;

    }

    // 16. read report type 100
    usleep(200);
    ReadReport(eRT_RawImageRT100);
    memcpy(baseline_image1[0], ImageFrame[0], (TRX_MAX * TRX_MAX * 4));

    // 17. get change between image0 and baseline

	for (ii = 0; ii < pdt.TxChannelCount; ii++){
		for (jj = 0; jj < pdt.RxChannelCount; jj++){
			delta[ii][jj] = abs(baseline_image0[ii][jj] - baseline_image1[ii][jj]);
			if(DEBUG_MODE)
				printf("%2.3f - ", delta[ii][jj]);
		}
		if(DEBUG_MODE)
			  printf("\n");
    }
    //get max of each rx
    GetMaxRx((float **)delta, pdt.RxChannelCount, (float *)maxRX);

    // 18. Check data: TREX w/o CBC raised changes >= 200 or TREXn* (TREX with CBC raised) changes < 2000
    // Flag TRX0* as 1 as well if any other RX changes are >=200
    for (rxIndex = 0; rxIndex < pdt.RxChannelCount; rxIndex++)
    {
      minrx = 0;
      if (rxIndex == logical_pin)
      {
        minrx = GetMinRx((float **)delta, logical_pin, isbutton);
        if (minrx < limit2)
        {
          printf("minRX[%d] = %6.3f when test pin %d (RX Logical pin [%d])\n", rxIndex, minrx, ExtendRT26_pin[i], logical_pin);
          setErr_Array(pdt.RxPhysical[rxIndex],true);
          ret2 = false;
        }
      }
      else
      {
        if ( maxRX[rxIndex] >= limit1)
        {
          printf("maxRX[%d] = %6.3f when test pin %d (RX Logical pin [%d])\n", rxIndex, maxRX[rxIndex], ExtendRT26_pin[i], logical_pin);
          setErr_Array(pdt.RxPhysical[rxIndex],true);
          setErr_Array(ExtendRT26_pin[i],true);
          ret2 = false;
        }
      }

    }
    if (ret2)
      setErr_Array(ExtendRT26_pin[i], false);
  }


  for (i = 0 ; i < TRX_BITMAP_LENGTH; i++) {
    for(j = 0; j < 8; j++) {
      if (err_array[i] & (0x01 << j))
      {
        printf("\tFailed :TRX physical [%d] \n", ((i * 8) + j ));
        ret = false;
      }

    }
  }
/*
  if(do_remapping)
    WriteConfig(&p);
*/
Exit:
  if (p.data != NULL)
  {
    free(p.data);
    p.data = NULL;
  }

    FUNC_END();
  //PrintResult(g_result, eRT_ExtendedTRexShort);
  //Reset();
  return ret;

}



int Noise_test()
{
    int count = 0;
    int ret = 0;

    FUNC_BEGIN();
	memset(NoiseDeltaMin, 0, TRX_MAX * TRX_MAX * sizeof(double));
	memset(NoiseDeltaMax, 0, TRX_MAX * TRX_MAX * sizeof(double));

    for (count = 0; count < 20; count++){
        //ret = ReadReport_eRT_Normalized16BitImageReport();
        ret = ReadReport(eRT_Normalized16BitImageReport);
        if(ret < false)
        {
            break;
        }
    }
    if(ret >= false)
    {
		ret = CompareNoiseReport();
		if(ret == true)
		{
            printf("Noise Test Pass \n");
			ret = true;
		}
        else
		{
            printf("Noise Test Fail \n");
			ret = false;
		}
    }
    else
    {
        printf("Noise Test Fail \n");
        if(ret == I2C_ERR)
		{
            printf("I2C ERR \n");
			ret = I2C_ERR;
		}
        else if(ret == R_OUT)
		{
            printf("[Read Report Fail] \n");
			ret = false;
		}

    }

    FUNC_END();
	return ret;
}

int Full_Raw_Capacitance_test()
{
    int ret = 0;

    FUNC_BEGIN();
    if((ret = TestPreparation())<= 0)
	{
       printf("Full Raw Capacitance TEST(Disable CBC/CDM) Fail \n");
		if(ret == false)
			ret = false;
	}
    else
    {
        if(ReadReport(eRT_FullRawCapacitance) == R_OUT)
		{
            printf("Full Raw Capacitance TEST(Disable CBC/CDM) Fail [Read Report Fail]\n");
	        ret = false;
	    }
        else
        {
            ret = CompareImageReport();
            if(ret == true)
                printf("Full Raw Capacitance TEST(Disable CBC/CDM) Pass \n");
            else
            {
                printf("Full Raw Capacitance TEST(Disable CBC/CDM) Fail \n");
                if(ret == I2C_ERR)
                    printf("I2C ERR \n");
            }
        }
    }
    FUNC_END();

	return ret;
}

int Hybrid_Raw_Capacitance_Tx_Rx_test()
{
	//Tx test, Rxtest,Total 2Test
	int ret = 0;

	FUNC_BEGIN();
	if((ret= ReadReport(eRT_HybridSensingRawCap)) >0)
	{
		ret = CompareHybridRawCap(p32data_ReadeHybridRawCap);

		if(ret >= 0)
		{
			printf("Hybrid Raw Capacitance Tx/Rx test ret = 0x%X \n",ret);
		}
		else
		{
			printf("Hybrid Raw Capacitance Tx/Rx test Fail ");
			if(ret == I2C_ERR)
			{
				ret = I2C_ERR;
	            printf("I2C ERR \n");
			}
			else
			{
	            printf("[what return Data]\n");
				ret = false;
			}
		}
	}
	else
	{
            printf("Hybrid Raw Capacitance Tx/Rx test Fail ");
            if(ret == I2C_ERR)
            {
                ret = I2C_ERR;
                printf("I2C ERR \n");
            }
            else if(ret == R_OUT)
            {
                printf("[Read Report Fail]\n");
                ret = false;
            }
            else
            {
                printf("[what return Data]\n");
                ret = false;
            }
	}

	FUNC_END();
	return ret;
	


}
int Extended_TRX_Short_test(EReportType input)
{
    //Tx test, Rxtest,Total 2Test
    int ret = 0;

    FUNC_BEGIN();

	switch(input)
	{
		case eRT_ExtendedTRexShortRT100 :
			ret = ExtendedTRXShortRT100Test();	
			break;

		case eRT_ExtendHybridTXShort:
		    ret = HybridTXShort();
			break;
		case eRT_ExtendHybridTXShort2:
            ret = ExtendHybridTXShort2();
			break;
        default:
            DERRPRINTF("unsupported….input=(%d)\n",input);
            break;    
	}

    FUNC_END();
	return	ret;
}
int Extended_High_Resistance_test()
{
    int ret = 0;

    FUNC_BEGIN();

    ret = ReadReport(eRT_HighResistance);

    if(ret == true)
    {
        printf("Extended High Resistance test Pass\n");
        FUNC_END();
        return  ret;
    }
    else
    {
        printf("Extended High Resistance test Fail ");
        if(ret == I2C_ERR)
        {
            ret = I2C_ERR;
            printf("I2C ERR \n");
        }
        else if(ret == R_OUT)
        {
            printf("[Read Report Fail]\n");
            ret = false;
        }
        else
        {
            printf("[what return Data]\n");
            ret = false;
        }

        FUNC_END();
        return ret;
    }

}


/*
 * Name : SideTouchRaw_test
 * Description : Side Touch Raw Cap test for synaptics touch.
 * Parameters :
 * Return value : error
 */
int SideTouchRaw_test(void)
{
	int ret = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int length = 0;
	int sidetouch = (pdt.bHasInCellSideTouchBottom
					+ pdt.bHasInCellSideTouchLeft
					+ pdt.bHasInCellSideTouchRight
					+ pdt.bHasInCellSideTouchTop);

	FUNC_BEGIN();

	if (sidetouch == 0)
	{
		DERRPRINTF("Side Touch didn't been defined.\n");
		FUNC_END();
		return false;
	}
	DisableCDM();
	ForceUpdate();
	ForceCal();
	ReadReport(eRT_RT127);	/* TODO:Need to check because of No eRT_RT127 in ReadReport - 20180509 - swchoi */

    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
	{
		DERRPRINTF("rmi_set_page!\n");
		FUNC_END();
		return	I2C_ERR;
	}

	length = sidetouch* pdt.RxChannelCount;
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], length * 2))
	{
		DERRPRINTF("i2c_general_read_1BAddr!\n");
		FUNC_END();
		return	I2C_ERR;
	}

	for (i = 0; i < sidetouch; i++)
	{
		if(DEBUG_MODE)
			DTPRINTF("Tx[%d]: ", i);
		for (j = 0; j < pdt.RxChannelCount; j++)
		{
//			int index = (i * pdt.RxChannelCount) + j;
			ImagepF[i][j] = (float)((int16_t)Data[k] | ((int16_t)Data[k + 1] << 8)) /1000;
			if(DEBUG_MODE)
				DTPRINTF("%3.3f,", ImagepF[i][j]);
			k +=2;
		}
		if(DEBUG_MODE)
			DTPRINTF("\n");
	}
	
	for (i = 0; i < sidetouch; i++)
	{
		for (j = 0; j < pdt.RxChannelCount; j++)
		{
			if (ImagepF[i][j] > l_limit.side_touch_raw_cap_max[i][j])
			{
				DERRPRINTF("MAX Fail -> \tFailed : Tx[%d] Rx[%d]. Value[%3.3f] LMT[%3.3f]\n", i, j,ImagepF[i][j],l_limit.side_touch_raw_cap_max[i][j]);
				FUNC_END();
				ret = false;
				return ret;
			}
			else if (ImagepF[i][j] < l_limit.side_touch_raw_cap_min[i][j])
			{
                DERRPRINTF("MIN Fail -> \tFailed : Tx[%d] Rx[%d]. Value[%3.3f] LMT[%3.3f]\n", i, j,ImagepF[i][j],l_limit.side_touch_raw_cap_min[i][j]);
				FUNC_END();
                ret = false;
				return ret;
			}
			else
			{
				if (DEBUG_MODE)
				{
					DTPRINTF("Min_limit=(%3.3f),Value=(%3.3f),Max_limt=(%3.3f)\n",l_limit.side_touch_raw_cap_min[i][j],ImagepF[i][j],l_limit.side_touch_raw_cap_max[i][j]);
				}
			}
		}
	}

	FUNC_END();
	ret = true;	/* Pass */
	return ret;
}

/*
 * Name : SideTouchNoise_test
 * Description : Side Touch Noise test for synaptics touch.
 * Parameters :
 * Return value : error
 */
int SideTouchNoise_test(void)
{
	int ret = 0;
	int frame_count = 20;
	int length = 0;
	int16_t *min = NULL;
	int16_t *max = NULL;
	int i = 0;
	int j = 0;
	int sidetouch = (pdt.bHasInCellSideTouchBottom
					+ pdt.bHasInCellSideTouchLeft
					+ pdt.bHasInCellSideTouchRight
					+ pdt.bHasInCellSideTouchTop);

	FUNC_BEGIN();

	if (sidetouch == 0)
	{
		DERRPRINTF("Side Touch didn't been defined.\n");
		return false;
	}

	length = sidetouch * pdt.RxChannelCount;
	min = (int16_t *)malloc(length * 2);	/* 16bits pointer is used, so need to multiple to get memory by malloc */
	max = (int16_t *)malloc(length * 2);	/* 16bits pointer is used, so need to multiple to get memory by malloc */
	DPRINTF("sidetouch=(%d),rxchcnt=(%d),length=(%d),min=(0x%x),max=(0x%x)\n",sidetouch,pdt.RxChannelCount,length,(int)min,(int)max);
	   
	for (i = 0; i < frame_count; i++)
	{
		int k = 0;

		ReadReport(eRT_RT128);	/* TODO:Need to check because of No eRT_RT128 in ReadReport - 20180509 - swchoi */

		if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
		{
			DERRPRINTF("rmi_set_page!\n");
			free(max);
			free(min);
			FUNC_END();
			return	I2C_ERR;
		}
		if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &Data[0], length * 2))
		{
			DERRPRINTF("i2c_general_read_1BAddr!\n");
			free(max);
			free(min);
			FUNC_END();
			return	I2C_ERR;
		}

		for (j = 0; j < length; j++)
		{
			int16_t value = (int16_t)Data[k] | ((int16_t)Data[k + 1] << 8);
			k += 2;
			if (i == 0)
			{
				max[j] = min[j] = value;
			}
			else
			{
				if (min[j] > value) min[j] = value; 
				if (max[j] < value) max[j] = value;
			}
		}
	}
	
	//delta
	for (i = 0; i < sidetouch; i++)
	{
		if(DEBUG_MODE)
			DTPRINTF("Tx[%d]: ", i);
		for (j = 0; j < pdt.RxChannelCount; j++)
		{
		  	int index = (i * pdt.RxChannelCount) + j;
		  	Image1[i][j] = max[index] - min[index];
			if(DEBUG_MODE)
				DTPRINTF("%2d,", Image1[i][j]);
		}
		if(DEBUG_MODE)
			DTPRINTF("\n");
	}
	//compare
	for (i = 0; i < sidetouch; i++)
	{
		for (j = 0; j < pdt.RxChannelCount; j++)
		{
            if (Image1[i][j] > l_limit.side_touch_noise_max)
            {
                DERRPRINTF("MAX Fail -> \tFailed : Tx[%d] Rx[%d]. Value[%d] LMT[%d]\n", i, j,Image1[i][j],l_limit.side_touch_noise_max);
				free(max);
				free(min);
                ret = false;
				FUNC_END();
				return ret;
            }
            else if (Image1[i][j] < l_limit.side_touch_noise_min)
            {
                DERRPRINTF("MIN Fail -> \tFailed : Tx[%d] Rx[%d]. Value[%d] LMT[%d]\n", i, j,Image1[i][j],l_limit.side_touch_noise_min);
				free(max);
				free(min);
                ret = false;
				FUNC_END();
				return ret;
            }
			else
			{
				if (DEBUG_MODE)
				{
					DTPRINTF("Min_limit=(%d),Value=(%d),Max_limt=(%d)\n",l_limit.side_touch_noise_min,Image1[i][j],l_limit.side_touch_noise_max);
				}
			}

		}
	}

	free(max);
	free(min);

	FUNC_END();

	ret = true;	/* Pass */
	return ret;
}
////////////////////////////////////////////////////


bool init_i2c_set_slvAddr_depending_channel(int ch, int slvAddr, unsigned int *result)
{

	char i2c_dev[30]="/dev/i2c-";
	char i2c_line = 13;
    unsigned long   funcs;

	FUNC_BEGIN();

	printf("CH : %d / Slave Addr : 0x%X \n", ch, slvAddr);

	if(ch == 1)
		i2c_line = 13;
	else
		i2c_line = 9;
		

    sprintf(i2c_dev,"%s%d",i2c_dev,i2c_line);
    printf("OPEN : %s \n",i2c_dev);

    snt_dev = open(i2c_dev, O_RDWR);
    if(snt_dev < 0)
    {
        printf("I2C Device Open Failed..\n");
		*result |= (1 << TOUCH_SNT_I2C_CHECK);
		FUNC_END();
        return false;
    }

    if (ioctl(snt_dev, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter "
            "functionality matrix: %s\n", strerror(errno));
        *result |= (1 << TOUCH_SNT_I2C_CHECK);
		close(snt_dev);
		snt_dev = 0;
		FUNC_END();
            return false;
    }

    if (ioctl(snt_dev, I2C_SLAVE_FORCE, slvAddr) < 0) {
        fprintf(stderr, "Error: Could not set address[reg:0x%X] \n",slvAddr);
        *result |= (1 << TOUCH_SNT_I2C_CHECK);
		close(snt_dev);
		snt_dev = 0;
		FUNC_END();
        return false;
    }

    *result &= ~(1 << TOUCH_SNT_I2C_CHECK);

	FUNC_END();
	return true;

}

bool release_i2c_set(void)
{
	FUNC_BEGIN();

	if (snt_dev > 0)
	{
		printf("CLOSE : i2c device\n");
		close(snt_dev);
		snt_dev = 0;
	}

	FUNC_END();
	return true;
}

void init_tch_power_set(int on)
{

	int power = 0;

    FUNC_BEGIN();

	power = 0;

    ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
    ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);
    ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
    ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

    usleep(1000);
#if 0
	if(on)
	{
        power = 1;
        ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
        ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);
        ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
        ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);

        usleep(30000); //must need
		printf("Touch Power ON \n");
	}
	else
		printf("Touch Power OFF \n");
#endif
#if 1
	if(on)
	{
		usleep(200000);	// [LWG 190409] DP116 solomon need 200ms delay between reset off-on
        power = 1;
        ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
        ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
        ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);

        usleep(30000); //must need
		printf("Touch Power ON \n");
	}
	else
		printf("Touch Power OFF \n");
#endif   
	FUNC_END();

}

void init_synaptics_data_for_start_test()
{
    FUNC_BEGIN();

	int i = 0;

    // init data
    bRepate_times = false;
    result_rt26 = true;
	 memset(TRX_Short, 0, sizeof(TRX_Short));

	//struct F34_Query  init
    g_F34Query.PartitionCount = 0;
    g_F34Query.BlockSize = 0x10;

	//struct PartitionTask init
	p.data = NULL;
	p.BlockOffset = 0;
	p.TransferLen = 0;
	p.size = 0;


	//struct PDT init
    pdt.bHaveF12Ctrl20 = false;
    pdt.bHaveF12Ctrl27 = false;
    pdt.bBoodloaderVersion6 = false;
    pdt.bSignalClarityOn = false;
    pdt.bHaveF54Ctrl07 = false;
    pdt.bHaveF54Ctrl57 = false;
    pdt.bHavePixelTouchThresholdTuning = false;
    pdt.bHaveInterferenceMetric = false;
    pdt.bHaveCtrl11 = false;
    pdt.bHaveRelaxationControl = false;
    pdt.bHaveSensorAssignment = false;
    pdt.bHaveSenseFrequencyControl = false;
    pdt.bHaveIIRFilter = false;
    pdt.bHaveCmnRemoval = false;
    pdt.bHaveCmnMaximum = false;
    pdt.bHaveTouchHysteresis = false;
    pdt.bHaveEdgeCompensation = false;
    pdt.bHavePerFrequencyNoiseControl = false;
    pdt.bHaveSignalClarity = false;
    pdt.bHaveMultiMetricStateMachine = false;
    pdt.bHaveVarianceMetric = false;
    pdt.bHave0DRelaxationControl = false;
    pdt.bHave0DAcquisitionControl = false;
    pdt.bHaveSlewMetric = false;
    pdt.bHaveHBlank = false;
    pdt.bHaveVBlank = false;
    pdt.bHaveLongHBlank = false;
    pdt.bHaveNoiseMitigation2 = false;
    pdt.bHaveSlewOption = false;
    pdt.bHaveEnhancedStretch = false;
    pdt.bHaveStartupFastRelaxation = false;
    pdt.bHaveESDControl = false;
    pdt.bHaveEnergyRatioRelaxation = false;
    pdt.bHaveF54CIDIM = false;
    pdt.bHasInCellSideTouchBottom = false;
    pdt.bHasInCellSideTouchTop = false;
    pdt.bHasInCellSideTouchRight = false;
    pdt.bHasInCellSideTouchLeft = false;

    pdt.bHaveF54Data00 = false;
    pdt.bHaveF54Data01 = false;
    pdt.bHaveF54Data02 = false;
    pdt.bHaveF54Data03 = false;
    pdt.bHaveF54Data04 = false;
    pdt.bHaveF54Data05 = false;
    pdt.bHaveF54Data0601 = false;
    pdt.bHaveF54Data0602 = false;
    pdt.bHaveF54Data0701 = false;
    pdt.bHaveF54Data0702 = false;
    pdt.bHaveF54Data0801 = false;
    pdt.bHaveF54Data0802 = false;
    pdt.bHaveF54Data0901 = false;
    pdt.bHaveF54Data0902 = false;
    pdt.bHaveF54Data10 = false;
    pdt.bHaveF54Data11 = false;
    pdt.bHaveF54Data12 = false;
    pdt.bHaveF54Data13 = false;
    pdt.bHaveF54Data14 = false;
    pdt.bHaveF54Data15 = false;
    pdt.bHaveF54Data16 = false;
    pdt.bHaveF54Data17 = false;
    pdt.bHaveF54Data18 = false;
    pdt.bHaveF54Data19 = false;
    pdt.bHaveF54Data20 = false;
    pdt.bHaveF54Data21 = false;
    pdt.bHaveF54Data22 = false;
    pdt.bHaveF54Data23 = false;
    pdt.bHaveF54Data24 = false;
    pdt.bHaveF54Data25 = false;
    pdt.bHaveF54Data26 = false;
    pdt.bHaveF54Data27 = false;
    pdt.bHaveF54Data28 = false;
    pdt.bHaveF54Data29 = false;
    pdt.bHaveF54Data30 = false;

    pdt.bHaveF54Ctrl86 = false;
    pdt.bHaveF54Ctrl87 = false;
    pdt.bHaveF54Ctrl88 = false;
    pdt.bHaveF54Ctrl89 = false;
    pdt.bHaveF54Ctrl90 = false;
    pdt.bHaveF54Ctrl91 = false;
    pdt.bHaveF54Ctrl92 = false;
    pdt.bHaveF54Ctrl93 = false;
    pdt.bHaveF54Ctrl94 = false;
    pdt.bHaveF54Ctrl95 = false;
    pdt.bHaveF54Ctrl96 = false;
    pdt.bHaveF54Ctrl97 = false;
    pdt.bHaveF54Ctrl98 = false;
    pdt.bHaveF54Ctrl99 = false;
    pdt.bHaveF54Ctrl100 = false;
    pdt.bHaveF54Ctrl101 = false;
    pdt.bHaveF54Ctrl102 = false;
    pdt.bHaveF54Ctrl103 = false;
    pdt.bHaveF54Ctrl104 = false;
    pdt.bHaveF54Ctrl105 = false;
    pdt.bHaveF54Ctrl106 = false;
    pdt.bHaveF54Ctrl107 = false;
    pdt.bHaveF54Ctrl108 = false;
    pdt.bHaveF54Ctrl109 = false;
    pdt.bHaveF54Ctrl110 = false;
    pdt.bHaveF54Ctrl111 = false;
    pdt.bHaveF54Ctrl112 = false;
    pdt.bHaveF54Ctrl113 = false;
    pdt.bHaveF54Ctrl114 = false;
    pdt.bHaveF54Ctrl115 = false;
    pdt.bHaveF54Ctrl116 = false;
    pdt.bHaveF54Ctrl117 = false;
    pdt.bHaveF54Ctrl118 = false;
    pdt.bHaveF54Ctrl119 = false;
    pdt.bHaveF54Ctrl120 = false;
    pdt.bHaveF54Ctrl121 = false;
    pdt.bHaveF54Ctrl122 = false;
    pdt.bHaveF54Ctrl123 = false;
    pdt.bHaveF54Ctrl124 = false;
    pdt.bHaveF54Ctrl125 = false;
    pdt.bHaveF54Ctrl126 = false;
    pdt.bHaveF54Ctrl127 = false;
    pdt.bHaveF54Ctrl128 = false;
    pdt.bHaveF54Ctrl129 = false;
    pdt.bHaveF54Ctrl130 = false;
    pdt.bHaveF54Ctrl131 = false;
    pdt.bHaveF54Ctrl132 = false;
    pdt.bHaveF54Ctrl133 = false;
    pdt.bHaveF54Ctrl134 = false;
    pdt.bHaveF54Ctrl135 = false;
    pdt.bHaveF54Ctrl136 = false;
    pdt.bHaveF54Ctrl137 = false;
    pdt.bHaveF54Ctrl138 = false;
    pdt.bHaveF54Ctrl139 = false;
    pdt.bHaveF54Ctrl140 = false;
    pdt.bHaveF54Ctrl141 = false;
    pdt.bHaveF54Ctrl142 = false;
    pdt.bHaveF54Ctrl143 = false;
    pdt.bHaveF54Ctrl144 = false;
    pdt.bHaveF54Ctrl145 = false;
    pdt.bHaveF54Ctrl146 = false;
    pdt.bHaveF54Ctrl147 = false;
    pdt.bHaveF54Ctrl148 = false;
    pdt.bHaveF54Ctrl149 = false;
    pdt.bHaveF54Ctrl150 = false;
    pdt.bHaveF54Ctrl151 = false;
    pdt.bHaveF54Ctrl152 = false;
    pdt.bHaveF54Ctrl153 = false;
    pdt.bHaveF54Ctrl154 = false;
    pdt.bHaveF54Ctrl155 = false;
    pdt.bHaveF54Ctrl156 = false;
    pdt.bHaveF54Ctrl157 = false;
    pdt.bHaveF54Ctrl158 = false;
    pdt.bHaveF54Ctrl159 = false;
    pdt.bHaveF54Ctrl160 = false;
    pdt.bHaveF54Ctrl161 = false;
    pdt.bHaveF54Ctrl162 = false;
    pdt.bHaveF54Ctrl163 = false;
    pdt.bHaveF54Ctrl164 = false;
    pdt.bHaveF54Ctrl165 = false;
    pdt.bHaveF54Ctrl166 = false;
    pdt.bHaveF54Ctrl167 = false;
    pdt.bHaveF54Ctrl168 = false;
    pdt.bHaveF54Ctrl169 = false;
    pdt.bHaveF54Ctrl170 = false;
    pdt.bHaveF54Ctrl171 = false;
    pdt.bHaveF54Ctrl172 = false;
    pdt.bHaveF54Ctrl173 = false;
    pdt.bHaveF54Ctrl174 = false;
    pdt.bHaveF54Ctrl175 = false;
    pdt.bHaveF54Ctrl176 = false;
    pdt.bHaveF54Ctrl177 = false;
    pdt.bHaveF54Ctrl178 = false;
    pdt.bHaveF54Ctrl179 = false;
    pdt.bHaveF54Ctrl180 = false;
    pdt.bHaveF54Ctrl181 = false;
    pdt.bHaveF54Ctrl182 = false;
    pdt.bHaveF54Ctrl183 = false;
    pdt.bHaveF54Ctrl184 = false;
    pdt.bHaveF54Ctrl185 = false;
    pdt.bHaveF54Ctrl186 = false;
    pdt.bHaveF54Ctrl187 = false;
    pdt.bHaveF54Ctrl188 = false;
    pdt.bHaveF54Ctrl189 = false;
    pdt.bHaveF54Ctrl190 = false;
    pdt.bHaveF54Ctrl191 = false;
    pdt.bHaveF54Ctrl192 = false;
    pdt.bHaveF54Ctrl193 = false;
    pdt.bHaveF54Ctrl194 = false;
    pdt.bHaveF54Ctrl195 = false;
    pdt.bHaveF54Ctrl196 = false;
    pdt.bHaveF54Ctrl197 = false;
    pdt.bHaveF54Ctrl198 = false;
    pdt.bHaveF54Ctrl199 = false;
    pdt.bHaveF54Ctrl200 = false;
    pdt.bHaveF54Ctrl201 = false;
    pdt.bHaveF54Ctrl202 = false;
    pdt.bHaveF54Ctrl203 = false;
    pdt.bHaveF54Ctrl204 = false;
    pdt.bHaveF54Ctrl205 = false;
    pdt.bHaveF54Ctrl206 = false;
    pdt.bHaveF54Ctrl207 = false;
    pdt.bHaveF54Ctrl208 = false;
    pdt.bHaveF54Ctrl209 = false;
    pdt.bHaveF54Ctrl210 = false;
    pdt.bHaveF54Ctrl211 = false;
    pdt.bHaveF54Ctrl212 = false;
    pdt.bHaveF54Ctrl213 = false;
    pdt.bHaveF54Ctrl214 = false;
    pdt.bHaveF54Ctrl215 = false;
    pdt.bHaveF54Ctrl216 = false;
    pdt.bHaveF54Ctrl217 = false;
    pdt.bHaveF54Ctrl218 = false;
    pdt.bHaveF54Ctrl219 = false;
    pdt.bHaveF54Ctrl220 = false;
    pdt.bHaveF54Ctrl221 = false;
    pdt.bHaveF54Ctrl222 = false;
    pdt.bHaveF54Ctrl223 = false;
    pdt.bHaveF54Ctrl224 = false;
    pdt.bHaveF54Ctrl225 = false;
    pdt.bHaveF54Ctrl226 = false;
    pdt.bHaveF54Ctrl227 = false;
    pdt.bHaveF54Ctrl228 = false;
    pdt.bHaveF54Ctrl229 = false;
    pdt.bHaveF54Ctrl230 = false;
    pdt.bHaveF54Ctrl231 = false;
    pdt.bHaveF54Ctrl232 = false;
    pdt.bHaveF54Ctrl233 = false;
    pdt.bHaveF54Ctrl234 = false;
    pdt.bHaveF54Ctrl235 = false;
    pdt.bHaveF54Ctrl236 = false;
    pdt.bHaveF54Ctrl237 = false;
    pdt.bHaveF54Ctrl238 = false;
    pdt.bHaveF54Ctrl239 = false;
    pdt.bHaveF54Ctrl240 = false;
    pdt.bHaveF54Ctrl241 = false;
    pdt.bHaveF54Ctrl242 = false;
    pdt.bHaveF54Ctrl243 = false;
    pdt.bHaveF54Ctrl244 = false;
    pdt.bHaveF54Ctrl245 = false;
    pdt.bHaveF54Ctrl246 = false;
    pdt.bIsTDDIHIC = false;
    pdt.bHaveF54Query13 = false;
    pdt.bHaveF54Query15 = false;
    pdt.bHaveF54Query16 = false;
    pdt.bHaveF54Query17 = false;
    pdt.bHaveF54Query18 = false;
    pdt.bHaveF54Query19 = false;
    pdt.bHaveF54Query20 = false;
    pdt.bHaveF54Query21 = false;
    pdt.bHaveF54Query22 = false;
    pdt.bHaveF54Query23 = false;
    pdt.bHaveF54Query24 = false;
    pdt.bHaveF54Query25 = false;
    pdt.bHaveF54Query26 = false;
    pdt.bHaveF54Query27 = false;
    pdt.bHaveF54Query28 = false;
    pdt.bHaveF54Query29 = false;
    pdt.bHaveF54Query30 = false;
    pdt.bHaveF54Query31 = false;
    pdt.bHaveF54Query32 = false;
    pdt.bHaveF54Query33 = false;
    pdt.bHaveF54Query34 = false;
    pdt.bHaveF54Query35 = false;
    pdt.bHaveF54Query36 = false;
    pdt.bHaveF54Query37 = false;
    pdt.bHaveF54Query38 = false;
    pdt.bHaveF54Query39 = false;
    pdt.bHaveF54Query40 = false;
    pdt.bHaveF54Query41 = false;
    pdt.bHaveF54Query42 = false;
    pdt.bHaveF54Query43 = false;
    pdt.bHaveF54Query44 = false;
    pdt.bHaveF54Query45 = false;
    pdt.bHaveF54Query46 = false;
    pdt.bHaveF54Query47 = false;
    pdt.bHaveF54Query48 = false;
    pdt.bHaveF54Query49 = false;
    pdt.bHaveF54Query50 = false;
    pdt.bHaveF54Query51 = false;
    pdt.bHaveF54Query52 = false;
    pdt.bHaveF54Query53 = false;
    pdt.bHaveF54Query54 = false;
    pdt.bHaveF54Query55 = false;
    pdt.bHaveF54Query56 = false;
    pdt.bHaveF54Query57 = false;
    pdt.bHaveF54Query58 = false;
    pdt.bHaveF54Query59 = false;
    pdt.bHaveF54Query60 = false;
    pdt.bHaveF54Query61 = false;
    pdt.bHaveF54Query62 = false;
    pdt.bHaveF54Query63 = false;
    pdt.bHaveF54Query64 = false;
    pdt.bHaveF54Query65 = false;
    pdt.bHaveF54Query66 = false;
    pdt.bHaveF54Query67 = false;
    pdt.bHaveF54Query68 = false;
    pdt.bHaveF54Query69 = false;
    pdt.bHaveF54Query70 = false;
    pdt.bHaveF54Query71 = false;
    // Function 55
    pdt.bHaveF55Query03 = false;
    pdt.bHaveF55Query04 = false;
    pdt.bHaveF55Query05 = false;
    pdt.bHaveF55Query06 = false;
    pdt.bHaveF55Query07 = false;
    pdt.bHaveF55Query08 = false;
    pdt.bHaveF55Query09 = false;
    pdt.bHaveF55Query10 = false;
    pdt.bHaveF55Query11 = false;
    pdt.bHaveF55Query12 = false;
    pdt.bHaveF55Query13 = false;
    pdt.bHaveF55Query14 = false;
    pdt.bHaveF55Query15 = false;
    pdt.bHaveF55Query16 = false;
    pdt.bHaveF55Query17 = false;
    pdt.bHaveF55Query18 = false;
    pdt.bHaveF55Query19 = false;
    pdt.bHaveF55Query20 = false;
    pdt.bHaveF55Query21 = false;
    pdt.bHaveF55Query22 = false;
    pdt.bHaveF55Query23 = false;
    pdt.bHaveF55Query24 = false;
    pdt.bHaveF55Query25 = false;
    pdt.bHaveF55Query26 = false;
    pdt.bHaveF55Query27 = false;
    pdt.bHaveF55Query28 = false;
    pdt.bHaveF55Query29 = false;
    pdt.bHaveF55Query30 = false;
    pdt.bHaveF55Query31 = false;
    pdt.bHaveF55Query32 = false;
    pdt.bHaveF55Query33 = false;

    pdt.bHaveF55Ctrl00 = false;
    pdt.bHaveF55Ctrl01 = false;
    pdt.bHaveF55Ctrl02 = false;
    pdt.bHaveF55Ctrl03 = false;
    pdt.bHaveF55Ctrl04 = false;
    pdt.bHaveF55Ctrl05 = false;
    pdt.bHaveF55Ctrl06 = false;
    pdt.bHaveF55Ctrl07 = false;
    pdt.bHaveF55Ctrl08 = false;
    pdt.bHaveF55Ctrl09 = false;
    pdt.bHaveF55Ctrl10 = false;
    pdt.bHaveF55Ctrl11 = false;
    pdt.bHaveF55Ctrl12 = false;
    pdt.bHaveF55Ctrl13 = false;
    pdt.bHaveF55Ctrl14 = false;
    pdt.bHaveF55Ctrl15 = false;
    pdt.bHaveF55Ctrl16 = false;
    pdt.bHaveF55Ctrl17 = false;
    pdt.bHaveF55Ctrl18 = false;
    pdt.bHaveF55Ctrl19 = false;
    pdt.bHaveF55Ctrl20 = false;
    pdt.bHaveF55Ctrl21 = false;
    pdt.bHaveF55Ctrl22 = false;
    pdt.bHaveF55Ctrl23 = false;
    pdt.bHaveF55Ctrl24 = false;
    pdt.bHaveF55Ctrl25 = false;
    pdt.bHaveF55Ctrl26 = false;
    pdt.bHaveF55Ctrl27 = false;
    pdt.bHaveF55Ctrl28 = false;
    pdt.bHaveF55Ctrl29 = false;
    pdt.bHaveF55Ctrl30 = false;
    pdt.bHaveF55Ctrl31 = false;
    pdt.bHaveF55Ctrl32 = false;
    pdt.bHaveF55Ctrl33 = false;
    pdt.bHaveF55Ctrl34 = false;
    pdt.bHaveF55Ctrl35 = false;
    pdt.bHaveF55Ctrl36 = false;
    pdt.bHaveF55Ctrl37 = false;
    pdt.bHaveF55Ctrl38 = false;
    pdt.bHaveF55Ctrl39 = false;
    pdt.bHaveF55Ctrl40 = false;
    pdt.bHaveF55Ctrl41 = false;
    pdt.bHaveF55Ctrl42 = false;
    pdt.bHaveF55Ctrl43 = false;
    pdt.bHaveF55Ctrl44 = false;
    pdt.bHaveF55Ctrl45 = false;
    pdt.bHaveF55Ctrl46 = false;

    pdt.ButtonShared = false;
    pdt.bIncellDevice = false;

    pdt.tsvd = 0xff;
    pdt.tshd = 0xff;
    pdt.tsstb = 0xff;
    pdt.tsfst = 0xff;
    pdt.tsfrq = 0xff;
    pdt.exvcom_pin_type = 0xff;
    pdt.exvcom1 = 0xff;
    pdt.exvcom2 = 0xff;
    pdt.exvcom_sel = 0xff;
    pdt.enable_guard = 0xff;
    pdt.guard_ring = 0xff;
    pdt.enable_verf  = 0xff;
    pdt.verf = 0xff;

    pdt.RxChannelCount = 0;
    pdt.TxChannelCount = 0;
    pdt.TxPhysicalMax = 0;

    pdt.TouchControllerFamily = 0;
    pdt.CurveCompensationMode = 0;
    pdt.NumberOfSensingFrequencies = 0;
    pdt.ForcePhysicalCount = 0;
    pdt.F12Ctrl20Size = 0;
    pdt.F12Ctrl27Size = 0;
    pdt.F12Ctrl20Offset = 0;
    pdt.F12Ctrl27Offset = 0;

    pdt.F54Ctrl07Offset = 0;
    pdt.F54Ctrl08Offset = 0;
    pdt.F54Ctrl20Offset = 0;
    pdt.F54Ctrl41Offset = 0;
    pdt.F54Ctrl57Offset = 0;
    pdt.F54Ctrl86Offset = 0;
    pdt.F54Ctrl88Offset = 0;
    pdt.F54Ctrl89Offset = 0;
    pdt.F54Ctrl91Offset = 0;
    pdt.F54Ctrl95Offset = 0;
    pdt.F54Ctrl96Offset = 0;
    pdt.F54Ctrl97Offset = 0;
    pdt.F54Ctrl98Offset = 0;
    pdt.F54Ctrl99Offset = 0;
    pdt.F54Ctrl102Offset = 0;
    pdt.F54Ctrl132Offset = 0;
    pdt.F54Ctrl146Offset = 0;
    pdt.F54Ctrl147Offset = 0;
    pdt.F54Ctrl149Offset = 0;
    pdt.F54Ctrl182Offset = 0;
    pdt.F54Ctrl186Offset = 0;
    pdt.F54Ctrl188Offset = 0;
    pdt.F54Ctrl189Offset = 0;
    pdt.F54Ctrl190Offset = 0;
    pdt.F54Ctrl215Offset = 0;
    pdt.F54Ctrl225Offset = 0;
    pdt.F54Ctrl246Offset = 0;

    pdt.F54Data04Offset = 0;
    pdt.F54Data06Offset = 0;
    pdt.F54Data07Offset = 0;
    pdt.F54Data10Offset = 0;
    pdt.F54Data14Offset = 0;
    pdt.F54Data17Offset = 0;
    pdt.F54Data24Offset = 0;
    pdt.F54Data31Offset = 0;

    pdt.F55Ctrl14Offset = 0;
    pdt.F55Ctrl45Offset = 0;

    pdt._2DTxCount = 0;
    pdt._2DRxCount = 0;
	/*
    pdt.ButtonTx[8] = (0,0,0,0,0,0,0,0);
    pdt.ButtonRx[8] = (0,0,0,0,0,0,0,0);
	*/
	memset(pdt.ButtonTx,0,sizeof(pdt.ButtonTx));
	memset(pdt.ButtonRx,0,sizeof(pdt.ButtonRx));

    pdt.ButtonCount = 0;
		for (i = 0; i < 4; i++)
		{
			pdt.GuardRx[i] = 0xff;
			pdt.GuardTx[i] = 0xff;
		}
    
    for(i = 0; i < FORCE_COUNT; i++)
    {
      pdt.ForceTx[i] = 0xFF;
      pdt.ForceRx[i] = 0xFF;
    }
    pdt.F12Support = 0;
    pdt.F12ControlRegisterPresence = 0;

    pdt.MaxArrayLength = 0;
    pdt.AbsRxRefLo = 0;
    pdt.AbsTxRefLo = 0;
    pdt.F01_interrupt = 0;
    for(i = 0; i < TRX_BITMAP_LENGTH; i++)
    {
      pdt.TRxPhysical_bit[i] = 0;
    }

    //

    FUNC_END();
}

bool synaptics_ScanPDT(unsigned int *result)
{
	int ret = 0;

    FUNC_BEGIN();
	printf(" [ Scan PDT ] \n");
    ret = ScanPDT();
printf("	==================== R E S U L T ========================\n");
    if(ret <= 0)
	{
		if(ret == I2C_ERR)
		{
            printf("    > scanPDT [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_SCAN_PDT); //need modify khl
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > scanPDT [Fail] \n");
            *result |= (1 << TOUCH_SNT_SCAN_PDT); //need modify khl
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }

        else
        {

            printf("    > scanPDT what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_SCAN_PDT); //need modify khl
printf("    =======================================================\n");
            FUNC_END();
            return false;
		}
	}

    printf("	> scanPDT [Pass] \n");
	*result &= ~(1 << TOUCH_SNT_SCAN_PDT); //need modify khlh
printf("	=======================================================\n");
    FUNC_END();
	return true;
}

/*
 * Name : synaptics_FW_Version_test
 * Description : Firmware version test for synaptics touch.
 * Parameters :
 * 		unsigned int *result : firmware version test result.
 * Return value : error
 */
bool synaptics_FW_Version_test(unsigned int *result)
{
	int ret = 0;

	FUNC_BEGIN();
	
	SW_Reset();
	printf(" [ Firmware Version Test ] \n");
	ret = FW_Version_test();
	printf("    ==================== R E S U L T ========================\n");
	if (ret <= 0)
	{
		printf("    > FW_Version_test [Fail] \n");
		*result |= (1 << TOUCH_SNT_FW_VERSION);
	}
	else
	{
		printf("    >  FW_Version_test [Pass] \n");
		*result &= ~(1 << TOUCH_SNT_FW_VERSION);
	}
	printf("    =======================================================\n");
	
	FUNC_END();
	
	return ret;
}

bool synaptics_ConfigID_test(unsigned int *result)
{

    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
	printf(" [ Config ID Test ] \n");
    ret = ConfigID_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
		    printf("    > ConfigID_test [I2C Fail] \n");
		    *result |= (1 << TOUCH_SNT_CONFIG);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
		else if (ret == false)
		{
            printf("    > ConfigID_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_CONFIG);
printf("    =======================================================\n");
            FUNC_END();
            return false;
		}
        else
        {
            printf("    >  ConfigID_test what return Data [Fail]\n");
		    *result |= (1 << TOUCH_SNT_CONFIG);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  ConfigID_test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_CONFIG);
printf("    =======================================================\n");
    FUNC_END();
    return true;


}

bool synaptics_Noise_test(unsigned int *result)
{

    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Noise Test ] \n");
    ret = Noise_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Noise_test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_NOISE);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Noise_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_NOISE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Noise_test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_NOISE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Noise_test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_NOISE);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}


bool synaptics_Full_Raw_Capacitance_test(unsigned int *result)
{

    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Full Raw Capacitance Test ] \n");
    ret = Full_Raw_Capacitance_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Full_Raw_Capacitance_test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_FULL_RAW_CAP);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Full_Raw_Capacitance_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_FULL_RAW_CAP);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Full_Raw_Capacitance_test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_FULL_RAW_CAP);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Full_Raw_Capacitance_test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_FULL_RAW_CAP);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

//bool synaptics_Hybrid_Raw_Capacitance_TX_RX_test(unsigned int *result) //2 test
int synaptics_Hybrid_Raw_Capacitance_TX_RX_test(unsigned int *result) //2 test
{

    int ret = 0;
    int eReturn = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Hybrid Raw Capacitance TX/RX Test ] \n");
    ret = Hybrid_Raw_Capacitance_Tx_Rx_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret < 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Hybrid_Raw_Capacitance_TX/RX_test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_TX);
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_RX);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
        }
        else
        {
            printf("    >  Hybrid_Raw_Capacitance_TX/RX_test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_TX);
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_RX);
printf("    =======================================================\n");
        }
        FUNC_END();
        return (((~((!false)<<TX_RESULT))&(~((!false) << RX_RESULT))) | 0x03);
    }
	else
	{
		if(ret & (1<<TX_RESULT))
		{
			printf("    >  Hybrid_Raw_Capacitance_TX_test [Pass] \n");
            *result &= ~(1 << TOUCH_SNT_HYBRID_RAW_CAP_TX);
			eReturn |= (true<<TX_RESULT);
		}
		else
		{
            printf("    >  Hybrid_Raw_Capacitance_TX_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_TX);
			eReturn &= ~((!false) << TX_RESULT);
		}

        if(ret & (1<<RX_RESULT))
        {
            printf("    >  Hybrid_Raw_Capacitance_RX_test [Pass] \n");
            *result &= ~(1 << TOUCH_SNT_HYBRID_RAW_CAP_RX);
			eReturn |= (true<<RX_RESULT);
printf("    =======================================================\n");
        }
        else
        {
            printf("    >  Hybrid_Raw_Capacitance_RX_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_HYBRID_RAW_CAP_RX);
			eReturn &= ~((!false) << RX_RESULT);
printf("    =======================================================\n");
        }
	}
	
    FUNC_END();
    return eReturn;

}

//bool synaptics_Hybrid_Raw_Capacitance_TX_RX_test(unsigned int *result) //2 test
int synaptics_Extended_TRX_Short_test(unsigned int *result) //2 test
{

    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Extended TRX Short Test ] \n");
	ret	= Extended_TRX_Short_test(eRT_ExtendedTRexShortRT100);

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Extended_TRX_Short_Test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Extended_TRX_Short_Test [Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Extended_TRX_Short_Test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Extended_TRX_Short_Test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}
/*
 * Name : synaptics_SideTouchRaw_test
 * Description : Side Touch Raw Cap test for synaptics touch.
 * Parameters :
 * 		unsigned int *result : Side Touch Raw Cap test result.
 * Return value : error
 */
bool synaptics_SideTouchRaw_test(unsigned int *result)
{
	int ret = 0;

	FUNC_BEGIN();
	
	SW_Reset();
	printf(" [ Side Touch Raw Cap Test ] \n");
	ret = SideTouchRaw_test();

	printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)	/* Fail */
    {
		if(ret == I2C_ERR)	/* I2C fail */
		{
			printf("    > SideTouchRaw_test [I2C Fail] \n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_RAW_CAP);
			*result |= (1 << TOUCH_SNT_I2C_CHECK);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else if (ret == false)	/* test fail */
		{
			printf("    >  SideTouchRaw_test [Fail]\n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_RAW_CAP);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else
		{
			printf("    >  SideTouchRaw_test what return Data [Fail]\n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_RAW_CAP);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
    }

	/* Pass */
	printf("    >  SideTouchRaw_test [Pass]\n");
	*result &= ~(1 << TOUCH_SNT_SIDE_TOUCH_RAW_CAP);
	printf("    =======================================================\n");

	FUNC_END();
	return true;
}

/*
 * Name : synaptics_SideTouchNoise_test
 * Description : Side Touch Noise test for synaptics touch.
 * Parameters :
 * 		unsigned int *result : Side Touch Noise test result.
 * Return value : error
 */
bool synaptics_SideTouchNoise_test(unsigned int *result)
{
	int ret = 0;

	FUNC_BEGIN();
	
	SW_Reset();
	printf(" [ Side Touch Noise Test ] \n");
	ret = SideTouchNoise_test();

	printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)	/* Fail */
    {
		if(ret == I2C_ERR)	/* I2C fail */
		{
			printf("    > SideTouchNoise_test [I2C Fail] \n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_NOISE);
			*result |= (1 << TOUCH_SNT_I2C_CHECK);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else if (ret == false)	/* test fail */
		{
			printf("    >  SideTouchNoise_test [Fail]\n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_NOISE);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else
		{
			printf("    >  SideTouchNoise_test what return Data [Fail]\n");
			*result |= (1 << TOUCH_SNT_SIDE_TOUCH_NOISE);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
    }

	/* Pass */
	printf("    >  SideTouchNoise_test [Pass]\n");
	*result &= ~(1 << TOUCH_SNT_SIDE_TOUCH_NOISE);
	printf("    =======================================================\n");

	FUNC_END();
	return true;
}


int synaptics_Extended_High_Resistance_test(unsigned int *result) //2 test
{

    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Extended High Resistance Test ] \n");

    ret = Extended_High_Resistance_test();


printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Extended_High_Resistance_Test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_HIGH_RESISTANCE);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Extended_High_Resistance_Test [Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_HIGH_RESISTANCE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Extended_High_Resistance_Test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_EXTENDED_HIGH_RESISTANCE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Extended_High_Resistance_Test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_EXTENDED_HIGH_RESISTANCE);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

/*
 * Name : PulseReset
 * Description : Pulse reset for synaptics touch.
 * Parameters :
 * 		int ch_num : channel number (CH1 : 1, CH2 : 2)
 * 		unsigned int timeperiod : time period between reset.
 * 		bool isHICTDDI : need to understand..., maybe not used.
 * Return value : error
 */
void PulseReset(int ch_num, unsigned int timeperiod, bool isHICTDDI)
{
	int ret = 0;
	int reset_level = 0;	/* High : 1, Low : 0 */

	FUNC_BEGIN();

	/* reset pin to low */
	if (ch_num == 1)	/* CH 1 */
	{
		/* set reset pin to low */
		reset_level = 0;
		ret = ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &reset_level);
		if (ret < 0)
		{
			DERRPRINTF("ioctl-IOCTL_1CH_TOUCH_RESET\n");
		}

		usleep(timeperiod * 1000);

		/* set reset pin to high */
		reset_level = 1;
		ret = ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &reset_level);
		if (ret < 0)
		{
			DERRPRINTF("ioctl-IOCTL_1CH_TOUCH_RESET\n");
		}
	}
	else if (ch_num == 2)	/* CH 2 */
	{
		/* set reset pin to low */
		reset_level = 0;
		ret = ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &reset_level);
		if (ret < 0)
		{
			DERRPRINTF("ioctl-IOCTL_2CH_TOUCH_RESET\n");
		}
		
		usleep(timeperiod * 1000);
		
		/* set reset pin to high */
		reset_level = 1;
		ret = ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &reset_level);
		if (ret < 0)
		{
			DERRPRINTF("ioctl-IOCTL_2CH_TOUCH_RESET\n");
		}
	}


	FUNC_END();
}

/*
 * Name : Reset_pin_test
 * Description : Reset pin test for synaptics touch.
 * Parameters :
 * 		int ch_num : CH1 = 1, CH2 = 2.
 * Return value : error
 */
int Reset_pin_test(int ch_num)
{
	int count = 0;
	unsigned char data = 0;
	int time_period = 0;
	
	FUNC_BEGIN();
	/* page set */
    if(!rmi_set_page((unsigned char)(((pdt.g_F01Descriptor.ControlBase)>> 8) &0xFF)))
	{
		DERRPRINTF("rmi_set_page!\n");
		FUNC_END();
		return	I2C_ERR;
	}

	/* read */
    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase)&0xFF), &data, 1))
	{
		DERRPRINTF("i2c_general_read_1BAddr!\n");
		FUNC_END();
		return	I2C_ERR;
	}

	/* set data */
	data |= (0x01 << 7);

	/* write */
    if(!i2c_general_write_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase)&0xFF), &data, 1))
	{
		DERRPRINTF("i2c_general_write_1BAddr!\n");
		FUNC_END();
		return	I2C_ERR;
	}

	count = 0;
	do
	{
		/* read */
		if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase)&0xFF), &data, 1))
		{
			DERRPRINTF("i2c_general_read_1BAddr!\n");
			FUNC_END();
			return	I2C_ERR;
		}

		/* check data */
		if ((data & (0x01 << 7)) != 0)
		{
			if (count > 100)
			{
				printf("Cannot configure device\n");
				FUNC_END();
				return false;
			}
			count++;
			usleep(50000);	/* 50ms delay */
		}
		else
			break;
	} while (1);
	
	/* MPC03	: Connect device pin to MPC pin 14
	MPC04	: Connect device pin to MPC pin 16
	TestJig	: Connect device pin to TestJig pin N, then pulse pin N to LOW for 10 ms
	*/
	time_period = 10;	/* 10ms */
	if (pdt.bIsTDDIHIC)
		PulseReset(ch_num, time_period, true);
	else
		PulseReset(ch_num, time_period, false);
	
	//check if device has been reset 
	count = 0;
	do
	{
		/* read */
		if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F01Descriptor.ControlBase)&0xFF), &data, 1))
		{
			DERRPRINTF("i2c_general_read_1BAddr!\n");
			FUNC_END();
			return	I2C_ERR;
		}
		/* check */
		if ((data & (0x01 << 7)) != (0x01 << 7))
		{
			if (count > 100)
			{
			  printf("Time out waiting for external reset test\n");
			  FUNC_END();
			  return false;
			}
			count++;
			usleep(1000);	/* 1ms delay */
		}
		else
		{
			/* Pass */
			FUNC_END();
			return true;
		}
	} while (1);

	FUNC_END();
	return true;
}

/*
 * Name : synaptics_ResetPin_test
 * Description : Reset pin test for synaptics touch.
 * Parameters :
 * 		int ch_num : channel number to make reset.
 * 		unsigned int *result : Reset pin test result.
 * Return value : error
 */
int synaptics_ResetPin_test(int ch_num, unsigned int *test_result_p)
{
	int ret = 0;

	FUNC_BEGIN();

	SW_Reset();
	printf(" [ Reset Pin Test ] \n");

	ret = Reset_pin_test(ch_num);
	printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)	/* Fail */
    {
		if(ret == I2C_ERR)	/* I2C fail */
		{
			printf("    > Reset_pin_test [I2C Fail] \n");
			*test_result_p |= (1 << TOUCH_SNT_RESET_PIN);
			*test_result_p |= (1 << TOUCH_SNT_I2C_CHECK);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else if (ret == false)	/* test fail */
		{
			printf("    >  Reset_pin_test [Fail]\n");
			*test_result_p |= (1 << TOUCH_SNT_RESET_PIN);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
		else
		{
			printf("    >  Reset_pin_test what return Data [Fail]\n");
			*test_result_p |= (1 << TOUCH_SNT_RESET_PIN);
			printf("    =======================================================\n");
			FUNC_END();
			return false;
		}
    }

	/* Pass */
	printf("    >  Reset_pin_test [Pass]\n");
	*test_result_p &= ~(1 << TOUCH_SNT_RESET_PIN);
	printf("    =======================================================\n");

	FUNC_END();
	return true;
}


int synaptics_Attention_test(int ch_num, unsigned int *result) //2 test
{
    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Attention Test ] \n");
    ret = Attention_test(ch_num);

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Attention_Test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_ATTENTION);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Attention_Test [Fail] \n");
            //*result |= (1 << TOUCH_SNT_ATTENTION);				// [LWG] 190513 base board check
            *result &= ~(1 << TOUCH_SNT_ATTENTION);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Attention_Test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_ATTENTION);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Attention_Test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_ATTENTION);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}


int Lockdown_Check_test()
{
	unsigned char buffer;
	unsigned char lockdown = 0;
	int result = 0;
	
    FUNC_BEGIN();

    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
        return I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F34Descriptor.DataBase)&0xFF), &buffer, 1))
        return I2C_ERR;

	printf("READ Data is 0x%X \n",buffer);
	lockdown = ((buffer >> 5) & 0x03);

    printf("\n -  R E S U L T  - \n");
	printf("Lockdown Data is 0x%X \n",lockdown);

    if(lockdown == 0x02) 
    {
        printf("PASS [Rd : 0x%X / Lockdown ON[0b10] ]\n",lockdown);
        result = true;
    }
    else if(lockdown == 0x00)
    {
        printf("FAIL [Rd : 0x%X / Lockdown OFF[0b00] ]\n",lockdown);
        result = false;
    }
	else
	{
        printf("FAIL  [Rd : 0x%X / wrong data ]\n",lockdown);
        result = false;
	}


    FUNC_END();
	return result;
}

int synaptics_Lockdown_Check_test(unsigned int *result)
{
    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Lockdown Check Test ] \n");
    ret = Lockdown_Check_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Lockdown_Check_test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_LOCKDOWN_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Lockdown_Check_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_LOCKDOWN_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
             printf("    >  Lockdown_Check_test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_LOCKDOWN_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Lockdown_Check_test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_LOCKDOWN_CHECK);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

///// 180516



int PackageID_test()
{
  uint8_t data[2];
  //uint8_t data17[4];
  //uint8_t data19[2];
  uint16_t pid_dev;
	int result = false;

    if(!rmi_set_page((unsigned char)(((pdt.g_F01Descriptor.QueryBase)>> 8) &0xFF)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F01Descriptor.QueryBase + 17)&0xFF), data, 2))
        return  I2C_ERR;

	pid_dev = (data[0] | data[1] << 8);

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F01Descriptor.QueryBase + 17)&0xFF), data, 4))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F01Descriptor.QueryBase + 19)&0xFF), data, 2))
        return  I2C_ERR;

    if(DEBUG_MODE)
    {
        printf("Package ID in Device = S%d",pid_dev);
    }


	printf("\n -  R E S U L T  - \n");

    if(pid_dev == l_limit.device_package)
    {
        printf("PASS [Rd : S%d / Limit : S%ld]\n",pid_dev,l_limit.device_package);
        result = true;
    }
    else
    {
        printf("FAIL [Rd : S%d / Limit : S%ld]\n",pid_dev,l_limit.device_package);
        result = false;
    }

	return result;

}

int synaptics_Device_Package_test(unsigned int *result)
{
    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Device Package Test ] \n");
    ret = PackageID_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Device_Package_test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_DEVICE_PACKAGE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Device_Package_test [Fail] \n");
            *result |= (1 << TOUCH_SNT_DEVICE_PACKAGE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Device_Package_test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_DEVICE_PACKAGE);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Device_Package_test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_DEVICE_PACKAGE);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

int ProjectID_test(MODEL model_id)
{
	int i =0;
	int loop_cnt = 0;
	unsigned char project_id[20] = {0,};
    int ret = true;	/* true == Pass */
	int test_result = 0xffffffff;	/* if each bit is true, Pass */

    FUNC_BEGIN();

  if (!pdt.bBoodloaderVersion6)
  {
    printf("Not support bootloader 6 and below yet\n");
    return false;
  }

  p.ID = CustomerSerialization;
  p.command = m_Read;

    if((ret = ParsePartitionTable())<=0)
        return  ret;
    if((ret = BL7_ReadPartition(&p))<=0)
        return  ret;
/*
    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.DataBase)>> 8) &0xFF)))
        return I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F34Descriptor.DataBase+5)&0xFF), (unsigned char *)project_id, l_limit.project_id_len))
        return I2C_ERR;
*/
	printf("\n -  R E S U L T  - \n");

	if (l_limit.project_id_len > 0)
	{
		if (model_id == STORM)
		{
			for (loop_cnt = 0;loop_cnt < STORM_TOUCH_PRODUCT_ID_LOOP_CNT;loop_cnt++)
			{
				for(i = 0; i < l_limit.project_id_len;i++)
				{
					project_id[i] = p.data[i + (loop_cnt * STORM_TOUCH_PRODUCT_ID_TEMP_OFFSET)];
				}

				printf("[PROJECT ID] Read :[%s] / LMT :[%s] (len:%d)\n",project_id, l_limit.project_id,l_limit.project_id_len);

				for(i = 0; i < l_limit.project_id_len; i++)
				{
					if(project_id[i] != l_limit.project_id[i])
					{
						printf("Fail> %dth word Not matching [Rd:%c / LMT:%c] \n",i+1,project_id[i],l_limit.project_id[i]);
						test_result &= ~(1 << loop_cnt);
					}
				}
			}
			if (((test_result & STORM_TOUCH_PRODUCT_ID_ADDRESS_0_BIT) == 0) && ((test_result & STORM_TOUCH_PRODUCT_ID_ADDRESS_1_BIT) == 0))
			{
				ret = false;
			}
		}
		else
		{
			for(i = 0; i < l_limit.project_id_len;i++)
			{
				project_id[i] = p.data[i];
			}

			printf("[PROJECT ID] Read :[%s] / LMT :[%s] (len:%d)\n",project_id, l_limit.project_id,l_limit.project_id_len);

			for(i = 0; i < l_limit.project_id_len; i++)
			{
				if(project_id[i] != l_limit.project_id[i])
				{
					printf("Fail> %dth word Not matching [Rd:%c / LMT:%c] \n",i+1,project_id[i],l_limit.project_id[i]);
					ret = false;
				}
			}
		}
	}
	else	/* No limit spec, so test should be failed */
	{
		printf("Fail> Limit length is 0, looks like no PROJECT_ID limit in spec \n");
		ret = false;
	}

	if (p.data != NULL)
	{
		printf("%s : free(p.data) \n",__func__);
		free(p.data);
		p.data = NULL;
	}

    FUNC_END();
    return ret;
}

int synaptics_ProjectID_test(MODEL model_id,unsigned int *result)
{
    int ret = 0;
    FUNC_BEGIN();

	SW_Reset();
    printf(" [ Project ID Test ] \n");
    ret = ProjectID_test(model_id);

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Project ID [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_PROJECT_ID);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Project ID [Fail] \n");
            *result |= (1 << TOUCH_SNT_PROJECT_ID);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Project ID what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_PROJECT_ID);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Project ID [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_PROJECT_ID);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}


//void HybridTXShort()
int HybridTXShort()
{
	int32_t CbcStepupTestValue = 1;
	int32_t imageA[TRX_MAPPING_MAX];
	int32_t imageB[TRX_MAPPING_MAX];
	int32_t read_len = (pdt.TxChannelCount + pdt.RxChannelCount) * 4;
	uint8_t trx_count = pdt.maxRxCount + pdt.maxTxCount;
	uint8_t *hCBC = (uint8_t *)malloc(trx_count);
	int32_t LIMIT1 = 1000;
	int32_t LIMIT2 = 180;
	uint8_t *result = (uint8_t *)malloc(pdt.TxChannelCount);
	int ret = true;
	int i,j = 0;

	for(i = 0; i < pdt.TxChannelCount; i++) result[i] = 0;
  //TX
  //2.	Write 0x02 to eF54 CTRL147 (see FWINCELL-404) to disable Hybrid CBC Auto Servo. ForceUpdate.
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.ControlBase)>> 8) &0xFF)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl147Offset)&0xFF), &Data[0], 1))
        return  I2C_ERR;
	Data[0] |= 0x02;
    if(!i2c_general_write_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl147Offset)&0xFF), &Data[0], 1))
        return  I2C_ERR; 
	ForceUpdate();
  //Sleep(50);
	usleep(50);
  //3.	Read and store all Hybrid TREX’s CBC settings for Tx
	if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl190Offset)&0xFF), &hCBC[0], trx_count))
        return  I2C_ERR;

	//4.	A: Read and record initial RT63 data
	ReadReport(eRT_HybridSensingRawCap);

	memcpy((uint8_t*)imageA, &Data[0], read_len);
    printf("\n -  R E S U L T  - \n");
	printf("[MIN LMT(if Tx=subTx) :0x%X] [MAX LMT :0x%X]\n",LIMIT1+1,LIMIT2-1);
    printf("Tx is Y(row), subTx is X(cal) \n");
	for(i = 0; i < pdt.TxChannelCount; i++)
	{
	    uint8_t c_index = pdt.maxRxCount + i;
	    uint8_t cbc = hCBC[c_index];
	    hCBC[c_index] += CbcStepupTestValue;
	    if(!i2c_general_write_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl190Offset)&0xFF), &hCBC[0], trx_count))
		    return  I2C_ERR;
	    ForceUpdate();
    //Sleep(50);
	    usleep(50);
		ReadReport(eRT_HybridSensingRawCap);

		memcpy((uint8_t*)imageB, &Data[0], read_len);
		hCBC[c_index] = cbc;
		if(DEBUG_MODE)
			printf("TX[%d]\n",i);
		//store min delta
		for(j = 0; j < pdt.TxChannelCount; j++)
		{
			uint8_t r_index = pdt.RxChannelCount + j;
			uint32_t delta = abs(imageA[r_index] - imageB[r_index]);
			if(DEBUG_MODE)
			    printf("%d - ", delta);

		    if(i == j)
			{
				if(delta <= LIMIT1)
				{
					printf("\nTX[%d] -> [subTx %d]LOW FAIL(if Tx=subTx) [Delta : 0x%X (MIN LMT:0x%X)] \n",i,j,delta,LIMIT1+1);
					ret = false;
					result[i] |= 1;
					break;
				}
				else
					continue;
			}

			if(delta >= LIMIT2)
			{
					printf("\nTX[%d] -> [subTx %d]HIGH FAIL [Delta : 0x%X (MAX LMT:0x%X)] \n",i,j,delta,LIMIT2-1);
				ret = false;
				result[i] |= 1;
				break;
			}
			else
				result[i] |= 0;
		}
		printf("\n");
	}
	if(DEBUG_MODE)
	{
		for(i = 0; i < pdt.TxChannelCount; i++)
		{
			if(result[i])
			printf("TX[%2d] - fail, ",i );
		}
	}
	printf("\n");

	if(ret == true)
		printf("Extended TRX Short TEST Pass \n");
	else
		printf("Extended TRX Short TEST Fail \n");
	
	free(hCBC);
	free(result);
	return ret;
}

//void ExtendHybridTXShort2()
int ExtendHybridTXShort2()
{
  const uint8_t CbcStepupTestValue = 1;
  const uint8_t CoolDownTime = 50;
  const uint32_t CbcSetDeltaLimit = 800;
  const uint32_t ShortDeltaLimit = 180;

  int32_t imageA[TRX_MAPPING_MAX];
  int32_t imageB[TRX_MAPPING_MAX];
  uint8_t trx_count = pdt.maxRxCount + pdt.maxTxCount;
  uint8_t *hCBC = (uint8_t *)malloc(trx_count);
/*
  int32_t *cbcDeltaResults = (int32_t *)malloc(pdt.maxTxCount);
  int32_t *shortedResults = (int32_t *)malloc(pdt.maxTxCount);
*/
  int32_t *cbcDeltaResults = (int32_t *)malloc(pdt.maxTxCount * sizeof(int32_t));
  int32_t *shortedResults = (int32_t *)malloc(pdt.maxTxCount * sizeof(int32_t));

  int32_t read_len = (pdt.TxChannelCount + pdt.RxChannelCount) * 4;
	int i,j = 0;
	int ret = true;
  for(i = 0; i < pdt.maxTxCount; i++)
  {
    cbcDeltaResults[i] = 0;
    shortedResults[i] = 0;
  }
  //DisableCbcAutoCorrection
    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.ControlBase)>> 8) &0xFF)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl147Offset)&0xFF), &Data[0], 1))
        return  I2C_ERR;

  Data[0] |= 0x02; 
    if(!i2c_general_write_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl147Offset)&0xFF), &Data[0], 1))
        return  I2C_ERR;

  //Sleep(CoolDownTime);
  usleep(CoolDownTime);
  ForceUpdate();
  //Sleep(CoolDownTime);
  usleep(CoolDownTime);
    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl190Offset)&0xFF), &hCBC[0], trx_count))
        return  I2C_ERR;
  // Step 2 - Save the base image - AKA baselineImage
  ReadReport(eRT_HybridSensingRawCap);
  memcpy((uint8_t*)imageA, &Data[0], read_len);

  for(i = 0; i < pdt.TxChannelCount; i++)
  {
    uint8_t c_index = pdt.maxRxCount + i;
    uint8_t cbc = hCBC[c_index];

    if (hCBC[c_index] == 15) hCBC[c_index] += 8;
    else if (hCBC[c_index] == 31) hCBC[c_index] -= 1;
    else hCBC[c_index] += CbcStepupTestValue;
    if(!i2c_general_write_1BAddr(((unsigned char)(pdt.g_F54Descriptor.ControlBase + pdt.F54Ctrl190Offset)&0xFF),  &hCBC[0], trx_count))
        return  I2C_ERR;

    ForceUpdate();
    //Sleep(CoolDownTime);
    usleep(CoolDownTime);
    ReadReport(eRT_HybridSensingRawCap);
    memcpy((uint8_t*)imageB, &Data[0], read_len);
    //StoreHighestDeltaWithCbcSetPixel
    uint8_t r_index = pdt.RxChannelCount + i;
    uint32_t delta = abs(imageA[r_index] - imageB[r_index]);
    if (cbcDeltaResults[i] < delta)
    {
      cbcDeltaResults[i] = delta;
    }
	if(DEBUG_MODE)
		printf("[TX%d] [CBCSET Delta : 0x%X(MIN LMT:0x%X)] ",i,cbcDeltaResults[i],CbcSetDeltaLimit);

    for(j = 0; j < pdt.TxChannelCount; j++)
    {
      r_index = pdt.RxChannelCount + j;
      delta = abs(imageA[r_index] - imageB[r_index]);
      if(i == j) continue; 
      if (shortedResults[i] < delta) shortedResults[i] = delta;
    }
	if(DEBUG_MODE)
		printf("[SHORTED Delta : 0x%X(MAX LMT:0x%X)] \n",shortedResults[i],ShortDeltaLimit);
    hCBC[c_index] = cbc;
  }
  for(i = 0; i < pdt.TxChannelCount; i++)
  {
    if (!((cbcDeltaResults[i] > CbcSetDeltaLimit) && (shortedResults[i] < ShortDeltaLimit)))
    {
      ret = false;
      printf("TX[%2d] - fail : [CBCSET Delta : 0x%X(MIN LMT:0x%X)] [SHORTED Delta : 0x%X(MAX LMT:0x%X)], \n",i,cbcDeltaResults[i],CbcSetDeltaLimit,shortedResults[i],ShortDeltaLimit);
    }
  }
  printf("\n");
    if(ret == true)
        printf("Extended TRX Short TEST Pass \n");
    else
        printf("Extended TRX Short TEST Fail \n");

  free(hCBC);
  free(cbcDeltaResults);
  free(shortedResults);
	return ret;
}


int synaptics_Extended_TRX_Short_test_mode_select(int mode, unsigned int *result)
{

    int ret = 0;
    FUNC_BEGIN();

    SW_Reset();
	printf(" [ Extended TRX Short Test ] \n");

	switch(mode)
	{

		case 1:
		    printf(" [ Mode 1 ] \n");
		    ret = Extended_TRX_Short_test(eRT_ExtendedTRexShortRT100);
			break;

		case 2:
		    printf(" [ Mode 2 ] \n");
		    ret = Extended_TRX_Short_test(eRT_ExtendHybridTXShort);
			break;

        case 3:
            printf(" [ Mode 3 ] \n");
		    ret = Extended_TRX_Short_test(eRT_ExtendHybridTXShort2);
            break;

		default:
	        DERRPRINTF("Invalid mode(%d)\n", mode);
		    ret = false;
			break;
	}

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Extended_TRX_Short_Test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > Extended_TRX_Short_Test [Fail] \n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  Extended_TRX_Short_Test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  Extended_TRX_Short_Test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_EXTENDED_TRX_SHORT);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

int TouchOTP_test()
{
    int i = 0;
	int ret = 0;
	int result = (((true << OTP_WRITE_RESULT)|(true << OTP_COUNT_RESULT)) & 0x03);
	static unsigned char	for_otp[((PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK+OTP_TOTAL_PART_TOTAL_BLOCK)*ONE_BLOCK_TOTAL_BYTE)+32] = {0,};
	static unsigned char	for_otp_buf[OTP_TOTAL_PART][OTP_ONE_PART_TOTAL_BLOCK][ONE_BLOCK_TOTAL_BYTE] = {{{0,},},};
    FUNC_BEGIN();
  if (!pdt.bBoodloaderVersion6)
  {
    printf("Not support bootloader 6 and below yet\n");
    return false;
  }

  memset(for_otp,0,sizeof(for_otp));
  memset(for_otp_buf,0,sizeof(for_otp_buf));
  //uint8_t config_id[32], CRC[4], data[16];
  //uint8_t config_id[32], CRC[4];
  uint8_t CRC[4];

  p.ID = CustomerSerialization;
  p.command = m_Read;

    if((ret = ParsePartitionTable())<=0)
		return	ret;
	CheckF34Status_no_param();

    if((ret = BL7_ReadPartition(&p))<=0)
		return	ret;


    if(DEBUG_MODE)
    {
        printf("\nCRC in hex\n");

        for(i = 0; i < 4; i++)
            printf("[%d] 0x%2x, ",i, CRC[i]);
        printf("\n");
    }
//// if do not this sequence.. program shutdown.. need test [unhandled level 3 translation fault]
    if(!rmi_set_page((unsigned char)(((pdt.g_F34Descriptor.ControlBase)>> 8) &0xFF)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr((unsigned char)((pdt.g_F34Descriptor.ControlBase)&0xFF), &for_otp[0], ((PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK+OTP_TOTAL_PART_TOTAL_BLOCK)*ONE_BLOCK_TOTAL_BYTE)))
        return  I2C_ERR;
////
    printf("++++++++++++++++++++++\n");
    printf("OTP in hex\n");
	int byte_count = 0;
	int block_count = 0;
	int otp_part = 0;

    for(i = 0 ;i<(PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK*ONE_BLOCK_TOTAL_BYTE);i++)
	{
		//printf("[%3x]%02X ",i, for_otp[i]);
		printf("[%3x]%02X ",i, p.data[i]);
        if ((i % 0x10) == 0xF)
            printf("\n");
	}

	otp_part = 1;	/* initial value is 1 */

    for(i = (PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK*ONE_BLOCK_TOTAL_BYTE); i < ((PRODUCT_ID_TOTAL_PART_TOTAL_BLOCK+OTP_TOTAL_PART_TOTAL_BLOCK)*ONE_BLOCK_TOTAL_BYTE); i++)
    { //for_otp_buf
		if((byte_count != 0) && ((byte_count % ONE_BLOCK_TOTAL_BYTE) == 0))
		{
			block_count++;
			byte_count = 0;
		}
		if((block_count != 0) && ((block_count % OTP_ONE_PART_TOTAL_BLOCK) == 0))
		{
			otp_part++;
			printf("\n--> %d PART (3BLOCK) <--\n",otp_part);
			block_count = 0;
			byte_count = 0;
		}
		else if ((otp_part == 1) && (block_count == 0) && (byte_count == 0))	/* only for debug print */
		{
			printf("\n--> %d PART (3BLOCK) <--\n",otp_part);
		}

		//for_otp_buf[otp_part-1][block_count][byte_count] = for_otp[i];
		for_otp_buf[otp_part-1][block_count][byte_count] = p.data[i];

		if(!for_otp_buf[otp_part-1][block_count][byte_count])
	        printf("[%3x]%02X ",i, p.data[i]);
	        //printf("[%3x]%02X ",i, for_otp[i]);
		else if(for_otp_buf[otp_part-1][block_count][byte_count] == 0xFF)
			printf("[%3x]\033[1;31m%02X\033[m ",i, p.data[i]); //red
			//printf("[%3x]\033[1;31m%02X\033[m ",i, for_otp[i]); //red
		else
			printf("[%3x]\033[0;34m%02X\033[m ",i, p.data[i]); //blue
			//printf("[%3x]\033[0;34m%02X\033[m ",i, for_otp[i]); //blue

#if	0	/* swchoi - add to debug - data compare to make sure */
		if(!for_otp_buf[otp_part-1][block_count][byte_count])
	        printf("[%3x]%02X:%02X ",i,for_otp_buf[otp_part-1][block_count][byte_count], p.data[i]);
		else if(for_otp_buf[otp_part-1][block_count][byte_count] == 0xFF)
			printf("[%3x]%02X:\033[1;31m%02X\033[m ",i,for_otp_buf[otp_part-1][block_count][byte_count], p.data[i]); //red
		else
			printf("[%3x]%02X:\033[0;34m%02X\033[m ",i,for_otp_buf[otp_part-1][block_count][byte_count], p.data[i]); //blue
#endif

        if ((i % 0x10) == 0xF)
		{
            printf("\n");
		}
		byte_count++;
    }

	int otp_write_flag[OTP_TOTAL_PART] = {0,};
	int otp_no_write_flag = 1;
	int otp_count = 0;

	for(otp_part = 0; otp_part<OTP_TOTAL_PART; otp_part++)
	{
		for(block_count = 0; block_count < OTP_ONE_PART_TOTAL_BLOCK; block_count++)
		{
			for(byte_count = 0; byte_count < ONE_BLOCK_TOTAL_BYTE; byte_count++)
			{
				
				if(for_otp_buf[otp_part][block_count][byte_count] != 0x00)
				{
					otp_write_flag[otp_part] = 1;
					otp_no_write_flag = 0;
					otp_count++;
					printf("OTP PART %d Write (1~6)\n",otp_part+1);
					//printf("OTP PART %d Write (1~6) [%3x]%02X\n",otp_part+1,((otp_part*OTP_ONE_PART_TOTAL_BLOCK*ONE_BLOCK_TOTAL_BYTE)+(blcok_count*ONE_BLOCK_TOTAL_BYTE)+byte_count),[otp_part][block_count][byte_count]);
					break;
				}

			}
			if(otp_write_flag[otp_part] == 1)
				break;
		}
	}

	printf("\n -  R E S U L T  - \n");
	if(otp_no_write_flag)
	{
		printf("[OTP NO Write!] \n");
		result &= ~(1 << OTP_WRITE_RESULT);
		result &= ~(1 << OTP_COUNT_RESULT); //temp
	}
	else 
	{
		printf("[OTP %d Write!] \n",otp_count);
	}


  if (p.data != NULL)
  {
    printf("%s : free(p.data) \n",__func__);
    free(p.data);
    p.data = NULL;
  }
    FUNC_END();
	return	result;
}



int synaptics_TouchOTP_test(unsigned int *result) //temp
{
    int ret = 0;
    int eReturn = 0;
    FUNC_BEGIN();
	SW_Reset();
    printf(" [ Touch OTP Write / Count Test ] \n");
    ret = TouchOTP_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret < 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > Touch OTP Write/Count [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_OTP_WRITE);
            *result |= (1 << TOUCH_SNT_OTP_COUNT_CHECK);
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
printf("    =======================================================\n");
        }
        else
        {
            printf("    >  Touch OTP Write/Count what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_OTP_WRITE);
            *result |= (1 << TOUCH_SNT_OTP_COUNT_CHECK);
printf("    =======================================================\n");
        }
        FUNC_END();
        return (((~((!false)<<OTP_WRITE_RESULT))&(~((!false) << OTP_COUNT_RESULT))) | 0x03);
    }
	else
	{
		if(ret & (1<<OTP_WRITE_RESULT))
		{
			printf("    >  Touch OTP Write [Pass] \n");
            *result &= ~(1 << TOUCH_SNT_OTP_WRITE);
			eReturn |= (true<<OTP_WRITE_RESULT);
		}
		else
		{
            printf("    > Touch OTP Write [Fail] \n");
            *result |= (1 << TOUCH_SNT_OTP_WRITE);
			eReturn &= ~((!false) << OTP_WRITE_RESULT);
		}

        if(ret & (1<<OTP_COUNT_RESULT))
        {
			printf("    >  Touch OTP Count [Pass] \n");
            *result &= ~(1 << TOUCH_SNT_OTP_COUNT_CHECK);
			eReturn |= (true<<OTP_COUNT_RESULT);
printf("    =======================================================\n");
        }
        else
        {
            printf("    > Touch OTP Count [Fail] \n");
            *result |= (1 << TOUCH_SNT_OTP_COUNT_CHECK);
			eReturn &= ~((!false) << OTP_COUNT_RESULT);
printf("    =======================================================\n");
        }
	}

    FUNC_END();
    return eReturn;

}

int ReadGpioOpenReport()
{
    FUNC_BEGIN();

    if(!rmi_set_page((unsigned char)(((pdt.g_F54Descriptor.DataBase)>> 8) &0xFF)))
        return  I2C_ERR;

    if(!i2c_general_read_1BAddr(((unsigned char)(pdt.g_F54Descriptor.DataBase + REPORT_DATA_OFFSET)&0xFF), &GpioOpenData[0], 1))
        return  I2C_ERR;

	printf("%s:Result : 0x%X\n",__func__,GpioOpenData[0]);
	return true;
    FUNC_END();

}

int	PA2_GND_test()
{
    FUNC_BEGIN();
	int ret = false;
	GpioOpenData[0] = 0;

	ret = ReadReport(eRT_GpioOpenTest);
	//use pin 2
	if(ret == I2C_ERR)
	{
		printf("I2C_ERR \n");
		return ret;
	}
	else if(ret == R_OUT)
	{
		printf("Read Report Fail \n");
		return false;
	}
	else if ((GpioOpenData[0] & 0x04) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}

    FUNC_END();
	return ret;

}
int synaptics_PA2_GND_test(unsigned int *result)
{
    int ret = 0;
    FUNC_BEGIN();

    SW_Reset();
    printf(" [ PA2 GND Test ] \n");
    ret = PA2_GND_test();

printf("    ==================== R E S U L T ========================\n");
    if(ret <= 0)
    {
        if(ret == I2C_ERR)
        {
            printf("    > PA2 GND Test [I2C Fail] \n");
            *result |= (1 << TOUCH_SNT_I2C_CHECK);
            *result |= (1 << TOUCH_SNT_PA2_GND);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else if (ret == false)
        {
            printf("    > PA2 GND Test [Fail] \n");
            *result |= (1 << TOUCH_SNT_PA2_GND);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
        else
        {
            printf("    >  PA2 GND Test what return Data [Fail]\n");
            *result |= (1 << TOUCH_SNT_PA2_GND);
printf("    =======================================================\n");
            FUNC_END();
            return false;
        }
    }

    printf("    >  PA2 GND Test [Pass] \n");
    *result &= ~(1 << TOUCH_SNT_PA2_GND);
printf("    =======================================================\n");
    FUNC_END();
    return true;

}

