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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>


#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>     //usleep 

#include <sys/time.h>
#include <sys/types.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <pthread.h>
#include <dirent.h>
#include <linux/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <type.h>
#include <rs485.h>
#include    <stm_touch.h>

#include <fts_lgd_08.h>
#include <i2c-dev.h>

#define MIN     0
#define MAX     1

/** @addtogroup FTS_Private_Variables
  * @{
  */
static	uint8_t fts_fifo_addr[2] = {FTS_FIFO_ADDR_V8, 0};

uint8_t		sysInfo_buf[SYS_INFO_SIZE_V8];
SysInfo		*pSysInfo;

int	stm_dev;
int	dic_dev;

int golden_value_err_state = 0;

unsigned short  l_fw_ver;
unsigned short  l_config_ver;
unsigned short  l_release_ver;
int    l_cm_reference_raw[2];
int    l_cm_reference_gap;
int    l_self_raw_tx[2];
int    l_self_raw_rx[2];
int		l_cm_jitter[2];
int		l_ss_jitter[2]; ///
int    l_lp_self_raw_tx[2];
int    l_lp_self_raw_rx[2];
int    l_lp_raw[2]; ///

int    l_hf_gap_rx[2]; ///
int    l_hf_gap_tx[2]; ///
int hf_TotalCx_Gap_Rx_MAX[300][300];
int hf_TotalCx_Gap_Rx_MIN[300][300];
int hf_TotalCx_Gap_Tx_MAX[300][300];
int hf_TotalCx_Gap_Tx_MIN[300][300];

int		l_hf_test_mode;


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
void fts_delay_v8(uint32_t msCount)
{
	usleep(msCount*1000);
}

/**
  * @brief  Supply a voltage for analog (3.3V)
  * @param  None
  * @retval None
  */
void analog_3V3_power_on_v8(void)
{
	/* user code */
}

/**
  * @brief  Supply a voltage for digital (1.8V)
  * @param  None
  * @retval None
  */
void digital_1V8_power_on_v8(void)
{
	/* user code */
}

/**
  * @brief  Supply a voltage to FTS
  * @note	Have to turn on 1.8V earlier than 3.3V or 3.3V and 1.8V at the same time.
  * @param  None
  * @retval None
  */
void power_on_v8(void)
{
	FUNC_BEGIN();
	/* Please turn on 1.8V earlier than 3.3V */
	digital_1V8_power_on_v8();
	fts_delay_v8(10);

	analog_3V3_power_on_v8();
	fts_delay_v8(10);
	FUNC_END();
}

#if	1

/**
  * @brief  Write data to target registers.
  * @param  pInBuf  : A pointer of the buffer to be sending to target.
  * @param  inCnt   : The count of pInBuf to be writing.
  * @retval OK if all operations done correctly. FAIL if error.
  */
int fts_write_reg_v8(uint8_t *pInBuf, int InCnt)
{
	/* Implement user's fts_write_reg depend on your system. */
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V8 >> 1;
    messages[0].flags = 0;
    messages[0].len = InCnt;
    messages[0].buf = (char *)pInBuf;

    packets.msgs = messages;
    packets.nmsgs = 1;

    if(ioctl(stm_dev, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
		printf("I2C WRITE FAIL \n");
		FUNC_END();
        return (FTS_ERR_V8 | FTS_ERR_I2C_V8);
    }

	FUNC_END();
	return	0;
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
int fts_read_reg_v8(uint8_t *pInBuf, int inCnt, uint8_t *pOutBuf, int outCnt)
{
	/* Implement user's fts_read_reg depend on your system. */

    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V8 >> 1;
    messages[0].flags = 0;
    messages[0].len = inCnt;
    messages[0].buf = (char *)pInBuf;

    messages[1].addr = FTS_I2C_ADDR_V8 >> 1;
    messages[1].flags = I2C_M_RD;
    messages[1].len = outCnt;
    messages[1].buf = (char *)pOutBuf;

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(stm_dev, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
		printf("I2C READ FAIL \n");
		FUNC_END();
        return (FTS_ERR_V8 | FTS_ERR_I2C_V8);
    }
	FUNC_END();
    return  0;
}

#endif

/**
  * @brief  Write a command to target.
  * @param  cmd		: A command.
  * @retval None
  */
int fts_command_v8(uint8_t cmd)
{
	uint8_t	regAdd = 0;

	FUNC_BEGIN();
	regAdd = cmd;
	if (fts_write_reg_v8(&regAdd, 1) != 0)
	{
		FUNC_END();
		return	(FTS_ERR_V8 | FTS_ERR_I2C_V8);
	}

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

/**
  * @brief  Write a command to target.
  * @param  cmd		: A command.
  * @retval None
  */
int fts_write_regU32_v8(uint8_t cmd, uint32_t reg_addr, int addr_size, uint8_t *data, int data_size)
{
	//uint8_t	regAdd[16];
	uint8_t	regAdd[32];
	int		i;

	FUNC_BEGIN();
	regAdd[0] = cmd;
	for (i = 0; i < addr_size; i++)
	{
		regAdd[i + 1] = (uint8_t) (reg_addr >> ((addr_size - 1 - i) * 8) & 0xFF);
	}
	for (i = 0; i < data_size; i++)
	{
		regAdd[i + 1 + addr_size] = data[i];
	}
	if (fts_write_reg_v8(regAdd, addr_size + data_size + 1) != 0)
	{
		FUNC_END();
		return	(FTS_ERR_V8 | FTS_ERR_I2C_V8);
	}

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

/**
  * @brief  Write a command to target.
  * @param  cmd		: A command.
  * @retval None
  */
int fts_read_regU32_v8(uint8_t cmd, uint32_t reg_addr, int addr_size, uint8_t *pOutBuf, int outCnt)
{
	uint8_t	regAdd[16];
	int		i;

	FUNC_BEGIN();
	regAdd[0] = cmd;
	for (i = 0; i < addr_size; i++)
	{
		regAdd[i + 1] = (uint8_t) (reg_addr >> ((addr_size - 1 - i) * 8) & 0xFF);
	}
	if (fts_read_reg_v8(regAdd, addr_size + 1, pOutBuf, outCnt) != 0)
	{
		FUNC_END();
		return	(FTS_ERR_V8 | FTS_ERR_I2C_V8);
	}

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

/**
  * @brief  Check event messages for commands
  * @param  event[1..3] : Event messages
  * @retval status	: TRUE if event message for command received, FALSE if not.
  */
int fts_err_event_handler_v8(uint8_t *val)
{
	int		err_type = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	if (val[0] != EVTID_ERROR_REPORT_V8)
	{
		FUNC_END();
		return	err_type;
	}

	printf("\r\n[err_check]ERROR [%02x %02x %02x %02x %02x %02x %02x %02x]",
			val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);

	switch (val[1] & 0xF0)
	{

	case EVTID_ERR_TYPE_SYSTEM1_V8:
	case EVTID_ERR_TYPE_SYSTEM2_V8:
		err_type |= (FTS_ERR_V8 | FTS_ERR_SYSTEM_V8);
		break;
	case	0xC0:
		break;
	case	EVTID_ERR_TYPE_CRC_V8:
// add by healim - 180514
        if ((val[1] >= 0x24) || (val[1] <= 0x27))
        {
            err_type |= (FTS_ERR_V8 | FTS_ERR_CRC_CORRUPT_V8);
        }

        if ((val[1] == 0x29) || (val[1] == 0x2A))
        {
            err_type |= (FTS_ERR_V8 | FTS_ERR_CRC_CORRUPT_V8);
        }
// end
		if ((val[1] == 0x20) || (val[1] == 0x21))
		{
			err_type |= (FTS_ERR_V8 | FTS_ERR_CRC_CFG_V8);
		}
		break;
	#if	0
	case	EVTID_ERR_TYPE_MS_TUNE:
			break;
	case	EVTID_ERR_TYPE_SS_TUNE:
		break;
	#endif
	case	EVTID_ERR_TYPE_CX_V8:
		err_type |= (FTS_ERR_V8 | FTS_ERR_CRC_CX_V8);
		break;
	default:
		err_type |= FTS_ERR_V8;
		break;
	}

	FUNC_END();
	return err_type;
}

/**
  * @brief  Parse and process received events.
  * @note	Reporting of finger data when the presence of fingers is detected.
  * @param  data		: The buffer of event saved.
  * @param	LeftEvent	: Count of events
  * @retval None
  */
int fts_event_handler_type_b_v8(uint8_t *data, uint8_t LeftEvent)
{
	uint8_t 	EventNum = 0;
	uint8_t		TouchID = 0, EventID = 0;
	uint16_t	x = 0, y = 0, z = 0;

	FUNC_BEGIN();
	for (EventNum = 0; EventNum < LeftEvent; EventNum++)
	{
		EventID = data[EventNum * FTS_EVENT_SIZE_V8];
		if ((EventID == EVTID_ENTER_POINTER_V8) || (EventID == EVTID_MOTION_POINTER_V8) || (EventID == EVTID_LEAVE_POINTER_V8))
		{
			TouchID = (data[1 + EventNum * FTS_EVENT_SIZE_V8] >> 4) & 0x0F;
			x = (data[2 + EventNum * FTS_EVENT_SIZE_V8] & 0x00FF) + ((data[3 + EventNum * FTS_EVENT_SIZE_V8] << 8) & 0x0F00);
			y = ((data[3 + EventNum * FTS_EVENT_SIZE_V8] >> 4) & 0x000F) + ((data[4 + EventNum * FTS_EVENT_SIZE_V8] << 4) & 0x0FF0);

			switch (EventID)
			{
			case	EVTID_ENTER_POINTER_V8:
				printf("\r\n[FTS]ENTER  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVTID_MOTION_POINTER_V8:
				printf("\r\n[FTS] MOTION : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			case	EVTID_LEAVE_POINTER_V8:
				printf("\r\n[FTS] LEAVE  : %d [x:%d][y:%d][z:%d]", TouchID, x, y, z);
				break;
			}
		}
	}

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

/**
  * @brief  Called by the ISR or the kernel when occurs an interrupt.
  * @note	This function handles the acquisition of finger data.
  * @param  None
  * @retval None
  */
int fts_event_handler_v8(void)
{
	uint8_t		data[FTS_EVENT_SIZE_V8 * FTS_FIFO_MAX_V8];
	int			evtcount = 0, remain_evtcnt = 0;

	FUNC_BEGIN();
	do	{
		fts_read_reg_v8(fts_fifo_addr, 1, (uint8_t *) data + (evtcount * FTS_EVENT_SIZE_V8), FTS_EVENT_SIZE_V8);
		remain_evtcnt = data[(evtcount * FTS_EVENT_SIZE_V8) + 7] & 0x1F;
		evtcount++;
	}	while (remain_evtcnt);

	fts_event_handler_type_b_v8(data, evtcount);

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

/**
  * @brief  Check event messages for commands
  * @param  event[1..3] : Event messages
  * @retval status	: TRUE if event message for command received, FALSE if not.
  */
int fts_cmd_completion_check_v8(uint8_t event_id, uint8_t rpt_type, uint8_t *system_cmd, uint8_t length, int timeout)
{
	uint8_t val[8];
	int 	retry;
	int		i;
	int		err_type = FTS_NO_ERR_V8, res = TRUE;
	int		ret = 0;

	FUNC_BEGIN();
	retry = timeout;
	while (retry--)
	{
		fts_delay_v8(50);
		ret =fts_read_reg_v8(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V8);
		if(ret == (FTS_ERR_V8 | FTS_ERR_I2C_V8))
		{
			FUNC_END();
			return ret;
		}
		switch (val[0])
		{
		case	EVTID_CONTROLLER_READY_V8:
			if (val[0] == event_id)
			{
				printf("\r\n[cmd_check] OK [%02x %02x %02x %02x %02x %02x %02x %02x]",
										val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);

				if ((err_type & (FTS_ERR_CRC_CX_V8 | FTS_ERR_CRC_CFG_V8| FTS_ERR_CRC_CORRUPT_V8)) != 0)
				{
					printf("\r\n[cmd_check]ERROR - there is CRC Error.");
					printf("\nTD32) Golden Value Check FAIL! [E : 0x%X]\n",(err_type & (FTS_ERR_CRC_CX_V8 | FTS_ERR_CRC_CFG_V8 | FTS_ERR_CRC_CORRUPT_V8)));
					printf("(CRC CX ERR 0x%X / CRC CFG ERR 0x%X) \n",FTS_ERR_CRC_CX_V8,FTS_ERR_CRC_CFG_V8);
					printf("(FTS_ERR_CRC_CORRUPT ERR 0x%X) \n",FTS_ERR_CRC_CORRUPT_V8);
					golden_value_err_state = 1;
					FUNC_END();
					return	err_type;
				}

				FUNC_END();
				return	FTS_NO_ERR_V8;
			}
			break;
		case	EVTID_STATUS_REPORT_V8:
			if ((val[0] == event_id) && (val[1] == rpt_type))
			{
				res = TRUE;
				for (i = 0; i < length; i++)
				{
					if (val[i + 2] != system_cmd[i])
						res = FALSE;
				}
				if (res == TRUE)
				{
					printf("\r\n[cmd_check] OK [%02x %02x %02x %02x %02x %02x %02x %02x]",
													val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
					FUNC_END();
					return	err_type;
				}
				else
				{
					printf("\r\n[cmd_check] [%02x %02x %02x %02x %02x %02x %02x %02x]",
													val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
				}
			}
			else
			{
				printf("\r\n[cmd_check] [%02x %02x %02x %02x %02x %02x %02x %02x]",
												val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
			}
			break;
		case	EVTID_ERROR_REPORT_V8:
			err_type = fts_err_event_handler_v8(val);
			break;
		default:
			break;
		}
	}

	if (err_type != FTS_NO_ERR_V8)
	{
		printf("\r\n[cmd_check]Error event generated.");
	}
	
	if (retry <= 0)
	{
		err_type |= (FTS_ERR_V8 | FTS_ERR_EVT_TIMEOVER_V8);
		printf("\r\n[cmd_check]Time Over");
	}

	FUNC_END();
	return err_type;
}

/**
  * @brief  System Reset
  * @param  None
  * @retval None
  */
int fts_systemreset_v8(int mode, int echo_check)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_V8, CMD_SYS_SPECIAL_SYSTEMRESET_V8, 0x00};
	uint8_t	val[4] = {SYSTEM_RESET_VALUE_V8, };
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	if (mode == SYSTEM_RESET_SOFT_V8)
	{
		fts_write_reg_v8(&regAdd[0], reg_leng);
	}
	else
	{
		fts_write_regU32_v8(FTS_CMD_HW_REG_W_V8, FTS_ADDR_SYSTEM_RESET_V8, BITS_32, val, 1);
	}

	if (echo_check == ENABLE)
	{
		err_code = fts_cmd_completion_check_v8(EVTID_CONTROLLER_READY_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50);
		if (err_code != FTS_NO_ERR_V8)
		{
			if(err_code == (FTS_ERR_V8 | FTS_ERR_I2C_V8))
			{
				FUNC_END();
				return err_code;
			}
			else
			{
				err_code |= (err_code | FTS_ERR_SYS_RST_V8);
			}
			FUNC_END();
			return	err_code;
		}
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Clear Event FIFO
  * @param  None
  * @retval None
  */
int fts_clear_FIFO_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_V8, CMD_SYS_SPECIAL_CLEAR_FIFO_V8, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50);

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Interrupt enable or disable
  * @param  onoff	: ENABLE or DISABLE
  * @retval None
  */
int fts_interrupt_control_v8(int onoff)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_INTB_V8, 0x00, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;
	int		ret = 0;

	FUNC_BEGIN();
	if (onoff == ENABLE)
		regAdd[2] |= 0x01;

	ret = fts_write_reg_v8(&regAdd[0], reg_leng);
	if (ret != 0)
	{
		printf("%s: fts_write_reg_v8 error\n", __func__);
	}
	err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50);

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Read chip id
  * @param  None
  * @retval status	: TRUE if Chip ID is okay, FALSE if not.
  */
int fts_read_chip_id_v8(void)
{
	uint8_t	val[8] = {0};
	int		retry = 10;
	int		err_code = FTS_NO_ERR_V8;
	int		ret = 0;

	FUNC_BEGIN();
	while (retry--)
	{
		fts_delay_v8(10);
		ret = fts_read_regU32_v8(FTS_CMD_HW_REG_W_V8, FTS_ADDR_CHIP_ID_V8, BITS_32, (uint8_t *) val, FTS_EVENT_SIZE_V8);
		if(ret == (FTS_ERR_V8 | FTS_ERR_I2C_V8))
		{
			FUNC_END();
			return	(FTS_ERR_V8 | FTS_ERR_I2C_V8);
		}
		if ((val[0] == FTS_ID0_V8) && (val[1] == FTS_ID1_V8))
		{
			if ((val[4] == 0x00) && (val[5] == 0x00))
			{
				printf("\r\n[ChipID] No FW [%02x %02x %02x %02x %02x %02x %02x %02x]",
							val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
				err_code |= (FTS_ERR_V8 | FTS_ERR_CHIPID_V8);
				FUNC_END();
				return	err_code;
			}
			else
			{
				printf("\r\n[ChipID] OK [%02x %02x %02x %02x %02x %02x %02x %02x]",
							val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
				FUNC_END();
				return	err_code;
			}
		}
	}

	if (retry <= 0)
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_EVT_TIMEOVER_V8);
	}

	FUNC_END();
	return	err_code;
}

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V8

/**
  * @brief  Poll the status of flash if the Flash becomes ready within a timeout.
  * @param  type
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int wait_for_flash_ready_v8(uint8_t type)
{
    uint8_t	cmd[5] = {FTS_CMD_HW_REG_R_V8, 0x20, 0x00, 0x00, type};
    uint8_t	readData[2] = {0};
    int		i, retry = 30000;

	FUNC_BEGIN();
    while (retry > 0)
    {
    	fts_delay_v8(10);
    	fts_read_reg_v8(cmd, 5, readData, 1);
    	if ((readData[0] & 0x80) == 0)
    	{
			FUNC_END();
    		return	FTS_NO_ERR_V8;
    	}
    	retry--;
    }
   	printf("\r\n[flash] Ready Timeout!");

	FUNC_END();
    return	(FTS_ERR_V8 | FTS_ERR_EVT_TIMEOVER_V8);
}

/**
  * @brief  Put the M3 in hold.
  * @param  None
  * @retval
  */
int hold_m3_v8(void)
{
    uint8_t	cmd[4] = {SYSTEM_HOLD_M3_VALUE_V8, 0};

	FUNC_BEGIN();
	fts_write_regU32_v8(FTS_CMD_HW_REG_W_V8, FTS_ADDR_SYSTEM_RESET_V8, BITS_32, cmd, 1);
	fts_delay_v8(10);

	FUNC_END();
    return	FTS_NO_ERR_V8;
}

/**
  * @brief  dma operation.
  * @param  None
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int start_flash_dma_v8()
{
    int		status = FTS_NO_ERR_V8;
    uint8_t	cmd[6] = {FTS_CMD_HW_REG_W_V8, 0x20, 0x00, 0x00, FLASH_DMA_CODE0_V8, FLASH_DMA_CODE1_V8};

	FUNC_BEGIN();
    fts_write_reg_v8(cmd, 6);

    status = wait_for_flash_ready_v8(FLASH_DMA_CODE0_V8);
    if (status != FTS_NO_ERR_V8)
    {
		FUNC_END();
        return	status;
    }

	FUNC_END();
    return	status;
}

/**
  * @brief  Fill the flash
  * @param  address		: Start address to fill
  * @param	data		: A pointer of binary file
  * @param	size		: Size of data to fill
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int fillFlash_v8(uint32_t address, uint8_t *data, int size)
{
    int		remaining, index = 0;
    int		toWrite = 0;
    int		byteBlock = 0;
    int		wheel = 0;
    uint32_t	addr = 0;
    int		res = FTS_NO_ERR_V8;
    int		delta;

    uint8_t		buff[DMA_CHUNK_V8 + 5] = {0};
    uint8_t		buff2[12] = {0};

	FUNC_BEGIN();
    remaining = size;
    while (remaining > 0)
    {
		byteBlock = 0;
		addr = 0x00100000;
		printf("\r\n[flash]Write to memory [%d]", wheel);
        while (byteBlock < FLASH_CHUNK_V8 && remaining > 0)
        {
        	index = 0;
            if (remaining >= DMA_CHUNK_V8)
            {
                if ((byteBlock + DMA_CHUNK_V8) <= FLASH_CHUNK_V8)
                {
                    toWrite = DMA_CHUNK_V8;
                    remaining -= DMA_CHUNK_V8;
                    byteBlock += DMA_CHUNK_V8;
                }
                else
                {
                    delta = FLASH_CHUNK_V8 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }
            else
            {
                if ((byteBlock + remaining) <= FLASH_CHUNK_V8)
                {
                    toWrite = remaining;
                    byteBlock += remaining;
                    remaining = 0;

                }
                else
                {
                    delta = FLASH_CHUNK_V8 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }

			buff[index++] = FTS_CMD_HW_REG_W_V8;
			buff[index++] = (uint8_t) ((addr & 0xFF000000) >> 24);
			buff[index++] = (uint8_t) ((addr & 0x00FF0000) >> 16);
			buff[index++] = (uint8_t) ((addr & 0x0000FF00) >> 8);
            buff[index++] = (uint8_t) (addr & 0x000000FF);
            memcpy(&buff[index], data, toWrite);
            fts_write_reg_v8(buff, index + toWrite);
            fts_delay_v8(1);

            addr += toWrite;
            data += toWrite;
        }

        //configuring the DMA
		printf("\r\n[flash]Configure DMA [%d]", wheel);
        byteBlock = (byteBlock / 4) - 1;
        index = 0;

        buff2[index++] = FTS_CMD_HW_REG_W_V8;
		buff2[index++] = 0x20;
        buff2[index++] = 0x00;
		buff2[index++] = 0x00;
        buff2[index++] = FLASH_DMA_CONFIG_V8;
        buff2[index++] = 0x00;
        buff2[index++] = 0x00;
		addr = address + ((wheel * FLASH_CHUNK_V8) / 4);
        buff2[index++] = (uint8_t) ((addr & 0x000000FF));
        buff2[index++] = (uint8_t) ((addr & 0x0000FF00) >> 8);
        buff2[index++] = (uint8_t) (byteBlock & 0x000000FF);
        buff2[index++] = (uint8_t) ((byteBlock & 0x0000FF00) >> 8);
		buff2[index++] = 0x00;

        fts_write_reg_v8(buff2, index);
        fts_delay_v8(10);

        printf("\r\n[flash]Start flash DMA [%d]", wheel);
        res = start_flash_dma_v8();
        if (res != FTS_NO_ERR_V8)
        {
        	printf("\r\n[flash]Error flashing DMA!");            return	res;
        }
        fts_delay_v8(100);
		printf("\r\n[flash]DMA done [%d]", wheel);

        wheel++;
    }
    return	res;
}

/**
  * @brief  Download a firmware to flash.
  * @param  pFilename	: A pointer of buffer for a read file.
  * @param	block_type
  * @param	system_reset
  * @retval None
  */
int fw_download_v8(uint8_t *pFilename, FW_FTB_HEADER *fw_Header)
{
	uint32_t	FTS_TOTAL_SIZE = (256 * 1024);
	int			HEADER_DATA_SIZE = 32;

	int			err_flag = FTS_NO_ERR_V8;
	uint8_t		regAdd[8] = {0};

	FUNC_BEGIN();
	printf("\r\n[flash]1. System Reset");	if ((err_flag = fts_systemreset_v8(SYSTEM_RESET_HARD_V8, DISABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[flash]Failed - system reset");
		FUNC_END();
		return	err_flag;
	}
	fts_delay_v8(10);

	printf("\r\n[flash]2. Hold M3");	hold_m3_v8();

	printf("\r\n[flash]3. Flash Unlock");	regAdd[0] = FTS_CMD_HW_REG_W_V8;	regAdd[1] = 0x20;	regAdd[2] = 0x00;	regAdd[3] = 0x00;	regAdd[4] = FLASH_UNLOCK_CODE0_V8;	regAdd[5] = FLASH_UNLOCK_CODE1_V8;
	fts_write_reg_v8(&regAdd[0], 6);
	fts_delay_v8(50);

	printf("\r\n[flash]4. Flash Erase Unlock");	regAdd[0] = FTS_CMD_HW_REG_W_V8;	regAdd[1] = 0x20;	regAdd[2] = 0x00;	regAdd[3] = 0x00;	regAdd[4] = FLASH_ERASE_UNLOCK_CODE0_V8;	regAdd[5] = FLASH_ERASE_UNLOCK_CODE1_V8;
	fts_write_reg_v8(&regAdd[0], 6);
	fts_delay_v8(30);

	printf("\r\n[flash]5. Flash Erase");	regAdd[0] = FTS_CMD_HW_REG_W_V8;	regAdd[1] = 0x20;	regAdd[2] = 0x00;	regAdd[3] = 0x00;	regAdd[4] = FLASH_ERASE_CODE0_V8 + 1;	regAdd[5] = 0x00;
	fts_write_reg_v8(&regAdd[0], 6);
	fts_delay_v8(10);

	regAdd[0] = FTS_CMD_HW_REG_W_V8;	regAdd[1] = 0x20;	regAdd[2] = 0x00;	regAdd[3] = 0x00;	regAdd[4] = FLASH_ERASE_CODE0_V8;	regAdd[5] = FLASH_ERASE_CODE1_V8;
	fts_write_reg_v8(&regAdd[0], 6);
	fts_delay_v8(10);

	err_flag = wait_for_flash_ready_v8(FLASH_ERASE_CODE0_V8);
	if (err_flag != FTS_NO_ERR_V8)
	{
        err_flag |= (FTS_ERR_FLASH_ERASE_V8);
		printf("\r\n[flash]Error - Flash Erase");

		FUNC_END();
        return	err_flag;
    }
	fts_delay_v8(200);

	printf("\r\n[flash]Program sec0:%d", fw_Header->sec0_size);
	err_flag = fillFlash_v8(FLASH_ADDR_CODE_V8, (pFilename + FW_HEADER_SIZE_V8), fw_Header->sec0_size);
	if (err_flag != FTS_NO_ERR_V8)
	{
		err_flag |= (FTS_ERR_V8 | FTS_ERR_FLASH_BURN_V8);
		printf("\r\n[flash]Error - sec0");
		FUNC_END();
		return	err_flag;
	}
	fts_delay_v8(100);
	printf("\r\n[flash]load sec0 program DONE!");

	printf("\r\n[flash]Program sec1:%d", fw_Header->sec1_size);
	err_flag = fillFlash_v8(FLASH_ADDR_CONFIG_V8, (pFilename + FW_HEADER_SIZE_V8 + fw_Header->sec0_size), fw_Header->sec1_size);
	if (err_flag != FTS_NO_ERR_V8)
	{
		err_flag |= (FTS_ERR_V8 | FTS_ERR_FLASH_BURN_V8);
		printf("\r\n[flash]Error - sec1");

		FUNC_END();
		return	err_flag;
	}
	fts_delay_v8(100);
	printf("\r\n[flash]load sec1 program DONE!");

	printf("\r\n[flash]Program sec2:%d", fw_Header->sec2_size);
	if (fw_Header->sec2_size !=0)
	{
		err_flag = fillFlash_v8(FLASH_ADDR_CX_V8, (pFilename + FW_HEADER_SIZE_V8 + fw_Header->sec0_size + fw_Header->sec1_size), fw_Header->sec2_size);
		if (err_flag != FTS_NO_ERR_V8)
		{
			err_flag |= (FTS_ERR_V8 | FTS_ERR_FLASH_BURN_V8);
			printf("\r\n[flash]Error - sec2");

			FUNC_END();
			return	err_flag;
		}
		fts_delay_v8(100);
		printf("\r\n[flash]load sec2 program DONE!");
	}

	printf("\r\n[flash]7. System Reset");	err_flag = fts_systemreset_v8(SYSTEM_RESET_HARD_V8, ENABLE);
	if (err_flag != FTS_NO_ERR_V8)
	{
		err_flag |= (FTS_ERR_V8 | FTS_ERR_SYS_RST_V8 | FTS_ERR_FLASH_BURN_V8);
		printf("\r\n[flash]Error - System Reset");
		FUNC_END();
		return	err_flag;
	}

	printf("\r\n[flash]8. Read SysInfo");	err_flag = fts_read_sysInfo_v8();
	if (err_flag != FTS_NO_ERR_V8)
	{
		printf("\r\n[flash]Error - Read sysInfo");		printf("\r\n[flash]code - %08X", err_flag);		err_flag |= (FTS_ERR_V8 | FTS_ERR_FLASH_BURN_V8);
	}

	FUNC_END();
	return	err_flag;
}

/**
  * @brief  Parsing the header of firmware binary file
  * @param  data		: A pointer of binary file
  * @param	fw_size		: Size of binary file
  * @param	fw_header	: A pointer of header parsing
  * @param	keep_cx		: Always '0'
  * @retval TRUE if all operation done correctly, FALSE if not..
  */
int parseBinFile_v8(uint8_t *data, int fw_size, FW_FTB_HEADER *fw_header, int keep_cx)
{
	int			dimension;
	uint32_t	temp;
	int			res, file_type;
	int			err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	memcpy(fw_header, &data[0], FW_HEADER_SIZE_V8);

	if (fw_header->signature != FW_HEADER_FTB_SIGNATURE_V8)
	{
		printf("\r\n[parseBinFile] Wrong FW Signature %08X", fw_header->signature);
		err_code |= (FTS_ERR_V8 | FTS_ERR_BIN_TYPE_V8);
		FUNC_END();
		return	err_code;
	}

	if (fw_header->target != FW_HEADER_TARGET_ID_V8)
	{
		printf("\r\n[parseBinFile] Wrong target version %08X ... ERROR", fw_header->target);
		err_code |= (FTS_ERR_V8 | FTS_ERR_BIN_TYPE_V8);
		FUNC_END();
		return	err_code;
	}

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

    if (dimension + FW_HEADER_SIZE_V8 + FW_BYTES_ALLIGN_V8 != temp)
    {
    	printf("\r\n[parseBinFile] Read only %d instead of %d... ERROR", fw_size, dimension + FW_HEADER_SIZE_V8 + FW_BYTES_ALLIGN_V8);
		err_code |= (FTS_ERR_V8 | FTS_ERR_BIN_TYPE_V8);
		FUNC_END();
		return	err_code;
    }

	FUNC_END();
	return	err_code;
}

/**
  * @brief  A buffer for saving a binary file (firmware)
  */
//const	uint8_t	fw_bin_data[131072] = {0, 1};

/**
  * @brief  Manage flash update procedure.
  * @param  force		: Always '0'
  * @param	keep_cx		: Always '0'
  * @retval TRUE if all operation done correctly, FALSE if not.
  */
int flashProcedure_v8(int force, int keep_cx)
{
	uint8_t			*pFilename = NULL;
	int				fw_size;
	int				err_code;

	FW_FTB_HEADER	fw_ftbHeader;

	FUNC_BEGIN();
	/* A pointer and size of buffer for binary file */
	pFilename = fw_bin_data;
	fw_size = sizeof(fw_bin_data);

	err_code = parseBinFile_v8(pFilename, fw_size, &fw_ftbHeader, keep_cx);
	if (err_code != FTS_NO_ERR_V8)
	{
		printf("\r\n[flash]Error - FW is not appreciate");
		FUNC_END();
		return	err_code;
	}
	printf("\r\nVer,E:%04X,F:%04X,C:%04X", fw_ftbHeader.ext_ver, fw_ftbHeader.fw_ver, fw_ftbHeader.cfg_ver);
	printf("\r\nSize,Sec0:%d,Sec1:%d", fw_ftbHeader.sec0_size, fw_ftbHeader.sec1_size);
	printf("\r\nSize,Sec2:%d,Sec3:%d", fw_ftbHeader.sec2_size, fw_ftbHeader.sec3_size);

	err_code = fw_download_v8(pFilename, &fw_ftbHeader);
	if (err_code != FTS_NO_ERR_V8)
	{
		printf("\r\n[flash]Error - Firmware update is not completed.");
		FUNC_END();
		return	err_code;
	}
	printf("\r\n[flash]Firmware update is done successfully.");

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

#endif

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_read_sysInfo_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_LOAD_DATA_MEM_V8, LOAD_SYS_INFO_V8, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	pSysInfo = (SysInfo *) sysInfo_buf;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50);
	if (err_code != FTS_NO_ERR_V8)
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_EVT_TIMEOVER_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}

	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, ADDR_FRAMEBUFFER_V8, BITS_16, sysInfo_buf, SYS_INFO_SIZE_V8);
	if ((pSysInfo->header != HEADER_SIGNATURE_V8) || (pSysInfo->host_data_mem_id != LOAD_SYS_INFO_V8))
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_get_ms_comp_data_v8(uint8_t memory_id, uint8_t *pbuf_data)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_LOAD_DATA_MEM_V8, 0x00, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;
	int		i;
	uint8_t	temp_header[COMP_DATA_HEADER_SIZE_V8];
	uint8_t	*temp_buf;
	MsCompHeader	*ptHeader;

	FUNC_BEGIN();
	regAdd[2] = memory_id;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(10);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_get_ms_totalcomp_data]FAILED - MUTUAL");
		err_code  |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}

	/* Read the header of compensation data */
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, ADDR_FRAMEBUFFER_V8, BITS_16, temp_header, COMP_DATA_HEADER_SIZE_V8);
	ptHeader = (MsCompHeader *) temp_header;
	if ((ptHeader->header != HEADER_SIGNATURE_V8) || (ptHeader->host_data_mem_id != memory_id))
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}
	temp_buf = (uint8_t *) malloc(ptHeader->force_leng * ptHeader->sense_leng * sizeof(uint8_t));
	if (temp_buf == NULL)
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
		free(temp_buf);
		FUNC_END();
		return	err_code;
	}

	/* Read the data */
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, ADDR_FRAMEBUFFER_V8 + COMP_DATA_HEADER_SIZE_V8, BITS_16, temp_buf, ptHeader->force_leng * ptHeader->sense_leng * sizeof(uint8_t));
	for (i = 0; i < ptHeader->force_leng * ptHeader->sense_leng; i++)
	{
		pbuf_data[i] = temp_buf[i];
	}
	free(temp_buf);

	FUNC_END();
	return	err_code;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_get_ss_totalcomp_data_v8(uint8_t memory_id, uint16_t *pbuf_tx, uint16_t *pbuf_rx)
{
	uint8_t		regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_LOAD_DATA_MEM_V8, 0x00, 0x00};
	int			reg_leng = 3, tempAddr;
	int			i = 0, err_code = FTS_NO_ERR_V8;
	uint8_t		temp_header[COMP_DATA_HEADER_SIZE_V8];
	uint8_t		*temp_buf_tx, *temp_buf_rx;
	SsCompHeader	*ptHeader;

	FUNC_BEGIN();
	regAdd[2] = memory_id;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(10);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_get_ss_totalcomp_data]FAILED");
		err_code  |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}

	/* Read the header of compensation data */
	tempAddr = ADDR_FRAMEBUFFER_V8;
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, temp_header, COMP_DATA_HEADER_SIZE_V8);
	ptHeader = (SsCompHeader *) temp_header;
	if ((ptHeader->header != HEADER_SIGNATURE_V8) || (ptHeader->host_data_mem_id != memory_id))
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}
	temp_buf_tx = (uint8_t *) malloc(ptHeader->force_leng * sizeof(uint16_t));
	temp_buf_rx = (uint8_t *) malloc(ptHeader->sense_leng * sizeof(uint16_t));
	if ((temp_buf_tx == NULL) || (temp_buf_rx == NULL))
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
		free(temp_buf_tx);
		free(temp_buf_rx);
		FUNC_END();
		return	err_code;
	}

	/* Load the tx data */
	tempAddr += COMP_DATA_HEADER_SIZE_V8;
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, temp_buf_tx, ptHeader->force_leng * sizeof(uint16_t));
	for (i = 0; i < ptHeader->force_leng; i++)
	{
		pbuf_tx[i] = ((uint16_t) temp_buf_tx[i * 2 + 1] << 8) | (uint16_t) temp_buf_tx[i * 2];
	}
	/* Load the rx data */
	tempAddr += ptHeader->force_leng * sizeof(uint16_t);
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, temp_buf_rx, ptHeader->sense_leng * sizeof(uint16_t));
	for (i = 0; i < ptHeader->sense_leng; i++)
	{
		pbuf_rx[i] = ((uint16_t) temp_buf_rx[i * 2 + 1] << 8) | (uint16_t) temp_buf_rx[i * 2];
	}
	free(temp_buf_tx);
	free(temp_buf_rx);

	FUNC_END();
	return	err_code;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_get_SyncFrame_data_v8(uint8_t memory_id, int16_t *pbuf_ms, int16_t *pbuf_ss_tx, int16_t *pbuf_ss_rx, int16_t *pbuf_key)
{
	uint8_t		regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_LOAD_DATA_MEM_V8, 0x00, 0x00};
	int			reg_leng = 3, tempAddr;
	int			i = 0, retry = 10, err_code = FTS_NO_ERR_V8;
	uint8_t		temp_header[COMP_DATA_HEADER_SIZE_V8];
	uint8_t		*tmp_buf_ms, *tmp_buf_ss_tx, *tmp_buf_ss_rx, *tmp_buf_key;
	SyncFrameHeader	*ptHeader;

	FUNC_BEGIN();
	regAdd[2] = memory_id;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(50);

	/* Read the header of compensation data */
	tempAddr = ADDR_FRAMEBUFFER_V8;
	ptHeader = (SyncFrameHeader *) temp_header;
	for (i = 0; i < retry; i++)
	{
		fts_delay_v8(50);
		fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, temp_header, COMP_DATA_HEADER_SIZE_V8);
		if ((ptHeader->header == HEADER_SIGNATURE_V8) || (ptHeader->host_data_mem_id == memory_id))
		{
			break;
		}
	}
	if (i >= retry)
	{
		err_code |= (FTS_ERR_V8 | FTS_ERR_HOSTDATA_ID_HD_V8);
		FUNC_END();
		return	err_code;
	}
	printf("\r\n[SyncFrame]Tx length:%02X, Rx length:%02X, key length:%02X", ptHeader->ms_force_leng, ptHeader->ms_sense_leng, ptHeader->key_leng);

	if (((ptHeader->ms_force_leng > 0) || (ptHeader->ms_sense_leng > 0)) && (pbuf_ms != NULL))
	{
		tmp_buf_ms = (uint8_t *) malloc(ptHeader->ms_force_leng * ptHeader->ms_sense_leng * sizeof(uint16_t));
		if (tmp_buf_ms == NULL)
		{
			err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
			free(tmp_buf_ms);
			FUNC_END();
			return	err_code;
		}
		/* Load the tx data */
		tempAddr += COMP_DATA_HEADER_SIZE_V8 + ptHeader->dbg_frm_leng;
		fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, tmp_buf_ms, ptHeader->ms_force_leng * ptHeader->ms_sense_leng * sizeof(uint16_t));
		for (i = 0; i < ptHeader->ms_force_leng * ptHeader->ms_sense_leng; i++)
		{
			pbuf_ms[i] = ((uint16_t) tmp_buf_ms[i * 2 + 1] << 8) | (uint16_t) tmp_buf_ms[i * 2];
		}
		free(tmp_buf_ms);
	}

	/* Load self raw data of tx */
	if ((ptHeader->ss_force_leng > 0) && (pbuf_ss_tx != NULL))
	{
		tmp_buf_ss_tx = (uint8_t *) malloc(ptHeader->ss_force_leng * sizeof(uint16_t));
		if (tmp_buf_ss_tx == NULL)
		{
			err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
			free(tmp_buf_ss_tx);
			FUNC_END();
			return	err_code;
		}
		/* Load the tx data */
		tempAddr += ptHeader->ms_force_leng * ptHeader->ms_sense_leng * sizeof(uint16_t);
		fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, tmp_buf_ss_tx, ptHeader->ss_force_leng * sizeof(uint16_t));
		for (i = 0; i < ptHeader->ss_force_leng; i++)
		{
			pbuf_ss_tx[i] = ((uint16_t) tmp_buf_ss_tx[i * 2 + 1] << 8) | (uint16_t) tmp_buf_ss_tx[i * 2];
		}
		free(tmp_buf_ss_tx);
	}

	/* Load self raw data of rx */
	if ((ptHeader->ss_sense_leng > 0) && (pbuf_ss_rx != NULL))
	{
		tmp_buf_ss_rx = (uint8_t *) malloc(ptHeader->ss_sense_leng * sizeof(uint16_t));
		if (tmp_buf_ss_rx == NULL)
		{
			err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
			free(tmp_buf_ss_rx);
			FUNC_END();
			return	err_code;
		}
		/* Load the tx data */
		tempAddr += ptHeader->ss_force_leng * sizeof(uint16_t);
		fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, tmp_buf_ss_rx, ptHeader->ss_sense_leng * sizeof(uint16_t));
		for (i = 0; i < ptHeader->ss_sense_leng; i++)
		{
			pbuf_ss_rx[i] = ((uint16_t) tmp_buf_ss_rx[i * 2 + 1] << 8) | (uint16_t) tmp_buf_ss_rx[i * 2];
		}
		free(tmp_buf_ss_rx);
	}

	/* Load key raw data */
	if ((ptHeader->key_leng > 0) && (pbuf_key != NULL))
	{
		tmp_buf_key = (uint8_t *) malloc(ptHeader->key_leng * sizeof(uint16_t));
		if (tmp_buf_key == NULL)
		{
			err_code |= (FTS_ERR_V8 | FTS_ERR_MEM_ALLC_V8);
			free(tmp_buf_key);
			FUNC_END();
			return	err_code;
		}
		/* Load the tx data */
		tempAddr += ptHeader->ss_sense_leng * sizeof(uint16_t);
		fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempAddr, BITS_16, tmp_buf_key, ptHeader->key_leng * sizeof(uint16_t));
		for (i = 0; i < ptHeader->key_leng; i++)
		{
			pbuf_key[i] = ((uint16_t) tmp_buf_key[i * 2 + 1] << 8) | (uint16_t) tmp_buf_key[i * 2];
		}
		free(tmp_buf_key);
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_get_ms_frame_data_v8(MSFrameType ms_type, int16_t *pbuf_data, int tx_cnt, int rx_cnt)
{
	int			err_code = FTS_NO_ERR_V8;
	int			i;
	uint8_t		*temp_buf;
	uint16_t	tempOffsetAddr = 0;

	FUNC_BEGIN();
	switch (ms_type)
	{
	case MS_RAW_V8:
		tempOffsetAddr = pSysInfo->msTchRawAddr;
		break;
	case MS_FILTER_V8:
		tempOffsetAddr = pSysInfo->msTchFilterAddr;
		break;
	case MS_STRENGHT_V8:
		tempOffsetAddr = pSysInfo->msTchStrenAddr;
		break;
	case MS_BASELINE_V8:
		tempOffsetAddr = pSysInfo->msTchBaselineAddr;
		break;
	case MS_KEY_RAW_V8:
		tempOffsetAddr = pSysInfo->keyRawAddr;
		break;
	case MS_KEY_FILTER_V8:
		tempOffsetAddr = pSysInfo->keyFilterAddr;
		break;
	case MS_KEY_STRENGHT_V8:
		tempOffsetAddr = pSysInfo->keyStrenAddr;
		break;
	case MS_KEY_BASELINE_V8:
		tempOffsetAddr = pSysInfo->keyBaselineAddr;
		break;
	case FRC_RAW_V8:
		tempOffsetAddr = pSysInfo->frcRawAddr;
		break;;
	case FRC_FILTER_V8:
		tempOffsetAddr = pSysInfo->frcFilterAddr;
		break;;
	case FRC_STRENGHT_V8:
		tempOffsetAddr = pSysInfo->frcStrenAddr;
		break;;
	case FRC_BASELINE_V8:
		tempOffsetAddr = pSysInfo->frcBaselineAddr;
		break;
	default:
		break;
	}

	temp_buf = (uint8_t *) malloc(tx_cnt * rx_cnt * sizeof(uint16_t));

	/* Load the frame data */
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tempOffsetAddr, BITS_16, temp_buf, tx_cnt * rx_cnt * sizeof(uint16_t));
	for (i = 0; i < tx_cnt * rx_cnt; i++)
	{
		pbuf_data[i] = ((int16_t) temp_buf[i * 2 + 1] << 8) | (uint16_t) temp_buf[i * 2];
	}
	free(temp_buf);

	FUNC_END();
	return	err_code;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
int fts_get_ss_frame_data_v8(SSFrameType ss_type, int16_t *pbuf_tx, int16_t *pbuf_rx, int tx_cnt, int rx_cnt)
{
	int			err_code = FTS_NO_ERR_V8;
	int			i;
	uint8_t		*tmp_buf_tx, *tmp_buf_rx;
	uint16_t	tmpOffsetForce = 0, tmpOffsetSense = 0;

	FUNC_BEGIN();
	switch (ss_type)
	{
	case SS_RAW_V8:
		tmpOffsetForce = pSysInfo->ssTchTxRawAddr;
		tmpOffsetSense = pSysInfo->ssTchRxRawAddr;
		break;
	case SS_FILTER_V8:
		tmpOffsetForce = pSysInfo->ssTchTxFilterAddr;
		tmpOffsetSense = pSysInfo->ssTchRxFilterAddr;
		break;
	case SS_STRENGHT_V8:
		tmpOffsetForce = pSysInfo->ssTchTxStrenAddr;
		tmpOffsetSense = pSysInfo->ssTchRxStrenAddr;
		break;
	case SS_BASELINE_V8:
		tmpOffsetForce = pSysInfo->ssTchTxBaselineAddr;
		tmpOffsetSense = pSysInfo->ssTchRxBaselineAddr;
		break;
	case SS_HVR_RAW_V8:
		tmpOffsetForce = pSysInfo->ssHvrTxRawAddr;
		tmpOffsetSense = pSysInfo->ssHvrRxRawAddr;
		break;
	case SS_HVR_FILTER_V8:
		tmpOffsetForce = pSysInfo->ssHvrTxFilterAddr;
		tmpOffsetSense = pSysInfo->ssHvrRxFilterAddr;
		break;
	case SS_HVR_STRENGHT_V8:
		tmpOffsetForce = pSysInfo->ssHvrTxStrenAddr;
		tmpOffsetSense = pSysInfo->ssHvrRxStrenAddr;
		break;
	case SS_HVR_BASELINE_V8:
		tmpOffsetForce = pSysInfo->ssHvrTxBaselineAddr;
		tmpOffsetSense = pSysInfo->ssHvrRxBaselineAddr;
		break;
	case SS_PRX_RAW_V8:
		tmpOffsetForce = pSysInfo->ssPrxTxRawAddr;
		tmpOffsetSense = pSysInfo->ssPrxRxRawAddr;
		break;
	case SS_PRX_FILTER_V8:
		tmpOffsetForce = pSysInfo->ssPrxTxFilterAddr;
		tmpOffsetSense = pSysInfo->ssPrxRxFilterAddr;
		break;
	case SS_PRX_STRENGHT_V8:
		tmpOffsetForce = pSysInfo->ssPrxTxStrenAddr;
		tmpOffsetSense = pSysInfo->ssPrxRxStrenAddr;
		break;
	case SS_PRX_BASELINE_V8:
		tmpOffsetForce = pSysInfo->ssPrxTxBaselineAddr;
		tmpOffsetSense = pSysInfo->ssPrxRxBaselineAddr;
		break;
	default:
		break;
	}

	tmp_buf_tx = (uint8_t *) malloc(tx_cnt * sizeof(uint16_t));
	tmp_buf_rx = (uint8_t *) malloc(rx_cnt * sizeof(uint16_t));

	/* Load the frame data */
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tmpOffsetForce, BITS_16, tmp_buf_tx, tx_cnt * sizeof(uint16_t));
	for (i = 0; i < tx_cnt; i++)
	{
		pbuf_tx[i] = ((uint16_t) tmp_buf_tx[i * 2 + 1] << 8) | (uint16_t) tmp_buf_tx[i * 2];
	}
	/* Load the frame data */
	fts_read_regU32_v8(CMD_FRM_BUFF_R_V8, tmpOffsetSense, BITS_16, tmp_buf_rx, rx_cnt * sizeof(uint16_t));
	for (i = 0; i < rx_cnt; i++)
	{
		pbuf_rx[i] = ((uint16_t) tmp_buf_rx[i * 2 + 1] << 8) | (uint16_t) tmp_buf_rx[i * 2];
	}
	free(tmp_buf_tx);
	free(tmp_buf_rx);

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Full panel initialization included auto-tune and saving flash.
  * @param  None
  * @retval None
  */
int fts_do_LPTimeCalib_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_TUNE_CMD_V8, CMD_SYS_SPECIAL_TUNE_CMD_LPTIMER_CAL_V8, 0x00};
	int		reg_leng = 4;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50)) != FTS_NO_ERR_V8)
	{
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Full panel initialization included auto-tune and saving flash.
  * @param  None
  * @retval None
  */
int fts_do_Ioffset_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_TUNE_CMD_V8, CMD_SYS_SPECIAL_TUNE_CMD_IOFFSET_TUNE_V8, 0x00};
	int		reg_leng = 4;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(300);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50)) != FTS_NO_ERR_V8)
	{
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Full panel initialization included auto-tune and saving flash.
  * @param  None
  * @retval None
  */
int fts_do_FullPanelInit_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_V8, CMD_SYS_SPECIAL_FULLPANEL_INIT_V8, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(500);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 300)) != FTS_NO_ERR_V8)
	{
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Panel initialization.
  * @param  None
  * @retval None
  */
int fts_do_PanelInit_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_V8, CMD_SYS_SPECIAL_PANEL_INIT_V8, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(500);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 300)) != FTS_NO_ERR_V8)
	{
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Initialize operations with Auto-tune sequence for LGD FSR.
  * 		This function has to be run a time after firmware update.
  * @param  None
  * @retval None
  */
int fts_do_autotune_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_ATUNE_V8, 0x00, 0x00};
	int		reg_leng = 4;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	err_code = fts_do_LPTimeCalib_v8();
	if (err_code != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_do_autotune]FAILED - LPTImer Calibration");
		FUNC_END();
		return	err_code;
	}
	
	err_code = fts_do_Ioffset_v8();
	if (err_code != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_do_autotune]FAILED - LPTImer Calibration");
		FUNC_END();
		return	err_code;
	}
	
	regAdd[2] = ((CMD_SYS_SCANTYPE_MUTUAL_V8 | CMD_SYS_SCANTYPE_MUTUAL_LP_V8) >> 8) & 0xFF;
	regAdd[3] = (CMD_SYS_SCANTYPE_MUTUAL_V8 | CMD_SYS_SCANTYPE_MUTUAL_LP_V8) & 0xFF;
	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(500);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 300)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_do_autotune]FAILED - MUTUAL");
		FUNC_END();
		return	err_code;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V8
	regAdd[2] = ((CMD_SYS_SCANTYPE_SELF_V8 | CMD_SYS_SCANTYPE_SELF_LP_V8) >> 8) & 0xFF;
	regAdd[3] = (CMD_SYS_SCANTYPE_SELF_V8 | CMD_SYS_SCANTYPE_SELF_LP_V8) & 0xFF;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(500);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 300)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_do_autotune]FAILED - SELF");
		FUNC_END();
		return	err_code;
	}
#endif

	FUNC_END();
	return	err_code;
}

/* add hyelim 180314 - from FTM5v09 */
/**
  * @brief  Save the config to Flash.
  * @param  None
  * @retval None
  */
int fts_save_config_v8(void)
{
    uint8_t regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SAVE2FLASH_V8, CMD_SYS_SAVE2FLASH_FWCONFIG_V8, 0x00};
    int     reg_leng = 3;
    int     status = FTS_NO_ERR_V8;

	FUNC_BEGIN();
    fts_write_reg_v8(&regAdd[0], reg_leng);
    fts_delay_v8(200);
    if ((status = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 100)) != FTS_NO_ERR_V8)
    {
        status |= (FTS_ERR_SAVE_COMP_V8 | FTS_ERR_V8);
		FUNC_END();
        return  status;
    }

	FUNC_END();
    return  status;
}

/* END - add hyelim 180314 - from FTM5v09 */

#ifdef FTS_SUPPORT_SAVE_COMP_V8

/**
  * @brief  Save the compensation data to Flash.
  * @param  None
  * @retval None
  */
int fts_save2flash_compensation_v8(void)
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_SAVE2FLASH_V8, CMD_SYS_SAVE2FLASH_CX_V8, 0x00};
	int		reg_leng = 3;
	int		status = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(200);
	if ((status = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 100)) != FTS_NO_ERR_V8)
	{
		status |= (FTS_ERR_SAVE_COMP_V8 | FTS_ERR_V8);
		FUNC_END();
		return	status;
	}

	FUNC_END();
	return	status;
}

#endif

/**
  * @brief  Initialize operations with Auto-tune sequence for LGD FSR.
  * 		This function has to be run a time after firmware update.
  * @param  None
  * @retval None
  */
int fts_scan_mode_control_v8(uint8_t mode_sel, uint8_t mode_setting, int onoff)
{
	uint8_t	regAdd[8] = {CMD_SCAN_MODE_V8, 0x00, 0x00, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;

	FUNC_BEGIN();
	if (mode_sel == CMD_SCAN_LPMODE_V8)
		reg_leng = 2;

	regAdd[1] = mode_sel;
	if (onoff == ENABLE)
		regAdd[2] = mode_setting;

	fts_write_reg_v8(&regAdd[0], reg_leng);
	fts_delay_v8(10);
	if ((err_code = fts_cmd_completion_check_v8(EVTID_STATUS_REPORT_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50)) != FTS_NO_ERR_V8)
	{
		FUNC_END();
		return	err_code;
	}

	FUNC_END();
	return	err_code;
}

/**
  * @brief  Check hw reset pin when jig is able to control this pin.
  * @param	None
  * @retval None
  */
int fts_hw_reset_pin_check_v8()
{
	uint8_t	regAdd[4] = {CMD_SYSTEM_V8, CMD_SYS_SPECIAL_V8, CMD_SYS_SPECIAL_SYSTEMRESET_V8, 0x00};
	int		reg_leng = 3;
	int		err_code = FTS_NO_ERR_V8;
	int		cnt = 50;
	int		tmp = 0;

	FUNC_BEGIN();
	if (fts_clear_FIFO_v8() != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_hw_reset_pin_check] Error-FIFO is not cleared");
		FUNC_END();
		return	FTS_ERR_V8;
	}

	{
		/*
		 * Drain down Reset pin to GND for 10ms by jig.
		 */

            tmp = 0;
            ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);
			usleep(10000);

            tmp = 1;
            ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &tmp);
            ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &tmp);

	}

	while (cnt--)
	{
		fts_delay_v8(10);
		err_code = fts_cmd_completion_check_v8(EVTID_CONTROLLER_READY_V8, EVTID_RPTTYPE_CMD_ECHO_V8, regAdd, reg_leng, 50);
		if (err_code == FTS_NO_ERR_V8)
		{
			break;
		}
	}

	if (cnt <= 0)
	{
		err_code |= (FTS_ERR_V8);
		printf("\r\n[fts_hw_reset_pin_check] Error-TimeOut");
	}

	FUNC_END();
	return	err_code;

}

/**
  * @brief  Check interrupt pin
  * @param	None
  * @retval None
  */
int fts_interrupt_pin_check_v8()
{
	uint8_t	regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_INTB_V8, 0x01, 0x00};
	int		reg_leng = 3;

	FUNC_BEGIN();
	if (fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, DISABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_interrupt_pin_check] Error-System Reset");
		FUNC_END();
		return	FTS_ERR_V8;
	}
	fts_delay_v8(50);
	fts_write_reg_v8(&regAdd[0], reg_leng);

	{
		/*
		 * Interrupt pin low check
		 *     If not low, return ERROR.
		 */
	}

	if (fts_clear_FIFO_v8() != FTS_NO_ERR_V8)
	{
		printf("\r\n[fts_interrupt_pin_check] Error-FIFO is not cleared");
		FUNC_END();
		return	FTS_ERR_V8;
	}
	fts_delay_v8(50);

	{
		/*
		 * Interrupt pin high check here
		 *     If not high, return ERROR.
		 */
	}

	FUNC_END();
	return	FTS_NO_ERR_V8;
}

#ifdef	FTS_SUPPORT_ITOTEST_V8

/* add hyelim 180314 - from FTM5v09 */

/**
  * @brief  Proceed the ITO TEST
  * @param  None
  * @retval None
  */
int fts_panel_ito_command_v8(void)
{
    uint8_t cmd_data[18] = {0x89, 0x86, 0x1A, 0x1F, 0x04, 0xD8, 0x08, 0x20, 0x00, 0x02, 0x00, 0x1F, 0x00, 0x24, 0x60, 0x06, 0x00, 0x00};
    int     data_leng = 18;
    int     err_flag = FTS_NO_ERR_V8;

	FUNC_BEGIN();
    if ((err_flag = fts_write_regU32_v8(CMD_CONFIG_W_V8, 0x00C8, BITS_16, cmd_data, data_leng)) != FTS_NO_ERR_V8)
    {
		FUNC_END();
        return  err_flag;
    }
	FUNC_END();
    return  err_flag;
}

/* END - add hyelim 180314 - from FTM5v09 */

/**
  * @brief  Proceed the ITO TEST
  * @param	None
  * @retval None
  */
int fts_panel_ito_test_v8(void)
{
	/*add hyelim 180306*/
    int count_min = 0;
    int count_max = 0;
	int fstat = 0;
	/*end*/
	uint8_t	val[8], regAdd[8] = {CMD_SYSTEM_V8, CMD_SYS_ITO_TEST_V8, 0xFF, 0x01};
	int		reg_leng = 4;
	int		cnt = 100;
	int		res = TRUE, err_flag = FTS_NO_ERR_V8;
	int		i;
	char	*errortypes[9] = {"Short - Force to GND", "Short - Sense to GND", "Short - Force to VDD", "Short - Sense to VDD",
								"Short - Force to Force", "Short - Sense to Sense", "Open - Force", "Open-Sense", "Open-KeyS2S"};

#ifdef	FTS_SUPPORT_HF_RAW_ADJ_V8
	int16_t		hf_raw[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8], hf_raw_vert[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8], hf_raw_horiz[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8];
	int		j, offset1, offset2;
#endif

	FUNC_BEGIN();
    /*
     * Commands for ITO OPEN (HF Raw adjacent node check)
     */
#if 0 //modify to erase, this sequence is for EVT2 sample. by healim  - 180514
	if ((res = fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[ITO_TEST]ERROR - System Reset");
		FUNC_END();
		return (res | FTS_ERR_HF_RAW_TEST_V8);
	}
/* add hyelim 180314 - from FTM5v09 */

    if ((res = fts_panel_ito_command_v8()) != FTS_NO_ERR_V8)
    {
        printf("\r\n[ITO_TEST]ERROR - ITO command");
		FUNC_END();
		return (res | FTS_ERR_HF_RAW_TEST_V8);
    }
    if ((res = fts_save_config_v8()) != FTS_NO_ERR_V8)
    {
        printf("\r\n[ITO_TEST]ERROR - Save the config");
		FUNC_END();
		return (res | FTS_ERR_HF_RAW_TEST_V8);
    }
    printf("\r\n[ITO_TEST]Save the config");
    fts_delay_v8(50);

/* 
 * 180514 modify to erase by healim
    if ((res = fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE)) != FTS_NO_ERR_V8)
    {
        printf("\r\n[ITO_TEST]ERROR - System Reset");
    }
*/
#endif
/* END -add hyelim 180314 - from FTM5v09 */

    printf("\n--------------------------------------\n");
	printf("[ TD17 : ITO SHORT TEST ]\n");
	fts_write_reg_v8(&regAdd[0], 4);
	fts_delay_v8(100);
	while (cnt--)
	{
		fts_delay_v8(5);
		fts_read_reg_v8(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V8);
		if ((val[0] == EVTID_STATUS_REPORT_V8) && (val[1] == EVTID_RPTTYPE_CMD_ECHO_V8))
		{
			res = TRUE;
			for (i = 0; i < reg_leng; i++)
			{
				if (val[i + 2] != regAdd[i])
					res = FALSE;
			}
			if (res == TRUE)
			{
				printf("\r\n[ITO_TEST] Finished [%02x %02x %02x %02x %02x %02x %02x %02x]",
							val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
				break;
			}
			else
			{
				printf("\r\n[ITO_TEST] [%02x %02x %02x %02x %02x %02x %02x %02x]",
							val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
			}
		}
		else if (val[0] == EVTID_ERROR_REPORT_V8)
		{
			switch (val[1])
			{
				case ITO_FORCE_SHRT_GND_V8:
				case ITO_SENSE_SHRT_GND_V8:
				case ITO_FORCE_SHRT_VDD_V8:
				case ITO_SENSE_SHRT_VDD_V8:
				case ITO_FORCE_SHRT_FORCE_V8:
				case ITO_SENSE_SHRT_SENSE_V8:
				case ITO_FORCE_OPEN_V8:
				case ITO_SENSE_OPEN_V8:
				case ITO_KEY_OPEN_V8:
					err_flag |= (FTS_ERR_V8 | FTS_ERR_ITO_TEST_V8);
					printf("\r\n[ITO_TEST] DETECTED-Error Type : %s, Channel : %d", errortypes[val[1]-0x60], val[2]);
					printf("\r\n -> [ITO_TEST] [%02x %02x %02x %02x %02x %02x %02x %02x]",
							val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
					break;
				default:
					printf("\r\n[ITO_TEST] INVALID [%02x %02x %02x %02x %02x %02x %02x %02x]",
								val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
					break;
			}
		}
	}

	if (cnt <= 0)
	{
		printf("\r\n[ITO_TEST] Time Over");
		err_flag |= FTS_ERR_EVT_TIMEOVER_V8;
	}
printf("1. ERR FLAG : 0x%X \n", err_flag);
    printf("\n--------------------------------------\n");

	//printf("[ TD19 : HF RAW GAP : RX]\n");
#ifdef	FTS_SUPPORT_HF_RAW_ADJ_V8
	memset(hf_raw, 0x00, pSysInfo->scrTxLen * pSysInfo->scrRxLen * sizeof(int16_t));
	if(FTS_NO_ERR_V8 !=  fts_get_ms_frame_data_v8(MS_RAW_V8, hf_raw, pSysInfo->scrTxLen, pSysInfo->scrRxLen))
	{
		printf("[ TD19 : HF RAW GAP : RX]\n");
		printf("[ TD19 : HF RAW GAP : TX]\n");
		printf("HF RAW GAP FAIL : Data Read Fail\n");	
		printf("\r\n[HF_RAW]ERROR- HF raw data");
		err_flag |= (FTS_ERR_V8 | FTS_ERR_HF_RAW_TEST_V8); //add - hyelim 180306
	}
	else
	{
		if(DEBUG_MODE)
		{
    		printf("\n--------------------------------------\n");
			printf("[ TD19 : HF RAW original Data]\n");

			for (i = 0; i < pSysInfo->scrTxLen; i++)
            {
	            for (j = 0; j < (pSysInfo->scrRxLen); j++)
    	        {
				    printf("%04d ",hf_raw[(i*(pSysInfo->scrRxLen)) + j]);

				}
				printf("\n");
			}
    		printf("\n--------------------------------------\n");
		}

		printf("[ TD19 : HF RAW GAP : RX]\n");
		
	    count_min = 0;
	    count_max = 0;

		//printf("Limit Data -> [MAX : %d] [MIN %d] \n",l_hf_gap_rx[1],l_hf_gap_rx[0]);

        for (i = 0; i < pSysInfo->scrTxLen; i++)
        {
            offset1 = i * pSysInfo->scrRxLen;
            for (j = 0; j < (pSysInfo->scrRxLen - 1); j++)
            {
                hf_raw_horiz[offset1 + j] = abs(hf_raw[offset1 + j] - hf_raw[offset1 + j + 1]);

                if(hf_raw_horiz[offset1 + j] > hf_TotalCx_Gap_Rx_MAX[i + 1][j + 1])
                {
                    count_max++;
                    printf("\033[1;31m%04d\033[m ",hf_raw_horiz[offset1 + j]);
                }
                else if(hf_raw_horiz[offset1 + j] < hf_TotalCx_Gap_Rx_MIN[i + 1][j + 1])
                {
                    count_min++;
                    printf("\033[1;34m%04d\033[m ",hf_raw_horiz[offset1 + j]);
                }
                else
                    printf("%04d ",hf_raw_horiz[offset1 + j]);
            }
            printf("\n");
        }

		if(count_max)
		{
		    printf("HF RAW DATA GAP(H) TOO HIGH FAIL [%d]\n",count_max);
			err_flag |= (FTS_ERR_V8 | FTS_ERR_HF_RAW_TEST_V8); //add - hyelim 180306
			fstat = 1;
		}
		if(count_min)
		{
		    printf("HF RAW DATA GAP(H) TOO LOW FAIL [%d]\n",count_min);
			err_flag |= (FTS_ERR_V8 | FTS_ERR_HF_RAW_TEST_V8); //add - hyelim 180306
			fstat = 1;
		}
		if(!fstat)
			printf("HF RAW DATA GAP(H) PASS\n");

		fstat = 0;

		printf("%d / %d \n",count_max,count_min);

		count_max = 0;
		count_min = 0;
	
	    printf("\n--------------------------------------\n");
		printf("[ TD19 : HF RAW GAP : TX]\n");
		//printf("no test! \n");
		//printf("Limit Data -> [MAX : %d] [MIN %d] \n",l_hf_gap_tx[1],l_hf_gap_tx[0]);

		// Calculate the vertical adjacent matrix of HF Raw
		for (i = 0; i < (pSysInfo->scrTxLen - 1); i++)
		{
			offset1 = i * pSysInfo->scrRxLen;
			offset2 = (i + 1) * pSysInfo->scrRxLen;
			for (j = 0; j < pSysInfo->scrRxLen; j++)
			{
				hf_raw_vert[offset1 + j] = abs(hf_raw[offset1 + j] - hf_raw[offset2 + j]);

                if(hf_raw_vert[offset1 + j] > hf_TotalCx_Gap_Tx_MAX[i + 1][j + 1])
                {
                    count_max++;
                    printf("\033[1;31m%04d\033[m ",hf_raw_vert[offset1 + j]);
                }
                else if(hf_raw_vert[offset1 + j] < hf_TotalCx_Gap_Tx_MIN[i + 1][j + 1])
                {
                    count_min++;
                    printf("\033[1;34m%04d\033[m ",hf_raw_vert[offset1 + j]);
                }
                else
                    printf("%04d ",hf_raw_vert[offset1 + j]);
            }
            printf("\n");

        }

        if(count_max)
        {
			//printf("no test! \n");
            printf("HF RAW DATA GAP(V) TOO HIGH FAIL [%d]\n",count_max);
            err_flag |= (FTS_ERR_V8 | FTS_ERR_HF_RAW_TEST_V8); //add - hyelim 180306
			fstat = 1;
        }
        if(count_min)
        {
			//printf("no test! \n");
            printf("HF RAW DATA GAP(V) TOO LOW FAIL [%d]\n",count_min);
            err_flag |= (FTS_ERR_V8 | FTS_ERR_HF_RAW_TEST_V8); //add - hyelim 180306
			fstat = 1;
        }
        if(!fstat)
            printf("HF RAW DATA GAP(V) PASS\n");


        printf("%d / %d \n",count_max,count_min);

		fstat = 0;
        count_max = 0;
        count_min = 0;
		
	}
#endif

	printf("2. ERR FLAG : 0x%X \n", err_flag);
	FUNC_END();
	return	err_flag;
}

#endif

/**
  * @brief  To inspect and check test items of the panel.
  * @param  None
  * @retval None
  */
int fts_panel_test_v8(int id, unsigned char *uart_buf)
{
	int			i, res;
//	uint8_t		ms_cx[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8 * 2];
//	uint16_t	ss_ix_tx[FTS_TX_LENGTH_V8 * 2], ss_ix_rx[FTS_RX_LENGTH_V8 * 2];
	int16_t		ms_raw[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8], ms_jitter[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8], ms_lp_raw[FTS_TX_LENGTH_V8 * FTS_RX_LENGTH_V8];
	int16_t		ss_raw_tx[FTS_TX_LENGTH_V8], ss_raw_rx[FTS_RX_LENGTH_V8], ss_jitter_tx[FTS_TX_LENGTH_V8], ss_jitter_rx[FTS_RX_LENGTH_V8], ss_lp_raw_tx[FTS_TX_LENGTH_V8];

#ifdef	FTS_GET_SERIALNUM_V8
	uint8_t		serial_num[DIE_INFO_SIZE_V8];
#endif

    /****************************************************/
    // 180307

    int count_max = 0;
    int count_min = 0;
    unsigned int    ret_state = 0;
    int fstat = 0;
    int ref_max = 0;
    int ref_min = 0;
    int cvt_data[FTS_TX_LENGTH_V8][FTS_RX_LENGTH_V8] = {{0,},};
    int txIdx = 0;
    int rxIdx = 0;
    int index = 0;
	int	j;	

    /****************************************************/

	FUNC_BEGIN();



	printf("\n\n< TEST VERSION 08Ver B1 > \n");

	printf("\n--------------------------------------\n");

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V8
	flashProcedure_v8(0, 0);
#endif

#ifdef	FTS_SUPPORT_HW_PIN_CHECK_V8
	fts_hw_reset_pin_check_v8();
	fts_interrupt_pin_check_v8();
#endif

////////// TD01
	printf("[ TD01 : I2C ]\n");

	if ((res = fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - System Reset");
		if(res == (FTS_ERR_V8 | FTS_ERR_I2C_V8))
		{
		    printf("\n--------------------------------------\n");
			printf("=======> I2C_FAIL \n");
		    printf("\n--------------------------------------\n");

	        uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
	        uart_buf[10] = l_hf_test_mode;

			FUNC_END();
			return	I2C_ERR;
		}
		printf("=======> I2C_PASS \n");
	}
	else
		printf("\r\n[FTS]System Reset");
	printf("=======> I2C_PASS \n");

	printf("\n--------------------------------------\n");
	if ((res = fts_read_chip_id_v8()) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - read chip id ");

		if(res & FTS_ERR_I2C_V8)
		{
			printf("-> I2C_ERR ");
            printf("\n--------------------------------------\n");
            printf("=======> I2C_FAIL \n");
            printf("\n--------------------------------------\n");

	        uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
	        uart_buf[10] = l_hf_test_mode;

			FUNC_END();
            return  I2C_ERR;
		}
		else if (res & FTS_ERR_EVT_TIMEOVER_V8)
		{
			printf("-> TIME OVER ");

		}
		else if (res & FTS_ERR_CHIPID_V8)
		{
			printf("-> NO FIRMWARE (byte 4,5 empty) ");

		}
	}
	else
		printf("\r\n[FTS]read chip id ");
#if 1
	res = fts_interrupt_control_v8(DISABLE);
    if(res & FTS_ERR_CRC_CORRUPT_V8)
        printf("-> FTS_ERR_CRC_CORRUPT FAIL ");

	fts_delay_v8(1);

	printf("\n--------------------------------------\n");
	printf("[ TD02 : FIRMWARE VERSION ]\n");
	printf("[ TD03 : CONFIG VERSION ]\n");
	printf("[ TD04 : RELEASE VERSION ]\n");
	if ((res = fts_read_sysInfo_v8()) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - sysInfo");
		if(res & FTS_ERR_EVT_TIMEOVER_V8)
			printf("-> TIME OVER ");
		else if(res & FTS_ERR_HOSTDATA_ID_HD_V8)
			printf("-> HOSTDATA READ FAIL ");
		if(res & FTS_ERR_CRC_CORRUPT_V8)
			printf("-> FTS_ERR_CRC_CORRUPT FAIL ");

		ret_state |= (1<<TOUCH_STM_FW_VER);
		ret_state |= (1<<TOUCH_STM_CONFIG_VER);
		ret_state |= (1<<TOUCH_STM_RELEASE_VER);

		printf("\n>>>>> FW_VERSION, CONFIG_VERSION, RELEASE_VERSION FAIL : Data read Fail\n");
	}
	else
	{
////////// TD02 , TD03
		printf("\r\n[FTS]sysInfo");
		printf("\r\n[FTS]FW:%04X, Config:%04X, Extenal:%4X", pSysInfo->fwVer, pSysInfo->cfgVer, pSysInfo->releaseInfo[0] | (pSysInfo->releaseInfo[1] << 8));
		printf("\r\n[FTS]TX:%04X, RX:%04X", pSysInfo->scrTxLen, pSysInfo->scrRxLen);

        printf("READ fw_version 0x%04X (limit:0x%04X)\n",pSysInfo->fwVer,l_fw_ver);
        printf("READ config_version 0x%04X (limit:0x%04X)\n",pSysInfo->cfgVer,l_config_ver);
        printf("READ Extenal_version 0x%04X (limit:0x%04X)\n",(pSysInfo->releaseInfo[0] | (pSysInfo->releaseInfo[1] << 8)),l_release_ver);

		if(pSysInfo->fwVer == l_fw_ver)
		{
			ret_state |= (0<<TOUCH_STM_FW_VER);
			printf(">>>>> FW_VERSION OK\n");
		}
		else
		{
			ret_state |= (1<<TOUCH_STM_FW_VER);
			printf(">>>>> FW_VERSION FAIL\n");
		}

		printf("\n--------------------------------------\n");
		if((pSysInfo->cfgVer) == l_config_ver)
		{
			ret_state |= (0<<TOUCH_STM_CONFIG_VER);
			printf(">>>>> CONFIG_VERSION OK\n");
		}
        else
        {
			ret_state |= (1<<TOUCH_STM_CONFIG_VER);
            printf(">>>>> CONFIG_VERSION FAIL\n");
        }		
		printf("\n--------------------------------------\n");
		if(((pSysInfo->releaseInfo[0] | (pSysInfo->releaseInfo[1]<<8))) == l_release_ver)
		{
			ret_state |= (0<<TOUCH_STM_RELEASE_VER);
			printf(">>>>> RELEASE_VERSION OK\n");
		}
        else
        {
			ret_state |= (1<<TOUCH_STM_RELEASE_VER);
            printf(">>>>> RELEASE_VERSION FAIL\n");
        }		
	}

	printf("\n--------------------------------------\n");
#ifdef	FTS_GET_SERIALNUM_V8
	printf("\r\n[FTS]Serial Number : ");
	for (i = 0; i < DIE_INFO_SIZE_V8; i++)
	{
		serial_num[i] = pSysInfo->dieInfo[DIE_INFO_SIZE_V8 - i - 1];
		printf("%02X", serial_num[i]);
	}
#endif

	printf("\n--------------------------------------\n");
#if defined (MACHINE_OTHERS_V8) || defined (FTS_SEL_DATA_MONITORING_V8)
 #ifdef	FTS_SEL_AUTOTUNE_FULLINIT_V8
	res = fts_do_FullPanelInit_v8();
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Full Panel Init.");
	}
	else
		printf("\r\n[FTS]Full Panel Init.");
 #else
	if ((res = fts_do_autotune_v8()) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Auto-tune");
        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");
	}
	else
		printf("\r\n[FTS]Auto-tune");

  #ifdef FTS_SUPPORT_SAVE_COMP_V8
	if ((res = fts_save2flash_compensation_v8()) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Save to Flash");
        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");

	}
	else
		printf("\r\n[FTS]Save to Flash");

  #endif
 #endif

	if ((res = fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense On");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL "); 

	}
	else
		printf("[FTS]Sense On");
	fts_delay_v8(200);
 #if	1
	if (fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, DISABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense Off");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");

	}
	else
		printf("\r\n[FTS]Sense Off");
 #endif

	memset(ms_cx, 0x00, pSysInfo->scrTxLen * pSysInfo->scrRxLen);
	memset(ss_ix_tx, 0x00, pSysInfo->scrTxLen * sizeof(int16_t));
	memset(ss_ix_rx, 0x00, pSysInfo->scrRxLen * sizeof(int16_t));
	res = fts_get_ms_comp_data_v8(LOAD_CX_MS_LOW_POWER_V8, ms_cx);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Get ms compensation");
        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL "); 

	}
	else
		printf("\r\n[FTS]Read MS compensation");

	res = fts_get_ss_totalcomp_data_v8(LOAD_PANEL_CX_TOT_SS_TOUCH_V8, ss_ix_tx, ss_ix_rx);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Get ss compensation");
        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL "); 
	}
	else
		printf("\r\n[FTS]Read SS compensation");

	res = fts_get_ms_frame_data_v8(MS_FILTER_V8, ms_raw, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS_FILTER");  
	}
	res = fts_get_ss_frame_data_v8(SS_FILTER_V8, ss_raw_tx, ss_raw_rx, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get SS_FILTER"); 
	}
	res = fts_get_ms_frame_data_v8(MS_STRENGHT_V8, ms_jitter, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS_STRENGHT");
	}
	res = fts_get_ss_frame_data_v8(SS_STRENGHT_V8, ss_jitter_tx, ss_jitter_rx, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS_STRENGHT");  
	}

	printf("\r\n[FTS]Read Raw data");
#endif

	printf("\n--------------------------------------\n");
#if defined (MACHINE_FINAL_INSPECTION_V8) || defined (MACHINE_OQC_INSPECTION_V8)
#ifdef FTS_METHOD_GOLDEN_VALUE_V8
	if ((res = fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - System Reset");
	}
	else
		printf("\r\n[FTS]System Reset");

	res=fts_interrupt_control_v8(DISABLE);
    if(res & FTS_ERR_CRC_CORRUPT_V8)
        printf("-> FTS_ERR_CRC_CORRUPT FAIL ");

	fts_delay_v8(1);

  #ifdef MACHINE_FINAL_INSPECTION_V8
	res = fts_do_PanelInit_v8();
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Panel Init.");
	}
	else
		printf("\r\n[FTS]Panel Init.");
  #endif

	/*
	 * Sense on and locked active mode
	 */
	if ((res = fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense On");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("[FTS]Sense On");
	if ((res = fts_scan_mode_control_v8(CMD_SCAN_LOCKED_V8, CMD_SCAN_LOCKED_ACTIVE_V8, ENABLE)) != FTS_NO_ERR_V8)/////////// T.O
	{
		printf("\r\n[FTS]ERROR - Locked active");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("[FTS]Locked active");
	fts_delay_v8(200);
	fts_delay_v8(1000);
#if	0	/* swchoi - comment as below delay is not in ST reference code - 20180827 */
	fts_delay_v8(2500); //test - 180515
	fts_delay_v8(6600); //test - 180515
#endif	/* swchoi - end */
	//printf("[FTS]Locked active");

	if (fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, DISABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense Off");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("\r\n[FTS]Sense Off");

	/*
	 * Buffer initialization
	 */
	memset(ms_raw, 0x00, pSysInfo->scrTxLen * pSysInfo->scrRxLen * sizeof(int16_t));
	memset(ms_jitter, 0x00, pSysInfo->scrTxLen * pSysInfo->scrRxLen * sizeof(int16_t));
	memset(ms_lp_raw, 0x00, pSysInfo->scrTxLen * pSysInfo->scrRxLen * sizeof(int16_t));
	memset(ss_raw_tx, 0x00, pSysInfo->scrTxLen * sizeof(int16_t));
	memset(ss_raw_rx, 0x00, pSysInfo->scrRxLen * sizeof(int16_t));
	memset(ss_jitter_tx, 0x00, pSysInfo->scrTxLen * sizeof(int16_t));
	memset(ss_jitter_rx, 0x00, pSysInfo->scrRxLen * sizeof(int16_t));
	memset(ss_lp_raw_tx, 0x00, pSysInfo->scrTxLen * sizeof(int16_t));

	printf("\n--------------------------------------\n");
	printf("[ TD06 : MUTUAL RAW DATA (CM REFERENCE) ]\n");
////////// TD06
	// mutual raw data (Cm reference)
	res = fts_get_ms_frame_data_v8(MS_FILTER_V8, ms_raw, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS_FILTER");
		printf("=======> CM REFERENCE FAIL : Data Read Fail\n");
		ret_state |= 1 << TOUCH_STM_CM_REFERENCE_RAW;
		ret_state |= 1 << TOUCH_STM_CM_REFERENCE_GAP;
	}
	else	
	{
	    for (txIdx = 0; txIdx < pSysInfo->scrTxLen; txIdx++)
	    {
	        for (rxIdx = 0; rxIdx < pSysInfo->scrRxLen; rxIdx++)
	        {
	            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
	            index = txIdx * pSysInfo->scrRxLen + rxIdx;
	            cvt_data[txIdx][rxIdx] = ms_raw[index];
	        }
	    }
	
	    // For Get Reference Gap (Max - Min)
	    // No Need Bubble Sort...
	    ref_max = 0;
	    ref_min = 0;
	    // Initialize... Set First Data.
	    ref_max = cvt_data[0][0];
	    ref_min = cvt_data[0][0];

		printf("Limit Data -> [MAX : %d] [MIN %d] \n",l_cm_reference_raw[1],l_cm_reference_raw[0]);
	    count_min = 0;
	    count_max = 0;

	    /****************************************************/
	    // Modified by iamozzi...
	    for (i = 0; i < pSysInfo->scrTxLen; i++)
	    {
	        for (j = 0; j < pSysInfo->scrRxLen; j++)
	        {
	            if (cvt_data[i][j] < l_cm_reference_raw[0])
	            {
	                count_min++;
	                if(DEBUG_MODE)
	                printf("\033[1;34m%05d\033[m ", cvt_data[i][j]);
	                //printf("-%04d ", cvt_data[i][j]);
	            }
	            else if (cvt_data[i][j] > l_cm_reference_raw[1])
	            {
	                count_max++;
	                if(DEBUG_MODE)
	                printf("\033[1;31m%05d\033[m ", cvt_data[i][j]);
	                //printf("+%04d ", cvt_data[i][j]);
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

		printf("\n--------------------------------------\n");
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
	}

////////// TD11,TD12
	// Self raw data
	printf("[ TD11 : SELF RAW TX ]\n");
	res = fts_get_ss_frame_data_v8(SS_FILTER_V8, ss_raw_tx, ss_raw_rx, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("[ TD11 : SELF RAW RX ]\n");
		printf("\r\n[FTS]ERROR-Get SS_FILTER");
		ret_state |= 1 << TOUCH_STM_SELF_RAW_TX;
		ret_state |= 1 << TOUCH_STM_SELF_RAW_RX;
		printf("=======> SELF_RAW_TX FAIL : Data Read Fail\n");
	}
	else
	{
	    count_min = 0;
	    count_max = 0;
	    if(DEBUG_MODE)
	        printf("LowLimit = %d / HighLimit = %d\n",l_self_raw_tx[0],l_self_raw_tx[1]);

	    if(DEBUG_MODE)
        printf("ss_raw_TX_buf : ");

		for(i=0; i<pSysInfo->scrTxLen; i++)
	    {
		    if(ss_raw_tx[i] < l_self_raw_tx[0])
			{
				count_min++;
				if(DEBUG_MODE)
				    printf("\033[1;34m%04d\033[m ",ss_raw_tx[i]);
				    //printf("-%03d ",ss_raw_tx[i]);
			}
			else if(ss_raw_tx[i] > l_self_raw_tx[1])
			{
			    count_max++;
			    if(DEBUG_MODE)
			        printf("\033[1;31m%04d\033[m ",ss_raw_tx[i]);
			        //printf("+%03d ",ss_raw_tx[i]);
			}
			else
			{
			    if(DEBUG_MODE)
			        printf("%04d ",ss_raw_tx[i]);
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
	

		printf("--------------------------------------\n");
		printf("[ TD12 : SELF RAW RX ]\n");
	
	    count_min = 0;
	    count_max = 0;
	
	    if(DEBUG_MODE)
	        printf("LowLimit = %d / HighLimit = %d\n",l_self_raw_rx[0],l_self_raw_rx[1]);
	
	    if(DEBUG_MODE)
	        printf("ss_raw_RX_buf : ");
	    for(i=0; i<pSysInfo->scrRxLen; i++)
	    {
	
	        if(ss_raw_rx[i] < l_self_raw_rx[0])
	        {
	            count_min++;
	            if(DEBUG_MODE)
	                printf("\033[1;34m%04d\033[m ",ss_raw_rx[i]);
	                //printf("-%03d ",ss_raw_rx[i]);
	        }
	        else if(ss_raw_rx[i] > l_self_raw_rx[1])
	        {
	            count_max++;
	            if(DEBUG_MODE)
	                printf("\033[1;31m%04d\033[m ",ss_raw_rx[i]);
	                //printf("+%03d ",ss_raw_rx[i]);
	        }
	        else
	        {
	            if(DEBUG_MODE)
	                printf("%04d ",ss_raw_rx[i]);
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
	}
	
	printf("\n--------------------------------------\n");
	count_min = 0;
	count_max = 0;

////////// TD07
    printf("[ TD07 : MUTUAL JITTER (CM(MS)_JITTER) ]\n");
	// mutual strength (jitter) data
	res = fts_get_ms_frame_data_v8(MS_STRENGHT_V8, ms_jitter, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS_STRENGHT");
		ret_state |= 1 << TOUCH_STM_CM_JITTER;
		printf("=======> CM JITTER FAIL : Data Read Fail\n");
	}
	else
	{
		printf("Limit Data -> [MAX : %d] [MIN %d] \n",l_cm_jitter[1],l_cm_jitter[0]);

		for (txIdx = 0; txIdx < pSysInfo->scrTxLen; txIdx++)
		{
		    for (rxIdx = 0; rxIdx < pSysInfo->scrRxLen; rxIdx++)
		    {
		        cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
		        index = txIdx * pSysInfo->scrRxLen + rxIdx;
		        cvt_data[txIdx][rxIdx] = ms_jitter[index];
		    }
		}
	
	    /****************************************************/
	
	    /****************************************************/
	    // Modified by iamozzi...
	    for (i = 0; i < pSysInfo->scrTxLen; i++)
	    {
	        for (j = 0; j < pSysInfo->scrRxLen; j++)
	        {
	            if (cvt_data[i][j] < l_cm_jitter[0])
	            {
	                count_min++;
					if(DEBUG_MODE)
					printf("\033[1;34m%04d\033[m ",cvt_data[i][j]);
	            }
	            else if (cvt_data[i][j] > l_cm_jitter[1])
	            {
	                count_max++;
					if(DEBUG_MODE)
					printf("\033[1;31m%04d\033[m ",cvt_data[i][j]);
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
	
	}

	printf("\n--------------------------------------\n");
////////// TD22
    printf("[ TD22 : SELF JITTER (SS_JITTER) : TX ]\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_ss_jitter[0],l_ss_jitter[1]);


	// self strength (jitter) data
	res = fts_get_ss_frame_data_v8(SS_STRENGHT_V8, ss_jitter_tx, ss_jitter_rx, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("[ TD22 : SELF JITTER (SS_JITTER) : RX ]\n");
		printf("=======> SELF_JITTER FAIL : Data Read Fail\n");
		ret_state |= 1 << TOUCH_STM_SELF_JITTER_TX;
		ret_state |= 1 << TOUCH_STM_SELF_JITTER_RX;

		printf("\r\n[FTS]ERROR-Get MS_STRENGHT");
	}
	else
	{
		if(DEBUG_MODE)
		    printf("ss_jitter_TX_buf : ");

		for(i=0; i<pSysInfo->scrTxLen; i++)
		{
		    if(ss_jitter_tx[i] < l_ss_jitter[0])
		    {
		        count_min++;
		        if(DEBUG_MODE)
		            printf("\033[1;34m%04d\033[m ",ss_jitter_tx[i]);
		            //printf("-%03d ",ss_jitter_tx[i]);
		    }
		    else if(ss_jitter_tx[i] > l_ss_jitter[1])
		    {
		        count_max++;
		        if(DEBUG_MODE)
		            printf("\033[1;31m%04d\033[m ",ss_jitter_tx[i]);
		            //printf("+%03d ",ss_jitter_tx[i]);
		    }
		    else
		    {
		        if(DEBUG_MODE)
		            printf("%04d ",ss_jitter_tx[i]);
		    }
	    }
	    if(DEBUG_MODE)
			printf("\n");
	
	    if(count_min)
	    {
	        printf("=======> SELF_JITTER_TX < TOO LOW > FAIL [%d]\n",count_min);
	        ret_state |= 1 << TOUCH_STM_SELF_JITTER_TX;
	        fstat = 1;
	    }
	    if(count_max)
	    {
	        printf("=======> SELF_JITTER_TX < TOO HIGH > FAIL [%d]\n",count_max);
	        ret_state |= 1 << TOUCH_STM_SELF_JITTER_TX;
	        fstat = 1;
	    }
	    if(!fstat)
	    {
	        printf("=======> SELF_JITTER_TX PASS\n");
	        ret_state |= 0 << TOUCH_STM_SELF_JITTER_TX;
	    }
	
	    fstat = 0;
	
	    printf("MAX / MIN : %d / %d \n",count_max,count_min);
	    printf("--------------------------------------\n\n");
	
	    printf("--------------------------------------\n");
		printf("[ TD22 : SELF JITTER (SS_JITTER) : RX ]\n");
	
	    count_min = 0;
	    count_max = 0;
	
	    if(DEBUG_MODE)
	        printf("LowLimit = %d / HighLimit = %d\n",l_ss_jitter[0],l_ss_jitter[1]);
	
	    if(DEBUG_MODE)
	        printf("ss_jitter_RX_buf : ");
	    for(i=0; i<pSysInfo->scrRxLen; i++)
	    {
	
	        if(ss_jitter_rx[i] < l_ss_jitter[0])
	        {
	            count_min++;
	            if(DEBUG_MODE)
	                printf("\033[1;34m%04d\033[m ",ss_jitter_rx[i]);
	        }
	        else if(ss_jitter_rx[i] > l_ss_jitter[1])
	        {
	            count_max++;
	            if(DEBUG_MODE)
	                printf("\033[1;31m%04d\033[m ",ss_jitter_rx[i]);
	        }
	        else
	        {
	            if(DEBUG_MODE)
	                printf("%04d ",ss_jitter_rx[i]);
	        }
	    }
	    if(DEBUG_MODE)
	        printf("\n");
	    if(count_min)
	    {
	        printf("=======> SELF_JITTER_RX < TOO LOW > FAIL [%d]\n",count_min);
	        ret_state |= 1 << TOUCH_STM_SELF_JITTER_RX;
	        fstat = 1;
	    }
		
		if(count_max)
		{
		    printf("=======> SELF_JITTER_RX < TOO HIGH > FAIL [%d]\n",count_max);
		    ret_state |= 1 << TOUCH_STM_SELF_JITTER_RX;
		    fstat = 1;
		}
	
	    if(!fstat)
	    {
	        printf("=======> SELF_JITTER_RX PASS\n");
	        ret_state |= 0 << TOUCH_STM_SELF_JITTER_RX;
	    }
	
	    fstat = 0;
	
	        printf("MAX / MIN : %d / %d \n",count_max,count_min);
	    if(DEBUG_MODE)
	        printf("[FTS] Getting raw of self is done!!\n\n");
	    printf("--------------------------------------\n\n");
	}

	printf("--------------------------------------\n\n");

	/*
	 * System reset and enter the LP mode
	 */
	if ((res = fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - System Reset");
	}
	else
		printf("\r\n[FTS]System Reset");

	if ((res = fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, ENABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense On");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	printf("\r\n[FTS]Sense On");
	if ((res = fts_scan_mode_control_v8(CMD_SCAN_LPMODE_V8, 0x00, DISABLE)) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - LP mode");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("\r\n[FTS]Enter LP mode");

	printf("\n--------------------------------------\n");

    count_min = 0;
    count_max = 0;


////////// TD24
    printf("[ TD24 : LP SELF RAW TX ]\n");
	/*
	 * Change LP detect mode and Get Self force raw data of LP detect mode.
	 */
	if ((res = fts_scan_mode_control_v8(CMD_SCAN_LOCKED_V8, CMD_SCAN_LOCKED_LP_DETECT_V8, ENABLE)) != FTS_NO_ERR_V8)//////////T.O
	{
		printf("\r\n[FTS]ERROR - Locked LP detect");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("\r\n[FTS]Locked LP detect");
	fts_delay_v8(50);
//test - 180515
    fts_delay_v8(200);
    fts_delay_v8(1000);
#if	0	/* swchoi - comment as below delay is not in ST reference code - 20180827 */
    fts_delay_v8(2500);
	fts_delay_v8(6600);
#endif	/* swchoi - end */
//

	if (fts_get_SyncFrame_data_v8(LOAD_SYNC_FRAME_RAW_V8, NULL, ss_lp_raw_tx, NULL, NULL) != FTS_NO_ERR_V8)  /////////////
	{
		printf("\r\n[FTS]ERROR - Ss LP force raw");
		printf("=======> LP_SELF_RAW_TX FAIL : Data Read Fail \n");
		ret_state |= 1 << TOUCH_STM_LP_SELF_RAW_TX;
	}
	else
	{
		printf("\nLimit Data -> [MAX : %d] [MIN %d] \n",l_lp_self_raw_tx[1],l_lp_self_raw_tx[0]);

        if(DEBUG_MODE)
            printf("ss_lp_raw_TX_buf : ");

        for(i=0; i<pSysInfo->scrTxLen; i++)
        {
            if(ss_lp_raw_tx[i] < l_lp_self_raw_tx[0])
            {
                count_min++;
                if(DEBUG_MODE)
                    printf("\033[1;34m%04d\033[m ",ss_lp_raw_tx[i]);
            }
            else if(ss_lp_raw_tx[i] > l_lp_self_raw_tx[1])
            {
                count_max++;
                if(DEBUG_MODE)
                    printf("\033[1;31m%04d\033[m ",ss_lp_raw_tx[i]);
            }
            else
            {
                if(DEBUG_MODE)
                    printf("%04d ",ss_lp_raw_tx[i]);
            }
    
        }
        if(DEBUG_MODE)
            printf("\n");

	    if(count_min)
	    {
	        printf("=======> LP_SELF_RAW_TX < TOO LOW > FAIL [%d]\n",count_min);
	        ret_state |= 1 << TOUCH_STM_LP_SELF_RAW_TX;
	        fstat = 1;
	    }
	    if(count_max)
	    {
	        printf("=======> LP_SELF_RAW_TX < TOO HIGH > FAIL [%d]\n",count_max);
	        ret_state |= 1 << TOUCH_STM_LP_SELF_RAW_TX;
	        fstat = 1;
	    }
	
	    if(!fstat)
	    {
	        printf("=======> LP_SELF_RAW_TX PASS\n");
	        ret_state |= 0 << TOUCH_STM_LP_SELF_RAW_TX;
	    }

        fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);
        printf("--------------------------------------\n\n");
	}

	printf("\n--------------------------------------\n");
////////// TD23
    printf("[ TD23 : LP MUTUAL RAW ]\n");
	/*
	 * Change LP active mode and Get mutual raw data of LP active mode.
	 */
	if ((res = fts_scan_mode_control_v8(CMD_SCAN_LOCKED_V8, CMD_SCAN_LOCKED_LP_ACTIVE_V8, ENABLE)) != FTS_NO_ERR_V8) //////////T.O
	{
		printf("\r\n[FTS]ERROR - Locked LP active");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("\r\n[FTS]Locked LP active");
	fts_delay_v8(200);

	if (fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, DISABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR - Sense Off");

        if(res & FTS_ERR_CRC_CORRUPT_V8)
            printf("-> FTS_ERR_CRC_CORRUPT FAIL ");  //need Data what Touch test fail if it is fail

	}
	else
		printf("\r\n[FTS]Sense Off");

	res = fts_get_ms_frame_data_v8(MS_FILTER_V8, ms_lp_raw, pSysInfo->scrTxLen, pSysInfo->scrRxLen);
	if (res != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]ERROR-Get MS lp active raw data");
		printf("=======> LP_RAW FAIL : Data Read Fail\n");
		ret_state |= 1 << TOUCH_STM_LP_RAW;
	}
	else
	{
		printf("Limit Data -> [MAX : %d] [MIN %d] \n",l_lp_raw[1],l_lp_raw[0]);

        for (txIdx = 0; txIdx < pSysInfo->scrTxLen; txIdx++)
        {
            for (rxIdx = 0; rxIdx < pSysInfo->scrRxLen; rxIdx++)
            {
                cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
                index = txIdx * pSysInfo->scrRxLen + rxIdx;
                cvt_data[txIdx][rxIdx] = ms_lp_raw[index];
            }
        }
    
        /****************************************************/
    
    
        /****************************************************/
        // Modified by iamozzi...
        for (i = 0; i < pSysInfo->scrTxLen; i++)
        {
            for (j = 0; j < pSysInfo->scrRxLen; j++)
            {
                if (cvt_data[i][j] < l_lp_raw[0])
                {
                    count_min++;
					if(DEBUG_MODE)
						printf("\033[1;34m%04d\033[m ",cvt_data[i][j]);
                }
                else if (cvt_data[i][j] > l_lp_raw[1])
                {
                    count_max++;
					if(DEBUG_MODE)
						printf("\033[1;31m%04d\033[m ",cvt_data[i][j]);
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
            printf("=======> LP_RAW < TOO LOW > FAIL [%d]\n",count_min);
            ret_state |= 1 << TOUCH_STM_LP_RAW;
            fstat = 1;
        }
        if(count_max)
        {
            printf("=======> LP_RAW < TOO HIGH > FAIL [%d]\n",count_max);
            ret_state |= 1 << TOUCH_STM_LP_RAW;
            fstat = 1;
        }

        if(!fstat)
        {
            printf("=======> LP_RAW PASS\n");
            ret_state |= 0 << TOUCH_STM_LP_RAW;
        }

        printf("MAX / MIN : %d / %d \n",count_max,count_min);

        if(DEBUG_MODE)
            printf("[FTS] Getting jitter data is done!!\n\n");
        printf("--------------------------------------\n\n");

	}
	fstat = 0;
    count_min = 0;
    count_max = 0;

	printf("\n--------------------------------------\n");
#endif
#endif

#ifdef	FTS_SUPPORT_ITOTEST_V8
////////// TD17 , TD19 
	res = FTS_NO_ERR_V8;
	res = fts_panel_ito_test_v8();

printf("3. ERR FLAG : 0x%X [0x%X]\n", res, FTS_ERR_EVT_TIMEOVER_V8);
    printf("[ TD17 : ITO SHORT TEST ]\n");

	if(res & FTS_ERR_EVT_TIMEOVER_V8)
	{
		ret_state |= 1 << TOUCH_STM_OPEN_SHORT;
		printf("=======> ITO SHORT TEST FAIL : Data Read Fail\n");
		
	}
	else if(res & FTS_ERR_ITO_TEST_V8)
    {
		ret_state |= 1 << TOUCH_STM_OPEN_SHORT;
		printf("=======> ITO SHORT TEST FAIL\n");
    }
	else
    {
		ret_state |= 0 << TOUCH_STM_OPEN_SHORT;
		printf("=======> ITO SHORT TEST PASS\n");
    }
	printf("\n--------------------------------------\n");

    printf("[ TD19 : HF RAW GAP ]\n");

	if(res & FTS_ERR_HF_RAW_TEST_V8)
    {
		ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_H;
		ret_state |= 1 << TOUCH_STM_CX2_HF_GAP_V;
		printf("=======> HF RAW GAP FAIL\n");
    }
	else
    {
		ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_H;
		ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_V;
		printf("=======> HF RAW GAP PASS\n");
    }

#endif
#endif

	printf("\n--------------------------------------\n");
    printf("[ TD32 : GOLDEN VALUE TEST ]\n");
	if(golden_value_err_state)
	{
        ret_state |= 1 << TOUCH_STM_GOLDEN_VALUE;
        printf("=======> GOLDEN VALUE FAIL\n");
	}
	else
	{
        ret_state |= 0 << TOUCH_STM_GOLDEN_VALUE;
        printf("=======> GOLDEN VALUE PASS\n");
	}

	golden_value_err_state = 0;
	printf("\n--------------------------------------\n");
    uart_buf[4] = ret_state & 0xFF;
    uart_buf[5] = (ret_state >> 8) & 0xFF;
    uart_buf[6] = (ret_state >> 16) & 0xFF;
    uart_buf[7] = (ret_state >> 24) & 0xFF;
    uart_buf[10] = l_hf_test_mode;

    int s =0;
    printf("STATE : ");
    for(s = 0; s <4; s++)
        printf("0x%X, ",uart_buf[s+4]);
    printf("\n");

	printf("\n--------------------------------------\n");
	printf("TEST END \n");
	printf("\n--------------------------------------\n");
	FUNC_END();
	return	TRUE;
}

/**
  * @brief  Initialize with Auto-tune sequence only for touch operation.
  * @param  None
  * @retval None
  */
int fts_init_v8(void)
{
	FUNC_BEGIN();
	power_on_v8();
	fts_delay_v8(50);

	if (fts_systemreset_v8(SYSTEM_RESET_SOFT_V8, ENABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]FAILED - System Reset");
		FUNC_END();
		return	FALSE;
	}

	if (fts_read_chip_id_v8() != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]FAILED - read chip id");
		FUNC_END();
		return	FALSE;
	}

	fts_interrupt_control_v8(DISABLE);
	fts_delay_v8(1);

	if (fts_do_autotune_v8() != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]FAILED - Auto-tune");
		FUNC_END();
		return	FALSE;
	}

	if (fts_scan_mode_control_v8(CMD_SCAN_ACTIVE_V8, CMD_SCAN_ACTIVE_MULTI_V8, ENABLE) != FTS_NO_ERR_V8)
	{
		printf("\r\n[FTS]FAILED - Sense On");

		FUNC_END();
		return	FALSE;
	}

	fts_interrupt_control_v8(ENABLE);
	printf("\r\n[FTS]Interrupt Enable.");

	FUNC_END();
	return	TRUE;
}


    /////////////////////////////////////////////////////////////////
int limit_data_match_v8(int id, struct stm_touch_limit* limit)
{
//    int i =0;

	FUNC_BEGIN();
    if(id != (int)limit->id)
    {
        printf("touch limit id match FAIL..[%d/%d] \n",id,(int)limit->id);
		FUNC_END();
        return FAIL;
    } 

////////////////////////

	l_fw_ver = 0;
	l_config_ver = 0;
	l_release_ver = 0;
	l_cm_reference_raw[MIN] = 0;
	l_cm_reference_raw[MAX] = 0;
	l_cm_reference_gap = 0;
	l_self_raw_tx[MIN] = 0;
	l_self_raw_tx[MAX] = 0;
	l_self_raw_rx[MIN] = 0;
	l_self_raw_rx[MAX] = 0;
	l_cm_jitter[MIN] = 0;
	l_cm_jitter[MAX] = 0;
	l_ss_jitter[MIN] = 0; ///
	l_ss_jitter[MAX] = 0; ///
	l_lp_self_raw_tx[MIN] = 0;
	l_lp_self_raw_tx[MAX] = 0;
	l_lp_self_raw_rx[MIN] = 0;
	l_lp_self_raw_rx[MAX] = 0;
	l_lp_raw[MIN] = 0; ///
	l_lp_raw[MAX] = 0; ///
	l_hf_gap_rx[MIN] = 0; ///
	l_hf_gap_rx[MAX] = 0; ///
	l_hf_gap_tx[MIN] = 0; ///
	l_hf_gap_tx[MAX] = 0; ///
	l_hf_test_mode = 0;

#if	0	/* swchoi - comment as warning: statement with no effect */
    for(i = 0; i < 300; i++)
    {
        memset(hf_TotalCx_Gap_Rx_MAX[i],0,sizeof(hf_TotalCx_Gap_Rx_MAX[i]));
        memset(hf_TotalCx_Gap_Rx_MIN[i],0,sizeof(hf_TotalCx_Gap_Rx_MIN[i]));
        memset(hf_TotalCx_Gap_Tx_MAX[i],0,sizeof(hf_TotalCx_Gap_Tx_MAX[i]));
        memset(hf_TotalCx_Gap_Tx_MIN[i],0,sizeof(hf_TotalCx_Gap_Tx_MIN[i]));
    }
#else	/* swchoi - add */
        memset(hf_TotalCx_Gap_Rx_MAX,0,sizeof(hf_TotalCx_Gap_Rx_MAX));
        memset(hf_TotalCx_Gap_Rx_MIN,0,sizeof(hf_TotalCx_Gap_Rx_MIN));
        memset(hf_TotalCx_Gap_Tx_MAX,0,sizeof(hf_TotalCx_Gap_Tx_MAX));
        memset(hf_TotalCx_Gap_Tx_MIN,0,sizeof(hf_TotalCx_Gap_Tx_MIN));
#endif	/* swchoi - end */

//////////////////////////

    l_fw_ver=limit->fw_ver;
    l_config_ver=limit->config_ver;
    l_release_ver=limit->release_ver;
    l_cm_reference_raw[MIN]=limit->cm_reference_raw[MIN]; // [0] : MIN / [1] : MAX
    l_cm_reference_raw[MAX]=limit->cm_reference_raw[MAX]; // [0] : MIN / [1] : MAX
	l_cm_reference_gap = limit->cm_reference_gap;
    l_self_raw_tx[MIN] = limit->self_raw_tx[MIN];
    l_self_raw_tx[MAX] = limit->self_raw_tx[MAX];
    l_self_raw_rx[MIN] = limit->self_raw_rx[MIN];
    l_self_raw_rx[MAX] = limit->self_raw_rx[MAX];
    l_cm_jitter[MIN]=limit->cm_jitter[MIN];
    l_cm_jitter[MAX]=limit->cm_jitter[MAX];
    l_ss_jitter[MIN]=limit->ss_jitter[MIN]; ///
    l_ss_jitter[MAX]=limit->ss_jitter[MAX]; ///
    l_lp_self_raw_tx[MIN]=limit->lp_self_raw_tx[MIN];
    l_lp_self_raw_tx[MAX]=limit->lp_self_raw_tx[MAX];
    l_lp_self_raw_rx[MIN]=limit->lp_self_raw_rx[MIN];
    l_lp_self_raw_rx[MAX]=limit->lp_self_raw_rx[MAX];
    l_lp_raw[MIN] = limit->lp_raw[MIN]; ///
    l_lp_raw[MAX] = limit->lp_raw[MAX]; ///
    l_hf_gap_rx[MIN] = limit->hf_gap_rx[MIN]; ///
    l_hf_gap_rx[MAX] = limit->hf_gap_rx[MAX]; ///
    l_hf_gap_tx[MIN] = limit->hf_gap_tx[MIN]; ///
    l_hf_gap_tx[MAX] = limit->hf_gap_tx[MAX]; ///

    memcpy(hf_TotalCx_Gap_Rx_MAX,limit->hf_TotalCx_Gap_Rx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Rx_MIN,limit->hf_TotalCx_Gap_Rx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Tx_MAX,limit->hf_TotalCx_Gap_Tx_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(hf_TotalCx_Gap_Tx_MIN,limit->hf_TotalCx_Gap_Tx_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..

	l_hf_test_mode = limit->hf_test_mode;

////////////////////////////
	FUNC_END();
	return PASS;
}
/////////////

//void i2c_dev_match_v8(int i2c_dev, int io_dev)
void i2c_dev_match_v8(int i2c_dev)
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

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/

