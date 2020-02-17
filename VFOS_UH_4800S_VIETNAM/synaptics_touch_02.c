#include <stdbool.h>
#include <stdio.h>  
#include <sys/types.h>  
#include <linux/types.h> 
#include <math.h>
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
#include <synaptics_touch_02.h>
#include <unistd.h>
#include <touch_comm_lib.h>

//#ifndef __packed
#define __packed __attribute__((packed))
//#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef __signed__ char __s8;                                     
typedef unsigned char __u8;    
typedef __signed__ short __s16;                            
typedef unsigned short __u16;    
typedef __signed__ int __s32;    
typedef unsigned int __u32;    

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;

//// SPI FUNCTION ADD

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if 0
typedef unsigned char	u8;
typedef char			s8;
typedef unsigned short	u16;
typedef short			s16;
typedef unsigned int	u32;
typedef int				s32;
#endif
#define	SPI_ADDR_W		0xF0
#define SPI_ADDR_R		0xF1

#define _IOCTL_GOODIX_START				0x1130
#define _IOCTL_GOODIX_READ				0x1131
#define	_IOCTL_GOODIX_CS_LOW			0x1132
#define	_IOCTL_GOODIX_CS_HIGH			0x1133

extern int dic_dev;

#define SPI_FUNC_DEBUG		0
void spi_func(u8* w_buf, u8* r_buf, u16 buf_len)
{
	int i, arg;

	ioctl(dic_dev, _IOCTL_GOODIX_CS_LOW, (unsigned long)&arg);
	for( i = 0; i < buf_len; i++ ){
		arg = w_buf[i];
		ioctl(dic_dev, _IOCTL_GOODIX_READ, (unsigned long)&arg);

		if(SPI_FUNC_DEBUG){
			printf("[%d:0x%02X]", i, arg);

			if(i%6 == 5)
				printf("\n");
		}
		r_buf[i] = arg;
	}

	if(SPI_FUNC_DEBUG)
		printf("\n");
	
	ioctl(dic_dev, _IOCTL_GOODIX_CS_HIGH, (unsigned long)&arg);

	return 1;
}

// not used
void init_tch_power_set_for_dp116_synap(int on)
{
	int power = 0;

	FUNC_BEGIN();

	//DPRINTF("dic_dev is %d\n",dic_dev);
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
		power = 1;
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);

		power = 0;
		ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

		usleep(200000);
		power = 1;
		ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

		usleep(30000); //must need
		printf("Touch Power ON \n");
	}
	else
		printf("Touch Power OFF \n");
#endif   
	FUNC_END();

}

extern int mux_device;	// solomon(atmel) already open mux_device

static struct synaptics_02_touch_limit l_limit;		// local limit : use this

int synaptics_02_touch_limit_table_parser(MODEL id, char *m_name, struct synaptics_02_touch_limit* limit)
{
	char string[500];
	FILE *fp;
	char *token = NULL;
	char file_name[50];
	char name_buf[30]={0,};
	int row = 0;
	int parsing_en = 0;
	int x = 0, y =0;
	int mode = 0;
	FUNC_BEGIN();

	sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_TABLE_FILE);

	if(id != limit->id)
		limit->id = id;

	if((fp=(fopen(file_name,"r"))) == 0 ){
		printf("%s : cannot open %s\n", __func__, file_name);
		FUNC_END();
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

				if(!strcmp(token, "X"))
				{
					token = strtok(NULL, TOKEN_SEP_COMMA);
					x = strtoul(token, NULL,10);
					if(!strcmp(strtok(NULL, TOKEN_SEP_COMMA),"Y"))
					{
						token = strtok(NULL, TOKEN_SEP_COMMA);
						y = strtoul(token, NULL,10);
					}
				}

				switch(mode)
				{	
					case 0:
						if(!strcmp(token, "fw_ver"))
							mode = 1;
						else if(!strcmp(token, "config_ver"))
							mode = 2;
						else if(!strcmp(token, "product_id"))
							mode = 3;
						else if(!strcmp(token, "Full_Raw_Capacitance_MIN"))
							mode = 4;
						else if(!strcmp(token, "Full_Raw_Capacitance_MAX"))
							mode = 5;
						else if(!strcmp(token, "Extended_High_Regi_MIN"))
							mode = 6;	
						else if(!strcmp(token, "Extended_High_Regi_MAX"))
							mode = 7;
						else if(!strcmp(token, "Hybrid_Raw_Cap_TX_MIN"))
							mode = 8;
						else if(!strcmp(token, "Hybrid_Raw_Cap_TX_MAX"))
							mode = 9;
						else if(!strcmp(token, "Hybrid_Raw_Cap_RX_MIN"))
							mode = 10;
						else if(!strcmp(token, "Hybrid_Raw_Cap_RX_MAX"))
							mode = 11;
						else if(!strcmp(token, "Customer_ID_Check"))
							mode = 12;
						else if(!strcmp(token, "cm_jitter_MIN"))
							mode = 13;
						else if(!strcmp(token, "cm_jitter_MAX"))
							mode = 14;
						else if(!strcmp(token, "sensor_MIN"))
							mode = 15;
						else if(!strcmp(token, "sensor_MAX"))
							mode = 16;
						else if(!strcmp(token, "avdd_MAX"))
							mode = 17;
						else if(!strcmp(token, "dvdd_MAX"))
							mode = 18;
						else if(!strcmp(token, "ADC_RANGE_MIN"))
							mode = 19;
						else if(!strcmp(token, "ADC_RANGE_MAX"))
							mode = 20;
						else if(!strcmp(token, "Full_Raw_Capacitance_Slop_H"))
							mode = 21;
						else if(!strcmp(token, "Full_Raw_Capacitance_Slop_V"))
							mode = 22;
						else if(!strcmp(token, "avdd_MIN"))
							mode = 23;
						else if(!strcmp(token, "dvdd_MIN"))
							mode = 24;
						break;

					case 1:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->fw_ver))
							mode = 0; 
						break;
					case 2:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->config_ver))
							mode = 0; 
						break;
					case 3:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->product_id))
							mode = 0; 
						break;
					case 4:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->full_raw_cap_MIN))
							mode = 0; 
						break;
					case 5:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->full_raw_cap_MAX))
							mode = 0; 
						break;
					case 6:
						if(synaptics_02_touch_csv_parser3(&x, &y, &row, token, limit->extend_high_regi_MIN))
							mode = 0; 
						break;
					case 7:
						if(synaptics_02_touch_csv_parser3(&x, &y, &row, token, limit->extend_high_regi_MAX))
							mode = 0; 
						break;
					case 8:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->hybrid_raw_cap_TX_MIN))
							mode = 0; 
						break;
					case 9:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->hybrid_raw_cap_TX_MAX))
							mode = 0; 
						break;
					case 10:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->hybrid_raw_cap_RX_MIN))
							mode = 0; 
						break;
					case 11:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->hybrid_raw_cap_RX_MAX))
							mode = 0; 
						break;
					case 12:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->customer_id))
							mode = 0; 
						break;
					case 13:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->cm_jitter_MIN))
							mode = 0; 
						break;
					case 14:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->cm_jitter_MAX))
							mode = 0; 
						break;
					case 15:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->sensor_MIN))
							mode = 0; 
						break;
					case 16:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->sensor_MAX))
							mode = 0; 
						break;
					case 17:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->avdd_MAX))
							mode = 0; 
						break;
					case 18:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->dvdd_MAX))
							mode = 0; 
						break;
					case 19:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->ADC_RANGE_MIN))
							mode = 0; 
						break;
					case 20:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->ADC_RANGE_MAX))
							mode = 0; 
						break;
					case 21:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->full_raw_cap_h))
							mode = 0; 
						break;
					case 22:
						if(synaptics_02_touch_csv_parser(&x, &y, &row, token, limit->full_raw_cap_v))
							mode = 0; 
						break;
					case 23:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->avdd_MIN))
							mode = 0;
						break;
					case 24:
						if(synaptics_02_touch_csv_parser2(&x, &y, &row, token, limit->dvdd_MIN))
							mode = 0;
						break;
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


int synaptics_02_touch_csv_parser(int *xx, int *yy, int *row, char *token, long limit_array[][300])
{
	int i = 0, j =0;
	int n = 0;
	int flag = 0;
	int row_result = 0;
	int x, y;
	x = *xx;
	y = *yy;
	row_result = *row;

	if(!strcmp(token,"S"))
	{
		for(n = 0; n < x+1 ; n++) //1 : x
		{
			token = strtok(NULL, TOKEN_SEP_COMMA);
			if(!strcmp(token,"E"))
			{
				row_result++;
				if(row_result > y-1) //2 :y
				{
					row_result = 0;
					flag = 1;

					if(DEBUG_MODE){
						for(i = 0; i< y; i++)
						{
							for(j = 0; j<x ;j++)
							{
								//printf(" %0.4f",limit_array[i][j]);
								//printf(" %ld",limit_array[i][j]);
							}
							//printf("\n");
						}
					}

				}
				break;
			}
			limit_array[row_result][n] = strtod(token,NULL);
		}
	}
	*row = row_result;
	return flag;
}

int synaptics_02_touch_csv_parser2(int *xx, int *yy, int *row, char *token, char *limit_array)
{
//	char test_array[10]={0,};
	char test_array[20]={0,};
	char * p = test_array;
	if(!strcmp(token,"S"))
	{
		p = strtok(NULL, TOKEN_SEP_COMMA);
		memcpy(limit_array,p,sizeof(test_array));
	}
	return TRUE;
}

int synaptics_02_touch_csv_parser3(int *xx, int *yy, int *row, char *token, float limit_array[][300])
{
	int i = 0, j =0;
	int n = 0;
	int flag = 0;
	int row_result = 0;
	int x, y;

	x = *xx;
	y = *yy;
	row_result = *row;

	//printf("///////////////////////////// TEST START ///////////////////////////\n");
	if(!strcmp(token,"S"))
	{
		for(n = 0; n < x+1 ; n++) //1 : x
		{
			token = strtok(NULL, TOKEN_SEP_COMMA);
			if(!strcmp(token,"E"))
			{
				row_result++;
				if(row_result > y-1) //2 :y
				{
					row_result = 0;
					flag = 1;

					if(DEBUG_MODE){
						for(i = 0; i< y; i++)
						{
							for(j = 0; j<x ;j++)
							{
								printf(" %0.4f",limit_array[i][j]);
							}
							printf("\n");
						}
					}

				}
				break;
			}
			limit_array[row_result][n] = strtod(token,NULL);
		}
	}
	*row = row_result;
	return flag;
}

void synaptics_02_init_limit_data(struct synaptics_02_touch_limit *limit){
	l_limit = *limit;
}

u8 synap_rbuf[200] = {0xff, };	// rbuf will duplicate somewhere.. static impossible, because functions are not static..

#define IMAGE_ROW				(18)
#define IMAGE_COL				(36)
#define IMAGE_COL_LONG			(40)

#define HYBRID_ABS_TX			(18)
#define HYBRID_ABS_RX			(36)
#define HYBRID_ABS_WITH_CBC		(HYBRID_ABS_TX + HYBRID_ABS_RX)			// 길이

#define NOISE					(648)		// 18 * 36
#define SENSOR					(720)		// 18 * 40
#define ADCRANGE				(720)		// 18 * 40
#define TRXGND					(8)

uint16_t fullraw[(IMAGE_ROW)*(IMAGE_COL)] = {0xffff, };					// TD06 FULL RAW CAP
int16_t fullraw_h[(IMAGE_ROW-1)*(IMAGE_COL)] = {0xffff, };				// TD19 FULL RAW H : 음수가능
int16_t fullraw_v[(IMAGE_ROW)*(IMAGE_COL-1)] = {0xffff, };				// TD20 FULL RAW V : 음수가능

uint16_t open_short[(IMAGE_ROW)*(IMAGE_COL)] = {0xffff, };				// TD18 HIGH RESIST, not 18 x 36, 705...
uint32_t hybrid_abs_with_cbc[HYBRID_ABS_WITH_CBC] = {0xffffffff, };		// TD11 Hybrid_abs_with_cbc

int16_t noise[NOISE] = {0xffff, };										// TD07 NOISE TEST : 음수가능
int16_t sensor[SENSOR] = {0xffff, };									// TD08 SENSOR SPEED : 음수가능

//uint16_t adcrange[ADCRANGE] = {0xffff, };								// TD15 ADC RANGE TEST
uint16_t adcrange[(IMAGE_ROW)*(IMAGE_COL)] = {0xffff, };

//#define NUM_RX		(29)		// FULL_RAW 길이와 다르다..184개가 모자르다...	
//#define NUM_TX		(16)
#define NUM_RX		(18)		// RX와 TX 길이가 반대로 들어가야 라이브러리 호출이 되는것 같다..	
#define NUM_TX		(36)
//#define NUM_RX		(36)		// RX와 TX 길이가 반대로 들어가야 라이브러리 호출이 되는것 같다..	
//#define NUM_TX		(18)

#define LIMIT_TIXEL (-10)
#define LIMIT_TXROE (100)
#define LIMIT_RXROE (100)

#if 0
short p_ref_frame[NUM_TX * NUM_RX] = {
568,578,576,575,575,574,576,575,575,573,572,571,571,572,571,569,568,568,569,566,562,563,565,563,559,558,574,556,556,556,554,550,548,547,548,588,
690,599,599,597,597,596,597,597,597,596,594,594,593,593,594,590,589,589,591,587,582,584,585,583,579,579,600,576,575,576,573,570,566,566,565,684,
693,597,596,595,595,594,594,594,594,593,591,591,591,591,592,588,586,587,589,585,580,580,582,581,577,576,576,573,573,573,571,566,563,562,562,679,
696,595,595,594,594,593,593,593,593,592,590,590,589,590,590,587,585,585,587,584,578,579,581,580,576,575,589,572,572,572,570,564,561,560,560,668,
698,595,595,595,595,593,594,594,593,593,591,591,590,590,590,587,586,586,588,584,578,580,581,580,576,575,575,572,571,571,569,564,562,561,560,678,
613,597,594,593,593,592,593,593,592,592,590,589,588,589,589,586,584,584,586,583,577,579,579,578,574,573,573,571,570,570,568,563,560,559,559,677,
519,609,591,589,590,589,590,589,590,589,587,586,586,587,587,583,582,583,584,580,575,577,578,576,572,571,572,569,568,568,566,561,559,558,557,680,
537,615,591,590,590,590,590,591,590,589,587,587,587,587,587,584,582,582,584,581,575,577,578,577,572,571,572,569,568,568,566,561,559,558,557,684,
581,616,592,590,590,589,590,590,590,589,588,587,587,586,587,583,582,582,585,581,575,577,578,577,572,572,572,569,568,568,566,561,559,557,556,684,
594,616,592,591,590,589,590,591,590,590,589,589,587,588,587,585,582,583,584,580,575,577,578,577,572,572,572,569,568,568,566,562,559,557,556,685,
597,611,587,585,586,585,586,586,586,585,584,583,583,583,583,579,578,578,580,576,571,572,574,572,568,567,567,564,564,564,562,556,554,552,551,680,
628,609,590,588,589,588,589,590,590,589,588,588,586,587,587,584,582,582,583,580,574,577,577,576,572,571,571,568,567,567,565,560,557,556,554,687,
969,596,592,591,591,590,591,591,591,591,590,589,588,589,589,585,584,583,585,581,576,577,578,576,573,572,573,569,568,568,566,560,558,556,555,687,
762,593,592,590,591,591,591,591,591,591,589,589,588,589,588,585,583,583,584,580,574,576,578,576,572,572,572,569,568,568,565,560,557,557,557,690,
731,593,592,591,591,591,591,591,591,591,589,589,589,589,588,585,583,583,585,581,575,577,577,576,572,571,571,568,568,568,567,562,559,558,557,691,
718,587,587,586,587,585,586,586,585,585,583,583,582,583,583,595,578,578,579,575,569,571,572,571,566,566,566,564,563,563,561,556,554,554,553,687,
716,591,590,589,589,593,588,588,588,588,586,586,585,586,585,601,580,580,582,577,572,574,575,574,569,568,569,566,566,566,564,561,559,558,558,695,
626,593,591,589,589,589,588,588,588,587,586,585,585,586,585,587,579,580,581,576,571,571,571,569,564,563,563,561,559,560,558,554,554,553,556,617
};
#endif

short p_ref_frame[NUM_TX * NUM_RX] = {
507,562,560,561,560,557,555,554,554,551,548,547,544,543,543,536,532,532,530,524,520,519,518,515,512,510,517,500,498,496,491,485,481,476,472,477,   
682,643,641,639,639,638,637,636,636,634,630,628,627,626,624,619,617,617,616,612,607,606,606,603,600,597,616,592,590,588,583,578,572,568,560,635,
713,658,656,654,653,653,652,651,651,649,645,644,643,642,639,635,633,632,632,628,623,622,621,619,615,612,611,608,606,603,599,593,588,583,576,659,
721,661,658,657,656,655,654,653,653,651,647,646,645,644,641,637,635,635,634,630,625,624,624,622,617,615,627,610,608,606,603,595,590,585,579,656,
726,664,661,659,658,658,656,656,655,653,650,648,647,646,644,640,638,637,637,633,628,627,626,624,620,618,616,613,611,608,604,598,593,588,581,667,
640,668,663,661,660,659,658,658,657,655,652,650,649,648,646,642,640,639,639,635,630,630,629,626,623,620,618,615,614,611,606,600,596,591,585,671,
549,668,658,656,655,654,653,653,652,650,647,645,645,644,642,637,636,635,636,634,627,626,626,624,620,618,616,613,612,609,605,600,596,591,585,675,
557,670,662,659,658,657,656,655,655,653,650,648,648,647,644,640,638,638,638,635,629,628,628,626,622,619,618,615,613,610,606,600,596,591,585,680,
600,675,666,664,662,661,660,660,659,657,654,653,652,651,649,644,643,642,642,638,633,632,632,630,626,623,621,619,617,614,610,604,599,595,589,685,
613,677,668,665,664,663,662,661,661,659,656,654,654,653,650,647,644,644,643,639,634,634,633,631,627,625,623,620,618,615,612,606,601,596,590,689,
617,672,664,661,660,659,658,657,657,655,652,651,650,649,646,642,640,640,639,635,630,629,629,627,623,620,619,616,614,611,607,602,597,592,586,687,
651,674,664,662,661,660,658,658,658,656,653,652,650,649,647,643,641,640,640,636,631,630,629,627,623,621,619,616,615,612,608,602,597,593,587,689,
987,674,666,664,663,662,661,661,660,658,655,654,653,652,649,645,643,642,642,637,632,631,631,628,624,622,620,617,615,612,608,602,598,593,587,691,
791,666,661,660,659,658,657,657,656,654,650,649,648,647,644,639,638,637,638,635,628,626,626,623,620,617,615,613,611,608,605,599,595,593,588,691,
757,663,658,657,656,655,654,654,653,651,648,646,646,644,641,637,635,634,633,629,624,623,622,620,616,613,611,609,607,606,602,597,592,587,580,685,
747,663,658,657,657,656,654,653,653,651,647,646,645,643,641,653,634,633,633,629,623,622,621,619,615,613,612,610,608,604,601,595,591,586,580,686,
717,645,641,640,638,642,636,635,635,632,629,627,626,625,623,635,616,615,616,614,606,605,604,602,598,596,594,592,590,588,584,578,575,570,565,664,
568,594,589,587,585,581,578,577,575,572,568,566,563,561,560,556,547,547,544,537,531,529,526,522,519,516,510,505,503,500,495,490,487,482,479,515
};
// AVDD DVDD : F2 reference
int synaptics_02_current_avdd_dvdd_check(){
	int16_t avdd_cur1;							// TBD : dvdd 250 avdd 200	
	int16_t dvdd_cur1;
	int16_t avdd_cur2;
	int16_t dvdd_cur2;
    char temp1[300];
    char temp2[300];
	char temp3[300];
	char temp4[300];
    int res;
    memcpy(temp1, l_limit.avdd_MAX, 300);
    memcpy(temp2, l_limit.dvdd_MAX, 300);
    memcpy(temp3, l_limit.avdd_MIN, 300);
    memcpy(temp4, l_limit.dvdd_MIN, 300);
    
    int16_t avdd_max = (int16_t) strtol(temp1, NULL, 0);
    int16_t dvdd_max = (int16_t) strtol(temp2, NULL, 0);
    int16_t avdd_min = (int16_t) strtol(temp3, NULL, 0);
    int16_t dvdd_min = (int16_t) strtol(temp4, NULL, 0);
    
	get_current_test_result_for_touch(&dvdd_cur1, &avdd_cur1, &dvdd_cur2, &avdd_cur2);
	printf("[TEST] dvdd_max is %d, avdd_max is %d\n",dvdd_max,avdd_max);
	printf("[TEST] dvdd_min is %d, avdd_min is %d\n",dvdd_min,avdd_min);
    printf("ch1 dvdd : %d\nch1 avdd : %d\nch2 dvdd : %d\nch2 avdd : %d\n",dvdd_cur1, avdd_cur1, dvdd_cur2, avdd_cur2);
	if((dvdd_cur1 == 0) && (avdd_cur1 == 0) && (dvdd_cur2 == 0) && (avdd_cur2 == 0)){		// i2c fail, no value
		res = 0;		// FAIL
	}else if((avdd_cur1 > avdd_max) || (dvdd_cur1 > dvdd_max) || (avdd_cur2 > avdd_max) || (dvdd_cur2 > dvdd_max)
			|| (avdd_cur1 < avdd_min) || (dvdd_cur1 < dvdd_min) || (avdd_cur2 < avdd_min) || (dvdd_cur2 < dvdd_min)){
        res = 0;		// FAIL
    }else{
        res = 1;		// PASS
    }
	return res;
}

// reference source : test_attention
// IDENTIFY timeout 내에 정상으로 받으면 통과
int synaptics_02_attention_check(){
	int res;									// PASS/FAIL	
	send_command(IDENTIFY);

	if((synap_rbuf[0] == 0xa5) && (synap_rbuf[1] == 0x10)){
		res = 1;		// PASS
	}else{
		res = 0;		// FAIL
	}
	return res;
}

#define _IOCTL_1CH_TOUCH_RESET      0x1122
#define _IOCTL_2CH_TOUCH_RESET      0x1123

// reference source : test_reset_pin
// RESET 핀을 PULSE??(아래위 조작)  한뒤 10회 packet read + 딜레이100ms 할때 정상이면 통과
int synaptics_02_reset_pin_check(){
	// from stm_touch_dev_07.c					// PASS/FAIL
	 /*
     * Notice!!
     *      Drive the reset pin to "LOW" and keep for 10ms over, then drive it to "HIGH".
     */
	int i, res, tmp;
#if 1
    tmp = 0;
    ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);                                                                                                       
    ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);                                                                                                           
    usleep(10000);                                                                                        

    tmp = 1;
    ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);                                                                                                       
    ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);  
	usleep(10000);
#endif

	//printf("START IDENTIFY TEST\n");	
	for(i=0;i<10;i++){
		send_command(IDENTIFY);
		if((synap_rbuf[0] == 0xa5) && (synap_rbuf[1] == 0x10)){
			res = 1;		// PASS
			break;
		}else{
			res = 0;		// FAIL
		}
		usleep(100000);		// 100ms
	}	
	return res;
}

// test_firmwareid
// IDENTIFY 한뒤 build_id 를 체크 
int synaptics_02_fw_version_check(){
	//u8 build_id[4];							// SPEC : 2942589
	int res;
	u8 build_id[5];
	char build_id_print[100];
	unsigned int result;

	send_command(IDENTIFY);
	printf("CHECK BUILD ID\n");

	// firmware build id
	build_id[0] = synap_rbuf[22];
	build_id[1] = synap_rbuf[23];
	build_id[2] = synap_rbuf[24];
	build_id[3] = synap_rbuf[25];

	{
		// sprintf는 자동으로 끝에 null문자 붙여준다.
		sprintf(build_id_print, "%02X%02X%02X%02X", build_id[3], build_id[2], build_id[1],build_id[0]);	// little endian 32bit
		sscanf(build_id_print, "%x", &result);
		memset(build_id_print, 0x00, sizeof(build_id_print));
		
		sprintf(build_id_print, "%u", result);
	}

	printf("SPEC : %s, TOUCH-IC : %u\n", l_limit.fw_ver, result);
	if(!strcmp(build_id_print, l_limit.fw_ver)){
//		printf("PASS\n");
		res = 1;
	}else{
//		printf("FAIL\n");
		res = 0;
	}

	return res;
}

// test_configuration
// GET_APP_INFO, ENTER BT MODE, READ_APP_CONFIG 후 crc, configid 체크
// 현제는 config id 만 체크

//#define CRC_OFFSET		8
int synaptics_02_touch_ic_config_check(){
	int res;									// SPEC : 0x29020000
	char config_id1[100];							

	send_command(GET_APP_INFO);

	// spec에 비해 rawdata 의 길이가 4배라서, 일단 4군데에 전부 받는다
	printf("CHECK CUSTOMER CONFIGURATION ID\n");		// start with 20, not 16
	{
		sprintf(config_id1, "%02X%02X%02X%02X%02X%02X%02X%02X", synap_rbuf[20], synap_rbuf[21], synap_rbuf[22], synap_rbuf[23], synap_rbuf[24], synap_rbuf[25], synap_rbuf[26], synap_rbuf[27]);
	}

	printf("SPEC : %s, TOUCH-IC : %s\n", l_limit.config_ver, config_id1);
	if(!strcmp(config_id1, l_limit.config_ver)){
	//	printf("PASS\n");
		res = 1;
	}else{
	//	printf("FAIL\n");
		res = 0;
	}

	return res;
}

// 190513 LINE SETUP
// test_devicepackage
// IDENTIFY 한뒤 part_number 확인 
int synaptics_02_device_package_check(){
	int res;									// SPEC : 3909
	u8 part_num[4];
	send_command(IDENTIFY);
	printf("CHECK PART NUMBER STRING\n");
	{
		printf("%s\n", synap_rbuf+6);	// "s3909-15.0.1\0"
	}

	part_num[0] = synap_rbuf[7];
	part_num[1] = synap_rbuf[8];
	part_num[2] = synap_rbuf[9];
	part_num[3] = synap_rbuf[10];
	
	printf("SPEC : %s, TOUCH-IC : %s\n", l_limit.product_id, part_num);
	if(!strcmp(part_num, l_limit.product_id)){
	//	printf("PASS\n");
		res = 1;
	}else{
	//	printf("FAIL\n");
		res = 0;
	}
	return res;
}

// test_lockdown
// BT ver1, AREA_BOOT_CONFIG, ENTIRE_BLOCK

// m_config_length = bt_info.boot_config_blocks * bt_info.write_block_words * 2;
// start_addr = bt_info.boot_config_start_block * bt_info.write_block_words;

// flash_read(start_addr, m_config, m_config_length);
// otp_data = m_config;		otp_total_length = m_config_length;

#define LOCKDOWN_DEBUG	0
int synaptics_02_lockdown_check(){	
	int res = 1;								// 레퍼런스 스펙 : 0xff 가 128개..?
	int otp_total_length;
	uint16_t block_num, block_size, write_block_size;
	uint32_t start_addr;
	u8 payload[6];	

	uint8_t otp_data[512];

	send_command_delay(ENTER_BOOTLOADER_MODE);
	send_command(GET_BOOT_INFO);	

	#if LOCKDOWN_DEBUG	
	printf("\n\n");
	printf("CHECK OTP BLOCK NUM / SIZE\n"); 	
	
	printf("OTP BLOCK NUM : 0x%02X%02X\n", synap_rbuf[17],synap_rbuf[16]);
	printf("OTP BLOCK SIZE : 0x%02X%02X\n", synap_rbuf[19],synap_rbuf[18]);
	#endif

	block_num = synap_rbuf[16] | synap_rbuf[17] << 8;
	block_size = synap_rbuf[18] | synap_rbuf[19] << 8;

	#if LOCKDOWN_DEBUG
	printf("WRITE BLOCK SIZE : 0x%02X\n", synap_rbuf[8]);
	#endif

	write_block_size = synap_rbuf[8];

	start_addr = block_num * write_block_size;

	// otp_total_length 의 2배로 byte를 읽는다. 16bit 시스템이므로 길이 단위가 short
	// 레퍼런스 코드의 TcmLimit::ParseTokens 함수에선 config.txt 에 대해 LIMIT_OTP 토큰을 파싱후, 값을 가져가고 있다. 
	
	otp_total_length = block_size * write_block_size * 2;

	#if LOCKDOWN_DEBUG
	printf("start_addr is %d 0x%02X, otp_total_length(unit : short) is %d 0x%02X\n", start_addr, start_addr, otp_total_length, otp_total_length);
	#endif

	payload[0] = start_addr & 0x000000ff;
	payload[1] = (start_addr & 0x0000ff00) >> 8;
	payload[2] = (start_addr & 0x00ff0000) >> 16;
	payload[3] = (start_addr & 0xff000000) >> 24;
	payload[4] = otp_total_length & 0x00ff;
	payload[5] = (otp_total_length & 0xff00) >> 8;

	#if LOCKDOWN_DEBUG
	printf("\n\n");
	printf("sizeof payload : %d\n", sizeof(payload));
	printf("payload is %02X %02X %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5]);
	#endif
	
	send_command_multi_payload_output_buf_uint8_t(READ_FLASH, payload, sizeof(payload), otp_data);
	send_command_delay(RUN_APPLICATION_FIRMWARE);

	#if LOCKDOWN_DEBUG
	printf("length is %d\n", sizeof(otp_data)/sizeof(uint8_t));
	#endif

	// 판정식은 아직 스펙이 없어서 못넣음
	#if LOCKDOWN_DEBUG	
	{
		int i;
		if(DEBUG_MODE){
			printf("\n\n");
			printf("PRINT OTP DATA\n");
		}
		for(i=0;i<sizeof(otp_data)/sizeof(uint8_t);i++){
			
			if((i%16) == 0){
				if(DEBUG_MODE)
					printf("\n");
			}

			if(DEBUG_MODE)
				printf("0x%x ", otp_data[i]);
		}
		if(DEBUG_MODE)
			printf("\n\n");
	}
	#endif

	return res;
}

/* LWG 190705 _IOCTL_CH1_TE_GET 핀은 회로도상 1CH_TE 가 아니라 1CH_TOUCH_INT 이므로, attention interrupt pin check 에 사용가능 */
#define _IOCTL_CH1_TE_GET               0x1004		// 1CH_IO2
#define _IOCTL_CH2_TE_GET               0x1005		// 2CH_IO2

#define _IOCTL_CH1_TCH_INT_ENABLE	0x1132
#define _IOCTL_CH1_TCH_INT_DISABLE	0x1133
#define _IOCTL_CH2_TCH_INT_ENABLE	0x1134
#define _IOCTL_CH2_TCH_INT_DISABLE	0x1135

// test_INT2
// gpio 핀 (IO2) 2번 읽은뒤 값이 같으면 통과
int synaptics_02_attention2_check(int ch_num){	
#if 0
	int res = 0;		// 0 : FAIL, 1 : PASS
	int read_first_int = 0;		// 0 : FAIL, 1 : FALLING EDGE OCCUR
	int read_second_int = 0;

	// READ GPIO FIRST
	switch(ch_num){
		case 1:
			ioctl(dic_dev, _IOCTL_CH1_TCH_INT_ENABLE, NULL);
			break;
		case 2:
			ioctl(dic_dev, _IOCTL_CH2_TCH_INT_ENABLE, NULL);
			break;
	}

	u8 payload[] = { FIELD_INT2, 3, 0 };		// 0 : FIELD ID
												// 1, 2 : FIELD VALUE
	send_command_multi_payload(SET_DYNAMIC_CONFIG, payload, sizeof(payload)); 
	usleep(30000);

	switch(ch_num){
		case 1:
			printf("READ FIRST INT\n");
			ioctl(dic_dev, _IOCTL_CH1_TCH_INT_DISABLE, &read_first_int);
			break;
		case 2:
			printf("READ FIRST INT\n");
			ioctl(dic_dev, _IOCTL_CH2_TCH_INT_DISABLE, &read_first_int);
			break;
	}

	printf("%d\n", read_first_int);
	usleep(30000);	

	// READ GPIO SECOND
	switch(ch_num){
		case 1:
			ioctl(dic_dev, _IOCTL_CH1_TCH_INT_ENABLE, NULL);
			break;
		case 2:
			ioctl(dic_dev, _IOCTL_CH2_TCH_INT_ENABLE, NULL);
			break;
	}

	payload[1] = 1;
	send_command_multi_payload(SET_DYNAMIC_CONFIG, payload, sizeof(payload));
	usleep(30000);	
	
	switch(ch_num){
		case 1:
			printf("READ SECOND INT\n");
			ioctl(dic_dev, _IOCTL_CH1_TCH_INT_DISABLE, &read_second_int);
			break;
		case 2:
			printf("READ SECOND INT\n");
			ioctl(dic_dev, _IOCTL_CH2_TCH_INT_DISABLE, &read_second_int);
			break;
	}
	printf("%d\n", read_second_int);
	usleep(30000);	

	payload[1] = 0;
	send_command_multi_payload(SET_DYNAMIC_CONFIG, payload, sizeof(payload));
	usleep(30000);	
	
	// if FIRST == inputpin.mask or SECOND == inputpin.mask
	if(read_first_int == read_second_int)
		res = 1;
	else
		res = 0;

	// 두번 테스트중 한번이라도 FALLING 가 발생하지 않았다면 FAIL 처리
	if((read_first_int == 0) || (read_second_int == 0))
		res = 0;

	return res;
#endif
	return 1;
}

// test_ExternalOSC
// PRODUCTION_TEST(TEST_EXTERNAL_OSC)
#define OSC_DEBUG	0
int synaptics_02_osc_check(){
	int res;									// PASS/FAIL
	u8 payload[] = { TEST_EXTERNAL_OSC };
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));		// 05/01 NOT IMPLEMENTED!
	if(synap_rbuf[1] == 0x01){
		#if OSC_DEBUG
		printf("PASS\n");
		#endif
		res = 1;
	}else{
		#if OSC_DEBUG
		printf("FAIL\n");
		#endif
		res = 0;
	}
	return res;
}

// test_BSCCalibration
// ERASE AND CALIBRATE
#define BSC_CALIBRATION_DEBUG	1
int synaptics_02_bsc_calibration_check(){
	int res = 1;									// PASS/FAIL
	u8 payload[] = { 3 };		// ERASE COMMAND
	send_command_multi_payload(CALIBRATE, payload, sizeof(payload));
	if(synap_rbuf[1] == 0x01){
		#if BSC_CALIBRATION_DEBUG
		printf("ERASE PASS\n");
		#endif
	}else{
		#if BSC_CALIBRATION_DEBUG
		printf("ERASE FAIL\n");
		#endif
		res = 0;
		return res;
	}
	payload[0] = 2;				// CALIBRATE COMMAND
	send_command_multi_payload(CALIBRATE, payload, sizeof(payload));
	if(synap_rbuf[1] == 0x01){
		#if BSC_CALIBRATION_DEBUG
		printf("CALIBRATE PASS\n");
		#endif
		if(res == 1){
			// DO NOTHING
		}else{
			res = 0;
		}
	}else{
		#if BSC_CALIBRATION_DEBUG
		printf("CALIBRATE FAIL\n");
		#endif
		res = 0;
	}
	return res;
}

// test_TRxShort, test_PinTest
//PRODUCTION_TEST(TEST_TRX_SENSOR_OPEN)
//PRODUCTION_TEST(TEST_TRX_GND_SHORTS)
#define SHORT_DEBUG		1
int synaptics_02_short_check(){	
	int res = 1;								// pass/fail
	int i, bit;
	uint8_t temp[8];
	uint8_t bitvalue, limit;
	int trx_sensor_result = 1;
	int trx_gnd_result = 1;

	u8 trx_sensor_spec[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 trx_gnd_spec[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f };

#if 1
	u8 payload[] = { TEST_TRX_SENSOR_OPEN };
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));	

	// 담는다.
	for(i=0;i<8;i++){
		temp[i] = synap_rbuf[4+i];
	}

	// temp를 trx_sensor_spec과 비교한다.
	printf("TRX_SENSOR_OPEN TEST\n");
	for(i=0;i<8;i++){
		if(temp[i] != trx_sensor_spec[i]){
			printf("!");
			res = 0;
			trx_sensor_result = 0;
		}
		printf("0x%x ", temp[i]);
	}
	printf("\n");
	
	// 결과를 출력한다.
	if(trx_sensor_result){
		printf("PASS\n");
	}else{
		printf("FAIL\n");
	}

#endif
#if 1
	payload[0] = TEST_TRX_GND_SHORTS;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));	

	// 담는다.
	for(i=0;i<8;i++){
		temp[i] = synap_rbuf[4+i];
	}

	// temp를 trx_gnd_spec과 비교한다.
	printf("TRX_GND_SHORTS TEST\n");
	for(i=0;i<8;i++){
		if(temp[i] != trx_gnd_spec[i]){
			printf("!");
			res = 0;
			trx_gnd_result = 0;
		}
		printf("0x%x ", temp[i]);
	}
	printf("\n");
	
	// 결과를 출력한다.
	if(trx_gnd_result){
		printf("PASS\n");
	}else{
		printf("FAIL\n");
	}

#endif
	return res;
}

// test_SensorSpeed							
// PRODUCTION_TEST(TEST_SENSORSPEED)
#define SENSOR_SPEED_DEBUG		0
int synaptics_02_sensor_speed_check(){
	int i;										// spec : -0.25 ~ 0.25
	int res = 1;

	char temp1[300];
	char temp2[300];

	memcpy(temp1, l_limit.sensor_MAX, 300);
    memcpy(temp2, l_limit.sensor_MIN, 300);

//  int16_t max = (int16_t) strtol(temp1, NULL, 0);
//  int16_t min = (int16_t) strtol(temp2, NULL, 0);
//	printf("temp1 : %s\ntemp2 : %s\n", temp1, temp2);		// OK
	float max = (float) strtof(temp1, NULL);
	float min = (float) strtof(temp2, NULL);
	
	u8 payload[] = { TEST_SENSORSPEED };
	//send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));	
	send_command_multi_payload_output_buf_int16_t(PRODUCTION_TEST, payload, sizeof(payload), sensor);	

	float value;

	printf("\n");
	printf("MAX is %3.3f, MIN is %3.3f\n", max, min);
	#if SENSOR_SPEED_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
 	#endif

	for (i = 0; i < SENSOR; i++){
		if(i%16 == 0){
			if(DEBUG_MODE)
				printf("\n");
		}

		value = (float)(sensor[i]) / 1000.f;
		if((value > max) || (value < min)){
			if(DEBUG_MODE)
				printf("!");
            res = 0;
        }

		if(DEBUG_MODE)
			printf("%3.3f ", value);
			//printf("%hd ", sensor[i]);

		#if 0
		printf("[%d:rawdata is %3.3f]\n", i, value);
		#endif
	}	

	printf("\n");
	return res;
}

// 라이브러리 함수
void extended_high_resistance_test(
        /* IN: Rx Channel number for 2D area*/
        unsigned char rx_2d_channel,
        /* IN: Tx Channel number for 2D area */
        unsigned char tx_2d_channel,
        /* IN: Pointer to the delta image for 2D area */
        signed short * delta_2d_image,
        /* IN: Pointer to the baseline image for 2D area */
        signed short * baseline_image,
        /* IN: Pointer to the reference raw image for 2D area */
        signed short * ref_2d_image,

        /* OUT: Pointer to Rx_Result arry, Rx result will be stored here */
        signed short * rx_Result,
        /* OUT: Pointer to Tx_Result arry, Tx result will be stored here */
        signed short * tx_Result,
        /* OUT: Pointer to Surface_Result arry, surface result will be stored here */
        signed short * surface_Result
);

// test_HighResistance
// PRODUCTION_TEST(TEST_FULLRAW)
#define HIGH_RESISTANCE_DEBUG	0
int synaptics_02_extended_high_resist_check(){	
	int row, col;								// spec : TABLE
    int offset;
	int res = 1;

	//short Baseline[NUM_RX * NUM_TX];
	short *Baseline = NULL;
	//short Delta[NUM_RX * NUM_TX];
	short *Delta = NULL;	

	short *p_col_result = NULL;
	short *p_row_result = NULL;
	short *p_result = NULL;

	unsigned char rx_2d_channel = NUM_RX;
	unsigned char tx_2d_channel = NUM_TX;

	u8 payload[] = { TEST_FULLRAW };		// test_HighResistance 는 FULLRAW로 시작함
	send_command_multi_payload_output_buf(PRODUCTION_TEST, payload, sizeof(payload), fullraw);	

	#if HIGH_RESISTANCE_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif


	Delta = calloc((NUM_RX) * (NUM_TX), sizeof(short));
	p_result = calloc((NUM_RX + 1) * (NUM_TX + 1), sizeof(short)); 
	p_row_result = calloc(NUM_RX, sizeof(short));
	p_col_result = calloc(NUM_TX, sizeof(short));

	Baseline = fullraw;		// unsigned short 를 short 배열에다 담았다.
	{
		int i;
		for(i=0;i<(NUM_RX*NUM_TX);i++){
			Delta[i] = Baseline[i] - p_ref_frame[i];
		}
	}

	extended_high_resistance_test(IMAGE_COL, IMAGE_ROW,
					Delta,
					Baseline,
					p_ref_frame, 
					p_col_result,
					p_row_result, 
					p_result);
	{
		int i,j;
		float value;

		if(DEBUG_MODE)
			printf("TEST RESULT ARRAY (result / 1000)\n");
	
		for(i = 0; i < tx_2d_channel + 1; i++) {
			for(j = 0; j < rx_2d_channel + 1; j++) {
				value = (float)p_result[i * rx_2d_channel + j];
				value = value / 1000.f;		
				if((value > l_limit.extend_high_regi_MAX[j][i]) || (value < l_limit.extend_high_regi_MIN[j][i])){
					if(DEBUG_MODE)
						printf("!");
					res = 0;
				}
				
				if(DEBUG_MODE)
					printf("%3.2f ",value);

				#if 0
				printf("[rawdata is %3.2f, MAX is %3.2f, MIN is %3.2f]\n", value, l_limit.extend_high_regi_MAX[j][i], l_limit.extend_high_regi_MIN[j][i]);
				#endif
			}
			if(DEBUG_MODE)
				printf("\n");
		}
	}
	
	free(Delta);
	free(p_result);
	free(p_col_result);
	free(p_row_result);

	return res;

}


// test_AdcRange
// PRODUCTION_TEST(TEST_ADCRANGE)
#define ADC_RANGE_DEBUG		0
int synaptics_02_adc_range_check(){	
	int i;										// spec : TABLE
	int res = 1;

	int row, col;
	int offset;

	u8 payload[] = { TEST_ADCRANGE };
	send_command_multi_payload_output_buf(PRODUCTION_TEST, payload, sizeof(payload), adcrange);	

	// rawdata 길이는 720으로 나오지만, 실제론 648개까지만 정상데이터, 이 이슈는 여기서 해결
	#if ADC_RANGE_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif

	for (row = 0; row < IMAGE_ROW; row++)
    {
        offset = row * (IMAGE_COL);
        for (col = 0; col < IMAGE_COL; col++)
        {
            if((adcrange[offset + col] > (uint16_t)(l_limit.ADC_RANGE_MAX)[row][col]) || (adcrange[offset + col] < (uint16_t)(l_limit.ADC_RANGE_MIN)[row][col])){
				if(DEBUG_MODE)
					printf("!");
                res = 0;
            }

			if(DEBUG_MODE)
				printf("%d ", adcrange[offset + col]);

			#if 0
			printf("[%d:rawdata is %hu, ", offset + col, adcrange[offset + col]);
			printf("MAX is %hu, MIN is %hu]\n", (uint16_t)(l_limit.ADC_RANGE_MAX)[row][col], (uint16_t)(l_limit.ADC_RANGE_MIN)[row][col]);
			#endif
        }
		if(DEBUG_MODE)
			printf("\n");
    }

	return res;
}

// test_FullRawCap
// PRODUCTION_TEST(TEST_FULLRAW)	
#define FULL_RAW_CAP_DEBUG	0
int synaptics_02_full_raw_cap_check(){
	int row, col;								// spec : TABLE
    int offset;
	int res = 1;

	u8 payload[] = { TEST_FULLRAW };
	send_command_multi_payload_output_buf(PRODUCTION_TEST, payload, sizeof(payload), fullraw);	

	#if FULL_RAW_CAP_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif

	for (row = 0; row < IMAGE_ROW; row++)
    {
        offset = row * (IMAGE_COL);
        for (col = 0; col < IMAGE_COL; col++)
        {
            if((fullraw[offset + col] > (uint16_t)(l_limit.full_raw_cap_MAX)[row][col]) || (fullraw[offset + col] < (uint16_t)(l_limit.full_raw_cap_MIN)[row][col])){

				if(DEBUG_MODE)
					printf("!");
                res = 0;
            }

			if(DEBUG_MODE)
				printf("%d ", fullraw[offset + col]);

			#if 0
			printf("[%d:rawdata is %hu, ", offset + col, fullraw[offset + col]);
			printf("MAX is %hu, MIN is %hu]\n", (uint16_t)(l_limit.full_raw_cap_MAX)[row][col], (uint16_t)(l_limit.full_raw_cap_MIN)[row][col]);
			#endif
        }
		if(DEBUG_MODE)
			printf("\n");
    }

	return res;
}



// PRODUCTION_TEST(TEST_FULLRAW)	
#define FULL_RAW_CAP_H_DEBUG	0
int synaptics_02_full_raw_cap_h_check(){
	int row, col;								// spec : TABLE
    int offset;
	int res = 1;

	#if FULL_RAW_CAP_H_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif
	for (row = 0; row < IMAGE_ROW; row++)
    {
        offset = row * (IMAGE_COL);
        for (col = 0; col < IMAGE_COL; col++)
        {
			if(col == IMAGE_COL -1){
				continue;				
			}else{
				fullraw_h[offset + col] = fullraw[offset + col] - fullraw[offset + col + 1];
				
				if((fullraw_h[offset + col] > (int16_t)(l_limit.full_raw_cap_h)[row][col])){
					if(DEBUG_MODE)
						printf("!");
                	res = 0;
            	}
				
				if(DEBUG_MODE)
					printf("%hd ", fullraw_h[offset + col]);

				#if 0
				printf("[rawdata is %hd, spec is %hd]\n", fullraw_h[offset + col], (int16_t) l_limit.full_raw_cap_h[row][col]);
				#endif
			}
			
        }
		if(DEBUG_MODE)
			printf("\n");
    }

	return res;
}

// PRODUCTION_TEST(TEST_FULLRAW)	
#define FULL_RAW_CAP_V_DEBUG	0
int synaptics_02_full_raw_cap_v_check(){
	int row, col;								// spec : TABLE
    int offset1, offset2;
	int res = 1;
	
	#if FULL_RAW_CAP_V_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif
	
	for (row = 0; row < IMAGE_ROW; row++)
    {
        offset1 = row * (IMAGE_COL);
		offset2 = (row+1) * (IMAGE_COL);

		if(row == IMAGE_ROW -1)	continue;
		// else
    	for (col = 0; col < IMAGE_COL; col++)
    	{
			fullraw_v[offset1 + col] = fullraw[offset1 + col] - fullraw[offset2 + col];
			
			if((fullraw_v[offset1 + col] > (int16_t)(l_limit.full_raw_cap_v)[row][col])){
				if(DEBUG_MODE)
					printf("!");
   	            res = 0;
   	        }

			if(DEBUG_MODE)
				printf("%hu ", fullraw_v[offset1 + col]);

			#if 0
			printf("[rawdata is %hd, spec is %hd]\n", fullraw_v[offset1 + col], (int16_t) l_limit.full_raw_cap_v[row][col]);
			#endif
		}

		if(DEBUG_MODE)
			printf("\n");
    }

	return res;
}


// test_HybridABS		// Tx Rx
// PRODUCTION_TEST(TEST_ABSRAW)
#define HYBRID_ABS_RAW_CAP_DEBUG	0
int synaptics_02_hybrid_abs_raw_cap_check(){
	int i;										// spec : ARRAY
	int res = 1;

	u8 payload[] = {TEST_ABSRAW};
	send_command_multi_payload_output_buf_for_hybrid_abs(PRODUCTION_TEST, payload, sizeof(payload), hybrid_abs_with_cbc);
	#if HYBRID_ABS_RAW_CAP_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif
 
	printf("RX 36(0 ~ 35)\n");
	// RX 36
	for (i = 0; i < HYBRID_ABS_RX; i++){
		if(i%16 == 0){
			printf("\n");
		}

		if((hybrid_abs_with_cbc[i] > (uint32_t)(l_limit.hybrid_raw_cap_RX_MAX)[0][i]) || (hybrid_abs_with_cbc[i] < (uint32_t)(l_limit.hybrid_raw_cap_RX_MIN)[0][i])){
			printf("!");
            res = 0;
        }

		printf("%d ", hybrid_abs_with_cbc[i]);

		#if 0
		printf("[%d:rawdata is %u, ", i, hybrid_abs_with_cbc[i]);
		//printf("[%d:rawdata is 0x%x, ", i, hybrid_abs_with_cbc[i]);
		printf("MAX is %u, MIN is %u]\n", (uint32_t)(l_limit.hybrid_raw_cap_RX_MAX)[0][i], (uint32_t)(l_limit.hybrid_raw_cap_RX_MIN)[0][i]);
		#endif
	}	
	printf("\n");
	
	// TX 18
	printf("TX 18(36 ~ 53)\n");
	for (i = 0; i < HYBRID_ABS_TX; i++){
		if(i%16 == 0){
			printf("\n");
		}

		if((hybrid_abs_with_cbc[HYBRID_ABS_RX+i] > (uint32_t)(l_limit.hybrid_raw_cap_TX_MAX)[0][i]) || (hybrid_abs_with_cbc[HYBRID_ABS_RX+i] < (uint32_t)(l_limit.hybrid_raw_cap_TX_MIN)[0][i])){
			printf("!");
            res = 0;
        }

		printf("%d ",  hybrid_abs_with_cbc[HYBRID_ABS_RX+i]);

		#if 0
		printf("[%d:rawdata is %u, ", i, hybrid_abs_with_cbc[HYBRID_ABS_RX+i]);
		printf("MAX is %u, MIN is %u]\n", (uint32_t)(l_limit.hybrid_raw_cap_TX_MAX)[0][i], (uint32_t)(l_limit.hybrid_raw_cap_TX_MIN)[0][i]);
		#endif

	}	
	printf("\n");
	return res;
}

// test_noise
// PRODUCTION_TEST(TEST_NOISE)
#define NOISE_DEBUG    0
int synaptics_02_noise_check(){	
	int i;										// spec : -10 ~ 10
	int res = 1;

	char temp1[300];
	char temp2[300];

	memcpy(temp1, l_limit.cm_jitter_MAX, 300);
    memcpy(temp2, l_limit.cm_jitter_MIN, 300);

    int16_t max = (int16_t) strtol(temp1, NULL, 0);
    int16_t min = (int16_t) strtol(temp2, NULL, 0);
	
	u8 payload[] = { TEST_NOISE };
	//send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));				
	//send_command_multi_payload_output_buf(PRODUCTION_TEST, payload, sizeof(payload), noise);				
	send_command_multi_payload_output_buf_int16_t(PRODUCTION_TEST, payload, sizeof(payload), noise);

	#if NOISE_DEBUG
	printf("[DEBUG] CHECK SPEC AND RAWDATA\n");
	#endif

	for (i = 0; i < NOISE; i++){

		if(i%16 == 0){
			if(DEBUG_MODE)
				printf("\n");
		}

		if((noise[i] > max) || (noise[i] < min)){
			if(DEBUG_MODE)
				printf("!");
            res = 0;
        }
	
		if(DEBUG_MODE)
			printf("%d ", noise[i]);	

		#if 0
		printf("[%d:rawdata is %hd, ", i, noise[i]);
		printf("MAX is %hd, MIN is %hd]\n", max, min);
		#endif
	}

	printf("\n");

	return res;
}

#define CUSTOMER_ID_DEBUG	0
int synaptics_02_customer_id_check(){
	int res = 1;								// SPEC : P116921000
	int otp_total_length; 				
	uint16_t block_num, block_size, write_block_size;
	uint32_t start_addr;
	u8 payload[6];	

	uint8_t otp_data[512];		// short 256개		// 한 sector를 통째로 읽는걸로 보인다.
	char customer_id[100];		// 문자열로 저장한뒤 출력, 비교

	send_command_delay(ENTER_BOOTLOADER_MODE);
	send_command(GET_BOOT_INFO);	

	#if CUSTOMER_ID_DEBUG	
	printf("CHECK OTP BLOCK NUM / SIZE\n"); 	
	
	printf("OTP BLOCK NUM : 0x%02X%02X\n", synap_rbuf[17],synap_rbuf[16]);
	printf("OTP BLOCK SIZE : 0x%02X%02X\n", synap_rbuf[19],synap_rbuf[18]);
	#endif

	block_num = synap_rbuf[16] | synap_rbuf[17] << 8;
	block_size = synap_rbuf[18] | synap_rbuf[19] << 8;

	#if CUSTOMER_ID_DEBUG
	printf("WRITE BLOCK SIZE : 0x%02X\n", synap_rbuf[8]);
	#endif

	write_block_size = synap_rbuf[8];

	start_addr = block_num * write_block_size;		// 0x880

	// 'otp_total_length' 의 2배로 byte를 읽는다. 길이 단위가 short이다.
	// 레퍼런스 코드의 TcmLimit::ParseTokens 함수에선 config.txt 에 대해 LIMIT_OTP 토큰을 파싱후, 값을 가져가고 있다. 
	
	otp_total_length = block_size * write_block_size * 2;	// 0x100 ( 256 x 2bytes : 512 bytes )

	#if CUSTOMER_ID_DEBUG
	printf("start_addr is %d 0x%02X, otp_total_length(unit : short) is %d 0x%02X\n", start_addr, start_addr, otp_total_length, otp_total_length);
	#endif

	payload[0] = start_addr & 0x000000ff;
	payload[1] = (start_addr & 0x0000ff00) >> 8;
	payload[2] = (start_addr & 0x00ff0000) >> 16;
	payload[3] = (start_addr & 0xff000000) >> 24;
	payload[4] = otp_total_length & 0x00ff;
	payload[5] = (otp_total_length & 0xff00) >> 8;

	#if CUSTOMER_ID_DEBUG
	printf("\n\n");
	printf("sizeof payload : %d\n", sizeof(payload));
	printf("payload is %02X %02X %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5]);
	#endif
	
	send_command_multi_payload_output_buf_uint8_t(READ_FLASH, payload, sizeof(payload), otp_data);
	send_command_delay(RUN_APPLICATION_FIRMWARE);

	#if CUSTOMER_ID_DEBUG
	printf("length is %d\n", sizeof(otp_data)/sizeof(uint8_t));
	#endif

	{
		int i, flag_stop;

		flag_stop = 0;
	
		if(DEBUG_MODE)
			printf("PRINT OTP DATA\n");
		for(i=0;i<sizeof(otp_data)/sizeof(uint8_t);i++){
			
			if((i%16) == 0){
				if(DEBUG_MODE)
					printf("\n");
			}

			if(DEBUG_MODE)
				printf("0x%x ", otp_data[i]);
			
			if(otp_data[i] == 0x00){
				customer_id[i] = otp_data[i];		// NULL 문자를 만나면, NULL 문자를 저장하고, 이후로 저장하지 않는다.
				flag_stop = 1;
			}
			
			if(flag_stop != 1){
				customer_id[i] = otp_data[i];		// NULL 문자를 만난적이 없으면, NULL 문자를 만날때까지 계속 저장한다.
			}
		}
		if(DEBUG_MODE)
			printf("\n\n");
	}

	printf("SPEC : %s, TOUCH-IC : %s\n", l_limit.customer_id, customer_id);
	if(!strcmp(l_limit.customer_id, customer_id)){
		//printf("PASS\n");
	}else{
		//printf("FAIL\n");
		res = 0;
	}

	return res;
}

// 완성
// test_HSyncVSyncCheck
int synaptics_02_hsync_vsync_check(){
	int res;									// PASS/FAIL
	u8 payload[] = { TEST_PID21 };
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));		
	if(synap_rbuf[1] == 0x01){
		printf("PASS\n");
		res = 1;
	}else{
		printf("FAIL\n");
		res = 0;
	}
	return res;
}



/////////////////////////////////////////////////////////////////////////////////////////
// 190517 renew : send_command, send_command_delay, send_command_multi_payload를 사용
// 1. send_command : 기본적인 커맨드
// 2. send_command_delay : 읽고 쓸때 200ms 정도 딜레이가 필요한 커맨드에 사용
// 3. send_command_multi_payload : Production Test, Read Flash 등, 인자를 넘겨주는 커맨드에 널리 사용

#define BASIC_DEBUG		1	
void send_command(u8 addr){	
	int rc, len, len_cnt;
	int arg;	// DUMMY ( NOT USED )
//	u8 wbuf[100] = {0xff, };
	u8 wbuf[200] = {0xff, };		// IN SPI_FUNC, THIS IS RBUF ALSO WBUF
	memset(wbuf, 0xff, sizeof(u8) * 200);
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2			// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 2);		// host send 1 bytes : addr
#else
	spi_func(wbuf+1, synap_rbuf, 1);
#endif
	usleep(10000);

	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2			// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes	
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(10000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2			// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif	

	usleep(10000);
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);	// valid length

	#if BASIC_DEBUG
	printf("len : %d\n", len);
	#endif

	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			#if BASIC_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
    		}
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit;
			#if BASIC_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
			#endif
			do{
				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if BASIC_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}

    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{
						exit = 1;
						break;
					}
    			}
				#if BASIC_DEBUG
    			printf("\n");
				#endif

				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);					// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(10000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(10000);

			}while(exit != 1);
		}
	}
}

#define DELAY_DEBUG		0
void send_command_delay(u8 addr){	// FOR ENTER BOOTLOADER MODE
	int rc, len, len_cnt;
	int arg;	// DUMMY ( NOT USED )
//	u8 wbuf[100] = {0xff, };
	u8 wbuf[200] = {0xff, };
	memset(wbuf, 0xff, sizeof(u8) * 200);
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 2);		// host send 1 bytes : addr
#else
	spi_func(wbuf+1, synap_rbuf, 1);
#endif
	usleep(200000);

	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif

	usleep(200000);
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);	// valid length

	#if DELAY_DEBUG
	printf("len : %d\n", len);
	#endif

	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			#if DELAY_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
    		}
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit;

			#if DELAY_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
			#endif
        	
			do{
				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if DELAY_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}

    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{
						exit = 1;
						break;
					}
    			}
				#if DELAY_DEBUG
    			printf("\n");
				#endif

				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
				usleep(200000);
			}while(exit != 1);
		}
	}
}

#define MULTI_PAYLOAD_DEBUG		0
void send_command_multi_payload(u8 addr, u8 *payload, int payload_len)
{
	int rc, len;
	int arg;	// DUMMY ( NOT USED )
//	u8 wbuf[100] = {0xff, };
	u8 wbuf[200] = {0xff, };
	memset(wbuf, 0xff, sizeof(u8) * 200);
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
	wbuf[2] = payload_len & 0xff;
	wbuf[3] = (payload_len & 0xff00) >> 8;
	
	{
		int i;
		for(i=0;i<payload_len;i++){
			wbuf[4+i] = payload[i];
		}
	}
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 4+payload_len);		// host send 3+payload_len bytes
#else
	spi_func(wbuf+1, synap_rbuf, 3+payload_len);
#endif
	usleep(200000);

REDO:
	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);		// host take 199 bytes
#endif
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);		// valid length

	// 190517 추가 : 목적은 FULLRAW, HYBRID_ABS_WITH_CBC 를 출력하는것
#if 1
	if(synap_rbuf[1] == 0x00){		// redo
		goto REDO;
	}
#endif

	#if MULTI_PAYLOAD_DEBUG
	printf("len : %d\n", len);
	#endif

	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			#if MULTI_PAYLOAD_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
    		}
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit, is_first_loop;		// 값중에 0xA5 가 나올수도 있다..... 대응법은? 길이를 가지고 해야할수도, OR A5를 연속으로 읽게되면 하도록...
			exit = is_first_loop = 0;

			#if MULTI_PAYLOAD_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
        	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);	
			printf("exit : %d\n", exit);
			#endif
			do{
				#if MULTI_PAYLOAD_DEBUG
				printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	    		printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
				#endif
				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if MULTI_PAYLOAD_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}

    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{			// 현제 값이 0x5A 라면, 패킷이 끝났다는 의미인지, 그냥 값인지를 알아야 한다.
						if(*(synap_rbuf+198) == 0x5A){			// 정말로 패킷이 다 끝난건지 확인해본다
							//printf("\n 0x5A detected\n");
							exit = 1;
							break;
						}
					}
    			}
				#if MULTI_PAYLOAD_DEBUG
    			printf("\n");
				#endif
				
				is_first_loop = 1;
				memset(synap_rbuf, 0xAA, 200);	// 초기화 시킨다.
				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
				usleep(200000);

			}while(exit != 1);
			#if MULTI_PAYLOAD_DEBUG
			printf("exit : %d\n", exit);
			#endif
		}
	}
}

#define OUTPUT_BUF_DEBUG	0
void send_command_multi_payload_output_buf(u8 addr, u8 *payload, int payload_len, uint16_t *output_buf)
{
	int rc, len;
	u8 wbuf[100] = {0xff, };
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
	wbuf[2] = payload_len & 0xff;
	wbuf[3] = (payload_len & 0xff00) >> 8;
	
	{
		int i;
		for(i=0;i<payload_len;i++){
			wbuf[4+i] = payload[i];
		}
	}
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 4+payload_len);		// host send 3+payload_len bytes
#else
	spi_func(wbuf+1, synap_rbuf, 3+payload_len);
#endif
	usleep(200000);

REDO:
	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);		// host take 199 bytes
#endif
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);		// valid length

	// 190517 추가 : 목적은 FULLRAW, HYBRID_ABS_WITH_CBC 를 출력하는것
	if(synap_rbuf[1] == 0x00){		// redo
		goto REDO;
	}

	#if OUTPUT_BUF_DEBUG
	printf("len : %d\n", len);
	#endif
	
	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			int output_buf_idx = 0;
			#if OUTPUT_BUF_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
			#endif
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
				#if OUTPUT_BUF_DEBUG
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
				#endif

				if((i%2) == 1){				// 값을 저장
					if((i!=1) && (i!=3)){	// i==1일 땐 응답코드, i==3일땐 길이이다.
						output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

						#if OUTPUT_BUF_DEBUG
						printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
						#endif

						output_buf_idx++;
					}else{
						// DO NOTHING
					}
				}
    		}
			#if OUTPUT_BUF_DEBUG
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit, is_first_loop;		// 값중에 0xA5 가 나올수도 있다..... 대응법은? 길이를 가지고 해야할수도, OR A5를 연속으로 읽게되면 하도록...
			int output_buf_idx = 0;
			uint8_t save_198 = 0;
			int use_save_198 = 1;
			exit = is_first_loop = 0;
			
			#if OUTPUT_BUF_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
			printf("exit : %d\n", exit);
			#endif

			do{
				#if OUTPUT_BUF_DEBUG
	    		printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
				#endif

				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if OUTPUT_BUF_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}
					#endif

					#if OUTPUT_BUF_DEBUG
    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{			// 현제 값이 0x5A 라면, 패킷이 끝났다는 의미인지, 그냥 값인지를 알아야 한다.
						if(*(synap_rbuf+198) == 0x5A){			// 정말로 패킷이 다 끝난건지 확인해본다

							#if OUTPUT_BUF_DEBUG
							printf("\n 0x5A detected\n");
							#endif

							exit = 1;
							break;
						}
					}

					////////////// OUTPUT_BUF 저장 알고리즘 ///////////////	
					if(*(synap_rbuf + 1) == 0x01)		{		// OK 패킷일때(0xA5 0x01)
						if((i%2) == 1){				// 값을 저장
							if((i!=1) && (i!=3)){	// i==1일 땐 응답코드, i==3일땐 길이이다.
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_DEBUG
								printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;
							}else{
								// DO NOTHING
							}
						}else if(i==198){			// [198] 값을 저장한다.
							save_198 = *(synap_rbuf+198);
							use_save_198 = 1;	
							
							#if OUTPUT_BUF_DEBUG
							printf("save_198 is %d\n", save_198);
							#endif
						}
					}else if (*(synap_rbuf + 1) == 0x03){		// CONTINUE 패킷일때(0xA5 0x03)
						// 1. 짝수번에 값을 가져올지,  홀수번에 값을 가져올지는 use_save_198 변수에 의해 결정된다.
						if(use_save_198 == 1){
							if(i==2){
								#if OUTPUT_BUF_DEBUG
								printf("use save_198 : %d\n", save_198);
								#endif

								output_buf[output_buf_idx] = *(synap_rbuf+2) << 8 | save_198;					

								#if OUTPUT_BUF_DEBUG
								printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;

							}else if(i==198){
								output_buf[output_buf_idx] = *(synap_rbuf+198) << 8 | *(synap_rbuf+197);					

								#if OUTPUT_BUF_DEBUG
								printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif	

								output_buf_idx++;
								use_save_198 = 0;

							}else if(((i%2) == 0) && (i!=0)){
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_DEBUG
								printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;

							}else{
								// DO NOTHING
							}
						}else{
							if(i==198){
								#if OUTPUT_BUF_DEBUG
								printf("save_198 is %d\n", save_198);
								#endif	

								save_198 = *(synap_rbuf+198);
								use_save_198 = 1;

							}else if(((i%2) == 1) && (i!=1)){
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_DEBUG
								printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;
							}else{
								// DO NOTHING
							}
						}
					}else{
						printf("WRONG RESPONSE\n");
						exit = 1;
					}
					//////////////////////////////////////////////////////////

    			}
				
				#if OUTPUT_BUF_DEBUG
    			printf("\n");
				#endif

				is_first_loop = 1;
				memset(synap_rbuf, 0xAA, 200);	// 초기화 시킨다.
				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
				usleep(200000);
			}while(exit != 1);
			#if OUTPUT_BUF_DEBUG
			printf("exit : %d\n", exit);
			#endif
		}
	}
}

#define OUTPUT_BUF_INT16_T_DEBUG	0
void send_command_multi_payload_output_buf_int16_t(u8 addr, u8 *payload, int payload_len, int16_t *output_buf)
{
	int rc, len;
	u8 wbuf[100] = {0xff, };
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
	wbuf[2] = payload_len & 0xff;
	wbuf[3] = (payload_len & 0xff00) >> 8;
	
	{
		int i;
		for(i=0;i<payload_len;i++){
			wbuf[4+i] = payload[i];
		}
	}
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 4+payload_len);		// host send 3+payload_len bytes
#else
	spi_func(wbuf+1, synap_rbuf, 3+payload_len);
#endif
	usleep(200000);

REDO:
	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);		// host take 199 bytes
#endif
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);		// valid length

	// 190517 추가 : 목적은 FULLRAW, HYBRID_ABS_WITH_CBC 를 출력하는것
	if(synap_rbuf[1] == 0x00){		// redo
		goto REDO;
	}

	#if OUTPUT_BUF_INT16_T_DEBUG
	printf("len : %d\n", len);
	#endif
	
	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			int output_buf_idx = 0;
			#if OUTPUT_BUF_INT16_T_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
				
				if((i%2) == 1){				// 값을 저장
					if((i!=1) && (i!=3)){	// i==1일 땐 응답코드, i==3일땐 길이이다.
						output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

						#if OUTPUT_BUF_INT16_T_DEBUG
						printf("[%d] rawdata is 0x%hd\n", output_buf_idx, output_buf[output_buf_idx]);
						#endif

						output_buf_idx++;
					}else{
						// DO NOTHING
					}
				}
    		}
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit, is_first_loop;		// 값중에 0xA5 가 나올수도 있다..... 대응법은? 길이를 가지고 해야할수도, OR A5를 연속으로 읽게되면 하도록...
			int output_buf_idx = 0;
			uint8_t save_198 = 0;
			int use_save_198 = 1;
			exit = is_first_loop = 0;
			
			#if OUTPUT_BUF_INT16_T_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
			printf("exit : %d\n", exit);
			#endif

			do{
				#if OUTPUT_BUF_INT16_T_DEBUG
	    		printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
				#endif

				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if OUTPUT_BUF_INT16_T_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}
					#endif

					#if OUTPUT_BUF_INT16_T_DEBUG
    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{			// 현제 값이 0x5A 라면, 패킷이 끝났다는 의미인지, 그냥 값인지를 알아야 한다.
						if(*(synap_rbuf+198) == 0x5A){			// 정말로 패킷이 다 끝난건지 확인해본다

							#if OUTPUT_BUF_INT16_T_DEBUG
							printf("\n 0x5A detected\n");
							#endif

							exit = 1;
							break;
						}
					}

					////////////// OUTPUT_BUF 저장 알고리즘 ///////////////	
					if(*(synap_rbuf + 1) == 0x01)		{		// OK 패킷일때(0xA5 0x01)
						if((i%2) == 1){				// 값을 저장
							if((i!=1) && (i!=3)){	// i==1일 땐 응답코드, i==3일땐 길이이다.
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("[%d] rawdata is %hd\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;
							}else{
								// DO NOTHING
							}
						}else if(i==198){			// [198] 값을 저장한다.
							save_198 = *(synap_rbuf+198);
							use_save_198 = 1;	
							
							#if OUTPUT_BUF_INT16_T_DEBUG
							printf("save_198 is %d\n", save_198);
							#endif
						}
					}else if (*(synap_rbuf + 1) == 0x03){		// CONTINUE 패킷일때(0xA5 0x03)
						// 1. 짝수번에 값을 가져올지,  홀수번에 값을 가져올지는 use_save_198 변수에 의해 결정된다.
						if(use_save_198 == 1){
							if(i==2){
								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("use save_198 : %d\n", save_198);
								#endif

								output_buf[output_buf_idx] = *(synap_rbuf+2) << 8 | save_198;					

								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("[%d] rawdata is %hd\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;

							}else if(i==198){
								output_buf[output_buf_idx] = *(synap_rbuf+198) << 8 | *(synap_rbuf+197);					

								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("[%d] rawdata is %hd\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif	

								output_buf_idx++;
								use_save_198 = 0;

							}else if(((i%2) == 0) && (i!=0)){
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("[%d] rawdata is %hd\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;

							}else{
								// DO NOTHING
							}
						}else{
							if(i==198){
								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("save_198 is %d\n", save_198);
								#endif	

								save_198 = *(synap_rbuf+198);
								use_save_198 = 1;

							}else if(((i%2) == 1) && (i!=1)){
								output_buf[output_buf_idx] = *(synap_rbuf+i) << 8 | *(synap_rbuf+i-1);					

								#if OUTPUT_BUF_INT16_T_DEBUG
								printf("[%d] rawdata is %hd\n", output_buf_idx, output_buf[output_buf_idx]);
								#endif

								output_buf_idx++;
							}else{
								// DO NOTHING
							}
						}
					}else{
						printf("WRONG RESPONSE\n");
						exit = 1;
					}
					//////////////////////////////////////////////////////////

    			}
				
				#if OUTPUT_BUF_INT16_T_DEBUG
    			printf("\n");
				#endif

				is_first_loop = 1;
				memset(synap_rbuf, 0xAA, 200);	// 초기화 시킨다.
				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
				usleep(200000);
			}while(exit != 1);
			#if OUTPUT_BUF_INT16_T_DEBUG
			printf("exit : %d\n", exit);
			#endif
		}
	}
}

#define OUTPUT_BUF_UINT8_T_DEBUG	0
void send_command_multi_payload_output_buf_uint8_t(u8 addr, u8 *payload, int payload_len, uint8_t *output_buf)
{
	int rc, len;
	u8 wbuf[100] = {0xff, };
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
	wbuf[2] = payload_len & 0xff;
	wbuf[3] = (payload_len & 0xff00) >> 8;
	
	{
		int i;
		for(i=0;i<payload_len;i++){
			wbuf[4+i] = payload[i];
		}
	}
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 4+payload_len);		// host send 3+payload_len bytes
#else
	spi_func(wbuf+1, synap_rbuf, 3+payload_len);
#endif
	usleep(200000);

REDO:
	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);		// host take 199 bytes
#endif
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);		// valid length

	// 190517 추가 : 목적은 FULLRAW, HYBRID_ABS_WITH_CBC 를 출력하는것
	if(synap_rbuf[1] == 0x00){		// redo
		goto REDO;
	}

	#if OUTPUT_BUF_UINT8_T_DEBUG
	printf("len : %d\n", len);
	#endif
	
	if(4+len <= 199){		// 버퍼를 안넘는다면 그대로 수행
		if(rc > -1){
			int i;
			int output_buf_idx = 0;
			#if OUTPUT_BUF_UINT8_T_DEBUG
			printf("PRINT %d BYTES\n", 4+len);
	    	printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, len);
        	for(i = 0; i < 4+len; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    		{
    		    if(!(i % 6))
    		    	printf("\n");
    		    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
				
				if(i>=4){
					output_buf[output_buf_idx] = *(synap_rbuf+i);

					#if OUTPUT_BUF_UINT8_T_DEBUG
					printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
					#endif
					
					output_buf_idx++;
				}
    		}
    		printf("\n");
			#endif
		}
	}else{					// 버퍼를 넘는다면 199개씩 계속 읽고 출력한다, 0x5A를 만나면 끝낸다.
		if(rc > -1){

			int i, exit, is_first_loop;		// 값중에 0xA5 가 나올수도 있다..... 대응법은? 길이를 가지고 해야할수도, OR A5를 연속으로 읽게되면 하도록...
			int output_buf_idx = 0;
			uint8_t save_198 = 0;
			int use_save_198 = 1;
			exit = is_first_loop = 0;
			
			#if OUTPUT_BUF_UINT8_T_DEBUG
			printf("PRINT 199 BYTES REPEATLY TILL 0x5A FOUND\n");
			printf("exit : %d\n", exit);
			#endif

			do{
				#if OUTPUT_BUF_UINT8_T_DEBUG
	    		printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
				#endif

				for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    			{
					#if OUTPUT_BUF_UINT8_T_DEBUG
    			    if(!(i % 6)){
    			    	printf("\n");
					}
					#endif

					#if OUTPUT_BUF_UINT8_T_DEBUG
    			    printf("[%d:0x%X] ",i,*(synap_rbuf+i));
					#endif

					if(*(synap_rbuf+i) == 0x5A)	{			// 현제 값이 0x5A 라면, 패킷이 끝났다는 의미인지, 그냥 값인지를 알아야 한다.
						if(*(synap_rbuf+198) == 0x5A){			// 정말로 패킷이 다 끝난건지 확인해본다

							#if OUTPUT_BUF_UINT8_T_DEBUG
							printf("\n 0x5A detected\n");
							#endif

							exit = 1;
							break;
						}
					}

					////////////// OUTPUT_BUF 저장 알고리즘 ///////////////	
					if(*(synap_rbuf + 1) == 0x01)		{		// OK 패킷일때(0xA5 0x01)
						if(i>=4){
							output_buf[output_buf_idx] = *(synap_rbuf+i);

							#if OUTPUT_BUF_UINT8_T_DEBUG
							printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
							#endif
					
							output_buf_idx++;
						}
					}else if (*(synap_rbuf + 1) == 0x03){		// CONTINUE 패킷일때(0xA5 0x03)
						if(i>=2){
							output_buf[output_buf_idx] = *(synap_rbuf+i);
	
							#if OUTPUT_BUF_UINT8_T_DEBUG
							printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
							#endif
					
							output_buf_idx++;
						}
					}else{
						printf("WRONG RESPONSE\n");
						exit = 1;
					}
					//////////////////////////////////////////////////////////

    			}
				
				#if OUTPUT_BUF_UINT8_T_DEBUG
    			printf("\n");
				#endif

				is_first_loop = 1;
				memset(synap_rbuf, 0xAA, 200);	// 초기화 시킨다.
				wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
				spi_func(wbuf+1, synap_rbuf, 199);
#endif
				usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
				rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
				usleep(200000);
			}while(exit != 1);
			#if OUTPUT_BUF_UINT8_T_DEBUG
			printf("exit : %d\n", exit);
			#endif
		}
	}
}

#define HYBRID_ABS_DEBUG	0
// 고정길이 216 bytes를 읽는다.
void send_command_multi_payload_output_buf_for_hybrid_abs(u8 addr, u8 *payload, int payload_len, uint32_t *output_buf)
{
	int rc, len;
	u8 wbuf[100] = {0xff, };
	
	wbuf[0] = 0x01;					// converter start (ss0)
	wbuf[1] = addr;
	wbuf[2] = payload_len & 0xff;
	wbuf[3] = (payload_len & 0xff00) >> 8;
	
	{
		int i;
		for(i=0;i<payload_len;i++){
			wbuf[4+i] = payload[i];
		}
	}
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 4+payload_len);		// host send 3+payload_len bytes
#else
	spi_func(wbuf+1, synap_rbuf, 3+payload_len);
#endif
	usleep(200000);
REDO:
	wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
	spi_func(wbuf+1, synap_rbuf, 199);
#endif
	usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
	rc = read(mux_device, synap_rbuf, 199);		// host take 199 bytes
#endif
	
	len = (synap_rbuf[2]) | (synap_rbuf[3] << 8);		// valid length

	if(synap_rbuf[1] == 0x00){		// redo
		goto REDO;
	}

	#if HYBRID_ABS_DEBUG
	printf("len : %d\n", len);
	#endif
	
	if(rc > -1){
		int i, exit;		// 값중에 0xA5 가 나올수도 있다..... 대응법은? 길이를 가지고 해야할수도, OR A5를 연속으로 읽게되면 하도록...
		int output_buf_idx = 0;
		u8 save[3];
			
		#if HYBRID_ABS_DEBUG
	    printf("\n - ReadBurst_read(%d) [ADDR:%d][SIZE:%d] - ", rc, addr, 199);
		#endif

		// 216개 중 195개를 가져왔다. 이중 192개가 짝이맞고, 나머지 3개는 임시저장해둔다.
		for(i = 0; i < 199; i++)	// [A5] [RESPONSE] [LEN1] [LEN2] ................
    	{
			#if HYBRID_ABS_DEBUG
    		if(!(i % 6)){
    			printf("\n");
			}
			#endif
			#if HYBRID_ABS_DEBUG
    		printf("[%d:0x%X] ",i,*(synap_rbuf+i));
			#endif

			if((i>=7) && ((i-7)%4 == 0)){				// 값을 저장
				output_buf[output_buf_idx] = *(synap_rbuf+i) << 24 | *(synap_rbuf+i-1) << 16 | *(synap_rbuf+i-2) << 8 | *(synap_rbuf+i-3);					
				#if HYBRID_ABS_DEBUG
				printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
				#endif
				output_buf_idx++;
			}else{
				// DO NOTHING
			}
    	}
				
		#if HYBRID_ABS_DEBUG
    	printf("\n");
		#endif

		// 임시저장
		save[0] = *(synap_rbuf+196);
		save[1] = *(synap_rbuf+197);
		save[2] = *(synap_rbuf+198);

		memset(synap_rbuf, 0xAA, 200);	// 초기화 시킨다.
		wbuf[1] = 0xff;
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
		write(mux_device, wbuf, 200);	// touch-ic ready to send 199 bytes
#else
		spi_func(wbuf+1, synap_rbuf, 199);
#endif
		usleep(200000);
	
#ifndef VFOS_SITE_VIETNAM_2
//#ifdef VFOS_SITE_VIETNAM_2		// VIETNAM1 에서 잘도는지 TEST 목적
		rc = read(mux_device, synap_rbuf, 199);			// host take 199 bytes
#endif
		usleep(200000);

		// 남은 21개를 가져왔다.
		for(i = 0; i < 23; i++)
		{
			// 출력한다.
			#if HYBRID_ABS_DEBUG
    		if(!(i % 6)){
    			printf("\n");
			}
			#endif
			#if HYBRID_ABS_DEBUG
    		printf("[%d:0x%X] ",i,*(synap_rbuf+i));
			#endif

			// 값을 가공한다.
			if((i==0) | (i==1)){
				// DO NOTHING
			}else if(i==2){					// 임시저장해둔 값을 사용한다
				#if HYBRID_ABS_DEBUG
				printf("SAVE2 %x SAVE1 %x SAVE0 %x\n", save[2], save[1], save[0]);
				#endif
				output_buf[output_buf_idx] = (*(synap_rbuf+2) << 24) | ((save[2]) << 16) | ((save[1]) << 8) | (save[0]);
				#if HYBRID_ABS_DEBUG
				printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
				#endif
				output_buf_idx++;
			}else if((i%4)==2){		// 6 10 14 18 22
				output_buf[output_buf_idx] = *(synap_rbuf+i) << 24 | *(synap_rbuf+i-1) << 16 | *(synap_rbuf+i-2) << 8 | *(synap_rbuf+i-3);
				#if HYBRID_ABS_DEBUG
                printf("[%d] rawdata is 0x%x\n", output_buf_idx, output_buf[output_buf_idx]);
                #endif
                output_buf_idx++;
			}else{
				// DO NOTHING
			}
		}
	}
}


// PRODUCTION TEST 항목들 rawdata 출력할때 사용하는 코드조각모음
#if 0
	payload[0] = TEST_13;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_ABSRAW;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_BSC_CAL;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_BUMP_SHORT;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_DRT;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_DRT_DOZE;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_EXTERNAL_OSC;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_FULLRAW;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
#endif
#if 0
	payload[0] = TEST_GPIO_SHORT;	
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_HYBRID_ABS_NOISE;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_HYBRID_ABS_WITH_CBC;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_NOISE;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_NOISE_DOZE;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_OPEN_SHORT;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_PT11;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_PT12;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_PT13;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
#endif
#if 0
	payload[0] = TEST_RAW_CAP;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_RT100;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_SENSORSPEED;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_TRANS_RX_SHORT;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));

	payload[0] = TEST_TRX_GND_SHORTS;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_TRX_OPEN;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_TRX_SENSOR_OPEN;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
	
	payload[0] = TEST_TRX_TRX_SHORTS;
	send_command_multi_payload(PRODUCTION_TEST, payload, sizeof(payload));
#endif
