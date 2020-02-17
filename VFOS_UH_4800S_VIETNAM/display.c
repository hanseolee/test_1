
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
#include <mipi_con.h>
#include <mipi_dsi.h>
#include <model_common.h>


int	spcial_pattern_dependence_prev = 0; //for func of view_image or view_init_for_state

//JOAN
char file_name_image_B[300][100];
char file_name_image_C[300][100];
char file_name_image_D[300][100];
char file_name_image_E[300][100];
char file_name_image_F[300][100];
char file_name_image_G[300][100];
char file_name_image_H[300][100];
char file_name_image_I[300][100];
char file_name_image_J[300][100];
char file_name_image_K[300][100];


char Bfile_name_vod[300][100];
char Cfile_name_vod[300][100];
char Dfile_name_vod[300][100];
char Efile_name_vod[300][100];
char Ffile_name_vod[300][100];
char Gfile_name_vod[300][100];
char Hfile_name_vod[300][100];
char Ifile_name_vod[300][100];
char Jfile_name_vod[300][100];
char Kfile_name_vod[300][100];


roll_thread_t roll_thread;
pthread_mutex_t *mutex_gray_thread;
//pthread_t       id_gray_thread;





int start_gray_scan_thread(struct display_limit *dp_limit)
{
    int ret = 0;

    FUNC_BEGIN();

    /* initialize mutex */
    ret = pthread_mutex_init(&roll_thread.gray_scan_thread_mutex,NULL);
    if (ret < 0)
    {
        DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }

    ret = pthread_create(&roll_thread.id_gray_scan_thread, NULL, gray_thread, (void *)dp_limit);
    if (ret < 0)
    {
        DERRPRINTF("pthread_create(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }

	mutex_gray_thread = &roll_thread.gray_scan_thread_mutex;
    FUNC_END();

    return ret;
}

int start_dimming_thread(struct display_limit *dp_limit)
{
    int ret = 0;

    FUNC_BEGIN();
    /* initialize mutex */
    ret = pthread_mutex_init(&roll_thread.dimming_thread_mutex,NULL);
    if (ret < 0)
    {
        DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }
    ret = pthread_create(&roll_thread.id_dimming_thread, NULL, gray_thread, (void *)dp_limit);
    if (ret < 0)
    {
        DERRPRINTF("pthread_create(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }

	mutex_gray_thread = &roll_thread.dimming_thread_mutex;
    FUNC_END();

    return ret;
}

int start_dsc_roll_thread(struct display_limit *dp_limit)
{
    int ret = 0;

    FUNC_BEGIN();

    /* initialize mutex */
    ret = pthread_mutex_init(&roll_thread.dsc_roll_thread_mutex,NULL);
    if (ret < 0)
    {
        DERRPRINTF("pthread_mutex_init(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }

    ret = pthread_create(&roll_thread.id_dsc_roll_thread, NULL, gray_thread, (void *)dp_limit);
    if (ret < 0)
    {
        DERRPRINTF("pthread_create(errno=%d)\n", errno);
        FUNC_END();
        return ret;
    }

	mutex_gray_thread = &roll_thread.dsc_roll_thread_mutex;
    FUNC_END();

    return ret;
}

int receive_roll_thread_mutex(pthread_mutex_t *receive_mutex)
{
    FUNC_BEGIN();

	//receive_mutex = &mutex_gray_thread;
	receive_mutex = &roll_thread.dsc_roll_thread_mutex;

    FUNC_END();
	if(&roll_thread.dsc_roll_thread_mutex == NULL)
		return false;

	return true;
}







/*
 * Name : get_display_image_info
 * Description : Get display image file name.
 * Parameters :
 * 		char directory : directory name
 * 		char *file_name_p : buffer for file name
 * Return value : error num
 */
int get_display_image_info(char directory, char (*file_name_p)[MAX_FILE_NAME_LENGTH])
{
	FUNC_BEGIN();

	switch (directory)
	{
		case 'B':
			memcpy(file_name_p,file_name_image_B, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'C':
			memcpy(file_name_p,file_name_image_C, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'D':
			memcpy(file_name_p,file_name_image_D, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'E':
			memcpy(file_name_p,file_name_image_E, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'F':
			memcpy(file_name_p,file_name_image_F, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'G':
			memcpy(file_name_p,file_name_image_G, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'H':
			memcpy(file_name_p,file_name_image_H, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'I':
			memcpy(file_name_p,file_name_image_I, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'J':
			memcpy(file_name_p,file_name_image_J, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		case 'K':
			memcpy(file_name_p,file_name_image_K, MAX_DISPLAY_IMAGE_NUM * MAX_FILE_NAME_LENGTH);
			break;
		default:
			printf("ERR: invalid directory name (%c)\n", directory);
			FUNC_END();
			return -1;
			break;
	}

	FUNC_END();

	return 0;
}


void display_limit_parser (MODEL id,int buf_index, struct display_limit *dl, char dir)
{
	FUNC_BEGIN();
	dl->id = id;
	dl->model_index = buf_index + 1;
	dl->image_count = scan_directory(dir);
	dl->vod_count = scan_directory_vod(dir);
	dl->dir = dir;
    dl->ori_pocb_on_ch1 = 2; //pocb no read
    dl->ori_pocb_on_ch2 = 2; //pocb no read

	if(DEBUG_MODE)
		printf("%s : id[%d] / dir[%c] / img_cnt[%d] / vod_cnt[%d] \n",__func__,dl->id,dir,dl->image_count,dl->vod_count);

	FUNC_END();
}


int scan_directory(char directory){
    struct dirent **namelist;
    int n, i;
    int count=0;
    char *buf;
	FUNC_BEGIN();
    buf = (char *)malloc(100);
    sprintf(buf,"/mnt/sd/%c",directory);
    n = scandir(buf, &namelist, 0, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        for(i=0 ; i < n ; i++) {
            if(strstr(namelist[i]->d_name, "jpg") || strstr(namelist[i]->d_name, "JPG") \
            || strstr(namelist[i]->d_name, "jpeg") || strstr(namelist[i]->d_name, "JPEG") \
            || strstr(namelist[i]->d_name, "gif") || strstr(namelist[i]->d_name, "GIF") \
            || strstr(namelist[i]->d_name, "bmp") || strstr(namelist[i]->d_name, "BMP"))
            {

                if(directory == 'B')
                {
                    strcpy(file_name_image_B[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_B[i]);
                    #endif
                }
                else if(directory == 'C')
                {
                    strcpy(file_name_image_C[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_C[i]);
                    #endif
                }
                else if(directory == 'D')
                {
                    strcpy(file_name_image_D[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_D[i]);
                    #endif
                }
                else if(directory == 'E')
                {
                    strcpy(file_name_image_E[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_E[i]);
                    #endif
                }
                else if(directory == 'F')
                {
                    strcpy(file_name_image_F[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_F[i]);
                    #endif
                }
                else if(directory == 'G')
                {
                    strcpy(file_name_image_G[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_G[i]);
                    #endif
                }
                else if(directory == 'H')
                {
                    strcpy(file_name_image_H[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_H[i]);
                    #endif
                }
                else if(directory == 'I')
                {
                    strcpy(file_name_image_I[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_I[i]);
                    #endif
                }
                else if(directory == 'J')
                {
                    strcpy(file_name_image_J[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_J[i]);
                    #endif
                }
                else if(directory == 'K')
                {
                    strcpy(file_name_image_K[count++], namelist[i]->d_name);
                    #ifdef  DEBUG_MSG
                    printf("%s \n", file_name_image_K[i]);
                    #endif
                }
                else
                    printf("not collect directory \n");
            }

        }
        usleep(10);
    }

    #ifdef  DEBUG_MSG
    printf("\nTotal Image = %d\n\n", count);
    #endif
    free(buf);
	FUNC_END();
    return count;
}

int scan_directory_vod(char directory){
    struct dirent **namelist;
    int n, i;
    int count=0;
    char *buf;
	FUNC_BEGIN();
    buf=(char *)malloc(100);
    sprintf(buf,"/mnt/sd/%c",directory);

    n = scandir(buf, &namelist, 0, alphasort);

    if (n < 0)
        perror("scandir");
    else {
        for(i=0 ; i < n ; i++)
        {
            if(strstr(namelist[i]->d_name, "mp4") || strstr(namelist[i]->d_name, "MP4") \
            || strstr(namelist[i]->d_name, "avi") || strstr(namelist[i]->d_name, "AVI")) \


            {

            if(directory == 'B')
            {
                strcpy(Bfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Bfile_name_vod[i]);
                #endif
            }
            else if(directory == 'C')
            {
                strcpy(Cfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Cfile_name_vod[i]);
                #endif
            }
            else if(directory == 'D')
            {
                strcpy(Dfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Dfile_name_vod[i]);
                #endif
            }
            else if(directory == 'E')
            {
                strcpy(Efile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Efile_name_vod[i]);
                #endif
            }
            else if(directory == 'F')
            {
                strcpy(Ffile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Ffile_name_vod[i]);
                #endif
            }

            else if(directory == 'G')
            {
                strcpy(Gfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Gfile_name_vod[i]);
                #endif
            }

            else if(directory == 'H')
            {
                strcpy(Hfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Hfile_name_vod[i]);
                #endif
            }

            else if(directory == 'I')
            {
                strcpy(Ifile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Ifile_name_vod[i]);
                #endif
            }

            else if(directory == 'J')
            {
                strcpy(Jfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Jfile_name_vod[i]);
                #endif
            }
            else if(directory == 'K')
            {
                strcpy(Kfile_name_vod[count++], namelist[i]->d_name);
                #ifdef  DEBUG_MSG
                printf("%s \n", Kfile_name_vod[i]);
                #endif
            }


            else
                printf("not collect directory \n");
            }
        }
    usleep(10);
    }
    #ifdef  DEBUG_MSG
    printf("\nTotal VOD = %d\n\n", count);
    #endif

    free(buf);
	FUNC_END();
    return count;
}

int image_file_check(char *image_name){
	FUNC_BEGIN();
    if(strstr(image_name, "jpg") || \
        strstr(image_name, "JPG") || \
        strstr(image_name, "jpeg") || \
        strstr(image_name, "JPEG")){
		FUNC_END();
        return 1;
    }
	FUNC_END();
    return 0;
}

int dsc_test_pic_view(int model_id, int max_ptn, int *cur_ptn);
int dsc_test_pic_find_max(char dir, int num, char *dsc_disc);
void *gray_thread(void *arg)
{
	int mode = 0;
	struct display_limit *dp_limit = (struct display_limit *)arg;
	unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	int dsc_roll_cur_ptn = 1;
	int dsc_roll_max_ptn = 0;
	FUNC_BEGIN();
    #ifdef  DEBUG_MSG
    printf("thread on\n");
    #endif

	/*lock*/
	pthread_mutex_lock(mutex_gray_thread);

    mipi_dev_open();
    usleep(50000);
	/* DBV register in initial code*/
	found_initial_dbv(dp_limit->id, dp_limit->model_index, &reg_dbv1, &reg_dbv2);

	if(dp_limit->st_dbv_scan_mode)
	{
		dp_limit->st_bist = 1;
	}
	else if(dp_limit->st_dsc_roll_mode)
	{
		dsc_roll_max_ptn = dsc_test_pic_find_max(dp_limit->dir,dp_limit->dsc_roll_ptn_num, dp_limit->dsc_roll_buf);
		if(!dsc_roll_max_ptn)
		{
			printf("[%02d DSC PTN : %s] pattern num fail.. check dir[%c]\n",dp_limit->dsc_roll_ptn_num, dp_limit->dsc_roll_buf, dp_limit->dir);

            usleep(5000);
            mipi_dev_close();
			if((dp_limit->id == B1) ||(dp_limit->id == AKATSUKI) || (dp_limit->id == JOAN) || (dp_limit->id == MV) || (dp_limit->id == JOAN_REL) || (dp_limit->id == JOAN_MANUAL) || (dp_limit->id == MV_MANUAL) || (dp_limit->id == JOAN_E5) || (dp_limit->id == A1))
			{
				system("/Data/Pattern 12 B1");
			}
			else
			{
				system("/Data/Pattern 12");
			}
			FUNC_END();
			return 0;	
		}
	}
	else if(dp_limit->st_gray_scan_mode)
	{
		if(dp_limit->id == B1)
			bist_control(2, dp_limit->id, dp_limit->model_index, &dp_limit->st_gray_scan_mode,spcial_pattern_dependence_prev); //BIST IN

	}
	kernel_msg_in_mipi_write(0);

    /* unlock */
    pthread_mutex_unlock(mutex_gray_thread);

    while(1)
    {
	    /*lock*/
	    pthread_mutex_lock(mutex_gray_thread);

        if(dp_limit->st_gray_scan_mode) //need modify
        {	
			mode = 1;

			bist_control(1, dp_limit->id,dp_limit->model_index, &dp_limit->st_gray_scan_mode,spcial_pattern_dependence_prev);
			usleep(500);
			printf("finish grayscan [%d]  \n",dp_limit->st_gray_scan_mode);
		}
        else if(dp_limit->st_dbv_scan_mode)
        {
			mode = 2;
            dbv_control(1,dp_limit->id,dp_limit->model_index,reg_dbv1, reg_dbv2, &dp_limit->st_dbv_scan_mode,NULL,spcial_pattern_dependence_prev);

            usleep(500);
            printf("finish DBV [%d]  \n",dp_limit->st_dbv_scan_mode);

        }
		else if(dp_limit->st_dsc_roll_mode)
		{
			mode = 3;
			dp_limit->st_dsc_fin = 0;

			/////
			dsc_test_pic_view(dp_limit->id, dsc_roll_max_ptn, &dsc_roll_cur_ptn);
			/////
			if(dsc_roll_cur_ptn == 1) printf("++ ");
			else if(dsc_roll_cur_ptn == 16) printf("-- ");
			dp_limit->st_dsc_fin = 1;
			//printf("finish DSC ROLL [%d]  \n",dp_limit->st_dsc_roll_mode);
		}
		else
		{
            #ifdef  DEBUG_MSG
            printf("Thread Finish..\n");
            #endif
			kernel_msg_in_mipi_write(1);
			if(mode == 1)
			{
				bist_control(0, dp_limit->id,dp_limit->model_index, NULL,spcial_pattern_dependence_prev);
				dp_limit->st_gray_fin = 1;
				dp_limit->st_gray_scan_mode = 0;
				dp_limit->st_bist = 0;

	            printf("***********************\n");
	            printf("GRAY OUT!!!\n");
	            //printf("***********************\n");
			}
			if(mode == 2)
			{
				if(dp_limit->id == B1)
				{
					system("/Data/Pattern 11 B1");
				}
                bist_control(0, dp_limit->id,dp_limit->model_index, NULL,spcial_pattern_dependence_prev);
                dp_limit->st_bist = 0;
////// state wait..................modify..
            	dbv_control(0,dp_limit->id,dp_limit->model_index,reg_dbv1, reg_dbv2, NULL,NULL,spcial_pattern_dependence_prev);

				printf("dimming end.. dbv 0x%X/0x%X set \n",reg_dbv1,reg_dbv2);
				dp_limit->st_dbv_fin = 1;
				dp_limit->st_dbv_scan_mode = 0;
				dp_limit->st_dbv = 0;
	            printf("***********************\n");
	            printf("DBV SCAN OUT!!!\n");
	            //printf("***********************\n");

			}
			if(mode == 3)
			{
				dp_limit->st_dsc_roll_mode = 0;
				dp_limit->dsc_roll_ptn_num = 0;
				memset(dp_limit->dsc_roll_buf,0,sizeof(dp_limit->dsc_roll_buf));
                #ifdef DEBUG_MSG
                printf("***********************\n");
                printf("DSC ROLL MODE OUT!!!\n");
                printf("***********************\n");
                #endif
			}

            usleep(5000);
            mipi_dev_close();
            /* unlock */
            pthread_mutex_unlock(mutex_gray_thread);
			FUNC_END();
            return 0;
		}
        /* unlock */
        pthread_mutex_unlock(mutex_gray_thread);


	}

}

char dsc_test_pic_dir[300];
char dsc_test_file_name[300][300];
int dsc_test_pic_find_max(char dir, int num, char *dsc_disc)
{
	struct dirent **namelist;
    int n, i;
    int count=0;
    char *buf;

	FUNC_BEGIN();
	memset(dsc_test_pic_dir,0,sizeof(dsc_test_pic_dir));
	memset(dsc_test_file_name,0,sizeof(dsc_test_file_name));

	printf("%s : %d / %s \n",__func__, num, dsc_disc);
    buf = (char *)malloc(100);
    sprintf(buf,"/mnt/sd/%c/%02d_dsc/",dir,num);
    n = scandir(buf, &namelist, 0, alphasort);
    if (n < 0){
        perror("scandir");
		free(buf);
		FUNC_END();
		return FAIL;
	}
    else {
		sprintf(dsc_test_pic_dir,"%s",buf);
		printf("%s : DSC TEST DIR > %s <\n",__func__,dsc_test_pic_dir);
        for(i=0 ; i < n ; i++) {
            if(strstr(namelist[i]->d_name,dsc_disc))
			{
				strcpy(dsc_test_file_name[count++], namelist[i]->d_name);
                printf("%s \n",dsc_test_file_name[i]);
			}
		}
		printf("%s : DSC FILE COUNT > %d <\n",__func__,count);
		printf("%s : [%s]~[%s] \n",__func__,dsc_test_file_name[0],dsc_test_file_name[count-1]);
		free(buf);
		FUNC_END();
		return count;
	}
	free(buf);
	FUNC_END();
	return 0;
}

int dsc_test_pic_view(int model_id, int max_ptn, int *cur_ptn)
{
	char buf[300];
	char cmd[300];

	int ptn = *cur_ptn;

	FUNC_BEGIN();
	//printf("%s : %02d / %d \n",__func__,max_ptn, *cur_ptn);
	sprintf(buf,"%s%s",dsc_test_pic_dir,((ptn-1)>0)?dsc_test_file_name[ptn-1]:dsc_test_file_name[0]);
	//printf("%s : [%s] VIEW \n",__func__,buf);

	if ((model_id == B1) || (model_id == AKATSUKI) || (model_id == JOAN) || (model_id == MV) || (model_id == JOAN_REL) || (model_id == JOAN_MANUAL) || (model_id == MV_MANUAL) || (model_id == JOAN_E5) || (model_id == A1))
	{
		sprintf(cmd,"/Data/pic_view %s B1  > /dev/null 2>&1",buf);
	}
	else
	{
		sprintf(cmd,"/Data/pic_view %s  > /dev/null 2>&1",buf);
	}
	system(cmd);

	//printf("%s : CUR [%s] >",__func__,dsc_test_file_name[*cur_ptn-1]);
	(max_ptn > *cur_ptn)? ((*cur_ptn)++) : (*cur_ptn = 1);

	//printf(" NEXT [%s]\n",dsc_test_file_name[*cur_ptn-1]);

	FUNC_END();
	return PASS;

}

int dsc_check_command_send(MODEL  id,struct display_limit *dp_limit,char dir)
{
	unsigned char uart_buf[MAX_PACKET] ={0,};
	unsigned char ch1_dsc_flag = 0;
	unsigned char ch2_dsc_flag = 0;
	char string_buf[300];
	FUNC_BEGIN();
	printf("DSI CHECK FUNC SEND.. \n");
	if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
	{
		if(dir == 'B')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_B[dp_limit->now_img -1]);
		else if(dir == 'C')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_C[dp_limit->now_img -1]);
		else if(dir == 'D')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_D[dp_limit->now_img -1]);
		else if(dir == 'E')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_E[dp_limit->now_img -1]);
		else if(dir == 'F')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_F[dp_limit->now_img -1]);
		else if(dir == 'G')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_G[dp_limit->now_img -1]);
		else if(dir == 'H')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_H[dp_limit->now_img -1]);
		else if(dir == 'I')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_I[dp_limit->now_img -1]);
		else if(dir == 'J')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_J[dp_limit->now_img -1]);
		else if(dir == 'K')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,file_name_image_K[dp_limit->now_img -1]);
		else
		    printf("DIR FAIL \n");
	}
	else
	{
		if(dir == 'B')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_B[dp_limit->now_img -1]);
		else if(dir == 'C')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_C[dp_limit->now_img -1]);
		else if(dir == 'D')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_D[dp_limit->now_img -1]);
		else if(dir == 'E')
			sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_E[dp_limit->now_img -1]);
		else if(dir == 'F')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_F[dp_limit->now_img -1]);
		else if(dir == 'G')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_G[dp_limit->now_img -1]);
		else if(dir == 'H')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_H[dp_limit->now_img -1]);
		else if(dir == 'I')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_I[dp_limit->now_img -1]);
		else if(dir == 'J')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_J[dp_limit->now_img -1]);
		else if(dir == 'K')
		    sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,file_name_image_K[dp_limit->now_img -1]);
		else
		    printf("DIR FAIL \n");
	}
	
	joan_dsc_err_flag_check(id, dp_limit->model_index, &ch1_dsc_flag, &ch2_dsc_flag);
	printf("CH1_DSC_FLAG [0x%X] CH2_DSC_FLAG [0x%X] \n",ch1_dsc_flag, ch2_dsc_flag);
	system(string_buf);

    memset(uart_buf, 0, MAX_PACKET);
    uart_buf[4] = dp_limit->now_img -1;
    uart_buf[5] = dir;
    uart_buf[6] = dp_limit->image_count;
    uart_buf[7] = dp_limit->vod_count;
    uart_buf[10] = 1; //dsc_checking state
    uart_buf[11] = ch1_dsc_flag;
    uart_buf[12] = ch2_dsc_flag;
    serial_packet_init(uart_buf, NEXT,0x00);
    serial_write_function(uart_buf);

	FUNC_END();
    return  PASS;
}

int view_init_for_state(int command, struct display_limit *dp_limit, char *model_name, struct status_flag *sf,char dir)
{
	int id = dp_limit->id;
	int model_index = dp_limit->model_index;
	unsigned char	ori_pocb_on_ch1 = dp_limit->ori_pocb_on_ch1;
	unsigned char	ori_pocb_on_ch2 = dp_limit->ori_pocb_on_ch2;
    char *joan_pwm = &dp_limit->joan_pwm;
	int *gray_mode = &dp_limit->st_gray_scan_mode;
    int *dimming_mode = &dp_limit->st_dbv_scan_mode;
    int *dbv_mode = &dp_limit->st_dbv;
	int *vod_mode = &dp_limit->st_vod_play;
	int *bist_mode = &dp_limit->st_bist;
	int *aod_mode = &dp_limit->st_aod;
	int *vr_mode = &dp_limit->st_vr;
	int  *sleep_mode = &dp_limit->st_sleep_mode;
	unsigned char *pocb_on_ch1 = &dp_limit->st_pocb_on_ch1;
	unsigned char *pocb_on_ch2 = &dp_limit->st_pocb_on_ch2;
	int *dsc_roll_mode = &dp_limit->st_dsc_roll_mode;
	unsigned char *pocb_wr_en = &dp_limit->st_pocb_wr_en;
	int *gray_fin = &dp_limit->st_gray_fin;
	int *dbv_fin = &dp_limit->st_dbv_fin;
	int *dsc_fin = &dp_limit->st_dsc_fin;
	int *blackpoint_mode = &dp_limit->st_blackpoint;
	int *variable_mode = &dp_limit->st_variable;
	int *emcontrol_in_pwm_mode = &dp_limit->st_emconpwm;
	int *emcontrol_no_pwm_mode = &dp_limit->st_emcon;
    int *bright_line = &dp_limit->st_bright_line;
    int *black_line = &dp_limit->st_black_line;
    int *power_bright_line = &dp_limit->st_power_bright_line;
    int *power_black_line = &dp_limit->st_power_black_line;
    int *lu_50p_power_bright_line = &dp_limit->st_lu_50p_power_bright_line;
    int *lu_50p_power_black_line = &dp_limit->st_lu_50p_power_black_line;
	int *bdtest_mode = &dp_limit->st_bdtest_mode; //

	unsigned char reg_dbv1 = 0, reg_dbv2 = 0;

	FUNC_BEGIN();
	printf("%s\n",__func__);
	//printf("S : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][DSC %d][DSCF %d][DROLL %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*dsc_pt,*dsc_flag_check,*dsc_roll_mode);
	printf("S : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][POCB_WR %d][POCB1ch %d][DROLL %d][SPDR %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*pocb_wr_en,*pocb_on_ch1,*dsc_roll_mode,spcial_pattern_dependence_prev);


	if(*pocb_wr_en)
	{
		if(command == FUNC2)
		{
			printf("write pocb data.. \n");
			mipi_dev_open();
			//dsc_check
			/*180119*/
			if(*pocb_on_ch1 || *pocb_on_ch2) // pocb_on state == 1(ON) or 2(NO Read)
			{
				*pocb_on_ch1 = 0;
				*pocb_on_ch2 = 0;
				pocb_write_control(id,0,pocb_on_ch1);
				printf(" // POCB ON->OFF \n");
			}
			else
			{
                *pocb_on_ch1 = 1;
                *pocb_on_ch2 = 1;
				pocb_write_control(id,0,pocb_on_ch1);
				printf(" // POCB OFF->ON \n");
			}
					
			mipi_dev_close();
		}

		else
		{
			printf("normal..(past pt, can pocb write) \n (pocb state [%d(O:%d)] [%d(O:%d)] if pocb state modified.. change original state)\n",*pocb_on_ch1, ori_pocb_on_ch1,*pocb_on_ch2, ori_pocb_on_ch2);

			if (id == B1)
			{
				/* Nothing to do in order to avoid noise */
			}
			else if ((id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
 				system("/Data/Pattern 11"); //180403 need test khl

			usleep(6000);
			mipi_dev_open();

			if(ori_pocb_on_ch1 <2) //if First POCB READ
			{
				if(*pocb_on_ch1 != ori_pocb_on_ch1)
				{
					*pocb_on_ch1 = ori_pocb_on_ch1;
					pocb_write_control(id,1,pocb_on_ch1);
					printf(" // 1CH POCB SET ORIGINAL[O:%d] \n",ori_pocb_on_ch1);
				}
			}
			else
			{
				printf(" // 1CH POCB ORIGINAL NO Data (Fisrt NO Read) \n");
				if((command != NEXT) && (command != PREV))
				{
            		*pocb_on_ch1 = ori_pocb_on_ch1;
            		*pocb_on_ch2 = ori_pocb_on_ch2;
				}
			}
			
			if(ori_pocb_on_ch2 <2) //if First POCB READ
			{
            	if(*pocb_on_ch2 != ori_pocb_on_ch2)
            	{
            	    *pocb_on_ch2 = ori_pocb_on_ch2;
            	    pocb_write_control(id,2,pocb_on_ch2);
            	    printf(" // 2CH POCB SET ORIGINAL[O:%d] \n",ori_pocb_on_ch2);
            	}
			}
            else
			{
                printf(" // 2CH POCB ORIGINAL NO Data (Fisrt NO Read) \n");
                if((command != NEXT) && (command != PREV))
                {
                    *pocb_on_ch1 = ori_pocb_on_ch1;
                    *pocb_on_ch2 = ori_pocb_on_ch2;
                }
			}

			*pocb_wr_en = 0;
			mipi_dev_close();

//check POCB state and save state,first..
//in this function, if pocb_on state is same with original POCB state, pass...
// else, use pocb_write_control function for original POCB state
		}
	}
	else
	{
		printf("normal..(past pt, can NOT pocb write) \n (pocb state [%d(O:%d)] [%d(O:%d)] if pocb state modified.. change original state)\n",*pocb_on_ch1, ori_pocb_on_ch1,*pocb_on_ch2, ori_pocb_on_ch2);
		if (command != FUNC2)
		{
			if (id == B1)
			{
				/* Nothing to do in order to avoid the noise */
			}
			else if ((id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
				system("/Data/Pattern 11"); // 180403 need test khl

			usleep(6000);
			mipi_dev_open();
			
			if(ori_pocb_on_ch1 <2) //if First POCB READ
			{
				if(*pocb_on_ch1 != ori_pocb_on_ch1)
				{
				    *pocb_on_ch1 = ori_pocb_on_ch1;
				    pocb_write_control(id,1,pocb_on_ch1);
				    printf(" // 1CH POCB SET ORIGINAL[O:%d] \n",ori_pocb_on_ch1);
				}
			}
			else
			{
				printf(" // 1CH POCB ORIGINAL NO Data (Fisrt NO Read) \n");
			    if((command != NEXT) && (command != PREV))
			    {
			        *pocb_on_ch1 = ori_pocb_on_ch1;
			    }
			}
			
			
			if(ori_pocb_on_ch2 <2) //if First POCB READ
			{
				if(*pocb_on_ch2 != ori_pocb_on_ch2)
				{
				    *pocb_on_ch2 = ori_pocb_on_ch2;
				    pocb_write_control(id,2,pocb_on_ch2);
					printf(" // 2CH POCB SET ORIGINAL[O:%d] \n",ori_pocb_on_ch2);
				}
			}
			else
			{
				printf(" // 2CH POCB ORIGINAL NO Data (Fisrt NO Read) \n");
			   	if((command != NEXT) && (command != PREV))
			    {
			    	*pocb_on_ch2 = ori_pocb_on_ch2;
			    }
			}
			
			mipi_dev_close();
		
//check POCB state and save state,first..
//in this function, if pocb_on state is same with original POCB state, pass...
// else, use pocb_write_control function for original POCB state
		}
		else
        {
			//printf("E : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][DSC %d][DSCF %d][DROLL %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*dsc_pt,*dsc_flag_check,*dsc_roll_mode);
    		printf("E : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][POCB_WR %d][POCB1ch %d][DROLL %d][SPDR %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*pocb_wr_en,*pocb_on_ch1,*dsc_roll_mode,spcial_pattern_dependence_prev);

			FUNC_END();
			return 0;
		}
	}

	if(*emcontrol_in_pwm_mode)
	{
        mipi_dev_open();
		*emcontrol_in_pwm_mode = 0;
        printf("EM CONTROL..IN PWM\n");
        emcon_control(id,emcontrol_in_pwm_mode,1,NULL,spcial_pattern_dependence_prev);
        mipi_dev_close();
	}
	else if(*emcontrol_no_pwm_mode)
	{
        mipi_dev_open();
		*emcontrol_no_pwm_mode = 0;
        printf("EM CONTROL..NO PWM\n");
        emcon_control(id,emcontrol_no_pwm_mode,0,NULL,spcial_pattern_dependence_prev);
        mipi_dev_close();
	}
	else if(*vod_mode)
	{
		printf("vod player END \n");
	}
	else if(*gray_mode)
	{
		if(command != FUNC2)
		{
			mipi_dev_open();
			
			*gray_mode = 0;
			while(!*gray_fin)
			{
			    if(*gray_fin)    break;
			    usleep(300);
			}
			printf("***********************\n");
			if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11");
			}
			usleep(500000);
			pthread_join(roll_thread.id_gray_scan_thread,NULL);
			mutex_gray_thread = NULL;
			usleep(500);
			if(*bist_mode)
			{
				bist_control(0, id,model_index, NULL,spcial_pattern_dependence_prev);
				*bist_mode = 0;
				printf("G.S : BIST END\n");
			}
			mipi_dev_close();
		}

	}
	else if(*dimming_mode)
	{
		if(command != FUNC2)
		{
			mipi_dev_open();
			*dimming_mode = 0;
			while(!*dbv_fin)
			{
				if(*dbv_fin)    break;
			    usleep(500);
			}
			printf("***********************\n");
	
			if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
			{
				system("/Data/Pattern 11 B1");
			}
			else
			{
				system("/Data/Pattern 11"); //180312
			}
			usleep(500000);
            pthread_join(roll_thread.id_dimming_thread,NULL);
            mutex_gray_thread = NULL;
			usleep(500);
			if(*bist_mode)
			{

				bist_control(0, id,model_index, NULL ,spcial_pattern_dependence_prev);
				*bist_mode = 0;
				printf("G.S : BIST END\n");
			}
			if(*dbv_mode)
			{
			    /* DBV register in initial code*/
			    found_initial_dbv(dp_limit->id, dp_limit->model_index, &reg_dbv1, &reg_dbv2);
				dbv_control(0, id,model_index,reg_dbv1,reg_dbv2,NULL,NULL,spcial_pattern_dependence_prev);
				*dbv_mode = 0;
			    printf("D.M : DBV END\n");
			}
			mipi_dev_close();
		}
	}
	else if(*dbv_mode)
	{
		mipi_dev_open();

        /* DBV register in initial code*/
        found_initial_dbv(dp_limit->id, dp_limit->model_index, &reg_dbv1, &reg_dbv2);
        dbv_control(0, id,model_index,reg_dbv1,reg_dbv2,NULL,NULL,spcial_pattern_dependence_prev);
        *dbv_mode = 0;
        printf("DBV END\n");
		mipi_dev_close();

	}
	else if(*dsc_roll_mode)
	{
        if(command != FUNC2)
        {
		    /* unlock */
		    pthread_mutex_unlock(mutex_gray_thread);
            *dsc_roll_mode = 0;
		    /* lock */
		    pthread_mutex_lock(mutex_gray_thread);
            while(!*dsc_fin)
            {
                if(*dsc_fin)    break;
                usleep(500);
            }
            pthread_join(roll_thread.id_dsc_roll_thread,NULL);
            mutex_gray_thread = NULL;
            usleep(500);
        }
	}
	else if(*aod_mode)
	{
		mipi_dev_open();

		*aod_mode = 0;
        if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL))
			aod_control(id,model_index,aod_mode,*joan_pwm,NULL,spcial_pattern_dependence_prev);
		else
			aod_control(id,model_index,aod_mode,1,NULL,spcial_pattern_dependence_prev);

		printf("AOD END\n");
		mipi_dev_close();
	}
    else if(*vr_mode)
    {

		if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
		{
			system("/Data/Pattern 11 B1");
		}
		else
		{
			system("/Data/Pattern 11");
		}
		usleep(500000);
		mipi_dev_open();

	    *vr_mode = 0;
        if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL))
            vr_control(id,model_index,vr_mode,*joan_pwm,NULL,spcial_pattern_dependence_prev);
        else
            vr_control(id,model_index,vr_mode,1,NULL ,spcial_pattern_dependence_prev);

		printf("VR END\n");
		mipi_dev_close();
    }

	else if(*sleep_mode)
	{
        mipi_dev_open();
        *sleep_mode = 0;
        sleep_control(id,model_index,*sleep_mode ,spcial_pattern_dependence_prev);
        printf("SLEEP END\n");
        mipi_dev_close();
	}
    else if(*blackpoint_mode)
    {
        mipi_dev_open();

        *blackpoint_mode = 0;
		blackpoint_control(id,model_index, blackpoint_mode,NULL ,spcial_pattern_dependence_prev);

        printf(">>>>> BLACKPOINT END\n");
        mipi_dev_close();
    }
    else if(*variable_mode)
    {
        mipi_dev_open();
	
        *variable_mode = 0;

		variable_control(id,model_index,variable_mode,NULL,spcial_pattern_dependence_prev);
	
        printf(">>>>> VARIABLE END\n");
        mipi_dev_close();
    }
    else if(*bright_line)
    {
        mipi_dev_open();
		*bright_line = 0;
        bright_line_control(id,model_index, bright_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> BRIGHT LINE END\n");
        mipi_dev_close();
    }
    else if(*black_line)
    {
        mipi_dev_open();
		*black_line = 0;
        black_line_control(id, model_index,  black_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> BLACK LINE END\n");
        mipi_dev_close();
    }
    else if(*power_bright_line)
    {
        mipi_dev_open();
        *power_bright_line = 0;
        power_bright_line_control(id, model_index,power_bright_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> POWER BRIGHT LINE END\n");
        mipi_dev_close();
    }
    else if(*power_black_line)
    {
        mipi_dev_open();
        *power_black_line = 0;
        power_black_line_control(id, model_index,power_black_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> POWER BLACK LINE ON\n");
        mipi_dev_close();
    }
    else if(*lu_50p_power_bright_line)
    {
        mipi_dev_open();
        *lu_50p_power_bright_line = 0;
        luminance_50per_power_bright_line_control(id, model_index,lu_50p_power_bright_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> 50Per LUMINANCE POWER BRIGHT LINE OFF\n");
        mipi_dev_close();
    }
    else if(*lu_50p_power_black_line)
    {
        mipi_dev_open();
        *lu_50p_power_black_line = 0;
        luminance_50per_power_black_line_control(id, model_index,lu_50p_power_black_line,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> 50Per LUMINANCE POWER BLACK LINE OFF\n");
        mipi_dev_close();
    }
    else if(*bdtest_mode)
    {
        mipi_dev_open();
        *bdtest_mode = 0;
        border_test_control(id, model_index,bdtest_mode,NULL ,spcial_pattern_dependence_prev);
        printf(">>>>> BORDER TEST MODE OFF\n");
        mipi_dev_close();
    }

	else
		printf("%s : init is not need.. \n",__func__);

    //printf("E : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][DSC %d][DSCF %d][DROLL %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*dsc_pt,*dsc_flag_check,*dsc_roll_mode);
	spcial_pattern_dependence_prev = 0;
    printf("E : StateDP > [VOD %d][G.S %d][D.M %d][DBV %d][BIST %d][AOD %d][POCB_WR %d][POCB1ch %d][DROLL %d],[SPDR %d]\n",*vod_mode,*gray_mode,*dimming_mode,*dbv_mode,*bist_mode, *aod_mode,*pocb_wr_en,*pocb_on_ch1,*dsc_roll_mode,spcial_pattern_dependence_prev);

	FUNC_END();
	return 0;
}

int view_image(MODEL id, char *src,struct display_limit *dp_limit, char dir, int command)
{
    char string_buf[200] = {0,};
    char *token;
    char buf[255];
	unsigned long pic_num = 0;
	int model_index = dp_limit->model_index;
		
	int st_already_pic_view = 0;

	char *joan_pwm = &dp_limit->joan_pwm;
	int *gray_mode = &dp_limit->st_gray_scan_mode;
	int *dimming_mode = &dp_limit->st_dbv_scan_mode;
    int *gray_fin = &dp_limit->st_gray_fin;
    int *dbv_fin = &dp_limit->st_dbv_fin;
	int *dbv_mode = &dp_limit->st_dbv;
	int dbv_level = 0; 
	int *aod_mode = &dp_limit->st_aod; //
	int *bdtest_mode = &dp_limit->st_bdtest_mode; //
	int *vr_mode = &dp_limit->st_vr; //
	unsigned char *pocb_wr_en = &dp_limit->st_pocb_wr_en; //
	int *sleep_mode = &dp_limit->st_sleep_mode; //
	int *dsc_roll_mode = &dp_limit->st_dsc_roll_mode; //
	int *dsc_roll_pic_num = &dp_limit->dsc_roll_ptn_num; //
	int *blackpoint_mode = &dp_limit->st_blackpoint;
	int *variable_mode = &dp_limit->st_variable;
	int *emcontrol_in_pwm_mode = &dp_limit->st_emconpwm;
	int *emcontrol_no_pwm_mode = &dp_limit->st_emcon;
	int *bright_line = &dp_limit->st_bright_line;
	int *black_line = &dp_limit->st_black_line;
    int *power_bright_line = &dp_limit->st_power_bright_line;
    int *power_black_line = &dp_limit->st_power_black_line;
    int *lu_50p_power_bright_line = &dp_limit->st_lu_50p_power_bright_line;
    int *lu_50p_power_black_line = &dp_limit->st_lu_50p_power_black_line;

    int grad_mode = 0;
    int grad_red = 0;
    int grad_rgb = 0;
	int aod_emma2_mode = 0;

	FUNC_BEGIN();

    memset(buf, 0, sizeof(buf));
    strcpy(buf, src);

	spcial_pattern_dependence_prev = 0;

	printf("PIC : %s \n",src);
//////////////////////////////////////////////////////////////////////

    token = strtok(buf, TOKEN_SEP_UNDER_POINT);
	printf("buf : %s / ",token);
	pic_num = strtoul(token,NULL,10);
	//*dsc_pt = 1;
	*pocb_wr_en = 1;


    while(token != NULL)
    {

        if((!strcmp(token, "scan")) || (!strcmp(token, "Scan"))){
			*gray_mode = 1;
			*pocb_wr_en = 0;
	
            #ifdef  DEBUG_MSG
            printf("GRAY_SCAN FILE\n");
            #endif
            break;
        }
        else if((!strcmp(token, "fbrightline")) || (!strcmp(token, "FBRIGHTLINE"))){

                *power_bright_line = 1;
				*pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("POWER BRIGHT LINE FILE\n");
                #endif
        }
        else if((!strcmp(token, "fblackline")) || (!strcmp(token, "FBLACKLINE"))){

                *power_black_line = 1;
				*pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("POWER BLACK LINE FILE\n");
                #endif
        }
        else if((!strcmp(token, "fbrightline50")) || (!strcmp(token, "FBRIGHTLINE50"))){

                *lu_50p_power_bright_line = 1;
				*pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("50Per LUMINANCE POWER BRIGHT LINE FILE\n");
                #endif
        }
        else if((!strcmp(token, "fblackline50")) || (!strcmp(token, "FBLACKLINE50"))){

                *lu_50p_power_black_line = 1;
				*pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("50Per LUMINANCE POWER BLACK LINE FILE\n");
                #endif
        }
        else if((!strcmp(token, "blackpoint")) || (!strcmp(token, "BLACKPOINT"))){

	            *blackpoint_mode = 1;
	            *pocb_wr_en = 0;
	            #ifdef  DEBUG_MSG
	            printf("BLACK_POINT FILE\n");
	            #endif
        }
        else if((!strcmp(token, "brightline")) || (!strcmp(token, "BRIGHTLINE"))){

                *bright_line = 1;
                *pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("BRIGHT LINE FILE\n");
                #endif
        }
        else if((!strcmp(token, "blackline")) || (!strcmp(token, "BLACKLINE"))){

                *black_line = 1;
                *pocb_wr_en = 0;
                #ifdef  DEBUG_MSG
                printf("BLACK LINE FILE\n");
                #endif
        }

        else if((!strcmp(token, "variable")) || (!strcmp(token, "VARIABLE"))){

				*variable_mode = 1;
				*pocb_wr_en = 0;
	            #ifdef  DEBUG_MSG
	            printf("VARIABLE FILE\n");
	            #endif
		}
		else if((!strcmp(token, "emconpwm")) || (!strcmp(token, "EMCONPWM"))){
		
				*emcontrol_in_pwm_mode = 1;
		        *pocb_wr_en = 0;
		        #ifdef  DEBUG_MSG
		        printf("EM CONTROL..IN PWM FILE\n");
		        #endif
        }
        else if((!strcmp(token, "emcon")) || (!strcmp(token, "EMCON"))){
        
	            *emcontrol_no_pwm_mode = 1;
	            *pocb_wr_en = 0;
	            #ifdef  DEBUG_MSGN
	            printf("EM CONTROL..NOT PWM FILE\n");
	            #endif
        }
        else if((!strcmp(token, "dim")) || (!strcmp(token, "Dim"))){
            *dimming_mode = 1;
            *dbv_mode = 1;
			*pocb_wr_en = 0;

            #ifdef  DEBUG_MSG
            printf("DIMMING FILE\n");
            #endif
        }
		else if((!strcmp(token,"dbv10")) || (!strcmp(token,"DBV10"))){
            *dimming_mode = 0;
			*dbv_mode = 1;
			dbv_level = 10;
			*pocb_wr_en = 0;
		}
		else if((!strcmp(token,"dbvvariable")) || (!strcmp(token,"DBVVARIABLE"))){
            *dimming_mode = 0;
			*dbv_mode = 1;
			dbv_level = 180;
			*pocb_wr_en = 0;
		}
        else if((!strcmp(token, "aod")) || (!strcmp(token, "AOD"))){

            *aod_mode = 1;
			*pocb_wr_en = 0;
            #ifdef  DEBUG_MSG
            printf("AOD FILE\n");
            #endif
        }
        else if((!strcmp(token, "vr")) || (!strcmp(token, "VR"))){

            *vr_mode = 1;
			*pocb_wr_en = 0;
            #ifdef  DEBUG_MSG
            printf("VR FILE\n");
            #endif
        }
        else if((!strcmp(token, "sleep")) || (!strcmp(token, "Sleep"))){

            *sleep_mode = 1;
			*pocb_wr_en = 0;
            #ifdef  DEBUG_MSG
            printf("SLEEP FILE\n");
            #endif
        }
        else if((!strcmp(token, "dsc")) || (!strcmp(token, "DSC"))){
			memset(dp_limit->dsc_roll_buf,0,sizeof(dp_limit->dsc_roll_buf));
            *dsc_roll_mode = 1;
			*dsc_roll_pic_num = pic_num;
			*pocb_wr_en = 0;
			sprintf(dp_limit->dsc_roll_buf,"%s",strtok(NULL,"."));
			printf("%s : dsc_roll_mode disc -> [%s] \n",__func__,dp_limit->dsc_roll_buf); 
            #ifdef  DEBUG_MSG
            printf("DSC ROLL FILE\n");
            #endif
        }
        else if((!strcmp(token, "BORDERTEST")) || (!strcmp(token, "bordertest"))){

            *bdtest_mode = 1;
            *pocb_wr_en = 0;
            #ifdef  DEBUG_MSG
            printf("BORDER TEST FILE\n");
            #endif
        }
        else if((!strcmp(token, "grad")) || (!strcmp(token, "GRAD")))
        {
				grad_mode = 1;
        }
		else if((!strcmp(token,"ifprev")) || (!strcmp(token,"IFPREV")))
		{
			if(command == PREV)
			{
				/*only in this point, can modify 'spcial_pattern_dependence_prev'*/
				spcial_pattern_dependence_prev = 1;
				printf("> Other code in return key\n");
			}
			printf("> Special Pattern to have Dependence code in Retern Key\n");
		}

		if (*aod_mode)
		{
            if((!strcmp(token, "emma2")) || (!strcmp(token, "EMMA2")))
            {
                aod_emma2_mode = 1;
				printf("\nNew AOD mode for EMMA(JOAN_E5)\n");
            }
		}

        if(grad_mode)
        {
            if((!strcmp(token, "rgb")) || (!strcmp(token, "RGB")))
            {
                grad_red = 0;
                grad_rgb = 1;
            }
            else if((!strcmp(token, "red")) || (!strcmp(token, "RED")))
            {
                grad_red = 1;
                grad_rgb = 0;
            }

            if(grad_rgb)
            {
                if((!strcmp(token, "ht")) || (!strcmp(token, "HT")))
                {
                    printf("RGB HORIZONTAL TOP\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 14 B1");
					}
					else
					{
                    	system("/Data/Pattern 14");
					}
                }
                else if((!strcmp(token, "hd")) || (!strcmp(token, "HD")))
                {
                    printf("RGB HORIZONTAL DOWN\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 14-1 B1");
					}
					else
					{
                    	system("/Data/Pattern 14-1");
					}
                }
                else if((!strcmp(token, "vl")) || (!strcmp(token, "VL")))
                {
                    printf("RGB VERTICAL LEFT\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 15 B1");
					}
					else
					{
                    	system("/Data/Pattern 15");
					}
                }
                else if((!strcmp(token, "vr")) || (!strcmp(token, "VR")))
                {
                    printf("RGB VERTICAL RIGHT\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 15-1 B1");
					}
					else
					{
                    	system("/Data/Pattern 15-1");
					}
                }
            }

            if(grad_red)
            {
                if((!strcmp(token, "ht")) || (!strcmp(token, "HT")))
                {
                    printf("RED HORIZONTAL TOP\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 13-3 B1");
					}
					else
					{
                    	system("/Data/Pattern 13-3");
					}
                }
                else if((!strcmp(token, "hd")) || (!strcmp(token, "HD")))
                {
                    printf("RED HORIZONTAL DOWN\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 13-2 B1");
					}
					else
					{
                    	system("/Data/Pattern 13-2");
					}
                }
                else if((!strcmp(token, "vl")) || (!strcmp(token, "VL")))
                {
                    printf("RGB VERTICAL LEFT\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 15 B1");
					}
					else
					{
                    	system("/Data/Pattern 15");
					}
                }
                else if((!strcmp(token, "vr")) || (!strcmp(token, "VR")))
                {
                    printf("RGB VERTICAL RIGHT\n");
					if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
					{
                    	system("/Data/Pattern 15-1 B1");
					}
					else
					{
                    	system("/Data/Pattern 15-1");
					}
                }
            }

        }

        token = strtok(NULL, TOKEN_SEP_UNDER_POINT);
		printf("%s / ",token);
	}
	printf("\n");
//////////////////////

	if((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
	{
    	sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,src);
	}
	else
	{
    	sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,src);
	}

    if(image_file_check(src)){
        #ifdef  DEBUG_MSG
        printf("JPG File : %s\n", string_buf);
        #endif
    }
    else{
        #ifdef  DEBUG_MSG
        printf("BMP File : %s\n", string_buf);
        #endif
    }

    if(st_is_remain_code_after_next_pt)
    {
        unsigned char reg_buf[30];
        unsigned char PacketType = 0x39;
        printf("-------------------------------------\n");
        printf("> input : OUT Remain Code, [0x%X]\n",st_is_remain_code_after_next_pt);

		if((!(*dimming_mode)) && (!(*dsc_roll_mode)) && (!(*gray_mode)) && (!(*sleep_mode)))
	        system(string_buf);

        mipi_dev_open();
 //test - 180515
        if(st_is_remain_code_after_next_pt & (1<<B1_AOD_OFF))
        { 
            printf(">> B1 model AOD OFF Remain Code\n");
   
            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x05;
            reg_buf[0] = 0x13;
            mipi_write(PacketType, reg_buf, 1);
   
            usleep(50000);

        }
        else if(st_is_remain_code_after_next_pt & (1<<B1_VARIABLE_OFF))

//        if(st_is_remain_code_after_next_pt & (1<<B1_VARIABLE_OFF)) // test - 180515
        {

            printf(">> B1 model VARIABLE OFF Remain Code\n");

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

            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x15;
            reg_buf[0] = 0xB0;
            reg_buf[1] = 0xCA;
            mipi_write(PacketType, reg_buf, 2);

        }
        else if(st_is_remain_code_after_next_pt & (1<<B1_VR_OFF))
        {
            printf(">> B1 model VR OFF Remain Code\n");

            memset(reg_buf,0,sizeof(reg_buf));
            PacketType = 0x05;
            reg_buf[0] = 0x29;
            mipi_write(PacketType, reg_buf, 1);

            usleep(100000);
        }
        mipi_dev_close();
        st_is_remain_code_after_next_pt = 0;
        printf("> end : OUT Remain Code, [0x%X]\n",st_is_remain_code_after_next_pt);

        st_already_pic_view = 1;
        printf("-------------------------------------\n");
    }

////////////////////


	if(*dimming_mode)
	{
		if(id == B1)
		{
			system("/Data/Pattern 11 B1"); //180312
			system("/Data/Pattern 12 B1"); //180312
		}
		else if ((id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
		{
			system("/Data/Pattern 12 B1");
		}
		else
			system("/Data/Pattern 12");

		printf("Thread : dbv_scan_mode ON \n");
		*dbv_fin = 0;
		start_dimming_thread(dp_limit);
        usleep(300);		
	}
	else if(*dsc_roll_mode)
	{
        printf("Thread : dsc_roll_mode ON \n");
		start_dsc_roll_thread(dp_limit);
        usleep(300);
	}
    else if(*gray_mode) //180316
    {
        printf("Thread : gray_scan_mode ON\n");
        *gray_fin = 0;
		start_gray_scan_thread(dp_limit);
        usleep(300);
    }
	else if(*sleep_mode) //180316
	{
		mipi_dev_open();
		printf("SLEEP ON\n");
		sleep_control(id,model_index,*sleep_mode,spcial_pattern_dependence_prev);
		mipi_dev_close();

		if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
		{
			system("/Data/Pattern 11 B1");
		}
		else
		{
			system("/Data/Pattern 11");
		}
	}

	else if(grad_mode)
	{
        printf("RGB MODE\n");
	}
	else //jpg pic_view..
	{
		if ((id == B1) || (id == AKATSUKI) || (id == JOAN) || (id == MV) || (id == JOAN_REL) || (id == JOAN_MANUAL) || (id == MV_MANUAL) || (id == JOAN_E5) || (id == A1))
		{
	    	sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\" B1\n",dir,src);
		}
		else
		{
	    	sprintf(string_buf, "/Data/pic_view /mnt/sd/%c/\"%s\"\n",dir,src);
		}

	    if(image_file_check(src)){
	        #ifdef  DEBUG_MSG
	        printf("JPG File : %s\n", string_buf);
	        #endif
	    }
	    else{
	        #ifdef  DEBUG_MSG
	        printf("BMP File : %s\n", string_buf);
	        #endif
	    }

////////////////////
	    if(st_is_remain_code_after_next_pt)
	    {   
	        unsigned char reg_buf[30];
	        unsigned char PacketType = 0x39;

			system(string_buf);

			printf("-------------------------------------\n");				
	        printf("> input : OUT Remain Code, [0x%X]\n",st_is_remain_code_after_next_pt);
	
	        mipi_dev_open();
	        if(st_is_remain_code_after_next_pt & (1<<B1_AOD_OFF))
	        {   
	            printf(">> B1 model AOD OFF Remain Code\n");
	
	            memset(reg_buf,0,sizeof(reg_buf));
	            PacketType = 0x05;
	            reg_buf[0] = 0x13;
	            mipi_write(PacketType, reg_buf, 1); 
	
	            usleep(50000);
	
	        }
			else if(st_is_remain_code_after_next_pt & (1<<B1_VARIABLE_OFF))
			{

	            printf(">> B1 model VARIABLE OFF Remain Code\n");

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
	
	            memset(reg_buf,0,sizeof(reg_buf));
	            PacketType = 0x15;
	            reg_buf[0] = 0xB0;
	            reg_buf[1] = 0xCA;
	            mipi_write(PacketType, reg_buf, 2);

			}
			else if(st_is_remain_code_after_next_pt & (1<<B1_VR_OFF))
			{
                printf(">> B1 model VR OFF Remain Code\n");

                memset(reg_buf,0,sizeof(reg_buf));
                PacketType = 0x05;
                reg_buf[0] = 0x29;
                mipi_write(PacketType, reg_buf, 1);
	
				usleep(100000);
			}
	        mipi_dev_close();
	        st_is_remain_code_after_next_pt = 0;
	        printf("> end : OUT Remain Code, [0x%X]\n",st_is_remain_code_after_next_pt);

			st_already_pic_view = 1;
			printf("-------------------------------------\n");				
	    }

////////////////////

		if(*emcontrol_in_pwm_mode)
		{
            mipi_dev_open();
            printf("EM CONTROL..IN PWM\n");
	        emcon_control(id,emcontrol_in_pwm_mode,*joan_pwm,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();

		}
        else if(*emcontrol_no_pwm_mode)
        {
            mipi_dev_open();
            printf("EM CONTROL..NOT IN PWM\n");
	        emcon_control(id,emcontrol_no_pwm_mode,*joan_pwm,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();

        }
		else if(*aod_mode)
		{
			mipi_dev_open();	
			printf("AOD ON\n");
	        if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL))
		        aod_control(id,model_index,aod_mode,*joan_pwm,string_buf,spcial_pattern_dependence_prev);
		    else
			{
				if (aod_emma2_mode == 1)
				{
			    	aod_control(id,model_index,aod_mode,2,string_buf,spcial_pattern_dependence_prev);	/* set mode to 2 */
				}
				else
				{
			    	aod_control(id,model_index,aod_mode,1,string_buf,spcial_pattern_dependence_prev);
				}
			}

			printf("%s: id = %d \n",__func__,id);
			mipi_dev_close();	
		}
        else if(*vr_mode)
        {

            mipi_dev_open();
            printf("VR ON\n");
	        if((id == JOAN) || (id == JOAN_REL) || (id == JOAN_MANUAL))
		        vr_control(id,model_index,vr_mode,*joan_pwm,string_buf,spcial_pattern_dependence_prev);
		    else
			    vr_control(id,model_index,vr_mode,1,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*blackpoint_mode)
        {
            mipi_dev_open();
            printf(">>>>> BLACKPOINT ON\n");
            blackpoint_control(id,model_index,blackpoint_mode,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*variable_mode)
        {
            mipi_dev_open();
            printf(">>>>> VARIABLE ON\n");
			variable_control(id,model_index,variable_mode,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*bright_line)
        {
            mipi_dev_open();
            printf(">>>>> BRIGHT LINE ON\n");
			bright_line_control(id,model_index, bright_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*black_line)
        {
            mipi_dev_open();
            printf(">>>>> BLACK LINE ON\n");
			black_line_control(id, model_index, black_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*power_bright_line)
        {
            mipi_dev_open();
            printf(">>>>> POWER BRIGHT LINE ON\n");
            power_bright_line_control(id, model_index,power_bright_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*power_black_line)
        {
            mipi_dev_open();
            printf(">>>>> POWER BLACK LINE ON\n");
            power_black_line_control(id, model_index,power_black_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*lu_50p_power_bright_line)
        {
            mipi_dev_open();
            printf(">>>>> 50Per LUMINANCE POWER BRIGHT LINE ON\n");
            luminance_50per_power_bright_line_control(id,model_index, lu_50p_power_bright_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
        else if(*lu_50p_power_black_line)
        {
            mipi_dev_open();
            printf(">>>>> 50Per LUMINANCE POWER BLACK LINE ON\n");
            luminance_50per_power_black_line_control(id, model_index,lu_50p_power_black_line,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
		else if(*dbv_mode)
		{
			mipi_dev_open();
			if(dbv_level == 10)
			{
				dbv_control(2, id,model_index,0x00,0x17,NULL,string_buf,spcial_pattern_dependence_prev); //for dp049
			}
			else if(dbv_level == 180)
			{
				if (id == DP049)
				{
					dbv_control(2, id,model_index,0x01,0x8C,NULL,string_buf,spcial_pattern_dependence_prev); //for dp049-180nit
				}
				else if (id == AKATSUKI)
				{
					dbv_control(2, id,model_index,0x01,0x50,NULL,string_buf,spcial_pattern_dependence_prev); //for akatsuki-180nit
				}
				else if (id == B1)
				{
					dbv_control(2, id,model_index,0x01,0x99,NULL,string_buf,spcial_pattern_dependence_prev); //for B1-180nit
				}
			}
			else
			{
				printf("what DBV Data \n");
			}
			mipi_dev_close();
		}
        else if(*bdtest_mode)
        {
            mipi_dev_open();
            printf(">>>>> BORDER TEST MODE\n");
            border_test_control(id, model_index,bdtest_mode,string_buf,spcial_pattern_dependence_prev);
            mipi_dev_close();
        }
		else
		{
			//normal pattern
			if(!st_already_pic_view)
				system(string_buf);
		}
	}
	FUNC_END();
	return PASS; 

}

int movie_play(MODEL  id,struct display_limit *dp_limit,int *now_img, char dir)
{
	FUNC_BEGIN();
	printf("movie_play... id:%d, now_img:%d dir:%c\n",id,*now_img,dir);
	FUNC_END();
	return PASS;
}

int	display_func(MODEL	id, struct display_limit *dp_limit, int *now_img, char dir, int command)
{
	int img_index = *now_img-1;
	FUNC_BEGIN();
	#ifdef DEBUG_MSG
    printf("*********************2. next %d , model select %d, img_count %d\n",*now_img, id, dp_limit->image_count);
    #endif

	if(img_index < dp_limit->image_count)
	{
		dp_limit->st_vod_play = 0;
        if(dir == 'B')
            view_image(id,file_name_image_B[img_index], dp_limit, dir, command);
        else if(dir == 'C')
            view_image(id,file_name_image_C[img_index], dp_limit, dir, command);
        else if(dir == 'D')
            view_image(id,file_name_image_D[img_index], dp_limit, dir, command);
        else if(dir == 'E')
            view_image(id,file_name_image_E[img_index], dp_limit, dir, command);
        else if(dir == 'F')
            view_image(id,file_name_image_F[img_index], dp_limit, dir, command);
        else if(dir == 'G')
            view_image(id,file_name_image_G[img_index], dp_limit, dir, command);
        else if(dir == 'H')
            view_image(id,file_name_image_H[img_index], dp_limit, dir, command);
        else if(dir == 'I')
            view_image(id,file_name_image_I[img_index], dp_limit, dir, command);
        else if(dir == 'J')
            view_image(id,file_name_image_J[img_index], dp_limit, dir, command);
        else if(dir == 'K')
            view_image(id,file_name_image_K[img_index], dp_limit, dir, command);
		else
		{
			FUNC_END();
			return	FAIL;
		}
	}
	else
	{
        movie_play(id,dp_limit,now_img, dir);
		dp_limit->st_vod_play = 1;
	}

	FUNC_END();
	return	PASS;
}

