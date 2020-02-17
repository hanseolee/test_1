/******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
* File Name          :
* Author             : AMG KOREA
* Version            : V0.16
* Date               : 27th of September, 2017
* Description        : Reference code for panel test of FTM
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef	__FTS_LGD_18_H__
#define	__FTS_LGD_18_H__

/** @addtogroup FTS_Standard_Driver
  * @{
  */

/** @defgroup FTS_Definitions
  * @{
  */
//#define	__DBG_FTS_JIG__

/**
  * @brief  FTS Configuration definitions
  */
//#define FTSD2

#define LGD_ID									"CURV431000"  //add khl 180119


#define	FTSD3_V18

#define FTS_I2C_ADDR_V18						0x92

#if	defined (FTSD3_V18)
 #define FTS_FIFO_MAX_V18						64
 #define FTS_ID0_V18							0x36
 #define FTS_ID1_V18							0x70
#else
 #define FTS_FIFO_MAX_V18						32
 #define FTS_ID0_V18							0x39
 #define FTS_ID1_V18							0x6C
#endif

#define FTS_EVENT_SIZE_V18						8
#define FTS_FIFO_ADDR_V18						0x85
#if defined (FTSD1_V18)
 #define DOFFSET_V18                 			0
#else
 #define	DOFFSET_V18							1
#endif

/**
  * @brief  Definitions for Project
  */
//#define	PRJ_LGMC
#define	PRJ_DP049_V18

//#define JIG_FINAL_INSPECTION_MACHINE_V18
#define JIG_OQC_MACHINE_V18
//#define JIG_SMT_INSPECTION_MACHINE_V18

#define LIMIT_MUTUAL_TOTAL_CX_V18
#ifdef LIMIT_MUTUAL_TOTAL_CX_V18
 #define MUTUAL_CX1_COEFF_V18					8
#endif

/**
  * @brief  Definitions for FTS Features
  */
//#define FTS_SUPPORT_FW_DOWNLOAD
#define FTS_SUPPORT_SELF_SENSE_V18
//#define	FTS_SUPPORT_MS_KEY
//#define	FTS_SUPPORT_FORCE_FSR
//#define FTS_USE_SELF_JITTER
#define	FTS_LOCKDOWNCODE_FEATURE_V18
#define	FTS_SUPPORT_CHK_HIGH_FREQ_V18
//#define	FTS_SUPPORT_CHK_LPMODE
#define FTS_SUPPORT_ITOTEST_HW_V18
//#define FTS_SUPPORT_ITOTEST_SW

/**
  * IMPORTANT : FTS_PURE_AUTOTUNE_FEATURE
  *   PLEASE CONTACT LGD OR STM IF YOU WANT TO ENABLE THIS FEATURE.
  *   THIS FEATURE MUST TO BE ENABLED ONLY AT FINAL INSPECTION MACHINES.
  */
//#define	FTS_PURE_AUTOTUNE_FEATURE
#ifndef	FTS_PURE_AUTOTUNE_FEATURE_V18
 #define	FTS_SUPPORT_CX_BACKUP_V18
#endif

#ifdef	FTS_USE_SELF_JITTER_V18
 #define	FTS_SELF_JITTER_COUNT_V18			10
#endif

//#define	FTS_HW_PIN_CHECK

#define FTS_TX_LENGTH_V18						16
#define FTS_RX_LENGTH_V18						32

#ifdef	FTS_SUPPORT_MS_KEY_V18
#define	FTS_MSKEY_TX_LENGTH_V18					1
#define	FTS_MSKEY_RX_LENGTH_V18					2
#endif

#ifdef	FTS_SUPPORT_FORCE_FSR_V18
#define FTS_FORCE_TX_LENGTH_v18				1
#define FTS_FORCE_RX_LENGTH_v18				4
#endif

/**
 * @brief  FTS lockdown code definitions
 */
#ifdef	FTS_LOCKDOWNCODE_FEATURE_V18
#ifdef PRJ_LGMC_V18
 #define FTS_LOCKDOWNCODE_SIZE_V18				13
#else
#ifdef PRJ_DP049_V18
 #define FTS_LOCKDOWN_SENDBYTE_SIZE_V18			8
 #define FTS_LOCKDOWN_PROJECTID_SIZE_V18		10
 #define FTS_LOCKDOWN_2DBARCODE_SIZE_V18		39
 #define LOCKDOWN_TYPEID_PROJECTID_V18			0x00
 #define LOCKDOWN_TYPEID_2DBARCODE_V18			0x01
 #define LOCKDOWN_LENGTH_2DBARCODE_V18			0x03
#else
 #define FTS_LOCKDOWNCODE_SIZE_V18				8
#endif
#endif
#endif

/** @defgroup FTS_Debug_Definitions
  * @{
  */
#define	__DBG_FTS_JIG_V18__
#ifdef	__DBG_FTS_JIG_V18__
 //#define	__DBG_FTS_MS_RAW__
 //#define	__DBG_FTS_MS_CX2__
 #define	__DBG_FTS_MS_JIT_V18__
 //#define	__DBG_FTS_SS_RAW__
 //#define	__DBG_FTS_SS_IX__
 //#define	__DBG_FTS_FORCE__
 //#ifdef	FTS_SUPPORT_MS_KEY
 //#define	__DBG_FTS_MS_KEY__
 //#endif
#endif

/**
  * @brief  FTS Event ID definitions
  */
#define EVENTID_NO_EVENT_V18					0x00
#define EVENTID_ENTER_POINTER_V18				0x03
#define EVENTID_LEAVE_POINTER_V18				0x04
#define EVENTID_MOTION_POINTER_V18				0x05
#define EVENTID_ERROR_V18						0x0F
#define EVENTID_CONTROLLER_READY_V18			0x10
#define EVENTID_SLEEPOUT_CONTROLLER_READY_V18	0x11
#define EVENTID_RESULT_READ_REGISTER_V18		0x12
#define	EVENTID_COMPENSATION_READ_V18			0x13
#define	EVENTID_SYSTEM_STATUS_UPDATE_V18		0x16
#define	EVENTID_LOCKDOWN_CODE_V18				0x1E

/**
  * @brief  FTS Status ID definitions of Event ID (0x16)
  */
#define	EVENTID_STATID_AT_MS_DONE_V18				0x01
#define EVENTID_STATID_AT_SS_DONE_V18				0x02
#define	EVENTID_STATID_CX_BACKUP_DONE_V18			0x04
#define EVENTID_STATID_FCAL_DONE_V18				0x06
#define	EVENTID_STATID_TA_ON_V18					0xCC
#define	EVENTID_STATID_TA_OFF_V18					0xCD

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V18
 #define EVENTID_STATID_LOCKDOWN_WRITE_DONE_V18		0x08
 #define EVENTID_STATID_LOCKDOWN_CUR_WRITE_DONE_V18	0x13
 #define EVENTID_STATID_LOCKDOWN_WRITECOUNT_V18		0x14
 #define EVENTID_ERRTYPE_LOCKDOWN_V18				0x0B
#endif

#define	EVENTID_STATID_PAT_SET_V18					0x10
#define	EVENTID_STATID_PAT_CLEAR_V18				0x11

/**
  * @brief  FTS Error Type definitions of Event ID (0x0F)
  */
#define	EVENTID_STATID_ERROR_CRC_V18				0x03

/**
  * @brief  FTS COMMAND RELATED FEATURES
  */
#ifdef PRJ_LGMC_V18
 #define	FTS_CMD_FEATURE_ACTIVEMODE_V18						(0x0100)
 #define	FTS_CMD_FEATURE_TA_V18								(0x0200)
 #define	FTS_CMD_FEATURE_WIRELESS_V18						(0x0400)
 #define	FTS_CMD_FEATURE_HANDSHAKE_V18						(0x0800)
#else
 #define	FTS_CMD_FEATURE_ACTIVEMODE_V18						(0x01)
 #define	FTS_CMD_FEATURE_TA_V18								(0x02)
 #define	FTS_CMD_FEATURE_WIRELESS_V18						(0x04)
 #define	FTS_CMD_FEATURE_HANDSHAKE_V18						(0x08)
#endif

/**
  * @brief  FTS Event IDe definitions
  */
#if defined (FTSD3_V18)
 #define EVENTID_INTERNAL_RELEASE_INFO_V18			0x14
 #define EVENTID_EXTERNAL_RELEASE_INFO_V18			0x15
 #define EVENTID_SERIALNUMBER_HIGH_V18				0x1B
 #define EVENTID_SERIALNUMBER_LOW_V18				0x1C
#else
 #define EVENTID_INTERNAL_RELEASE_INFO_V18       0x19
 #define EVENTID_EXTERNAL_RELEASE_INFO_V18       0x1A
#endif

/**
  * @brief  FTS command definitions
  */
#define READ_STATUS_V18							0x84
#define READ_ONE_EVENT_V18						0x85
#define READ_ALL_EVENT_V18						0x86
#define	SLEEPOUT_V18									0x91
#define SENSEOFF_V18									0x92
#define SENSEON_V18										0x93
#define SELF_SENSEOFF_V18							0x94
#define SELF_SENSEON_V18							0x95
#define LPOSC_TRIM_V18								0x97
#define MSKEY_SENSEOFF_V18						0x9A
#define MSKEY_SENSEON_V18						0x9B
#define FLASH_BACKUP_V18						0xFB
#define FLASH_CX_BACKUP_V18						0xFC

#define FLUSHBUFFER_V18							0xA1
#define FORCECALIBRATION_V18					0xA2
#define MUTUAL_AUTO_TUNE_V18					0xA3
#define SELF_AUTO_TUNE_V18						0xA4
#ifdef PRJ_LGMC_V18
#define HF_MUTUAL_AT_V18						0xA5
#endif
#define MSKEY_AUTO_TUNE_V18						0x96
#define ITO_CHECK_V18							0xA7
#define FTS_CMD_RELEASEINFO_V18			   		0xAA
#define LOWPOWER_MODE_V18						0xAD

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V18
 #define LOCKDOWN_WRITE_V18						0xC5
 #define LOCKDOWN_REWRITE_V18					0xC6
 #define LOCKDOWN_READ_V18						0xC4
#endif

/**
  * @brief  FTS Flash definitions
  */
#define FW_MAIN_BLOCK_V18						0x01
#define FW_INFO_BLOCK_V18						0x02

#if	defined(FTSD3_V18)
 #define WRITE_CHUNK_SIZE_V18					32
 #define FLASH_CHUNK_V18						(64 * 1024)
 #define DMA_CHUNK_V18							32

 #define FW_HEADER_SIZE_V18						64
 #define FW_HEADER_FTB_SIGNATURE_V18			0xAA55AA55
 #define FW_FTB_VER_V18							0x00000001
 #define FW_BYTES_ALLIGN_V18					4
 #define FW_BIN_VER_OFFSET_V18					16
 #define FW_BIN_CONFIG_VER_OFFSET_V18			20

 /* Command for flash */
 #define FLASH_CMD_UNLOCK_V18					0xF7
 #define FLASH_CMD_WRITE_64K_V18				0xF8
 #define FLASH_CMD_READ_REGISTER_V18			0xF9
 #define FLASH_CMD_WRITE_REGISTER_V18			0xFA

 /* Parameters for commands */
 #define ADDR_WARM_BOOT_V18						0x001E
 #define WARM_BOOT_VALUE_V18					0x38
 #define FLASH_ADDR_CODE_V18					0x00000000
 #define FLASH_ADDR_CONFIG_V18					0x0000FC00

 #define FLASH_UNLOCK_CODE0_V18					0x74
 #define FLASH_UNLOCK_CODE1_V18					0x45

 #define FLASH_ERASE_UNLOCK_CODE0_V18			0x72
 #define FLASH_ERASE_UNLOCK_CODE1_V18			0x03
 #define FLASH_ERASE_UNLOCK_CODE2_V18			0x02
 #define FLASH_ERASE_CODE0_V18					0x02
 #define FLASH_ERASE_CODE1_V18					0xC0
 #define FLASH_DMA_CODE0_V18					0x05
 #define FLASH_DMA_CODE1_V18					0xC0
 #define FLASH_DMA_CONFIG_V18					0x06

#else
 #define FTS_CMD_WRITE_PRAM_V18					0xF0
 #define FTS_CMD_BURN_PROG_FLASH_V18			0xF2
 #define FTS_CMD_ERASE_PROG_FLASH_V18			0xF3
 #define FTS_CMD_READ_FLASH_STAT_V18			0xF4

 #define WRITE_CHUNK_SIZE_V18					(2 * 1024) - 16
 #define WRITE_BURN_SIZE_V18					64
 //#define WRITE_CMD_SIZE					32
 //#define WRITE_CMD_LENGTH					165

 #define FTS_FIRMWARE_SIZE_V18				(122 * 1024)
 #define FTS_CONFIG_SIZE_V18					(2 * 1024)
 #define FTS_BIN_SIZE_V18						(128 * 1024)
#endif

/**
  * @brief  Offset of compensation data
  */
#define	FTS_COMP_NODEDATA_OFFSET_V18			0x10
#define	FTS_COMP_MS_CX_SEL_V18					(uint16_t) 0x0002
#define	FTS_COMP_MS_LP_SEL_V18					(uint16_t) 0x0004
#define	FTS_COMP_MS_KEY_SEL_V18					(uint16_t) 0x0010
#define	FTS_COMP_SS_IX_SEL_V18					(uint16_t) 0x0020
#define	FTS_COMP_MS_FORCE_SEL_V18				(uint16_t) 0x0200

#ifdef	PRJ_LGMC_V18
 #define	FTS_COMP_SS_TX_IX1_RATIO_V18		4
#else
 #define	FTS_COMP_SS_TX_IX1_RATIO_V18		2
#endif
#define	FTS_COMP_SS_RX_IX1_RATIO_V18		2

/*
  * @brief  SYSINFO - Offset address for getting data
  */
#define FTS_MS_RAW_ADDR_V18       				0x02
#define FTS_MS_JIT_ADDR_V18						0x04
#define FTS_SS_F_RAW_ADDR_V18					0x1E
#define FTS_SS_S_RAW_ADDR_V18					0x20
#define	FTS_SS_F_JITTER_ADDR_V18				0x22
#define	FTS_SS_S_JITTER_ADDD_V18			0x24
#define FTS_MS_KEY_RAW_ADDR_V18			0x34
#define	FTS_MS_KEY_STR_ADDR_V18					0x36
#define FTS_READ_PAT_FLAG_V18					0x4E
#define FTS_GET_AFE_STATUS_V18					0x52

/**
 * @brief  Type of data for FSR sensor
 */
#ifdef	FTS_SUPPORT_FORCE_FSR_V18

 #define FTS_FORCE_RAW_ADDR_V18       			(uint16_t) 0x0052
 #define FTS_FORCE_STR_ADDR_V18       			(uint16_t) 0x0056
#endif

/**
  * @brief  FTS general definitions
  */
//#define ON				0x01
//#define OFF				0x00

#define TRUE			1
#define FALSE			0

#define	ENABLE			1
#define	DISABLE			0

enum	fts_error_type_v18
{
	FTS_ERR_CRC_CX_V18					= -12,
	FTS_ERR_CRC_CFG_V18					= -11,
	FTS_ERR_CRC_V18						= -10,
	FTS_ERR_TIMEOVER_SENSEON_V18		= -3,
	FTS_ERR_TIMEOVER_BACKUP_V18			= -2,
	FTS_ERR_TIMEOVER_SYSRESET_V18		= -1,
	FTS_NO_ERR_V18						= 1
};
/*
enum	pure_type
{
	FAILED = -1,
	NOT_FINAL = 1,
	DONE = 2,
	PAT_ALREADY_SET = 3
};
*/
/**
  * @}
  */
enum	protocol_type_v18
{
	VER_1p1_V18 = 1,
	VER_1p2_V18 = 2,
	VER_1p3_V18 = 3
};

enum	write_type_v18
{
	FIRMWARE_V18 = 0,
	CONFIG_V18,
	BOTH_V18,
	BINARY_V18
};

enum binfile_type_v18
{
	BIN_FTS128_V18 = 1,
	BIN_FTS256_V18 = 2,
	BIN_FTB_V18 = 3
};

enum	ito_error_type_v18
{
	NO_ERROR_V18 = 0,
	ITO_FORCE_OPEN_V18,
	ITO_SENSE_OPEN_V18,
	ITO_FORCE_SHRT_GND_V18,
	ITO_SENSE_SHRT_GND_V18,
	ITO_FORCE_SHRT_VCM_V18,
	ITO_SENSE_SHRT_VCM_V18,
	ITO_FORCE_SHRT_FORCE_V18,
	ITO_SENSE_SHRT_SENSE_V18,
	ITO_F2E_SENSE_V18,
	ITO_FPC_FORCE_OPEN_V18,
	ITO_FPC_SENSE_OPEN_V18,
	ITO_KEY_FORCE_OPEN_V18,
	ITO_KEY_SENSE_OPEN_V18,
	ITO_RESERVED0_V18,
	ITO_RESERVED1_V18,
	ITO_RESERVED2_V18,
	ITO_MAX_ERR_REACHED_V18 = 0xFF
};

/* Global typedef -----------------------------------------------------------*/
typedef unsigned int	uint_t;
//#if	1
#if	0
 typedef unsigned short	uint16_t;
 typedef unsigned char	uint8_t;

 typedef unsigned long	uint32_t;
 typedef char			int8_t;
 typedef short			int16_t;
 typedef long			int32_t;
#endif

struct fts64_header_v18 {
	uint_t		signature;
	uint16_t	fw_ver;
	uint8_t		fw_id;
	uint8_t		reserved1;
	uint8_t		internal_ver[8];
	uint8_t		released_ver[8];
	uint_t 		reserved2;
	uint_t 		checksum;
};

typedef struct {
	uint32_t	signature;
	uint32_t	ftb_ver;
	uint32_t	target;
	uint32_t	fw_id;
	uint32_t	fw_ver;
	uint32_t	cfg_id;
	uint32_t	cfg_ver;
	//uint32_t	reserved[2];
	uint32_t	reserved[3];
	uint32_t	ext_ver;
	//uint32_t	reserved2;
	uint32_t	sec0_size;
	uint32_t	sec1_size;
	uint32_t	sec2_size;
	uint32_t	sec3_size;
	uint32_t	hdr_crc;
}	FW_FTB_HEADER_V18;

/** @defgroup FTS_Exported_FunctionsPrototype
  * @{
  */
int fts_systemreset_v18(void);
int fts_event_handler_v18(void);
int flashProcedure_v18(int force, int keep_cx);
int fts_panel_test_v18(int id, unsigned char *uart_buf);
int fts_init_v18(void);


//////////////////////////////////////////PDK
void i2c_dev_match_v18(int i2c_dev);
int fts_get_channelInfo_v18(void);
#ifdef PRJ_DP049_V18
	int fts_lockdown_read_v18(int id, int type);
#else
	int fts_lockdown_read_v18(void);
#endif


/**
  * @}
  */

/**
  * @}
  */

#endif	/* __FTS_LGD_18_H__ */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/

