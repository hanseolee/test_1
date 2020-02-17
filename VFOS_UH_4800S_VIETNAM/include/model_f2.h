#ifndef	__MODEL_F2_H__
#define	__MODEL_F2_H__

#include <pthread.h>
#include <model_common.h>

/* define for f2 configuration */
#define	F2_CONFIG_DIR_NAME			"initial"
#define	F2_CONFIG_FILE_NAME			"f2_config.tty"

/* MAX and MIN for special pattern mode */
#define	F2_GRAY_SCAN_MIN_VALUE					0x0000
#define	F2_GRAY_SCAN_MAX_VALUE					0x03FC

#define	F2_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	F2_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	F2_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	F2_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	F2_GRAY_SCAN_LOW_8BITS_FILTER			0xFC

#define	F2_GRAY_SCAN_RGB_HIGH_BYTE_DATA_NUM		4
#define	F2_GRAY_SCAN_RED_LOW_BYTE_DATA_NUM		5
#define	F2_GRAY_SCAN_GREEN_LOW_BYTE_DATA_NUM		6
#define	F2_GRAY_SCAN_BLUE_LOW_BYTE_DATA_NUM		7

#define	F2_DBV_MAX_VALUE				0x0FFF
#define F2_DBV_MIN_VALUE                0x0000
#define	F2_DBV_REF_VALUE				0x0FEF
//#define	F2_DBV_HIGH_8BITS_FILTER		0x03
#define	F2_DBV_HIGH_8BITS_FILTER		0xFF

/* define for F2 OTP read */

/*
#define	F2_OTP_READ_CHANNEL_NUM	2
#define	F2_OTP_READ_LENGTH		5
#define	F2_OTP_MAX_WRITE_TIME		5
#define	F2_OTP_PRE_GAMMA_OFFSET		2	
#define	F2_OTP_GAMMA_1_OFFSET		3	
#define	F2_OTP_MLPIS_OFFSET			3	
#define	F2_OTP_GAMMA_2_OFFSET		4	
#define	F2_OTP_DISPLAY_OFFSET		4	
#define	GET_F2_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_F2_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_F2_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_F2_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_F2_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
*/

#define	F2_OTP_READ_CHANNEL_NUM	2
#define	F2_OTP_READ_LENGTH		1

#define	F2_OTP_MAX_WRITE_TIME		3

#define	F2_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	F2_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	F2_OTP_DISPLAY_OFFSET		3	/* 4th bytes */
#define	F2_OTP_PTN_OFFSET			4	/* 5th bytes */
#define	F2_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	F2_OTP_MLPIS_OFFSET			5	/* 6th bytes */

#define	GET_F2_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_F2_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_F2_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_F2_OTP_PTN_VALUE(x)			(((x) >> 4) & 0x7)
#define	GET_F2_OTP_GAMMA_2_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_F2_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define F2_OTP_OFFSET             0   /* 1st bytes */

/* define for F2 POCB read/write */
#define	F2_POCB_READ_CHANNEL_NUM		2
#define	F2_POCB_WRITE_CHANNEL_NUM	2
#define	F2_POCB_READ_LENGTH			1
#define	F2_POCB_WRITE_LENGTH			1

#define	F2_POCB_ENABLE_BIT		(1 << 7)
#define	F2_POCB_ENABLE_VALUE		0x85
#define	F2_POCB_DISABLE_VALUE	    0xA5

/* define for F2 CURRENT */
#define	F2_CURRENT_PATTERN_NUM		2
#define	F2_CURRENT_TEST_CHANNEL_NUM	2

/* define for F2 TOUCH */
#define	F2_TOUCH_I2C_SLAVE_ADDR		40

/* define for PRODUCT_ID offset which is used temporarily */
#define	F2_TOUCH_PRODUCT_ID_TEMP_OFFSET	0x10



/* type define */
typedef struct model_f2_info_s {
	int	key_dev;
	int	model_f2_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER f2_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_f2_info_t;

typedef struct model_f2_s {
	pthread_t id_f2_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t f2_thread_mutex;
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
	unsigned char otp_read_val[F2_OTP_READ_LENGTH];
	int flag_otp_test_result_ch1;
	int flag_otp_test_result_ch2;
	int flag_touch_test_result_ch1;
	int flag_touch_test_result_ch2;
	int flag_current_test_result_ch1;
	int flag_current_test_result_ch2;
	//int flag_display_test_result;
	pocb_status_t pocb_status;
	model_f2_info_t	model_f2_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[F2_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
    mipi_read_data_t g_mipi_read;       /*mipi read buffer.. only use model_f2.c*/
    mipi_write_data_t g_mipi_write;     /*mipi write buffer.. only use model_f2.c*/
    int flag_already_set_model;
    hz_status_t hz_status;

} model_f2_t;

enum{           // TOUCH_SS_ TX쪽에서 total과 raw 모두 플래그가 1개부족(TD22 tx idle, TD34 total ix tx)
    TOUCH_POWER_CONSUMPTION = 0,
    TOUCH_PIN_TEST,
    TOUCH_FW_VERSION,
    TOUCH_CONFIG_VERSION,
    TOUCH_RELEASE_VERSION,
    TOUCH_ITO_SHORT,
    TOUCH_HF_RAW_GAP_H,
    TOUCH_HF_RAW_GAP_V,
    TOUCH_MS_CX2,
    TOUCH_MS_CX2_GAP_H,
    TOUCH_MS_CX2_GAP_V,
    TOUCH_CM_RAW_DATA,
    TOUCH_SS_TOTAL_LX_TX_ACTIVE,
    TOUCH_SS_TOTAL_LX_TX_IDLE,      // TD22
    TOUCH_SS_TOTAL_LX_RX,
    TOUCH_SS_RAW_DATA_TX_ACTIVE,
    TOUCH_SS_RAW_DATA_TX_IDLE,
    TOUCH_SS_RAW_DATA_RX,
    TOUCH_CM_JITTER,
    TOUCH_AVDD_DVDD_CURRENT,
};

/* function proto type */
void put_info_for_f2_thread(model_f2_info_t *);
void start_f2_thread(void);
int init_model_f2(void);
int release_model_f2(void);
int get_last_key_value_for_f2(void);

#endif	/* __MODEL_F2_H__ */
