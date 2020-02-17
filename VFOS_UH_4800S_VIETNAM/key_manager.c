
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

#include <type.h>
#include <rs485.h>
#include <display.h>
#include <current.h>
#include <stm_touch.h>
#include <mipi_con.h>
#include <model_common.h>
#include <touch.h>


/////////////////////////
int dic_dev = 0;
int dicOpen = 0;
//////////////////////////
int DEBUG_MODE;

int first_next = 1;
int first_prev = 1;
/////////////////////////
//for interlock (about old models, like B1 ...)
int is_display_on = 0;

int flag_otp_test_result_ch1 = 0;
int flag_otp_test_result_ch2 = 0;
int flag_touch_test_result_ch1 = 0;	
int flag_touch_test_result_ch2 = 0;	
int flag_current_test_result_ch1 = 0;
int flag_current_test_result_ch2 = 0;

extern int flag_interlock;
extern int flag_judge;
extern int flag_password;

extern int password[PW_LEN];
extern int pw_value[PW_LEN];
extern int pw_idx;

char display_version[10];
char touch_version[10];
char	site_version;
char	en_modify_ori_pocb = 2;

void swap (unsigned long *a, unsigned long *b)
{
    unsigned long temp;

    temp = *a;
    *a = *b;
    *b = temp;
}

void swap_short (unsigned short *a, unsigned short *b)
{
    unsigned short temp;

    temp = *a;
    *a = *b;
    *b = temp;
}

void bubble(unsigned long *buf)
{
    int i;
    int j;

    for (i=0; i< SUM_COUNT-1; i++){
        for (j=0; j<SUM_COUNT; j++){

        if( buf[j] > buf[j+1] )
            swap( &buf[j], &buf[j+1] );
        }
    }
}

void bubble_short(unsigned long count, unsigned short *buf)
{
    int i;
    int j;

    for (i=0; i< count-1; i++){
        for (j=0; j<count; j++){

        if( buf[j] > buf[j+1] )
            swap_short( &buf[j], &buf[j+1] );
        }
    }
}


int b1_init_first = 1; //180318

int module_initialize(int id, int model_index, char *name, int onoff,int logost) // 180104
{
	char comm[100] ={0,};

	FUNC_BEGIN();
	if(onoff == ON){
		is_display_on = 1;
		sprintf(comm,"/Data/reg_init /mnt/sd/initial/register_data%d.tty",model_index);
		printf("command : %s [%s(%d) %d-initial]\n",comm,name,id, model_index);
		//system(comm);
		//if( id == B1) //temp.. need modify 180311 khl
		if((id == B1) && (b1_init_first)) //180318
		{
///test 180404
				
			printf("\n\n  ******** FIRST B1 MODULE ON ******** \n\n");
            mipi_dev_open();
            decon_dev_open();
			temp_mipi_video_reset_for_b1();
            mipi_dev_close();
            decon_dev_close();
usleep(500000);
///test 180404

			system(comm);

usleep(500000);
            mipi_dev_open();
            decon_dev_open();
            temp_mipi_video_reset_for_b1();
            mipi_dev_close();
            decon_dev_close();

usleep(500000);

			b1_init_first = 0;
		}
		else if(id != B1) //180318
		{
			b1_init_first = 1;
		}
		system(comm);

		if(logost)
		{
			if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
                sprintf(comm,"/Data/pic_view /mnt/sd/logo/%d_logo.jpg B1",model_index);
            else
                sprintf(comm,"/Data/pic_view /mnt/sd/logo/%d_logo.jpg",model_index);

	        printf("command : %s [VIEW LOGO]\n",comm);
	        system(comm);
		}
		
	}
	else if(onoff == OFF){
		is_display_on = 0;
		sprintf(comm,"/Data/reg_init /mnt/sd/initial/register_sleep_data%d.tty",model_index);
		printf("command : %s [%s(%d) %d-sleep]\n",comm,name,id,model_index);
		system(comm);
	}
	else
	{
		FUNC_END();
		return FAIL;
	}

	FUNC_END();
	return PASS;
}

unsigned char version_check_display(int id, int model_index) //180104
{
    char init_path[300] = "/mnt/sd/initial/register_data";
    char string[500];
    FILE *fp;
    char *token = NULL;
    unsigned long tmp = 0;
	char version_temp[10]={0,};

	FUNC_BEGIN();

    sprintf(init_path,"%s%d.tty",init_path,model_index);
    printf("%s : %s [MODEL:%d]searching....\n",__func__,init_path,id);

    if((fp=(fopen(init_path,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, init_path);
		FUNC_END();
        return FAIL;
    }

   while((fgets(string, 500, fp)) != NULL){
        token = strtok(string, TOKEN_SEP);
        while(token != NULL){

            if (!strcmp(token, "CMD"))
            {
				token = strtok(NULL, TOKEN_SEP);
				if(!strcmp(token,"VERSION"))
				{
				    printf("id[%d][index:%d] display version > \n",id,model_index);

				    token = strtok(NULL, TOKEN_SEP);
				    tmp = (unsigned long)strtoul(token,NULL,10);
					if(DEBUG_MODE)
				        printf("(%ld) ",tmp);
				
					sprintf(version_temp,"%ld",tmp);
					if(DEBUG_MODE)
						printf("%s \n",version_temp);
					memset(display_version,0,sizeof(display_version));
					memcpy(display_version,version_temp,sizeof(version_temp));
					printf("%s \n",display_version);

					FUNC_END();
					return 0;
				}
			}

            token = strtok(NULL, TOKEN_SEP);
		}
    }
	
    
	printf("\n");
    //printf("%s : DATA FOUND FAIL[id:%d]...\n",__func__,id);
    printf("%s : DATA FOUND FAIL[id:%d][model_index:%d]...\n",__func__,id,model_index); // 180104
	FUNC_END();
    return -1;
}

unsigned char version_check_touch_limit(void)
{
    //char tl_path[300] = "/mnt/sd/A/touch_limit_vfos2.bmp";
    char tl_path[300] = "/mnt/sd/A/touch_limit_table_vfos2.csv";
    char string[500];
    FILE *fp;
    char *token = NULL;
    unsigned long tmp = 0;
    char version_temp[10]={0,};

	FUNC_BEGIN();
    printf("%s : %s searching....\n",__func__,tl_path);

    if((fp=(fopen(tl_path,"r"))) == 0 ){
        printf("%s : cannot open %s\n", __func__, tl_path);
		FUNC_END();
        return FAIL;
    }

   while((fgets(string, 500, fp)) != NULL){
        //token = strtok(string, TOKEN_SEP);
        token = strtok(string, TOKEN_SEP_COMMA);
        while(token != NULL){
            if(!strcmp(token,"VERSION"))
            {
                printf("touch limit version > \n");

                token = strtok(NULL, TOKEN_SEP);
                tmp = (unsigned long)strtoul(token,NULL,10);
                if(DEBUG_MODE)
                    printf("(%ld) ",tmp);

                sprintf(version_temp,"%ld",tmp);
                if(DEBUG_MODE)
                    printf("%s \n",version_temp);
                memset(touch_version,0,sizeof(touch_version));
                memcpy(touch_version,version_temp,sizeof(version_temp));
                printf("%s \n",touch_version);

				FUNC_END();
                return 0;
            }

            token = strtok(NULL, TOKEN_SEP);
        }
   } 
	printf("\n");
    printf("%s : DATA FOUND FAIL...\n",__func__);
	FUNC_END();
    return -1;
}

int key_counter(MODEL id, MODEL_MANAGER *model, struct status_flag *sf){

	unsigned char uart_buf[MAX_PACKET] = {0,};
	unsigned char read_buf[MAX_PACKET] = {0,};
	int model_index = model->buf_index + 1;
	int buf_index = model->buf_index;
	struct Limit	*limit = &model->limit;
	char dir = model->dir;
    int command = sf->key_act;
	char cmd[300] ={0,};
    //for mipi_read
    unsigned char reg_buf[300];
    unsigned char PacketType = 0x39;
    int DataLen = 3;
    unsigned char RdReg;
    int ret = 0;
		FUNC_BEGIN();
    memset(uart_buf, 0, MAX_PACKET);
    memset(read_buf, 0, MAX_PACKET);
    memset(cmd, 0, sizeof(cmd));

	printf("%s : MODEL...[ID:%d/INDEX:%d][%s] \n",__func__,id,model_index,model->name);

	printf("flag_interlock %d\n", flag_interlock);
	// LWG 190401 모든 키 정지
	//if(flag_judge != 1){

	if(flag_password == 1){			// PASWORD INSERT
		// 1. RESET 키입력시 패스워드 체크
		if(command == RESET){
			int i;
			printf("\n PASSWORD CHECK\n");
			for(i=0;i<PW_LEN;i++){
				if(password[i] != pw_value[i]){
					printf("PASSWORD WRONG %d %d\n", password[i], pw_value[i]);
					flag_password = 0;
					pw_idx = -1;		// 초기화
					memset(pw_value, 0, PW_LEN); 
					return buf_index;		// 틀리면 나감
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
			printf("key %d\n", command);	
			pw_value[pw_idx] = command;
			printf("pw_value[i] %d\n", pw_value[pw_idx]);
		}
		
		return buf_index;
	}	

	view_init_for_state(command, &limit->display,model->name,sf,dir);


    if((sf->key_pre == FUNC) || (sf->key_pre == RESET))
    {
        if((command != FUNC) && (command != FUNC2) && (command != RESET))
        {
            printf("FUNC2\n");
#ifdef POCB_FIRST_READ
            if(id == DP049 || (id == AKATSUKI))
            {
                module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                dp049_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();

                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);

                module_initialize(id,model_index,model->name,ON,true);
             }
            else if(id == B1) //180316
            {
                module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                b1_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();
                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);
                module_initialize(id,model_index,model->name,ON,true);

            }
			else if((id == MV) || (id == MV_MANUAL) || (id == MV_MQA) || (id == MV_DQA))
			{
                module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                mv_joan_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();
                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);
                module_initialize(id,model_index,model->name,ON,true);
			}
            else
            {
                limit->display.ori_pocb_on_ch1 = 2; //pocb no read
                limit->display.ori_pocb_on_ch2 = 2; //pocb no read
                module_initialize(id,model_index,model->name,ON,true);
            }

#else

			limit->display.ori_pocb_on_ch1 = 2; //pocb no read
			limit->display.ori_pocb_on_ch2 = 2; //pocb no read
			module_initialize(id,model_index,model->name,ON,true);

#endif

/////////////////////////////////

            memset(uart_buf, 0, 30);
            uart_buf[4] = model_index;
            if(!version_check_display(id, model_index))
            {
                uart_buf[7] = display_version[7];
                uart_buf[8] = display_version[6];
                uart_buf[9] = display_version[5];
                uart_buf[10] = display_version[4];
                uart_buf[11] = display_version[3];
                uart_buf[12] = display_version[2];
                uart_buf[13] = display_version[1];
                uart_buf[14] = display_version[0];
                printf("%s : %s[ID:%d][index:%d] \n",__func__,display_version,id,model_index);
            }
            else
                printf("%s : Display Version FAIL[ID:%d][index:%d]\n",__func__,id,model_index);

            serial_packet_init(uart_buf, FUNC2, 0x00);
            serial_write_function(uart_buf);

///////////////////////////////////
			sf->key_pre = FUNC2;
        }
    }
//////////////////////////////
	if(DEBUG_MODE)
	{
		printf("\n\n******************************* CMD:%d [<-PRE:%d]\n",
			sf->key_act,
			sf->key_pre);
	}

	//if(flag_judge != 1){
    switch(command){
			printf("command start\nflag_interlock %d\n", flag_interlock);
        case	NEXT:
			printf("NEXT\n");
#if 1	
		// 세 테스트 통과하지 않고 NEXT 누르면(화상검사) interlock
		if(((flag_otp_test_result_ch1 != 1) || (flag_otp_test_result_ch2 != 1) 
			|| (flag_touch_test_result_ch1 != 1) || (flag_touch_test_result_ch2 != 1)
			|| (flag_current_test_result_ch1 != 1) || (flag_current_test_result_ch2 != 1))
			&& (flag_interlock)){
				printf("\n LWG judge\n");
				send_judge_status_to_uart( flag_otp_test_result_ch1,  flag_otp_test_result_ch2,
								 flag_touch_test_result_ch1,  flag_touch_test_result_ch2,
								 flag_current_test_result_ch1,  flag_current_test_result_ch2);
				flag_judge = 1;
			//	return is_exit;
				break;

		// 모든 테스트를 통과하고 NEXT 누르면(화상검사) judge 플래그를 끈다. (재 테스트시 사용됨)
		}else if((( flag_otp_test_result_ch1 == 1) && ( flag_otp_test_result_ch2 == 1) 
				&& ( flag_touch_test_result_ch1 == 1) && ( flag_touch_test_result_ch2 == 1)
				&& ( flag_current_test_result_ch1 == 1) && ( flag_current_test_result_ch2 == 1))){
				printf("\n LWG all pass\n");			
				flag_judge = 0;
		}
#endif
			if(en_modify_ori_pocb == 2)
				en_modify_ori_pocb =0;

			if((sf->key_pre == RESET) ||  (sf->key_pre == FUNC) || (sf->key_pre == OTP) || (sf->key_pre == TOUCH) || (sf->key_pre == CURRENT)) //test
			{
				module_initialize(id,model_index,model->name,ON,false);		
			}
			if((!limit->display.image_count) && (!limit->display.vod_count))
			{
				printf("%s : no image in SD [dir:%c]\n",__func__,dir);
				break;;
			}

			if(limit->display.now_img < (limit->display.image_count + limit->display.vod_count))
				limit->display.now_img++;
			else
				limit->display.now_img = 1;

            /* uart command */
            memset(uart_buf, 0,	MAX_PACKET);
            uart_buf[4] = limit->display.now_img -1;
            uart_buf[5] = dir;
            uart_buf[6] = limit->display.image_count;
            uart_buf[7] = limit->display.vod_count;
            uart_buf[10] = 0; //POCB INFO 
			serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

			if(first_next)
			{
				if((id == MV_MQA) || (id == MV_DQA))
				{
                    printf("----------------------------------\n");
                    printf("---    DSIM LINK RECOVERY!!!   ---\n");
                    printf("----------------------------------\n");
                    sprintf(cmd,"%s%d%s",RECOVERY_CMD1,model_index,RECOVERY_CMD2);
                    printf("%s\n",cmd);
                    system(cmd);

				}
                else if(((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == JOAN_E5)) && (!limit->display.joan_pwm))
                {
                        mipi_dev_open();
                        decon_dev_open();
                        memset(reg_buf, 0, sizeof(reg_buf));
                        PacketType = 0x06;
                        DataLen = 3; //temp
                        RdReg = 0xDC;
                        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
						if (ret < 0)
						{
							DERRPRINTF("mipi_read\n");
						}

                        printf("   >>>   read Data : 0x%X / 0x%X / 0x%X  \n",reg_buf[0],reg_buf[1],reg_buf[2]);
                        mipi_dev_close();
                        decon_dev_close();
                        if((reg_buf[0] == 0xEC) || (reg_buf[0] == 0xED) || (reg_buf[0] == 0xF0))
                            limit->display.joan_pwm = reg_buf[0]; //joan no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0)
                        else
                            limit->display.joan_pwm = 0;

						printf("   >>>   JOAN MODE = 0x%X (no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0) \n",limit->display.joan_pwm);
	
				}
				first_next = 0;
				first_prev = 0;
			}

			display_func(id, &limit->display, &limit->display.now_img, dir,  NEXT);

			uart_buf[5] = limit->display.st_pocb_on_ch1;
			uart_buf[6] = limit->display.st_pocb_on_ch2;
			uart_buf[7] = limit->display.ori_pocb_on_ch1;
			uart_buf[8] = limit->display.ori_pocb_on_ch2;
			uart_buf[9] = limit->display.st_pocb_wr_en;
			uart_buf[10] = 1;  //POCB INFO
			serial_packet_init(uart_buf, NEXT,0x00);
			serial_write_function(uart_buf);
			
			
			printf("Display: POCB Write Enable [%d] \n",uart_buf[9]);
			printf("Display: POCB CH1[%d] (Ori:%d)\n",uart_buf[5],uart_buf[7]);
			printf("Display: POCB CH2[%d] (Ori:%d)\n",uart_buf[6],uart_buf[8]);

			break;
		case	PREV:
			printf("PREV\n");
#if 1	
		// 세 테스트 통과하지 않고 NEXT 누르면(화상검사) interlock
		if((( flag_otp_test_result_ch1 != 1) || ( flag_otp_test_result_ch2 != 1) 
			|| ( flag_touch_test_result_ch1 != 1) || ( flag_touch_test_result_ch2 != 1)
			|| ( flag_current_test_result_ch1 != 1) || ( flag_current_test_result_ch2 != 1))
			&& (flag_interlock)){
				printf("\n LWG judge\n");
				send_judge_status_to_uart( flag_otp_test_result_ch1,  flag_otp_test_result_ch2,
								 flag_touch_test_result_ch1,  flag_touch_test_result_ch2,
								 flag_current_test_result_ch1,  flag_current_test_result_ch2);
				flag_judge = 1;
				//return is_exit;
				break;

		// 모든 테스트를 통과하고 NEXT 누르면(화상검사) judge 플래그를 끈다. (재 테스트시 사용됨)
		}else if((( flag_otp_test_result_ch1 == 1) && ( flag_otp_test_result_ch2 == 1) 
				&& ( flag_touch_test_result_ch1 == 1) && ( flag_touch_test_result_ch2 == 1)
				&& ( flag_current_test_result_ch1 == 1) && ( flag_current_test_result_ch2 == 1))){
				printf("\n LWG all pass\n");			
				flag_judge = 0;
		}
#endif
			if(en_modify_ori_pocb == 2)
				en_modify_ori_pocb = 0;

            if(sf->key_pre == RESET ||  sf->key_pre == FUNC || sf->key_pre == OTP || sf->key_pre == TOUCH || sf->key_pre == CURRENT)
            {
                module_initialize(id,model_index,model->name,ON,false);
            }
            if(!limit->display.image_count && !limit->display.vod_count)
            {
                printf("%s : no image in SD [dir:%c]\n",__func__,dir);
                break;
			}
            if(limit->display.now_img > 1)
                limit->display.now_img--;
            else
                limit->display.now_img = limit->display.image_count + limit->display.vod_count;

            /* uart command */
            memset(uart_buf, 0, MAX_PACKET);
            uart_buf[4] = limit->display.now_img-1;
            uart_buf[5] = dir;
            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

            if(first_prev)
            {


                if((id == MV_MQA) || (id == MV_DQA))
                {
                    printf("----------------------------------\n");
                    printf("---    DSIM LINK RECOVERY!!!   ---\n");
                    printf("----------------------------------\n");
                    sprintf(cmd,"%s%d%s",RECOVERY_CMD1,model_index,RECOVERY_CMD2);
                    printf("%s\n",cmd);
                    system(cmd);

                }
                else if(((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == JOAN_E5)) && (!limit->display.joan_pwm))
                {  
                        mipi_dev_open();
                        decon_dev_open();
                        memset(reg_buf, 0, sizeof(reg_buf));
                        PacketType = 0x06;
                        DataLen = 3; //temp
                        RdReg = 0xDC;
                        ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
						if (ret < 0)
						{
							DERRPRINTF("mipi_read\n");
						}

                        printf("   >>>   read Data : 0x%X / 0x%X / 0x%X  \n",reg_buf[0],reg_buf[1],reg_buf[2]);
                        mipi_dev_close();
                        decon_dev_close();
                        if((reg_buf[0] == 0xEC) || (reg_buf[0] == 0xED) || (reg_buf[0] == 0xF0))
                            limit->display.joan_pwm = reg_buf[0]; //joan no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0)
                        else
                            limit->display.joan_pwm = 0;

                        printf("   >>>   JOAN MODE = 0x%X (no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0) \n",limit->display.joan_pwm);

                }
                first_next = 0;
                first_prev = 0;

            }
			display_func(id, &limit->display,&limit->display.now_img,dir, PREV);

			uart_buf[5] = limit->display.st_pocb_on_ch1;
			uart_buf[6] = limit->display.st_pocb_on_ch2;
			uart_buf[7] = limit->display.ori_pocb_on_ch1;
			uart_buf[8] = limit->display.ori_pocb_on_ch2;
			uart_buf[9] = limit->display.st_pocb_wr_en;
			uart_buf[10] = 1;  //POCB INFO
			serial_packet_init(uart_buf, NEXT,0x00);
			serial_write_function(uart_buf);
			
			
			printf("Display: POCB Write Enable [%d] \n",uart_buf[9]);
			printf("Display: POCB CH1[%d] (Ori:%d)\n",uart_buf[5],uart_buf[7]);
			printf("Display: POCB CH2[%d] (Ori:%d)\n",uart_buf[6],uart_buf[8]);

            break;
		case	OTP:
			printf("OTP\n");
            
			//if((flag_interlock == 1) && (flag_otp_test_result == 1))	break;			// 통과한 테스트는 재수행 하지 않는다.

			if ((sf->key_pre == RESET) || (sf->key_pre == FUNC) || (sf->key_pre == TOUCH) || (sf->key_pre == CURRENT))
			{
                    module_initialize(id,model_index,model->name,ON,false);
            }

            /* uart command */
            memset(uart_buf, 0, MAX_PACKET);
            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);
			
			if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11");
			}
			
			// LWG skip pass fail
			//otp_func(id, model_index,&limit->display.st_pocb_on_ch1, &limit->display.st_pocb_on_ch2);
			flag_otp_test_result_ch1 = 0;		// 재 테스트를 위한 초기화
			flag_otp_test_result_ch2 = 0;		// 재 테스트를 위한 초기화
			otp_func(id, model_index,&limit->display.st_pocb_on_ch1, &limit->display.st_pocb_on_ch2);

			if(en_modify_ori_pocb == 2)
			{
				en_modify_ori_pocb = 1;
				limit->display.ori_pocb_on_ch1 = limit->display.st_pocb_on_ch1;
				limit->display.ori_pocb_on_ch2 = limit->display.st_pocb_on_ch2;
				printf(" > POCB Original Data SET [ %d ] \n",limit->display.ori_pocb_on_ch1);
			}

			limit->display.now_img = 0;
            first_prev = 1;
            first_next = 1;
			
			module_initialize(id,model_index,model->name,OFF,false);
			break;
		case	TOUCH:
			printf("TOUCH\n");

			//if((flag_interlock == 1) && (flag_touch_test_result == 1))	break;			// 통과한 테스트는 재수행 하지 않는다.
			
			printf("                >>> LWG <<< [%s %d] %s CALL ====== %d\n", __FILE__, __LINE__, __FUNCTION__,is_display_on);	
			if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11");
			}
			
			if ((sf->key_pre == RESET) || (sf->key_pre == FUNC) || (sf->key_pre == OTP) || (sf->key_pre == CURRENT))
			{
					module_initialize(id,model_index,model->name,ON,false);
			} //what	// LWG ????? 왜 안켜는거지??
            /* uart command */
            memset(uart_buf, 0, MAX_PACKET);
            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);
			
			// LWG fail pass skip
			// touch_func(id,model_index, limit->ptouch);
			flag_touch_test_result_ch1 = 0;
			flag_touch_test_result_ch2 = 0;
			touch_func(id,model_index, limit->ptouch);
			limit->display.now_img = 0;
            first_prev = 1;
            first_next = 1;
			// LWG 일단 여기서 다시켠다
			is_display_on = 1;
			printf("                >>> LWG <<< [%s %d] %s CALL ====== %d\n", __FILE__, __LINE__, __FUNCTION__,is_display_on);	
			module_initialize(id,model_index,model->name,OFF,false);
            break;
		case	CURRENT:
			printf("CURRENT\n");

			//if((flag_interlock == 1) && (flag_current_test_result == 1))	break;			// 통과한 테스트는 재수행 하지 않는다.

			module_initialize(id,model_index,model->name,ON,false);
            if((id == MV_MQA) || (id == MV_DQA))
            {
                printf("----------------------------------\n");
                printf("---    DSIM LINK RECOVERY!!!   ---\n");
                printf("----------------------------------\n");
                sprintf(cmd,"%s%d%s",RECOVERY_CMD1,model_index,RECOVERY_CMD2);
                printf("%s\n",cmd);
                system(cmd);

            }
            else if(((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == JOAN_E5)) && (!limit->display.joan_pwm))
            {
                 mipi_dev_open();
                 decon_dev_open();
                 memset(reg_buf, 0, sizeof(reg_buf));
                 PacketType = 0x06;
                 DataLen = 3; //temp
                 RdReg = 0xDC;
                 ret = mipi_read(PacketType, RdReg, DataLen, reg_buf);
				 if (ret < 0)
				 {
				 	DERRPRINTF("mipi_read\n");
				 }

                 printf("   >>>   read Data : 0x%X / 0x%X / 0x%X  \n",reg_buf[0],reg_buf[1],reg_buf[2]);
                 mipi_dev_close();
                 decon_dev_close();
                 if((reg_buf[0] == 0xEC) || (reg_buf[0] == 0xED) || (reg_buf[0] == 0xF0))
                     limit->display.joan_pwm = reg_buf[0]; //joan no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0)
                 else
                     limit->display.joan_pwm = 0;

                 printf("   >>>   JOAN MODE = 0x%X (no pwm : 0xEC / pwm : 0xED (joan E5 pwm : 0xF0) \n",limit->display.joan_pwm);

          }

			limit->current.joan_pwm = limit->display.joan_pwm;
            /* uart command */
            memset(uart_buf, 0, MAX_PACKET);
            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

			// LWG fail pass skip
			// 
			flag_current_test_result_ch1 = 0;
			flag_current_test_result_ch2 = 0;
			limit->display.st_aod = current_func(id, model_index, &limit->current, dir);
			limit->display.now_img = 0;
            first_prev = 1;
            first_next = 1;

			module_initialize(id,model_index,model->name,OFF,false);
            break;
		case	RESET:
			printf("RESET\n");
			printf("LWG %d %d, (%d %d) (%d %d) (%d %d)\n", 
						flag_interlock, flag_judge, 
						 flag_otp_test_result_ch1,  flag_otp_test_result_ch2,
						 flag_touch_test_result_ch1,  flag_touch_test_result_ch2, 
						 flag_current_test_result_ch1,  flag_current_test_result_ch2);
			if(flag_judge == 1){
				goto RESET;
			}

			if(is_display_on == 0){		// LWG 190328 디스플레이가 켜졌는지 확인할 방법은 module_initialize 를 이용한다. 
//				printf("\n LWG interlock\n");
//				flag_interlock = (flag_interlock==0)?1:0;
//				send_interlock_key_to_uart();
			if(flag_password == 0){
				printf("\n ENTER PASSWORD\n");
				flag_password = 1;
			}
			printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
#if 0
			}else if((( flag_otp_test_result != 1) || (flag_touch_test_result != 1) ||
					(flag_current_test_result != 1)) 
					&& (flag_interlock)){
				printf("\n LWG judge\n");
				send_judge_status_to_uart(flag_otp_test_result, flag_touch_test_result,
					flag_current_test_result);
				flag_judge = 1;
#endif
			}
			else{
RESET:
			 flag_otp_test_result_ch1 = 0;
			 flag_otp_test_result_ch2 = 0;
			 flag_touch_test_result_ch1 = 0;
			 flag_touch_test_result_ch2 = 0;
			 flag_current_test_result_ch1 = 0;
			 flag_current_test_result_ch2 = 0;
			flag_judge = 0;
			is_display_on = 0;
			en_modify_ori_pocb = 2;
			
			module_initialize(id,model_index,model->name,OFF,false);

			//what
            if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == JOAN_E5))
            {
                limit->display.joan_pwm = 0;
			}

            limit->display.ori_pocb_on_ch1 = 2; //no read
            limit->display.ori_pocb_on_ch2 = 2; //no read
            limit->display.st_pocb_on_ch1 = 2; //no read
            limit->display.st_pocb_on_ch2 = 2; //no read
            limit->display.st_pocb_wr_en = 0;

            /* uart command */
            memset(uart_buf, 0, MAX_PACKET);
            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

			sleep(4);

            memset(uart_buf, 0, MAX_PACKET);
			uart_buf[VER_INFO_MODEL_ID_BUF_NUM] = id;
			uart_buf[VER_INFO_VFOS_VER_BUF_NUM] = VFOS_VER;
			uart_buf[VER_INFO_VFOS_REV_BUF_NUM] = VFOS_REV;
			uart_buf[VER_INFO_VFOS_REV_MINOR_BUF_NUM] = VFOS_REV_MINOR;
			uart_buf[VER_INFO_TOUCH_VER_BYTE_7_BUF_NUM] = touch_version[7];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_6_BUF_NUM] = touch_version[6];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_5_BUF_NUM] = touch_version[5];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_4_BUF_NUM] = touch_version[4];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_3_BUF_NUM] = touch_version[3];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_2_BUF_NUM] = touch_version[2];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_1_BUF_NUM] = touch_version[1];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_0_BUF_NUM] = touch_version[0];
			printf("Touch_Version-> %s \n", touch_version);
			uart_buf[VER_INFO_SITE_VER_BUF_NUM] = site_version;
			printf("Site_Version-> 0x%X \n", site_version);

            serial_packet_init(uart_buf, FUNC,0x00);
            serial_write_function(uart_buf);
			if(DEBUG_MODE)
			{
				printf("mode select %d[index:%d]\n", id,model_index);
			}

			limit->display.now_img = 0;

			first_prev = 1;
			first_next = 1;
			}

			//flag_interlock = 1;		//	RESET need to interlock flag 
			break;

		case	FUNC: //model
			printf("FUNC\n");

			if(flag_judge == 1)	break;			

            if((sf->key_pre == NEXT) || (sf->key_pre == PREV) || (sf->key_pre == OTP) || (sf->key_pre == TOUCH) || (sf->key_pre == CURRENT) || (sf->key_pre == FUNC2))
            {
                printf("RESET\n");
				en_modify_ori_pocb = 2;
				view_init_for_state(command, &limit->display,model->name,sf,dir);
				module_initialize(id,model_index,model->name,OFF,false);
				if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == JOAN_E5))
				{
				    limit->display.joan_pwm = 0;
				}
	            limit->display.ori_pocb_on_ch1 = 2; //no read
	            limit->display.ori_pocb_on_ch2 = 2; //no read
	            limit->display.st_pocb_on_ch1 = 2; //no read
	            limit->display.st_pocb_on_ch2 = 2; //no read

        	    limit->display.st_pocb_wr_en = 0;
	
				/* uart command */
				memset(uart_buf, 0, MAX_PACKET);
				serial_packet_init(uart_buf, RESET,0x00);
				serial_write_function(uart_buf);
            	sleep(4);
			}
			//printf("SERIAL READ END.......\n");
			
			limit->display.now_img = 0;

            buf_index++;
            if (buf_index == en_model_count)
                buf_index = 0;

            usleep(20000);

            memset(uart_buf, 0, MAX_PACKET);
			uart_buf[VER_INFO_MODEL_ID_BUF_NUM] = sf->next_model_id;
			uart_buf[VER_INFO_VFOS_VER_BUF_NUM] = VFOS_VER;
			uart_buf[VER_INFO_VFOS_REV_BUF_NUM] = VFOS_REV;
			uart_buf[VER_INFO_VFOS_REV_MINOR_BUF_NUM] = VFOS_REV_MINOR;
			uart_buf[VER_INFO_TOUCH_VER_BYTE_7_BUF_NUM] = touch_version[7];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_6_BUF_NUM] = touch_version[6];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_5_BUF_NUM] = touch_version[5];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_4_BUF_NUM] = touch_version[4];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_3_BUF_NUM] = touch_version[3];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_2_BUF_NUM] = touch_version[2];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_1_BUF_NUM] = touch_version[1];
			uart_buf[VER_INFO_TOUCH_VER_BYTE_0_BUF_NUM] = touch_version[0];
			printf("Touch_Version-> %s \n", touch_version);
			uart_buf[VER_INFO_SITE_VER_BUF_NUM] = site_version;
			printf("Site_Version-> 0x%X \n", site_version);

            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

			if(DEBUG_MODE)
				printf("[FUNC]mode select %d[index:%d]\n", id,model_index);

            first_prev = 1;
            first_next = 1;
			
            break;

		case	FUNC2: //set
			printf("FUNC2\n");

			if(flag_judge == 1)		break;

				uart_buf[5] = 0;
				uart_buf[6] = 0;

            if((sf->key_pre == OTP) || (sf->key_pre == TOUCH) || (sf->key_pre == CURRENT))
				break;
			else if((sf->key_pre == NEXT) || (sf->key_pre == PREV))
			{
///////////////////////////
//for pocb write
				uart_buf[5] = limit->display.st_pocb_on_ch1;
				uart_buf[6] = limit->display.st_pocb_on_ch2; 
                uart_buf[7] = limit->display.ori_pocb_on_ch1;
                uart_buf[8] = limit->display.ori_pocb_on_ch2; 
                uart_buf[9] = limit->display.st_pocb_wr_en; 
				uart_buf[10] = 1;  //POCB INFO
	            serial_packet_init(uart_buf, NEXT,0x00);
    	        serial_write_function(uart_buf);

				printf("Display: POCB Write Enable [%d] \n",uart_buf[9]);
				printf("Display: POCB CH1[%d] (Ori:%d)\n",uart_buf[5],uart_buf[7]);
				printf("Display: POCB CH2[%d] (Ori:%d)\n",uart_buf[6],uart_buf[8]);
//////////////////////////
                break;
			}
			
///////////	POCB STATE Check!

#ifdef POCB_FIRST_READ

            if(id == DP049 || (id == AKATSUKI))
            {
				module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                dp049_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();

                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);

				module_initialize(id,model_index,model->name,ON,true);
             }
			else if(id == B1) //180316
            {
				module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                b1_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();
                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);
				module_initialize(id,model_index,model->name,ON,true);

            }
			else if((id == MV) || (id == MV_MANUAL) || (id == MV_MQA) || (id == MV_DQA))
			{
                module_initialize(id,model_index,model->name,ON,false);
                char str[300] = {0,};
                unsigned char   ch1_pocb = 0, ch2_pocb = 0;

                sprintf(str,"/Data/reg_init /mnt/sd/initial/register_read_data%d.tty",model_index);
                system(str);
                mipi_dev_open();
                decon_dev_open();

                mv_joan_pocb_check(id, &ch1_pocb, &ch2_pocb);

                mipi_dev_close();
                decon_dev_close();
                limit->display.ori_pocb_on_ch1 = ch1_pocb;
                limit->display.ori_pocb_on_ch2 = ch2_pocb;
                printf(">> ORIGINAL POCB STATE is [CH1:%d][CH2:%d] \n", limit->display.ori_pocb_on_ch1, limit->display.ori_pocb_on_ch2);
                module_initialize(id,model_index,model->name,ON,true);

			}
			else
			{
	            limit->display.ori_pocb_on_ch1 = 2; //pocb no read
	            limit->display.ori_pocb_on_ch2 = 2; //pocb no read
				module_initialize(id,model_index,model->name,ON,true);
			}

#else

            limit->display.ori_pocb_on_ch1 = 2; //pocb no read
            limit->display.ori_pocb_on_ch2 = 2; //pocb no read
            module_initialize(id,model_index,model->name,ON,true);

#endif

///////////////////////////////////////
///////////////////////////

            memset(uart_buf, 0, MAX_PACKET);
            //uart_buf[4] = id;
            uart_buf[4] = model_index;
            if(!version_check_display(id,model_index))
            {
                uart_buf[7] = display_version[7];
                uart_buf[8] = display_version[6];
                uart_buf[9] = display_version[5];
                uart_buf[10] = display_version[4];
                uart_buf[11] = display_version[3];
                uart_buf[12] = display_version[2];
                uart_buf[13] = display_version[1];
                uart_buf[14] = display_version[0];
                printf("%s : %s[ID:%d][index:%d] \n",__func__,display_version,id,model_index);
            }
            else
                printf("%s : Display Version FAIL[ID:%d]\n",__func__,id);

            serial_packet_init(uart_buf, command,0x00);
            serial_write_function(uart_buf);

//////////////////////////////
			break;

        default:
            break;
	} //switch(command)
//	}
    if(sf->key_act == FUNC2)
    {
        if((sf->key_pre == FUNC) || (sf->key_pre == FUNC2) || (sf->key_pre == RESET))
            sf->key_pre = sf->key_act;
    }
    else
        sf->key_pre = sf->key_act;

	FUNC_END();
	return buf_index;
}

/*
 * Name : get_vfos_version_info
 * Description : Get VFOS version information to send to UI.
 * Parameters :
 * 		int model_id
 * 		int model_index
 * 		vfos_version_info_t *info_p
 * Return value :
 */
void get_vfos_version_info(int model_id, int model_index, vfos_version_info_t *info_p)
{
	FUNC_BEGIN();

	/* copy version information from global variables to return parameters */
	if(!version_check_display(model_id, model_index))
	{
		memcpy(&info_p->display_version,display_version,VFOS_MAX_VERSION_LENGTH);
	}
	else
	{
		DERRPRINTF("version_check_display\n");
	}
	memcpy(&info_p->touch_version,touch_version,VFOS_MAX_VERSION_LENGTH);
	info_p->site_version = site_version;

	FUNC_END();
}

