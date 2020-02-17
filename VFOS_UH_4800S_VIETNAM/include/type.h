#ifndef	__TYPE_H__
#define	__TYPE_H__

#include <vfos_version.h>

#define	RECOVERY_CMD1	"/Data/reg_init /mnt/sd/initial/recovery_"
#define	RECOVERY_CMD2	".tty > /dev/null 2>&1"

/* if you need to read POCB data when sample power on(initial on), define POCB_FIRST_READ (180318)*/
//#define POCB_FIRST_READ 



#define	PASS			0
#define	FAIL			-1

#define	TRUE	1
#define	FALSE	0


#define ENABLE          1
#define DISABLE         0

#define	I2C_ERR		-5

#define TOTAL_MODEL     10

#define MAIN_START      20

#define	CONFIG_DIR			"/mnt/sd/A/"
#define	C_LIMIT_FILE		"current_limit_vfos2.bmp"
#define	T_LIMIT_FILE		"touch_limit_vfos2.bmp"
#define	T_LIMIT_TABLE_FILE	"touch_limit_table_vfos2.csv"

#define TOKEN_SEP		" \t\r\n"
#define TOKEN_SEP_UNDER  "_ \t\r\n"
#define TOKEN_SEP_UNDER  "_ \t\r\n"
#define TOKEN_SEP_UNDER_POINT  "._ \t\r\n"
#define TOKEN_SEP_COMMA  ", \t\r\n"

#define SUM_COUNT                   50

#define ON      1
#define OFF     0

/* define of debug print */
//#define	DEBUG_FUNCTION_FLOW		(1)

#if	defined(DEBUG_FUNCTION_FLOW)
#define	FUNC_BEGIN()	printf("---(%s)(line:%d)-BEGIN\n", __func__,__LINE__)
#define	FUNC_END()		printf("---(%s)(line:%d)-END\n", __func__,__LINE__)
#else
#define	FUNC_BEGIN()	
#define	FUNC_END()	
#endif	/* DEBUG_FUNCTION_FLOW */

#define	DEBUG_PRINT_ENABLE		(1)

#if	defined(DEBUG_PRINT_ENABLE)
#define	DPRINTF(fmt,...)	printf("(%s)(%d)"fmt, __func__,__LINE__,##__VA_ARGS__)
#else
#define	DPRINTF(fmt,...)	
#endif	/* DEBUG_PRINT_ENABLE */

#define	DEBUG_TOUCH_PRINT_ENABLE		(1)

#if	defined(DEBUG_TOUCH_PRINT_ENABLE)
//#define	DTPRINTF(fmt,...)	printf("(%s)(%d)"fmt, __func__,__LINE__,##__VA_ARGS__)	/* seems to avoid printing function name and line number to make much visable */
#define	DTPRINTF(fmt,...)	printf(fmt,##__VA_ARGS__)
#else
#define	DTPRINTF()	
#endif	/* DEBUG_PRINT_ENABLE */

/* define of error print */
#define	DERRPRINTF(fmt,...)	printf("(%s)(%d)ERR:"fmt, __func__,__LINE__,##__VA_ARGS__)

/* define of command */
#define	REG_INIT_COMMAND			"/Data/reg_init"
#define	PIC_VIEW_COMMAND			"/Data/pic_view"
#define	PATTERN_COMMAND				"/Data/Pattern"
#define	DECON_START_STOP_COMMAND	"B1"

/* define of data path */
#define	INITIAL_CODE_DIR_PATH	"/mnt/sd/initial"
#define	LOGO_DIR_PATH			"/mnt/sd/logo"
#define	SD_CARD_DIR_PATH		"/mnt/sd"

extern	char ch1_TDev[30];
extern	char ch2_TDev[30];


extern  int dic_dev;
extern  int dicOpen;
extern  int DEBUG_MODE;
extern  int PACKEY_DEBUG;
extern	char display_version[10];
extern	char touch_version[10];
extern  char site_version;

extern int temp_mode;
extern	int first_next;
extern	int first_prev;
#if 1
enum{
    OTP=1,
    TOUCH,
    CURRENT,
    FUNC,
    NEXT,
    PREV,
    RESET,
    FUNC2, 
	INTERLOCK,		// LWG
	JUDGE,
	SYNAPTICS_TOUCH_UI,
};



#else //test
enum{
    OTP=1,
    NEXT,
    PREV,
    FUNC,
    TOUCH,
    CURRENT,
    RESET,
    FUNC2,
};
#endif


enum {
    VCC1,
    VCC2,
    VDDVDH,
    VDDEL,
	TTL,
};

typedef struct key_event{
    struct timeval time;
    unsigned short type;
    unsigned short  code;
    unsigned long value;
}KEY_EVENT;


///////////////////////////////////////////////////

typedef enum{
    JOAN = 1,
    MV,
    JOAN_REL,
    JOAN_MANUAL,
    MV_MANUAL,
	A1,
    JOAN_E5,
	DP049,
	AKATSUKI,
	B1,
	MV_MQA,
	MV_DQA,
	STORM,
	DP076,
	AOP,
	ALPHA,
    DP086,
    F2,
	DP116,
	DP116_SYNAP,
	DP150,
	DP150_SYNAP,
	DP173,
	DP173_SYNAP,
}MODEL;

unsigned int en_model_count;
///////////////////////////////////////////////

typedef struct  roll_thread_s {
    MODEL   id;
    pthread_t id_gray_scan_thread;
    pthread_t id_dimming_thread;
    pthread_t id_dsc_roll_thread;
    pthread_mutex_t gray_scan_thread_mutex;
    pthread_mutex_t dimming_thread_mutex;
    pthread_mutex_t dsc_roll_thread_mutex;
} roll_thread_t;


//////////////////////////////////////////////////// LIMIT

#define	MAX_CURRENT_PATTERN_NUM		4
#define	MAX_CURRENT_VOLTAGE_NUM		5

struct  current_limit{
	MODEL	id;
    int pattern_count;
    int volt_count;
    unsigned int    max_current[MAX_CURRENT_PATTERN_NUM][MAX_CURRENT_VOLTAGE_NUM]; //max_current[total_pattern][total_voltage]
	char joan_pwm;
};

struct display_limit{
	MODEL	id;
	int	model_index;
	char    dir;
    int image_count;
    int vod_count;
    int now_img;
	unsigned char  ori_pocb_on_ch1; //pocb original state..(use dp049 etc..
	unsigned char  ori_pocb_on_ch2; //pocb original state..(use dp049 etc..
	char joan_pwm;
    int st_gray_scan_mode;
	int st_dbv_scan_mode;
	int st_dbv;
	int st_bist;
	int st_aod;
	int st_vr;
	int st_vod_play;
	//int st_dsc_flag_check;
	unsigned char st_pocb_on_ch1;
	unsigned char st_pocb_on_ch2;
	int st_sleep_mode;
	int st_dsc_roll_mode;
	int dsc_roll_ptn_num;
	char dsc_roll_buf[300];
	//int st_dsc_pt;
	unsigned char st_pocb_wr_en;
	int st_blackpoint;
	int st_gray_fin;
	int st_dbv_fin;
	int st_dsc_fin;
	int st_variable;
	int	st_emconpwm;
	int	st_emcon;
    int st_bright_line;
    int st_black_line;
	int st_power_bright_line;
	int st_power_black_line;
	int st_lu_50p_power_bright_line;
	int st_lu_50p_power_black_line;
    int st_bdtest_mode;
};

//JOAN
/*
struct joan_otp_limit{
	int test1;

};
*/
/*
struct joan_touch_limit{
	int test1;

};
*/

struct stm_touch_limit{

	MODEL id;
	unsigned short  product_id;
	unsigned short  fw_ver;
	unsigned short  config_ver;
	unsigned short  release_ver;
    unsigned int    pat_cm_reference_raw[2];
    unsigned int    pat_self_raw_tx[2];
    unsigned int    pat_self_raw_rx[2];
	int    cm_reference_raw[2]; // 180307
	int    cm_reference_gap; // 180307
	int             cm_jitter[2]; 
	unsigned int    total_cx[2];
	int    self_raw_tx[2]; // 180307
	int    self_raw_rx[2]; // 180307
    int    lp_self_raw_tx[2]; // 180307
    int    lp_self_raw_rx[2]; // 180307
	unsigned int    self_ix_tx[2];
	unsigned int    self_ix_rx[2];
	unsigned int    cx2_hf[2];
	unsigned char	otp_param;
	unsigned char	otp_param1;
	unsigned char	otp_param2;
	unsigned char	otp_param3;
	int				hf_test_mode;
    int     ss_jitter[2]; /// 180307 	
    int    lp_raw[2]; /// 180307
    int    hf_gap_rx[2]; /// 180307
    int    hf_gap_tx[2]; /// 180307

	
	int totalCx_MAX[300][300];
	int totalCx_MIN[300][300];
	int totalCx_Gap_Rx_MAX[300][300];
	int totalCx_Gap_Rx_MIN[300][300];
	int totalCx_Gap_Tx_MAX[300][300];
	int totalCx_Gap_Tx_MIN[300][300];
	
	int hf_TotalCx_Gap_Rx_MAX[300][300];
	int hf_TotalCx_Gap_Rx_MIN[300][300];
	int hf_TotalCx_Gap_Tx_MAX[300][300];
	int hf_TotalCx_Gap_Tx_MIN[300][300];

};

struct atmel_touch_limit{
    int id;
    unsigned short  fw_ver;
    unsigned int    config_ver;
    float           md_set_charge;
    double          md_x_center;
    double          md_y_center;
    double          md_x_border;
    double          md_y_border;

    int nodeDetection_MAX[300][300];
    int nodeDetection_MIN[300][300];
    int delta_MAX[300][300];
    int delta_MIN[300][300];
};

struct atmel_02_touch_limit{
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long nodeDetection_MAX[300][300];
	long nodeDetection_MIN[300][300];
	long delta_MAX[300][300];
	long delta_MIN[300][300];
	long micro_defect[300][300];
	long force_touch_MIN[300];
	long force_touch_MAX[300];
};

// LWG 190410 for DP116
struct atmel_03_touch_limit{
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long nodeDetection_MAX[300][300];
	long nodeDetection_MIN[300][300];
	long delta_MAX[300][300];
	long delta_MIN[300][300];
	long micro_defect[300][300];
	long force_touch_MIN[300];
	long force_touch_MAX[300];
};

// LWG 190424 for DP116 synaptics
struct synaptics_02_touch_limit{	// not tested
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long full_raw_cap_MIN[300][300];
	long full_raw_cap_MAX[300][300];
	float extend_high_regi_MIN[300][300];
	float extend_high_regi_MAX[300][300];
	long hybrid_raw_cap_TX_MIN[300][300];
	long hybrid_raw_cap_TX_MAX[300][300];
	long hybrid_raw_cap_RX_MIN[300][300];
	long hybrid_raw_cap_RX_MAX[300][300];
	
	char customer_id[300];
	long cm_jitter_MIN[300];
	long cm_jitter_MAX[300];
	long sensor_MIN[300];
	long sensor_MAX[300];

	char avdd_MAX[300];
	char dvdd_MAX[300];
	
	char avdd_MIN[300];
	char dvdd_MIN[300];

	long ADC_RANGE_MIN[300][300];
	long ADC_RANGE_MAX[300][300];

	long full_raw_cap_h[300][300];
	long full_raw_cap_v[300][300];
};

// LWG 191015 for DP150
struct atmel_04_touch_limit{
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long nodeDetection_MAX[300][300];
	long nodeDetection_MIN[300][300];
	long delta_MAX[300][300];
	long delta_MIN[300][300];
	long micro_defect[300][300];
	long force_touch_MIN[300];
	long force_touch_MAX[300];
};

// LWG 191015 for DP150 synaptics
struct synaptics_03_touch_limit{	// not tested
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long full_raw_cap_MIN[300][300];
	long full_raw_cap_MAX[300][300];
	float extend_high_regi_MIN[300][300];
	float extend_high_regi_MAX[300][300];
	long hybrid_raw_cap_TX_MIN[300][300];
	long hybrid_raw_cap_TX_MAX[300][300];
	long hybrid_raw_cap_RX_MIN[300][300];
	long hybrid_raw_cap_RX_MAX[300][300];
	
	char customer_id[300];
	long cm_jitter_MIN[300];
	long cm_jitter_MAX[300];
	long sensor_MIN[300];
	long sensor_MAX[300];

	char avdd_MAX[300];
	char dvdd_MAX[300];
	
	char avdd_MIN[300];
	char dvdd_MIN[300];

	long ADC_RANGE_MIN[300][300];
	long ADC_RANGE_MAX[300][300];

	long full_raw_cap_h[300][300];
	long full_raw_cap_v[300][300];
};

// LWG 200129 for DP173
struct atmel_05_touch_limit{
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long nodeDetection_MAX[300][300];
	long nodeDetection_MIN[300][300];
	long delta_MAX[300][300];
	long delta_MIN[300][300];
	long micro_defect[300][300];
	long force_touch_MIN[300];
	long force_touch_MAX[300];
};

// LWG 200129 for DP173 synaptics
struct synaptics_04_touch_limit{
	int id;
	char fw_ver[300];
	char config_ver[300];
	char product_id[300];
	
	long full_raw_cap_MIN[300][300];
	long full_raw_cap_MAX[300][300];
	float extend_high_regi_MIN[300][300];
	float extend_high_regi_MAX[300][300];
	long hybrid_raw_cap_TX_MIN[300][300];
	long hybrid_raw_cap_TX_MAX[300][300];
	long hybrid_raw_cap_RX_MIN[300][300];
	long hybrid_raw_cap_RX_MAX[300][300];
	
	char customer_id[300];
	long cm_jitter_MIN[300];
	long cm_jitter_MAX[300];
	long sensor_MIN[300];
	long sensor_MAX[300];

	char avdd_MAX[300];
	char dvdd_MAX[300];
	
	char avdd_MIN[300];
	char dvdd_MIN[300];

	long ADC_RANGE_MIN[300][300];
	long ADC_RANGE_MAX[300][300];

	long full_raw_cap_h[300][300];
	long full_raw_cap_v[300][300];
};


// 2/21 lee won guk add
struct stm_07_touch_limit{
	int id;         // ??
	char fw_ver[300];
	char config_ver[300];
	char rel_ver[300];
	
	long HF_Raw_Gap_H[300][300];
	long HF_Raw_Gap_V[300][300];
	long MS_Cx2_MAX[300][300];
	long MS_Cx2_MIN[300][300];
	long MS_Cx2_Gap_H_MAX[300][300];
	long MS_Cx2_Gap_V_MAX[300][300];     // hmm.........??
	long cm_raw_data_MIN[300][300];
    long cm_raw_data_MAX[300][300];
    long ss_total_ix_tx_MIN[300][300];
    long ss_total_ix_tx_MAX[300][300];
    long ss_total_ix_rx_MIN[300][300];
    long ss_total_ix_rx_MAX[300][300];
/*
	char ss_raw_data_tx_MIN[300];
    char ss_raw_data_tx_MAX[300];
    char ss_raw_data_rx_MIN[300];
    char ss_raw_data_rx_MAX[300];
*/
	long ss_raw_data_tx_MIN[300][300];
    long ss_raw_data_tx_MAX[300][300];
    long ss_raw_data_rx_MIN[300][300];
    long ss_raw_data_rx_MAX[300][300];
	char cm_jitter_MIN[300];
    char cm_jitter_MAX[300];
    long ss_total_tx_idle_MAX[300][300];
    long ss_total_tx_idle_MIN[300][300];
	long ss_raw_tx_idle_MAX[300][300];
	long ss_raw_tx_idle_MIN[300][300];
    char avdd_MAX[300];
    char dvdd_MAX[300];
};

struct synaptics_touch_limit{

   int     id;
   long    config; 
   long		fw_version;
   int     noise_test_min; 
   int     noise_test_max; 
#if	0	/* swchoi - comment as touch table will be used */
   long    hybrid_raw_cap_tx_min;  
   long    hybrid_raw_cap_tx_max;  
   long    hybrid_raw_cap_rx_00_min;   
   long    hybrid_raw_cap_rx_00_max;   
   long    hybrid_raw_cap_rx_other_min;    
   long    hybrid_raw_cap_rx_other_max;    
#endif	/* swchoi - end */
/*
   float     extended_high_regi_min; 
   float     extended_high_regi_max; 
*/
   long    device_package;    
	char	project_id[50];
	unsigned char project_id_len;
   float full_raw_cap_MAX[300][300];
   float full_raw_cap_MIN[300][300];
#if	1	/* swchoi - add as touch table should be used */
   long hybrid_raw_cap_rx_MAX[300][300];
   long hybrid_raw_cap_rx_MIN[300][300];
   long hybrid_raw_cap_tx_MAX[300][300];
   long hybrid_raw_cap_tx_MIN[300][300];
#endif	/* swchoi - end */
	float	extended_high_regi_MIN[300][300];
	float	extended_high_regi_MAX[300][300];
   /* side touch test */
	float side_touch_raw_cap_max[3][100];	/* Second array size should be set with define value but it depends on chip, so set to 100 to avoid leakage */
	float side_touch_raw_cap_min[3][100];	/* Second array size should be set with define value but it depends on chip, so set to 100 to avoid leakage */
	int side_touch_noise_max;	/* max value of side touch noise */
	int side_touch_noise_min;	/* min value of side touch noise */
};

struct siw_touch_limit{                                                                                                                                                               
    long    id;
    char    lg_product_id[300];                                                                                                                                                                       
    long    chip_id1[300][300];                                                                                                                                                       
    long    chip_id2[300][300];                                                                                                                                                       
    long    fw_version_major[300][300];                                                                                                                                               
    long    fw_version_minor[300][300];                                                                                                                                               
                                                                                                                                                                                      
    long    open_test_result1_MIN[300][300];                                                                                                                                          
    long    open_test_result1_MAX[300][300];                                                                                                                                          
    long    open_test_result2_MIN[300][300];                                                                                                                                          
    long    open_test_result2_MAX[300][300];                                                                                                                                          
                                                                                                                                                                                      
    long    short_test_result1_MIN[300][300];                                                                                                                                         
    long    short_test_result1_MAX[300][300];                                                                                                                                         
    long    short_test_result2_MIN[300][300];                                                                                                                                         
    long    short_test_result2_MAX[300][300];                                                                                                                                         
                                                                                                                                                                                      
    long    m2_raw_mutual_test_MIN[300][300];                                                                                                                                         
    long    m2_raw_mutual_test_MAX[300][300];                                                                                                                                         
    long    m2_raw_self_test_MIN[300][300];                                                                                                                                           
    long    m2_raw_self_test_MAX[300][300];                                                                                                                                           
                                                                                                                                                                                      
    long    m2_jitter_mutual_test_MIN[300][300];                                                                                                                                      
    long    m2_jitter_mutual_test_MAX[300][300];                                                                                                                                      
    long    m2_jitter_self_test_MIN[300][300];                                                                                                                                        
    long    m2_jitter_self_test_MAX[300][300];                                                                                                                                        
                                                                                                                                                                                      
    long    m1_raw_test_MIN[300][300];                                                                                                                                                
    long    m1_raw_test_MAX[300][300];
    
    long    m1_jitter_test_MIN[300][300];
    long    m1_jitter_test_MAX[300][300];
}; 






////////////////////////////////////////////////////// FOR MODEL SETTING

typedef struct status_flag{
    int key_act;
    int key_pre;
	int onoff;
	int next_model_index;
	int next_model_id;
}_status_flag;

struct Limit {
    MODEL id;
    void *ptouch;
    struct	current_limit current;
    struct  display_limit display;
    unsigned long touch_data_size;
};


typedef struct __model_manager{
    MODEL                   id;
    char                    name[30];
    struct  Limit           limit;
    int     en;
    int     buf_index;
	char    dir;

}MODEL_MANAGER;

/////////////////////////////////////////////////////// USE MODEL SETTING

MODEL_MANAGER   Joan;
MODEL_MANAGER   Mv;
MODEL_MANAGER   Joan_r;
MODEL_MANAGER   Joan_m;
MODEL_MANAGER   Mv_m;
MODEL_MANAGER   a1;
MODEL_MANAGER   JoanE5;
MODEL_MANAGER   Dp049;
MODEL_MANAGER   b1;
MODEL_MANAGER   Akatsuki;
MODEL_MANAGER   Mv_mqa;
MODEL_MANAGER   Mv_dqa;
MODEL_MANAGER	Storm;
MODEL_MANAGER	Dp076;
MODEL_MANAGER	Aop;
MODEL_MANAGER	Alpha;
MODEL_MANAGER	Dp086;
MODEL_MANAGER	f2;     // 이걸 이용해서 비교한다!
MODEL_MANAGER	Dp116;
MODEL_MANAGER	Dp150;
MODEL_MANAGER	Dp173;
MODEL_MANAGER   model_list[TOTAL_MODEL];

///////////////////////////////////////////////////////// DIC IOCTL

#define _IOCTL_DELAY                0x1007

#define _IOCTL_LED1_CONTROL         0x1115
#define _IOCTL_LED2_CONTROL         0x1116
#define _IOCTL_LED3_CONTROL         0x1119
#define _IOCTL_LED4_CONTROL         0x111A
#define _IOCTL_LED5_CONTROL         0x111B
#define _IOCTL_LED6_CONTROL         0x111C

#define _IOCTL_1CH_LCD_RESET        0x1120
#define _IOCTL_2CH_LCD_RESET        0x1121

#define _IOCTL_1CH_TOUCH_RESET      0x1122
#define _IOCTL_2CH_TOUCH_RESET      0x1123

#define _IOCTL_1CH_VPNL_EN          0x1124
#define _IOCTL_2CH_VPNL_EN          0x1125

#define _IOCTL_1CH_VDDI_EN          0x1126 
#define _IOCTL_2CH_VDDI_EN          0x1127

#define _IOCTL_1CH_VDDVDH_EN        0x1128
#define _IOCTL_2CH_VDDVDH_EN        0x1129

#define _IOCTL_TOUCH_EN_V1_8        0x112A

#define _IOCTL_TOUCH_EN_V3_3        0x112B

#define _IOCTL_CH1_GPIO_IO1         0x112C
#define _IOCTL_CH1_GPIO_IO2         0x112D
#define _IOCTL_CH2_GPIO_IO1         0x112E
#define _IOCTL_CH2_GPIO_IO2         0x112F

#define _IOCTL_CH1_GPIO_IO1_GET         0x1130      
#define _IOCTL_CH1_GPIO_IO2_GET         0x1131    
#define _IOCTL_CH2_GPIO_IO1_GET         0x1132
#define _IOCTL_CH2_GPIO_IO2_GET         0x1133  
//////////////////////////////////////////////////////// DEBUG


#ifdef DEBUG_MSG
void Debug(char *msg)
{
    printf("%s\n",msg);
}

void Debug_1(char *msg,int param)
{
    printf("%s[%d]\n",msg,param);
}
void Debug_s(char *msg,char *param)
{
    printf("%s[%s]\n",msg,param);
}
#else

static inline int Debug(char *msg)
{
	return 0;
}
static inline int Debug_1(char *msg,int param)
{
	return 0;
}
static inline int Debug_s(char *msg,char *param)
{
	return 0;
}

#endif


//////////////////////////////test
int scan_directory(char directory);
int scan_directory_vod(char directory);

int vfos_dev_open(void);
void bubble_short(unsigned long count, unsigned short *buf);
int key_counter(MODEL id, MODEL_MANAGER *model,struct status_flag *sf);
int module_initialize(int id, int model_index, char *name, int onoff,int logost);

unsigned char version_check_display(int id, int model_index);
unsigned char version_check_touch_limit(void);
int stm_control(MODEL   id, struct stm_touch_limit *t_limit);
int atmel_control(MODEL   id, struct atmel_touch_limit *t_limit);

#endif	/* __TYPE_H__ */
