#ifndef	__MODEL_DP173_H__
#define	__MODEL_DP173_H__

#include <pthread.h>
#include <model_common.h>
#include <synaptics_touch_04.h>		// for synaptics_04_touch_limit_parser
/* define for dp173 configuration */
#define	DP173_CONFIG_DIR_NAME			"initial"
#define	DP173_CONFIG_FILE_NAME			"dp173_config.tty"

/* MAX and MIN for special pattern mode */
#define	DP173_GRAY_SCAN_MIN_VALUE					0x00
#define	DP173_GRAY_SCAN_MAX_VALUE					0xFF

#define	DP173_GRAY_SCAN_RED_BYTE_1_DATA_NUM			4
#define	DP173_GRAY_SCAN_GREEN_BYTE_1_DATA_NUM		5
#define	DP173_GRAY_SCAN_BLUE_BYTE_1_DATA_NUM		6
#define	DP173_GRAY_SCAN_NO_INFO_BYTE_1_DATA_NUM		7
#define	DP173_GRAY_SCAN_RED_BYTE_2_DATA_NUM			8
#define	DP173_GRAY_SCAN_GREEN_BYTE_2_DATA_NUM		9
#define	DP173_GRAY_SCAN_BLUE_BYTE_2_DATA_NUM		10
#define	DP173_GRAY_SCAN_NO_INFO_BYTE_2_DATA_NUM		11

#if	0	/* No need for DP173, will be removed after confirmation */
#define	DP173_GRAY_SCAN_HIGH_2BITS_FILTER			0x03
#define	DP173_GRAY_SCAN_HIGH_2BITS_RED_FILTER		0x03
#define	DP173_GRAY_SCAN_HIGH_2BITS_GREEN_FILTER		0x0C
#define	DP173_GRAY_SCAN_HIGH_2BITS_BLUE_FILTER		0x30
#define	DP173_GRAY_SCAN_LOW_8BITS_FILTER			0xFC
#endif

//#define	DP173_DBV_MAX_VALUE				0x03FF
#define	DP173_DBV_MAX_VALUE				0x0FFF
//#define	DP173_DBV_MIN_VALUE				0x001F
#define	DP173_DBV_MIN_VALUE				0x0012
//#define	DP173_DBV_REF_VALUE				0x03E1
#define	DP173_DBV_REF_VALUE				0x0EF1
//#define	DP173_DBV_HIGH_8BITS_FILTER		0x03
#define	DP173_DBV_HIGH_8BITS_FILTER		0xFF

/* define for DP173 OTP read */
#define	DP173_OTP_READ_CHANNEL_NUM	2
#define	DP173_OTP_READ_LENGTH		1

#define	DP173_OTP_MAX_WRITE_TIME		5
#define	DP173_CRC_MAX_WRITE_TIME		5

/* define for DP173 CRC read */
#define DP173_CRC_READ_CHANNEL_NUM	2
#define DP173_CRC_READ_LENGTH		4

#if	0	/* No need for DP173, will be removed after confirmation */
#define	DP173_OTP_PRE_GAMMA_OFFSET		2	/* 3rd bytes */
#define	DP173_OTP_GAMMA_1_OFFSET		3	/* 4th bytes */
#define	DP173_OTP_MLPIS_OFFSET			3	/* 4th bytes */
#define	DP173_OTP_GAMMA_2_OFFSET		4	/* 5th bytes */
#define	DP173_OTP_DISPLAY_OFFSET		4	/* 5th bytes */

#define	GET_DP173_OTP_PRE_GAMMA_VALUE(x)	(((x) >> 0) & 0x7)
#define	GET_DP173_OTP_GAMMA_1_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_DP173_OTP_MLPIS_VALUE(x)		(((x) >> 0) & 0x7)
#define	GET_DP173_OTP_GAMMA_2_VALUE(x)		(((x) >> 4) & 0x7)
#define	GET_DP173_OTP_DISPLAY_VALUE(x)		(((x) >> 0) & 0x7)
#endif
#define	DP173_OTP_OFFSET			0	/* 1st bytes */
#define DP173_CRC_OFFSET			0
#define	GET_DP173_OTP_VALUE(x)		(((x) >> 5) & 0x3)

/* define for DP173 POCB read/write */
#define	DP173_POCB_READ_CHANNEL_NUM		2
#define	DP173_POCB_WRITE_CHANNEL_NUM	2
#define	DP173_POCB_READ_LENGTH			5
#define	DP173_POCB_WRITE_LENGTH			1
#define	DP173_LABEL_MAX_LENGTH			5

#define	DP173_POCB_ENABLE_BIT		(1 << 2) & ( 1 << 7)
#define	DP173_POCB_ENABLE_VALUE		0x01
#define	DP173_POCB_DISABLE_VALUE	0x00

/* define for DP173 CURRENT */
//#define	DP173_CURRENT_PATTERN_NUM		2	/* swchoi - comment as this define is not used - 20180827 */
#define	DP173_CURRENT_TEST_CHANNEL_NUM	2

#define	DP173_CURRENT_PATTERN_WHITE		0
#define	DP173_CURRENT_PATTERN_40		1
#define	DP173_CURRENT_PATTERN_SMILE		2
#define DP173_CURRENT_PATTERN_SLEEP		3
#if 0
/* define of LGD offset */
#define	DP173_CURRENT_WHITE_VPNL_LGD_OFFSET			(-50)
#define	DP173_CURRENT_WHITE_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_WHITE_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_WHITE_VDDEL_LGD_OFFSET		0

#define	DP173_CURRENT_40_VPNL_LGD_OFFSET			(-50)
#define	DP173_CURRENT_40_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_40_VDDVDH_LGD_OFFSET			0
#define	DP173_CURRENT_40_VDDEL_LGD_OFFSET			0

#define	DP173_CURRENT_SMILE_VPNL_LGD_OFFSET			(-50)
#define	DP173_CURRENT_SMILE_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_SMILE_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_SMILE_VDDEL_LGD_OFFSET		0

// LWG 190702 ???
#define	DP173_CURRENT_SLEEP_VPNL_LGD_OFFSET			(-50)
#define	DP173_CURRENT_SLEEP_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_SLEEP_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_SLEEP_VDDEL_LGD_OFFSET		0
#endif
#define	DP173_CURRENT_WHITE_VPNL_LGD_OFFSET			0
#define	DP173_CURRENT_WHITE_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_WHITE_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_WHITE_VDDEL_LGD_OFFSET		0

#define	DP173_CURRENT_40_VPNL_LGD_OFFSET			0
#define	DP173_CURRENT_40_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_40_VDDVDH_LGD_OFFSET			0
#define	DP173_CURRENT_40_VDDEL_LGD_OFFSET			(-10)

#define	DP173_CURRENT_SMILE_VPNL_LGD_OFFSET			0
#define	DP173_CURRENT_SMILE_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_SMILE_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_SMILE_VDDEL_LGD_OFFSET		0
#define DP173_CURRENT_SMILE_VDDD_LGD_OFFSET			(-173)

// LWG 190702 ???
#define	DP173_CURRENT_SLEEP_VPNL_LGD_OFFSET		    0
#define	DP173_CURRENT_SLEEP_VDDI_LGD_OFFSET			0
#define	DP173_CURRENT_SLEEP_VDDVDH_LGD_OFFSET		0
#define	DP173_CURRENT_SLEEP_VDDEL_LGD_OFFSET		0


/* define for DP173 TOUCH */
#define	DP173_TOUCH_I2C_SLAVE_ADDR		0x28


/* type define */
typedef struct model_dp173_info_s {
	int	key_dev;
	int	model_dp173_id;
	int	next_model_id;
	int	buf_index;
	char image_directory;
	char display_image_file_name[MAX_DISPLAY_IMAGE_NUM][MAX_FILE_NAME_LENGTH];
	MODEL_MANAGER dp173_manager;
	char display_image_dir[MAX_FILE_NAME_LENGTH];
	vfos_version_info_t version_info;
} model_dp173_info_t;

#define SOLOMON		(1)
#define SYNAPTICS	(2)
typedef struct model_dp173_s {
	pthread_t id_dp173_thread;
	pthread_t id_gray_scan_thread;
	pthread_t id_dimming_thread;
	pthread_t id_dsc_roll_thread;
	pthread_mutex_t dp173_thread_mutex;
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
	int cur_key_value;				/* LWG 191106 : need for many things */
	int cur_image_num;
	int cur_test_mode;				/* test mode - Display or OTP or Touch or Current */
	unsigned char otp_read_val[DP173_OTP_READ_LENGTH];
	int flag_otp_test_result_ch1;
	int flag_otp_test_result_ch2;
	int flag_touch_test_result_ch1;
	int flag_touch_test_result_ch2;
	int flag_current_test_result_ch1;
	int flag_current_test_result_ch2;
	int flag_display_test_result;
	int flag_synaptics;
	pocb_status_t pocb_status;
	model_dp173_info_t	model_dp173_info;
	special_pattern_mode_t special_pattern_mode;
	current_test_result_t current_test_result[DP173_CURRENT_TEST_CHANNEL_NUM][MAX_VOLT_NUM];	/* current result */
	mipi_read_data_t g_mipi_read;		/*mipi read buffer.. only use model_dp173.c*/
	mipi_write_data_t g_mipi_write;		/*mipi write buffer.. only use model_dp173.c*/
	int touch_ic_kind;
	int ch1_label_status;                           /* if label fail, flag_otp_test_result also fail */
	int ch2_label_status;                           /* if label fail, flag_otp_test_result also fail */
	int ch1_pocb_status;				/* if pocb fail, flag_otp_test_result also fail */
	int ch2_pocb_status;				/* if pocb fail, flag_otp_test_result also fail */
} model_dp173_t;

/* constant define */
#if 0
// FOR SOLOMON TOUCH TEST
enum{
	TOUCH_SOLOMON_SPI_TEST = 0,
	TOUCH_SOLOMON_FW_VER_TEST,
	TOUCH_SOLOMON_CONFIG_VER_TEST,
	TOUCH_SOLOMON_ATMEL_PRODUCT_ID_TEST,
	TOUCH_SOLOMON_NODE_TEST,
	TOUCH_SOLOMON_DELTA_TEST,
	TOUCH_SOLOMON_PIN_FAULT_TEST,
	TOUCH_SOLOMON_MICRO_TEST,
	TOUCH_SOLOMON_FORCE_TEST,
};

// FOR SYNAPTICS TOUCH TEST : 17ea
enum{
	TOUCH_SYNAPTICS_ATTN_PIN_TEST = 0,			// TD01
	TOUCH_SYNAPTICS_FW_VER_TEST,				// TD02
	TOUCH_SYNAPTICS_CONFIG_VER_TEST,			// TD03
	TOUCH_SYNAPTICS_DEV_PACKAGE_TEST,			// TD04
	TOUCH_SYNAPTICS_FULL_RAW_CAP_TEST,			// TD06
	TOUCH_SYNAPTICS_CM_JITTER_TEST,				// TD07 ( noise )
	TOUCH_SYNAPTICS_SENSOR_SPEED_TEST,			// TD08
	TOUCH_SYNAPTICS_HYBRID_ABS_RAW_CAP_TEST,	// TD11	( TX, RX )
	TOUCH_SYNAPTICS_ADC_RAW_CAP_TEST,			// TD15
	TOUCH_SYNAPTICS_TRX_SHORT_TEST,				// TD17 ( TRX-TRX, TRX-GND )
	TOUCH_SYNAPTICS_EXTENDED_HIGH_RESIST_TEST,	// TD18
	TOUCH_SYNAPTICS_FULL_RAW_CAP_GAP_H_TEST,	// TD19
	TOUCH_SYNAPTICS_FULL_RAW_CAP_GAP_V_TEST,	// TD20
	TOUCH_SYNAPTICS_CUSTOMER_ID_TEST,			// TD26
	TOUCH_SYNAPTICS_LOCKDOWN_TEST,				// TD27
	TOUCH_SYNAPTICS_RESET_PIN_TEST,				// TD28	( third sequence )
//	TOUCH_SYNAPTICS_BSC_CALIBRATION_TEST,		// TD32
	TOUCH_SYNAPTICS_AVDD_DVDD_TEST,				// TD35 ( first sequence )
	TOUCH_SYNAPTICS_OSC_TEST,					// TD36
	TOUCH_SYNAPTICS_ATTN2_PIN_TEST,				// TD37
	TOUCH_SYNAPTICS_HSYNC_PIN_TEST				// TD38
};
#endif
/* function proto type */
void put_info_for_dp173_thread(model_dp173_info_t *);
void start_dp173_thread(void);
int init_model_dp173(void);
int release_model_dp173(void);
int get_last_key_value_for_dp173(void);

#endif	/* __MODEL_DP173_H__ */
