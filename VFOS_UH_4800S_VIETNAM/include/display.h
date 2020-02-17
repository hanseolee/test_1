#ifndef	__DISPLAY_H__
#define	__DISPLAY_H__

extern	char file_name_image_B[300][100];
extern  char file_name_image_C[300][100];
extern  char file_name_image_D[300][100];
extern  char file_name_image_E[300][100];
extern  char file_name_image_F[300][100];
extern  char file_name_image_G[300][100];
extern  char file_name_image_H[300][100];
extern  char file_name_image_I[300][100];
extern  char file_name_image_J[300][100];

extern  char Bfile_name_vod[300][100];
extern  char Cfile_name_vod[300][100];
extern  char Dfile_name_vod[300][100];
extern  char Efile_name_vod[300][100];
extern  char Ffile_name_vod[300][100];
extern  char Gfile_name_vod[300][100];
extern  char Hfile_name_vod[300][100];
extern  char Ifile_name_vod[300][100];
extern  char Jfile_name_vod[300][100];

extern	pthread_t       id_gray_thread;
extern	int spcial_pattern_dependence_prev; //for func of view_image or view_init_for_state

void *gray_thread(void *arg);
int receive_roll_thread_mutex(pthread_mutex_t *receive_mutex);

int get_display_image_info(char , char (*)[]);
void display_limit_parser (MODEL id,int buf_index, struct display_limit *dl, char dir);
int scan_directory(char directory);
int scan_directory_vod(char directory);

int view_init_for_state(int command, struct display_limit *dp_limit, char *model_name, struct status_flag *sf,char dir);
int display_func(MODEL  id, struct display_limit *dp_limit, int *now_img, char dir, int command);

#endif	/* __DISPLAY_H__ */

