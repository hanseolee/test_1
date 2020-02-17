/******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
* File Name          :
* Author             : AMG KOREA
* Version            : V0.15
* Date               : 7th of September, 2017
* Description        : Reference code for panel test of FTM
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////

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
#include <type.h>
#include <rs485.h>
#include    <stm_touch.h>
#include <i2c-dev.h>
#include <fts_lgd_15.h>

#define _IOCTL_TS1_INT_GET          0x15005
#define _IOCTL_TS2_INT_GET          0x15006
#define TM_GET_INTERVAL(st, et) ((et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec))
#define MIN     0
#define MAX     1

#define	vcp_printf(x)	printf(x)

int stm_dev;
int fts_read_chip_id_v15(void);

int chip_id_v15 = 0;
int fw_version_v15 = 0;
int config_ver_v15 = 0;
int release_ver_v15 = 0;

unsigned short  l_product_id;
unsigned short  l_fw_ver;
unsigned short  l_config_ver;
unsigned short  l_release_ver;
unsigned int    l_pat_cm_reference_raw[2];
unsigned int    l_pat_self_raw_tx[2];
unsigned int    l_pat_self_raw_rx[2];

int    l_cm_reference_raw[2];
int    l_cm_reference_gap;
int             l_cm_jitter[2];
unsigned int    l_total_cx[2];
int    l_self_raw_tx[2];
int    l_self_raw_rx[2];
int    l_lp_self_raw_tx[2];
int    l_lp_self_raw_rx[2];

unsigned int    l_self_ix_tx[2];
unsigned int    l_self_ix_rx[2];
unsigned int    l_cx2_hf[2];
unsigned char   l_otp_param;
unsigned char   l_otp_param1;
unsigned char   l_otp_param2;
unsigned char   l_otp_param3;
int             l_hf_test_mode;

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


///////////////////////////////////////////////////////////////////////



/** @addtogroup FTS_Private_Variables
  * @{
  */
static uint8_t fts_fifo_addr[2] = {FTS_FIFO_ADDR_V15, 0};

/**
  * @}
  */

/** @addtogroup FTS_Functions
  * @{
  */

/**
  * @brief  fts_delay
  *         Internal milli-second delay function
  * @param  msCount : specifies the delay time length
  * @retval None
  */

void fts_delay_v15(uint32_t msCount)
{
    uint32_t i;

    for(i=0; i<300; i++)    //500
        usleep(msCount);
}

/**
  * @brief  Supply a voltage for analog (3.3V)
  * @param  None
  * @retval None
  */
void analog_3V3_power_on_v15(void)
{
	/* user code */
}

/**
  * @brief  Supply a voltage for digital (1.8V)
  * @param  None
  * @retval None
  */
void digital_1V8_power_on_v15(void)
{
	/* user code */
}

/**
  * @brief  Supply a voltage to FTS
  * @note	Recommends to turn on 3.3V and 1.8V at the same time or 3.3V earlier than 1.8V.
  * @param  None
  * @retval None
  */
void power_on_v15(void)
{
	FUNC_BEGIN();
	/* Please turn on 3.3V earlier than 1.8V */
	analog_3V3_power_on_v15();
	fts_delay_v15(5);

	digital_1V8_power_on_v15();
	fts_delay_v15(10);
	FUNC_END();
}

#if	1  

/**
  * @brief  Write data to target registers.
  * @param  pInBuf  : A pointer of the buffer to be sending to target.
  * @param  inCnt   : The count of pInBuf to be writing.
  * @retval OK if all operations done correctly. FAIL if error.
  */

//////////////////////////////////////////////// modify 171017 khl
int fts_write_reg_v15(uint8_t *pInBuf, int InCnt)
{
	/* Implement user's fts_write_reg depend on your system. */
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V15 >> 1;
    messages[0].flags = 0;
    messages[0].len = InCnt;
    messages[0].buf = (char *)pInBuf;

    packets.msgs = messages;
    packets.nmsgs = 1;

    if(ioctl(stm_dev, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
		FUNC_END();
        return 1;
    }
	FUNC_END();
    return  0;
}

/**
  * @brief  Read data from the device registers.
  * @note	It has to use reStart transaction when read data.
  * @param  pInBuf  : A pointer of the target register adress.
  * @param  inCnt   : The count of pInBuf for sending.
  * @param  pOutBuf : A pointer of the buffer containing the read bytes.
  * @param  outBuf  : The count of bytes to be reading from taget.
  * @retval OK if all operations done correctly. FAIL if error.
  */
int fts_read_reg_v15(uint8_t *pInBuf, int inCnt, uint8_t *pOutBuf, int outCnt)
{
	/* Implement user's fts_read_reg depend on your system. */
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V15 >> 1;
    messages[0].flags = 0;
    messages[0].len = inCnt;
    messages[0].buf = (char *)pInBuf;

    messages[1].addr = FTS_I2C_ADDR_V15 >> 1;
    messages[1].flags = I2C_M_RD;
    messages[1].len = outCnt;
    messages[1].buf = (char *)pOutBuf;

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(stm_dev, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
		FUNC_END();
        return 1;
    }
	FUNC_END();
    return  0;
}

#endif
//////////////////////////
/**
  * @brief  Write a command to target.
  * @param  cmd		: A command.
  * @retval None
  */

///////////////////////////////////////////modify 171017 khl
//void fts_command(uint8_t cmd)
int fts_command_v15(uint8_t cmd)
{
	uint8_t	regAdd = 0;

	FUNC_BEGIN();
	regAdd = cmd;
	if(fts_write_reg_v15(&regAdd, 1))
	{
		FUNC_END();
        return I2C_ERR;
	}
	FUNC_END();
    return 0;

}
/////////////////////////
/**
  * @brief  Write a command related feature.
  * @param  cmd		: A command.
  * @retval None
  */
void fts_write_feature_v15(uint8_t status, uint16_t feature)
{
	uint8_t	regAdd[8];

	FUNC_BEGIN();
#ifdef PRJ_LGMC_V15
	regAdd[0] = (status == ENABLE) ? 0xC1 : 0xC2;
	regAdd[1] = 0x00;		regAdd[2] = 0x00; 	regAdd[3] = (feature >> 8) & 0xFF;	regAdd[4] = feature & 0xFF;
	fts_write_reg_v15(&regAdd[0], 5);
#else
	regAdd[0] = (status == ENABLE) ? 0xC1 : 0xC2;
	regAdd[1] = 0x00;		regAdd[2] = 0x00; 	regAdd[3] = feature;
	fts_write_reg_v15(&regAdd[0], 4);
#endif
	fts_delay_v15(1);
	FUNC_END();
}

/**
  * @brief  System Reset
  * @param  None
  * @retval None
  */
/////////////////////////////////////modify 171017 khl
int fts_systemreset_v15(void)
{
#if	defined(FTSD3_V15)
	uint8_t regAdd[4] = {0xB6, 0x00, 0x28, 0x80};
	uint8_t	warmReg[4] = {0xB6, 0x00, 0x1E, 0x20};
#else
	uint8_t regAdd[4] = {0xB6, 0x00, 0x23, 0x01};
#endif

	FUNC_BEGIN();
#if	defined(FTSD3_V15)
	if(fts_write_reg_v15(&warmReg[0], 4))
		return I2C_ERR;
	fts_delay_v15(10);
#endif

	if(fts_write_reg_v15(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(10);

	FUNC_END();
	return 0;
}

int fts_get_interrupt_status_v15(void)
{
#if	defined(FTSD3_V15)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x2C, 0x00};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x1C, 0x00};
#endif
	uint8_t	val[10];

	FUNC_BEGIN();
	if(fts_read_reg_v15(regAdd, 3, (uint8_t *) val, 2))
	{
		FUNC_END();
		return	I2C_ERR;	
	}
	if (val[1] & 0x40)
	{
		printf("\n\r[fts_get_interrupt_status] Interrupt is enabled.");
		FUNC_END();
		return	TRUE;
	}
	else
	{
		printf("\n\r[fts_get_interrupt_status] Interrupt is disabled.");
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	FALSE;
}

/**
  * @brief  Interrupt enable or disable
  * @param  onoff	: ENABLE or DISABLE
  * @retval None
  */
int fts_interrupt_control_v15(int onoff)
{
#if	defined(FTSD3_V15)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x2C, 0x08};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x1C, 0x00};
#endif

	FUNC_BEGIN();
	if (onoff == ENABLE)
		regAdd[3] |= 0x40;

	if(fts_write_reg_v15(&regAdd[0], 4))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v15(1);

	FUNC_END();
	return 0;
}

/**
  * @brief  Read chip id of FTS
  * @param  None
  * @retval status	: TRUE if Chip ID is okay, FALSE if not.
  */
int fts_read_chip_id_v15(void)
{
#if	defined(FTSD3_V15)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x04, 0x00};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x07, 0x00};
#endif
	uint8_t	val[8] = {0};
	int		retry = 10;

	FUNC_BEGIN();
	while (retry--)
	{
		fts_delay_v15(1);
		if(fts_read_reg_v15(regAdd, 3, (uint8_t *) val, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if ((val[1] == ((l_product_id >> 8)&0xFF)) && (val[2] == (l_product_id & 0xFF )))
		{
		#if	defined(FTSD3_V15)
			if ((val[5] == 0x00) && (val[6] == 0x00))
			{
				printf("\n\r[fts_read_chip_id] Error - No FW : %02X %02X", val[5], val[6]);
		#else
			if ((val[4] == 0x00) && (val[5] == 0x00))
			{
				printf("\n\r[fts_read_chip_id] Error - No FW : %02X %02X", val[4], val[5]);
		#endif
				FUNC_END();
				return	FALSE;
			}
			else
			{
				printf("\n\r[fts_read_chip_id] Chip ID : %02X %02X", val[1], val[2]);
				chip_id_v15 = (val[1] << 8) | val[2];
				FUNC_END();
				return	TRUE;
			}
		}
	}
	if (retry <= 0)
		printf("[fts_read_chip_id] Error - Time Over\n (config product ID : %02X%02X / original product ID : %02X%02X)\n", ((l_product_id >> 8)&0xFF), (l_product_id & 0xFF ), FTS_ID0_V15, FTS_ID1_V15);
		//printf("\n\r[fts_read_chip_id] Error - Time Over");

	FUNC_END();
	return FALSE;
}

/**
  * @brief  Check event messages for commands
  * @param  event[1..3] : Event messages
  * @retval status	: TRUE if event message for command received, FALSE if not.
  */
int fts_cmd_completion_check_v15(uint8_t event1, uint8_t event2, uint8_t event3)
{
	uint8_t val[8];
	int 	retry = 100;
	int		status = TRUE;

	FUNC_BEGIN();
	while (retry--)
	{
		fts_delay_v15(10);

		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if ((val[0] == event1) && (val[1] == event2) && (val[2] == event3))
		{
			printf("\n\r[fts_cmd_completion_check] OK [%02x][%02x][%02x] %d\n", val[0], val[1], val[2],retry);
			FUNC_END();
			return TRUE;
		}
		else if (val[0] == EVENTID_ERROR_V15)
		{
			if (val[1] == EVENTID_STATID_ERROR_CRC_V15)
			{
				if (val[2] == 0x02)
					status = FTS_ERR_CRC_CFG_V15; //-11
				else if (val[2] == 0x03)
					status = FTS_ERR_CRC_CX_V15; //-12
				else
					status = FTS_ERR_CRC_V15; //-10

				printf("\n\r[fts_cmd_completion_check] CRC ERROR - [%02x][%02x][%02x], status = %d\n", val[0], val[1], val[2],status);
				FUNC_END();
				return	status;
			}
			printf("\n\r[fts_cmd_completion_check] Error - [%02x][%02x][%02x]\n", val[0], val[1], val[2]);
		}
	}
	if (retry <= 0)
		printf("\n\r[fts_cmd_completion_check] Error - Time Over [%02x][%02x][%02x]\n", event1, event2, event3);

	FUNC_END();
	return FALSE;
}
/////////////////////////////////////////////////////below need modify
/**
  * @brief  Read released version for firmware
  * @param  None
  * @retval None
  */
int fts_get_versionInfo_v15(void) //modify 171023 khl
{
	uint8_t	data[FTS_EVENT_SIZE_V15];
//	uint8_t serial_num[6];
	int		retry = 30, intr_status = FALSE;
	int		i;
	int		ret = 0;

	FUNC_BEGIN();
	intr_status = fts_get_interrupt_status_v15();
	if (intr_status == TRUE)
	{
		fts_interrupt_control_v15(DISABLE);
		fts_delay_v15(10);
	}

	fts_command_v15(FTS_CMD_RELEASEINFO_V15);
	while (retry--)
	{
		fts_delay_v15(1);
		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}

		if (data[0] == EVENTID_INTERNAL_RELEASE_INFO_V15)
		{
			//vcp_printf("\n\r[fts_get_versionInfo] FW Ver. : 0x%2x%2x, Config Ver. : 0x%2x%2x", data[3], data[4], data[6], data[5]);

            fw_version_v15 = (data[4] << 8) | data[3]; //171123
            config_ver_v15 = (data[6] << 8) | data[5];

            printf("READ fw_version 0x%04X (limit:0x%04X)\n",fw_version_v15,l_fw_ver);
            printf("READ config_version 0x%04X (limit:0x%04X)\n",config_ver_v15,l_config_ver);

            if(fw_version_v15 == l_fw_ver)
            {
                //printf(">>>>> FW_VERSION OK\n");
                ret |= (TRUE << 0);
            }
            else
            {
                //printf(">>>>> FW_VERSION FAIL\n");
                ret |= (FALSE << 0);
            }


            if(config_ver_v15 == l_config_ver)
            {
                //printf(">>>>> CONFIG_VERSION OK\n");
                ret |= (TRUE << 1);
            }
            else
            {
                //printf(">>>>> CONFIG_VERSION FAIL\n");
                ret |= (FALSE << 1);
            }

		}
		else if (data[0] == EVENTID_EXTERNAL_RELEASE_INFO_V15)
		{
//			vcp_printf("\n\r[fts_get_versionInfo] Release Version : 0x%2x%2x", data[1], data[2]);

            release_ver_v15 = (data[1] << 8) | data[2];
            printf("READ release_version 0x%04X (limit:0x%04X)\n",release_ver_v15,l_release_ver);
            if(release_ver_v15 == l_release_ver)
            {
                //printf(">>>>> RELEASE_VERSION OK\n");
                ret |= (TRUE << 2);
            }
            else
            {
                //printf(">>>>> RELEASE_VERSION FAIL\n");
                ret |= (FALSE << 2);
            }

            break;
		}
		else if (data[0] == EVENTID_SERIALNUMBER_HIGH_V15) //??
		{
			for (i = 0; i < 6; i++)
			{
//				serial_num[i] = data[i + 1];
			}
		}
		else if (data[0] == EVENTID_SERIALNUMBER_LOW_V15) //??
		{
			for (i = 0; i < 6; i++)
			{
//				serial_num[i + 6] = data[i + 1];
			}
			break; 
		}
	}
	if (retry <= 0)
	{
		printf("\n\r[fts_get_versionInfo] Error - Time Over");
        ret |= FALSE << 0;
        ret |= FALSE << 1;
        ret |= FALSE << 2;
	}

	if (intr_status == TRUE)
	{
		fts_interrupt_control_v15(ENABLE);
		fts_delay_v15(10);
	}
	
	FUNC_END();
	return ret;
}

/**
  * @brief  Read channel information for sensor
  * @param  None
  * @retval TRUE if All operations done correctly, FALSE if not.
  */
int fts_get_channelInfo_v15(void)
{
	int		status = -1;
	uint8_t cmd[4] = {0xB2, 0x00, 0x14, 0x02};
	uint8_t data[FTS_EVENT_SIZE_V15];
	int		retry = 30, intr_status;

	FUNC_BEGIN();
	intr_status = fts_get_interrupt_status_v15();
	if (intr_status == TRUE)
	{
		fts_interrupt_control_v15(DISABLE);
		fts_delay_v15(10);
	}
	memset(data, 0x0, FTS_EVENT_SIZE_V15);

	if(fts_write_reg_v15(&cmd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	while (retry--)
	{
		fts_delay_v15(5);
		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}

		if (data[0] == EVENTID_RESULT_READ_REGISTER_V15)
		{
			if ((data[1] == cmd[1]) && (data[2] == cmd[2]))
			{
				printf("\n\r[fts_get_channelInfo] Sense length : %d", data[3]);
				printf("\n\r[fts_get_channelInfo] Force length : %d", data[4]);
				status = TRUE;
				break;
			}
		}
	}
	if (retry <= 0)
	{
		printf("\n\r[fts_get_channelInfo] Error - Time over");
		status = FALSE;
	}

	if (intr_status == TRUE)
	{
		fts_interrupt_control_v15(ENABLE);
		fts_delay_v15(10);
	}

	FUNC_END();
	return	status;
}

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V15

/**
  * @brief	Calculates CRC8 value of an array of bytes & return 8-bit CRC value
  * @param	u8_srcBuff[]	: input array of bytes
  * @param	u32_len			: the number of input bytes
  * @param	u8_polynomial	: polynomial for CRC calculation
  * @param	u8_initCRCVal	: Initial CRC value
  * @return	CRC8 result
  * @note	The CRC Polynomial and initial CRC value are fixed, as following:
			uint8_t u8_polynomial = 0x9B // WCDMA standard
			uint8_t u8_initCRCValue = 0x00 // WCDMA standard.
 */
uint8_t CalculateCRC8_v15(uint8_t* u8_srcBuff, uint32_t u32_len, uint8_t u8_polynomial, uint8_t u8_initCRCVal)
{
	uint8_t u8_remainder;
	u8_remainder = u8_initCRCVal;

	FUNC_BEGIN();
	// Perform modulo-2 division, a byte at a time.
	for (uint32_t u32_i = 0; u32_i < u32_len; u32_i++)
	{
		//Bring the next byte into the remainder.
		u8_remainder ^= u8_srcBuff[u32_i];

		//Perform modulo-2 division, a bit at a time.
		for (uint8_t bit = 8; bit > 0; --bit)
		{
			//Try to divide the current data bit.
			if (u8_remainder & (0x1 << 7)) // MSB is 1
			{
				u8_remainder = (u8_remainder << 1) ^ u8_polynomial;
			}
			else // MSB is 0
			{
				u8_remainder = (u8_remainder << 1);
			}
		}
	}
	// The final remainder is the CRC result.
	FUNC_END();
	return (u8_remainder);
}

uint8_t lockdown_v15(uint8_t *input_code)
{
	uint8_t u8_polynomial = 0x9B;		// WCDMA standard
	uint8_t u8_initCRCValue = 0x00;		// WCDMA standard.
	uint8_t u8_crcResult;

	FUNC_BEGIN();
	u8_crcResult = CalculateCRC8_v15(input_code, FTS_LOCKDOWNCODE_SIZE_V15, u8_polynomial, u8_initCRCValue);

	FUNC_END();
	return	u8_crcResult; // this is CRC result
}

/**
  * @note	Must to use this function after getting a Permission from LGD.
  * 		Total 3-time writings of lockdown code is supported.
  * @brief  Write/Re-write the lockdown code
  * @param  command			: write or re-write command.
  * @retval TRUE if All operations done correctly, FALSE if not.
  */
//int fts_lockdown_read(void)
int fts_lockdown_read_v15(int id)
{
	uint8_t	data[10];
	int		retry = 50;
	int		nwr_count = 0;
    int     count = 0;
    unsigned char   param = 0;
    unsigned char   param1 = 0;
    unsigned char   param2 = 0;
    unsigned char   param3 = 0;

	FUNC_BEGIN();
	fts_systemreset_v15();
	if (fts_cmd_completion_check_v15(EVENTID_CONTROLLER_READY_V15, 0x00, 0x00) < TRUE)
	{
		FUNC_END();
		printf("\n\r[fts_lockdown_read] FAILED - System Reset");
		return	FALSE;
	}
	fts_interrupt_control_v15(ENABLE);
	fts_delay_v15(50);
	fts_command_v15(LOCKDOWN_READ_V15);
    if(DEBUG_MODE)
        printf("RAW DATA : ");

	while (retry--)
	{
		fts_delay_v15(5);

		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return	I2C_ERR;
		}
		if ((data[0] == EVENTID_LOCKDOWN_CODE_V15) && (data[1] == FTS_LOCKDOWNCODE_SIZE_V15))
		{
			count++;
			//printf("\n\r[fts_lockdown_read] code[0x%02X] : 0x%02X 0x%02X 0x%02X 0x%02X", data[2], data[3], data[4], data[5], data[6]);

            if(count == 1)
            {
                if((id == MV)||(id == MV_MANUAL))
                    param1 = data[6] & 0xFF;
            }
            else if(count == 2)
            {
                if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL))
                    param = data[3] & 0xFF;
                else if((id == MV)||(id == MV_MANUAL))
                {
                    param2 = data[3] & 0xFF;
                    param3 = data[4] & 0xFF;

                }
            }

			if (data[2] == 0x00)
				nwr_count = data[7];

			if (data[2] >= FTS_LOCKDOWNCODE_SIZE_V15 - 4)
			{
				//printf("\n\r[fts_lockdown_read] write count : %d", nwr_count);
                if(DEBUG_MODE)
                    printf("\n");
                if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL))
                    printf(" >> OTP READ DATA [0x%X]\n>> OTP SPEC [0x%X]\n",param,l_otp_param);
                else if((id == MV)||(id == MV_MANUAL))
                    printf(" >> OTP READ DATA [0x%X][0x%X][0x%X]\nOTP SPEC [0x%X] [0x%X] [0x%X]\n",param1,param2,param3,l_otp_param1,l_otp_param2,l_otp_param3);
//what
                //printf("[fts_lockdown_read] write count : %d\n", nwr_count);
                if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL))
                {
                    if(param == l_otp_param)
					{
						FUNC_END();
                        return  TRUE;
					}
                    else
					{
						FUNC_END();
                        return FAILED;
					}
                }
                else if((id == MV)||(id == MV_MANUAL))
                {
                    if((param1 == l_otp_param1) && (param2 == l_otp_param2) && (param3 == l_otp_param3))
					{
						FUNC_END();
                        return  TRUE;
					}
#if 1
                    else if((param1 == l_otp_param1) && (param3 == l_otp_param2) && (param2 == l_otp_param3))
					{
						FUNC_END();
                        return TRUE;
					}
/***********************************/
/* Added by iamozzi. 17.09.08.     */
                    else if ((param1 == l_otp_param1) && (param3 == 0x01) && (param2 == 0x01))
					{
/**********************************/
						FUNC_END();
                        return TRUE;
					}
#endif
                    else
					{
						FUNC_END();
                        return FAILED;
					}
                }
			}
		}

		if ((data[0] == EVENTID_ERROR_V15) && (data[1] == EVENTID_ERRTYPE_LOCKDOWN_V15))
		{
			switch (data[2] & 0x0F)
			{
			case	0x01:	printf("\n\r[fts_lockdown_read] Error - no lockdown code");
				FUNC_END();
				return	FALSE;
			case	0x02:	printf("\n\r[fts_lockdown_read] Error - Data Corrupted");
				FUNC_END();
				return	FALSE;
			case	0x03:	printf("\n\r[fts_lockdown_read] Error - Command format invalid");
				FUNC_END();
				return	FALSE;
			}
		}
	}
	if (retry++ > 50)
		printf("\n\r[fts_lockdown_read] Error - Time over");

    printf("\n");
    printf(" > OTP READ DATA [0x%X]\n",param);

	FUNC_END();
	return	FALSE;
}

/**
  * @note	Must to use this function after getting a Permission from LGD.
  * 		Total 3-time writings of lockdown code is supported.
  * @brief  Write/Re-write the lockdown code
  * @param  command			: write or re-write command.
  * @retval TRUE if All operations done correctly, FALSE if not.
  */

int fts_lockdown_write_v15(uint8_t command)
{
	/* Important : Maximum lockdown code size is 13 bytes */
	uint8_t lockdown_code[FTS_LOCKDOWNCODE_SIZE_V15] = { 0x42, 0x32, 0x31, 0x01, 0x43, 0x32, 0x31, 0x00 };
	uint8_t	regAdd[20], data[10];
	int		retry, i;

	FUNC_BEGIN();
	fts_systemreset_v15();
	if (fts_cmd_completion_check_v15(EVENTID_CONTROLLER_READY_V15, 0x00, 0x00) < TRUE)
	{
		printf("\n\r[fts_lockdown_write] FAILED - System Reset");
		FUNC_END();
		return	FALSE;
	}

	regAdd[0] = command;
	memcpy(regAdd + 1, lockdown_code, FTS_LOCKDOWNCODE_SIZE_V15);			// Copy locdown code
	regAdd[FTS_LOCKDOWNCODE_SIZE_V15 + 1] = lockdown(lockdown_code);		// Calculate a CRC.
	if(fts_write_reg_v15(&regAdd[0], FTS_LOCKDOWNCODE_SIZE_V15 + 2))
	{
		FUNC_END();
		return I2C_ERR;
	}
	printf("\n\r[fts_lockdown_write] lockdown code : ");
	for (i = 0; i < FTS_LOCKDOWNCODE_SIZE_V15 + 2; i++)
		printf("0x%02X, ", regAdd[i]);
	printf("\n");

	retry = 50;
	while (retry--)
	{
		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return	I2C_ERR;
		}
		fts_delay_v15(50);
		printf("\n\r[fts_lockdown_write] read data : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X"
							, data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
		if ((data[0] == EVENTID_SYSTEM_STATUS_UPDATE_V15) && (data[1] == EVENTID_STATID_LOCKDOWN_WRITE_DONE_V15))
		{
			printf("\n\r[fts_lockdown_write] lockdown code wrote successfully [%d]", data[2]);
			FUNC_END();
			return	TRUE;
		}

		if ((data[0] == EVENTID_ERROR_V15) && (data[1] == EVENTID_ERRTYPE_LOCKDOWN_V15))
		{
			switch (data[2] & 0x0F)
			{
			case	0x01:	printf("\n\r[fts_lockdown_write] Error - Already locked down");
				FUNC_END();
				return	FALSE;
			case	0x02:	printf("\n\r[fts_lockdown_write] Error - CRC Check Failed.");
				FUNC_END();
				return	FALSE;
			case	0x03:	printf("\n\r[fts_lockdown_write] Error - Command format invalid");
				FUNC_END();
				return	FALSE;
			case	0x04:	printf("\n\r[fts_lockdown_write] Error - Memory corrupted");
				FUNC_END();
				return	FALSE;
			}
		}
		else
		{
			if (data[0])
				printf("\n\r[fts_lockdown_write] Error - %02x %02x %02x %02x %02x %02x %02x %02x"
									, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		}
	}
	if (retry <= 0)
		printf("\n\r[fts_lockdown_write] Error - Time over");

	FUNC_END();
	return	FALSE;
}

#endif

/**
  * @brief	Get pure auto-tune flag
  * @param	void
  * @return	TRUE if All operations done correctly, FALSE if not.
 */
int fts_get_pure_autotune_flag_v15(void)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[4] = {0};
	int				ret = FALSE;

	FUNC_BEGIN();
	regAdd[2] = FTS_READ_PAT_FLAG_V15;
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], 3))
	{
		FUNC_END();
		return I2C_ERR;
	}

	printf("\n\r[fts_get_pure_autotune_flag] Pure auto-tune flag [0x%02X] [0x%02X]", tempBuf[1], tempBuf[2]);
	if ((tempBuf[1] == 0xA5) && (tempBuf[2] == 0x96))
		ret = TRUE;

	FUNC_END();
	return ret;
}

#ifdef	FTS_PURE_AUTOTUNE_FEATURE_V15

/**
  * @brief	Get the AFE status
  * @param	void
  * @return	The status of AFE
 */
int fts_get_afe_status_v15(void)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[4] = {0};
	int				ret = DISABLE;

	FUNC_BEGIN();
	regAdd[2] = FTS_GET_AFE_STATUS_V15;
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], 3))
	{
		FUNC_END();
		return I2C_ERR;
	}

	printf("\n\r[fts_get_afe_status] Final AFE Status [0x%02X], AFE Ver. [0x%02X]", tempBuf[1], tempBuf[2]);
	if (tempBuf[1] == 0x1)
		ret = ENABLE;

	FUNC_END();
	return ret;
}

/**
  * @brief	Clear pure auto-tune flag
  * @param	void
  * @return	TRUE if All operations done correctly, FALSE if not.
 */
int fts_clear_pure_autotune_flag_v15(void)
{
	uint8_t		regAdd[3] = {0xC8, 0x01, 0x00};

	FUNC_BEGIN();
	if(fts_write_reg_v15(&regAdd[0], 2))
	{
		FUNC_END();
		return I2C_ERR;
	}
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_PAT_CLEAR_V15, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	fts_delay_v15(1);
	fts_command_v15(FLASH_CX_BACKUP_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_CX_BACKUP_DONE_V15, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	TRUE;
}

/**
  * @brief	Set pure auto-tune flag
  * @param	void
  * @return	TRUE if All operations done correctly, FALSE if not.
 */
int fts_set_pure_autotune_flag_v15(void)
{
	uint8_t		regAdd[3] = {0xC7, 0x01, 0x00};

	FUNC_BEGIN();
	if(fts_write_reg_v15(&regAdd[0], 2))
	{
		FUNC_END();
		return I2C_ERR;
	}
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_PAT_SET_V15, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	fts_delay_v15(1);
	fts_command_v15(FLASH_CX_BACKUP);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_CX_BACKUP_DONE_V15, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	TRUE;
}

/**
  * @brief	Run pure auto-tune with checking AFE status
  * @param	void
  * @return	TRUE if All operations done correctly, FALSE if not.
 */
int fts_run_pure_autotune_v15(void)
{
	FUNC_BEGIN();
	fts_delay_v15(10);
	if (fts_get_afe_status_v15() < ENABLE)
	{
		printf("\n\r[fts_run_pure_autotune] Final AFE flag is disabled");
		fts_command_v15(FLASH_CX_BACKUP_V15);
		fts_delay_v15(300);
		if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_CX_BACKUP_DONE_V15, 0x00) < TRUE)
		{
			printf("\n\r[fts_run_pure_autotune] FAILED - Cx back-up");
			FUNC_END();
			return	FALSE;
		}
		printf("\n\r[fts_run_pure_autotune] Cx back-up is done");
		FUNC_END();
		return	NOT_FINAL;
	}

	fts_delay_v15(1);
	if (fts_set_pure_autotune_flag_v15() < TRUE)
	{
		printf("\n\r[fts_run_pure_autotune] FAILED - Flag setting Failed");
		FUNC_END();
		return	FAILED;
	}

	fts_delay_v15(50);
	fts_systemreset_v15();
	if (fts_cmd_completion_check_v15(EVENTID_CONTROLLER_READY_V15, 0x00, 0x00) < TURE)
	{
		printf("\n\r[fts_run_pure_autotune] FAILED - System Reset");
		FUNC_END();
		return	FAILED;
	}

	fts_delay_v15(10);
	if (fts_get_pure_autotune_flag_v15() < TRUE)
	{
		printf("\n\r[fts_run_pure_autotune] FAILED - Pure auto-tune flag is not correct");
		FUNC_END();
		return	FAILED;
	}

	printf("\n\r[fts_run_pure_autotune] Pure procedure is done successfully");

	FUNC_END();
	return	DONE;
}

#endif

/**
  * @brief  Parse and process received events.
  * @note	Reporting of finger data when the presence of fingers is detected.
  * @param  data		: The buffer of event saved.
  * @param	LeftEvent	: Count of events
  * @retval None
  */
int fts_event_handler_type_b_v15(uint8_t *data, uint8_t LeftEvent)
{
	uint8_t 	EventNum = 0;
	uint8_t		TouchID = 0, EventID = 0;
//	uint8_t		TouchCount = 0;
	uint16_t	x = 0, y = 0, z = 0;
#ifdef	FTS_SUPPORT_FORCE_FSR_V15
	uint16_t	force_sum1 = 0, force_sum2 = 0;
	uint8_t		force1_TID = 0, force2_TID = 0;
#endif

	FUNC_BEGIN();
	for (EventNum = 0; EventNum < LeftEvent; EventNum++)
	{
	#ifdef	FTS_SUPPORT_FORCE_FSR_V15
		/* Event for 3D */
		if (data[EventNum * FTS_EVENT_SIZE_V15] == 0x24)
		{
			force_sum1 = data[1 + EventNum * FTS_EVENT_SIZE_V15] + (data[2 + EventNum * FTS_EVENT_SIZE_V15] << 8);
			force_sum2 = data[3 + EventNum * FTS_EVENT_SIZE_V15] + (data[4 + EventNum * FTS_EVENT_SIZE_V15] << 8);
			force1_TID = (data[6 + EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0F;
			force2_TID = data[6 + EventNum * FTS_EVENT_SIZE_V15] & 0x0F;
			printf("\n\r[FTS] Force : %d, %d", force_sum1, force_sum2);
		}
	#endif

	#ifdef PRJ_LGMC_V15
		EventID = data[EventNum * FTS_EVENT_SIZE_V15] & 0x0F;
		/* Event for 2D - protocol of china project */
		if ((EventID >= 0x3) && (EventID <= 0x5))
		{
//			TouchCount = (data[1 + EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0F;
			TouchID = (data[EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0F;
			x = (data[1 + EventNum * FTS_EVENT_SIZE_V15] << 4) + ((data[3 + EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0F);
			y = (data[2 + EventNum * FTS_EVENT_SIZE_V15] << 4) + (data[3 + EventNum * FTS_EVENT_SIZE_V15] & 0x0F);
			z = data[4 + EventNum * FTS_EVENT_SIZE_V15];

			switch (EventID)
			{
			case	EVENTID_ENTER_POINTER_V15:
				printf("\n\r[FTS] ENTER  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVENTID_MOTION_POINTER_V15:
				printf("\n\r[FTS] MOTION : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVENTID_LEAVE_POINTER_V15:
				printf("\n\r[FTS] LEAVE  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			}
		}
	#else
		EventID = data[EventNum * FTS_EVENT_SIZE_V15];
		/* Event for 2D - protocol of china project */
		if ((EventID >= 0x3) && (EventID <= 0x5))
		{
//			TouchCount = (data[1 + EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0F;
			TouchID = data[1 + EventNum * FTS_EVENT_SIZE_V15] & 0x0F;
			x = (data[2 + EventNum * FTS_EVENT_SIZE_V15] << 4) + ((data[4 + EventNum * FTS_EVENT_SIZE_V15] >> 4) & 0x0f);
			y = (data[3 + EventNum * FTS_EVENT_SIZE_V15] << 4) + (data[4 + EventNum * FTS_EVENT_SIZE_V15] & 0x0F);
			z = data[5 + EventNum * FTS_EVENT_SIZE_V15];

			switch (EventID)
			{
			case	EVENTID_ENTER_POINTER_V15:
				printf("\n\r[FTS] ENTER  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVENTID_MOTION_POINTER_V15:
				printf("\n\r[FTS] MOTION : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVENTID_LEAVE_POINTER_V15 :
				printf("\n\r[FTS] LEAVE  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			}
		}
	#endif
	}

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Called by the ISR or the kernel when occurs an interrupt.
  * @note	This function handles the acquisition of finger data.
  * @param  None
  * @retval None
  */
int fts_event_handler_v15(void)
{
#if	defined(FTSD3_V15)
	uint8_t		regAdd[4] = {0xb6, 0x00, 0x23, READ_ALL_EVENT_V15};
#else
	uint8_t		regAdd[4] = {0xb6, 0x00, 0x45, READ_ALL_EVENT_V15};
#endif
	uint16_t	evtcount = 0;
	uint8_t		data[FTS_EVENT_SIZE_V15 * FTS_FIFO_MAX_V15];

	FUNC_BEGIN();
	fts_read_reg_v15(&regAdd[0], 3, (uint8_t *) &evtcount, 2);
#if	defined(FTSD3_V15)
	evtcount = evtcount >> 8;
	evtcount = evtcount / 2;
#else
	evtcount = evtcount >> 10;
#endif

	if (evtcount > FTS_FIFO_MAX_V15)
		evtcount = FTS_FIFO_MAX_V15;

	if (evtcount > 0) {
		memset(data, 0x0, FTS_EVENT_SIZE_V15 * evtcount);
		fts_read_reg_v15(&regAdd[3], 1, data, FTS_EVENT_SIZE_V15 * evtcount);
	}

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Turn off the auto-tune protection
  * @param  None
  * @retval None
  */
//void fts_auto_protection_off_v15()
int fts_auto_protection_off_v15()
{
	uint8_t regAdd[8];
	
	FUNC_BEGIN();
	regAdd[0] = 0xB0;	regAdd[1] = 0x03;	regAdd[2] = 0x60;	regAdd[3] = 0x00;
	if(fts_write_reg_v15(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(1);

	FUNC_END();
	return	0;
}

/**
  * @brief  To keep active mode for getting data.
  * @param  None
  * @retval None
  */
int fts_keep_active_mode_v15() //tt
{
    uint8_t regAdd[8];

	FUNC_BEGIN();
    /* New Command for keeping active mode for China projects */
    regAdd[0] = 0xC1;       regAdd[1] = 0x00;       regAdd[2] = 0x00;   regAdd[3] = 0x01;
    if(fts_write_reg_v15(&regAdd[0], 4))
	{
		FUNC_END();
        return  I2C_ERR;
	}
    fts_delay_v15(1);

	FUNC_END();
    return 0;
}
#if !defined(FTSD3_V15)

/**
  * @brief  Check the flash status is busy or not.
  * @param  None
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int fts_fw_wait_for_flash_ready_v15()
{
	uint8_t regAdd;
	uint8_t buf[3];
	int 	retry = 0;

	FUNC_BEGIN();
	regAdd = FTS_CMD_READ_FLASH_STAT_V15;
	retry = 0;
	while (1)
	{
		fts_read_reg_v15(&regAdd, 1, (uint8_t *) buf, FTS_EVENT_SIZE_V15);
		if ((buf[0] & 0x01) == 0)
			break;
		
		if (retry++ > 300)
		{
			FUNC_END();
			return -1;
		}

		fts_delay_v15(10);
	}
	
	FUNC_END();
	return	0;
}

/**
  * @brief  Unlock the flash.
  * @param  None
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int fw_unlock_v15(void)
{
	int 	ret = 0;
	uint8_t regAdd[4] = {0xF7, 0x74, 0x45, 0x0};

	FUNC_BEGIN();
	ret = fts_write_reg_v15(&regAdd[0], 4);

	if (ret <= 0)
		printf("\n\r[fw_unlock] fw_erase error \n");
	else
		printf("\n\r[fw_unlock] Flash Unlocked\n");
	
	fts_delay_v15(1);
	
	FUNC_END();
	return	ret;
}

#endif

#if defined(FTSD3_V15)

/**
  * @brief  Convert 4 bytes of unsigned char to 1 byte of unsigned long
  * @param  src		: A pointer of source data (unsigned char)
  * @retval Return 1 byte of unsigned long converted.
  */
uint32_t convU8toU32_v15(uint8_t *src)
{
	uint32_t	tmpData;

	tmpData = (uint32_t) (((src[3] & 0xFF) << 24) + ((src[2] & 0xFF) << 16) + ((src[1] & 0xFF) << 8) + (src[0] & 0xFF));

	return	tmpData;
}

/**
  * @brief  Parsing the header of firmware binary file
  * @param  data		: A pointer of binary file
  * @param	fw_size		: Size of binary file
  * @param	fw_header	: A pointer of header parsing
  * @param	keep_cx		: Always '0'
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int parseBinFile_v15(uint8_t *data, int fw_size, FW_FTB_HEADER_V15 *fw_header, int keep_cx)
{
	int			dimension, index;
	uint32_t	temp;
	int			file_type;

	FUNC_BEGIN();
	/* start the parsing */
	index = 0;
	fw_header->signature = convU8toU32_v15(&data[index]);
	if (fw_header->signature == FW_HEADER_FTB_SIGNATURE_V15)
	{
		printf("\n\r[parseBinFile] FW Signature - ftb file");
		file_type = BIN_FTB_V15;
	}
	else
	{
		printf("\n\r[parseBinFile] FW Signature - ftsxxx file. %08X", fw_header->signature);
		file_type = BIN_FTS256_V15;
		FUNC_END();
		return	file_type;
	}

	index += FW_BYTES_ALLIGN_V15;
	fw_header->ftb_ver = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->target = convU8toU32_v15(&data[index]);
	if (fw_header->target != 0x00007036)
	{
		printf("\n\r[parseBinFile] Wrong target version %08X ... ERROR", fw_header->target);
		FUNC_END();
		return	FALSE;
	}

	index += FW_BYTES_ALLIGN_V15;
	fw_header->fw_id = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->fw_ver = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->cfg_id = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->cfg_ver = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15 * 4;

	fw_header->ext_ver = convU8toU32_v15(&data[index]);
	printf("\n\r[parseBinFile] Version : External = %04X, FW = %04X, CFG = %04X", fw_header->ext_ver, fw_header->fw_ver, fw_header->cfg_ver);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->sec0_size = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->sec1_size = convU8toU32_v15(&data[index]);
	printf("\n\r[parseBinFile] sec0_size = %08X (%d bytes), sec1_size = %08X (%d bytes), ", fw_header->sec0_size, fw_header->sec0_size, fw_header->sec1_size, fw_header->sec1_size);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->sec2_size = convU8toU32_v15(&data[index]);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->sec3_size = convU8toU32_v15(&data[index]);
	printf("\n\r[parseBinFile] sec2_size = %08X (%d bytes), sec3_size = %08X (%d bytes)", fw_header->sec2_size, fw_header->sec2_size, fw_header->sec3_size, fw_header->sec3_size);

	index += FW_BYTES_ALLIGN_V15;
	fw_header->hdr_crc = convU8toU32_v15(&data[index]);

    if (!keep_cx)
    {
    	dimension = fw_header->sec0_size + fw_header->sec1_size + fw_header->sec2_size + fw_header->sec3_size;
    	temp = fw_size;
    }
    else
    {
    	//sec2 may contain cx data (future implementation) sec3 atm not used
		dimension = fw_header->sec0_size + fw_header->sec1_size;
		temp = fw_size - fw_header->sec2_size - fw_header->sec3_size;
    }

    if (dimension + FW_HEADER_SIZE_V15 + FW_BYTES_ALLIGN_V15 != temp)
    {
    	printf("\n\r[parseBinFile] Read only %d instead of %d... ERROR", fw_size, dimension + FW_HEADER_SIZE_V15 + FW_BYTES_ALLIGN_V15);
		FUNC_END();
    	return	FALSE;
    }

	FUNC_END();
	return	file_type;
}

/**
  * @brief  Check status of flash
  * @param  type
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int wait_for_flash_ready_v15(uint8_t type)
{
    uint8_t	cmd[2] = {FLASH_CMD_READ_REGISTER_V15, type};
    uint8_t	readData;
    int		i, res = -1;

	FUNC_BEGIN();
    printf("\n\r[wait_for_flash_ready] Waiting for flash ready");
    for (i = 0; i < 1000 && res != 0; i++)
    {
    	if(fts_read_reg_v15(cmd, sizeof(cmd), &readData, 1))
		{
			FUNC_END();
			return	I2C_ERR;
		}
    	res = readData & 0x80;
        fts_delay_v15(50);
    }

    if (i >= 1000 && res != 0)
    {
    	printf("\n\r[wait_for_flash_ready] Wait for flash TIMEOUT! ERROR");
		FUNC_END();
        return	FALSE;
    }

    printf("\n\r[wait_for_flash_ready] Flash READY!");
	FUNC_END();
    return	TRUE;
}

/**
  * @brief  dma operation.
  * @param  None
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int start_flash_dma_v15()
{
    int		status;
    uint8_t	cmd[3] = {FLASH_CMD_WRITE_REGISTER_V15, FLASH_DMA_CODE0_V15, FLASH_DMA_CODE1_V15};

	FUNC_BEGIN();
    printf("\n\r[start_flash_dma] Command flash DMA ...");
    if(fts_write_reg_v15(cmd, sizeof(cmd)))
	{
		FUNC_END();
		return I2C_ERR;
	}

    status = wait_for_flash_ready_v15(FLASH_DMA_CODE0_V15);

    if (status != TRUE)
    {
    	printf("\n\r[start_flash_dma] start_flash_dma: ERROR");
		FUNC_END();
        return	FALSE;
    }
    printf("\n\r[start_flash_dma] flash DMA DONE!");

	FUNC_END();
    return	TRUE;
}

/**
  * @brief  Fill the flash
  * @param  address		: Start address to fill
  * @param	data		: A pointer of binary file
  * @param	size		: Size of data to fill
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int fillFlash_v15(uint32_t address, uint8_t *data, int size)
{
    int		remaining;
    int		toWrite = 0;
    int		byteBlock = 0;
    int		wheel = 0;
    uint32_t	addr = 0;
    int		res;
    int		delta;

    uint8_t		buff[DMA_CHUNK_V15 + 3] = {0};

	FUNC_BEGIN();
    remaining = size;
    while (remaining > 0)
    {
		byteBlock = 0;
		addr =0;
		printf("\n\r[fillFlash] [%d] Write data to memory.", wheel);
        while (byteBlock < FLASH_CHUNK_V15 && remaining > 0)
        {
            buff[0] = FLASH_CMD_WRITE_64K_V15;
            if (remaining >= DMA_CHUNK_V15)
            {
                if ((byteBlock + DMA_CHUNK_V15) <= FLASH_CHUNK_V15)
                {
                    toWrite = DMA_CHUNK_V15;
                    remaining -= DMA_CHUNK_V15;
                    byteBlock += DMA_CHUNK_V15;
                }
                else
                {
                    delta = FLASH_CHUNK_V15 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }
            else
            {
                if ((byteBlock + remaining) <= FLASH_CHUNK_V15)
                {
                    toWrite = remaining;
                    byteBlock += remaining;
                    remaining = 0;

                }
                else
                {
                    delta = FLASH_CHUNK_V15 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }

            buff[1] = (uint8_t) ((addr & 0x0000FF00) >> 8);
            buff[2] = (uint8_t) (addr & 0x000000FF);
            memcpy(&buff[3], data, toWrite);
            if(fts_write_reg_v15(buff, 3 + toWrite))
			{
				FUNC_END();
				return I2C_ERR;
			}

            addr += toWrite;
            data += toWrite;
        }

        //configuring the DMA
        printf("\n\r[fillFlash] [%d] Configure DMA", wheel);
        byteBlock = byteBlock / 4 - 1;

        buff[0] = FLASH_CMD_WRITE_REGISTER_V15;
        buff[1] = FLASH_DMA_CONFIG_V15;
        buff[2] = 0x00;
        buff[3] = 0x00;

        addr = address + ((wheel * FLASH_CHUNK_V15)/4);
        buff[4] = (uint8_t) ((addr & 0x000000FF));
        buff[5] = (uint8_t) ((addr & 0x0000FF00) >> 8);
        buff[6] = (uint8_t) (byteBlock & 0x000000FF);
        buff[7] = (uint8_t) ((byteBlock & 0x0000FF00)>> 8);
        buff[8] = 0x00;

        if(fts_write_reg_v15(buff, 9))
		{
			FUNC_END();
			return	I2C_ERR;
		}
        fts_delay_v15(10);

        printf("\n\r[fillFlash] [%d] Start flash DMA", wheel);
        res = start_flash_dma_v15();
        if (res < TRUE) {
        	printf("\n\r[fillFlash] Error during flashing DMA! ERROR");
			FUNC_END();
            return	FALSE;
        }
        printf("\n\r[fillFlash] [%d] DMA done", wheel);

        wheel++;
    }
	FUNC_END();
    return	TRUE;
}

#endif

/**
  * @brief  Download a firmware to flash.
  * @param  pFilename	: A pointer of buffer for a read file.
  * @param	block_type
  * @param	system_reset
  * @retval None
  */
int fw_download_v15(uint8_t *pFilename, FW_FTB_HEADER_V15 *fw_Header, int8_t block_type)
{
#if	defined(FTSD3_V15)
	uint32_t	FTS_TOTAL_SIZE = (256 * 1024);	// Total 256kB
	int			HEADER_DATA_SIZE = 32;

	int			res;
	uint8_t		regAdd[8] = {0};

	FUNC_BEGIN();
	//==================== System reset ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x52;		regAdd[2] = 0x34;
	if(fts_write_reg_v15(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(30);

	//==================== Warm Boot ====================
	regAdd[0] = 0xB6;	regAdd[1] = (ADDR_WARM_BOOT_V15 >> 8) & 0xFF;	regAdd[2] = ADDR_WARM_BOOT_V15 & 0xFF;	regAdd[3] = WARM_BOOT_VALUE_V15;
	if(fts_write_reg_v15(&regAdd[0],4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(30);

	//==================== Unlock Flash ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x74;		regAdd[2] = 0x45;
	if(fts_write_reg_v15(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(30);

	//==================== Unlock Erase & Programming Operation ====================
	regAdd[0] = 0xFA;		regAdd[1] = 0x72;		regAdd[2] = 0x03;
	if(fts_write_reg_v15(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(30);

	//==================== Erase full Flash ====================
	regAdd[0] = 0xFA;		regAdd[1] = 0x02;		regAdd[2] = 0xC0;
	if(fts_write_reg_v15(&regAdd[0],3))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v15(200);

	//========================== Write to FLASH ==========================
	if (block_type == BIN_FTB_V15)
	{
		printf("\n\r[fw_download] Start sec0 program");
		res = fillFlash_v15(FLASH_ADDR_CODE_V15, &pFilename[FW_HEADER_SIZE_V15], fw_Header->sec0_size);
		if (res != TRUE)
		{
			printf("\n\r[fw_download] Error - load sec0 program");
			FUNC_END();
			return	FALSE;
		}
		printf("\n\r[fw_download] load sec0 program DONE!");
		printf("\n\r[fw_download] Start sec1 program");
		res = fillFlash_v15(FLASH_ADDR_CONFIG_V15, &pFilename[FW_HEADER_SIZE_V15 + fw_Header->sec0_size], fw_Header->sec1_size);
		if (res != TRUE)
		{
			printf("\n\r[fw_download] Error - load sec1 program");
			FUNC_END();
			return	FALSE;
		}
		printf("\n\r[fw_download] load sec1 program DONE!");

		printf("\n\r[fw_download] Flash burn COMPLETED!");
	}
	else
	{
		printf("\n\r[fw_download] Start firmware downloading");
		res = fillFlash_v15(FLASH_ADDR_CODE_V15, &pFilename[HEADER_DATA_SIZE], FTS_TOTAL_SIZE);
		if (res != TRUE)
		{
			printf("\n\r[fw_download] Error - load sec0 program");
			FUNC_END();
			return	FALSE;
		}
	}

	//==================== System reset ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x52;		regAdd[2] = 0x34;
	if(fts_write_reg_v15(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
    if (fts_cmd_completion_check_v15(0x10, 0x00, 0x00) < TRUE)
	{
		printf("\n\r[fw_download] Error - System Reset FAILED");
		FUNC_END();
		return	FALSE;
	}

#else	//FTSD2

	uint32_t	size = 0;
	uint_t		section = 0;
	uint_t		offset = 0;
	uint8_t		buf[WRITE_CHUNK_SIZE_V15 + 2] = {0};
	uint8_t		*pData;
	uint8_t		regAdd[8] = {0};
	uint_t		*parsing_size;
	int			rtn = 0;
	int			Result = -1;

	fts_systemreset_v15();
	fts_delay_v15(50);

	if (fts_read_chip_id_v15() == FALSE)
	{
		FUNC_END();
		return	FALSE;
	}

	//copy to PRAM
	switch (block_type)
	{
		case	BINARY:
			size = FTS_BIN_SIZE_V15;
			offset = 0;
			break;
		default:
			printf("Error !!! This is not block type\n");
			break;
	}

	//Check busy flash
	fts_fw_wait_for_flash_ready_v15();

	pData = pFilename;
	
	//FTS_CMD_UNLOCK_FLASH
	if (fw_unlock_v15() <= 0)
	{
		FUNC_END();
		return	FALSE;
	}

	if (block_type == BINARY_V15)
	{
		for (section = 0; section < ( size / WRITE_BURN_SIZE_V15) ; section++)
		{
			buf[0] = FTS_CMD_WRITE_PRAM_V15 + (((section * WRITE_BURN_SIZE_V15) >> 16) & 0x0f);
			buf[1] = ((section * WRITE_BURN_SIZE_V15) >> 8) & 0xFF;
			buf[2] = (section * WRITE_BURN_SIZE_V15) & 0xFF;
			memcpy(&buf[3], &pData[section * WRITE_BURN_SIZE_V15 + sizeof(struct fts64_header_v15)], WRITE_BURN_SIZE_V15);
			if(fts_write_reg_v15(&buf[0], WRITE_BURN_SIZE_V15 + 3))
			{
				FUNC_END();
				return I2C_ERR;
			}
			fts_delay_v15(1);		// adding delay
		}
	}

	//Erase Program Flash
	fts_command_v15(FTS_CMD_ERASE_PROG_FLASH_V15);
	fts_delay_v15(200);

	//Check busy Flash
	fts_fw_wait_for_flash_ready_v15();

	//Burn Program Flash
	fts_command_v15(FTS_CMD_BURN_PROG_FLASH_V15);
	fts_delay_v15(200);

	//Check busy Flash
	fts_fw_wait_for_flash_ready_v15();

	//Reset FTS
	fts_systemreset_v15();

	fts_delay_v15(50);
	
#endif

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Manage flash update procedure.
  * @param  force		: Always '0'
  * @param	keep_cx		: Always '0'
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
#ifdef  FTS_SUPPORT_FW_DOWNLOAD_V15 //modify khl 171023

int flashProcedure_v15(int force, int keep_cx)
{
	uint8_t			*pFilename = NULL;
	int				fw_size;
	int				status, fw_type;

	FW_FTB_HEADER_V15	fw_ftbHeader;

	FUNC_BEGIN();
	/* A pointer and size of buffer for binary file */
	pFilename = fw_bin_data;
	fw_size = sizeof(fw_bin_data);
	printf("\n\r[flashProcedure] Firmware size : %d", fw_size);

	fw_type = parseBinFile_v15(pFilename, fw_size, &fw_ftbHeader, keep_cx);
	if (fw_type == FALSE)
	{
		printf("\n\r[flashProcedure] Error - FW is not appreciate");
		FUNC_END();
		return	FALSE;
	}

	status = fw_download_v15(pFilename, &fw_ftbHeader, fw_type);
	if (status == FALSE)
	{
		printf("\n\r[flashProcedure] Error - Firmware update is not completed.");
		FUNC_END();
		return	FALSE;
	}

	printf("\n\r[flashProcedure] Firmware update is done successfully.");

	FUNC_END();
	return	TRUE;
}

#endif

/**
  * @brief  Get the data for mutual sensing.
  * @param  tx_num, rx_num	Number of channel
  * @param	type_addr		A type of address to get data data
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
uint8_t fts_get_ms_data_v15(int tx_num, int rx_num, uint16_t *ms_data, uint8_t type_addr)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[FTS_RX_LENGTH_V15 * 2 + 1] = {0};
	uint16_t	offsetAddr = 0, startAddr = 0, endAddr = 0, writeAddr = 0;
	int			col_cnt =0, row_cnt = 0;

	FUNC_BEGIN();
	regAdd[2] = type_addr;
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], FTS_EVENT_SIZE_V15))
	{
		FUNC_BEGIN();
		return I2C_ERR;
	}

	offsetAddr = tempBuf[DOFFSET_V15] +  (tempBuf[DOFFSET_V15 + 1] << 8);

	if (type_addr == FTS_MS_KEY_RAW_ADDR_V15)
		startAddr = offsetAddr;
	else
		startAddr = offsetAddr + rx_num * 2;
	endAddr = startAddr + tx_num * rx_num * 2;

	fts_delay_v15(10);
	memset(&tempBuf[0], 0x0, sizeof(tempBuf));
	
	for (writeAddr = startAddr; writeAddr < endAddr; col_cnt++, writeAddr += ((rx_num * 2)))
	{
		regAdd[1] = (writeAddr >> 8) & 0xFF;
		regAdd[2] = writeAddr & 0xFF;

		if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], rx_num * 2 + 1))
		{
			FUNC_END();
			return I2C_ERR;
		}
		for (row_cnt = 0; row_cnt < rx_num; row_cnt++)
		{
			ms_data[col_cnt * rx_num + row_cnt] = tempBuf[row_cnt * 2 + DOFFSET_V15] + (tempBuf[row_cnt * 2 + DOFFSET_V15 + 1] << 8);
		}
	}

	FUNC_END();
	return 0;
}

/**
  * @brief  Get characteristic data related to mutual.
  * @param  sel_data	type of characteristic data
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int fts_get_ms_cx_data_v15(uint8_t *cx_data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[FTS_RX_LENGTH_V15 + DOFFSET_V15];
	uint8_t		tuning_ver, cx1_val;
	uint16_t	node_startAddr, temp_addr;
	int			i, j, tx_num, rx_num;

	FUNC_BEGIN();
	/* Request to read compensation data */
	regAdd[0] = 0xB8;
	regAdd[1] = (uint8_t) sel_data;
	regAdd[2] = (uint8_t) (sel_data >> 8);
	if(fts_write_reg_v15(regAdd, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_cmd_completion_check_v15(EVENTID_COMPENSATION_READ_V15, regAdd[1], regAdd[2]);

	/* Read the offset address for compensation data */
	regAdd[0] = 0xD0; regAdd[1] = 0x00; regAdd[2] = 0x50;
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	node_startAddr = tempBuf[0 + DOFFSET_V15] + (tempBuf[1 + DOFFSET_V15] << 8) + FTS_COMP_NODEDATA_OFFSET_V15;
	fts_delay_v15(1);

	/* Read global and compensation header */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V15]; regAdd[2] = tempBuf[0 + DOFFSET_V15];
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, 16 + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}
	tx_num = tempBuf[4 + DOFFSET_V15]; rx_num = tempBuf[5 + DOFFSET_V15];
	printf("\n\r[FTS] tx : %d, rx : %d", tx_num, rx_num);
	tuning_ver = tempBuf[8 + DOFFSET_V15]; cx1_val = tempBuf[9 + DOFFSET_V15];
	printf("\n\r[FTS] tuning version : %d, Cx1 : %d", tuning_ver, cx1_val);
	fts_delay_v15(1);

	/* Read node compensation data */
	for (j = 0; j < tx_num; j++)
	{
		temp_addr = node_startAddr + (rx_num * j);
		regAdd[0] = 0xD0; regAdd[1] = (uint8_t) (temp_addr >> 8); regAdd[2] = (uint8_t) temp_addr;
		if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, rx_num + DOFFSET_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		for(i = 0; i < rx_num; i++)
		{
		#ifdef LIMIT_MUTUAL_TOTAL_CX_V15
			cx_data[j * rx_num + i] = (cx1_val * MUTUAL_CX1_COEFF_V15) + tempBuf[i + DOFFSET_V15];
		#else
			cx_data[j * rx_num + i] = tempBuf[i + DOFFSET_V15];
		#endif
		}
	}
	#ifdef LIMIT_MUTUAL_TOTAL_CX_V15
	printf("%s : cx_data = (cx1 * %d) + cx2 \n",__func__,MUTUAL_CX1_COEFF_V15);
	#endif

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Get the data for self sensing.
  * @param  tx_num, rx_num	Number of channel
  * @param	type_addr		A type of address to get data data
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
uint8_t fts_get_ss_data_v15(int tx_num, int rx_num, uint16_t *ss_f_data, uint16_t *ss_s_data, uint8_t type_addr)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[FTS_RX_LENGTH_V15 * 2 + 1] = {0};
	uint_t		offsetAddr = 0;
	int			count=0;

	FUNC_BEGIN();
	regAdd[1] = 0x00;
	regAdd[2] = type_addr;
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}

	offsetAddr = tempBuf[DOFFSET_V15] +  (tempBuf[DOFFSET_V15 + 1] << 8);
	regAdd[1] = (offsetAddr >> 8) & 0xFF;
	regAdd[2] = offsetAddr & 0xFF;
	
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], tx_num * 2 + 1))
	{
		FUNC_END();
		return I2C_ERR;
	}
	for (count = 0; count < tx_num; count++)
	{
		ss_f_data[count] = tempBuf[count * 2 + DOFFSET_V15] + (tempBuf[count * 2 + DOFFSET_V15 + 1] << 8);
	}

	regAdd[1] = 0x00;
	regAdd[2] = type_addr + 2;
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}

	offsetAddr = tempBuf[DOFFSET_V15] +  (tempBuf[DOFFSET_V15+1] << 8);
	regAdd[1] = (offsetAddr >> 8) & 0xFF;
	regAdd[2] = offsetAddr & 0xFF;
		
	if(fts_read_reg_v15(regAdd, 3, &tempBuf[0], rx_num * 2 + 1))
	{
		FUNC_END();
		return I2C_ERR;
	}
	for (count = 0; count < rx_num; count++)
	{
		ss_s_data[count] = tempBuf[count * 2 + DOFFSET_V15] + (tempBuf[count * 2 + DOFFSET_V15 + 1] << 8);
	}
	
	FUNC_END();
	return 0;
}

/**
  * @brief  Get characteristic data related to self sensing.
  * @param  xx_x_data	buffers of compensation data
  * @param  sel_data	type of characteristic data
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int fts_get_ss_ix_data_v15(uint8_t *tx_ix1, uint8_t *ix_tx_data, uint8_t *rx_ix1, uint8_t *ix_rx_data, uint8_t *cx_tx_data, uint8_t *cx_rx_data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[FTS_RX_LENGTH_V15 + DOFFSET_V15];
	uint8_t		tuning_ver, ix1_tx = 0, ix1_rx = 0;
	uint16_t	ix2_tx_addr, ix2_rx_addr, cx2_tx_addr, cx2_rx_addr;
	int			tx_num, rx_num;

	FUNC_BEGIN();
	/* Request to read compensation data */
	regAdd[0] = 0xB8;
	regAdd[1] = (uint8_t) sel_data;
	regAdd[2] = (uint8_t) (sel_data >> 8);
	fts_write_reg_v15(regAdd, 3);
	fts_cmd_completion_check_v15(EVENTID_COMPENSATION_READ_V15, regAdd[1], regAdd[2]);

	/* Read the offset address for mutual compensation data */
	regAdd[0] = 0xD0; regAdd[1] = 0x00; regAdd[2] = 0x50;
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	ix2_tx_addr = tempBuf[0 + DOFFSET_V15] + (tempBuf[1 + DOFFSET_V15] << 8) + FTS_COMP_NODEDATA_OFFSET_V15;
	fts_delay_v15(1);

	/* Read global and compensation header */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V15]; regAdd[2] = tempBuf[0 + DOFFSET_V15];
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, 16 + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}

	tx_num = tempBuf[4 + DOFFSET_V15]; rx_num = tempBuf[5 + DOFFSET_V15];
	printf("\n\r[FTS] tx : %d, rx : %d", tx_num, rx_num);
	ix2_rx_addr = ix2_tx_addr + tx_num;
	cx2_tx_addr = ix2_rx_addr + rx_num;
	cx2_rx_addr = cx2_tx_addr + tx_num;

	tuning_ver = tempBuf[8 + DOFFSET_V15];
	ix1_tx = tempBuf[9 + DOFFSET_V15]; ix1_rx = tempBuf[10 + DOFFSET_V15];
	printf("\n\r[FTS] tuning version : %d, Ix1 force : %d, Ix1 sense : %d", tuning_ver, ix1_tx, ix1_rx);

    *tx_ix1 = ix1_tx;
printf("->ix1_tx : %d \n",ix1_tx);
    *rx_ix1 = ix1_rx;
printf("->ix1_rx : %d \n",ix1_rx);

	/* Read data */
	regAdd[1] = (ix2_tx_addr >> 8) & 0xFF;
	regAdd[2] = ix2_tx_addr & 0xFF;
	if(fts_read_reg_v15(regAdd, 3, tempBuf, tx_num + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(ix_tx_data, tempBuf + 1, tx_num);
	fts_delay_v15(1);

	regAdd[1] = (ix2_rx_addr >> 8) & 0xFF;
	regAdd[2] = ix2_rx_addr & 0xFF;
	if(fts_read_reg_v15(regAdd, 3, tempBuf, rx_num + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(ix_rx_data, tempBuf + 1, rx_num);
	fts_delay_v15(1);
	
	regAdd[1] = (cx2_tx_addr >> 8) & 0xFF;
	regAdd[2] = cx2_tx_addr & 0xFF;
	if(fts_read_reg_v15(regAdd, 3, tempBuf, tx_num + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(cx_tx_data, tempBuf + 1, tx_num);
	fts_delay_v15(1);

	regAdd[1] = (cx2_rx_addr >> 8) & 0xFF;
	regAdd[2] = cx2_rx_addr & 0xFF;
	if(fts_read_reg_v15(regAdd, 3, tempBuf, rx_num + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(cx_rx_data, tempBuf + 1, rx_num);
	fts_delay_v15(1);

	FUNC_END();
	return	TRUE;
}

#if	defined (FTS_SUPPORT_FORCE_FSR_V15)

/**
  * @brief  Get the data for force sensor.
  * @param	mode	type of data like raw or strength
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int fts_get_force_data_v15(int tx_num, int rx_num, uint16_t *data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[(FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15) * 2 + DOFFSET_V15];
	int			i, j, tmpIdx;

	FUNC_BEGIN();
	fts_delay_v15(10);

	/* Read the offset address of selected data */
	regAdd[0] = 0xD0; regAdd[1] = (uint8_t) (sel_data >> 8); regAdd[2] = (uint8_t) sel_data;
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(1);

	/* Read the selected data from offset address */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V15]; regAdd[2] = tempBuf[0 + DOFFSET_V15];
	if(fts_read_reg_v15(&regAdd[0], 3, tempBuf, (FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15) * 2 + DOFFSET_V15))
	{
		FUNC_END();
		return I2C_ERR;
	}

	/* Read node compensation data */
	for (j = 0; j < tx_num; j++)
	{
		tmpIdx = j * rx_num;
		for (i = 0; i < rx_num; i++)
		{
			data[tmpIdx + i] = (uint16_t) tempBuf[(tmpIdx + i) * 2 + DOFFSET_V15] + (uint16_t) (tempBuf[(tmpIdx + i) * 2 + 1 + DOFFSET_V15] << 8);
		}
	}
	fts_delay_v15(10);

	FUNC_END();
	return	TRUE;
}

#endif

/**
  * @brief  Check hw reset pin when jig is able to control this pin.
  * @param	None
  * @retval None
  */
int fts_hw_reset_pin_check_v15()
{
	uint8_t	buf[8] = {0,};
	uint8_t	cnt = 20;

	FUNC_BEGIN();
	fts_command_v15(FLUSHBUFFER_V15);

	/*
	 * Drain down Reset pin to GND for 10ms by jig.
	 */

	while (cnt--)
	{
		fts_delay_v15(5);
		if(fts_read_reg_v15(fts_fifo_addr, 1, &buf[0], FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if (buf[0] == 0x10 && buf[1] == 0x00 && buf[2] == 0x00)
		{
			printf("\n\r[fts_hw_reset_pin_check] OK");
			FUNC_END();
			return	0;
		}
	}

	if (cnt == 0)
		printf("\n\r[fts_hw_reset_pin_check] Error");

	FUNC_END();
	return	-1;
}

/**
  * @brief  Check interrupt pin
  * @param	None
  * @retval None
  */
int fts_interrupt_pin_check_v15()
{
	FUNC_BEGIN();
	fts_systemreset_v15();
	fts_delay_v15(50);
	if(fts_interrupt_control_v15(ENABLE) == I2C_ERR)		// Need to enable interrupt pin before status check
	{
		FUNC_END();
		return I2C_ERR;
	}

	/*
	 * Interrupt pin low check
	 *     If not low, return ERROR.
	 */

	if(fts_command_v15(FLUSHBUFFER_V15) == I2C_ERR)
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(20);

	/*
	 * Interrupt pin high check here
	 *     If not high, return ERROR.
	 */
	FUNC_END();
	return 0;
}

/**
  * @brief  Check ta pin
  * @param	None
  * @retval None
  */
int fts_ta_pin_check_v15(void)
{
	FUNC_BEGIN();
	fts_systemreset_v15();
	fts_delay_v15(50);

	fts_write_feature_v15(ENABLE, FTS_CMD_FEATURE_TA_V15);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_TA_ON_V15, 0x00) <= FALSE)
	{
		vcp_printf("\n\r[fts_ta_pin_check] Error - ON");
		FUNC_END();
		return	FALSE;
	}

	fts_write_feature_v15(DISABLE, FTS_CMD_FEATURE_TA_V15);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_TA_OFF_V15, 0x00) <= FALSE)
	{
		vcp_printf("\n\r[fts_ta_pin_check] Error - OFF");
		FUNC_END();
		return	FALSE;
	}

	vcp_printf("\n\r[fts_ta_pin_check] OK");

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Check event message for ITO open/short test
  * @param	None
  * @retval None
  */
int fts_panel_ito_test_v15(void)
{
	uint8_t	val[8];
	uint_t	cnt = 100;
	int		err_flag = TRUE;
	char	*errortypes[16] = {"No Error", "F open", "S open", "F2G short", "S2G short", "F2V short", "S2V short", "F2F short", "S2S short",
							"F2S short", "FPC F open", "FPC S open", "Key F open", "Key S open", "Reserved", "Reserved"};

	FUNC_BEGIN();
	cnt = 100;
	do	{
		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		fts_delay_v15(1);
	}	while (cnt-- && (val[0] != 0x0F && val[1] != 0x05));	

    if(DEBUG_MODE)
    printf("[fts_panel_ito_test] ITO RESULT : ");

//	printf("\n\r[fts_panel_ito_test] ITO RESULT : ");

	cnt = 100;
	do
	{
		if (val[0] == 0x0F && val[1] == 0x05)
		{
			switch (val[2])
			{
				case	NO_ERROR_V15:
					if (val[3] == 0x00)
					{
						if(DEBUG_MODE)
							printf("ITO open / short test PASS!!\n");
						//printf("ITO open / short test PASS!!\n");
						FUNC_END();
						return	TRUE;
					}
					break;
				case ITO_FORCE_OPEN_V15:
				case ITO_SENSE_OPEN_V15:
				case ITO_FORCE_SHRT_GND_V15:
				case ITO_SENSE_SHRT_GND_V15:
				case ITO_FORCE_SHRT_VCM_V15:
				case ITO_SENSE_SHRT_VCM_V15:
				case ITO_FORCE_SHRT_FORCE_V15:
				case ITO_SENSE_SHRT_SENSE_V15:
				case ITO_F2E_SENSE_V15:
				case ITO_FPC_FORCE_OPEN_V15:
				case ITO_FPC_SENSE_OPEN_V15:
				case ITO_KEY_FORCE_OPEN_V15:
				case ITO_KEY_SENSE_OPEN_V15:
				case ITO_RESERVED0_V15:
				case ITO_RESERVED1_V15:
				case ITO_RESERVED2_V15:
				case ITO_MAX_ERR_REACHED_V15:
					err_flag = FALSE;
					if(DEBUG_MODE)
						printf("ITO open / short test FAIL!! Error Type : %s, Channel : %d", errortypes[val[2]], val[3]);
					break;
			}
		}		
		if(fts_read_reg_v15(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V15))
		{
			FUNC_END();
			return I2C_ERR;
		}
		fts_delay_v15(1);
	}	while (cnt-- && (val[1] != 0x00 && val[2] != 0x00));

	fts_delay_v15(10);
	
	fts_systemreset_v15();
	fts_delay_v15(50);

	FUNC_END();
	return	err_flag;
}

/**
  * @brief  Proceed the ITO open/short test
  * @param	None
  * @retval None
  */
//void fts_panel_short_test()
int fts_panel_short_test_v15()
{
	uint8_t regAdd[8] = {0,};
	int ret = TRUE;

	FUNC_BEGIN();
	/* HW ITO OPEN/SHORT TEST */
#ifdef	FTS_SUPPORT_ITOTEST_HW_V15
	fts_systemreset_v15();
	fts_delay_v15(50);

	fts_command_v15(SLEEPOUT_V15);
	fts_delay_v15(10);

	fts_command_v15(FLUSHBUFFER_V15);
	fts_delay_v15(1);

	// HW short test
	regAdd[0] = 0xA7;
	regAdd[1] = 0x01;
	regAdd[2] = 0x00;
	if(fts_write_reg_v15(&regAdd[0], 3))		// 2 byte
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(200);

	if(fts_panel_ito_test_v15()<TRUE)
		ret = FALSE;
#endif

	/* SW ITO OPEN/SHORT TEST */
#ifdef	FTS_SUPPORT_ITOTEST_SW_V15
	fts_systemreset_v15();
	fts_delay_v15(50);

	fts_command_v15(SLEEPOUT_V15);
	fts_delay_v15(10);

	fts_command_v15(FLUSHBUFFER_V15);
	fts_delay_v15(1);

	regAdd[0] = 0xA7;
	if(fts_write_reg_v15(&regAdd[0], 1))		// 1 byte
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v15(200);

	if(fts_panel_ito_test_v15() < TRUE)
		ret = FALSE;
#endif

	FUNC_END();
	return	ret;
}

#ifdef	FTS_SUPPORT_FORCE_FSR_V15

/**
  * @brief  To check either some faulty for force sensor (panel) is or not.
  * @param  None
  * @retval None
  */
int fts_force_panel_test_v15(void)
{
	uint8_t		ms_force_cx_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
	uint16_t	ms_force_raw_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
	uint16_t	ms_force_str_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];

	FUNC_BEGIN();
	power_on_v15();
	fts_delay_v15(30);

	fts_systemreset_v15();
	if (fts_cmd_completion_check_v15(0x10, 0x00, 0x00) <= FALSE)
	{
		printf("\n\r[FTS] System Reset FAILED");
		FUNC_END();
		return	FALSE;
	}

	fts_read_chip_id_v15();
	fts_get_versionInfo_v15();

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V15
	flashProcedure_v15(0, 0);
#endif

	fts_interrupt_control_V15(DISABLE);

	fts_command_v15(MUTUAL_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_MS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] Mutual Auto Tune FAILED");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V15
	fts_command_v15(SELF_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_SS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] Self Auto Tune FAILED");
		FUNC_END();
		return	FALSE;
	}
#endif

	/* Get cx value of mutual sensing */
	printf("\n\r[FTS] Force Cx Value");
	fts_get_ms_cx_data_v15(ms_force_cx_buf, FTS_COMP_MS_FORCE_SEL_V15);

	fts_auto_protection_off_v15();
	fts_keep_active_mode_v15();
	fts_delay_v15(10);

	fts_command_v15(SENSEON_V15);
	fts_delay_v15(100);

	/* Get the mutual raw data */
	printf("\n\r[FTS] Force Raw Data");
	fts_get_force_data_v15(FTS_FORCE_TX_LENGTH_V15, FTS_FORCE_RX_LENGTH_V15, ms_force_raw_buf, FTS_FORCE_RAW_ADDR_V15);
	fts_get_force_data_v15(FTS_FORCE_TX_LENGTH_V15, FTS_FORCE_RX_LENGTH_V15, ms_force_str_buf, FTS_FORCE_STR_ADDR_V15);

	FUNC_END();
	return	TRUE;
}

/**
  * @brief  To check a change of strength data when was pressurized on the panel.
  * @param  None
  * @retval None
  */
void fts_check_force_press_v15(void)
{
	uint16_t	ms_force_str_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
	int			nCount = 2;		/* user count */

	FUNC_BEGIN();
	while (nCount--)
	{
		fts_delay_v15(10);
		printf("\n\r[FTS] Force Strength Data");
		fts_get_force_data_v15(FTS_FORCE_TX_LENGTH_V15, FTS_FORCE_RX_LENGTH_V15, ms_force_str_buf, FTS_FORCE_STR_ADDR_V15);
	}
	FUNC_END();
}

/**
  * @brief  Initialize operations with Auto-tune sequence for LGD FSR.
  * 		This function has to be run a time after firmware update.
  * @param  None
  * @retval None
  */
int fts_force_test_proc_v15(void)
{
	FUNC_BEGIN();
	fts_force_panel_test_v15();
	fts_check_force_press_v15();

	FUNC_END();
	return	TRUE;
}

#endif

#ifdef	JIG_SMT_INSPECTION_MACHINE_V15

/**
  * @brief  For Joan model, write some commands for SMT inspection equipment.
  * @param  None
  * @retval None
  */
void fts_write_commands_for_sensor_v15(void)
{
	FUNC_BEGIN();
#if	0
	uint8_t	regAdd[8];

	/* Just added for SMT inspection equipment */
	regAdd[0] = 0xB0;		regAdd[1] = 0x07;		regAdd[2] = 0xB7; 	regAdd[3] = 0x0C;
	fts_write_reg(&regAdd[0], 4);
	fts_delay(5);

	regAdd[0] = 0xB0;		regAdd[1] = 0x07;		regAdd[2] = 0xBF;		regAdd[3] = 0x0C;
	fts_write_reg(&regAdd[0], 4);
	fts_delay(5);
#endif
	FUNC_END();
}

#endif

int fts_do_autotune_v15(void)
{
	FUNC_BEGIN();
#ifdef	JIG_SMT_INSPECTION_MACHINE_V15
	/* Just added for SMT inspection machine */
	fts_write_commands_for_sensor_v15();
#endif

	fts_command_v15(LPOSC_TRIM_V15);
	fts_delay_v15(100);

	fts_command_v15(MUTUAL_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_MS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] FAILED - Mutual Auto Tune");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V15
	fts_command_v15(SELF_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_SS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] FAILED - Self Auto Tune");
		FUNC_END();
		return	FALSE;
	}
#endif

	FUNC_END();
	return	TRUE;
}
////////////////////////////////////////////////////////
/**
  * @brief  To check either some faulty for sensor is or not.
  * @param  None
  * @retval None
  */
int fts_panel_test_v15(int id, unsigned char *uart_buf)
{
	uint8_t		regAdd[8];
	int			i, j;
#ifndef  FTS_SUPPORT_CX_BACKUP_V15
	int 		status;
#endif	/* !FTS_SUPPORT_CX_BACKUP_V15 */

    /****************************************************/
    // Modified by iamozzi...171023 khl
    int         ms_cx_data_Hgap[FTS_TX_LENGTH_V15][FTS_RX_LENGTH_V15 - 1];
    int         ms_cx_data_Vgap[FTS_TX_LENGTH_V15 - 1][FTS_RX_LENGTH_V15];
    /****************************************************/


    /****************************************************/
    // Modified by iamozzi...171023 khl
    // NOTE : Check Size of Array...
    int         ms_hf_cx_data_Hgap[FTS_TX_LENGTH_V15][FTS_RX_LENGTH_V15 - 1];
    int         ms_hf_cx_data_Vgap[FTS_TX_LENGTH_V15 - 1][FTS_RX_LENGTH_V15];
    /****************************************************/


	uint16_t    ms_raw_data[FTS_TX_LENGTH_V15 * FTS_RX_LENGTH_V15];
	uint8_t      ix1_tx = 0, ix1_rx = 0;
	int16_t     ms_jitter_data[FTS_TX_LENGTH_V15 * FTS_RX_LENGTH_V15];
	uint16_t	ss_raw_f_buf[FTS_TX_LENGTH_V15], ss_raw_s_buf[FTS_RX_LENGTH_V15];
	uint8_t		ms_cx_data[FTS_TX_LENGTH_V15 * FTS_RX_LENGTH_V15];
	uint8_t		ss_ix_tx_data[FTS_TX_LENGTH_V15], ss_ix_rx_data[FTS_RX_LENGTH_V15];
	uint8_t		ss_cx_tx_data[FTS_TX_LENGTH_V15], ss_cx_rx_data[FTS_RX_LENGTH_V15];
#ifdef	FTS_SUPPORT_MS_KEY_V15
	uint16_t	ms_key_raw_data[FTS_MSKEY_TX_LENGTH_V15 * FTS_MSKEY_RX_LENGTH_V15];
	uint8_t		ms_key_cx_data[FTS_MSKEY_TX_LENGTH_V15 * FTS_MSKEY_RX_LENGTH_V15];
#endif
#ifdef	FTS_SUPPORT_FORCE_FSR_V15
	uint8_t		ms_force_cx_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
	uint16_t	ms_force_raw_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
	uint16_t	ms_force_str_buf[FTS_FORCE_TX_LENGTH_V15 * FTS_FORCE_RX_LENGTH_V15];
#endif
#ifdef	FTS_SUPPORT_CHK_HIGH_FREQ_V15
	uint8_t		ms_hf_cx_data[FTS_TX_LENGTH_V15 * FTS_RX_LENGTH_V15];
#endif

    /****************************************************/
    // 171023 khl

	int	count_max = 0;
    int count_min = 0;
    int ret = 0;
    unsigned int    ret_state = 0;
    int fstat = 0;
    int ref_max = 0;
    int ref_min = 0;
    int cvt_data[FTS_TX_LENGTH_V15][FTS_RX_LENGTH_V15] = {{0,},};
    int txIdx = 0;
    int rxIdx = 0;
    int index = 0;
	/****************************************************/

	FUNC_BEGIN();
    printf("\n\n< TEST VERSION 15Ver > \n");

#ifdef	FTS_HW_PIN_CHECK_V15
    if(fts_hw_reset_pin_check_v15() == I2C_ERR)
    {
    printf("\n--------------------------------------\n");

        printf("=======> I2C FAIL \n"); //mm
    printf("\n--------------------------------------\n");

        uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
        uart_buf[10] = l_hf_test_mode;
		FUNC_END();
        return I2C_ERR;
    }

	fts_interrupt_pin_check_v15();
	fts_ta_pin_check_v15();
#endif

	fts_systemreset_v15();
    if (fts_cmd_completion_check_v15(EVENTID_CONTROLLER_READY_V15, 0x00, 0x00) < TRUE)
    {
    printf("\n--------------------------------------\n");

        printf("[FTS] FAILED - System Reset\n");
    printf("\n--------------------------------------\n");

        uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
        uart_buf[10] = l_hf_test_mode;
		FUNC_END();
        return  FALSE;
    }
    printf("\n--------------------------------------\n");
	ret = fts_read_chip_id_v15();
	if(ret < TRUE)
	{

        if(ret == I2C_ERR)
        {
            uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
            uart_buf[10] = l_hf_test_mode;
            printf("=======> I2C_FAIL \n");
			FUNC_END();
            return I2C_ERR;
        }
    }
    else
    {
        ret_state |= 0<<TOUCH_STM_PRODUCT_ID;
        printf("=======> PRODUCT ID PASS \n");
    }
    ret = 0;

    ret = fts_get_versionInfo_v15();


    if(ret == I2C_ERR)
    {
        printf("=======> I2C_FAIL \n");
        uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
        uart_buf[10] = l_hf_test_mode;
		FUNC_END();
        return I2C_ERR;
    }
    else
    {
        if(!(ret & 0x1))
        {
            ret_state |= (1<<TOUCH_STM_FW_VER);
            printf("=======> FW_VER FAIL \n");
        }
        else
        {
            ret_state |= (0<<TOUCH_STM_FW_VER);
            printf("=======> FW_VER PASS \n");
        }

        if(!(ret & 0x2))
        {
            ret_state |= (1<<TOUCH_STM_CONFIG_VER);
            printf("=======> CONFIG_VER FAIL \n");
        }
        else
        {
            ret_state |= (0<<TOUCH_STM_CONFIG_VER);
            printf("=======> CONFIG_VER PASS \n");
        }

        if(!(ret & 0x4))
        {
            ret_state |= (1<<TOUCH_STM_RELEASE_VER);
            printf("=======> RELEASE_VER FAIL \n");
        }
        else
        {
            ret_state |= (0<<TOUCH_STM_RELEASE_VER);
            printf("=======> RELEASE_VER PASS \n");
        }

    }

    printf("\n--------------------------------------\n");
////////////////////////111
#ifdef  FTS_SUPPORT_FW_DOWNLOAD_V15
	flashProcedure_v15(0, 0);
#endif

	fts_interrupt_control_v15(DISABLE);
///////////////////////////

#ifdef  FTS_SUPPORT_CX_BACKUP_V15
    {
        fts_do_autotune_v15();

    #ifdef JIG_FINAL_INSPECTION_MACHINE_V15
        fts_command_v15(FLASH_CX_BACKUP_V15);
        fts_delay_v15(300);
        if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_CX_BACKUP_DONE_V15, 0x00) <= FALSE)
        {
            printf("\n\r[FTS] FAILED - Cx back-up");
        }
    #endif
    }
#else
    if (fts_get_pure_autotune_flag_v15() == FALSE)
    {
        fts_do_autotune_v15();

        status = fts_run_pure_autotune_v15();
        switch (status)
        {
        case    FAILED: printf("\n\r[FTS] FAILED - Set Pure auto-tune flag");
            break;
        case    NOT_FINAL:
            break;
        case    DONE:       printf("\n\r[FTS] Pure auto-tune is done");
            break;
        default:
            break;
        }
    }
    else
    {
        printf("\n\r[FTS] PAT is already set");
    }
#endif

	fts_delay_v15(10);

    /***************************************************/
    /* LGD_STM_TEST :: Get cx value of mutual sensing */
    /***************************************************/
    printf("\n--------------------------------------\n");
    printf("[ TOTAL-CX ]\n");
#ifdef PRJ_LGMC_V15
    fts_get_ms_cx_data_v15(ms_cx_data, FTS_COMP_MS_LP_SEL_V15);
#else
    fts_get_ms_cx_data_v15(ms_cx_data, FTS_COMP_MS_CX_SEL_V15);
#endif

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // mx_cx_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32


    memset(cvt_data,0,sizeof(cvt_data));
    txIdx = 0;
    rxIdx = 0;
    index = 0;

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V15; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V15; rxIdx++)
        {
            index = txIdx * FTS_RX_LENGTH_V15 + rxIdx;
            cvt_data[txIdx][rxIdx] = ms_cx_data[index];
        }
    }

    /****************************************************/
    /****************************************************/
    count_min = 0;
    count_max = 0;
    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_total_cx[0],l_total_cx[1]);

    /****************************************************/
    // Modified by iamozzi...
    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V15; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V15; rxIdx++)
        {

			if((!totalCx_MAX[0][0])||(!totalCx_MIN[0][0]))
			{
				if (cvt_data[txIdx][rxIdx] < l_total_cx[0])
				{
				    count_min++;
				    if(DEBUG_MODE)
				        printf("-%03d ", cvt_data[txIdx][rxIdx]);
				}
				else if (cvt_data[txIdx][rxIdx] > l_total_cx[1])
				{
				    count_max++;
				    if(DEBUG_MODE)
				        printf("+%03d ", cvt_data[txIdx][rxIdx]);
				}
				else
				{
				    if(DEBUG_MODE)
				        printf("%04d ", cvt_data[txIdx][rxIdx]);
				}
			}
			else
			{
				if (cvt_data[txIdx][rxIdx] > totalCx_MAX[txIdx + 1][rxIdx + 1])
				{
				    count_max++;
				    if(DEBUG_MODE)
				    printf("+%03d ", cvt_data[txIdx][rxIdx]);
				}
				else if (cvt_data[txIdx][rxIdx] < totalCx_MIN[txIdx + 1][rxIdx + 1])
				{
				    count_min++;
				    if(DEBUG_MODE)
				    printf("-%03d ", cvt_data[txIdx][rxIdx]);
				}
				else
				{
				    if(DEBUG_MODE)
				    printf("%04d ", cvt_data[txIdx][rxIdx]);
				}
			}
        }
        if(DEBUG_MODE)
            printf("\n");
    }
    /****************************************************/

    if(count_min)
    {
        printf("=======> TOTAL-CX OPEN FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_TOTAL_CX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> TOTAL-CX SHORT FAIL [%d]\n",count_max);
        ret_state |= 1<< TOUCH_STM_TOTAL_CX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> TOTAL-CX PASS\n");
        ret_state |= 0 << TOUCH_STM_TOTAL_CX;
    }

    fstat = 0;

    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    printf("--------------------------------------\n\n");


    if(DEBUG_MODE)
        printf("[FTS] Getting comp. data of mutual is done!!\n\n");


    printf("--------------------------------------\n");
    printf("[ TOTAL-CX-GAP(H) ]\n");

    count_max = 0;
    count_min = 0;

    //printf("LowLimit = %d / HighLimit = %d\n",);

    /****************************************************/
    // Modified by iamozzi...

    for (i = 0; i < FTS_TX_LENGTH_V15; i++)
    {
        for (j = 0; j < (FTS_RX_LENGTH_V15 - 1); j++)
        {
            // Get Gap...
            if (cvt_data[i][j] > cvt_data[i][j + 1])
            {
                ms_cx_data_Hgap[i][j] = cvt_data[i][j] - cvt_data[i][j + 1];
            }
            else
            {
                ms_cx_data_Hgap[i][j] = cvt_data[i][j + 1] - cvt_data[i][j];
            }

            // Couting Fail...
            if (ms_cx_data_Hgap[i][j] > totalCx_Gap_Rx_MAX[i + 1][j + 1])
            {
                count_max++;
                if(DEBUG_MODE)
                printf("+%03d ", ms_cx_data_Hgap[i][j]);
            }
            else if (ms_cx_data_Hgap[i][j] < totalCx_Gap_Rx_MIN[i + 1][j + 1])
            {
                count_min++;
                if(DEBUG_MODE)
                printf("-%03d ", ms_cx_data_Hgap[i][j]);
            }
            else
            {
                if(DEBUG_MODE)
                printf("%04d ", ms_cx_data_Hgap[i][j]);
            }
        }
        if(DEBUG_MODE)
            printf("\n");
    }
    /****************************************************/


    if(count_max)
    {
        printf("=======> TOTAL-CX-GAP(H) TOO HIGH FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_TOTAL_CX_H;
        fstat = 1;
    }
    if(count_min)
    {
        printf("=======> TOTAL-CX-GAP(H) TOO LOW FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_TOTAL_CX_H;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> TOTAL-CX-GAP(H) PASS\n");
        ret_state |= 0 << TOUCH_STM_TOTAL_CX_H;
    }
    fstat = 0;


    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    printf("--------------------------------------\n\n");


    printf("--------------------------------------\n");
    printf("[ TOTAL-CX-GAP(V) ]\n");

    count_max = 0;
    count_min = 0;

    //printf("LowLimit = %d / HighLimit = %d\n");

    /****************************************************/
    // Modified by iamozzi...
    for (i = 0; i < (FTS_TX_LENGTH_V15 - 1); i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V15; j++)
        {
            // Get Gap...
            if (cvt_data[i][j] > cvt_data[i + 1][j])
            {
                ms_cx_data_Vgap[i][j] = cvt_data[i][j] - cvt_data[i + 1][j];
            }
            else
            {
                ms_cx_data_Vgap[i][j] = cvt_data[i + 1][j] - cvt_data[i][j];
            }

            // Counting Fail...
            if (ms_cx_data_Vgap[i][j] > totalCx_Gap_Tx_MAX[i + 1][j + 1])
            {
                count_max++;
                if(DEBUG_MODE)
                printf("+%03d ", ms_cx_data_Vgap[i][j]);
            }
            else if (ms_cx_data_Vgap[i][j] < totalCx_Gap_Tx_MIN[i + 1][j + 1])
            {
                count_min++;
                if(DEBUG_MODE)
                printf("-%03d ", ms_cx_data_Vgap[i][j]);
            }
            else
            {
                if(DEBUG_MODE)
                printf("%04d ", ms_cx_data_Vgap[i][j]);
            }
        }
        if(DEBUG_MODE)
            printf("\n");
    }
    /****************************************************/

    if(count_max)
    {
        printf("=======> TOTAL-CX-GAP(V) TOO HIGH FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_TOTAL_CX_V;
        fstat = 1;
    }
    if(count_min)
    {
        printf("=======> TOTAL-CX-GAP(V) TOO LOW FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_TOTAL_CX_V;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> TOTAL-CX-GAP(V) PASS\n");
        ret_state |= 0 << TOUCH_STM_TOTAL_CX_V;
    }

    fstat = 0;
    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    printf("--------------------------------------\n\n");

    /***************************************************/
    /* LGD_STM_TEST :: Get ix and cx value of self sensing */
    /***************************************************/
    printf("--------------------------------------\n");
    printf("[ SELF IX & CX ] :  TOTAL TX\n"); //need debuging
    fts_get_ss_ix_data_v15(&ix1_tx, ss_ix_tx_data, &ix1_rx, ss_ix_rx_data, ss_cx_tx_data, ss_cx_rx_data, FTS_COMP_SS_IX_SEL_V15);
    //printf("ss_ix_tx_data : ");
    //printf("TOTAL IX (tx) [%d]--> \n",ix1_tx);
	printf("\n 1. ix1_tx = %u \n",ix1_tx);
    ix1_tx *= 2; //171123
	printf("2. ix1_tx = %u \n",ix1_tx);

    count_max = 0;
    count_min = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_self_ix_tx[0],l_self_ix_tx[1]);

    for(i=0; i<sizeof(ss_ix_tx_data); i++)
    {
        ss_ix_tx_data[i] += ix1_tx;
        if(ss_ix_tx_data[i] < l_self_ix_tx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_ix_tx_data[i]);
        }
        else if(ss_ix_tx_data[i] > l_self_ix_tx[1])
        {
            count_max++;
            if(DEBUG_MODE)
                printf("+%03d ",ss_ix_tx_data[i]);
        }
        else
        {
            if(DEBUG_MODE)
                printf("%04d ",ss_ix_tx_data[i]);
        }
    }
    if(DEBUG_MODE)
        printf("\n");
    if(count_max)
    {
        printf("=======> SELF_IX <TOTAL TX> TOO HIGH FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_IX_TOTAL_TX;
        fstat = 1;
    }

    if(count_min)
    {
        printf("=======> SELF_IX <TOTAL TX> TOO LOW FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_IX_TOTAL_TX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> SELF_IX <TOTAL TX> PASS\n");
        ret_state |= 0 << TOUCH_STM_IX_TOTAL_TX;
    }

    fstat = 0;

    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    printf("--------------------------------------\n\n");

    printf("--------------------------------------\n");
    printf("[ SELF IX & CX ] :  TOTAL RX\n"); // need debuging

    //printf("SELF_IX_TOTAL_TX : %d\n",ss_ix_tx);

    //printf("ss_ix_rx_data : ");
    //printf("TOTAL IX (rx) [%d]--> \n",ix1_rx);
    ix1_rx *= 2;

    count_max = 0;
    count_min = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_self_ix_rx[0],l_self_ix_rx[1]);


    for(i=0; i<sizeof(ss_ix_rx_data); i++)
    {
        ss_ix_rx_data[i] += ix1_rx;
        if(ss_ix_rx_data[i] < l_self_ix_rx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_ix_rx_data[i]);
        }
        else if(ss_ix_rx_data[i] > l_self_ix_rx[1])
        {
            count_max++;
            if(DEBUG_MODE)
                printf("+%03d ",ss_ix_rx_data[i]);
        }
        else
        {
            if(DEBUG_MODE)
                printf("%04d ",ss_ix_rx_data[i]);
        }

    }
    if(DEBUG_MODE)
        printf("\n");
    //printf("SELF_IX_TOTAL_RX : %d\n",ss_ix_rx);

    if(count_max)
    {
        printf("=======> SELF_IX <TOTAL RX> TOO HIGH FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_IX_TOTAL_RX;
        fstat = 1;
    }

    if(count_min)
    {
        printf("=======> SELF_IX <TOTAL RX> TOO LOW FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_IX_TOTAL_RX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> SELF_IX <TOTAL RX> PASS\n");
        ret_state |= 0 << TOUCH_STM_IX_TOTAL_RX;
    }

    fstat = 0;


    printf("MAX / MIN : %d / %d \n",count_max,count_min);
    printf("--------------------------------------\n\n");


    if(DEBUG_MODE)
    {
        printf("ss_cx_tx_data : ");
        for(i=0; i<sizeof(ss_cx_tx_data); i++)
            printf("%d ",ss_cx_tx_data[i]);
        printf("\n");

        printf("ss_cx_rx_data : ");
        for(i=0; i<sizeof(ss_cx_rx_data); i++)
            printf("%d ",ss_cx_rx_data[i]);
        printf("\n");
    }

    if(DEBUG_MODE)
        printf("[FTS] Getting comp. data of self is done !!\n\n");
#ifdef FTS_SUPPORT_MS_KEY_V15
    /* Get cx value of mutual key */
    fts_get_ms_cx_data_v15(ms_key_cx_data, FTS_COMP_MS_KEY_SEL_V15);
    printf("[FTS] Getting comp. data of MSKey is done.\n");
#endif

#ifdef  FTS_SUPPORT_FORCE_FSR_V15
    /* Get cx value of force sensor (FSR) */
    fts_get_ms_cx_data_v15(ms_force_cx_buf, FTS_COMP_MS_FORCE_SEL_V15);
    printf("[FTS] Getting comp. data of fsr is done.\n");
#endif

	fts_write_feature_v15(ENABLE, FTS_CMD_FEATURE_ACTIVEMODE_V15);      // Keep active mode.
    fts_delay_v15(10);

    fts_command_v15(SENSEON_V15);
#ifdef  FTS_SUPPORT_MS_KEY_V15
    fts_command_v15(MSKEY_SENSEON_V15);
#endif
    if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_FCAL_DONE_V15, 0x23) < TRUE)
    {
        printf("[FTS] FAILED - Event(FCAL) is not reached\n");
    }
    fts_delay_v15(300);

    fts_command_v15(SENSEOFF_V15);
#ifdef  FTS_SUPPORT_MS_KEY_V15
    fts_command_v15(MSKEY_SENSEOFF_V15);
#endif
    fts_delay_v15(50);

    /***************************************************/
    /* LGD_STM_TEST :: Get mutual raw data */
    /***************************************************/

    printf("\n--------------------------------------\n");
    printf("[ CM REFERENCE RAW ]\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_cm_reference_raw[0],l_cm_reference_raw[1]);

    fts_get_ms_data_v15(FTS_TX_LENGTH_V15, FTS_RX_LENGTH_V15, ms_raw_data, FTS_MS_RAW_ADDR_V15);

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // mx_raw_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V15; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V15; rxIdx++)
        {
            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
            index = txIdx * FTS_RX_LENGTH_V15 + rxIdx;
            cvt_data[txIdx][rxIdx] = ms_raw_data[index];
        }
    }

    // For Get Reference Gap (Max - Min)
    // No Need Bubble Sort...
    ref_max = 0;
    ref_min = 0;
    // Initialize... Set First Data.
    ref_max = cvt_data[0][0];
    ref_min = cvt_data[0][0];

    /****************************************************/

    /****************************************************/
    // Modified by iamozzi...
    for (i = 0; i < FTS_TX_LENGTH_V15; i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V15; j++)
        {
            if (cvt_data[i][j] < l_cm_reference_raw[0])
            {
                count_min++;
                if(DEBUG_MODE)
                printf("-%04d ", cvt_data[i][j]);
            }
            else if (cvt_data[i][j] > l_cm_reference_raw[1])
            {
                count_max++;
                if(DEBUG_MODE)
                printf("+%04d ", cvt_data[i][j]);
            }
            else
            {
                if(DEBUG_MODE)
                printf("%05d ", cvt_data[i][j]);
            }

            // Get Max and Min Value For Reference Gap (Max - Min)
            if (ref_max < cvt_data[i][j])
            {
                ref_max = cvt_data[i][j];
            }

            if (ref_min > cvt_data[i][j])
            {
                ref_min = cvt_data[i][j];
            }
        }
        if(DEBUG_MODE)
            printf("\n");
    }

    /****************************************************/

    if(count_min)
    {
        printf("=======> CM REFERENCE_RAW < OPEN > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_CM_REFERENCE_RAW;
        fstat = 1;
    }

    if(count_max)
    {
        printf("=======> CM REFERENCE_RAW < SHORT > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_CM_REFERENCE_RAW;
        fstat = 1;
    }


    if(!fstat)
    {
        printf("=======> CM REFERENCE_RAW PASS\n");
        ret_state |= 0 << TOUCH_STM_CM_REFERENCE_RAW;
    }

    fstat = 0;

    printf("MAX / MIN : %d / %d \n",count_max,count_min);
    printf("--------------------------------------\n\n");


    printf("--------------------------------------\n");
    printf("[ CM REFERENCE GAP ]\n");

    /****************************************************/
    // Modified by iamozzi...

    //Remove Func. : memset / memcpy / bubble_sort Func.

    printf("MAX %d // MIN %d \n", ref_max, ref_min);
    printf("CM REFFERENCE GAP = %d, High Limit = %d\n", ref_max - ref_min, l_cm_reference_gap);
    if (l_cm_reference_gap > (ref_max - ref_min))
    {
        printf("=======> CM REFERENCE GAP -> PASS\n");
        ret_state |= 0 << TOUCH_STM_CM_REFERENCE_GAP;
    }
    else
    {
        ret_state |= 1 << TOUCH_STM_CM_REFERENCE_GAP;
        printf("=======> CM REFERENCE GAP -> FAIL\n");
    }
    printf("--------------------------------------\n\n");

    /****************************************************/

    /***************************************************/
    /* LGD_STM_TEST :: Get jitter data */
    /***************************************************/
    printf("--------------------------------------\n");
    printf("[ CM JITTER ]\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_cm_jitter[0],l_cm_jitter[1]);

    fts_get_ms_data_v15(FTS_TX_LENGTH_V15, FTS_RX_LENGTH_V15, (uint16_t *)ms_jitter_data, FTS_MS_JIT_ADDR_V15);

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // ms_jitter_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V15; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V15; rxIdx++)
        {
            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
            index = txIdx * FTS_RX_LENGTH_V15 + rxIdx;
            cvt_data[txIdx][rxIdx] = ms_jitter_data[index];
        }
    }

    /****************************************************/


    /****************************************************/
    // Modified by iamozzi...
    for (i = 0; i < FTS_TX_LENGTH_V15; i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V15; j++)
        {
            if (cvt_data[i][j] < l_cm_jitter[0])
            {
                count_min++;
            }
            else if (cvt_data[i][j] > l_cm_jitter[1])
            {
                count_max++;
            }

            if(DEBUG_MODE)
                printf("%04d ", cvt_data[i][j]);
        }
        if(DEBUG_MODE)
            printf("\n");
    }

    /****************************************************/

    if(count_min)
    {
        printf("=======> CM JITTER < OPEN > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_CM_JITTER;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> CM JITTER < SHORT > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_CM_JITTER;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> CM JITTER PASS\n");
        ret_state |= 0 << TOUCH_STM_CM_JITTER;
    }

    fstat = 0;


    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    if(DEBUG_MODE)
        printf("[FTS] Getting jitter data is done!!\n\n");
    printf("--------------------------------------\n\n");

    /***************************************************/
    /* LGD_STM_TEST :: Get self raw data */
    /***************************************************/
    printf("--------------------------------------\n");
    printf("[ SELF RAW DATA ] : TX\n");
    fts_get_ss_data_v15(FTS_TX_LENGTH_V15, FTS_RX_LENGTH_V15, ss_raw_f_buf, ss_raw_s_buf, FTS_SS_F_RAW_ADDR_V15);

    count_min = 0;
    count_max = 0;
    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_self_raw_tx[0],l_self_raw_tx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_TX_buf : ");

    for(i=0; i<FTS_TX_LENGTH_V15; i++)
    {
        if(ss_raw_f_buf[i] < l_self_raw_tx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_f_buf[i]);
        }
        else if(ss_raw_f_buf[i] > l_self_raw_tx[1])
        {
            count_max++;
            if(DEBUG_MODE)
                printf("+%03d ",ss_raw_f_buf[i]);
        }
        else
        {
            if(DEBUG_MODE)
                printf("%04d ",ss_raw_f_buf[i]);
        }

    }
    if(DEBUG_MODE)
        printf("\n");

    if(count_min)
    {
        printf("=======> SELF_RAW_TX < TOO LOW > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_TX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> SELF_RAW_TX < TOO HIGH > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_TX;
        fstat = 1;
    }
    if(!fstat)
    {
        printf("=======> SELF_RAW_TX PASS\n");
        ret_state |= 0 << TOUCH_STM_SELF_RAW_TX;
    }

    fstat = 0;


    printf("MAX / MIN : %d / %d \n",count_max,count_min);
    printf("--------------------------------------\n\n");

    printf("--------------------------------------\n");
    printf("[ SELF RAW DATA ] : RX\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_self_raw_rx[0],l_self_raw_rx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_RX_buf : ");
    for(i=0; i<FTS_RX_LENGTH_V15; i++)
    {

        if(ss_raw_s_buf[i] < l_self_raw_rx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_s_buf[i]);
        }
        else if(ss_raw_s_buf[i] > l_self_raw_rx[1])
        {
            count_max++;
            if(DEBUG_MODE)
                printf("+%03d ",ss_raw_s_buf[i]);
        }
        else
        {
            if(DEBUG_MODE)
                printf("%04d ",ss_raw_s_buf[i]);
        }
    }
    if(DEBUG_MODE)
        printf("\n");
    if(count_min)
    {
        printf("=======> SELF_RAW_RX < TOO LOW > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_RX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> SELF_RAW_RX < TOO HIGH > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_RX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> SELF_RAW_RX PASS\n");
        ret_state |= 0 << TOUCH_STM_SELF_RAW_RX;
    }

    fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);
    if(DEBUG_MODE)
        printf("[FTS] Getting raw of self is done!!\n\n");
    printf("--------------------------------------\n\n");

#ifdef	FTS_SUPPORT_MS_KEY_V15
	/* Get mutual key raw data */
	fts_get_ms_data_v15(FTS_MSKEY_TX_LENGTH_V15, FTS_MSKEY_RX_LENGTH_V15, ms_key_raw_data, FTS_MS_KEY_RAW_ADDR_V15);
	printf("\n\r[FTS] Getting raw of MSKey is done.");
#endif

#ifdef	FTS_SUPPORT_FORCE_FSR_V15
	fts_get_force_data_v15(FTS_FORCE_TX_LENGTH_V15, FTS_FORCE_RX_LENGTH_V15, ms_force_raw_buf, FTS_FORCE_RAW_ADDR_V15);
	fts_get_force_data_v15(FTS_FORCE_TX_LENGTH_V15, FTS_FORCE_RX_LENGTH_V15, ms_force_str_buf, FTS_FORCE_STR_ADDR_V15);
	printf("\n\r[FTS] Getting raw of fsr is done.");
#endif

    printf("--------------------------------------\n");
    printf("OPEN SHORT TEST\n");

	ret = fts_panel_short_test_v15();
    if(ret < TRUE)
    {
        if(ret == I2C_ERR)
        {
            ret_state |= 1<<TOUCH_STM_I2C_CHECK;
            uart_buf[10] = l_hf_test_mode;
            printf("=======> I2C FAIL!! \n");
        }
        else
        {
            ret_state |= 1<<TOUCH_STM_OPEN_SHORT;
            printf("=======> OPEN_SHORT_TEST FAIL\n");
        }
    }
    else
    {
        ret_state |= 0<<TOUCH_STM_OPEN_SHORT;
        printf("=======> OPEN_SHORT_TEST PASS\n");
    }
     printf("--------------------------------------\n\n");

////////////////// HF_TEST....

	/* Auto tune with High Frequency */
#ifdef	FTS_SUPPORT_CHK_HIGH_FREQ_V15

    if(fts_systemreset_v15() == I2C_ERR)
	{
        ret_state |= 1<<TOUCH_STM_I2C_CHECK;
        uart_buf[10] = l_hf_test_mode;

		FUNC_END();
		return I2C_ERR;
	}

    if (fts_cmd_completion_check_v15(EVENTID_CONTROLLER_READY_V15, 0x00, 0x00) < TRUE)
    {
        printf("[FTS] FAILED - System Reset\n");
    }

#ifdef PRJ_LGMC_V15
    fts_command_v15(HF_MUTUAL_AT_V15);
#else
    regAdd[0] = 0xA3;       regAdd[1] = 0x01;
    if(fts_write_reg_v15(regAdd, 2))
    {
        ret_state |= 1<<TOUCH_STM_I2C_CHECK;
        uart_buf[10] = l_hf_test_mode;
		FUNC_END();
        return I2C_ERR;
    }
#endif
    fts_delay_v15(300);

#if 1
    if(l_hf_test_mode && (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_MS_DONE_V15, 0x00) == TRUE))
	{

#if 1 //////////////////////////   test
        fstat = 0;
/* if 3 recycle.. 
        printf("--------------------------------HF TEST[%d]--------------------------------\n",reHF);
*/
        printf("Mutual Auto Tune with High Frequency Done\n");

        /***************************************************/
        /* LGD_STM_TEST :: Get cx value of mutual sensing (High Frequency) */
        /***************************************************/
        printf("\n--------------------------------------\n");
        printf("TOTAL-CX (HIGH FREQUENCY)\n");
        fts_get_ms_cx_data_v15(ms_hf_cx_data, FTS_COMP_MS_CX_SEL_V15);

        /****************************************************/
        /* Added by iamozzi. 2017.06.22.                    */
        /****************************************************/
        // Convert To 2 Dimension Matrix From 1 Dimension Array
        // ms_hf_cx_data[] ->> cvt_data[][]
        // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32

        for (txIdx = 0; txIdx < FTS_TX_LENGTH_V15; txIdx++)
        {
            for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V15; rxIdx++)
            {
                cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
                index = txIdx * FTS_RX_LENGTH_V15 + rxIdx;
                cvt_data[txIdx][rxIdx] = ms_hf_cx_data[index];
            }
        }
        /****************************************************/

        count_min = 0;
        count_max = 0;

        /****************************************************/
        // Modified by iamozzi...

        for (i = 0; i < FTS_TX_LENGTH_V15; i++)
        {
            for (j = 0; j < FTS_RX_LENGTH_V15; j++)
            {
                if (cvt_data[i][j] < l_cx2_hf[0])
                {
                    count_min++;
                    if(DEBUG_MODE)
                        printf("-%03d ", cvt_data[i][j]);
                }
                else if (cvt_data[i][j] > l_cx2_hf[1])
                {
                    count_max++;
                    if(DEBUG_MODE)
                        printf("+%03d ", cvt_data[i][j]);
                }
                else
                {
                    if(DEBUG_MODE)
                        printf("%04d ", cvt_data[i][j]);
                }
            }
            if(DEBUG_MODE)
                printf("\n");
        }

        /****************************************************/

        if(count_min)
        {
            printf("=======> CX2 HF < TOO LOW > FAIL [%d]\n",count_min);
            ret_state |= 1 << TOUCH_STM_CX2_HF;
            fstat = 1;
        }
        if(count_max)
        {
            printf("=======> CX2 HF < TOO HIGH > FAIL [%d]\n",count_max);
            ret_state |= 1 << TOUCH_STM_CX2_HF;
            fstat = 1;
        }

        if(!fstat)
        {
            printf("=======> CX2 HF PASS\n");
            ret_state |= 0 << TOUCH_STM_CX2_HF;
        }
/* if 3 recycle.. 
*/
        fstat = 0;
   
        printf("MAX / MIN : %d / %d \n",count_max,count_min);

        printf("[FTS] Getting comp. data of mutual(HF) is done!!\n\n");
        printf("--------------------------------------\n\n");
   
        count_max = 0;
        count_min = 0;

        printf("--------------------------------------\n");
        printf("TOTAL-CX-HF-GAP(H)  (HIGH FREQUENCY)\n");
        for (i = 0; i < FTS_TX_LENGTH_V15; i++)
        {
            for (j = 0; j < (FTS_RX_LENGTH_V15 - 1); j++)
            {
                    // Get Gap...
                if (cvt_data[i][j] > cvt_data[i][j + 1])
                {
                    ms_hf_cx_data_Hgap[i][j] = cvt_data[i][j] - cvt_data[i][j + 1];
                }

                else
                {
                    ms_hf_cx_data_Hgap[i][j] = cvt_data[i][j + 1] - cvt_data[i][j];
                }

                    // Counting Fail...
                if (ms_hf_cx_data_Hgap[i][j] > hf_TotalCx_Gap_Rx_MAX[i + 1][j + 1])
                {
                    count_max++;
                    if(DEBUG_MODE)
                        printf("+%03d ", ms_hf_cx_data_Hgap[i][j]);
                }
                else if (ms_hf_cx_data_Hgap[i][j] < hf_TotalCx_Gap_Rx_MIN[i + 1][j + 1])
                {
                    count_min++;
                    if(DEBUG_MODE)
                        printf("-%03d ", ms_hf_cx_data_Hgap[i][j]);
                }

                else
                {
                    if(DEBUG_MODE)
                        printf("%04d ", ms_hf_cx_data_Hgap[i][j]);
                }

            }
            if(DEBUG_MODE)
                printf("\n");
        }

        /****************************************************/
        if(count_max)
        {
            printf("=======> TOTAL-CX-HF-GAP(H) < TOO HIGH > FAIL [%d]\n",count_max);
            ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_H;
            fstat = 1;
        }
        if(count_min)
        {
            printf("=======> TOTAL-CX-HF-GAP(H) < TOO LOW > FAIL [%d]\n",count_min);
            ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_H;
            fstat = 1;
        }
   
        if(!fstat)
        {
            printf("=======> TOTAL-CX-HF-GAP(H) PASS\n");
            ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_H;
        }
/* if 3 recycle.. 
*/
        fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);

        printf("--------------------------------------\n\n");

        count_max = 0;
        count_min = 0;

            //printf("LowLimit = %d / HighLimit = %d\n",);


            /****************************************************/
            // Modified by iamozzi...

    /****************************************************/
        printf("--------------------------------------\n");
        printf("TOTAL-CX-HF-GAP(V)  (HIGH FREQUENCY)\n");

        for (i = 0; i < (FTS_TX_LENGTH_V15 - 1); i++)
        {
            for (j = 0; j < FTS_RX_LENGTH_V15; j++)
            {
                // Get Gap...
                if (cvt_data[i][j] > cvt_data[i + 1][j])
                {
                    ms_hf_cx_data_Vgap[i][j] = cvt_data[i][j] - cvt_data[i + 1][j];
                }
                else
                {
                    ms_hf_cx_data_Vgap[i][j] = cvt_data[i + 1][j] - cvt_data[i][j];
                }


                // Counting Fail...
                if (ms_hf_cx_data_Vgap[i][j] > hf_TotalCx_Gap_Tx_MAX[i + 1][j + 1])
                {
                    count_max++;
                    if(DEBUG_MODE)
                    printf("+%03d ", ms_hf_cx_data_Vgap[i][j]);
                }
                else if (ms_hf_cx_data_Vgap[i][j] < hf_TotalCx_Gap_Tx_MIN[i + 1][j + 1])
                {
                    count_min++;
                    if(DEBUG_MODE)
                        printf("-%03d ", ms_hf_cx_data_Vgap[i][j]);
                }
                else
                {
                    if(DEBUG_MODE)
                        printf("%04d ", ms_hf_cx_data_Vgap[i][j]);

                }
            }
            if(DEBUG_MODE)
                printf("\n");
        }
        if(count_max)
        {
            printf("=======> TOTAL-CX-HF-GAP(V) < TOO HIGH > FAIL [%d]\n",count_max);
            ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_V;
            fstat = 1;
        }
        if(count_min)
        {
            printf("=======> TOTAL-CX-HF-GAP(V) < TOO LOW > FAIL [%d]\n",count_min);
            ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_V;
            fstat = 1;
        }

        if(!fstat)
        {
            printf("=======> TOTAL-CX-HF-GAP(V) PASS\n");
            ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_V;
        }
/* if 3 recycle.. 
*/
        fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);
        printf("--------------------------------------\n\n");
        printf("----------------------------------------------------------------\n");
/* if 3 recycle.. 
*/
    }
    else
#endif  /////////////////////// test
#endif
#if 1 
    {
            printf("Mutual Auto Tune with High Frequency Failed\n"); //PRINT SAME RESULT -> TOTAL-CX(NORMAL)
            /***************************************************/
            /* LGD_STM_TEST :: Get cx value of mutual sensing (High Frequency) */
            /***************************************************/
            printf("\n--------------------------------------\n");
            printf("TOTAL-CX (HIGH FREQUENCY)\n");
            printf("--------------------------------------\n");
            printf("> Can't test TOTAL-CX-HF..FAIL \n");
            if(l_hf_test_mode)
            { 
                ret_state |= 1 << TOUCH_STM_CX2_HF;
                ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_H;
                ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_V;
/* if 3 recycle.. 
*/
            printf("----------------------------------------------------------------\n");
            fstat = 0;
            }
            else
            {
                ret_state |= 0 << TOUCH_STM_CX2_HF;
                ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_H;
                ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_V;
                printf("> HF_TEST OFF MODE \n");
/* if 3 recycle.. 
*/
            }
    }
#endif 
/* if 3 recycle.. 
*/
#endif

    uart_buf[4] = ret_state & 0xFF;
    uart_buf[5] = (ret_state >> 8) & 0xFF;
    uart_buf[6] = (ret_state >> 16) & 0xFF;
    uart_buf[7] = (ret_state >> 24) & 0xFF;
    uart_buf[10] = l_hf_test_mode;

    int s =0;
    printf("STATE : ");
    for(s = 0; s <4; s++)
        printf("0x%X, ",uart_buf[s+4]);
    printf("/\n");

	FUNC_END();
    return  TRUE;
}


/**
  * @brief  Initialize operations with Auto-tune sequence for LGD FSR.
  * 		This function has to be run a time after firmware update.
  * @param  None
  * @retval None
  */
int fts_init_v15(void)
{
	int ret = 0;

	FUNC_BEGIN();
	ret = fts_systemreset_v15();
    if(ret == I2C_ERR)
	{
		FUNC_END();
        return I2C_ERR;
	}

	if (fts_cmd_completion_check_v15(0x10, 0x00, 0x00) < TRUE)
	{
		printf("\n\r[FTS] System Reset FAILED");
		FUNC_END();
		return	FALSE;
	}

	fts_read_chip_id_v15();

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V15
	flashProcedure_v15(0, 0);
#endif

	fts_interrupt_control_v15(DISABLE);
	fts_delay_v15(1);

	fts_command_v15(MUTUAL_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_MS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] Mutual Auto Tune FAILED");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V15
	fts_command_v15(SELF_AUTO_TUNE_V15);
	fts_delay_v15(300);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_AT_SS_DONE_V15, 0x00) < TRUE)
	{
		printf("\n\r[FTS] Self Auto Tune FAILED");
		FUNC_END();
		return	FALSE;
	}
#endif

	fts_command_v15(SENSEON_V15);
	if (fts_cmd_completion_check_v15(EVENTID_SYSTEM_STATUS_UPDATE_V15, EVENTID_STATID_FCAL_DONE_V15, 0x23) < TRUE)
	{
		printf("\n\r[FTS] FAILED - Event(FCAL) for SenseOn is not reached");
		FUNC_END();
		return	FALSE;
	}

	vcp_printf("\n\r[FTS] Sense On");
#ifdef	FTS_SUPPORT_MS_KEY_V15
	fts_command_v15(MSKEY_SENSEON_V15);
	printf("\n\r[FTS] MS KEY Sense On");
#endif

	fts_command_v15(FLUSHBUFFER_V15);
	fts_delay_v15(50);

	fts_interrupt_control_v15(ENABLE);
	printf("\n\r[FTS] Interrupt Enable.");

	FUNC_END();
	return	TRUE;
}


int limit_data_match_v15(int id, struct stm_touch_limit* limit)
{

    int i =0;

	FUNC_BEGIN();
    if(id != (int)limit->id)
    {
        printf("touch limit id match FAIL..[%d/%d] \n",id,(int)limit->id);
		FUNC_END();
        return FAIL;
    } 
   
////////////////init

    l_product_id = 0;
    l_fw_ver = 0;
    l_config_ver = 0;
    l_release_ver = 0;
    l_pat_cm_reference_raw[MIN] = 0;
    l_pat_cm_reference_raw[MAX] = 0;
    l_pat_self_raw_tx[MIN] = 0;
    l_pat_self_raw_tx[MAX] = 0;
    l_pat_self_raw_rx[MIN] = 0;
    l_pat_self_raw_rx[MAX] = 0;
    l_cm_reference_raw[MIN] = 0;
    l_cm_reference_raw[MAX] = 0;
    l_cm_reference_gap = 0;
    l_cm_jitter[MIN] = 0;
    l_cm_jitter[MAX] = 0;
    l_total_cx[MIN] = 0;
    l_total_cx[MAX] = 0;
    l_self_raw_tx[MIN] = 0;
    l_self_raw_tx[MAX] = 0;
    l_self_raw_rx[MIN] = 0;
    l_self_raw_rx[MAX] = 0;
    l_lp_self_raw_tx[MAX] = 0;
    l_lp_self_raw_tx[MIN] = 0;
    l_lp_self_raw_rx[MAX] = 0;
    l_lp_self_raw_rx[MIN] = 0;
    l_self_ix_tx[MIN] = 0;
    l_self_ix_tx[MAX] = 0;
    l_self_ix_rx[MIN] = 0;
    l_self_ix_rx[MAX] = 0;
    l_cx2_hf[MIN] = 0;
    l_cx2_hf[MAX] = 0;
    l_hf_test_mode = 1;
    l_otp_param = 0;
    l_otp_param1 = 0;
    l_otp_param2 = 0;
    l_otp_param3 = 0;

    for(i = 0; i < 300; i++)
    {
        memset(totalCx_MAX[i],0,sizeof(totalCx_MAX[i]));
        memset(totalCx_MIN[i],0,sizeof(totalCx_MIN[i]));
        memset(totalCx_Gap_Rx_MAX[i],0,sizeof(totalCx_Gap_Rx_MAX[i]));
        memset(totalCx_Gap_Rx_MIN[i],0,sizeof(totalCx_Gap_Rx_MIN[i]));
        memset(totalCx_Gap_Tx_MAX[i],0,sizeof(totalCx_Gap_Tx_MAX[i]));
        memset(totalCx_Gap_Tx_MIN[i],0,sizeof(totalCx_Gap_Tx_MIN[i]));
        memset(hf_TotalCx_Gap_Rx_MAX[i],0,sizeof(hf_TotalCx_Gap_Rx_MAX[i]));
        memset(hf_TotalCx_Gap_Rx_MIN[i],0,sizeof(hf_TotalCx_Gap_Rx_MIN[i]));
        memset(hf_TotalCx_Gap_Tx_MAX[i],0,sizeof(hf_TotalCx_Gap_Tx_MAX[i]));
        memset(hf_TotalCx_Gap_Tx_MIN[i],0,sizeof(hf_TotalCx_Gap_Tx_MIN[i]));
    }

/////////////////////////////////

    l_product_id=limit->product_id;
    l_fw_ver=limit->fw_ver;
    l_config_ver=limit->config_ver;
    l_release_ver=limit->release_ver;
    l_pat_cm_reference_raw[MIN]=limit->pat_cm_reference_raw[MIN]; // [0] : MIN / [1] : MAX
    l_pat_cm_reference_raw[MAX]=limit->pat_cm_reference_raw[MAX]; // [0] : MIN / [1] : MAX
    l_pat_self_raw_tx[MIN]=limit->pat_self_raw_tx[MIN];
    l_pat_self_raw_tx[MAX]=limit->pat_self_raw_tx[MAX];
    l_pat_self_raw_rx[MIN]=limit->pat_self_raw_rx[MIN];
    l_pat_self_raw_rx[MAX]=limit->pat_self_raw_rx[MAX];
    l_cm_reference_raw[MIN]=limit->cm_reference_raw[MIN]; // [0] : MIN / [1] : MAX
    l_cm_reference_raw[MAX]=limit->cm_reference_raw[MAX]; // [0] : MIN / [1] : MAX


    l_cm_reference_gap=limit->cm_reference_gap;
    l_cm_jitter[MIN]=limit->cm_jitter[MIN];
    l_cm_jitter[MAX]=limit->cm_jitter[MAX];
    l_total_cx[MIN]=limit->total_cx[MIN];
    l_total_cx[MAX]=limit->total_cx[MAX];
    l_self_raw_tx[MIN]=limit->self_raw_tx[MIN];
    l_self_raw_tx[MAX]=limit->self_raw_tx[MAX];
    l_self_raw_rx[MIN]=limit->self_raw_rx[MIN];
    l_self_raw_rx[MAX]=limit->self_raw_rx[MAX];
    l_lp_self_raw_tx[MIN]=limit->lp_self_raw_tx[MIN];
    l_lp_self_raw_tx[MAX]=limit->lp_self_raw_tx[MAX];
    l_lp_self_raw_rx[MIN]=limit->lp_self_raw_rx[MIN];
    l_lp_self_raw_rx[MAX]=limit->lp_self_raw_rx[MAX];
    l_self_ix_tx[MIN]=limit->self_ix_tx[MIN];
    l_self_ix_tx[MAX]=limit->self_ix_tx[MAX];
    l_self_ix_rx[MIN]=limit->self_ix_rx[MIN];
    l_self_ix_rx[MAX]=limit->self_ix_rx[MAX];
    l_cx2_hf[MIN]=limit->cx2_hf[MIN];
    l_cx2_hf[MAX]=limit->cx2_hf[MAX];
    l_hf_test_mode = limit->hf_test_mode;
    l_otp_param = limit->otp_param;
    l_otp_param1 = limit->otp_param1;
    l_otp_param2 = limit->otp_param2;
    l_otp_param3 = limit->otp_param3;

    memcpy(totalCx_MAX,limit->totalCx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(totalCx_MIN,limit->totalCx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(totalCx_Gap_Rx_MAX,limit->totalCx_Gap_Rx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(totalCx_Gap_Rx_MIN,limit->totalCx_Gap_Rx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(totalCx_Gap_Tx_MAX,limit->totalCx_Gap_Tx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(totalCx_Gap_Tx_MIN,limit->totalCx_Gap_Tx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Rx_MAX,limit->hf_TotalCx_Gap_Rx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Rx_MIN,limit->hf_TotalCx_Gap_Rx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Tx_MAX,limit->hf_TotalCx_Gap_Tx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Tx_MIN,limit->hf_TotalCx_Gap_Tx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..

///////////////////////////////////////

	FUNC_END();
    return  PASS;
}

void i2c_dev_match_v15(int i2c_dev)
{
	FUNC_BEGIN();
    stm_dev = i2c_dev;
    printf("%s : STM API : I2C Device Match OK! \n",__func__);
    if(!dicOpen)
        vfos_dev_open();
    else
        printf("%s : DIC dev already OPEN \n",__func__);

	FUNC_END();
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/

