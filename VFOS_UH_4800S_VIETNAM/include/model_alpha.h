#ifndef	__MODEL_ALPHA_H__
#define	__MODEL_ALPHA_H__

#include <pthread.h>
#include <model_common.h>

/* define for alpha configuration */
#define	ALPHA_CONFIG_DIR_NAME			"initial"
#define	ALPHA_CONFIG_FILE_NAME			"alpha_config.tty"

/* MAX and MIN for special pattern mode */
#define	ALPHA_GRAY_SCAN_MIN_VALUE					0x0000
#define	ALPHA_GRAY_SCAN_MAX_VALUE					0x03FC

#define	ALPHA_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	ALPHA_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	ALPHA_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	ALPHA_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	ALPHA_GRAY_SCAN_LOW_8BITS_FILTER			0xFC

#define	ALPHA_GRAY_SCAN_RGB_HIGH_BYTE_DATA_NUM		4
#define	ALPHA_GRAY_SCAN_RED_LOW_BYTE_DATA_NUM		5
#define	ALPHA_GRAY_SCAN_GREEN_LOW_BYTE_DATA_NUM		6
#define	ALPHA_GRAY_SCAN_BLUE_LOW_BYTE_DATA_NUM		7

#define	ALPHA_DBV_MAX_VALUE				0x03FF
#define	ALPHA_DBV_MIN_VALUE				0x0005
//#define	ALPHA_DBV_REF_VALUE				0x03E5
#define	ALPHA_DBV_REF_VALUE				0x0309
#define	ALPHA_DBV_HIGH_8BITS_FILTER		0x03

/* define for ALPHA OTP read */

/*
#define	ALPHA_OTP_READ_CHANNEL_NUM	2
#define	ALPHA_OTP_READ_LENGTH		5
#define	ALPHA_OTP_MAX_WRITE_TIME		5
#define	ALPHA_OTP_PRE_GAMMA_OFFSET		2	
#define	ALPHA_OTP_GAMMA_1_OFFSET		3	
#define	ALPHA_OTP_MLPIS_OFFSET			3	
#define	ALPHA_OTP_GAMMA_2_OFFSET		4	
#define	ALPHA_OTP_DISPLAY_OFFSET		4	
#define	GET_ALPHA_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_ALPHA_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_ALPHA_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_ALPHA_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_ALPHA_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
*/

#define	ALPHA_OTP_READ_CHANNEL_NUM	2
#define	ALPHA_OTP_READ_LENGTH		6

#define	ALPHA_OTP_MAX_WRITE_TIME		5

#define	ALPHA_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	ALPHA_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	ALPHA_OTP_DISPLAY_OFFSET		3	/* 4th bytes */
#define	ALPHA_OTP_PTN_OFFSET			4	/* 5th bytes */
#define	ALPHA_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	ALPHA_OTP_MLPIS_OFFSET			5	/* 6th bytes */

#define	GET_ALPHA_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_ALPHA_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_ALPHA_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_ALPHA_OTP_PTN_VALUE(x)			(((x) >> 4) & 0x7)
#define	GET_ALPHA_OTP_GAMMA_2_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_ALPHA_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)


/* define for ALPHA POCB read/write */
#define	ALPHA_POCB_READ_CHANNEL_NUM		2
#define	ALPHA_POCB_WRITE_CHANNEL_NUM	2
#define	ALPHA_POCB_READ_LENGTH			1
#define	ALPHA_POCB_WRITE_LENGTH			1

#define	ALPHA_POCB_ENABLE_BIT		(1 << 7)
#define	ALPHA_POCB_ENABLE_VALUE		0
#define	ALPHA_POCB_DISABLE_VALUE	0x80

/* define for ALPHA CURRENT */
#define	ALPHA_CURRENT_PATTERN_NUM		2
#define	ALPHA_CURRENT_TEST_CHANNEL_NUM	2

/* define for ALPHA TOUCH */
#define	ALPHA_TOUCH_I2C_SLAVE_ADDR		0x28

/* define for PRODUCT_ID offset which is used temporarily */
#define	ALPHA_TOUCH_PRODUCT_ID_TEMP_OFFSET	0x10



/* type define */
typedef struct model_alpha_info_s {
	int	key_dev;
	int	model_alpha_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER alpha_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_alpha_info_t;

typedef struct model_alpha_s {
	pthread_t id_alpha_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t alpha_thread_mutex;
	pthread_mutex_t gray_scan_thread_mutex;
	pthread_mutex_t dimming_thread_mutex;
	pthread_mutex_t dsc_roll_thread_mutex;
	int flag_run_gray_scan_thread;
	int flag_run_dimming_thread;
	int flag_run_dsc_roll_thread;
	int flag_finish_gray_scan_thread;
	int flag_finish_dimming_thread;
	int flag_finish_dsc_roll_thread;
	int flag_already_module_on;	/* whether or not module has been already ON */
	int flag_need_to_init_module;	/* whether or not initialize module is needed */
	int flag_need_to_pocb_write;	/* whether or not write POCB */
	int last_key_value;
	int cur_image_num;
	int cur_test_mode;				/* test mode - Display or OTP or Touch or Current */
	unsigned char otp_read_val[ALPHA_OTP_READ_LENGTH];
	int flag_otp_test_result_ch1;
	int flag_otp_test_result_ch2;
	int flag_touch_test_result_ch1;
	int flag_touch_test_result_ch2;
	int flag_current_test_result_ch1;
	int flag_current_test_result_ch2;
	//int flag_display_test_result;
	pocb_status_t pocb_status;
	model_alpha_info_t	model_alpha_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[ALPHA_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
    mipi_read_data_t g_mipi_read;       /*mipi read buffer.. only use model_alpha.c*/
    mipi_write_data_t g_mipi_write;     /*mipi write buffer.. only use model_alpha.c*/

} model_alpha_t;

enum{
    TOUCH_CONNECION_CHECK = 0,
    TOUCH_IC_INFO_TEST,
	TOUCH_PRODUCT_ID_TEST,
	TOUCH_OPEN_TEST,
    TOUCH_SHORT_TEST,
	TOUCH_M2_RAW_MUTUAL_TEST,
	TOUCH_M2_RAW_SELF_TEST,
    TOUCH_M2_JITTER_TEST,
    TOUCH_M1_RAW_TEST,
    TOUCH_M1_JITTER_TEST,
};
/* function proto type */
void put_info_for_alpha_thread(model_alpha_info_t *);
void start_alpha_thread(void);
int init_model_alpha(void);
int release_model_alpha(void);
int get_last_key_value_for_alpha(void);

#endif	/* __MODEL_ALPHA_H__ */
