#ifndef	__MODEL_DP076_H__
#define	__MODEL_DP076_H__

#include <pthread.h>
#include <model_common.h>

/* define for dp076 configuration */
#define	DP076_CONFIG_DIR_NAME			"initial"
#define	DP076_CONFIG_FILE_NAME			"dp076_config.tty"

/* MAX and MIN for special pattern mode */
#define	DP076_GRAY_SCAN_MIN_VALUE					0x00
#define	DP076_GRAY_SCAN_MAX_VALUE					0xFF

#define	DP076_GRAY_SCAN_RED_BYTE_1_DATA_NUM			4
#define	DP076_GRAY_SCAN_GREEN_BYTE_1_DATA_NUM		5
#define	DP076_GRAY_SCAN_BLUE_BYTE_1_DATA_NUM		6
#define	DP076_GRAY_SCAN_NO_INFO_BYTE_1_DATA_NUM		7
#define	DP076_GRAY_SCAN_RED_BYTE_2_DATA_NUM			8
#define	DP076_GRAY_SCAN_GREEN_BYTE_2_DATA_NUM		9
#define	DP076_GRAY_SCAN_BLUE_BYTE_2_DATA_NUM		10
#define	DP076_GRAY_SCAN_NO_INFO_BYTE_2_DATA_NUM		11

#if	0	/* No need for DP076, will be removed after confirmation */
#define	DP076_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	DP076_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	DP076_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	DP076_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	DP076_GRAY_SCAN_LOW_8BITS_FILTER			0xFC
#endif

#define	DP076_DBV_MAX_VALUE				0x03FF
#define	DP076_DBV_MIN_VALUE				0x001F
#define	DP076_DBV_REF_VALUE				0x03E1
#define	DP076_DBV_HIGH_8BITS_FILTER		0x03

/* define for DP076 OTP read */
#define	DP076_OTP_READ_CHANNEL_NUM	2
#define	DP076_OTP_READ_LENGTH		2

#define	DP076_OTP_MAX_WRITE_TIME		3

#if	0	/* No need for DP076, will be removed after confirmation */
#define	DP076_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	DP076_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	DP076_OTP_MLPIS_OFFSET			3	/* 4th bytes */
#define	DP076_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	DP076_OTP_DISPLAY_OFFSET		4	/* 5th bytes */

#define	GET_DP076_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_DP076_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_DP076_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_DP076_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_DP076_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
#endif
#define	DP076_OTP_OFFSET			0	/* 1st bytes */
#define	GET_DP076_OTP_VALUE(x)		(((x) >> 5) & 0x3)

/* define for DP076 POCB read/write */
#define	DP076_POCB_READ_CHANNEL_NUM		2
#define	DP076_POCB_WRITE_CHANNEL_NUM	2
#define	DP076_POCB_READ_LENGTH			1
#define	DP076_POCB_WRITE_LENGTH			1

#define	DP076_POCB_ENABLE_BIT		(1 << 0)
#define	DP076_POCB_ENABLE_VALUE		0x01
#define	DP076_POCB_DISABLE_VALUE	0x00

/* define for DP076 CURRENT */
//#define	DP076_CURRENT_PATTERN_NUM		2	/* swchoi - comment as this define is not used - 20180827 */
#define	DP076_CURRENT_TEST_CHANNEL_NUM	2

#define	DP076_CURRENT_PATTERN_WHITE		0
//#define	DP076_CURRENT_PATTERN_40		1
#define	DP076_CURRENT_PATTERN_SMILE		1

/* define of LGD offset */
#define	DP076_CURRENT_WHITE_VPNL_LGD_OFFSET			8
#define	DP076_CURRENT_WHITE_VDDI_LGD_OFFSET			57
#define	DP076_CURRENT_WHITE_VDDVDH_LGD_OFFSET		(-7)
#define	DP076_CURRENT_WHITE_VDDEL_LGD_OFFSET		(-26)

#define	DP076_CURRENT_40_VPNL_LGD_OFFSET			8
#define	DP076_CURRENT_40_VDDI_LGD_OFFSET			80
#define	DP076_CURRENT_40_VDDVDH_LGD_OFFSET			(-24)
#define	DP076_CURRENT_40_VDDEL_LGD_OFFSET			(-14)

#define	DP076_CURRENT_SMILE_VPNL_LGD_OFFSET			9
#define	DP076_CURRENT_SMILE_VDDI_LGD_OFFSET			66
#define	DP076_CURRENT_SMILE_VDDVDH_LGD_OFFSET		(-8)
#define	DP076_CURRENT_SMILE_VDDEL_LGD_OFFSET		(-16)

/* define for DP076 TOUCH */
//#define	DP076_TOUCH_I2C_SLAVE_ADDR		0x70
#define	DP076_TOUCH_I2C_SLAVE_ADDR		0x28

/* type define */
typedef struct model_dp076_info_s {
	int	key_dev;
	int	model_dp076_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER dp076_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_dp076_info_t;

typedef struct model_dp076_s {
	pthread_t id_dp076_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t dp076_thread_mutex;
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
	int flag_need_to_display_on;	/* if flag is set, display on code has to be set after display PTN */
	int last_key_value;
	int cur_image_num;
	int cur_test_mode;				/* test mode - Display or OTP or Touch or Current */
	unsigned char otp_read_val[DP076_OTP_READ_LENGTH];
	int flag_otp_test_result_ch1;
	int flag_otp_test_result_ch2;
	int flag_touch_test_result_ch1;
	int flag_touch_test_result_ch2;
	int flag_current_test_result_ch1;
	int flag_current_test_result_ch2;
//	int flag_display_test_result;
	pocb_status_t pocb_status;
	model_dp076_info_t	model_dp076_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[DP076_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
	mipi_read_data_t g_mipi_read;		/*mipi read buffer.. only use model_dp076.c*/
	mipi_write_data_t g_mipi_write;		/*mipi write buffer.. only use model_dp076.c*/
} model_dp076_t;

/* constant define */

/* function proto type */
void put_info_for_dp076_thread(model_dp076_info_t *);
void start_dp076_thread(void);
int init_model_dp076(void);
int release_model_dp076(void);
int get_last_key_value_for_dp076(void);

#endif	/* __MODEL_DP076_H__ */
