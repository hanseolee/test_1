/******************** (C) COPYRIGHT 2017 STMicroelectronics ********************
* File Name          :
* Author             : AMG KOREA
* Version            : V0.08
* Date               : 2nd of February, 2018
* Description        : Reference code for FST1 and FTM5
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef	__FTS_LGD_08_H__
#define	__FTS_LGD_08_H__
/*
////temp
#define _IOCTL_TOUCH_EN_V1_8        0x112A
#define _IOCTL_TOUCH_EN_V3_3        0x112B
#define _IOCTL_1CH_TOUCH_RESET      0x1122
#define _IOCTL_2CH_TOUCH_RESET      0x1123
////

struct	stm_touch_limit{

	int		id;
	unsigned short  fw_ver;
	unsigned short  config_ver;
	int    cm_reference_raw[2];
	int    cm_reference_gap;
	int    self_raw_tx[2];
	int    self_raw_rx[2];
	int		cm_jitter[2];
	int     ss_jitter[2]; ///
	int    lp_self_raw_tx[2];
	int    lp_self_raw_rx[2];
	int    lp_raw[2]; ///
	int    hf_gap_rx[2]; ///
	int    hf_gap_tx[2]; ///
};

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

	TOUCH_STM_SELF_JITTER_TX,
	TOUCH_STM_SELF_JITTER_RX,
	TOUCH_STM_LP_RAW,

};

*/
/* Global typedef -----------------------------------------------------------*/
typedef unsigned int	uint_t;
#if	0 //180307 hyelim modify
 typedef unsigned short	uint16_t;
 typedef unsigned char	uint8_t;

 typedef unsigned long	uint32_t;
#if 0 //180214 hyelim modify
 typedef char			int8_t;
 typedef short			int16_t;
 typedef long			int32_t;
#endif
#endif

/** @addtogroup FTS_Standard_Driver
  * @{
  */

/**
  * @brief  FTS Configuration definitions
  */
#define	FTM5_FST_V8

#define FTS_I2C_ADDR_V8						0x92
//#define I2C_TIMEOUT							3000
#define I2C_TIMEOUT_ATMEL_V8							3000

#if	defined (FTM5_FST_V8)
 #define FTS_ID0_V8							0x36
 #define FTS_ID1_V8							0x39
#endif

#define FTS_FIFO_MAX_V8						32
#define FTS_EVENT_SIZE_V8						8
#define FTS_FIFO_ADDR_V8						0x86
#define FTS_FIFO_READALL_V8					0x86
#define	DOFFSET_V8								1

/**
  * @brief  Definitions for Project
  */
//#define MACHINE_OTHERS //original
//#define MACHINE_FINAL_INSPECTION
#define MACHINE_OQC_INSPECTION_V8 //180214 khl modify

#if	0	/* swchoi - undefine of FTS_SEL_DATA_MONITORING_V8 as it must not be used for OQC machine - 20180806 */
#define FTS_SEL_DATA_MONITORING_V8
#endif	/* swchoi - end */
//#define FTS_GET_SERIALNUM

#ifdef MACHINE_OTHERS_V8
 //
#endif

#if defined (MACHINE_FINAL_INSPECTION_V8) || defined (MACHINE_OQC_INSPECTION_V8)
 #define FTS_METHOD_GOLDEN_VALUE_V8
#endif

/**
  * @brief  Definitions for FTS Features
  */
//#define FTS_SUPPORT_FW_DOWNLOAD
#define FTS_SUPPORT_SELF_SENSE_V8
#define FTS_SUPPORT_ITOTEST_V8
#ifdef	FTS_SUPPORT_ITOTEST_V8
#define FTS_SUPPORT_HF_RAW_ADJ_V8
//#define HF_RAW_ADJ_THRESHOLD_V8                500
#define HF_RAW_ADJ_THRESHOLD_V8                250  //180314

#endif
//#define FTS_SUPPORT_HW_PIN_CHECK  //180214 khl modify

/**
  * @brief  Definitions for number of sensor (panel)
  */
#define FTS_TX_LENGTH_V8						15
#define FTS_RX_LENGTH_V8						30

/**
  * @brief  FTS command definitions
  */
#define CMD_SCAN_MODE_V8							0xA0
#define CMD_FEATURE_V8								0xA2
#define CMD_SYSTEM_V8								0xA4
#define CMD_FRM_BUFF_R_V8							0xA6
#define CMD_CONFIG_R_V8						0xA8
#define CMD_CONFIG_W_V8							0xA8

#define FTS_CMD_HW_REG_W_V8						0xFA
#define FTS_CMD_HW_REG_R_V8						0xFA

/**
  * @brief  Scan mode selection of Scan mode Command (0xA0)
  */
#define CMD_SCAN_ACTIVE_V8							0x00
#define	CMD_SCAN_LPMODE_V8							0x01
#define	CMD_SCAN_TUNING_WIZARD_V8					0x02
#define	CMD_SCAN_LOCKED_V8							0x03

/**
  * @brief  Scan mode Setting of Scan mode Command (0xA0 - 0x00)
  */
#define CMD_SCAN_ACTIVE_MULTI_V8					0x01
#define	CMD_SCAN_ACTIVE_KEY_V8						0x02
#define	CMD_SCAN_ACTIVE_HOVER_V8					0x04
#define	CMD_SCAN_ACTIVE_PROXIMITY_V8				0x08
#define	CMD_SCAN_ACTIVE_FORCE_V8					0x10
#define CMD_SCAN_ACTIVE_SIDETOUCH_V8				0x20
/**
 * @brief  Scan mode Setting of Scan mode Command (0xA0 - 0x03)
 */
#define CMD_SCAN_LOCKED_ACTIVE_V8					0x00
#define CMD_SCAN_LOCKED_NOTOUCH_V8					0x01
#define CMD_SCAN_LOCKED_IDLE_V8					0x02
#define CMD_SCAN_LOCKED_LP_DETECT_V8				0x10
#define CMD_SCAN_LOCKED_LP_ACTIVE_V8				0x11

/**
  * @brief  Command Type of System Command (0xA4)
  */
#define CMD_SYS_SPECIAL_V8							0x00
#define	CMD_SYS_INTB_V8							0x01
#define	CMD_SYS_FCAL_V8							0x02
#define	CMD_SYS_ATUNE_V8							0x03
#define	CMD_SYS_ITO_TEST_V8						0x04
#define	CMD_SYS_SAVE2FLASH_V8						0x05
#define	CMD_SYS_LOAD_DATA_MEM_V8					0x06
#define CMD_SYS_CX_COMMON_CORRECTION_V8			0x07
#define CMD_SYS_SPECIAL_TUNE_CMD_V8				0x08

/**
  * @brief  Operation Setting of Special Commands (0xA4 - 0x00)
  */
#define CMD_SYS_SPECIAL_SYSTEMRESET_V8				0x00
#define CMD_SYS_SPECIAL_CLEAR_FIFO_V8				0x01
#define CMD_SYS_SPECIAL_PANEL_INIT_V8				0x02
#define CMD_SYS_SPECIAL_FULLPANEL_INIT_V8			0x03

/**
  * @brief  Operation Setting of Auto Tune Commands (0xA4 - 0x02 and 0x03)
  */
#define CMD_SYS_SCANTYPE_MUTUAL_V8					0x0100
#define CMD_SYS_SCANTYPE_MUTUAL_LP_V8				0x0200
#define CMD_SYS_SCANTYPE_SELF_V8					0x0400
#define CMD_SYS_SCANTYPE_SELF_LP_V8				0x0800
#define CMD_SYS_SCANTYPE_MSKEY_V8					0x1000
#define CMD_SYS_SCANTYPE_SSKEY_V8					0x2000
#define CMD_SYS_SCANTYPE_MS_FORCE_V8				0x4000
#define CMD_SYS_SCANTYPE_SS_FORCE_V8				0x8000

/**
  * @brief  Operation Setting of Save to Flash Commands (0xA4 - 0x05)
  */
#define CMD_SYS_SAVE2FLASH_FWCONFIG_V8				0x01
#define CMD_SYS_SAVE2FLASH_CX_V8					0x02
#define CMD_SYS_SAVE2FLASH_PANELCONFIG_V8			0x04

/**
  * @brief  Operation Setting of Host Data Memory (0xA4 - 0x06)
  * 		Host Data Memory ID
  */
#define LOAD_SYS_INFO_V8							0x01								///< Load System Info
#define LOAD_CX_MS_TOUCH_V8						0x10								///< Load MS Init Data for Active Mode
#define LOAD_CX_MS_LOW_POWER_V8					0x11								///< Load MS Init Data for Low Power Mode
#define LOAD_CX_SS_TOUCH_V8					0x12								///< Load SS Init Data for Active Mode
#define LOAD_CX_SS_TOUCH_IDLE_V8					0x13								///< Load SS Init Data for Low Power Mode
#define LOAD_CX_MS_KEY_V8							0x14								///< Load MS Init Data for Key
#define LOAD_CX_SS_KEY_V8							0x15								///< Load SS Init Data for Key
#define LOAD_CX_MS_FORCE_V8						0x16								///< Load MS Init Data for Force
#define LOAD_CX_SS_FORCE_V8						0x17								///< Load SS Init Data for Force
#define LOAD_SYNC_FRAME_RAW_V8						0x30								///< Load a Synchronized Raw Frame
#define LOAD_SYNC_FRAME_FILTER_V8					0x31								///< Load a Synchronized Filter Frame
#define LOAD_SYNC_FRAME_STRENGTH_V8				0x33								///< Load a Synchronized Strength Frame
#define LOAD_SYNC_FRAME_BASELINE_V8				0x32								///< Load a Synchronized Baseline Frame
#define LOAD_PANEL_CX_TOT_MS_TOUCH_V8				0x50								///< Load TOT MS Init Data for Active Mode
#define LOAD_PANEL_CX_TOT_MS_LOW_POWER_V8			0x51								///< Load TOT MS Init Data for Low Power Mode
#define LOAD_PANEL_CX_TOT_SS_TOUCH_V8				0x52								///< Load TOT SS Init Data for Active Mode
#define LOAD_PANEL_CX_TOT_SS_TOUCH_IDLE_V8			0x53								///< Load TOT SS Init Data for Low Power Mode
#define LOAD_PANEL_CX_TOT_MS_KEY_V8				0x54								///< Load TOT MS Init Data for Key
#define LOAD_PANEL_CX_TOT_SS_KEY_V8				0x55								///< Load TOT SS Init Data for Key
#define LOAD_PANEL_CX_TOT_MS_FORCE_V8				0x56								///< Load TOT MS Init Data for Force
#define LOAD_PANEL_CX_TOT_SS_FORCE_V8				0x57								///< Load TOT SS Init Data for Force

/**
  * @brief  Operation Setting of Save to Flash Commands (0xA4 - 0x08)
  */
#define CMD_SYS_SPECIAL_TUNE_CMD_LPTIMER_CAL_V8		0x01
#define CMD_SYS_SPECIAL_TUNE_CMD_IOFFSET_TUNE_V8	0x02
#define CMD_SYS_SPECIAL_TUNE_CMD_IOFFSET_DATA_V8	0x04

/**
  * @brief  FTS Event ID definitions for Event Messages
  */
#define EVTID_NO_EVENT_V8							0x00
#define EVTID_CONTROLLER_READY_V8					0x03
#define EVTID_ENTER_POINTER_V8						0x13
#define EVTID_MOTION_POINTER_V8					0x23
#define EVTID_LEAVE_POINTER_V8						0x33
#define EVTID_STATUS_REPORT_V8						0x43
#define EVTID_USER_REPORT_V8						0x53
#define EVTID_ERROR_REPORT_V8						0xF3

/**
  * @brief  Report Type of Status Report Event (0xA4)
  */
#define EVTID_RPTTYPE_CMD_ECHO_V8					0x01
#define	EVTID_RPTTYPE_FRAME_DROP_V8				0x03
#define	EVTID_RPTTYPE_FCAL_TRIG_V8					0x05


#define	EVTID_ERR_TYPE_SYSTEM1_V8					0x00 //add by healim - 180514

/**
  * @brief  Report Type of Status Report Event (0xF3)
  */
#define EVTID_ERR_TYPE_SYSTEM2_V8					0x10
#define	EVTID_ERR_TYPE_CRC_V8						0x20
#define	EVTID_ERR_TYPE_FW_V8						0x40
#define	EVTID_ERR_TYPE_MS_TUNE_V8					0x70
#define	EVTID_ERR_TYPE_SS_TUNE_V8					0x80
#define	EVTID_ERR_TYPE_CX_V8						0xA0

/**
  * @brief  FTS Address
  */
#define FTS_ADDR_CHIP_ID_V8						(uint32_t) 0x20000000
#define FTS_ADDR_SYSTEM_RESET_V8					(uint32_t) 0x20000024

#define ADDR_FRAMEBUFFER_V8						(uint16_t) 0x0000						///< frame buffer address in memory
#define ADDR_ERROR_DUMP_V8							(uint16_t) 0xEF80						///< start address dump error log


#define SYSTEM_RESET_SOFT_V8						0
#define	SYSTEM_RESET_HARD_V8						1
#define SYSTEM_RESET_VALUE_V8						0x80
#define SYSTEM_HOLD_M3_VALUE_V8					0x01

/**
  * @brief  FTS Flash definitions
  */
#define FW_MAIN_BLOCK_V8							0x01
#define FW_INFO_BLOCK_V8							0x02

#define WRITE_CHUNK_SIZE_V8						32
#define FLASH_CHUNK_V8							(32 * 1024)
#define DMA_CHUNK_V8								32

#define FW_HEADER_SIZE_V8							64
#define FW_HEADER_FTB_SIGNATURE_V8					0xAA55AA55
#define FW_HEADER_TARGET_ID_V8						0x00003936
#define FW_BYTES_ALLIGN_V8							4

#define FLASH_ADDR_CODE_V8							0x00000000				// starting address in the flash of the code in FTM4
#define FLASH_ADDR_CONFIG_V8						0x00007C00				// starting address in the flash of the config in FTI
#define FLASH_ADDR_CX_V8							0x00007000				///< starting address (words) in the flash of the Init data in FTI

#define FLASH_UNLOCK_CODE0_V8						0x25
#define FLASH_UNLOCK_CODE1_V8						0x20

#define FLASH_ERASE_START_V8						0x80
#define FLASH_ERASE_CODE1_V8                       0xC0
#define FLASH_DMA_CODE1_V8                         0xC0
#define FLASH_ERASE_UNLOCK_CODE0_V8				0xDE
#define FLASH_ERASE_UNLOCK_CODE1_V8				0x03
#define FLASH_ERASE_CODE0_V8                       0x6A
#define FLASH_DMA_CODE0_V8                      	0x71
#define FLASH_DMA_CONFIG_V8                        0x72
#define FLASH_NUM_PAGE_V8							32						// number of pages in main flash
#define FLASH_CX_PAGE_START_V8						26						// starting page which contain Panel Init data
#define FLASH_CX_PAGE_END_V8						30

/**
  * @brief  Enumeration - Error type
  */
enum	fts_error_type
{
	FTS_ERR_V8							= 0x80000000,
	FTS_NO_ERR_V8						= 0x00000000,
	FTS_ERR_EVT_TIMEOVER_V8			= 0x00000001,
	FTS_ERR_SYS_RST_V8					= 0x00000010,
	FTS_ERR_CHIPID_V8					= 0x00000020,
	FTS_ERR_AUTOTUNE_V8				= 0x00000040,
	FTS_ERR_SAVE_COMP_V8				= 0x00000080,
	FTS_ERR_HOSTDATA_ID_HD_V8			= 0x00000100,
	FTS_ERR_ITO_TEST_V8				= 0x00000200,
	FTS_ERR_SYSTEM_V8					= 0x00001000, //add - hyelim 180514
	FTS_ERR_HF_RAW_TEST_V8				= 0x00000400, //add - hyelim 180306
	FTS_ERR_CRC_V8						= 0x00010000,
	FTS_ERR_CRC_CFG_V8					= 0x00020000,
	FTS_ERR_CRC_CX_V8					= 0x00040000,
	FTS_ERR_CRC_CORRUPT_V8				= 0x00080000, // add - hyelim 180514
	FTS_ERR_BIN_TYPE_V8				= 0x00100000,
	FTS_ERR_FLASH_BURN_V8				= 0x00200000,
	FTS_ERR_FLASH_ERASE_V8				= 0x00400000,
	FTS_ERR_I2C_V8						= 0x10000000,
	FTS_ERR_MEM_ALLC_V8				= 0x20000000,
};

/**
  * @brief  Enumeration - Type of Binary File
  */
enum binfile_type
{
	BIN_FTS128_V8 = 1,
	BIN_FTS256_V8 = 2,
	BIN_FTB_V8 = 3,
};

/**
  * @brief  Type definitions - header structure of firmware file (ftb)
  */
//typedef __packed struct {
typedef struct {
	uint32_t	signature;
	uint32_t	ftb_ver;
	uint32_t	target;
	uint32_t	fw_id;
	uint32_t	fw_ver;
	uint32_t	cfg_id;
	uint32_t	cfg_ver;
	uint32_t	reserved1[2];
	uint32_t	ext_ver;
	uint32_t	reserved2[1];
	uint32_t	sec0_size;
	uint32_t	sec1_size;
	uint32_t	sec2_size;
	uint32_t	sec3_size;
	uint32_t	hdr_crc;
}	FW_FTB_HEADER;

/** @defgroup system_info	System Info
  * System Info Data collect the most important informations about hw and fw
  * @{
  */
#define SYS_INFO_SIZE_V8							208									///< Size in bytes of System Info data
#define DIE_INFO_SIZE_V8							16									///< num bytes of die info
#define EXTERNAL_RELEASE_INFO_SIZE_V8				8									///< num bytes of external release in config
#define RELEASE_INFO_SIZE_V8						(EXTERNAL_RELEASE_INFO_SIZE_V8)		///< num bytes of release info in sys info (first bytes are external release)

#define COMP_DATA_HEADER_SIZE_V8					16
#define COMP_DATA_HEADER_V8						4									///< size in bytes of initialization data header
#define COMP_DATA_GLOBAL_V8						(16 - COMP_DATA_HEADER)				///< size in bytes of initialization data general info

#define HEADER_SIGNATURE_V8						0xA5								///< signature used as starting byte of data loaded in memory

//typedef	__packed struct	{
typedef struct	{
	uint8_t		header;
	uint8_t		host_data_mem_id;
	uint16_t	cnt;
	uint8_t		force_leng;
	uint8_t		sense_leng;
	uint8_t		cx1;
	uint8_t		reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
}	MsCompHeader;

//typedef	__packed struct	{
typedef struct	{
	uint8_t		header;
	uint8_t		host_data_mem_id;
	uint16_t	cnt;
	uint8_t		force_leng;
	uint8_t		sense_leng;
	uint8_t		force_ix1;
	uint8_t		sense_ix1;
	uint8_t		force_cx1;
	uint8_t		sense_cx1;
	uint8_t		force_maxn;
	uint8_t		sense_maxn;
	uint32_t	reserved;
}	SsCompHeader;

//typedef	__packed struct	{
typedef struct	{
	uint8_t		header;
	uint8_t		host_data_mem_id;
	uint16_t	cnt;
	uint8_t		dbg_frm_leng;
	uint8_t		ms_force_leng;
	uint8_t		ms_sense_leng;
	uint8_t		ss_force_leng;
	uint8_t		ss_sense_leng;
	uint8_t		key_leng;
	uint16_t	reserved1;
	uint32_t	reserved2;
}	SyncFrameHeader;

/**
 * @brief  Struct which contains fundamental informations about the chip and its configuration
 */
//typedef __packed struct {
typedef struct {
	uint8_t		header;
	uint8_t		host_data_mem_id;
	uint16_t	cnt;
	uint16_t	apiVer_rev;									// API revision version
	uint8_t		apiVer_minor;								// API minor version
	uint8_t		apiVer_major;								// API major version
	uint16_t	chip0Ver;									// Dev0 version
	uint16_t	chip0Id;									// Dev0 ID
	uint16_t	chip1Ver;									// Dev1 version
	uint16_t	chip1Id;									// Dev1 ID

	uint16_t	fwVer;										// Fw version
	uint16_t	svn_rev;									// Revision of svn
	uint16_t	cfgVer;										// Config Version
	uint16_t	cfgProjctId;								// Project ID
	uint16_t	cxVer;										// Cx Version
	uint16_t	cxId;										// Cx ID
	uint8_t		CfgAfeVer;									// AFE version in Config
	uint8_t		CxAfeVer;									// AFE version of Cx
	uint8_t		PanelCfgAfeVer;
	uint8_t		protocol;

	uint8_t		dieInfo[DIE_INFO_SIZE_V8];						// Die information
	uint8_t		releaseInfo[RELEASE_INFO_SIZE_V8];				// Release information
	uint32_t	fw_crc;										// crc of fw
	uint32_t	cfg_crc;									// crc of cfg

	uint32_t	reserved2[4];

	uint16_t	scrResX;									// X resolution on main screen
	uint16_t	scrResY;									// Y resolution on main screen
	uint8_t		scrTxLen;									// Tx length
	uint8_t		scrRxLen;									// Rx length
	uint8_t		keyLen;										// Key Len
	uint8_t		forceLen;									// Force Len
	uint32_t	reserved3[10];

	uint16_t	dbgFrameAddr;								// Offset of debug Info structure
	uint16_t	reserved4[3];

	uint16_t	msTchRawAddr;								// Offset of MS touch raw frame
    uint16_t	msTchFilterAddr;							// Offset of MS touch filter frame
    uint16_t	msTchStrenAddr;								// Offset of MS touch strength frame
    uint16_t	msTchBaselineAddr;							// Offset of MS touch baseline frame

    uint16_t	ssTchTxRawAddr;								// Offset of SS touch force raw frame
    uint16_t	ssTchTxFilterAddr;							// Offset of SS touch force filter frame
    uint16_t	ssTchTxStrenAddr;							// Offset of SS touch force strength frame
    uint16_t	ssTchTxBaselineAddr;						// Offset of SS touch force baseline frame

    uint16_t	ssTchRxRawAddr;								// Offset of SS touch sense raw frame
    uint16_t	ssTchRxFilterAddr;							// Offset of SS touch sense filter frame
    uint16_t	ssTchRxStrenAddr;							// Offset of SS touch sense strength frame
    uint16_t	ssTchRxBaselineAddr;						// Offset of SS touch sense baseline frame

    uint16_t	keyRawAddr;									// Offset of key raw frame
    uint16_t	keyFilterAddr;								// Offset of key filter frame
    uint16_t	keyStrenAddr;								// Offset of key strength frame
    uint16_t	keyBaselineAddr;							// Offset of key baseline frame

    uint16_t	frcRawAddr;									// Offset of force touch raw frame
    uint16_t	frcFilterAddr;								// Offset of force touch filter frame
    uint16_t	frcStrenAddr;								// Offset of force touch strength frame
    uint16_t	frcBaselineAddr;							// Offset of force touch baseline frame

    uint16_t	ssHvrTxRawAddr;								// Offset of SS hover Force raw frame
    uint16_t	ssHvrTxFilterAddr;							// Offset of SS hover Force filter frame
    uint16_t	ssHvrTxStrenAddr;							// Offset of SS hover Force strength frame
    uint16_t	ssHvrTxBaselineAddr;						// Offset of SS hover Force baseline frame

    uint16_t	ssHvrRxRawAddr;								// Offset of SS hover Sense raw frame
    uint16_t	ssHvrRxFilterAddr;							// Offset of SS hover Sense filter frame
    uint16_t	ssHvrRxStrenAddr;							// Offset of SS hover Sense strength frame
    uint16_t	ssHvrRxBaselineAddr;						// Offset of SS hover Sense baseline frame

    uint16_t	ssPrxTxRawAddr;								// Offset of SS proximity force raw frame
    uint16_t	ssPrxTxFilterAddr;							// Offset of SS proximity force filter frame
    uint16_t	ssPrxTxStrenAddr;							// Offset of SS proximity force strength frame
    uint16_t	ssPrxTxBaselineAddr;						// Offset of SS proximity force baseline frame

    uint16_t	ssPrxRxRawAddr;								// Offset of SS proximity sense raw frame
    uint16_t	ssPrxRxFilterAddr;							// Offset of SS proximity sense filter frame
    uint16_t	ssPrxRxStrenAddr;							// Offset of SS proximity sense strength frame
    uint16_t	ssPrxRxBaselineAddr;						// Offset of SS proximity sense baseline frame
}	SysInfo;

/** @}*/

/**
* Possible types of MS frames
*/
typedef enum	{
	MS_RAW_V8 = 0,																	///< Mutual Sense Raw Frame
	MS_FILTER_V8 = 1,																///< Mutual Sense Filtered Frame
	MS_STRENGHT_V8 = 2,															///< Mutual Sense Strength Frame (Baseline-Raw)
	MS_BASELINE_V8 = 3,															///< Mutual Sense Baseline Frame
	MS_KEY_RAW_V8 = 4,																///< Mutual Sense Key Raw Frame
	MS_KEY_FILTER_V8 = 5,															///< Mutual Sense Key Filter Frame
	MS_KEY_STRENGHT_V8 = 6,														///< Mutual Sense Key Strength Frame (Baseline-Raw)
	MS_KEY_BASELINE_V8 = 7,														///< Mutual Sense Key Baseline Frame
	FRC_RAW_V8 = 8,																///< Force Raw Frame
	FRC_FILTER_V8 = 9,																///< Force Filtered Frame
	FRC_STRENGHT_V8 = 10,															///< Force Strength Frame (Baseline-Raw)
	FRC_BASELINE_V8 = 11															///< Force Baseline Frame
}	MSFrameType;


/**
* Possible types of SS frames
*/
typedef enum	{
	SS_RAW_V8 = 0,																	///< Self Sense Raw Frame
	SS_FILTER_V8 = 1,																///< Self Sense Filtered Frame
	SS_STRENGHT_V8 = 2,															///< Self Sense Strength Frame (Baseline-Raw)
	SS_BASELINE_V8 = 3,															///< Self Sense Baseline Frame
	SS_HVR_RAW_V8 = 4,																///< Self Sense Hover Raw Frame
	SS_HVR_FILTER_V8 = 5,															///< Self Sense Hover Filter Frame
	SS_HVR_STRENGHT_V8 = 6,														///< Self Sense Hover Strength Frame (Baseline-Raw)
	SS_HVR_BASELINE_V8 = 7,														///< Self Sense Hover Baseline Frame
	SS_PRX_RAW_V8 = 8,																///< Self Sense Proximity Raw Frame
	SS_PRX_FILTER_V8 = 9,															///< Self Sense Proximity Filtered Frame
	SS_PRX_STRENGHT_V8 = 10,														///< Self Sense Proximity Strength Frame (Baseline-Raw)
	SS_PRX_BASELINE_V8 = 11														///< Self Sense Proximity Baseline Frame
}	SSFrameType;

/**
  * @brief  Enumeration - Address size of Register
  */
enum	Reg_Address_Size	{
 	NO_ADDR = 0,
 	BITS_8 = 1,
 	BITS_16 = 2,
 	BITS_24 = 3,
 	BITS_32 = 4,
 	BITS_40 = 5,
 	BITS_48 = 6,
 	BITS_56 = 7,
 	BITS_64 = 8
};

/**
  * @brief  Enumeration - Error type of ITO TEST
  */
enum	ito_error_type
{
	ITO_FORCE_SHRT_GND_V8		= 0x60,
	ITO_SENSE_SHRT_GND_V8		= 0x61,
	ITO_FORCE_SHRT_VDD_V8		= 0x62,
	ITO_SENSE_SHRT_VDD_V8		= 0x63,
	ITO_FORCE_SHRT_FORCE_V8	= 0x64,
	ITO_SENSE_SHRT_SENSE_V8	= 0x65,
	ITO_FORCE_OPEN_V8 			= 0x66,
	ITO_SENSE_OPEN_V8 			= 0x67,
	ITO_KEY_OPEN_V8			= 0x68
};

/**
  * @brief  FTS general definitions
  */

//#define ON					0x01
//#define OFF					0x00

#define TRUE				1
#define FALSE				0

#define	ENABLE				1
#define	DISABLE				0

/** @defgroup FTS_Exported_FunctionsPrototype
  * @{
  */
int fts_event_handler_v8(void);
int flashProcedure_v8(int force, int keep_cx);
int fts_init_v8(void);
///////////////////////////////////////////

void i2c_dev_match_v8(int i2c_dev);
//void i2c_dev_match_v8(int i2c_dev, int io_dev);
int fts_panel_test_v8(int id, unsigned char *uart_buf);


/**
  * @}
  */

/**
  * @}
  */

#endif	/* __FTS_LGD_H__ */

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
