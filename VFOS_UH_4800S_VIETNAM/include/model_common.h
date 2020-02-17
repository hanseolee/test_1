#ifndef	__MODEL_COMMON_H__
#define	__MODEL_COMMON_H__

#define	VFOS_CHANNEL_1_NUM		0
#define	VFOS_CHANNEL_2_NUM		1

#define	MAX_DISPLAY_IMAGE_NUM	300
#define	MAX_FILE_NAME_LENGTH	100

#define	PARSING_BY_DECIMAL		10
#define	PARSING_BY_HEX			16

/* define of decon ioctl */
#define DECON_IOC_FRAME_UPDATE_START    _IO('F', 230)
#define DECON_IOC_FRAME_UPDATE_STOP     _IO('F', 231)

/* define for delay */
#define	DELAY_1MS		usleep(1000)
#define	DELAY_10MS		usleep(10000)
#define	DELAY_20MS		usleep(20000)
#define	DELAY_30MS		usleep(30000)
#define	DELAY_40MS		usleep(40000)
#define	DELAY_50MS		usleep(50000)
#define	DELAY_100MS		usleep(100000)

/* define for parsing pattern code */
#define	MAX_MIPI_REGISTER_SET_COUNT	100
#define	MAX_MIPI_DATA_SET_COUNT		100
#define	MAX_PARSING_NAME_LENGTH		100
#define	MAX_PARSING_STRING_LENGTH	500
#define	MAX_CODE_NAME_STRING_LENGTH	30

/* define strings for parsing from image file name */
/* do not use naming "_" */
#define	STRING_SCAN_CAP_SMALL		"Scan"
#define	STRING_SCAN_SMALL			"scan"
#define	STRING_FBRIGHTLINE_CAP		"FBRIGHTLINE"
#define	STRING_FBRIGHTLINE_SMALL	"fbrightline"
#define	STRING_FBLACKLINE_CAP		"FBLACKLINE"
#define	STRING_FBLACKLINE_SMALL		"fblackline"
#define	STRING_FBRIGHTLINE50_CAP	"FBRIGHTLINE50"
#define	STRING_FBRIGHTLINE50_SMALL	"fbrightline50"
#define	STRING_FBLACKLINE50_CAP		"FBLACKLINE50"
#define	STRING_FBLACKLINE50_SMALL	"fblackline50"
#define	STRING_BLACKPOINT_CAP		"BLACKPOINT"
#define	STRING_BLACKPOINT_SMALL		"blackpoint"
#define	STRING_BRIGHTLINE_CAP		"BRIGHTLINE"
#define	STRING_BRIGHTLINE_SMALL		"brightline"
#define	STRING_BLACKLINE_CAP		"BLACKLINE"
#define	STRING_BLACKLINE_SMALL		"blackline"
#define	STRING_VARIABLE_CAP			"VARIABLE"
#define	STRING_VARIABLE_SMALL		"variable"
#define	STRING_EMCONPWM_CAP			"EMCONPWM"
#define	STRING_EMCONPWM_SMALL		"emconpwm"
#define	STRING_EMCON_CAP			"EMCON"
#define	STRING_EMCON_SMALL			"emcon"
#define	STRING_DIM_CAP_SMALL		"Dim"
#define	STRING_DIM_SMALL			"dim"
#define	STRING_DBV10_CAP			"DBV10"
#define	STRING_DBV10_SMALL			"dbv10"
#define	STRING_AOD_CAP				"AOD"
#define	STRING_AOD_SMALL			"aod"
#define	STRING_VR_CAP				"VR"
#define	STRING_VR_SMALL				"vr"
#define	STRING_SLEEP_CAP_SMALL		"Sleep"
#define	STRING_SLEEP_SMALL			"sleep"
#define	STRING_DSC_CAP				"DSC"
#define	STRING_DSC_SMALL			"dsc"
#define	STRING_BORDERTEST_CAP		"BORDERTEST"
#define	STRING_BORDERTEST_SMALL		"bordertest"
#define	STRING_GRAD_CAP				"GRAD"
#define	STRING_GRAD_SMALL			"grad"
#define	STRING_RGB_CAP				"RGB"
#define	STRING_RGB_SMALL			"rgb"
#define	STRING_RED_CAP				"RED"
#define	STRING_RED_SMALL			"red"
#define	STRING_HT_CAP				"HT"
#define	STRING_HT_SMALL				"ht"
#define	STRING_HD_CAP				"HD"
#define	STRING_HD_SMALL				"hd"
#define	STRING_VL_CAP				"VL"
#define	STRING_VL_SMALL				"vl"
//#define	STRING_VR_CAP				"VR"
//#define	STRING_VR_SMALL				"vr"
#define	STRING_DBVVARIABLE_CAP		"DBVVARIABLE"
#define	STRING_DBVVARIABLE_SMALL	"dbvvariable"
#define	STRING_BLACKPOINT_VARIABLE_CAP		"BLACKPOINTVARIABLE"
#define	STRING_BLACKPOINT_VARIABLE_SMALL	"blackpointvariable"

#define	STRING_DBV_1ST_NIT_CAP				"1ST"
#define	STRING_DBV_1ST_NIT_SMALL			"1st"
#define	STRING_DBV_2ND_NIT_CAP				"2ND"
#define	STRING_DBV_2ND_NIT_SMALL			"2nd"
#define	STRING_DBV_3RD_NIT_CAP				"3RD"
#define	STRING_DBV_3RD_NIT_SMALL			"3rd"
#define STRING_DBV_4RD_NIT_CAP				"4TH"
#define STRING_DBV_4RD_NIT_SMALL			"4th"
#define STRING_DBV_5RD_NIT_CAP				"5TH"
#define STRING_DBV_5RD_NIT_SMALL			"5th"
#define	STRING_DBV_4TH_NIT_CAP				"4NIT"
#define	STRING_DBV_4TH_NIT_SMALL			"4nit"

#define	STRING_HBM_CAP				"HBM"
#define	STRING_HBM_SMALL			"hbm"

#define	STRING_WHITE_BOX_CAP			"WHITEBOX"
#define	STRING_WHITE_BOX_SMALL			"whitebox"

#define	STRING_DARKMURA_CAP			    "DARKMURA"
#define	STRING_DARKMURA_SMALL			"darkmura"

#define STRING_ALP1_CAP				"ALP1"
#define STRING_ALP1_SMALL			"alp1"
#define STRING_ALP2_CAP				"ALP2"
#define STRING_ALP2_SMALL			"alp2"
#define STRING_ALP3_CAP				"ALP3"
#define STRING_ALP3_SMALL			"alp3"

#define STRING_VOLTAGE_CAP			"VOLTAGE"
#define STRING_VOLTAGE_SMALL		"voltage"

#define STRING_HL_L_CAP				"HLL"
#define STRING_HL_L_SMALL			"hll"

#define STRING_VL_L_CAP				"VLL"
#define STRING_VL_L_SMALL			"vll"

#define STRING_L_EM_CAP				"LEM"
#define STRING_L_EM_SMALL			"lem"

#define STRING_R_EM_CAP				"REM"
#define STRING_R_EM_SMALL			"rem"

#define STRING_60HZ_CAP				"60HZ"
#define STRING_60HZ_SMALL			"60hz"

#define STRING_90HZ_CAP				"90HZ"
#define STRING_90HZ_SMALL			"90hz"

#define STRING_FLOAT_CAP			"FLOAT"
#define STRING_FLOAT_SMALL			"float"

#define STRING_HFR_90HZ_CAP         "HFR90HZ"
#define STRING_HFR_90HZ_SMALL       "hfr90hz"

#define STRING_HFR_60HZ_CAP         "HFR60HZ"
#define STRING_HFR_60HZ_SMALL       "hfr60hz"

#define STRING_DIMMING_60TO10_CAP   "DIM60TO10"
#define STRING_DIMMING_60TO10_SMALL "DIM60TO10"

#define STRING_DIMMING_10TO60_CAP   "DIM10TO60"
#define STRING_DIMMING_10TO60_SMALL "dim10to60"

#define STRING_START_CAP   "START"
#define STRING_START_SMALL "start"

#define STRING_END_CAP   "END"
#define STRING_END_SMALL "end"

/* define for image directory parsing - not used yet */
#define	IMAGE_FILES_DIR_CODE_NAME	"SDCARD_DIR_PATH"

/* define for command parsing */
#define	EM_ON_CODE_NAME							"EM_ON"
#define	EM_OFF_CODE_NAME						"EM_OFF"
#define	AOD_ON_CODE_NAME						"AOD_ON"
#define	AOD_OFF_CODE_NAME						"AOD_OFF"
#define	SLEEP_IN_CODE_NAME						"SLEEP_IN"
#define	SLEEP_OUT_CODE_NAME						"SLEEP_OUT"
#define	BRIGHT_LINE_ON_CODE_NAME				"BRIGHT_LINE_ON"
#define	BRIGHT_LINE_OFF_CODE_NAME				"BRIGHT_LINE_OFF"
#define	BLACK_LINE_ON_CODE_NAME					"BLACK_LINE_ON"
#define	BLACK_LINE_OFF_CODE_NAME				"BLACK_LINE_OFF"
#define	BLACK_POINT_ON_CODE_NAME				"BLACK_POINT_ON"
#define	BLACK_POINT_OFF_CODE_NAME				"BLACK_POINT_OFF"
#define	VARIABLE_ON_CODE_NAME					"VARIABLE_ON"
#define	VARIABLE_OFF_CODE_NAME					"VARIABLE_OFF"
#define	VR_ON_CODE_NAME							"VR_ON"
#define	VR_OFF_CODE_NAME						"VR_OFF"
#define	BIST_IN_CODE_NAME						"BIST_IN"
#define	BIST_OUT_CODE_NAME						"BIST_OUT"
#define	BIST_START_CODE_NAME					"BIST_START"
#define	DBV_IN_CODE_NAME						"DBV_IN"
#define	DBV_OUT_CODE_NAME						"DBV_OUT"
#define	DISPLAY_ON_CODE_NAME					"DISPLAY_ON"
#define	DISPLAY_OFF_CODE_NAME					"DISPLAY_OFF"
#define	OTP_READ_PRE_WRITE_CODE_NAME			"OTP_READ_PRE_WRITE"
#define	OTP_READ_CODE_NAME						"OTP_READ"
#define	CRC_READ_PRE_WRITE_CODE_NAME			"CRC_READ_PRE_WRITE"
#define	CRC_READ_99_CODE_NAME					"CRC_READ_99"
#define CRC_READ_9A_CODE_NAME					"CRC_READ_9A"
#define CRC_READ_9B_CODE_NAME					"CRC_READ_9B"
#define CRC_READ_9C_CODE_NAME					"CRC_READ_9C"
#define CRC_READ_9D_CODE_NAME					"CRC_READ_9D"
#define CRC_READ_9E_CODE_NAME					"CRC_READ_9E"
#define CRC_READ_9F_CODE_NAME					"CRC_READ_9F"
#define CRC_READ_A0_CODE_NAME					"CRC_READ_A0"
#define CRC_READ_CODE_NAME						"CRC_READ"
#define CRC_READ_2_PRE_WRITE_CODE_NAME			"CRC_READ_2_PRE_WRITE"
#define CRC_READ_2_CODE_NAME					"CRC_READ_2"
#define	POCB_READ_PRE_WRITE_CODE_NAME			"POCB_READ_PRE_WRITE"
#define	POCB_READ_CODE_NAME						"POCB_READ"
#define POCB_READ_2_PRE_WRITE_CODE_NAME			"POCB_READ_2_PRE_WRITE"
#define POCB_READ_2_CODE_NAME					"POCB_READ_2"
#define	POCB_ON_WRITE_CODE_NAME					"POCB_ON_WRITE"
#define	POCB_OFF_WRITE_CODE_NAME				"POCB_OFF_WRITE"
#define	POCB_ERROR_READ_CODE_NAME				"POCB_ERROR_READ"
#define	DBV_VARIABLE_ON_CODE_NAME				"DBV_VARIABLE_ON"
#define	DBV_VARIABLE_OFF_CODE_NAME				"DBV_VARIABLE_OFF"
#define	BLACKPOINT_VARIABLE_ON_CODE_NAME		"BLACKPOINT_VARIABLE_ON"
#define	BLACKPOINT_VARIABLE_OFF_CODE_NAME		"BLACKPOINT_VARIABLE_OFF"
#define	DBV_1ST_NIT_ON_CODE_NAME				"DBV_1STNIT_ON"
#define	DBV_1ST_NIT_OFF_CODE_NAME				"DBV_1STNIT_OFF"
#define	DBV_2ND_NIT_ON_CODE_NAME				"DBV_2NDNIT_ON"
#define	DBV_2ND_NIT_OFF_CODE_NAME				"DBV_2NDNIT_OFF"
#define	DBV_3RD_NIT_ON_CODE_NAME				"DBV_3RDNIT_ON"
#define	DBV_3RD_NIT_OFF_CODE_NAME				"DBV_3RDNIT_OFF"
#define	DBV_4TH_NIT_ON_CODE_NAME				"DBV_4NIT"
#define	DBV_REFERENCE_CODE_NAME					"DBV_REFERENCE" 
#define ALP1_ON_CODE_NAME						"ALP1_ON"
#define ALP1_OFF_CODE_NAME						"ALP1_OFF"
#define ALP2_ON_CODE_NAME						"ALP2_ON"
#define ALP2_OFF_CODE_NAME						"ALP2_OFF"
#define ALP3_ON_CODE_NAME						"ALP3_ON"
#define ALP3_OFF_CODE_NAME						"ALP3_OFF"
#define	HBM_ON_CODE_NAME			        	"HBM_ON"
#define	HBM_OFF_CODE_NAME			            "HBM_OFF"
#define VOLTAGE_ON_CODE_NAME					"VOLTAGE_ON"
#define VOLTAGE_OFF_CODE_NAME					"VOLTAGE_OFF"
#define	WHITE_BOX_ON_CODE_NAME			       	"WHITE_BOX_ON"
#define	WHITE_BOX_OFF_CODE_NAME			       	"WHITE_BOX_OFF"
#define	DARKMURA_ON_CODE_NAME			       	"DARKMURA_ON"
#define	DARKMURA_OFF_CODE_NAME			       	"DARKMURA_OFF"
#define HL_L_ON_CODE_NAME						"HL_L_ON"
#define HL_L_OFF_CODE_NAME						"HL_L_OFF"
#define VL_L_ON_CODE_NAME						"VL_L_ON"
#define VL_L_OFF_CODE_NAME						"VL_L_OFF"
#define L_EM_ON_CODE_NAME						"L_EM_ON"
#define L_EM_OFF_CODE_NAME						"L_EM_OFF"
#define R_EM_ON_CODE_NAME						"R_EM_ON"
#define R_EM_OFF_CODE_NAME						"R_EM_OFF"
#define	LABEL_READ_PRE_WRITE_CODE_NAME			"LABEL_READ_PRE_WRITE"
#define LABEL_2_READ_PRE_WRITE_CODE_NAME		"LABEL_2_READ_PRE_WRITE"
#define	LABEL_READ_CODE_NAME				    "LABEL_READ"
#define	LABEL_2_READ_CODE_NAME				    "LABEL_2_READ"
#define	LABEL_CODE_NAME				            "LABEL"
#define HZ_ON_WRITE_CODE_NAME                                   "HZ_ON"
#define HZ_OFF_WRITE_CODE_NAME                                  "HZ_OFF"
#define HZ_60_ON_WRITE_CODE_NAME                                   "60HZ_ON"
#define HZ_60_OFF_WRITE_CODE_NAME                                  "60HZ_OFF"
#define HZ_90_ON_WRITE_CODE_NAME                                   "90HZ_ON"
#define HZ_90_OFF_WRITE_CODE_NAME                                  "90HZ_OFF"
#define DBV_4RD_NIT_ON_CODE_NAME				"DBV_4RDNIT_ON"
#define DBV_4RD_NIT_OFF_CODE_NAME				"DBV_4RDNIT_OFF"
#define DBV_5RD_NIT_ON_CODE_NAME				"DBV_5RDNIT_ON"
#define DBV_5RD_NIT_OFF_CODE_NAME				"DBV_5RDNIT_OFF"
#define FLOAT_ON_CODE_NAME						"FLOAT_ON"
#define FLOAT_OFF_CODE_NAME						"FLOAT_OFF"
#define HFR_90HZ_ON_CODE_NAME			"HFR_90HZ_ON"
#define HFR_90HZ_OFF_CODE_NAME			"HFR_90HZ_OFF"
#define HFR_60HZ_ON_CODE_NAME			"HFR_60HZ_ON"
#define HFR_60HZ_OFF_CODE_NAME			"HFR_60HZ_OFF"
#define DIMMING_60TO10_ON_CODE_NAME     "DIMMING_60TO10_ON"
#define DIMMING_60TO10_OFF_CODE_NAME    "DIMMING_60TO10_OFF"
#define DIMMING_10TO60_ON_CODE_NAME     "DIMMING_10TO60_ON"
#define DIMMING_10TO60_OFF_CODE_NAME    "DIMMING_10TO60_OFF"
#define	STRING_MIPI_WRITE_COMMAND		"mipi.write"
#define	STRING_MIPI_READ_COMMAND		"mipi.read"
#define	STRING_LABEL_LIMIT              "label.limit"
//#define	STRING_MIPI_READ_LENGTH_COMMAND	"READ_LEN"
#define	STRING_END_COMMAND				"END"
#define	STRING_DELAY_COMMAND_CAP_SMALL	"Delay"
#define	STRING_DELAY_COMMAND_SMALL		"delay"
#define	COMMENT_START_CHARACTER			'#'
#define	DEFINE_OF_DELAY_COMMAND			0x00
#define	STRING_PPS_SET_COMMAND			"PPS0x0A"
#define	DEFINE_OF_PPS_SET_COMMAND		0x0A

/* special pattern mode */
#define	NORMAL_MODE						(0)
#define	GRAY_MODE						(1ULL << 1)
#define	POWER_BRIGHT_LINE_MODE			(1ULL << 2)
#define	POWER_BLACK_LINE_MODE			(1ULL << 3)
#define	LU_50P_POWER_BRIGHT_LINE_MODE	(1ULL << 4)
#define	LU_50P_POWER_BLACK_LINE_MODE	(1ULL << 5)
#define	BLACKPOINT_MODE					(1ULL << 6)
#define	BRIGHT_LINE_MODE				(1ULL << 7)
#define	BLACK_LINE_MODE					(1ULL << 8)
#define	VARIABLE_MODE					(1ULL << 9)
#define	EMCONTROL_IN_PWM_MODE			(1ULL << 10)
#define	EMCONTROL_NO_PWM_MODE			(1ULL << 11)
#define	DIMMING_MODE					(1ULL << 12)
#define	DBV_MODE						(1ULL << 13)
#define	AOD_MODE						(1ULL << 14)
#define	VR_MODE							(1ULL << 15)
#define	SLEEP_MODE						(1ULL << 16)
#define	DSC_ROLL_MODE					(1ULL << 17)
#define	BDTEST_MODE						(1ULL << 18)
#define	GRAD_MODE						(1ULL << 19)
#define	IFPREV_MODE						(1ULL << 20)
#define	GRAD_RED_MODE					(1ULL << 21)
#define	GRAD_RGB_MODE					(1ULL << 22)
#define	HORIZONTAL_TOP_MODE				(1ULL << 23)
#define	HORIZONTAL_DOWN_MODE			(1ULL << 24)
#define	VERTICAL_LEFT_MODE				(1ULL << 25)
#define	VERTICAL_RIGHT_MODE				(1ULL << 26)
#define	POCB_WRITE_ENABLE_MODE			(1ULL << 27)
#define	DBV_VARIABLE_MODE				(1ULL << 28)
#define	BLACKPOINT_VARIABLE_MODE		(1ULL << 29)
#define	DBV_4NIT_MODE					(1ULL << 30)
#define	DBV_1ST_NIT_MODE				(1ULL << 31)
#define	DBV_2ND_NIT_MODE				(1ULL << 32) /* used 64 bitshift */
#define	DBV_3RD_NIT_MODE				(1ULL << 33)
#define	HBM_MODE           				(1ULL << 34)
#define	WHITE_BOX_MODE        			(1ULL << 35)
#define	DARKMURA_MODE        			(1ULL << 36) /* only DP076 model */
#define ALP1_MODE						(1ULL << 37)
#define ALP2_MODE						(1ULL << 38)
#define VOLTAGE_MODE					(1ULL << 39)
#define HL_L_MODE						(1ULL << 40)
#define VL_L_MODE						(1ULL << 41)
#define L_EM_MODE						(1ULL << 42)
#define R_EM_MODE						(1ULL << 43)
#define HZ_60_MODE						(1ULL << 44)
#define HZ_90_MODE						(1ULL << 45)
#define DBV_4RD_NIT_MODE				(1ULL << 46)
#define DBV_5RD_NIT_MODE				(1ULL << 47)
#define FLOAT_MODE						(1ULL << 48)
#define HFR_90HZ_MODE					(1ULL << 49)
#define HFR_60HZ_MODE					(1ULL << 50)
#define DIMMING_60TO10_MODE             (1ULL << 51)
#define DIMMING_10TO60_MODE             (1ULL << 52)
#define START_MODE             			(1ULL << 53)
#define END_MODE             			(1ULL << 54)

/* define for timeout */
#define	TIMEOUT_WAIT_FOR_FINISH_THREAD		100

/* define for control special pattern mode */
#define	SPECIAL_PATTERN_PREVIOUS_MODE	0
#define	SPECIAL_PATTERN_CURRENT_MODE	1

/* define for DSC_ROLL */
#define	DSC_ROLL_TEST_DIR_STRING		"_dsc"

/* define for POCB */
#define	POCB_STATUS_OFF			0
#define	POCB_STATUS_ON			1
#define	POCB_STATUS_NO_READ		2

#define	POCB_STATUS_CHANGED		1
#define	POCB_STATUS_NO_CHANGE	0

#define	POCB_CHANNEL_1			0
#define	POCB_CHANNEL_2			1

#define	POCB_WRITE_DEFAULT_CHANNEL_NUM	2

/* define for CURRENT test */
#define	MAX_VOLT_NUM		4
#define	MAX_VOLT_NUM_VDDD	5

#define	CURRENT_DIR_NAME			"current"
#define	CURRENT_TEST_FILE_STRING		"c_test_"
#define	CURRENT_TEST_FILE_EXT		".jpg"
#define	CURRENT_TEST_I2C_1_DEV		"/dev/i2c-1"
#define	CURRENT_TEST_I2C_2_DEV		"/dev/i2c-2"
#define CURRENT_TEST_I2C_3_DEV		"/dev/i2c-13"
#define CURRENT_TEST_I2C_4_DEV		"/dev/i2c-9"

/* define of global test mode */
#define	VFOS_RESET_MODE				0
#define	VFOS_OTP_TEST_MODE			1
#define	VFOS_TOUCH_TEST_MODE		2
#define	VFOS_CURRENT_TEST_MODE		3
#define	VFOS_DISPLAY_TEST_MODE		4

/* define for version information */
#define	VFOS_MAX_VERSION_LENGTH			10

/* define for pattern number */
#define	BLACK_PATTERN_COMMAND_NUM					11
#define	WHITE_PATTERN_COMMAND_NUM					12
#define	GRAD_RGB_HT_PATTERN_COMMAND_NUM				14
#define	GRAD_RGB_HD_PATTERN_COMMAND_NUM				14-1
#define	GRAD_RGB_VL_PATTERN_COMMAND_NUM				15
#define	GRAD_RGB_VR_PATTERN_COMMAND_NUM				15-1
#define	GRAD_RED_HT_PATTERN_COMMAND_NUM				13-3
#define	GRAD_RED_HD_PATTERN_COMMAND_NUM				13-2
#define	GRAD_RED_VL_PATTERN_COMMAND_NUM				15
#define	GRAD_RED_VR_PATTERN_COMMAND_NUM				15-1

#define	MAX_PATTERN_NUM_STRING_NUM					10
#define	GRAD_RGB_HT_PATTERN_COMMAND_NUM_STRING		"14"
#define	GRAD_RGB_HD_PATTERN_COMMAND_NUM_STRING		"14-1"
#define	GRAD_RGB_VL_PATTERN_COMMAND_NUM_STRING		"15"
#define	GRAD_RGB_VR_PATTERN_COMMAND_NUM_STRING		"15-1"
#define	GRAD_RED_HT_PATTERN_COMMAND_NUM_STRING		"13-3"
#define	GRAD_RED_HD_PATTERN_COMMAND_NUM_STRING		"13-2"
#define	GRAD_RED_VL_PATTERN_COMMAND_NUM_STRING		"15"
#define	GRAD_RED_VR_PATTERN_COMMAND_NUM_STRING		"15-1"

/* 
 * UART commaand define 
 * */
/* Version info */
#define	VER_INFO_MODEL_ID_BUF_NUM			4
#define	VER_INFO_VFOS_VER_BUF_NUM			5
#define	VER_INFO_VFOS_REV_BUF_NUM			6
#define	VER_INFO_VFOS_REV_MINOR_BUF_NUM		7
#define VER_INFO_TOUCH_VER_BYTE_7_BUF_NUM	8
#define VER_INFO_TOUCH_VER_BYTE_6_BUF_NUM	9
#define VER_INFO_TOUCH_VER_BYTE_5_BUF_NUM	10
#define VER_INFO_TOUCH_VER_BYTE_4_BUF_NUM	11
#define VER_INFO_TOUCH_VER_BYTE_3_BUF_NUM	12
#define VER_INFO_TOUCH_VER_BYTE_2_BUF_NUM	13
#define VER_INFO_TOUCH_VER_BYTE_1_BUF_NUM	14
#define VER_INFO_TOUCH_VER_BYTE_0_BUF_NUM	15
#define	VER_INFO_SITE_VER_BUF_NUM			16

/* type define */
typedef struct vfos_version_info_s {
	unsigned char display_version[VFOS_MAX_VERSION_LENGTH];
	unsigned char touch_version[VFOS_MAX_VERSION_LENGTH];
	unsigned char site_version;
} vfos_version_info_t;

typedef struct dbv_pattern_mode_s {
	int dbv_level;								/* DBV level value for DBV mode */
} dbv_pattern_mode_t;

typedef struct dsc_pattern_mode_s {
	int dsc_roll_pic_num;								/* Picture number for DSC_ROLL mode */
	char dsc_roll_name_string[MAX_FILE_NAME_LENGTH];	/* name to trace files for DSC_ROLL mode */
} dsc_pattern_mode_t;
typedef struct hz_status_s {
           unsigned char hz_init_status[POCB_WRITE_DEFAULT_CHANNEL_NUM];                   /* POCB status which was read in OTP key action */
           unsigned char hz_cur_status[POCB_WRITE_DEFAULT_CHANNEL_NUM];                    /* Current POCB status */
           int flag_hz_changed;                                                                                                    /* flag for whether or not POCB status is changed by NEXT or PREV key */
    } hz_status_t;
typedef struct pocb_status_s {
	unsigned char pocb_init_status[POCB_WRITE_DEFAULT_CHANNEL_NUM];			/* POCB status which was read in OTP key action */
	unsigned char pocb_cur_status[POCB_WRITE_DEFAULT_CHANNEL_NUM];			/* Current POCB status */
	int flag_pocb_changed;													/* flag for whether or not POCB status is changed by NEXT or PREV key */
} pocb_status_t;

typedef struct current_test_result_s {
	unsigned long voltage;		/* test result of voltage */
	unsigned long current;		/* test result of current */
	int is_over_limit;			/* whether or not current is over limit */
} current_test_result_t;

typedef struct special_pattern_mode_s {
	//unsigned int pattern_mode;					/* Pattern mode type */
	unsigned long long pattern_mode;			/* use when Pattern mode's numbering upper to 32 */
	dbv_pattern_mode_t dbv_pattern_mode;
	dsc_pattern_mode_t dsc_pattern_mode;
} special_pattern_mode_t;

typedef struct mipi_write_data_s {
	unsigned char data_buf[MAX_MIPI_REGISTER_SET_COUNT][MAX_MIPI_DATA_SET_COUNT];
	int reg_cnt;
	int data_cnt[MAX_MIPI_REGISTER_SET_COUNT];
} mipi_write_data_t;

typedef struct mipi_read_data_s {
	unsigned char data_buf[MAX_MIPI_REGISTER_SET_COUNT][MAX_MIPI_DATA_SET_COUNT];
	int reg_cnt;
	int data_cnt[MAX_MIPI_REGISTER_SET_COUNT];
} mipi_read_data_t;

/* function proto type */
void send_vfos_display_version_to_uart(vfos_version_info_t *, int);
void send_vfos_touch_version_to_uart(vfos_version_info_t *, int);
void send_touch_test_result_to_uart(unsigned int, int, int);
void send_touch_test_error_to_uart(int, int);
int parsing_pattern_mode(char *, special_pattern_mode_t *);
int parsing_pattern_write_command(char *, char *, mipi_write_data_t *);
int parsing_pattern_read_command(char *, char *, mipi_read_data_t *);
int set_mipi_port(int);
int write_mipi_command(mipi_write_data_t *);
int read_mipi_command(mipi_read_data_t *, int, unsigned char *);
int parsing_dsc_roll_test_file_name(char *, int, char *, char (*)[MAX_FILE_NAME_LENGTH]);
void send_pocb_status_to_uart(unsigned char,unsigned char,unsigned char,unsigned char,unsigned int);
void send_function_start_info_to_uart(int);
void send_otp_key_to_uart(int, unsigned char, int, unsigned char, unsigned char);
void send_func_key_to_uart(vfos_version_info_t *,int);
void send_reset_key_to_uart(void);
unsigned long get_measured_voltage(int);
unsigned long get_measured_current(int);
void display_logo_for_vfos(int);
void display_image_for_vfos(char *,char *);
void display_black_pattern_for_vfos(void);
void display_white_pattern_for_vfos(void);
void display_pattern_for_vfos(char *);
void display_grad_rgb_ht_pattern_for_vfos(void);
void display_grad_rgb_hd_pattern_for_vfos(void);
void display_grad_rgb_vl_pattern_for_vfos(void);
void display_grad_rgb_vr_pattern_for_vfos(void);
void display_grad_red_ht_pattern_for_vfos(void);
void display_grad_red_hd_pattern_for_vfos(void);
void display_grad_red_vl_pattern_for_vfos(void);
void display_grad_red_vr_pattern_for_vfos(void);
int start_decon_frame_update(void);
int stop_decon_frame_update(void);

#define PW_LEN		(10)

#endif	/* __MODEL_COMMON_H__ */

