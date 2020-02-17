#ifndef	__STM_TOUCH_H__
#define	__STM_TOUCH_H__

#define TOKEN_SEP_COMMA  ", \t\r\n"
#define TOKEN_SEP  " \t\r\n"

extern	int	stm_dev;
//////////////////////////////////
typedef struct stm_event
{
    unsigned short x;
    unsigned short y;
    unsigned short id;
    unsigned short pressure;
    unsigned char status;
}event_stm;
event_stm   ts_event;

enum    pure_type
{
    FAILED = -1,
    NOT_FINAL = 1,
    DONE = 2,
    PAT_ALREADY_SET = 3
};
///////////////////////////////////

enum {

    TOUCH_STM_I2C_CHECK = 0,
    TOUCH_STM_PRODUCT_ID,
    TOUCH_STM_FW_VER,
    TOUCH_STM_CONFIG_VER,
    TOUCH_STM_RELEASE_VER,
    TOUCH_STM_OTP_READ,
    TOUCH_STM_CM_REFERENCE_RAW,
    TOUCH_STM_CM_REFERENCE_GAP,
    TOUCH_STM_CM_JITTER,

    TOUCH_STM_TOTAL_CX,
    TOUCH_STM_TOTAL_CX_H,
    TOUCH_STM_TOTAL_CX_V,
    TOUCH_STM_SELF_RAW_TX,
    TOUCH_STM_SELF_RAW_RX,
    TOUCH_STM_LP_SELF_RAW_TX,
    TOUCH_STM_LP_SELF_RAW_RX,
    TOUCH_STM_IX_TOTAL_TX,

    TOUCH_STM_IX_TOTAL_RX,
    TOUCH_STM_SELF_IXCX_TX,
    TOUCH_STM_SELF_IXCX_RX,
    TOUCH_STM_OPEN_SHORT,
    TOUCH_STM_CX2_HF,
    TOUCH_STM_CX2_HF_GAP_H,
    TOUCH_STM_CX2_HF_GAP_V,
    TOUCH_STM_PURE_AUTOTUNE_FLAG_CHECK,

    TOUCH_STM_SELF_JITTER_TX,  // 180307
    TOUCH_STM_SELF_JITTER_RX, // 180307
    TOUCH_STM_LP_RAW, // 180307
	TOUCH_STM_GOLDEN_VALUE, //180310
};

extern	unsigned short  l_product_id;
extern  unsigned short  l_fw_ver;
extern  unsigned short  l_config_ver;
extern  unsigned short  l_release_ver;
extern  unsigned int    l_pat_cm_reference_raw[2];
extern  unsigned int    l_pat_self_raw_tx[2];
extern  unsigned int    l_pat_self_raw_rx[2];

extern  int    l_cm_reference_raw[2];
extern  int    l_cm_reference_gap;

extern  int             l_cm_jitter[2];
extern  unsigned int    l_total_cx[2];

extern  int    l_self_raw_tx[2];
extern  int    l_self_raw_rx[2];
extern  int    l_lp_self_raw_tx[2];
extern  int    l_lp_self_raw_rx[2];


extern  unsigned int    l_self_ix_tx[2];
extern  unsigned int    l_self_ix_rx[2];
extern  unsigned int    l_cx2_hf[2];
extern  unsigned char   l_otp_param;
extern  unsigned char   l_otp_param1;
extern  unsigned char   l_otp_param2;
extern  unsigned char   l_otp_param3;
extern  int             l_hf_test_mode;


extern	int    l_lp_raw[2]; /// 180307
extern  int    l_hf_gap_rx[2]; /// 180307
extern  int    l_hf_gap_tx[2]; /// 180307
extern  int     l_ss_jitter[2]; /// 180307

extern  int totalCx_Gap_Rx_MAX[300][300];
extern  int totalCx_Gap_Rx_MIN[300][300];
extern  int totalCx_Gap_Tx_MAX[300][300];
extern  int totalCx_Gap_Tx_MIN[300][300];

extern  int hf_TotalCx_Gap_Rx_MAX[300][300];
extern  int hf_TotalCx_Gap_Rx_MIN[300][300];
extern  int hf_TotalCx_Gap_Tx_MAX[300][300];
extern  int hf_TotalCx_Gap_Tx_MIN[300][300];


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
#include <sys/time.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <linux/types.h>


/////////////////////
int limit_data_match_v9(int id, struct stm_touch_limit* limit);
int limit_data_match_v15(int id, struct stm_touch_limit* limit);
int limit_data_match_v18(int id, struct stm_touch_limit* limit);
int limit_data_match_v8(int id, struct stm_touch_limit* limit);

////////////////////


int stm_touch_limit_parser(MODEL id, char *m_name, struct stm_touch_limit* limit);
int stm_touch_limit_table_parser(MODEL id, char *m_name, struct stm_touch_limit* limit);
int stm_touch_func(MODEL      id, int model_index, void *t_limit);

#endif	/* __STM_TOUCH_H__ */

