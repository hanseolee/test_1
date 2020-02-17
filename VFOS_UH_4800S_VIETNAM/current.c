#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>     //usleep 
#include <stdlib.h>     //system, strtoul 
#include <mipi_con.h>

#include <rs485.h>
#include <type.h>
#include <current.h>
#include <math.h>

#define SLV_ADDR_VCC1						0x40
#define SLV_ADDR_VCC2						0x41
#define SLV_ADDR_VDDVDH						0x44
#define SLV_ADDR_VDDEL						0x45

//#define CURRENT_ERR
#include <i2c-dev.h>

/* Define of LGD offset */
#define	AKATSUKI_WHITE_VDDEL_LGD_OFFSET			100	/* 10mA offset */
#define	AKATSUKI_40PER_VDDEL_LGD_OFFSET			200	/* 20mA offset */

#define	B1_AOD_VDDVDH_LGD_OFFSET				16	/* 1.6mA offset */

short i_buf[SUM_COUNT]={0,};
short i_current = 0;
int i_voltage = 0;
int i2c_fd = 0;

extern int flag_current_test_result;

int current_text_parser(struct current_limit *cl, char *index_name)
{
	char string[500];
    FILE *fp;
    char *token = NULL;
	char file_name[50];
	char name_buf[50];
	int	find_stat = 0;

	FUNC_BEGIN();

	sprintf(name_buf,"[%s]",index_name);
	sprintf(file_name, "%s%s", CONFIG_DIR,C_LIMIT_FILE);

    if((fp=(fopen(file_name,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, file_name);
		FUNC_END();
        return FAIL;
    }

	printf("%s : %s limit parser START.. \n",__func__,name_buf);

	while((fgets(string, 500, fp)) != NULL){
		token = strtok(string, TOKEN_SEP);
		while(token != NULL){
//////////////////////////////////////////////////
			if(!strcmp(token,name_buf))
			{
				find_stat = 1;
				printf("%s : %s FOUND.. \n",__func__,index_name);
			}

			if(find_stat)
			{

                if(!strcmp(token, "IMAGE_COUNT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->pattern_count= (unsigned int)strtoul(token, NULL,10);
                    printf("IMAGE_COUNT[%d]\n",cl->pattern_count);
                }
				
				if(!strcmp(token, "1_VPNL_LIMIT"))
				{
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[0][VCC1]= (unsigned int)strtoul(token, NULL,10);
					printf("MODE1_VPNL_LIMIT[%d]\n",cl->max_current[0][VCC1]);
				}
                if(!strcmp(token, "1_VDDI_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[0][VCC2]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE1_VDDI_LIMIT[%d]\n",cl->max_current[0][VCC2]);
                }

                if(!strcmp(token, "1_VDDVDH_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[0][VDDVDH]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE1_VDDVDH_LIMIT[%d]\n",cl->max_current[0][VDDVDH]);
                }

                if(!strcmp(token, "1_VDDEL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[0][VDDEL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE1_VDDEL_LIMIT[%d]\n",cl->max_current[0][VDDEL]);
                }

                if(!strcmp(token, "1_TTL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[0][TTL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE1_TTL_LIMIT[%d]\n",cl->max_current[0][TTL]);
                }

                if(!strcmp(token, "2_VPNL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[1][VCC1]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE2_VPNL_LIMIT[%d]\n",cl->max_current[1][VCC1]);
                }

                if(!strcmp(token, "2_VDDI_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[1][VCC2]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE2_VDDI_LIMIT[%d]\n",cl->max_current[1][VCC2]);
                }
                if(!strcmp(token, "2_VDDVDH_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[1][VDDVDH]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE2_VDDVDH_LIMIT[%d]\n",cl->max_current[1][VDDVDH]);
                }

                if(!strcmp(token, "2_VDDEL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[1][VDDEL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE2_VDDEL_LIMIT[%d]\n",cl->max_current[1][VDDEL]);
                }

                if(!strcmp(token, "2_TTL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[1][TTL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE2_TTL_LIMIT[%d]\n",cl->max_current[1][TTL]);
                }


                if(!strcmp(token, "3_VPNL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[2][VCC1]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE3_VPNL_LIMIT[%d]\n",cl->max_current[2][VCC1]);
                }

                if(!strcmp(token, "3_VDDI_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[2][VCC2]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE3_VDDI_LIMIT[%d]\n",cl->max_current[2][VCC2]);
                }
                if(!strcmp(token, "3_VDDVDH_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[2][VDDVDH]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE3_VDDVDH_LIMIT[%d]\n",cl->max_current[2][VDDVDH]);
                }

                if(!strcmp(token, "3_VDDEL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[2][VDDEL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE3_VDDEL_LIMIT[%d]\n",cl->max_current[2][VDDEL]);
                }
                if(!strcmp(token, "3_TTL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[2][TTL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE3_TTL_LIMIT[%d]\n",cl->max_current[2][TTL]);
                }

				if(!strcmp(token, "4_VPNL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[3][VCC1]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE4_VPNL_LIMIT[%d]\n",cl->max_current[3][VCC1]);
                }

                if(!strcmp(token, "4_VDDI_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[3][VCC2]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE4_VDDI_LIMIT[%d]\n",cl->max_current[3][VCC2]);
                }
                if(!strcmp(token, "4_VDDVDH_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[3][VDDVDH]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE4_VDDVDH_LIMIT[%d]\n",cl->max_current[3][VDDVDH]);
                }

                if(!strcmp(token, "4_VDDEL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[3][VDDEL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE4_VDDEL_LIMIT[%d]\n",cl->max_current[3][VDDEL]);
                }
                if(!strcmp(token, "4_TTL_LIMIT"))
                {
                    token = strtok(NULL, TOKEN_SEP);
                    cl->max_current[3][TTL]= (unsigned int)strtoul(token, NULL,10);
                    printf("MODE4_TTL_LIMIT[%d]\n",cl->max_current[3][TTL]);
                }

                if(!strcmp(token, "END"))
                {
					goto ending;

				}
			}
//////////////////////////////////////////////////	
			token = strtok(NULL, TOKEN_SEP);
		}// token null while end

	} // fgets while end

	if(!find_stat)
	{
		printf("%s : Current Limit parsing FAIL END \n",__func__);
		FUNC_END();
		return FAIL;
	}

ending:
	printf("%s : Current Limit parsing END \n",__func__);
	fclose(fp);
	FUNC_END();
	return PASS;

}

int current_limit_parser(MODEL id, struct current_limit *cl)
{
	int ret = FAIL;

	FUNC_BEGIN();
	printf("%s : id = %d \n",__func__,id);
    switch(id)
    {
        case    JOAN:
            cl->id = JOAN;
			cl->pattern_count = 0;
			cl->volt_count = 4;
			ret = current_text_parser(cl, "JOAN");
			if(cl->pattern_count == 0)
				cl->pattern_count = 3;
            break;

        case    JOAN_REL:
            cl->id = JOAN_REL;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "JOAN");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

			break;
        case    JOAN_MANUAL:
            cl->id = JOAN_MANUAL;
            cl->pattern_count = 0;
            cl->volt_count = 4;
			ret = current_text_parser(cl, "JOAN");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;
        case    MV:
            cl->id = MV;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "MV");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

			break;

        case    MV_MANUAL:
            cl->id = MV_MANUAL;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "MV");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    A1:
            cl->id = A1;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "A1");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    JOAN_E5:
            cl->id = JOAN_E5;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "JOAN_E5");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    DP049:
            cl->id = DP049;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "DP049");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    B1:
            cl->id = B1;
            cl->pattern_count = 0;
            cl->volt_count = 5;	/* 4 current value + 1 TTL value */
            ret = current_text_parser(cl, "B1");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;


        case    AKATSUKI:
            cl->id = AKATSUKI;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "AKATSUKI");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    MV_MQA:
            cl->id = MV_MQA;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "MV_MQA");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    MV_DQA:
            cl->id = MV_DQA;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "MV_DQA");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    STORM:
            cl->id = STORM;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "STORM");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    DP076:
            cl->id = DP076;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "DP076");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;

            break;

        case    AOP:
            cl->id = AOP;
            cl->pattern_count = 0;
            cl->volt_count = 4;
			ret = current_text_parser(cl, "AOP");
			if(cl->pattern_count == 0)
				cl->pattern_count = 3;

			break;

		case    ALPHA:
			cl->id = ALPHA;
			cl->pattern_count = 0;
			cl->volt_count = 4;
			ret = current_text_parser(cl, "ALPHA");
			if(cl->pattern_count == 0)
				cl->pattern_count = 3;

			break;
		
		case    DP086:
            cl->id = DP086;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "DP086");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;
            break;

        case    F2:
            cl->id = F2;
            cl->pattern_count = 0;
            cl->volt_count = 4;
            ret = current_text_parser(cl, "F2");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;
            break;

		case	DP116:
            cl->id = DP116;
            cl->pattern_count = 0;
            cl->volt_count = 5;		// 190404 add VDDD
            ret = current_text_parser(cl, "DP116");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;
            break;

		case	DP150:
            cl->id = DP150;
            cl->pattern_count = 0;
            cl->volt_count = 5;		// 190404 add VDDD
            ret = current_text_parser(cl, "DP150");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;
            break;

		case	DP173:
            cl->id = DP173;
            cl->pattern_count = 0;
            cl->volt_count = 5;		// 190404 add VDDD
            ret = current_text_parser(cl, "DP173");
            if(cl->pattern_count == 0)
                cl->pattern_count = 3;
            break;

		

        default :
            printf("%s : this model id(%d) is not used \n",__func__,id);
			ret = FAIL;
            break;
    }

	FUNC_END();
	return ret;
}

unsigned short i2c_slv_addr(int index)
{
	FUNC_BEGIN();
	switch(index)
	{
		case VCC1 : 
			FUNC_END();
			return SLV_ADDR_VCC1;
        case VCC2 : 
			FUNC_END();
            return SLV_ADDR_VCC2;
        case VDDVDH : 
			FUNC_END();
            return SLV_ADDR_VDDVDH;
        case VDDEL : 
			FUNC_END();
            return SLV_ADDR_VDDEL;
		default : 
			printf("%s :  ina slv set err.. \n",__func__);
			
	}

	FUNC_END();
	return FAIL;
}


int get_cal_value(void)
{
    double min_lsb,max_pos_i;
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

unsigned long Measure_Voltage(void)
{
    unsigned long bus;
	FUNC_BEGIN();
    bus = i2c_smbus_read_word_data(i2c_fd, INA219_REG_BUS);
    #ifdef NOSWAP   
    #else
    bus = ((bus & 0xFF00) >> 8 | (bus & 0xFF)  << 8);
    #endif
    bus = (bus >> 3)*4;

    i_voltage = bus;

	FUNC_END();
    return i_voltage;
}

unsigned long Measure_Current(void)
{
    unsigned int half = 0;
    int i;
    short vl = 0;
    unsigned short conf = 0x3C1F;
    unsigned short conf_swap = (((int)conf & 0xFF00) >> 8 | ((int)conf & 0xFF)  << 8);;
	FUNC_BEGIN();
    #ifdef NOSWAP
    i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf);
    #else
    i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf_swap);
    #endif

    usleep(600);

    i_current = 0;

    for(i = 0; i < SUM_COUNT; i++)
    {
        get_cal_value();
        usleep(600);
        vl=i2c_smbus_read_word_data(i2c_fd,INA219_REG_CURRENT);
        #ifdef NOSWAP
        #else
        vl = ((vl & 0xFF00) >> 8 | (vl & 0x00FF)  << 8);
        #endif
        if(vl < 0)
            vl *= -1;
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
printf("i_current = %d \n",i_current);
    #endif
    if(i_current <= 0)  i_current = 0;

	FUNC_END();
    return i_current;
}

int current_test_pattern_on(int id,int model_index,int p_count, char dir,int pattern, int mode,char sub_mode)
{
	char comm[300] = {0,};
	int arg;
	int ret = 0;

	FUNC_BEGIN();
	if(pattern < p_count)
	{
		switch(mode)
		{
			case 1:
				arg = 1;
				if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
				{
					system("/Data/Pattern 11 B1");
				}
				else
				{
					system("/Data/Pattern 11");
				}
				printf("AOD MODE\n");
				mipi_dev_open();
				aod_control(id,model_index,&arg,sub_mode,NULL,0);
				usleep(5000);
				mipi_dev_close();
				ret = 1;
				break;
			case 0:
				break;
		}

		if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
		{
        	sprintf(comm,"/Data/pic_view /mnt/sd/%c/current/c_test_%d.jpg B1",dir,pattern+1);
		}
		else
		{
        	sprintf(comm,"/Data/pic_view /mnt/sd/%c/current/c_test_%d.jpg",dir,pattern+1);
		}
        printf("%s\n",comm);
        system(comm);
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("%s : [pattern %d] ON\n",__func__,pattern+1);
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++\n");

		if(mode == 1)
			sleep(2);

	}
	else
	{
		printf("%s : pattern num err [%d] \n",__func__,pattern+1);
		FUNC_END();
		return	FAIL;
	}

	FUNC_END();
	return ret;
}


extern int flag_current_test_result_ch1;
extern int flag_current_test_result_ch2;
int current_func(MODEL id, int model_index, struct current_limit *c_limit, char dir)
{
    int index;
    int ch;
    unsigned char uart_buf[MAX_PACKET] = {0,};
    short value = 0;
    short current = 0;
	short lgd_offset = 0;
	float temp_f = 0;
	float err_f = 0;
	int voltage = 0;
	unsigned int ttl_value = 0;
	unsigned int ttl_limit = 0;
	int flag_over_limit = 0;
    unsigned int limit =0;
	char *i2c_dev = NULL;
//	int ret = PASS;
	unsigned long funcs = 0;	
	int pattern = 0;
	unsigned short ina_slv = 0;
	int aod_st = 0;	
	int joan_pwm = 0;

	FUNC_BEGIN();
	uart_buf[CURRENT_TOTAL_PTN_COUNT_OFFSET] = c_limit->pattern_count;

    if(c_limit->volt_count > MAX_CURRENT_VOLTAGE_NUM)
    {
        printf("%s : Volt Count err[%d] \n",__func__,c_limit->volt_count);
		FUNC_END();
        return FAIL;
    }
	printf("Pattern Count [%d] Volt Count [%d] \n", c_limit->pattern_count,c_limit->volt_count);

	if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL))
	{
		if(c_limit->joan_pwm == 0xF0)
		{
			joan_pwm = 2;
		}
		else if (c_limit->joan_pwm == 0xED)
		{
			joan_pwm = 1;
		}
		else
		{
			joan_pwm = 0;
		}
	}
	else
		joan_pwm = 1;
		

	for(pattern = 0; pattern < c_limit->pattern_count; pattern++)
	{
		if((pattern == 2) && (id != A1)&& (id != DP049)&&(id != AKATSUKI))
		{
	   		aod_st = current_test_pattern_on(id,model_index,c_limit->pattern_count,dir,pattern,1,joan_pwm);

		}
		else
			current_test_pattern_on(id,model_index,c_limit->pattern_count,dir,pattern,0,joan_pwm);

		for ( ch = 1; ch < 3; ch++){
	
			if(ch == 1)
			{
				i2c_dev = "/dev/i2c-1";
				printf("\n+++++++ CH1 +++++++\n");
			}
			else if(ch == 2)
			{
				i2c_dev = "/dev/i2c-2";

				printf("\n+++++++ CH2 +++++++\n");
			}
			else
			{
				printf("%s : wrong channel [%d].. \n",__func__,ch);
//				ret = FAIL;
				continue;
			}
	
		    i2c_fd = open(i2c_dev, O_RDWR);
		    if(i2c_fd < 0)
		    {
		        printf("%s : [%s] I2C Device Open Failed..\n",__func__, i2c_dev);
//				ret = FAIL;
				continue;
	
		    }
	
		    if (ioctl(i2c_fd, I2C_FUNCS, &funcs) < 0) {
		        fprintf(stderr, "Error: Could not get the adapter "
		            "functionality matrix: %s\n", strerror(errno));
//				ret = FAIL;
				close(i2c_fd);
		        continue;
		    }
	
#define DEBUG_MSG	
			for(index = VCC1; index < c_limit->volt_count; index++)
			{
				if(index == VCC1)
				{
					printf("> VCC1 ------------ \n");
					if(id != A1)
						err_f = 0.790;
					else
						err_f = 0.506;
					temp_f = 0;
				}
				else if(index == VCC2)
				{
					printf("> VCC2 ------------ \n");

                    err_f = 0.930; // 180221
                    temp_f = 0;
				}
	            else if(index == VDDVDH)
				{
	                printf("> VDDVDH ------------ \n");
					if(id != A1)
						err_f = 0.940;
					else
						err_f = 0.805; //171127
                    temp_f = 0;
				}
	            else if(index == VDDEL)
				{
	                printf("> VDDEL ------------ \n");
					if(id != A1)
						err_f = 0.987;
					else
						err_f = 0.968; //171127
                    temp_f = 0;
				}
	            else if(index == TTL)
				{
	                printf("> TTL ------------ \n");
				}
				else
				{
	                printf("%s : Volt Index err[%d] \n",__func__,index);
//					ret = FAIL;
					continue;
				}
	
	
				ina_slv = i2c_slv_addr(index);
				if (ioctl(i2c_fd, I2C_SLAVE_FORCE, ina_slv) < 0) 
				{
					fprintf(stderr, "Error: Could not set address[reg:0x%X] \n",ina_slv);
//					ret = FAIL;
					continue;
				}

				/////////////////////////////// VOLTAGE CALCULATE
				voltage = Measure_Voltage();
	
				#ifdef DEBUG_MSG
		        printf("	V > %d\n",voltage);
			    #endif
		
				/////////////////////////////// CURRENT CALCULATE
				value = Measure_Current();
				temp_f = value * err_f;
				if( aod_st && (index == VDDEL))
				{
					current = (short)temp_f;
				//	printf("raw : %d \n",(int)current);
					current -=20;
					if(current < 0)
						current = 0;
				}
				else if((id == B1) && (index == VDDVDH) && aod_st) //add 180516 for B1 model
                {
                    #ifdef CURRENT_ERR
                    printf("Apply MODEL B1 AOD offset -13(-1.3mA) \n");
                    #endif
                    temp_f -= 13;

                    current = (short)temp_f;
                    if(current < 0)
                        current = 0;
                }
				else
				{
					current = (short)temp_f;
                    if(current < 0)
                        current = 0;
				}
					
				/* Apply LGD offset for B1 and AKATSUKI */
				if((id == B1) && (index == VDDVDH) && (pattern == 2))	/* B1, VDDVDH, AOD PTN */
				{
					lgd_offset = B1_AOD_VDDVDH_LGD_OFFSET;
#if	0	/* debug print */
					printf("B1:VDDVDH:AOD_PTN:orig_cur=(%d.%d),offset=(%d.%d),final_cur=(%d.%d)\n",current/10,current%10,lgd_offset/10,lgd_offset%10,(current-lgd_offset)/10,(current-lgd_offset)%10);	/* debug */
#endif
					current -= lgd_offset;
					if (current < 0)
					{
						current = 0;
					}
				}
				else if((id == AKATSUKI) && (index == VDDEL) && (pattern == 0))	/* AKATSUKI, VDDEL, WHITE PTN */
				{
					lgd_offset = AKATSUKI_WHITE_VDDEL_LGD_OFFSET;
#if	0	/* debug print */
					printf("AKATSUKI:VDDEL:WHITE_PTN:orig_cur=(%d.%d),offset=(%d.%d),final_cur=(%d.%d)\n",current/10,current%10,lgd_offset/10,lgd_offset%10,(current-lgd_offset)/10,(current-lgd_offset)%10);	/* debug */
#endif
					current -= lgd_offset;
					if (current < 0)
					{
						current = 0;
					}
				}
				else if((id == AKATSUKI) && (index == VDDEL) && (pattern == 1))	/* AKATSUKI, VDDEL, 40% PTN */
				{
					lgd_offset = AKATSUKI_40PER_VDDEL_LGD_OFFSET;
#if	0	/* debug print */
					printf("AKATSUKI:VDDEL:40P_PTN:orig_cur=(%d.%d),offset=(%d.%d),final_cur=(%d.%d)\n",current/10,current%10,lgd_offset/10,lgd_offset%10,(current-lgd_offset)/10,(current-lgd_offset)%10);	/* debug */
#endif
					current -= lgd_offset;
					if (current < 0)
					{
						current = 0;
					}
				}
				

				#ifdef CURRENT_ERR
				printf("0. [CH%d] [PW%d] ori current %d \n",ch,index,value);
				printf("1. [CH%d] [PW%d] float current %f \n",ch,index,temp_f);
				printf("2. [CH%d] [PW%d] modify current %d \n",ch,index,current);
				printf("3. [CH%d] [PW%d] err data %f \n",ch,index,err_f);
				#endif
                uart_buf[CURRENT_VPNL_HIGH_8_BITS_OFFSET+(index*2)] = (current >> 8) & 0xFF;
                uart_buf[CURRENT_VPNL_LOW_8_BITS_OFFSET+(index*2)] = current & 0xFF;
				printf("0x%X , 0x%X \n",uart_buf[CURRENT_VPNL_HIGH_8_BITS_OFFSET+(index*2)],uart_buf[CURRENT_VPNL_LOW_8_BITS_OFFSET+(index*2)]);
	
			    #ifdef DEBUG_MSG
		        printf("	C > %d\n",current);
			    #endif
	
				limit = c_limit->max_current[pattern][index];
			    #ifdef DEBUG_MSG
				printf("	C Limit > %d\n",limit);
		        #endif
	
				///////////////////////////////// JUDGEMENT
	
		        if(current > limit)
			    {
				    printf("[OCP] CH %2d, %d.%dmA Limit %d.%dmA \n",ch, current/10,current %10, limit/10, limit%10);
					uart_buf[CURRENT_OCP_VPNL_OFFSET+index] = 0x1;
					flag_over_limit = true;
				}
				else{
					uart_buf[CURRENT_OCP_VPNL_OFFSET+index] = 0;
				}
				///////////////////////////////// LIMIT
				
				/* calculate TTL */
				ttl_value += ((voltage * current)/1000);
			}
			/* TTL value */
			ttl_limit = c_limit->max_current[pattern][TTL];

			/* set TTL value to UART buffer */
			uart_buf[CURRENT_TTL_HIGH_8_BITS_OFFSET] = (ttl_value >> 8) & 0xFF;
			uart_buf[CURRENT_TTL_LOW_8_BITS_OFFSET] = ttl_value & 0xFF;
			#ifdef DEBUG_MSG
			if (id == B1)
			{
				printf("	TTL > %d\n",ttl_value);
				printf("	TTL Limit > %d\n",ttl_limit);
			}
			#endif

			if (ttl_value > ttl_limit)
			{
				uart_buf[CURRENT_OCP_TTL_OFFSET] = 0x1;
				#ifdef DEBUG_MSG
				if (id == B1)
				{
					printf("[TTL] CH %2d, %d.%dmV Limit %d.%dmV \n",ch, ttl_value/10,ttl_value%10,ttl_limit/10,ttl_limit%10);
				}
				#endif
			}
			else
			{
				uart_buf[CURRENT_OCP_TTL_OFFSET] = 0x0;
			}

			/* check if value is over spec */
			if (flag_over_limit == true)
			{
				if(ch == 1){
					flag_current_test_result_ch1 = 2;	
				}else{
					flag_current_test_result_ch2 = 2;	
				}

				if (ttl_value <= ttl_limit)
				{
					/* only B1 model needs to clear OCP of each current in case TTL value is not over limit - 20180706 */
					if (id ==B1)
					{
						/* clear OCP of each current to meet LGD request */
						uart_buf[CURRENT_OCP_VPNL_OFFSET] = 0;
						uart_buf[CURRENT_OCP_VDDI_OFFSET] = 0;
						uart_buf[CURRENT_OCP_VDDVDH_OFFSET] = 0;
						uart_buf[CURRENT_OCP_VDDEL_OFFSET] = 0;
					}
				}
				flag_over_limit = false;
			}else{
					if(ch == 1){
						if(flag_current_test_result_ch1 != 2){
							flag_current_test_result_ch1 = 1;
						}
					}else{
						if(flag_current_test_result_ch2 != 2){
							flag_current_test_result_ch2 = 1;
						}
					}									// PASS : 현 테스트를 통과하고, 이전테스트에서 FAIL이 나온적이 없어야한다.
			}
			/* clear ttl_value */
			ttl_value = 0;

            ///////////////////////////////// UART SEND
            /* uart command */
            uart_buf[CURRENT_PTN_NUMBER_OFFSET] = pattern;
            serial_packet_init(uart_buf, CURRENT, ch);
            serial_write_function(uart_buf);

			close(i2c_fd);
		}
	}
	FUNC_END();
	return aod_st;
}

