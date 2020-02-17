
/* Includes ------------------------------------------------------------------*/
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
#include <fts_lgd_09.h>
#define _IOCTL_TS1_INT_GET_V9          0x15005
#define _IOCTL_TS2_INT_GET_V9          0x15006
#define TM_GET_INTERVAL_V9(st, et) ((et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec))
#define MIN		0
#define MAX		1


int chip_id_v9 = 0;
int fw_version_v9 = 0;
int config_ver_v9 = 0;
int release_ver_v9 = 0;
int stm_dev;
int fts_read_chip_id_v9(void);

/** @addtogroup FTS_Private_Variables
  * @{
  */
static uint8_t fts_fifo_addr[2] = {FTS_FIFO_ADDR_V9, 0};

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
unsigned char	l_otp_param;
unsigned char	l_otp_param1;
unsigned char	l_otp_param2;
unsigned char	l_otp_param3;
int				l_hf_test_mode;

int totalCx_Gap_Rx_MAX[300][300];
int totalCx_Gap_Rx_MIN[300][300];
int totalCx_Gap_Tx_MAX[300][300];
int totalCx_Gap_Tx_MIN[300][300];

int hf_TotalCx_Gap_Rx_MAX[300][300];
int hf_TotalCx_Gap_Rx_MIN[300][300];
int hf_TotalCx_Gap_Tx_MAX[300][300];
int hf_TotalCx_Gap_Tx_MIN[300][300];


void fts_delay_v9(uint32_t msCount)
{
    uint32_t i;

    for(i=0; i<300; i++)	//500
        usleep(msCount);
}

void analog_3V3_power_on_v9(void)
{
}

void digital_1V8_power_on_v9(void)
{
}

void power_on_v9(void)
{
	FUNC_BEGIN();
	analog_3V3_power_on_v9();
	fts_delay_v9(5);

	digital_1V8_power_on_v9();
	fts_delay_v9(10);
	FUNC_END();
}

int fts_write_reg_v9(uint8_t *pInBuf, int InCnt)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V9 >> 1; 
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
	return	0;
}

int fts_read_reg_v9(uint8_t *pInBuf, int inCnt, uint8_t *pOutBuf, int outCnt)
{
    struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	FUNC_BEGIN();
    messages[0].addr = FTS_I2C_ADDR_V9 >> 1; 
    messages[0].flags = 0; 
    messages[0].len = inCnt; 
    messages[0].buf = (char *)pInBuf; 

    messages[1].addr = FTS_I2C_ADDR_V9 >> 1; 
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
	return	0;
}

int fts_command_v9(uint8_t cmd)  //tt
{
	uint8_t	regAdd = 0;

	FUNC_BEGIN();
	regAdd = cmd;
	if(fts_write_reg_v9(&regAdd, 1))
	{
		FUNC_END();
		return I2C_ERR;
	}
	FUNC_END();
	return 0;
}

int fts_systemreset_v9(void) //tt
{
#if	defined(FTSD3_V9)
	uint8_t regAdd[4] = {0xB6, 0x00, 0x28, 0x80};
	uint8_t	warmReg[4] = {0xB6, 0x00, 0x1E, 0x20};
#else
	uint8_t regAdd[4] = {0xB6, 0x00, 0x23, 0x01};
#endif

	FUNC_BEGIN();
#if	defined(FTSD3_V9)
	if(fts_write_reg_v9(&warmReg[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(10);
#endif
	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(10);

	FUNC_END();
	return 0;
}

int fts_get_interrupt_status_v9(void)
{
#if	defined(FTSD3_V9)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x2C, 0x00};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x1C, 0x00};
#endif
	uint8_t	val[10];

	FUNC_BEGIN();
	if(fts_read_reg_v9(regAdd, 3, (uint8_t *) val, 2))
	{
		FUNC_END();
		return I2C_ERR;
	}
	if (val[1] & 0x40)
	{
		printf("[fts_get_interrupt_status] Interrupt is enabled.\n");
		FUNC_END();
		return	TRUE;
	}
	else
	{
		printf("[fts_get_interrupt_status] Interrupt is disabled.\n");
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	FALSE;
}

int fts_interrupt_control_v9(int onoff) //tt
{
#if	defined(FTSD3_V9)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x2C, 0x08};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x1C, 0x00};
#endif

	FUNC_BEGIN();
	if (onoff == ENABLE)
		regAdd[3] |= 0x40;

	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(1);
	
	FUNC_END();
	return 0;
}

int fts_read_chip_id_v9(void)
{
#if	defined(FTSD3_V9)
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x04, 0x00};
#else
	uint8_t	regAdd[4] = {0xB6, 0x00, 0x07, 0x00};
#endif
	uint8_t	val[8] = {0};
	int		retry = 10;

	FUNC_BEGIN();
	while (retry--)
	{
		fts_delay_v9(1);
		if(fts_read_reg_v9(regAdd, 3, (uint8_t *) val, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}

		if ((val[1] == ((l_product_id >> 8)&0xFF)) && (val[2] == (l_product_id & 0xFF )))
		{
		#if	defined(FTSD3_V9)
			if ((val[5] == 0x00) && (val[6] == 0x00))
			{
				printf("[fts_read_chip_id] Error - No FW : %02X %02X\n", val[5], val[6]);
		#else
			if ((val[4] == 0x00) && (val[5] == 0x00))
			{
				printf("[fts_read_chip_id] Error - No FW : %02X %02X\n", val[4], val[5]);
		#endif
				FUNC_END();
				return	FALSE;
			}
			else
			{
				printf("[fts_read_chip_id] Chip ID : %02X%02X\n", val[1], val[2]);
				chip_id_v9 = (val[1] << 8) | val[2];
				FUNC_END();
				return	TRUE;
			}
		}
	}
	if (retry <= 0)
		printf("[fts_read_chip_id] Error - Time Over\n (config product ID : %02X%02X / original product ID : %02X%02X)\n", ((l_product_id >> 8)&0xFF), (l_product_id & 0xFF ), FTS_ID0_V9, FTS_ID1_V9);

	FUNC_END();
	return FALSE;
}

int fts_cmd_completion_check_v9(uint8_t event1, uint8_t event2, uint8_t event3)
{
	uint8_t val[8];
	int		retry = 100;

	FUNC_BEGIN();
	while (retry--)
	{
		fts_delay_v9(10);

		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;	
		}
		if ((val[0] == event1) && (val[1] == event2) && (val[2] == event3))
		{
			printf("[fts_cmd_completion_check] OK [%02x][%02x][%02x] %d\n", val[0], val[1], val[2], retry);
			FUNC_END();
			return TRUE;
		}
		else if (val[0] == 0x0F)
		{
			printf("[fts_cmd_completion_check] Error - [%02x][%02x][%02x]\n", val[0], val[1], val[2]);
		}
	}
	if (retry <= 0)
		printf("[fts_cmd_completion_check] Error - Time Over [%02x][%02x][%02x]\n", event1, event2, event3);

	FUNC_END();
	return FALSE;
}

int fts_get_versionInfo_v9(void) //tt
{
	uint8_t	data[FTS_EVENT_SIZE_V9];
	int		retry = 30, intr_status = FALSE;
	int		ret = 0;

	FUNC_BEGIN();
	intr_status = fts_get_interrupt_status_v9();
	if (intr_status == TRUE)
	{
		fts_interrupt_control_v9(DISABLE);
		fts_delay_v9(10);
	}

	fts_command_v9(FTS_CMD_RELEASEINFO_V9);
	while (retry--)
	{
		fts_delay_v9(1);
		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if (data[0] == EVENTID_INTERNAL_RELEASE_INFO_V9)
		{
		//	printf("[fts_get_versionInfo] FW Ver. : 0x%02x%02x, Config Ver. : 0x%02x%02x\n", data[4], data[3], data[6], data[5]);
			fw_version_v9 = (data[3] << 8) | data[4];
			config_ver_v9 = (data[6] << 8) | data[5];

			printf("READ fw_version 0x%04X (limit:0x%04X)\n",fw_version_v9,l_fw_ver);
			printf("READ config_version 0x%04X (limit:0x%04X)\n",config_ver_v9,l_config_ver);
			if(fw_version_v9 == l_fw_ver)
			{
				//printf(">>>>> FW_VERSION OK\n");
				ret |= (TRUE << 0);
			}
			else
			{
				//printf(">>>>> FW_VERSION FAIL\n");
                ret |= (FALSE << 0);
            }


            if(config_ver_v9 == l_config_ver)
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
		else if (data[0] == EVENTID_EXTERNAL_RELEASE_INFO_V9)
		{
			//printf("[fts_get_versionInfo] Release Version : 0x%02x%02x\n", data[1], data[2]);
			release_ver_v9 = (data[1] << 8) | data[2];
			printf("READ release_version 0x%04X (limit:0x%04X)\n",release_ver_v9,l_release_ver);
			if(release_ver_v9 == l_release_ver)
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
	}
	if (retry <= 0)
	{
		printf("[fts_get_versionInfo] Error - Time Over\n");
        ret |= FALSE << 0;
        ret |= FALSE << 1;
        ret |= FALSE << 2;
	}

	if (intr_status == TRUE)
	{
		fts_interrupt_control_v9(ENABLE);
		fts_delay_v9(10);
	}
		
	FUNC_END();
	return ret;
}

int fts_get_channelInfo_v9(void)
{
	int		status = -1;
	uint8_t cmd[4] = {0xB2, 0x00, 0x14, 0x02};
	uint8_t data[FTS_EVENT_SIZE_V9];
	int		retry = 30, intr_status;

	FUNC_BEGIN();
	intr_status = fts_get_interrupt_status_v9();
	if (intr_status == TRUE)
	{
		fts_interrupt_control_v9(DISABLE);
		fts_delay_v9(10);
	}
	memset(data, 0x0, FTS_EVENT_SIZE_V9);

	if(fts_write_reg_v9(&cmd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	while (retry--)
	{
		fts_delay_v9(5);
		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}

		if (data[0] == EVENTID_RESULT_READ_REGISTER_V9)
		{
			if ((data[1] == cmd[1]) && (data[2] == cmd[2]))
			{
				printf("[fts_get_channelInfo] Sense length : %d\n", data[3]);
				printf("[fts_get_channelInfo] Force length : %d\n", data[4]);
				status = TRUE;
				break;
			}
		}
	}
	if (retry <= 0)
	{
		printf("[fts_get_channelInfo] Error - Time over\n");
		status = FALSE;
	}

	if (intr_status == TRUE)
	{
		fts_interrupt_control_v9(ENABLE);
		fts_delay_v9(10);
	}

	FUNC_END();
	return	status;
}

#ifdef	FTS_LOCKDOWNCODE_FEATURE_V9
uint8_t CalculateCRC8_v9(uint8_t* u8_srcBuff, uint32_t u32_len, uint8_t u8_polynomial, uint8_t u8_initCRCVal)
{
	uint8_t u8_remainder;
	uint32_t u32_i;
	uint8_t bit;
	u8_remainder = u8_initCRCVal;

	FUNC_BEGIN();
	// Perform modulo-2 division, a byte at a time.
	for(u32_i = 0; u32_i < u32_len; u32_i++)
	{
		//Bring the next byte into the remainder.
		u8_remainder ^= u8_srcBuff[u32_i];

		//Perform modulo-2 division, a bit at a time.
		for (bit = 8; bit > 0; --bit)
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
uint8_t lockdown_v9(uint8_t *input_code)
{
	uint8_t u8_polynomial = 0x9B;		// WCDMA standard
	uint8_t u8_initCRCValue = 0x00;		// WCDMA standard.
	uint8_t u8_crcResult;

	FUNC_BEGIN();
	u8_crcResult = CalculateCRC8_v9(input_code, FTS_LOCKDOWNCODE_SIZE_V9, u8_polynomial, u8_initCRCValue);

	FUNC_END();
	return	u8_crcResult; // this is CRC result
}

int fts_lockdown_read_v9(int id)
{
	uint8_t	data[10];
	int		retry = 50;
//	int		nwr_count = 0;
	int		count = 0;
	unsigned char	param = 0;
	unsigned char	param1 = 0;
	unsigned char	param2 = 0;
	unsigned char	param3 = 0;

	FUNC_BEGIN();
/* plus..*/
    fts_systemreset_v9();
    if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) <= FALSE)
    {
		//vcp_printf("\n\r[fts_lockdown_read] FAILED - System Reset");
		printf("\n\r[fts_lockdown_read] FAILED - System Reset");
		FUNC_END();
        return FALSE;
    }
/*********/

	fts_interrupt_control_v9(ENABLE);
	fts_delay_v9(50);
	fts_command_v9(LOCKDOWN_READ_V9);
	if(DEBUG_MODE)
		printf("RAW DATA : ");

	while (retry--)
	{
		fts_delay_v9(5);

		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if ((data[0] == EVENTID_LOCKDOWN_CODE_V9) && (data[1] == FTS_LOCKDOWNCODE_SIZE_V9))
		{
			count++;
			//printf("[fts_lockdown_read] code[0x%02X] : 0x%02X 0x%02X 0x%02X 0x%02X\n", data[2], data[3], data[4], data[5], data[6]);
			if(DEBUG_MODE)
				printf("0x%02X 0x%02X 0x%02X 0x%02X ", data[3], data[4], data[5], data[6]);

			if(count == 1)
			{
				if((id == MV)||(id == MV_MANUAL) || (id == MV_MQA) || (id ==MV_DQA))
					param1 = data[6] & 0xFF;
			}
			else if(count == 2)
			{
				if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL) || (id == JOAN_E5))
					param = data[3] & 0xFF;
				else if((id == MV)||(id == MV_MANUAL) || (id == MV_MQA) || (id == MV_DQA))
				{
					param2 = data[3] & 0xFF;
					param3 = data[4] & 0xFF;
				}
			}
			if (data[2] == 0x00)
			{
//				nwr_count = data[7];
			}

			if (data[2] >= FTS_LOCKDOWNCODE_SIZE_V9 - 4)
			{
				if(DEBUG_MODE)
					printf("\n");
				if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL)|| (id == JOAN_E5))
					printf(" >> OTP READ DATA [0x%X]\n>> OTP SPEC [0x%X]\n",param,l_otp_param);
				else if((id == MV)||(id == MV_MANUAL) || (MV_MQA) || (MV_DQA))
					printf(" >> OTP READ DATA [0x%X][0x%X][0x%X]\nOTP SPEC [0x%X] [0x%X] [0x%X]\n",param1,param2,param3,l_otp_param1,l_otp_param2,l_otp_param3);
//what
				//printf("[fts_lockdown_read] write count : %d\n", nwr_count);
				if((id == JOAN)||(id == JOAN_REL)||(id == JOAN_MANUAL)|| (id == JOAN_E5))
				{

		
					if(param == l_otp_param)
					{
						FUNC_END();
						return	TRUE;
					}
					else
					{
						FUNC_END();
						return FAILED;
					}
				}
				else if((id == MV)||(id == MV_MANUAL) || (MV_MQA) || (MV_DQA))
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

		if ((data[0] == EVENTID_ERROR_V9) && (data[1] == EVENTID_ERRTYPE_LOCKDOWN_V9))
		{
			switch (data[2] & 0x0F)
			{
			case	0x01:	printf("[fts_lockdown_read] Error - no lockdown code\n");
				FUNC_END();
				return	FALSE;
			case	0x02:	printf("[fts_lockdown_read] Error - Data Corrupted\n");
				FUNC_END();
				return	FALSE;
			case	0x03:	printf("[fts_lockdown_read] Error - Command format invalid\n");
				FUNC_END();
				return	FALSE;
			}
		}
	}
	if (retry++ > 50)
		printf("[fts_lockdown_read] Error - Time over\n");

	printf("\n");
	printf(" > OTP READ DATA [0x%X]\n",param);

	FUNC_END();
	return	FALSE;
}

int fts_lockdown_write_v9(uint8_t command)
{
	/* Important : Maximum lockdown code size is 13 bytes */
	uint8_t lockdown_code[FTS_LOCKDOWNCODE_SIZE_V9] = { 0x42, 0x32, 0x31, 0x01, 0x43, 0x32, 0x31, 0x00 };
	uint8_t	regAdd[20], data[10];
	int		retry, i;

	FUNC_BEGIN();
	fts_systemreset_v9();
	if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) < TRUE)
	{
		printf("\n\r[fts_lockdown_write] FAILED - System Reset");
		FUNC_END();
		return	FALSE;
	}

	regAdd[0] = command;
	memcpy(regAdd + 1, lockdown_code, FTS_LOCKDOWNCODE_SIZE_V9);			// Copy locdown code
	regAdd[FTS_LOCKDOWNCODE_SIZE_V9 + 1] = lockdown_v9(lockdown_code);		// Calculate a CRC.
	if(fts_write_reg_v9(&regAdd[0], FTS_LOCKDOWNCODE_SIZE_V9 + 2))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	printf("[fts_lockdown_write] lockdown code : ");
	for (i = 0; i < FTS_LOCKDOWNCODE_SIZE_V9 + 2; i++)
		printf("0x%02X, ", regAdd[i]);
	printf("\n");

	retry = 50;
	while (retry--)
	{
		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) data, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		fts_delay_v9(50);
		printf("[fts_lockdown_write] read data : 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n"
							, data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
		if ((data[0] == EVENTID_SYSTEM_STATUS_UPDATE_V9) && (data[1] == EVENTID_STATID_LOCKDOWN_WRITE_DONE_V9))
		{
			printf("[fts_lockdown_write] lockdown code wrote successfully [%d]\n", data[2]);
			FUNC_END();
			return	TRUE;
		}

		if ((data[0] == EVENTID_ERROR_V9) && (data[1] == EVENTID_ERRTYPE_LOCKDOWN_V9))
		{
			switch (data[2] & 0x0F)
			{
			case	0x01:	printf("[fts_lockdown_write] Error - Already locked down\n");
				FUNC_END();
				return	FALSE;
			case	0x02:	printf("[fts_lockdown_write] Error - CRC Check Failed.\n");
				FUNC_END();
				return	FALSE;
			case	0x03:	printf("[fts_lockdown_write] Error - Command format invalid\n");
				FUNC_END();
				return	FALSE;
			case	0x04:	printf("[fts_lockdown_write] Error - Memory corrupted\n");
				FUNC_END();
				return	FALSE;
			}
		}
		else
		{
			if (data[0])
				printf("[fts_lockdown_write] Error - %02x %02x %02x %02x %02x %02x %02x %02x\n"
									, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		}
	}
	if (retry <= 0)
		printf("[fts_lockdown_write] Error - Time over\n");

	FUNC_END();
	return	FALSE;
}
#endif

int fts_get_pure_autotune_flag_v9(void)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[4] = {0};
	int				ret = FALSE;

	FUNC_BEGIN();
	regAdd[2] = FTS_READ_PAT_FLAG_V9;
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], 3))
	{
		FUNC_END();
		return I2C_ERR;
	}

	printf("[fts_get_pure_autotune_flag] Pure auto-tune flag [0x%02X] [0x%02X]\n", tempBuf[1], tempBuf[2]);
	if ((tempBuf[1] == 0xA5) && (tempBuf[2] == 0x96))
		ret = TRUE;

	FUNC_END();
	return ret;
}

#ifdef	FTS_PURE_AUTOTUNE_FEATURE_V9

int fts_get_afe_status_v9(void)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[4] = {0};
	int				ret = DISABLE;

	FUNC_BEGIN();
	regAdd[2] = FTS_GET_AFE_STATUS_V9;
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], 3))
	{
		FUNC_END();
		return I2C_ERR;
	}

	printf("[fts_get_afe_status] Final AFE Status [0x%02X], AFE Ver. [0x%02X]\n", tempBuf[1], tempBuf[2]);
	if (tempBuf[1] == 0x1)
		ret = ENABLE;

	FUNC_END();
	return ret;
}

int fts_clear_pure_autotune_flag_v9(void)
{
	uint8_t		regAdd[3] = {0xC8, 0x01, 0x00};

	FUNC_BEGIN();
	if(fts_write_reg_v9(&regAdd[0], 2))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_PAT_CLEAR_V9, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	fts_delay_v9(1);
	fts_command_v9(FLASH_CX_BACKUP_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_CX_BACKUP_DONE_V9, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	TRUE;
}

int fts_set_pure_autotune_flag_v9(void)
{
	uint8_t		regAdd[3] = {0xC7, 0x01, 0x00};

	FUNC_BEGIN();
	if(fts_write_reg_v9(&regAdd[0], 2))
	{
		FUNC_END();
		return I2C_ERR;
	}
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_PAT_SET_V9, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	fts_delay_v9(1);
	fts_command_v9(FLASH_CX_BACKUP_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_CX_BACKUP_DONE_V9, 0x00) < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	FUNC_END();
	return	TRUE;
}

int fts_run_pure_autotune_v9(void)
{
	FUNC_BEGIN();
	fts_delay_v9(10);
	if (fts_get_afe_status_v9() < ENABLE)
	{
		printf("[fts_run_pure_autotune] Final AFE flag is disabled\n");
		fts_command_v9(FLASH_CX_BACKUP_V9);
		fts_delay_v9(300);
		if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_CX_BACKUP_DONE_V9, 0x00) < TRUE)
		{
			printf("[fts_run_pure_autotune] FAILED - Cx back-up\n");
			FUNC_END();
			return	FALSE;
		}
		printf("[fts_run_pure_autotune] Cx back-up is done\n");
		FUNC_END();
		return	NOT_FINAL;
	}

	fts_delay_v9(1);
	if (fts_set_pure_autotune_flag_v9() < TRUE)
	{
		printf("[fts_run_pure_autotune] FAILED - Flag setting Failed\n");
		FUNC_END();
		return	FAILED;
	}

	fts_delay_v9(50);
	fts_systemreset_v9();
	if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) < TRUE)
	{
		printf("[fts_run_pure_autotune] FAILED - System Reset\n");
		FUNC_END();
		return	FAILED;
	}

	fts_delay_v9(10);
	if (fts_get_pure_autotune_flag_v9() < TRUE)
	{
		printf("[fts_run_pure_autotune] FAILED - Pure auto-tune flag is not correct\n");
		FUNC_END();
		return	FAILED;
	}

	printf("[fts_run_pure_autotune] Pure procedure is done successfully\n");

	FUNC_END();
	return	DONE;
}
#endif

int fts_auto_protection_off_v9() //tt
{
	uint8_t regAdd[8];
	
	FUNC_BEGIN();
	regAdd[0] = 0xB0;	regAdd[1] = 0x03;	regAdd[2] = 0x60;	regAdd[3] = 0x00;
	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v9(1);
	FUNC_END();
	return	0;
}

int fts_keep_active_mode_v9() //tt
{
	uint8_t	regAdd[8];

	FUNC_BEGIN();
	/* New Command for keeping active mode for China projects */
	regAdd[0] = 0xC1;		regAdd[1] = 0x00;		regAdd[2] = 0x00;	regAdd[3] = 0x01;
	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v9(1);

	FUNC_END();
	return 0;
}

#if !defined(FTSD3_V9)
int fts_fw_wait_for_flash_ready_v9()
{
	uint8_t regAdd;
	uint8_t buf[3];
	int		retry = 0;

	FUNC_BEGIN();
	regAdd = FTS_CMD_READ_FLASH_STAT_V9;
	retry = 0;
	while (1)
	{
		if(fts_read_reg_v9(&regAdd, 1, (uint8_t *) buf, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if ((buf[0] & 0x01) == 0)
			break;
		
		if (retry++ > 300)
		{
			FUNC_END();
			return -1;
		}

		fts_delay_v9(10);
	}
	
	FUNC_END();
	return	0;
}

int fw_unlock_v9(void)
{
	int		ret = 0;
	uint8_t regAdd[4] = {0xF7, 0x74, 0x45, 0x0};

	FUNC_BEGIN();
	ret = fts_write_reg_v9(&regAdd[0], 4);

    if (ret <= 0)
        printf("\n\r[fw_unlock] fw_erase error \n");
    else
        printf("\n\r[fw_unlock] Flash Unlocked\n");
	

/*
	if (ret)
		printf("[fw_unlock] fw_erase error\n");
	else
		printf("[fw_unlock] Flash Unlocked\n");
*/ //...??? need debug	
	fts_delay_v9(1);
	
	FUNC_END();
	return	ret;
}

#endif

#if defined(FTSD3_V9)
uint32_t convU8toU32_v9(uint8_t *src)
{
	uint32_t	tmpData;
	FUNC_BEGIN();
	tmpData = (uint32_t) (((src[3] & 0xFF) << 24) + ((src[2] & 0xFF) << 16) + ((src[1] & 0xFF) << 8) + (src[0] & 0xFF));
	FUNC_END();
	return	tmpData;
}

int parseBinFile_v9(uint8_t *data, int fw_size, FW_FTB_HEADER_V9 *fw_header, int keep_cx)
{
	int			dimension, index;
	uint32_t	temp;
	int			file_type;

	FUNC_BEGIN();
	/* start the parsing */
	index = 0;
	fw_header->signature = convU8toU32_v9(&data[index]);
	if (fw_header->signature == FW_HEADER_FTB_SIGNATURE_V9)
	{
		printf("[parseBinFile] FW Signature - ftb file\n");
		file_type = BIN_FTB_V9;
	}
	else
	{
		printf("[parseBinFile] FW Signature - ftsxxx file. %08X\n", fw_header->signature);
		file_type = BIN_FTS256_V9;
		FUNC_END();
		return	file_type;
	}

	index += FW_BYTES_ALLIGN_V9;
	fw_header->ftb_ver = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->target = convU8toU32_v9(&data[index]);
	if (fw_header->target != 0x00007036)
	{
		printf("[parseBinFile] Wrong target version %08X ... ERROR\n", fw_header->target);
		FUNC_END();
		return	FALSE;
	}

	index += FW_BYTES_ALLIGN_V9;
	fw_header->fw_id = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->fw_ver = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->cfg_id = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->cfg_ver = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9 * 4;

	fw_header->ext_ver = convU8toU32_v9(&data[index]);
	printf("[parseBinFile] Version : External = %04X, FW = %04X, CFG = %04X\n", fw_header->ext_ver, fw_header->fw_ver, fw_header->cfg_ver);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->sec0_size = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->sec1_size = convU8toU32_v9(&data[index]);
	printf("[parseBinFile] sec0_size = %08X (%d bytes), sec1_size = %08X (%d bytes)\n", fw_header->sec0_size, fw_header->sec0_size, fw_header->sec1_size, fw_header->sec1_size);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->sec2_size = convU8toU32_v9(&data[index]);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->sec3_size = convU8toU32_v9(&data[index]);
	printf("[parseBinFile] sec2_size = %08X (%d bytes), sec3_size = %08X (%d bytes)\n", fw_header->sec2_size, fw_header->sec2_size, fw_header->sec3_size, fw_header->sec3_size);

	index += FW_BYTES_ALLIGN_V9;
	fw_header->hdr_crc = convU8toU32_v9(&data[index]);

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

    if (dimension + FW_HEADER_SIZE_V9 + FW_BYTES_ALLIGN_V9 != temp)
    {
		printf("[parseBinFile] Read only %d instead of %d... ERROR\n", fw_size, dimension + FW_HEADER_SIZE_V9 + FW_BYTES_ALLIGN_V9);
		FUNC_END();
		return	FALSE;
    }

	FUNC_END();
	return	file_type;
}

int wait_for_flash_ready_v9(uint8_t type)
{
    uint8_t	cmd[2] = {FLASH_CMD_READ_REGISTER_V9, type};
    uint8_t	readData;
    int		i, res = -1;

	FUNC_BEGIN();
    printf("[wait_for_flash_ready] Waiting for flash ready\n");
    for (i = 0; i < 1000 && res != 0; i++)
    {
		if(fts_read_reg_v9(cmd, sizeof(cmd), &readData, 1))
		{
			FUNC_END();
			return I2C_ERR;
		}
		res = readData & 0x80;
        fts_delay_v9(50);
    }

    if (i >= 1000 && res != 0)
    {
		printf("[wait_for_flash_ready] Wait for flash TIMEOUT! ERROR\n");
		FUNC_END();
        return	FALSE;
    }

    printf("[wait_for_flash_ready] Flash READY!");
	FUNC_END();
    return	TRUE;
}

int start_flash_dma_v9()
{
    int		status;
    uint8_t	cmd[3] = {FLASH_CMD_WRITE_REGISTER_V9, FLASH_DMA_CODE0_V9, FLASH_DMA_CODE1_V9};

	FUNC_BEGIN();
    printf("[start_flash_dma] Command flash DMA ...\n");
    if(fts_write_reg_v9(cmd, sizeof(cmd)))
	{
		FUNC_END();
		return I2C_ERR;
	}

    status = wait_for_flash_ready_v9(FLASH_DMA_CODE0_V9);

    if (status != TRUE)
    {
		printf("[start_flash_dma] start_flash_dma: ERROR\n");
		FUNC_END();
        return	FALSE;
    }
    printf("[start_flash_dma] flash DMA DONE!\n");
	FUNC_END();
    return	TRUE;
}

int fillFlash_v9(uint32_t address, uint8_t *data, int size)
{
    int		remaining;
    int		toWrite = 0;
    int		byteBlock = 0;
    int		wheel = 0;
    uint32_t	addr = 0;
    int		res;
    int		delta;

    uint8_t		buff[DMA_CHUNK_V9 + 3] = {0};

	FUNC_BEGIN();
    remaining = size;
    while (remaining > 0)
    {
		byteBlock = 0;
		addr =0;
		printf("[fillFlash] [%d] Write data to memory.\n", wheel);
        while (byteBlock < FLASH_CHUNK_V9 && remaining > 0)
        {
            buff[0] = FLASH_CMD_WRITE_64K_V9;
            if (remaining >= DMA_CHUNK_V9)
            {
                if ((byteBlock + DMA_CHUNK_V9) <= FLASH_CHUNK_V9)
                {
                    toWrite = DMA_CHUNK_V9;
                    remaining -= DMA_CHUNK_V9;
                    byteBlock += DMA_CHUNK_V9;
                }
                else
                {
                    delta = FLASH_CHUNK_V9 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }
            else
            {
                if ((byteBlock + remaining) <= FLASH_CHUNK_V9)
                {
                    toWrite = remaining;
                    byteBlock += remaining;
                    remaining = 0;

                }
                else
                {
                    delta = FLASH_CHUNK_V9 - byteBlock;
                    toWrite = delta;
                    remaining -= delta;
                    byteBlock += delta;
                }
            }

            buff[1] = (uint8_t) ((addr & 0x0000FF00) >> 8);
            buff[2] = (uint8_t) (addr & 0x000000FF);
            memcpy(&buff[3], data, toWrite);
            if(fts_write_reg_v9(buff, 3 + toWrite))
			{
				FUNC_END();
				return I2C_ERR;
			}

            addr += toWrite;
            data += toWrite;
        }

        //configuring the DMA
        printf("[fillFlash] [%d] Configure DMA\n", wheel);
        byteBlock = byteBlock / 4 - 1;

        buff[0] = FLASH_CMD_WRITE_REGISTER_V9;
        buff[1] = FLASH_DMA_CONFIG_V9;
        buff[2] = 0x00;
        buff[3] = 0x00;

        addr = address + ((wheel * FLASH_CHUNK_V9)/4);
        buff[4] = (uint8_t) ((addr & 0x000000FF));
        buff[5] = (uint8_t) ((addr & 0x0000FF00) >> 8);
        buff[6] = (uint8_t) (byteBlock & 0x000000FF);
        buff[7] = (uint8_t) ((byteBlock & 0x0000FF00)>> 8);
        buff[8] = 0x00;

        if(fts_write_reg_v9(buff, 9))
		{
			FUNC_END();
			return I2C_ERR;
		}
        fts_delay_v9(10);

        printf("[fillFlash] [%d] Start flash DMA\n", wheel);
        res = start_flash_dma_v9();
        if (res < TRUE) {
			printf("[fillFlash] Error during flashing DMA! ERROR\n");
			FUNC_END();
            return	FALSE;
        }
        printf("[fillFlash] [%d] DMA done\n", wheel);

        wheel++;
    }
	FUNC_END();
    return	TRUE;
}

#endif
int fw_download_v9(uint8_t *pFilename, FW_FTB_HEADER_V9 *fw_Header, int8_t block_type)
{
#if	defined(FTSD3_V9)
	uint32_t	FTS_TOTAL_SIZE = (256 * 1024);	// Total 256kB
	int			HEADER_DATA_SIZE = 32;

	int			res;
	uint8_t		regAdd[8] = {0};

	FUNC_BEGIN();
	//==================== System reset ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x52;		regAdd[2] = 0x34;
	if(fts_write_reg_v9(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(30);

	//==================== Warm Boot ====================
	regAdd[0] = 0xB6;	regAdd[1] = (ADDR_WARM_BOOT_V9 >> 8) & 0xFF;	regAdd[2] = ADDR_WARM_BOOT_V9 & 0xFF;	regAdd[3] = WARM_BOOT_VALUE_V9;
	if(fts_write_reg_v9(&regAdd[0],4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(30);

	//==================== Unlock Flash ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x74;		regAdd[2] = 0x45;
	if(fts_write_reg_v9(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(30);

	//==================== Unlock Erase & Programming Operation ====================
	regAdd[0] = 0xFA;		regAdd[1] = 0x72;		regAdd[2] = 0x03;
	if(fts_write_reg_v9(&regAdd[0],3))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v9(30);

	//==================== Erase full Flash ====================
	regAdd[0] = 0xFA;		regAdd[1] = 0x02;		regAdd[2] = 0xC0;
	if(fts_write_reg_v9(&regAdd[0],3))
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v9(200);

	//========================== Write to FLASH ==========================
	if (block_type == BIN_FTB_V9)
	{
		printf("[fw_download] Start sec0 program\n");
		res = fillFlash_v9(FLASH_ADDR_CODE_V9, &pFilename[FW_HEADER_SIZE_V9], fw_Header->sec0_size);
		if (res != TRUE)
		{
			printf("[fw_download] Error - load sec0 program\n");
			FUNC_END();
			return	FALSE;
		}
		printf("[fw_download] load sec0 program DONE!\n");
		printf("[fw_download] Start sec1 program\n");
		res = fillFlash_v9(FLASH_ADDR_CONFIG_V9, &pFilename[FW_HEADER_SIZE_V9 + fw_Header->sec0_size], fw_Header->sec1_size);
		if (res != TRUE)
		{
			printf("[fw_download] Error - load sec1 program\n");
			FUNC_END();
			return	FALSE;
		}
		printf("[fw_download] load sec1 program DONE!\n");

		printf("[fw_download] Flash burn COMPLETED!\n");
	}
	else
	{
		printf("[fw_download] Start firmware downloading\n");
		res = fillFlash_v9(FLASH_ADDR_CODE_V9, &pFilename[HEADER_DATA_SIZE], FTS_TOTAL_SIZE);
		if (res != TRUE)
		{
			printf("[fw_download] Error - load sec0 program\n");
			FUNC_END();
			return	FALSE;
		}
	}

	//==================== System reset ====================
	regAdd[0] = 0xF7;		regAdd[1] = 0x52;		regAdd[2] = 0x34;
	if(fts_write_reg_v9(&regAdd[0],3))
	{
		FUNC_END();
		return I2C_ERR;
	}
    if (fts_cmd_completion_check_v9(0x10, 0x00, 0x00) < TRUE)
	{
		printf("[fw_download] Error - System Reset FAILED\n");
		FUNC_END();
		return	FALSE;
	}

#else	//FTSD2

	uint32_t	size = 0;
	uint_t		section = 0;
	uint_t		offset = 0;
	uint8_t		buf[WRITE_CHUNK_SIZE_V9 + 2] = {0};
	uint8_t		*pData;
	uint8_t		regAdd[8] = {0};
	uint_t		*parsing_size;
	int			rtn = 0;
	int			Result = -1;

	FUNC_BEGIN();
	fts_systemreset_v9();
	fts_delay_v9(50);

	if (fts_read_chip_id_v9() < TRUE)
	{
		FUNC_END();
		return	FALSE;
	}

	//copy to PRAM
	switch (block_type)
	{
		case	BINARY_V9:
			size = FTS_BIN_SIZE_V9;
			offset = 0;
			break;
		default:
			printf("Error !!! This is not block type\n");
			break;
	}

	//Check busy flash
	fts_fw_wait_for_flash_ready_v9();

	pData = pFilename;
	
	//FTS_CMD_UNLOCK_FLASH
	if (fw_unlock_v9() <= 0)
	{
		FUNC_END();
		return	FALSE;
	}

	if (block_type == BINARY_V9)
	{
		for (section = 0; section < ( size / WRITE_BURN_SIZE_V9) ; section++)
		{
			buf[0] = FTS_CMD_WRITE_PRAM_V9 + (((section * WRITE_BURN_SIZE_V9) >> 16) & 0x0f);
			buf[1] = ((section * WRITE_BURN_SIZE_V9) >> 8) & 0xFF;
			buf[2] = (section * WRITE_BURN_SIZE_V9) & 0xFF;
			memcpy(&buf[3], &pData[section * WRITE_BURN_SIZE_V9 + sizeof(struct fts64_header_v9)], WRITE_BURN_SIZE_V9);
			if(fts_write_reg_v9(&buf[0], WRITE_BURN_SIZE_V9 + 3))
			{
				FUNC_END();
				return I2C_ERR;
			}
			fts_delay_v9(1);		// adding delay
		}
	}

	//Erase Program Flash
	fts_command_v9(FTS_CMD_ERASE_PROG_FLASH_V9);
	fts_delay_v9(200);

	//Check busy Flash
	fts_fw_wait_for_flash_ready_v9();

	//Burn Program Flash
	fts_command_v9(FTS_CMD_BURN_PROG_FLASH_V9);
	fts_delay_v9(200);

	//Check busy Flash
	fts_fw_wait_for_flash_ready_v9();

	//Reset FTS
	fts_systemreset_v9();

	fts_delay_v9(50);
	
#endif
	FUNC_END();
	return	TRUE;
}

uint8_t fts_get_ms_data_v9(int tx_num, int rx_num, uint16_t *ms_data, uint8_t type_addr)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[FTS_RX_LENGTH_V9 * 2 + 1] = {0};
	uint16_t	offsetAddr = 0, startAddr = 0, endAddr = 0, writeAddr = 0;
	int			col_cnt =0, row_cnt = 0;

	FUNC_BEGIN();
	regAdd[2] = type_addr;
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], FTS_EVENT_SIZE_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}

	offsetAddr = tempBuf[DOFFSET_V9] +  (tempBuf[DOFFSET_V9 + 1] << 8);

	if (type_addr == FTS_MS_KEY_RAW_ADDR_V9)
		startAddr = offsetAddr;
	else
		startAddr = offsetAddr + rx_num * 2;
	endAddr = startAddr + tx_num * rx_num * 2;

	fts_delay_v9(10);
	memset(&tempBuf[0], 0x0, sizeof(tempBuf));
	
	for (writeAddr = startAddr; writeAddr < endAddr; col_cnt++, writeAddr += ((rx_num * 2)))
	{
		regAdd[1] = (writeAddr >> 8) & 0xFF;
		regAdd[2] = writeAddr & 0xFF;

		if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], rx_num * 2 + 1))
		{
			FUNC_END();
			return I2C_ERR;
		}
		for (row_cnt = 0; row_cnt < rx_num; row_cnt++)
		{
			ms_data[col_cnt * rx_num + row_cnt] = tempBuf[row_cnt * 2 + DOFFSET_V9] + (tempBuf[row_cnt * 2 + DOFFSET_V9 + 1] << 8);
		}
	}
	FUNC_END();
	return 0;
}

int fts_get_ms_cx_data_v9(uint8_t *cx_data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[FTS_RX_LENGTH_V9 + DOFFSET_V9];
	uint8_t		tuning_ver, cx1_val;
	uint16_t	node_startAddr, temp_addr;
	int			i, j, tx_num, rx_num;

	FUNC_BEGIN();
	/* Request to read compensation data */
	regAdd[0] = 0xB8;
	regAdd[1] = (uint8_t) sel_data;
	regAdd[2] = (uint8_t) (sel_data >> 8);
	if(fts_write_reg_v9(regAdd, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_cmd_completion_check_v9(EVENTID_COMPENSATION_READ_V9, regAdd[1], regAdd[2]);

	/* Read the offset address for compensation data */
	regAdd[0] = 0xD0; regAdd[1] = 0x00; regAdd[2] = 0x50;
	if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	node_startAddr = tempBuf[0 + DOFFSET_V9] + (tempBuf[1 + DOFFSET_V9] << 8) + FTS_COMP_NODEDATA_OFFSET_V9;
	fts_delay_v9(1);

	/* Read global and compensation header */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V9]; regAdd[2] = tempBuf[0 + DOFFSET_V9];
	if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, 16 + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	tx_num = tempBuf[4 + DOFFSET_V9]; rx_num = tempBuf[5 + DOFFSET_V9];
	printf("[FTS] tx : %d, rx : %d\n", tx_num, rx_num);

	//TUSUKANII
	tx_num = 16;
	rx_num = 32;

	tuning_ver = tempBuf[8 + DOFFSET_V9]; cx1_val = tempBuf[9 + DOFFSET_V9];
	printf("[FTS] tuning version : %d, Cx1 : %d\n", tuning_ver, cx1_val);
	fts_delay_v9(1);

	/* Read node compensation data */
	for (j = 0; j < tx_num; j++)
	{
		temp_addr = node_startAddr + (rx_num * j);
		regAdd[0] = 0xD0; regAdd[1] = (uint8_t) (temp_addr >> 8); regAdd[2] = (uint8_t) temp_addr;
		if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, rx_num + DOFFSET_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		for (i = 0; i < rx_num; i++)
		{
		#ifdef LIMIT_MUTUAL_TOTAL_CX_V9
			cx_data[j * rx_num + i] = (cx1_val * 8) + tempBuf[i + DOFFSET_V9];
		#else
			cx_data[j * rx_num + i] = tempBuf[i + DOFFSET_V9];
		#endif
		}
	}

	FUNC_END();
	return	TRUE;
}

uint8_t fts_get_ss_data_v9(int tx_num, int rx_num, uint16_t *ss_f_data, uint16_t *ss_s_data, uint8_t type_addr)
{
	uint8_t		regAdd[3] = {0xD0, 0x00, 0x00};
	uint8_t		tempBuf[FTS_RX_LENGTH_V9 * 2 + 1] = {0};
	uint16_t		offsetAddr = 0;
	int			count=0;

	FUNC_BEGIN();
	regAdd[1] = 0x00;
	regAdd[2] = type_addr;
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}

	offsetAddr = tempBuf[DOFFSET_V9] +  (tempBuf[DOFFSET_V9 + 1] << 8);
	regAdd[1] = (offsetAddr >> 8) & 0xFF;
	regAdd[2] = offsetAddr & 0xFF;
	
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], tx_num * 2 + 1))
	{
		FUNC_END();
		return I2C_ERR;
	}
	for (count = 0; count < tx_num; count++)
	{
		ss_f_data[count] = tempBuf[count * 2 + DOFFSET_V9] + (tempBuf[count * 2 + DOFFSET_V9 + 1] << 8);
	}

	regAdd[1]  = 0x00;	
	regAdd[2] = type_addr + 2;	
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], 4))
	{
		FUNC_END();
		return I2C_ERR;	
	}
	
	offsetAddr = tempBuf[DOFFSET_V9] +  (tempBuf[DOFFSET_V9+1] << 8);
	regAdd[1] = (offsetAddr >> 8) & 0xFF;
	regAdd[2] = offsetAddr & 0xFF;
		
	if(fts_read_reg_v9(regAdd, 3, &tempBuf[0], rx_num * 2 + 1))
	{
		FUNC_END();
		return I2C_ERR;
	}
	for (count = 0; count < rx_num; count++)
	{
		ss_s_data[count] = tempBuf[count * 2 + DOFFSET_V9] + (tempBuf[count * 2 + DOFFSET_V9 + 1] << 8);
	}
	
	FUNC_END();
	return 0;
}

int fts_get_ss_ix_data_v9(int8_t *tx_ix1, uint8_t *ix_tx_data, int8_t *rx_ix1, uint8_t *ix_rx_data, uint8_t *cx_tx_data, uint8_t *cx_rx_data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[FTS_RX_LENGTH_V9 + DOFFSET_V9];
	uint8_t		tuning_ver, ix1_tx = 0, ix1_rx = 0;
	uint16_t	ix2_tx_addr, ix2_rx_addr, cx2_tx_addr, cx2_rx_addr;
	int			tx_num, rx_num;

	FUNC_BEGIN();
	/* Request to read compensation data */
	regAdd[0] = 0xB8;
	regAdd[1] = (uint8_t) sel_data;
	regAdd[2] = (uint8_t) (sel_data >> 8);
	if(fts_write_reg_v9(regAdd, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_cmd_completion_check_v9(EVENTID_COMPENSATION_READ_V9, regAdd[1], regAdd[2]);

	/* Read the offset address for mutual compensation data */
	regAdd[0] = 0xD0; regAdd[1] = 0x00; regAdd[2] = 0x50;
	if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	ix2_tx_addr = tempBuf[0 + DOFFSET_V9] + (tempBuf[1 + DOFFSET_V9] << 8) + FTS_COMP_NODEDATA_OFFSET_V9;
	fts_delay_v9(1);

	/* Read global and compensation header */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V9]; regAdd[2] = tempBuf[0 + DOFFSET_V9];
	if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, 16 + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	tx_num = tempBuf[4 + DOFFSET_V9]; rx_num = tempBuf[5 + DOFFSET_V9];
	printf("[FTS] tx : %d, rx : %d\n", tx_num, rx_num);
	ix2_rx_addr = ix2_tx_addr + tx_num;
	cx2_tx_addr = ix2_rx_addr + rx_num;
	cx2_rx_addr = cx2_tx_addr + tx_num;

	tuning_ver = tempBuf[8 + DOFFSET_V9];
	ix1_tx = tempBuf[9 + DOFFSET_V9]; ix1_rx = tempBuf[10 + DOFFSET_V9];
	printf("[FTS] tuning version : %d, Ix1 force : %d, Ix1 sense : %d\n", tuning_ver, ix1_tx, ix1_rx);
	*tx_ix1 = ix1_tx;
	*rx_ix1 = ix1_rx;
	
	/* Read data */
	regAdd[1] = (ix2_tx_addr >> 8) & 0xFF;
	regAdd[2] = ix2_tx_addr & 0xFF;
	if(fts_read_reg_v9(regAdd, 3, tempBuf, tx_num + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(ix_tx_data, tempBuf + 1, tx_num);
	fts_delay_v9(1);

	regAdd[1] = (ix2_rx_addr >> 8) & 0xFF;
	regAdd[2] = ix2_rx_addr & 0xFF;
	if(fts_read_reg_v9(regAdd, 3, tempBuf, rx_num + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(ix_rx_data, tempBuf + 1, rx_num);
	fts_delay_v9(1);
	
	regAdd[1] = (cx2_tx_addr >> 8) & 0xFF;
	regAdd[2] = cx2_tx_addr & 0xFF;
	if(fts_read_reg_v9(regAdd, 3, tempBuf, tx_num + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(cx_tx_data, tempBuf + 1, tx_num);
	fts_delay_v9(1);

	regAdd[1] = (cx2_rx_addr >> 8) & 0xFF;
	regAdd[2] = cx2_rx_addr & 0xFF;
	if(fts_read_reg_v9(regAdd, 3, tempBuf, rx_num + DOFFSET_V9))
	{
		FUNC_END();
		return I2C_ERR;
	}
	memcpy(cx_rx_data, tempBuf + 1, rx_num);
	fts_delay_v9(1);

	FUNC_END();
	return	TRUE;
}
#if	defined (FTS_SUPPORT_FORCE_FSR_V9)
int fts_get_force_data_v9(int tx_num, int rx_num, uint16_t *data, uint16_t sel_data)
{
	uint8_t		regAdd[4];
	uint8_t		tempBuf[(FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9) * 2 + DOFFSET_V9];
	int			i, j, tmpIdx;

	FUNC_BEGIN();
	fts_delay_v9(10);

	/* Read the offset address of selected data */
	regAdd[0] = 0xD0; regAdd[1] = (uint8_t) (sel_data >> 8); regAdd[2] = (uint8_t) sel_data;
	if(fts_read_reg(&regAdd[0], 3, tempBuf, 3))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(1);

	/* Read the selected data from offset address */
	regAdd[0] = 0xD0; regAdd[1] = tempBuf[1 + DOFFSET_V9]; regAdd[2] = tempBuf[0 + DOFFSET_V9];
	if(fts_read_reg_v9(&regAdd[0], 3, tempBuf, (FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9) * 2 + DOFFSET_V9))
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
			data[tmpIdx + i] = (uint16_t) tempBuf[(tmpIdx + i) * 2 + DOFFSET_V9] + (uint16_t) (tempBuf[(tmpIdx + i) * 2 + 1 + DOFFSET_V9] << 8);
		}
	}
	fts_delay_v9(10);

	FUNC_END();
	return	TRUE;
}
#endif
int fts_hw_reset_pin_check_v9()
{
	uint8_t	buf[8] = {0,};
	uint8_t	cnt = 20;

	FUNC_BEGIN();
	fts_command_v9(FLUSHBUFFER_V9);

	/*
	 * Drain down Reset pin to GND for 10ms by jig.
	 */

	while (cnt--)
	{
		fts_delay_v9(5);
		if(fts_read_reg_v9(fts_fifo_addr, 1, &buf[0], FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		if (buf[0] == 0x10 && buf[1] == 0x00 && buf[2] == 0x00)
		{
			printf("[fts_hw_reset_pin_check] OK\n");
			FUNC_END();
			return 0;
		}
	}

	if (cnt == 0)
		printf("[fts_hw_reset_pin_check] Error\n");

	FUNC_END();
	return -1;
}

int fts_interrupt_pin_check_v9()
{
	FUNC_BEGIN();
	fts_systemreset_v9();
	fts_delay_v9(50);
	if(fts_interrupt_control_v9(ENABLE) == I2C_ERR)		// Need to enable interrupt pin before status check
	{
		FUNC_END();
		return I2C_ERR;
	}

	/*
	 * Interrupt pin low check
	 *     If not low, return ERROR.
	 */

	if(fts_command_v9(FLUSHBUFFER_V9) == I2C_ERR)
	{
		FUNC_END();
		return	I2C_ERR;
	}
	fts_delay_v9(20);

	/*
	 * Interrupt pin high check here
	 *     If not high, return ERROR.
	 */
	FUNC_END();
	return 0;
}

int fts_panel_ito_test_v9(void)
{
	uint8_t	val[8];
	uint16_t	cnt = 100;
	int		err_flag = TRUE;
	char	*errortypes[16] = {"No Error", "F open", "S open", "F2G short", "S2G short", "F2V short", "S2V short", "F2F short", "S2S short",
							"F2S short", "FPC F open", "FPC S open", "Key F open", "Key S open", "Reserved", "Reserved"};

	FUNC_BEGIN();
	cnt = 100;
	do	{
		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V9))		
		{
			FUNC_END();
			return I2C_ERR;
		}
		fts_delay_v9(1);
	}	while (cnt-- && (val[0] != 0x0F && val[1] != 0x05));

	if(DEBUG_MODE)
	printf("[fts_panel_ito_test] ITO RESULT : ");

	cnt = 100;
	do
	{
		if (val[0] == 0x0F && val[1] == 0x05)
		{
			switch (val[2])
			{
				case	NO_ERROR_V9:
					if (val[3] == 0x00)
					{
						if(DEBUG_MODE)
						printf("ITO open / short test PASS!!\n");

						FUNC_END();
						return	TRUE;
					}
					break;
				case ITO_FORCE_OPEN_V9:
				case ITO_SENSE_OPEN_V9:
				case ITO_FORCE_SHRT_GND_V9:
				case ITO_SENSE_SHRT_GND_V9:
				case ITO_FORCE_SHRT_VCM_V9:
				case ITO_SENSE_SHRT_VCM_V9:
				case ITO_FORCE_SHRT_FORCE_V9:
				case ITO_SENSE_SHRT_SENSE_V9:
				case ITO_F2E_SENSE_V9:
				case ITO_FPC_FORCE_OPEN_V9:
				case ITO_FPC_SENSE_OPEN_V9:
				case ITO_KEY_FORCE_OPEN_V9:
				case ITO_KEY_SENSE_OPEN_V9:
				case ITO_RESERVED0_V9:
				case ITO_RESERVED1_V9:
				case ITO_RESERVED2_V9:
				case ITO_MAX_ERR_REACHED_V9:
					err_flag = FALSE;
				if(DEBUG_MODE)
					printf("ITO open / short test FAIL!! Error Type : %s, Channel : %d\n", errortypes[val[2]], val[3]);
					break;
			}
		}		
		if(fts_read_reg_v9(fts_fifo_addr, 1, (uint8_t *) val, FTS_EVENT_SIZE_V9))
		{
			FUNC_END();
			return I2C_ERR;
		}
		fts_delay_v9(1);
	}	while (cnt-- && (val[1] != 0x00 && val[2] != 0x00));

	fts_delay_v9(10);
	
	fts_systemreset_v9();
	fts_delay_v9(50);

	FUNC_END();
	return	err_flag;
}

int fts_panel_open_short_test_v9()
{
	uint8_t regAdd[8] = {0,};
	int ret = TRUE;

	FUNC_BEGIN();
	/* HW ITO OPEN/SHORT TEST */
#ifdef	FTS_SUPPORT_ITOTEST_HW_V9
	fts_systemreset_v9();
	fts_delay_v9(50);

	fts_command_v9(SLEEPOUT_V9);
	fts_delay_v9(10);

	fts_command_v9(FLUSHBUFFER_V9);
	fts_delay_v9(1);

	// HW short test
	regAdd[0] = 0xA7;
	regAdd[1] = 0x01;
	regAdd[2] = 0x00;
	if(fts_write_reg_v9(&regAdd[0], 3))		// 2 byte
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(200);

	if(fts_panel_ito_test_v9()<TRUE)
		ret = FALSE;
		
#endif

	/* SW ITO OPEN/SHORT TEST */
#ifdef	FTS_SUPPORT_ITOTEST_SW_V9
	fts_systemreset_v9();
	fts_delay_v9(50);

	fts_command_v9(SLEEPOUT_V9);
	fts_delay_v9(10);

	fts_command_v9(FLUSHBUFFER_V9);
	fts_delay_v9(1);

	regAdd[0] = 0xA7;
	if(fts_write_reg_v9(&regAdd[0], 1))		// 1 byte
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(200);

	if(fts_panel_ito_test_v9() < TRUE)
		ret = FALSE;
#endif

	FUNC_END();
	return ret;
}

#ifdef	FTS_SUPPORT_FORCE_FSR_V9
int fts_force_panel_test_v9(void)
{
	uint8_t		ms_force_cx_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
	uint16_t	ms_force_raw_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
	uint16_t	ms_force_str_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];

	FUNC_BEGIN();
	power_on_v9();
	fts_delay_v9(30);

	fts_systemreset_v9();
	if (fts_cmd_completion_check_v9(0x10, 0x00, 0x00) < TRUE)
	{
		printf("[FTS] System Reset FAILED\n");
		FUNC_END();
		return	FALSE;
	}

	fts_read_chip_id_v9();
	fts_get_versionInfo_v9();

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V9
	flashProcedure_v9(0, 0);
#endif

	fts_interrupt_control_v9(DISABLE);

	fts_command_v9(MUTUAL_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_MS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] Mutual Auto Tune FAILED\n");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V9
	fts_command_v9(SELF_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_SS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] Self Auto Tune FAILED\n");
		FUNC_END();
		return	FALSE;
	}
#endif

	/* Get cx value of mutual sensing */
	printf("[FTS] Force Cx Value\n");
	fts_get_ms_cx_data_v9(ms_force_cx_buf, FTS_COMP_MS_FORCE_SEL_V9);

	fts_auto_protection_off_v9();
	fts_keep_active_mode_v9();
	fts_delay_v9(10);

	fts_command_v9(SENSEON_V9);
	fts_delay_v9(100);

	/* Get the mutual raw data */
	printf("[FTS] Force Raw Data\n");
	fts_get_force_data_v9(FTS_FORCE_TX_LENGTH_V9, FTS_FORCE_RX_LENGTH_V9, ms_force_raw_buf, FTS_FORCE_RAW_ADDR_V9);
	fts_get_force_data_v9(FTS_FORCE_TX_LENGTH_V9, FTS_FORCE_RX_LENGTH_V9, ms_force_str_buf, FTS_FORCE_STR_ADDR_V9);

	FUNC_END();
	return	TRUE;
}

void fts_check_force_press_v9(void)
{
	uint16_t	ms_force_str_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
	int			nCount = 2;		/* user count */

	FUNC_BEGIN();
	while (nCount--)
	{
		fts_delay_v9(10);
		printf("[FTS] Force Strength Data\n");
		fts_get_force_data_v9(FTS_FORCE_TX_LENGTH_V9, FTS_FORCE_RX_LENGTH_V9, ms_force_str_buf, FTS_FORCE_STR_ADDR_V9);
	}
	FUNC_END();
}

int fts_force_test_proc_v9(void)
{
	FUNC_BEGIN();
	fts_force_panel_test_v9();
	fts_check_force_press_v9();

	FUNC_END();
	return	TRUE;
}
#endif

#ifdef	JIG_SMT_INSPECTION_EQUIPMENT_V9

/**
  * @brief  For Joan model, write some commands for SMT inspection equipment.
  * @param  None
  * @retval None
  */
int fts_write_commands_for_sensor_v9(void) //tt
{
	uint8_t	regAdd[8];

	FUNC_BEGIN();
	/* Just added for SMT inspection equipment from release version 0x0011. */
	regAdd[0] = 0xB0;		regAdd[1] = 0x07;		regAdd[2] = 0xB7;	regAdd[3] = 0x0C;
	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(5);

	regAdd[0] = 0xB0;		regAdd[1] = 0x07;		regAdd[2] = 0xBF;		regAdd[3] = 0x0C;
	if(fts_write_reg_v9(&regAdd[0], 4))
	{
		FUNC_END();
		return I2C_ERR;
	}
	fts_delay_v9(5);
}
#endif
int fts_do_autotune_v9(void)
{
	FUNC_BEGIN();
#ifdef PRJ_LGMC_V9
 #ifdef	JIG_SMT_INSPECTION_EQUIPMENT_V9
	/* Just added for SMT inspection equipment from release version 0x0011. */
	fts_write_commands_for_sensor_v9();
 #endif
#endif

	fts_command_v9(LPOSC_TRIM_V9);
	fts_delay_v9(100);

	fts_command_v9(MUTUAL_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_MS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] FAILED - Mutual Auto Tune\n");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V9
	fts_command_v9(SELF_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_SS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] FAILED - Self Auto Tune\n");
		FUNC_END();
		return	FALSE;
	}
#endif

	FUNC_END();
	return	TRUE;
}

int fts_init_v9(void)
{
	int ret=0;

	FUNC_BEGIN();
	ret = fts_systemreset_v9();
	if(ret == I2C_ERR)
	{
		FUNC_END();
		return I2C_ERR;
	}
	if (fts_cmd_completion_check_v9(0x10, 0x00, 0x00) < TRUE)
	{
		printf("[FTS] System Reset FAILED\n");
		FUNC_END();
		return	FALSE;
	}

	fts_read_chip_id_v9();

#ifdef	FTS_SUPPORT_FW_DOWNLOAD_V9
	flashProcedure_v9(0, 0);
#endif

	fts_interrupt_control_v9(DISABLE);
	fts_delay_v9(1);

	fts_command_v9(MUTUAL_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_MS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] Mutual Auto Tune FAILED\n");
		FUNC_END();
		return	FALSE;
	}

#ifdef	FTS_SUPPORT_SELF_SENSE_V9
	fts_command_v9(SELF_AUTO_TUNE_V9);
	fts_delay_v9(300);
	if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_SS_DONE_V9, 0x00) < TRUE)
	{
		printf("[FTS] Self Auto Tune FAILED\n");
		FUNC_END();
		return	FALSE;
	}
#endif

	fts_command_v9(SENSEON_V9);
	fts_delay_v9(50);
	printf("[FTS] Sense On\n");
#ifdef	FTS_SUPPORT_MS_KEY_V9
	fts_command_v9(MSKEY_SENSEON_V9);
	printf("[FTS] MS KEY Sense On\n");
#endif

	fts_command_v9(FLUSHBUFFER_V9);
	fts_delay_v9(50);

	fts_interrupt_control_v9(ENABLE);
	printf("[FTS] Interrupt Enable.\n");

	FUNC_END();
	return	TRUE;
}


int stm_get_coord_v9(event_stm *p, int mode)
{
    uint8_t     EventNum = 0;
    uint8_t     EventID = 0;
    uint16_t    evtcount = 0;
    uint8_t     data[FTS_EVENT_SIZE_V9 * FTS_FIFO_MAX_V9];

	FUNC_BEGIN();
    evtcount = 1;

#if defined(FTSD3_V9)
    evtcount = evtcount >> 8;
    evtcount = evtcount / 2;
#else
    evtcount = evtcount >> 10;
#endif

    if (evtcount > FTS_FIFO_MAX_V9)
        evtcount = FTS_FIFO_MAX_V9;

    evtcount = 1;
    if (evtcount > 0)
    {
        memset(data, 0x0, FTS_EVENT_SIZE_V9 * evtcount);
        i2c_smbus_read_i2c_block_data(stm_dev, READ_ALL_EVENT_V9, FTS_EVENT_SIZE_V9 * evtcount, data);
        for (EventNum = 0; EventNum < evtcount; EventNum++)
        {
            EventID = data[EventNum * FTS_EVENT_SIZE_V9] & 0x0F;

            if(EventID == EVENTID_LEAVE_POINTER_V9)
            {
                p->status = 0;
                //printf("Stm Released..[%d]\n",EventNum);
				FUNC_END();
                return 0;
            }
            else if (EventID == EVENTID_ENTER_POINTER_V9 || EventID == EVENTID_MOTION_POINTER_V9)
            {
                p->id = (data[EventNum * FTS_EVENT_SIZE_V9] >> 4) & 0x0F;
                p->x  = (data[2 + EventNum * FTS_EVENT_SIZE_V9] << 4) + ((data[4 + EventNum * FTS_EVENT_SIZE_V9] & 0xf0) >> 4);
                p->y  = (data[3 + EventNum * FTS_EVENT_SIZE_V9] << 4) +  (data[4 + EventNum * FTS_EVENT_SIZE_V9] & 0x0f);
                p->status = 1;
                //printf("[%d]X:%d / Y:%d\n",EventNum, p->x, p->y); 
				FUNC_END();
                return 1;
#ifdef  SINGLE_TOUCH
                printf("\n");
				FUNC_END();
                return 1;
#endif
                //break;
            }
        }
        //printf("\n");
    }
	FUNC_END();
    return 0;
}



void stm_close(void)
{
	FUNC_BEGIN();
    close(stm_dev);
	FUNC_END();
}

int stm_reset_v9(int mode)
{
	FUNC_BEGIN();
    fts_systemreset_v9();
	FUNC_END();
    return 0;
}

int stm_init_v9(int mode)
{
    unsigned long funcs;

	FUNC_BEGIN();
    stm_dev = open("/dev/i2c-13", O_RDWR);
    if (ioctl(stm_dev, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter "
        "functionality matrix: %s\n", strerror(errno));
		FUNC_END();
        return -1;
    }
    if (ioctl(stm_dev, I2C_SLAVE_FORCE, FTS_I2C_ADDR_V9>>1) < 0) {
        fprintf(stderr, "Error: Could not set address \n");
		FUNC_END();
        return -1;
    }
    printf("stm init completed..\n");
	FUNC_END();
    return 0;
}
///////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
int fts_panel_test_v9(int id, unsigned char *uart_buf)
{
	//uart_buf[4]~[7] --> touch state...
    int i,j;

    /****************************************************/
    // Modified by iamozzi...
    int         ms_cx_data_Hgap[FTS_TX_LENGTH_V9][FTS_RX_LENGTH_V9 - 1];
    int         ms_cx_data_Vgap[FTS_TX_LENGTH_V9 - 1][FTS_RX_LENGTH_V9];
    /****************************************************/

    /****************************************************/
    // Modified by iamozzi...
    // NOTE : Check Size of Array...
    int         ms_hf_cx_data_Hgap[FTS_TX_LENGTH_V9][FTS_RX_LENGTH_V9 - 1];
    int         ms_hf_cx_data_Vgap[FTS_TX_LENGTH_V9 - 1][FTS_RX_LENGTH_V9];
    /****************************************************/

    int8_t      ix1_tx = 0, ix1_rx = 0;
    uint16_t    ms_raw_data[FTS_TX_LENGTH_V9 * FTS_RX_LENGTH_V9];
    int16_t     ms_jitter_data[FTS_TX_LENGTH_V9 * FTS_RX_LENGTH_V9];
    uint16_t    ss_raw_f_buf[FTS_TX_LENGTH_V9], ss_raw_s_buf[FTS_RX_LENGTH_V9];
    uint8_t     ms_cx_data[FTS_TX_LENGTH_V9 * FTS_RX_LENGTH_V9];
    uint8_t     ss_ix_tx_data[FTS_TX_LENGTH_V9], ss_ix_rx_data[FTS_RX_LENGTH_V9];
    uint8_t     ss_cx_tx_data[FTS_TX_LENGTH_V9], ss_cx_rx_data[FTS_RX_LENGTH_V9];
#ifdef  FTS_SUPPORT_MS_KEY_V9
    uint16_t    ms_key_raw_data[FTS_MSKEY_TX_LENGTH_V9 * FTS_MSKEY_RX_LENGTH_V9];
    uint8_t     ms_key_cx_data[FTS_MSKEY_TX_LENGTH_V9 * FTS_MSKEY_RX_LENGTH_V9];
#endif
#ifdef  FTS_SUPPORT_FORCE_FSR_V9
    uint8_t     ms_force_cx_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
    uint16_t    ms_force_raw_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
    uint16_t    ms_force_str_buf[FTS_FORCE_TX_LENGTH_V9 * FTS_FORCE_RX_LENGTH_V9];
#endif
#ifdef  FTS_SUPPORT_CHK_HIGH_FREQ_V9
    uint8_t     ms_hf_cx_data[FTS_TX_LENGTH_V9 * FTS_RX_LENGTH_V9];
#endif
    int count_max = 0;
    int count_min = 0;
	int ret = 0;
	unsigned int	ret_state = 0;
	int fstat = 0;
    int ref_max = 0;
    int ref_min = 0;
    int cvt_data[FTS_TX_LENGTH_V9][FTS_RX_LENGTH_V9] = {{0,},};
    int txIdx = 0;
    int rxIdx = 0;
    int index = 0;

	FUNC_BEGIN();
    printf("\n\n< TEST VERSION 09Ver > \n");

#ifdef  FTS_HW_PIN_CHECK_V9
    if(fts_hw_reset_pin_check_v9() == I2C_ERR)
	{
    printf("\n--------------------------------------\n");

		printf("=======> I2C FAIL \n"); //mm
    printf("\n--------------------------------------\n");

		uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
		uart_buf[10] = l_hf_test_mode;
		FUNC_END();
		return I2C_ERR;
	}
    fts_interrupt_pin_check_v9();
#endif

    fts_systemreset_v9();
    if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) < TRUE)
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

    ret = fts_read_chip_id_v9();
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
		else
		{
			ret_state |= 1<<TOUCH_STM_PRODUCT_ID;
			printf("=======> PRODUCT ID FAIL \n");
		}
	}
	else
	{
		ret_state |= 0<<TOUCH_STM_PRODUCT_ID;
		printf("=======> PRODUCT ID PASS \n");
	}
	ret = 0;

    ret = fts_get_versionInfo_v9();

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

    printf("\n--------------------------------------\n");
    printf("[ TOUCH OTP READ ]\n");

	if(fts_lockdown_read_v9(id) < TRUE)
	{
		printf("=======> OTP READ FAIL \n");
		ret_state |= (1<<TOUCH_STM_OTP_READ);
	}
	else
	{
		printf("=======> OTP READ PASS \n");
		ret_state |= (0<<TOUCH_STM_OTP_READ);
	}

    printf("\n--------------------------------------\n");


#ifdef  FTS_SUPPORT_CX_BACKUP_V9
    /* If pure auto-tune flag is already set, CX BACKUP don't proceed. */
    if (fts_get_pure_autotune_flag_v9() == TRUE)
    {
        printf("[FTS] PAT is already set\n");
    }
    else
    {
        fts_do_autotune_v9();
        fts_command_v9(FLASH_CX_BACKUP_V9);
        fts_delay_v9(300);
        if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_CX_BACKUP_DONE_V9, 0x00) < TRUE)
        {
            printf("[FTS] FAILED - Cx back-up\n");
        }
    }
#else

    printf("\n--------------------------------------\n");
    printf("[ TOUCH Pure Autotune Flag Set Check ]\n");

    if (fts_get_pure_autotune_flag_v9() < TRUE)
    {
        printf("=======> PURE_AUTOTUNE_FLAG_SET_CHECK FAIL \n");
        ret_state |= (1<<TOUCH_STM_PURE_AUTOTUNE_FLAG_CHECK);
    }
    else
    {
        ret_state |= (0<<TOUCH_STM_PURE_AUTOTUNE_FLAG_CHECK);

        printf("=======> PURE_AUTOTUNE_FLAG_SET_CHECK PASS \n");
        //printf("[FTS] PAT is already set\n");
    }
#endif
    fts_delay_v9(10);

    printf("\n--------------------------------------\n");
/////////////////////////////////////////////170814
    fts_keep_active_mode_v9();
    fts_delay_v9(10);

    fts_command_v9(SENSEON_V9);
#ifdef  FTS_SUPPORT_MS_KEY_V9
    fts_command_v9(MSKEY_SENSEON_V9);
#endif
    if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_FCAL_DONE_V9, 0x23) < TRUE)
    {
        printf("[FTS] FAILED - Event(FCAL) is not reached\n");
    }
    fts_delay_v9(300);

    fts_command_v9(SENSEOFF_V9);
#ifdef  FTS_SUPPORT_MS_KEY_V9
    fts_command_v9(MSKEY_SENSEOFF_V9);
#endif
    fts_delay_v9(50);

    /***************************************************/
    /* LGD_STM_TEST :: Get mutual raw data */
    /***************************************************/

    printf("\n--------------------------------------\n");
    printf("[ CM REFERENCE RAW < Pure > ]\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_pat_cm_reference_raw[0],l_pat_cm_reference_raw[1]);

    fts_get_ms_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, ms_raw_data, FTS_MS_RAW_ADDR_V9);

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // mx_raw_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32


    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
        {
            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
            index = txIdx * FTS_RX_LENGTH_V9 + rxIdx;
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
    for (i = 0; i < FTS_TX_LENGTH_V9; i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V9; j++)
        {
            if (cvt_data[i][j] < l_pat_cm_reference_raw[0])
            {
                count_min++;
                if(DEBUG_MODE)
                printf("-%04d ", cvt_data[i][j]);
            }
            else if (cvt_data[i][j] > l_pat_cm_reference_raw[1])
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
        printf("=======> Pure_CM REFERENCE_RAW < OPEN > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_CM_REFERENCE_RAW;
        fstat = 1;
    }

    if(count_max)
    {
        printf("=======> Pure_CM REFERENCE_RAW < SHORT > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_CM_REFERENCE_RAW;
        fstat = 1;
    }


    if(!fstat)
    {
        printf("=======> Pure_CM REFERENCE_RAW PASS\n");
        ret_state |= 0 << TOUCH_STM_CM_REFERENCE_RAW;
    }

    fstat = 0;

    printf("MAX / MIN : %d / %d \n",count_max,count_min);
    printf("--------------------------------------\n\n");
    printf("--------------------------------------\n");
    printf("[ SELF RAW DATA ] : TX < Pure >\n");
    fts_get_ss_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, ss_raw_f_buf, ss_raw_s_buf, FTS_SS_F_RAW_ADDR_V9);

    count_min = 0;
    count_max = 0;
    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_pat_self_raw_tx[0],l_pat_self_raw_tx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_TX_buf : ");

    for(i=0; i<FTS_TX_LENGTH_V9; i++)
    {
        if(ss_raw_f_buf[i] < l_pat_self_raw_tx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_f_buf[i]);
        }
        else if(ss_raw_f_buf[i] > l_pat_self_raw_tx[1])
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
        printf("=======> Pure_SELF_RAW_TX < TOO LOW > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_TX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> Pure_SELF_RAW_TX < TOO HIGH > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_TX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> Pure_SELF_RAW_TX PASS\n");
        ret_state |= 0 << TOUCH_STM_SELF_RAW_TX;
    }

    fstat = 0;


    printf("MAX / MIN : %d / %d \n",count_max,count_min);

    printf("--------------------------------------\n\n");

    printf("--------------------------------------\n");
    printf("[ SELF RAW DATA ] : RX < Pure >\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_pat_self_raw_rx[0],l_pat_self_raw_rx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_RX_buf : ");
    for(i=0; i<FTS_RX_LENGTH_V9; i++)
    {

        if(ss_raw_s_buf[i] < l_pat_self_raw_rx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_s_buf[i]);
        }
        else if(ss_raw_s_buf[i] > l_pat_self_raw_rx[1])
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
        printf("=======> Pure_SELF_RAW_RX < TOO LOW > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_RX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> Pure_SELF_RAW_RX < TOO HIGH > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_SELF_RAW_RX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> Pure_SELF_RAW_RX PASS\n");
        ret_state |= 0 << TOUCH_STM_SELF_RAW_RX;
    }

    fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);
    if(DEBUG_MODE)
        printf("[FTS] Getting raw of self is done!!\n\n");
    printf("--------------------------------------\n\n");



    fts_systemreset_v9();
    if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) < TRUE)
    {
        printf("\n\r[FTS] FAILED - System Reset");
        printf("=======> NORMAL AUTOTUNE < SYSTEM RESET > FAIL\n");
    }

    if (fts_do_autotune_v9() == FALSE)
    {
        printf("=======> NORMAL AUTOTUNE < NORMAL AUTOTUNE SET > FAIL\n");
    }
//////////////////////////////////////////////

    /***************************************************/
    /* LGD_STM_TEST :: Get cx value of mutual sensing */
    /***************************************************/
	printf("\n--------------------------------------\n");
    printf("[ TOTAL-CX ]\n");
#ifdef PRJ_LGMC_V9
    fts_get_ms_cx_data_v9(ms_cx_data, FTS_COMP_MS_LP_SEL_V9);
#else
    fts_get_ms_cx_data_v9(ms_cx_data, FTS_COMP_MS_CX_SEL_V9);
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

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
        {
            index = txIdx * FTS_RX_LENGTH_V9 + rxIdx;
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
    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
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
    for (i = 0; i < FTS_TX_LENGTH_V9; i++)
    {
        for (j = 0; j < (FTS_RX_LENGTH_V9 - 1); j++)
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
    for (i = 0; i < (FTS_TX_LENGTH_V9 - 1); i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V9; j++)
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
    fts_get_ss_ix_data_v9(&ix1_tx, ss_ix_tx_data, &ix1_rx, ss_ix_rx_data, ss_cx_tx_data, ss_cx_rx_data, FTS_COMP_SS_IX_SEL_V9);
    //printf("ss_ix_tx_data : ");
    //printf("TOTAL IX (tx) [%d]--> \n",ix1_tx);
    ix1_tx *= 4;

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
#ifdef FTS_SUPPORT_MS_KEY_V9
    /* Get cx value of mutual key */
    fts_get_ms_cx_data_v9(ms_key_cx_data, FTS_COMP_MS_KEY_SEL_V9);
    printf("[FTS] Getting comp. data of MSKey is done.\n");
#endif

#ifdef  FTS_SUPPORT_FORCE_FSR_V9
    /* Get cx value of force sensor (FSR) */
    fts_get_ms_cx_data_v9(ms_force_cx_buf, FTS_COMP_MS_FORCE_SEL_V9);
    printf("[FTS] Getting comp. data of fsr is done.\n");
#endif

    fts_keep_active_mode_v9();
    fts_delay_v9(10);

    fts_command_v9(SENSEON_V9);
#ifdef  FTS_SUPPORT_MS_KEY_V9
    fts_command_v9(MSKEY_SENSEON_V9);
#endif
    if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_FCAL_DONE_V9, 0x23) < TRUE)
    {
        printf("[FTS] FAILED - Event(FCAL) is not reached\n");
    }
    fts_delay_v9(300);

    fts_command_v9(SENSEOFF_V9);
#ifdef  FTS_SUPPORT_MS_KEY_V9
    fts_command_v9(MSKEY_SENSEOFF_V9);
#endif
    fts_delay_v9(50);

    /***************************************************/
    /* LGD_STM_TEST :: Get mutual raw data */
    /***************************************************/

    printf("\n--------------------------------------\n");
    printf("[ CM REFERENCE RAW ]\n");

    count_min = 0;
    count_max = 0;

	if(DEBUG_MODE)
		printf("LowLimit = %d / HighLimit = %d\n",l_cm_reference_raw[0],l_cm_reference_raw[1]);

    fts_get_ms_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, ms_raw_data, FTS_MS_RAW_ADDR_V9);

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // mx_raw_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
        {
            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
            index = txIdx * FTS_RX_LENGTH_V9 + rxIdx;
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
    for (i = 0; i < FTS_TX_LENGTH_V9; i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V9; j++)
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

    fts_get_ms_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, (uint16_t *)ms_jitter_data, FTS_MS_JIT_ADDR_V9);

    /****************************************************/
    /* Added by iamozzi. 2017.06.22.                    */
    /****************************************************/

    // Convert To 2 Dimension Matrix From 1 Dimension Array
    // ms_jitter_data[] ->> cvt_data[][]
    // FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32

    for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
    {
        for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
        {
            cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
            index = txIdx * FTS_RX_LENGTH_V9 + rxIdx;
            cvt_data[txIdx][rxIdx] = ms_jitter_data[index];
        }
    }

    /****************************************************/

    /****************************************************/
    // Modified by iamozzi...
    for (i = 0; i < FTS_TX_LENGTH_V9; i++)
    {
        for (j = 0; j < FTS_RX_LENGTH_V9; j++)
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
    fts_get_ss_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, ss_raw_f_buf, ss_raw_s_buf, FTS_SS_F_RAW_ADDR_V9);

    count_min = 0;
    count_max = 0;
	if(DEBUG_MODE)
		printf("LowLimit = %d / HighLimit = %d\n",l_self_raw_tx[0],l_self_raw_tx[1]);

	if(DEBUG_MODE)
	    printf("ss_raw_TX_buf : ");

    for(i=0; i<FTS_TX_LENGTH_V9; i++)
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
    for(i=0; i<FTS_RX_LENGTH_V9; i++)
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

#ifdef  FTS_SUPPORT_MS_KEY_V9
    /* Get mutual key raw data */
    fts_get_ms_data_v9(FTS_MSKEY_TX_LENGTH_V9, FTS_MSKEY_RX_LENGTH_V9, ms_key_raw_data, FTS_MS_KEY_RAW_ADDR_V9);
    printf("[FTS] Getting raw of MSKey is done.\n");
#endif

#ifdef  FTS_SUPPORT_FORCE_FSR_V9
    fts_get_force_data_v9(FTS_FORCE_TX_LENGTH_V9, FTS_FORCE_RX_LENGTH_V9, ms_force_raw_buf, FTS_FORCE_RAW_ADDR_V9);
    fts_get_force_data_v9(FTS_FORCE_TX_LENGTH_V9, FTS_FORCE_RX_LENGTH_V9, ms_force_str_buf, FTS_FORCE_STR_ADDR_V9);
    printf("[FTS] Getting raw of fsr is done.\n");
#endif

#ifdef  FTS_SUPPORT_CHK_LPMODE_V9

	/**/
    fts_command_v9(LOWPOWER_MODE_V9);
    if (fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_FCAL_DONE_V9, 0x44) < TRUE)
    {
        printf("[FTS] FAILED - Event(FCAL) is not reached\n");
    }
    fts_delay_v9(300);
    fts_command_v9(SENSEOFF_V9);
////////////////////////////////////////////////////////////////////////////////// i haven't done...
    /***************************************************/
    /* LGD_STM_TEST :: Get self raw data (LP_MODE)*/
    /***************************************************/
    printf("--------------------------------------\n");
    printf("SELF RAW DATA (LP_MODE)	:  TX\n");
    printf("--------------------------------------\n");
    fts_get_ss_data_v9(FTS_TX_LENGTH_V9, FTS_RX_LENGTH_V9, ss_raw_f_buf, ss_raw_s_buf, FTS_SS_F_RAW_ADDR_V9);

////////////////
    count_min = 0;
    count_max = 0;
    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_lp_self_raw_tx[0],l_lp_self_raw_tx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_TX_buf : ");

    for(i=0; i<FTS_TX_LENGTH_V9; i++)
    {
        if(ss_raw_f_buf[i] < l_lp_self_raw_tx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_f_buf[i]);
        }
        else if(ss_raw_f_buf[i] > l_lp_self_raw_tx[1])
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

    printf("--------------------------------------\n");
    printf("SELF RAW DATA (LP_MODE) :  RX\n");
    printf("--------------------------------------\n");

    count_min = 0;
    count_max = 0;

    if(DEBUG_MODE)
        printf("LowLimit = %d / HighLimit = %d\n",l_lp_self_raw_rx[0],l_lp_self_raw_rx[1]);

    if(DEBUG_MODE)
        printf("ss_raw_RX_buf : ");
    for(i=0; i<FTS_RX_LENGTH_V9; i++)
    {
        if(ss_raw_s_buf[i] < l_lp_self_raw_rx[0])
        {
            count_min++;
            if(DEBUG_MODE)
                printf("-%03d ",ss_raw_s_buf[i]);
        }
        else if(ss_raw_s_buf[i] > l_lp_self_raw_rx[1])
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
        printf("=======> LP_SELF_RAW_RX < TOO LOW > FAIL [%d]\n",count_min);
        ret_state |= 1 << TOUCH_STM_LP_SELF_RAW_RX;
        fstat = 1;
    }
    if(count_max)
    {
        printf("=======> LP_SELF_RAW_RX < TOO HIGH > FAIL [%d]\n",count_max);
        ret_state |= 1 << TOUCH_STM_LP_SELF_RAW_RX;
        fstat = 1;
    }

    if(!fstat)
    {
        printf("=======> LP_SELF_RAW_RX PASS\n");
        ret_state |= 0 << TOUCH_STM_LP_SELF_RAW_RX;
    }

    fstat = 0;

        printf("MAX / MIN : %d / %d \n",count_max,count_min);
    if(DEBUG_MODE)
        printf("[FTS] Getting raw of self is done!!\n\n");
    printf("--------------------------------------\n\n");

    /*
     * Have to check the self raw data of tx under low-power mode.
     */
#endif
    printf("--------------------------------------\n");
    printf("OPEN SHORT TEST\n");

    ret = fts_panel_open_short_test_v9();
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
/////////////////////// test

////////////////// HF_TEST....
    int reHF = 0;
    for(reHF = 1; reHF < 4; reHF++){
    	/* Auto tune with High Frequency(A3 01) for China projects */
#ifdef  FTS_SUPPORT_CHK_HIGH_FREQ_V9
		if(fts_systemreset_v9() == I2C_ERR)
			
		if (fts_cmd_completion_check_v9(EVENTID_CONTROLLER_READY_V9, 0x00, 0x00) < TRUE)
		{
		    printf("[FTS] FAILED - System Reset\n");
		}

#ifdef PRJ_LGMC_V9
	    fts_command_v9(HF_MUTUAL_AT_V9);
#else
		regAdd[0] = 0xA3;       regAdd[1] = 0x01;
		if(fts_write_reg_v9(&regAdd, 2))
		{
			uart_buf[4] |= 1<<TOUCH_STM_I2C_CHECK;
			uart_buf[10] = l_hf_test_mode;
			FUNC_END();
			return I2C_ERR;
		}
#endif
	    fts_delay_v9(300);
#if 1

		if(l_hf_test_mode && fts_cmd_completion_check_v9(EVENTID_SYSTEM_STATUS_UPDATE_V9, EVENTID_STATID_AT_MS_DONE_V9, 0x00) == TRUE)
		{
#endif
			fstat = 0;
			printf("--------------------------------HF TEST[%d]--------------------------------\n",reHF);
			printf("Mutual Auto Tune with High Frequency Done\n");
			
			/***************************************************/
			/* LGD_STM_TEST :: Get cx value of mutual sensing (High Frequency) */
			/***************************************************/
			printf("\n--------------------------------------\n");
			printf("TOTAL-CX (HIGH FREQUENCY)\n");
			fts_get_ms_cx_data_v9(ms_hf_cx_data, FTS_COMP_MS_CX_SEL_V9);
			
			/****************************************************/
			/* Added by iamozzi. 2017.06.22.                    */
			/****************************************************/
			// Convert To 2 Dimension Matrix From 1 Dimension Array
			// ms_hf_cx_data[] ->> cvt_data[][]
			// FTS_TX_LENGTH = 16, FTS_RX_LENGTH = 32
			
			for (txIdx = 0; txIdx < FTS_TX_LENGTH_V9; txIdx++)
			{
			    for (rxIdx = 0; rxIdx < FTS_RX_LENGTH_V9; rxIdx++)
			    {
			        cvt_data[txIdx][rxIdx] = 0; // <<-- For Initialize...
			        index = txIdx * FTS_RX_LENGTH_V9 + rxIdx;
			        cvt_data[txIdx][rxIdx] = ms_hf_cx_data[index];
			    }
			}
	
	    	/****************************************************/
			count_min = 0;
			count_max = 0;
		    /****************************************************/
		    // Modified by iamozzi...
	
			for (i = 0; i < FTS_TX_LENGTH_V9; i++)
			{
			    for (j = 0; j < FTS_RX_LENGTH_V9; j++)
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
			else if(reHF <3)
			{
				fstat = 0;
				printf("MAX / MIN : %d / %d \n",count_max,count_min);
				printf("--------------------------------------\n\n");
				printf(">> HF FAIL.. TEST RESTART! [%d] \n",reHF);
				printf("----------------------------------------------------------------\n");
				continue;
			}
		
			fstat = 0;
			
			printf("MAX / MIN : %d / %d \n",count_max,count_min);
			
			printf("[FTS] Getting comp. data of mutual(HF) is done!!\n\n");
			printf("--------------------------------------\n\n");
			
			count_max = 0;
			count_min = 0;
			
			printf("--------------------------------------\n");
			printf("TOTAL-CX-HF-GAP(H)  (HIGH FREQUENCY)\n");
			for (i = 0; i < FTS_TX_LENGTH_V9; i++)
			{
				for (j = 0; j < (FTS_RX_LENGTH_V9 - 1); j++)
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

		    //printf("LowLimit = %d / HighLimit = %d\n",);
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
			else if(reHF <3)
			{
				printf("MAX / MIN : %d / %d \n",count_max,count_min);
				printf("--------------------------------------\n\n");
			    printf(">> HF FAIL.. TEST RESTART! [%d] \n",reHF);
				printf("----------------------------------------------------------------\n");
				fstat = 0;
			    continue;
			}
		
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
			
			for (i = 0; i < (FTS_TX_LENGTH_V9 - 1); i++)
			{
				for (j = 0; j < FTS_RX_LENGTH_V9; j++)
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
			else if(reHF <3)
			{
			printf("MAX / MIN : %d / %d \n",count_max,count_min);
			printf("--------------------------------------\n\n");
			    printf(">> HF FAIL.. TEST RESTART! [%d] \n",reHF);
				printf("----------------------------------------------------------------\n");
				fstat = 0;
			    continue;
			}
			
			fstat = 0;
			
			printf("MAX / MIN : %d / %d \n",count_max,count_min);
			printf("--------------------------------------\n\n");
			printf("----------------------------------------------------------------\n");
			break;
		
#if 1
			}
			else
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
					printf(">> HF FAIL.. TEST RESTART! [%d] \n",reHF);
					printf("----------------------------------------------------------------\n");
					fstat = 0;
				}
				else
				{
				    ret_state |= 0 << TOUCH_STM_CX2_HF;
				    ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_H;
				    ret_state |= 0 << TOUCH_STM_CX2_HF_GAP_V;
					printf("> HF_TEST OFF MODE \n");
					break;	
				}
			}
	} //for
#endif
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
	printf("\n");
	
	FUNC_END();
	return  TRUE;
}

	/////////////////////////////////////////////////////////////////
int limit_data_match_v9(int id, struct stm_touch_limit* limit)
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
	return	PASS;
}

void i2c_dev_match_v9(int i2c_dev)
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


