#ifndef	__MODEL_AOP_H__
#define	__MODEL_AOP_H__

#include <pthread.h>
#include <model_common.h>

/* define for aop configuration */
#define	AOP_CONFIG_DIR_NAME			"initial"
#define	AOP_CONFIG_FILE_NAME			"aop_config.tty"

/* MAX and MIN for special pattern mode */
#define	AOP_GRAY_SCAN_MIN_VALUE					0x00
#define	AOP_GRAY_SCAN_MAX_VALUE					0xFF

#define	AOP_GRAY_SCAN_RED_BYTE_1_DATA_NUM			4
#define	AOP_GRAY_SCAN_GREEN_BYTE_1_DATA_NUM		5
#define	AOP_GRAY_SCAN_BLUE_BYTE_1_DATA_NUM		6
#define	AOP_GRAY_SCAN_NO_INFO_BYTE_1_DATA_NUM		7
#define	AOP_GRAY_SCAN_RED_BYTE_2_DATA_NUM			8
#define	AOP_GRAY_SCAN_GREEN_BYTE_2_DATA_NUM		9
#define	AOP_GRAY_SCAN_BLUE_BYTE_2_DATA_NUM		10
#define	AOP_GRAY_SCAN_NO_INFO_BYTE_2_DATA_NUM		11

#if	0	/* No need for AOP, will be removed after confirmation */
#define	AOP_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	AOP_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	AOP_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	AOP_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	AOP_GRAY_SCAN_LOW_8BITS_FILTER			0xFC
#endif

#define	AOP_DBV_MAX_VALUE				0x03E5
#define	AOP_DBV_MIN_VALUE				0x0003
#define	AOP_DBV_REF_VALUE				0x03E5
#define	AOP_DBV_HIGH_8BITS_FILTER		0x03

/* define for AOP OTP read */
#define	AOP_OTP_READ_CHANNEL_NUM	2
#define	AOP_OTP_READ_LENGTH		2

#define	AOP_OTP_MAX_WRITE_TIME		3

#if	0	/* No need for AOP, will be removed after confirmation */
#define	AOP_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	AOP_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	AOP_OTP_MLPIS_OFFSET			3	/* 4th bytes */
#define	AOP_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	AOP_OTP_DISPLAY_OFFSET		4	/* 5th bytes */

#define	GET_AOP_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_AOP_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_AOP_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_AOP_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_AOP_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
#endif
#define	AOP_OTP_OFFSET			0	/* 1st bytes */
#define	GET_AOP_OTP_VALUE(x)		(((x) >> 5) & 0x3)

/* define for AOP POCB read/write */
#define	AOP_POCB_READ_CHANNEL_NUM		2
#define	AOP_POCB_WRITE_CHANNEL_NUM	2
#define	AOP_POCB_READ_LENGTH			1
#define	AOP_POCB_WRITE_LENGTH			1

#define	AOP_POCB_ENABLE_BIT		(1 << 0)
#define	AOP_POCB_ENABLE_VALUE		0x01
#define	AOP_POCB_DISABLE_VALUE	0x00

/* define for AOP CURRENT */
#define	AOP_CURRENT_PATTERN_NUM		1	/* it should be 2 but 1 is used for the time being because LGD provided only 1 current spec - 20180425 */
#define	AOP_CURRENT_TEST_CHANNEL_NUM	2

/* define for AOP TOUCH */
#define	AOP_TOUCH_I2C_SLAVE_ADDR		0x4A


/* type define */
typedef struct model_aop_info_s {
	int	key_dev;
	int	model_aop_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER aop_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_aop_info_t;

typedef struct model_aop_s {
	pthread_t id_aop_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t aop_thread_mutex;
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
	unsigned char otp_read_val[AOP_OTP_READ_LENGTH];
	int flag_otp_test_result;
	int flag_touch_test_result;
	int flag_current_test_result;
	//int flag_display_test_result;
	pocb_status_t pocb_status;
	model_aop_info_t	model_aop_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[AOP_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
	mipi_read_data_t g_mipi_read;		/*mipi read buffer.. only use model_aop.c*/
	mipi_write_data_t g_mipi_write;		/*mipi write buffer.. only use model_aop.c*/
} model_aop_t;

/* constant define */

/* function proto type */
void put_info_for_aop_thread(model_aop_info_t *);
void start_aop_thread(void);
int init_model_aop(void);
int release_model_aop(void);
int get_last_key_value_for_aop(void);

#endif	/* __MODEL_AOP_H__ */
