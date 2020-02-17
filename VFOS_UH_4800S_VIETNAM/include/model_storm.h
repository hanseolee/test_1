#ifndef	__MODEL_STORM_H__
#define	__MODEL_STORM_H__

#include <pthread.h>
#include <model_common.h>

/* define for storm configuration */
#define	STORM_CONFIG_DIR_NAME			"initial"
#define	STORM_CONFIG_FILE_NAME			"storm_config.tty"

/* MAX and MIN for special pattern mode */
#define	STORM_GRAY_SCAN_MIN_VALUE					0x0000
#define	STORM_GRAY_SCAN_MAX_VALUE					0x03FC

#define	STORM_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	STORM_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	STORM_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	STORM_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	STORM_GRAY_SCAN_LOW_8BITS_FILTER			0xFC

#define	STORM_GRAY_SCAN_RGB_HIGH_BYTE_DATA_NUM		4
#define	STORM_GRAY_SCAN_RED_LOW_BYTE_DATA_NUM		5
#define	STORM_GRAY_SCAN_GREEN_LOW_BYTE_DATA_NUM		6
#define	STORM_GRAY_SCAN_BLUE_LOW_BYTE_DATA_NUM		7

#define	STORM_DBV_MAX_VALUE				0x03FF
#define	STORM_DBV_MIN_VALUE				0x0003
#define	STORM_DBV_REF_VALUE				0x03E5
#define	STORM_DBV_HIGH_8BITS_FILTER		0x03

/* define for STORM OTP read */
#define	STORM_OTP_READ_CHANNEL_NUM	2
#define	STORM_OTP_READ_LENGTH		5

#define	STORM_OTP_MAX_WRITE_TIME		5

#define	STORM_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	STORM_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	STORM_OTP_MLPIS_OFFSET			3	/* 4th bytes */
#define	STORM_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	STORM_OTP_DISPLAY_OFFSET		4	/* 5th bytes */

#define	GET_STORM_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_STORM_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_STORM_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_STORM_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_STORM_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)

/* define for STORM POCB read/write */
#define	STORM_POCB_READ_CHANNEL_NUM		2
#define	STORM_POCB_WRITE_CHANNEL_NUM	2
#define	STORM_POCB_READ_LENGTH			1
#define	STORM_POCB_WRITE_LENGTH			1

#define	STORM_POCB_ENABLE_BIT		(1 << 7)
#define	STORM_POCB_ENABLE_VALUE		0
#define	STORM_POCB_DISABLE_VALUE	0x80

/* define for STORM CURRENT */
//#define	STORM_CURRENT_PATTERN_NUM		2	/* swchoi - comment as this define is not used - 20180904 */
#define	STORM_CURRENT_TEST_CHANNEL_NUM	2

#define	STORM_CURRENT_PATTERN_WHITE		0
#define	STORM_CURRENT_PATTERN_40		1

/* define of LGD offset */
#define	STORM_CURRENT_WHITE_VPNL_LGD_OFFSET			11
#define	STORM_CURRENT_WHITE_VDDI_LGD_OFFSET			47
#define	STORM_CURRENT_WHITE_VDDVDH_LGD_OFFSET		(-11)
#define	STORM_CURRENT_WHITE_VDDEL_LGD_OFFSET		(-85)

#define	STORM_CURRENT_40_VPNL_LGD_OFFSET			8
#define	STORM_CURRENT_40_VDDI_LGD_OFFSET			43
#define	STORM_CURRENT_40_VDDVDH_LGD_OFFSET			(-5)
#define	STORM_CURRENT_40_VDDEL_LGD_OFFSET			(14)

/* define for STORM TOUCH */
#define	STORM_TOUCH_I2C_SLAVE_ADDR		0x20

/* define for PRODUCT_ID offset which is used temporarily */
#define	STORM_TOUCH_PRODUCT_ID_LOOP_CNT		2	/* check address 0 and address 2 - 20180831 */
#define	STORM_TOUCH_PRODUCT_ID_TEMP_OFFSET	0x10
#define	STORM_TOUCH_PRODUCT_ID_ADDRESS_0_BIT	(1 << 0)
#define	STORM_TOUCH_PRODUCT_ID_ADDRESS_1_BIT	(1 << 1)

/* type define */
typedef struct model_storm_info_s {
	int	key_dev;
	int	model_storm_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER storm_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_storm_info_t;

typedef struct model_storm_s {
	pthread_t id_storm_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t storm_thread_mutex;
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
	unsigned char otp_read_val[STORM_OTP_READ_LENGTH];
	int flag_otp_test_result_ch1;
	int flag_otp_test_result_ch2;
	int flag_touch_test_result_ch1;
	int flag_touch_test_result_ch2;
	int flag_current_test_result_ch1;
	int flag_current_test_result_ch2;
	//int flag_display_test_result;
	pocb_status_t pocb_status;
	model_storm_info_t	model_storm_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[STORM_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
    mipi_read_data_t g_mipi_read;       /*mipi read buffer.. only use model_storm.c*/
    mipi_write_data_t g_mipi_write;     /*mipi write buffer.. only use model_storm.c*/

} model_storm_t;

/* constant define */

/* function proto type */
void put_info_for_storm_thread(model_storm_info_t *);
void start_storm_thread(void);
int init_model_storm(void);
int release_model_storm(void);
int get_last_key_value_for_storm(void);

#endif	/* __MODEL_STORM_H__ */
