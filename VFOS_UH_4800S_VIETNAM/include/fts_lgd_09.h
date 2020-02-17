/******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
* File Name          :
* Author             : AMG KOREA
* Version            : V0.10
* Date               : 28th, April 2017
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

#ifndef __FTS_LGD_09_H__
#define __FTS_LGD_09_H__


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
/////////////////////////////////////////////////////////////////
#if 0
#define I2C_ERR     -5 //khl

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
};


#if 1
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#endif

#endif

/////////////////////////////////////////////////////////////////


#define	FTSD3_V9

#define FTS_I2C_ADDR_V9						0x92

#if	defined (FTSD3_V9)
 #define FTS_FIFO_MAX_V9						64
 #define FTS_ID0_V9							0x36
 #define FTS_ID1_V9							0x70
#else
 #define FTS_FIFO_MAX_V9						32
 #define FTS_ID0_V9							0x39
 #define FTS_ID1_V9							0x6C
#endif

#define FTS_EVENT_SIZE_V9						8
#define FTS_FIFO_ADDR_V9						0x85
#if defined (FTSD1_V9)
 #define DOFFSET_V9                 			0
#else
 #define	DOFFSET_V9							1
#endif

/**
  * @brief  Definitions for Project
  */
#define	PRJ_LGMC_V9
#ifdef PRJ_LGMC_V9
 //#define JIG_SMT_INSPECTION_EQUIPMENT
 #define LIMIT_MUTUAL_TOTAL_CX_V9
#endif

/**
  * @brief  Definitions for FTS Features
  */
//#define FTS_SUPPORT_FW_DOWNLOAD
#define FTS_SUPPORT_SELF_SENSE_V9
//#define	FTS_SUPPORT_MS_KEY
//#define	FTS_SUPPORT_FORCE_FSR
#define	FTS_LOCKDOWNCODE_FEATURE_V9 //modify 170724 khl
#define	FTS_SUPPORT_CHK_HIGH_FREQ_V9
#define FTS_SUPPORT_CHK_LPMODE_V9
#define FTS_SUPPORT_ITOTEST_HW_V9
#define FTS_SUPPORT_ITOTEST_SW_V9

/**
  * IMPORTANT : FTS_PURE_AUTOTUNE_FEATURE
  *   PLEASE CONTACT LGD OR STM IF YOU WANT TO ENABLE THIS FEATURE.
  *   THIS FEATURE MUST TO BE ENABLED ONLY AT FINAL INSPECTION JIG EQUIPMENT.
  */
#define	FTS_PURE_AUTOTUNE_FEATURE_V9
#ifndef	FTS_PURE_AUTOTUNE_FEATURE_V9
 #define	FTS_SUPPORT_CX_BACKUP_V9
#endif

//#define	FTS_HW_PIN_CHECK

#define FTS_TX_LENGTH_V9						16
#define FTS_RX_LENGTH_V9						32

#ifdef	FTS_SUPPORT_MS_KEY_V9
#define	FTS_MSKEY_TX_LENGTH_V9					1
#define	FTS_MSKEY_RX_LENGTH_V9					2
#endif

#ifdef	FTS_SUPPORT_FORCE_FSR_V9
#define FTS_FORCE_TX_LENGTH_V9					1
#define FTS_FORCE_RX_LENGTH_V9					4
#endif

/**
 * @brief  FTS lockdown code definitions
 */
#ifdef	FTS_LOCKDOWNCODE_FEATURE_V9
#ifdef PRJ_LGMC_V9
 #define FTS_LOCKDOWNCODE_SIZE_V9					13
#else
 #define FTS_LOCKDOWNCODE_SIZE_V9					8
#endif
 
#endif

/** @defgroup FTS_Debug_Definitions
  * @{
  */
#define	__DBG_FTS_JIG_V9__
#ifdef	__DBG_FTS_JIG_V9__
 //#define	__DBG_FTS_MS_RAW_V9__
 //#define	__DBG_FTS_MS_CX2_V9__
 #define	__DBG_FTS_MS_JIT_V9__
 //#define	__DBG_FTS_SS_RAW_V9__
 //#define	__DBG_FTS_SS_IX_V9__
 //#define	__DBG_FTS_FORCE_V9__
 //#ifdef	FTS_SUPPORT_MS_KEY_V9
 //#define	__DBG_FTS_MS_KEY_V9__
 //#endif
#endif

/**
  * @brief  FTS Event ID definitions
  */
#define EVENTID_NO_EVENT_V9					0x00
#define EVENTID_ENTER_POINTER_V9				0x03
#define EVENTID_LEAVE_POINTER_V9				0x04
#define EVENTID_MOTION_POINTER_V9				0x05
#define EVENTID_ERROR_V9						0x0F
#define EVENTID_CONTROLLER_READY_V9			0x10
#define EVENTID_SLEEPOUT_CONTROLLER_READY_V9	0x11
#define EVENTID_RESULT_READ_REGISTER_V9		0x12
#define	EVENTID_COMPENSATION_READ_V9			0x13
#define	EVENTID_SYSTEM_STATUS_UPDATE_V9		0x16
#define	EVENTID_LOCKDOWN_CODE_V9				0x1E

/**
  * @brief  FTS Status ID & Error Type definitions for Event ID
  */
#define	EVENTID_STATID_AT_MS_DONE_V9							0x01
#define EVENTID_STATID_AT_SS_DONE_V9							0x02
#define	EVENTID_STATID_CX_BACKUP_DONE_V9					0x04
#define EVENTID_STATID_FCAL_DONE_V9							0x06

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V9
 #define	EVENTID_STATID_LOCKDOWN_WRITE_DONE_V9	0x08
 #define	EVENTID_ERRTYPE_LOCKDOWN_V9			0x0B
#endif

#define	EVENTID_STATID_PAT_SET_V9					0x10
#define	EVENTID_STATID_PAT_CLEAR_V9				0x11

/**
  * @brief  FTS Event IDe definitions
  */
#if defined (FTSD3_V9)
 #define EVENTID_INTERNAL_RELEASE_INFO_V9       0x14
 #define EVENTID_EXTERNAL_RELEASE_INFO_V9       0x15
#else
 #define EVENTID_INTERNAL_RELEASE_INFO_V9       0x19
 #define EVENTID_EXTERNAL_RELEASE_INFO_V9       0x1A
#endif

/**
  * @brief  FTS command definitions
  */
#define READ_STATUS_V9							0x84
#define READ_ONE_EVENT_V9						0x85
#define READ_ALL_EVENT_V9						0x86
#define	SLEEPOUT_V9									0x91
#define SENSEOFF_V9									0x92
#define SENSEON_V9										0x93
#define SELF_SENSEOFF_V9							0x94
#define SELF_SENSEON_V9							0x95
#define LPOSC_TRIM_V9								0x97
#define MSKEY_SENSEOFF_V9						0x9A
#define MSKEY_SENSEON_V9						0x9B
#define FLASH_BACKUP_V9						0xFB
#define FLASH_CX_BACKUP_V9						0xFC

#define FLUSHBUFFER_V9							0xA1
#define FORCECALIBRATION_V9					0xA2
#define MUTUAL_AUTO_TUNE_V9					0xA3
#define SELF_AUTO_TUNE_V9						0xA4
#ifdef PRJ_LGMC_V9
#define HF_MUTUAL_AT_V9							0xA5
#endif
#define MSKEY_AUTO_TUNE_V9						0x96
#define ITO_CHECK_V9									0xA7
#define FTS_CMD_RELEASEINFO_V9    		0xAA
#define LOWPOWER_MODE_V9							0xAD

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V9
 #define LOCKDOWN_WRITE_V9						0xC5
 #define LOCKDOWN_REWRITE_V9					0xC6
 #define LOCKDOWN_READ_V9						0xC4
#endif

/**
  * @brief  FTS Flash definitions
  */
#define FW_MAIN_BLOCK_V9						0x01
#define FW_INFO_BLOCK_V9						0x02

#if	defined(FTSD3_V9)
 #define WRITE_CHUNK_SIZE_V9					32
 #define FLASH_CHUNK_V9						(64 * 1024)
 #define DMA_CHUNK_V9							32

 #define FW_HEADER_SIZE_V9						64
 #define FW_HEADER_FTB_SIGNATURE_V9			0xAA55AA55
 #define FW_FTB_VER_V9							0x00000001
 #define FW_BYTES_ALLIGN_V9					4
 #define FW_BIN_VER_OFFSET_V9					16
 #define FW_BIN_CONFIG_VER_OFFSET_V9			20

 /* Command for flash */
 #define FLASH_CMD_UNLOCK_V9					0xF7
 #define FLASH_CMD_WRITE_64K_V9				0xF8
 #define FLASH_CMD_READ_REGISTER_V9			0xF9
 #define FLASH_CMD_WRITE_REGISTER_V9			0xFA

 /* Parameters for commands */
 #define ADDR_WARM_BOOT_V9						0x001E
 #define WARM_BOOT_VALUE_V9					0x38
 #define FLASH_ADDR_CODE_V9					0x00000000
 #define FLASH_ADDR_CONFIG_V9					0x0000FC00

 #define FLASH_UNLOCK_CODE0_V9					0x74
 #define FLASH_UNLOCK_CODE1_V9					0x45

 #define FLASH_ERASE_UNLOCK_CODE0_V9			0x72
 #define FLASH_ERASE_UNLOCK_CODE1_V9			0x03
 #define FLASH_ERASE_UNLOCK_CODE2_V9			0x02
 #define FLASH_ERASE_CODE0_V9					0x02
 #define FLASH_ERASE_CODE1_V9					0xC0
 #define FLASH_DMA_CODE0_V9					0x05
 #define FLASH_DMA_CODE1_V9					0xC0
 #define FLASH_DMA_CONFIG_V9					0x06

#else
 #define FTS_CMD_WRITE_PRAM_V9					0xF0
 #define FTS_CMD_BURN_PROG_FLASH_V9			0xF2
 #define FTS_CMD_ERASE_PROG_FLASH_V9			0xF3
 #define FTS_CMD_READ_FLASH_STAT_V9			0xF4

 #define WRITE_CHUNK_SIZE_V9					(2 * 1024) - 16
 #define WRITE_BURN_SIZE_V9					64
 //#define WRITE_CMD_SIZE					32
 //#define WRITE_CMD_LENGTH					165

 #define FTS_FIRMWARE_SIZE_V9					(122 * 1024)
 #define FTS_CONFIG_SIZE_V9					(2 * 1024)
 #define FTS_BIN_SIZE_V9						(128 * 1024)
#endif

/**
  * @brief  FTS inspection data definitions
  */
#define	FTS_COMP_NODEDATA_OFFSET_V9			0x10
#define	FTS_COMP_MS_CX_SEL_V9					(uint16_t) 0x0002
#define	FTS_COMP_MS_LP_SEL_V9					(uint16_t) 0x0004
#define	FTS_COMP_MS_KEY_SEL_V9					(uint16_t) 0x0010
#define	FTS_COMP_SS_IX_SEL_V9					(uint16_t) 0x0020
#define	FTS_COMP_MS_FORCE_SEL_V9				(uint16_t) 0x0200

#define	FTS_COMP_SS_TX_IX1_RATIO_V9				2
#define	FTS_COMP_SS_RX_IX1_RATIO_V9				2

/**
  * @brief  SYSINFO - Offset address for getting data
  */
#define FTS_MS_RAW_ADDR_V9       				0x02
#define FTS_MS_JIT_ADDR_V9						0x04
#define FTS_SS_F_RAW_ADDR_V9					0x1E
#define FTS_SS_S_RAW_ADDR_V9					0x20
#define FTS_MS_KEY_RAW_ADDR_V9					0x34
#define	FTS_MS_KEY_STR_ADDR_V9					0x36
#define FTS_READ_PAT_FLAG_V9					0x4E
#define FTS_GET_AFE_STATUS_V9					0x52

/**
 * @brief  Type of data for FSR sensor
 */
#ifdef	FTS_SUPPORT_FORCE_FSR_V9

 #define FTS_FORCE_RAW_ADDR_V9       			(uint16_t) 0x0052
 #define FTS_FORCE_STR_ADDR_V9       			(uint16_t) 0x0056
#endif

/**
  * @brief  FTS general definitions
  */
/*
#define ON				0x01
#define OFF				0x00
*/
#define TRUE			1
#define FALSE			0

#define	ENABLE			1
#define	DISABLE			0

enum	fts_error_type_v9
{
	FTS_NO_ERR_V9 = 1,
	FTS_ERR_TIMEOUT_V9 = 0xF
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
enum	protocol_type_v9
{
	VER_1p1_V9 = 1,
	VER_1p2_V9 = 2,
	VER_1p3_V9 = 3
};

enum	write_type_v9
{
	FIRMWARE_V9 = 0,
	CONFIG_V9,
	BOTH_V9,
	BINARY_V9
};

enum binfile_type_v9
{
	BIN_FTS128_V9 = 1,
	BIN_FTS256_V9 = 2,
	BIN_FTB_V9 = 3
};

enum	ito_error_type_v9
{
	NO_ERROR_V9 = 0,
	ITO_FORCE_OPEN_V9,
	ITO_SENSE_OPEN_V9,
	ITO_FORCE_SHRT_GND_V9,
	ITO_SENSE_SHRT_GND_V9,
	ITO_FORCE_SHRT_VCM_V9,
	ITO_SENSE_SHRT_VCM_V9,
	ITO_FORCE_SHRT_FORCE_V9,
	ITO_SENSE_SHRT_SENSE_V9,
	ITO_F2E_SENSE_V9,
	ITO_FPC_FORCE_OPEN_V9,
	ITO_FPC_SENSE_OPEN_V9,
	ITO_KEY_FORCE_OPEN_V9,
	ITO_KEY_SENSE_OPEN_V9,
	ITO_RESERVED0_V9,
	ITO_RESERVED1_V9,
	ITO_RESERVED2_V9,
	ITO_MAX_ERR_REACHED_V9 = 0xFF
};

/* Global typedef -----------------------------------------------------------*/
typedef unsigned int	uint_t;
#if	0
 typedef unsigned short	uint16_t;
 typedef unsigned char	uint8_t;

 typedef unsigned long	uint32_t;
 typedef char			int8_t;
 typedef short			int16_t;
 typedef long			int32_t;
#endif
struct fts64_header_v9 {
	unsigned int		signature;
	uint16_t	fw_ver;
	uint8_t		fw_id;
	uint8_t		reserved1;
	uint8_t		internal_ver[8];
	uint8_t		released_ver[8];
	unsigned int 		reserved2;
	unsigned int 		checksum;
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
}	FW_FTB_HEADER_V9;

/** @defgroup FTS_Exported_FunctionsPrototype
  * @{
  */
int fts_systemreset_v9(void);
int fts_event_handler_v9(void);
int flashProcedure_v9(int force, int keep_cx);
int fts_panel_test_v9(int id, unsigned char *uart_buf);
int fts_init_v9(void);
//////////////////////////////////////////PDK
void i2c_dev_match_v9(int i2c_dev);
int fts_get_channelInfo_v9(void);
int fts_lockdown_read_v9(int id);
/**
  * @}
  */

/**
  * @}
  */

#endif	/* __FTS_LGD_09_H__ */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/

