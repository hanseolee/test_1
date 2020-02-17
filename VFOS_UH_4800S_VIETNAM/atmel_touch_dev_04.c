#include <stdbool.h>
#include <stdio.h>  
#include <sys/types.h>  
#include <linux/types.h> 
#include <math.h>
#include <sysfs.h> 
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <type.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <spi.h>
#include <i2c-dev.h>
#include <atmel_touch_dev_04.h>
#include <unistd.h>
#include <model_dp150.h>

//#ifndef __packed
#define __packed __attribute__((packed))
//#endif


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define PT_CH_SIZE_X					16
#define PT_CH_SIZE_Y					34

/* Delay times */                           
#define MXT_RESET_TIME       200 /* msec */                                                                                                                                 
#define MXT_RESET_TIMEOUT   3000 /* msec */                                                                                                                                 
#define MXT_CRC_TIMEOUT     1000 /* msec */                                                                                                                             
#define MXT_FW_RESET_TIME   3000 /* msec */                                                                                                                                 
#define MXT_WAKEUP_TIME       25 /* msec */                                                                                                                                 
#define MXT_REGULATOR_DELAY  150 /* msec */                                                                                                                                 
#define MXT_CHG_DELAY        100 /* msec */                                                                                                                                 
#define MXT_POWERON_DELAY    150 /* msec */   

// opcodes
#define SPI_WRITE_REQ    0x01
#define SPI_WRITE_OK     0x81
#define SPI_WRITE_FAIL   0x41
#define SPI_READ_REQ     0x02
#define SPI_READ_OK      0x82
#define SPI_READ_FAIL    0x42
#define SPI_INVALID_REQ  0x04
#define SPI_INVALID_CRC  0x08

#define SPI_APP_DATA_MAX_LEN  64
//#define SPI_APP_DATA_MAX_LEN  128 + 2
#define SPI_APP_HEADER_LEN     6
// header 6 bytes + Data[]
// 0 opcode
// 1 address LSB
// 2 address MSB
// 3 length LSB
// 4 length MSB
// 5 CRC
// 6+ Data[]


#define SPI_BOOTL_HEADER_LEN   2

#define T117_BYTES_READ_LIMIT    1505 // 7 x 215 (T117 size)
#define SPI_APP_BUF_SIZE_WRITE  (SPI_APP_HEADER_LEN+SPI_APP_DATA_MAX_LEN)
#define SPI_APP_BUF_SIZE_READ   (SPI_APP_HEADER_LEN+T117_BYTES_READ_LIMIT)


/* Configuration file format version expected*/
#define MXT_CFG_MAGIC       "OBP_RAW V1"

/* Object types */
#define MXT_GEN_MESSAGEPROCESSOR_T5                5
#define MXT_GEN_COMMANDPROCESSOR_T6                6
#define MXT_GEN_POWERCONFIG_T7                     7
#define MXT_GEN_ACQUISITIONCONFIG_T8               8
#define MXT_TOUCH_KEYARRAY_T15                    15
#define MXT_SPT_COMMSCONFIG_T18                   18
#define MXT_SPT_GPIOPWM_T19                       19
#define MXT_PROCI_GRIPFACE_T20                    20
#define MXT_PROCG_NOISE_T22                       22
#define MXT_TOUCH_PROXIMITY_T23                   23
#define MXT_PROCI_ONETOUCH_T24                    24
#define MXT_SPT_SELFTEST_T25                      25
#define MXT_PROCI_TWOTOUCH_T27                    27
#define MXT_SPT_CTECONFIG_T28                     28
#define MXT_DEBUG_DIAGNOSTIC_T37				  37
#define MXT_SPT_USERDATA_T38                      38
#define MXT_PROCI_GRIP_T40                        40
#define MXT_PROCI_PALM_T41                        41
#define MXT_PROCI_TOUCHSUPPRESSION_T42            42
#define MXT_SPT_DIGITIZER_T43                     43
#define MXT_SPT_MESSAGECOUNT_T44                  44
#define MXT_SPT_CTECONFIG_T46                     46
#define MXT_PROCI_STYLUS_T47                      47
#define MXT_TOUCH_PROXKEY_T52                     52
#define MXT_GEN_DATASOURCE_T53                    53
#define MXT_GEN_MICRO_T56		                  56
#define MXT_GEN_MICRO_T65		                  65
#define MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71 71
#define MXT_PROCI_SYMBOLGESTUREPROCESSOR_T92      92
#define MXT_PROCI_TOUCHSEQUENCELOGGER_T93         93
#define MXT_TOUCH_MULTITOUCHSCREEN_T100          100
#define MXT_PROCI_ACTIVESTYLUS_T107              107

/* T5 Message Processor */
#define MXT_T5_REPORTID_VAL_NOMSG     0xff

/* T6 Command Processor */
#define MXT_T6_CFG_RESET_OFFSET       0
#define MXT_T6_CFG_BACKUPNV_OFFSET    1
#define MXT_T6_CFG_CALIBRATE_OFFSET   2
#define MXT_T6_CFG_REPORTALL_OFFSET   3
#define MXT_T6_CFG_DIAGNOSTIC_OFFSET  5

#define MXT_T6_CFG_RESET_VAL_TO_BOOTL    0xa5
#define MXT_T6_CFG_RESET_VAL_TO_APP      0x01 // any non zero and non 0xa5
#define MXT_T6_CFG_BACKUPNV_VAL_TO_NVM   0x55
#define MXT_T6_CFG_CALIBRATE_VAL_EXEC    0x01 // any non zero
#define MXT_T6_CFG_REPORTALL_VAL_EXEC    0x01 // any non zero

#define MXT_T6_MSG_STATUS_RESET_BIT     (1 << 7)
#define MXT_T6_MSG_STATUS_OFL_BIT       (1 << 6)
#define MXT_T6_MSG_STATUS_SIGERR_BIT    (1 << 5)
#define MXT_T6_MSG_STATUS_CAL_BIT       (1 << 4)
#define MXT_T6_MSG_STATUS_CFGERR_BIT    (1 << 3)
#define MXT_T6_MSG_STATUS_COMSERR_BIT   (1 << 2)





typedef __signed__ char __s8;                                     
typedef unsigned char __u8;    
typedef __signed__ short __s16;                            
typedef unsigned short __u16;    
typedef __signed__ int __s32;    
typedef unsigned int __u32;    

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;

void init_tch_power_set_for_dp150(int on)
{
	int power = 0;

	FUNC_BEGIN();

	//DPRINTF("dic_dev is %d\n",dic_dev);
	ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
	ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);
	ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
	ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

	usleep(1000);
#if 0
	if(on)
	{
		power = 1;
		ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);

		usleep(30000); //must need
		printf("Touch Power ON \n");
	}
	else
		printf("Touch Power OFF \n");
#endif
#if 1
	if(on)
	{
		power = 1;
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
		ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);

		power = 0;
		ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

		usleep(200000);
		//sleep(3);
		power = 1;
		ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
		ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

		usleep(30000); //must need
		printf("Touch Power ON \n");
	}
	else
		printf("Touch Power OFF \n");
#endif   
	FUNC_END();

}

//global
static u8 spi_tx_buf[SPI_APP_BUF_SIZE_WRITE];
//static int mux_device;
int mux_device;		// use both solomon and synaptics
struct address_table touch_table;

u8 p_for_dp150[120] = {0,};
u8 *ID_Info_for_dp150=p_for_dp150;

u8 mem_for_dp150[sizeof(u8)*130*11+1] = {0,};//130(64+64+2) * 11page + 1
u8 *read_buf_global_for_dp150 = mem_for_dp150;

struct atmel_04_touch_limit l_limit;
int mxt_write_blks_for_dp150(u16 start, u16 count, u8 *buf);

struct mxt_info
{
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct mxt_object
{
	u8 type;
	u16 start_address;
	u8 size_minus_one;
	u8 instances_minus_one;
	u8 num_report_ids;
} __packed;

#define MXT_OBJECT_START     0x07 // after struct mxt_info
#define MXT_INFO_CHECKSUM_SIZE  3 // after list of struct mxt_object


/* T7 Power Configuration */
struct t7_powerconfig
{
	u8 idle;
	u8 active;
} __packed;

enum t7_powerconfig_type
{
	MXT_T7_POWER_CFG_RUN       = 0,
	MXT_T7_POWER_CFG_DEEPSLEEP = 1,
	MXT_T7_POWER_CFG_POWERSAVE = 2,
	MXT_T7_POWER_CFG_GESTURE   = 3,
};


/* Firmware update context */
struct mxt_flash
{
	struct mxt_data *mxtdata;
	const struct firmware *fw;
	u8 *pframe;
	loff_t fw_pos;
	size_t frame_size;
	unsigned int frame_count;
	unsigned int frame_retry;
};


struct address_table
{
	u16 t5_cfg_address;
	u16 t6_cfg_address;
	u16 t7_cfg_address;
	u16 t8_cfg_address;
	u16 t15_cfg_address;
	u16 t18_cfg_address;
	u16 t25_cfg_address;
	u16 t37_cfg_address;
	u16 t44_cfg_address;
	u16 t46_cfg_address;
	u16 t56_cfg_address;
	u16 t65_cfg_address;
	u16 t71_cfg_address;
	u16 t93_cfg_address;
	u16 t100_cfg_address;
	u16 t107_cfg_address;
};

/* Each client has this additional data */
struct mxt_data
{
	struct spi_device *spidevice;
	struct input_dev *inputdev;
	char phys[64];      /* device physical location */
	const struct mxt_platform_data *mxtplatform;
	struct mxt_object *object_table;
	struct mxt_info *mxtinfo;
	void *raw_info_block;
	unsigned int chg_irq;
	unsigned int t100_max_x;
	unsigned int t100_max_y;
	bool t100_xy_switch;
	bool in_bootloader;
	bool force_update_fw;
	u16 mem_size;
	u8 t100_aux_ampl_idx;
	u8 t100_aux_area_idx;
	u8 t100_aux_vect_idx;
	struct bin_attribute mem_access_attr;
	bool debug_enabled;
	bool debug_v2_enabled;
	u8 *debug_msg_data;
	u16 debug_msg_count;
	struct bin_attribute debug_msg_attr;
	//    struct mutex debug_msg_lock;
	u8 max_reportid;
	u32 config_crc;
	u32 info_crc;
	u8 *msg_buf;
	u8 t6_status;
	bool update_input;
	u8 last_message_count;
	u8 t100_num_touchids;
	struct t7_powerconfig t7_powercfg;
	unsigned long t15_keyarray_keystatus;
	u8 t100_stylus_aux_pressure_idx;
	u8 t100_stylus_aux_xpeak_idx;
	u8 t100_stylus_aux_ypeak_idx;
	bool use_retrigen_workaround;
	u8 double_tap_enable;
	struct regulator *reg_vdd;
	struct regulator *reg_avdd;
	char *fw_name;
	char *cfg_name;
	struct mxt_flash *mxtflash;

	// objects whose configuration address
	// we want to cache rather than frequently
	// retrieve for read/write
	u16 t5_cfg_address;
	u16 t6_cfg_address;
	u16 t7_cfg_address;
	u16 t15_cfg_address;
	u16 t18_cfg_address;
	u16 t44_cfg_address;
	u16 t71_cfg_address;
	u16 t93_cfg_address;
	u16 t100_cfg_address;
	u16 t107_cfg_address;

	// size of the largest possible message
	// NOTE: all messages are shipped via T5
	u8 t5_msg_size;

	// objects that support sending messages
	// whose report_id we want to cache
	u8 t6_msg_reportid;
	u8 t100_msg_reportid_min;
	u8 t100_msg_reportid_max;

	/* Indicates whether device is in suspend */
	bool suspended;

	/* Indicates whether device is updating configuration */
	bool updating_config;
};

static int i2c_general_read(u16 addr, u16 len, u8 *buf, u8 crc)
{
	int rc;
	u8 temp_buf[300]={0xFF,};
	temp_buf[0] = 0x01;//ss0
	temp_buf[1] = 0x02;//read mode
	temp_buf[2] = addr & 0xFF;
	temp_buf[3] = (addr >> 8);
	temp_buf[4] = len & 0xFF;
	temp_buf[5] = (len >> 8);
	temp_buf[6] = crc;//crc
	FUNC_BEGIN();
	//DPRINTF("mux is %d\n",mux_device);
	rc = write(mux_device, temp_buf ,len+50);
	
	if(rc < 0){
		//printf("Read Fail.. \n");
		return FALSE;
	}
	usleep(10000);

	int r=0;
	int response_flag = 0;// 0x82
	for(r=0; r<2; r++){
	//	DPRINTF("mux is %d\n",mux_device);
		rc = read(mux_device, buf, len+50);
		if(rc >-1){

			int i;
			if(0){//only coding debug
				printf("\n - ReadBurst_read(%d) [ADDR:0x%X] [SIZE:%d] - ", rc, addr, len);
				for(i = 0; i <len+50; i++)
				{
					if(!(i % 6))
						printf("\n");
					printf("[%d:0x%X] ",i,*(buf+i));
				}
				printf("\n");
			}
			for(i = 0; i <len+50; i++)
			{
				if(*(buf+i) == 0x82){
					memmove(buf, (buf+i)+6,len);
					response_flag = 1;
					break;
				}
			}
			if(0){//only coding debug
				for(i = 0; i <len; i++)
				{
					if(!(i % 6))
						printf("\n");
					printf("[%d:0x%02X] ",i,*(buf+i));
				}
				printf("\n");
			}
			break;
		}
		usleep(10000);
		rc=write(mux_device, "0xFF", 1);
		printf("write dummy rc = %d\n",rc);
		usleep(10000);
	}
	if(rc < 0 || response_flag == 0 )
	{
		//printf("Read Fail.. \n");
		FUNC_END();
		return FALSE;
	}

	FUNC_END();
	return TRUE;
}
#define DUMMY 50
static int i2c_general_write(u16 addr, u16 len, u8 *data, u8 crc)
{
	int rc;
	u8 temp_buf[300]={0xFF,};

	temp_buf[0] = 0x01;//ss0
	temp_buf[1] = 0x01;//write mode
	temp_buf[2] = addr & 0xFF;
	temp_buf[3] = (addr >> 8);
	temp_buf[4] = len & 0xFF;
	temp_buf[5] = (len >> 8);
	temp_buf[6] = crc;//crc
	FUNC_BEGIN();

	int i=0;
	for(i=7; i<len+7; i++){
		temp_buf[i] = data[i-7];
		//printf(" - writeBurst [ADDR(%d):0x%02X] \n", i, temp_buf[i]); 
	}


	//DPRINTF("mux is %d\n",mux_device);
	rc = write(mux_device, temp_buf ,len+20);
	//printf("\n - WRITEBurst_write(%d) [ADDR:0x%X] [SIZE:%d] - ", rc, addr, len);
	usleep(10000);//need modify timing, 100000=fail
	
	u8 p[100] = {0,};
	u8 *buf = p;
	
	int r=0;
	for(r=0; r<2; r++){
		rc = read(mux_device, buf, len+30);
		if(rc > -1){
#if	0	/* debug print */
			printf("\n - WRITEBurst_read(%d) [ADDR:0x%X] [SIZE:%d] - ", rc, addr, len);
			for(i = 0; i <len+30; i++)
			{
				if(!(i % 4))
					printf("\n");
				printf("[%d:0x%02X] ",i,*(buf+i));
			}
			printf("\n");
#endif
			break;
		}

		rc=write(mux_device, "0xFF", 1);
		//printf("write retry - write dummy = %d\n",rc);
		usleep(100000);
	}
	if(rc < 0)
	{
		//printf("Write Fail.. \n");
		FUNC_END();
		return FALSE;
	}
	FUNC_END();
	return TRUE;
}

static u8 get_crc8_iter(u8 crc, u8 data)
{
	static const u8 crcpoly = 0x8c;
	u8 index = 8;
	u8 fb;
	do
	{
		fb = (crc ^ data) & 0x01;
		data >>= 1;
		crc >>= 1;
		if (fb)
		{
			crc ^= crcpoly;
		}
	} while (--index);
	return crc;
}

static u8 get_header_crc(u8 *p_msg)
{
	u8 calc_crc = 0;
	int i = 0;
	for (; i < SPI_APP_HEADER_LEN-1; i++)
	{
		calc_crc = get_crc8_iter(calc_crc, p_msg[i]);
	}
	return calc_crc;
}

static void spi_prepare_header(u8 *header,
		u8 opcode,
		u16 start_register,
		u16 count)
{
	header[0] = opcode;
	header[1] = start_register & 0xff;
	header[2] = start_register >> 8;
	header[3] = count & 0xff;
	header[4] = count >> 8;
	header[5] = get_header_crc(header);
}

static int __mxt_read_reg(u16 start_register, u16 len, u8 *val)
{
	usleep(MXT_WAKEUP_TIME);
	spi_prepare_header(spi_tx_buf, SPI_READ_REQ, start_register, len);
	if(!i2c_general_read(start_register, len, val, spi_tx_buf[5])){
		//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
		return FALSE;
	}
	return TRUE;
}

static int mxt_read_blks(u16 start, u16 count, u8 *buf)
{
	u16 offset = 0;
	u16 size;
	while (offset < count)
	{
		size = min(SPI_APP_DATA_MAX_LEN, count - offset);

		if(!__mxt_read_reg(start + offset, size, buf + offset)){
			//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
			return FALSE;
		}

		offset += size;
	}
	return TRUE;
}

static int mxt_read_page(u16 start, u16 count, u8 *buf)
{
	u16 offset = 0;
	u16 offset_for_count = 0;
	int ret_val = 0;
	u16 size = 0;
	u8 write_cmd = 0x31;
	
	u8 p[120] = {0,};
	u8 *trans_buf = p;
	

	while (offset_for_count < count-2)
	{
		//	printf("		>>> SSW <<< [%s %d] %s CALL ====== count - offset:%d %d %d\n", __FILE__, __LINE__, __FUNCTION__, count , offset_for_count, size);
		size = min(SPI_APP_DATA_MAX_LEN, count - offset_for_count);

		if(!__mxt_read_reg(start , size, trans_buf))
			return FALSE;
		memmove(buf+offset, trans_buf+2, 64 - 2);
		offset += 62;

		if(!__mxt_read_reg(start + 64, size, trans_buf))
			return FALSE;
		memmove(buf+offset, trans_buf, 64);
		offset += 64;

		if(!__mxt_read_reg(start + 64 + 64, size, trans_buf))
			return FALSE;
		memmove(buf+offset, trans_buf, 2);
		offset += 2;

		offset_for_count = offset + 2;//plus header

		//page_up
		write_cmd = 0x01;
		mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &write_cmd);
	}
	return ret_val;
}

static int __mxt_write_reg(u16 start_register, u16 len, u8 *val)
{
	usleep(MXT_WAKEUP_TIME);
	spi_prepare_header(spi_tx_buf, SPI_WRITE_REQ, start_register, len);

	if(!i2c_general_write(start_register, len, val, spi_tx_buf[5]))
		return FALSE;

	return TRUE;
}

int mxt_write_blks_for_dp150(u16 start, u16 count, u8 *buf)
{
	u16 offset = 0;
	u16 size;

	while (offset < count)
	{
		size = min(SPI_APP_DATA_MAX_LEN, count - offset);

		if(!__mxt_write_reg(start + offset, size, buf + offset))
			return FALSE;
		offset += size;
	}

	return TRUE;
}



static int mxt_parse_object_table(struct mxt_data *mxtdata, struct mxt_object *object_table)
{
	int i;

	for (i = 0; i < ID_Info_for_dp150[6]; i++)
	{
		struct mxt_object *object = object_table + i;
		switch (object->type)
		{
			case MXT_GEN_MESSAGEPROCESSOR_T5:
				touch_table.t5_cfg_address = object->start_address;         
				break;
			case MXT_GEN_COMMANDPROCESSOR_T6:
				touch_table.t6_cfg_address = object->start_address;         
				break;
			case MXT_GEN_POWERCONFIG_T7:
				touch_table.t7_cfg_address = object->start_address;         
				break;
			case MXT_SPT_COMMSCONFIG_T18:
				touch_table.t18_cfg_address = object->start_address;         
				break;
			case MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71:
				touch_table.t71_cfg_address = object->start_address;         
				break;
			case MXT_TOUCH_KEYARRAY_T15:
				touch_table.t15_cfg_address = object->start_address;         
				break;
			case MXT_SPT_MESSAGECOUNT_T44:
				touch_table.t44_cfg_address = object->start_address;         
				break;
			case MXT_PROCI_TOUCHSEQUENCELOGGER_T93:
				touch_table.t93_cfg_address = object->start_address;         
				break;
			case MXT_TOUCH_MULTITOUCHSCREEN_T100:
				touch_table.t100_cfg_address = object->start_address;         
				break;
			case MXT_PROCI_ACTIVESTYLUS_T107:
				touch_table.t107_cfg_address = object->start_address;         
				break;
			case MXT_DEBUG_DIAGNOSTIC_T37:
				touch_table.t37_cfg_address = object->start_address;         
				break;
			case MXT_SPT_SELFTEST_T25:
				touch_table.t25_cfg_address = object->start_address;         
				break;
			case MXT_GEN_MICRO_T56:
				touch_table.t56_cfg_address = object->start_address;         
				break;
			case MXT_GEN_ACQUISITIONCONFIG_T8:
				touch_table.t8_cfg_address = object->start_address;         
				break;
			case MXT_SPT_CTECONFIG_T46:
				touch_table.t46_cfg_address = object->start_address;         
				break;
			case MXT_GEN_MICRO_T65:
				touch_table.t65_cfg_address = object->start_address;         
				break;
		}
	}
	return TRUE;
}

static int mxt_read_info_block(struct mxt_data *mxtdata)
{
	size_t size;
	u8 num_objects;

	/* Read 7-byte ID information block starting at address 0 */
	size = sizeof(struct mxt_info);
	/* Read information block, starting at address 0 */
	//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	if(!mxt_read_blks(0, size, ID_Info_for_dp150)){
	//	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
		return FALSE;
	}
	/* Resize buffer to give space for the object table and checksum */
	num_objects = ID_Info_for_dp150[6];

	size += (num_objects * sizeof(struct mxt_object)) + MXT_INFO_CHECKSUM_SIZE;
	/* Read rest of info block */
	//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	if(!mxt_read_blks(MXT_OBJECT_START, size - MXT_OBJECT_START, read_buf_global_for_dp150 + MXT_OBJECT_START))
		goto err_free_buf;
	/* Parse object table information */
	//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	if(!mxt_parse_object_table(mxtdata, read_buf_global_for_dp150 + MXT_OBJECT_START))
		goto err_free_buf;
	//printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	return TRUE;
err_free_buf:
	return FALSE;
}

int mxt_initialize_for_dp150(void)         
{
	struct mxt_data *mxtdata;
	if(!mxt_read_info_block(mxtdata)){
		return FALSE;
	}
	return TRUE;
}

int initial_check_for_dp150(void)         
{
	if(!INT_CHG_PIN_check_for_dp150())
		printf("FAIL\n");
		return FALSE;
	if(!AVDD_check_for_dp150())
		printf("FAIL\n");
		return FALSE;
	printf("PASS\n");
	return TRUE;
}

void backup_reset_for_dp150(void){
	u8 write_cmd = 0x55;
	usleep(10000);
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+1, 1, &write_cmd);//backup
	usleep(100000);

	write_cmd = 0x01;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+0, 1, &write_cmd);//reset
	//brightsource
	//sleep(1);
	usleep(500000);		// 500m OK, 200ms touch raw data 0000, 300ms 0000 
}

bool atmel_init_i2c_set_slvAddr_depending_channel2_for_dp150(int ch, int slvAddr, unsigned int *result)
{
	char i2c_dev[30]="/dev/i2c-";
	char i2c_line = 13;

	FUNC_BEGIN();
	printf("CH : %d / Slave Addr : 0x%X \n", ch, slvAddr);

	if(ch == 1)
		i2c_line = 13;
	else
		i2c_line = 9;

	sprintf(i2c_dev,"%s%d",i2c_dev,i2c_line);
	printf("OPEN : %s \n",i2c_dev);

	mux_device = open(i2c_dev, O_RDWR);
	//DPRINTF("mux is %d\n",mux_device);
	if(mux_device < 0)
	{
		printf("I2C Device Open Failed..\n");
		FUNC_END();
		return false;
	}

	if (ioctl(mux_device, I2C_SLAVE_FORCE, slvAddr) < 0) {
		fprintf(stderr, "Error: Could not set address[reg:0x%X] \n",slvAddr);
		close(mux_device);
		mux_device = 0;
		FUNC_END();
		return false;
	}

	FUNC_END();
	return true;
}

bool release_i2c_set2_for_dp150(void)
{
	FUNC_BEGIN();

	if (mux_device > 0)
	{
		printf("CLOSE : i2c device\n");
		close(mux_device);
		mux_device = 0;
	}

	FUNC_END();
	return true;
}

int set_frequency_for_dp150()
{
	int ret;
	unsigned char i2c_send_buf[12];

	i2c_send_buf[0] = 0xF0;
	i2c_send_buf[1] = 0x00;//00=1843kHz, 01=461, 02=115, 03=58
	
	ret = write(mux_device, i2c_send_buf, 2);

	memset(i2c_send_buf, 0, 12);

	if(ret < 0)
	{
		FUNC_END();
		return -1;
	}

	FUNC_END();
	return 0;

}

int INT_CHG_PIN_check_for_dp150(void)
{
	u8 INACT_cmd = 0x02;//HIGH = inactive
	u8 ACT_cmd = 0x03;//LOW = active

	if(!mxt_write_blks_for_dp150(touch_table.t18_cfg_address+1, 1, &INACT_cmd)){
		return FALSE;
	}

	if(!mxt_write_blks_for_dp150(touch_table.t18_cfg_address+1, 1, &ACT_cmd)){
		return FALSE;
	}

	if(!mxt_write_blks_for_dp150(touch_table.t18_cfg_address+1, 1, &INACT_cmd)){
		return FALSE;
	}

	return TRUE;
}

int AVDD_check_for_dp150(void)
{
	u8 write_cmd = 0x01;
	int len = 6;
	u8 p[120] = {0,};
	u8 *read_buf = p;
	
	mxt_write_blks_for_dp150(touch_table.t25_cfg_address+1, 1, &write_cmd);
	mxt_read_blks(touch_table.t25_cfg_address, len, read_buf);

	if(DEBUG_MODE)
//		printf("0x%02X\n",read_buf[0]);		// LWG

	if(read_buf[0] != 0x01)
		return TRUE;
	return FALSE;
}

int FW_Ver_check_for_dp150(model_dp150_t* dp150_p)
{
	char result_buf[10]={0,};
	char result_buf2[10]={0,};
	
	int len1, len2;
	sprintf(result_buf, "%02X", ID_Info_for_dp150[2]);
	sprintf(result_buf2, "%C.%C.%02X",result_buf[0], result_buf[1], ID_Info_for_dp150[3]);
	//printf("[%s %d] %s CALL ====== %s = %s\n", __FILE__, __LINE__, __FUNCTION__, result_buf2,l_limit.fw_ver);
	//if(DEBUG_MODE)
	printf("Spec : %s = TouchIC : %s\n", l_limit.fw_ver, result_buf2);
	
	len1 = strlen("0.0.04");
	len2 = strlen(result_buf2);

	printf("len1 %d, len2 %d\n", len1, len2);
	if((len1 == len2) && (!strncmp("0.0.04", result_buf2, 6))){		// equal(0), not equal(n)
		dp150_p->touch_ic_kind = SOLOMON;
	}else{
		dp150_p->touch_ic_kind = SYNAPTICS;	// SYNAPTICS touch test function is in synaptics_touch_02.c, not in atmel_touch_dev_03.c
	}
	if(!strncmp(l_limit.fw_ver, result_buf2, 6)){
		return TRUE;
	}
	return FALSE;
}

int CONFIG_Ver_check_for_dp150(void)
{
	char result_buf[10]={0,};
	u8 p[120] = {0,};
	u8 *config_buf = p;
	
	u8 backup = 0x55;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+1, 1, &backup);//backup
	
	u8 reportall = 0x01;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+3, 1, &reportall);
	
	mxt_read_blks(touch_table.t5_cfg_address, 10, config_buf);

	sprintf(result_buf, "%02X%02X%02X", config_buf[4], config_buf[3], config_buf[2]);

	//if(DEBUG_MODE)
		printf("Spec : %s = TouchIC : %s\n", l_limit.config_ver, result_buf);

	if(!strncmp(l_limit.config_ver, result_buf, 6))
		return TRUE;
	return FALSE;
}

int atmel_04_product_id_check(void)
{
	u8 backup = 0x81;
	u8 p[120] = {0,};
	u8 *config_buf = p;
	char result_buf[100]={0,};

	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &backup);
	mxt_read_blks(touch_table.t37_cfg_address, 14, config_buf);

	memmove(config_buf, &(config_buf[4]),10);		// [4] --> [0] : 10bytes
	config_buf[10]=0x00;	//	NULL
	//sprintf(result_buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
	sprintf(result_buf, "%c%c%c%c%c%c%c%c%c%c%c",
		config_buf[0], config_buf[1], config_buf[2], config_buf[3], config_buf[4], 
		config_buf[5], config_buf[6], config_buf[7], config_buf[8], config_buf[9],
		config_buf[10]); 
	//if(DEBUG_MODE)
	//		printf("Spec : %s = TouchIC : %s\n", l_limit.product_id, config_buf);
			printf("Spec : %s = TouchIC : %s\n", l_limit.product_id, result_buf);
	//if(!strncmp(l_limit.product_id, (char *)config_buf, 10))
	if(!strncmp(l_limit.product_id, result_buf, 10))
		return TRUE;
	return FALSE;
}

int pin_fault_check_for_dp150(void)
{
	u8 write_cmd = 0x12;
	u8 p[120] = {0,};
	u8 *read_buf = p;

	int len = 6;

	mxt_write_blks_for_dp150(touch_table.t25_cfg_address+1, 1, &write_cmd);
	mxt_read_blks(touch_table.t25_cfg_address, len, read_buf);

//	if(DEBUG_MODE)
		printf("0x%02X\n", read_buf[0]);

	if(read_buf[0] == 0x12)
		return FALSE;
	return TRUE;
}

int force_touch_check_for_dp150(void)
{
	u8 write_cmd = 0xF8;
	u8 p[120] = {0,};
	u8 *read_buf = p;
	
	int len = 64;

	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &write_cmd);//ref selfcap mode
	write_cmd = 0x01;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &write_cmd);//page_up1
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &write_cmd);//page_up2
	mxt_read_blks(touch_table.t37_cfg_address, len, read_buf);

	int result, result2 = 0;
	result =  (unsigned int)((unsigned int)(read_buf[39]<<8)|(unsigned int)(read_buf[38]));
	result2 = (unsigned int)((unsigned int)(read_buf[41]<<8)|(unsigned int)(read_buf[40]));

	if(DEBUG_MODE){
		printf("%d < %d < %d\n", atoi((char *)l_limit.force_touch_MIN), result, atoi((char *)l_limit.force_touch_MAX));
		printf("%d < %d < %d\n", atoi((char *)l_limit.force_touch_MIN), result2, atoi((char *)l_limit.force_touch_MAX));
	}

	if((atoi((char *)l_limit.force_touch_MIN)<= result && result <= atoi((char *)l_limit.force_touch_MAX))){
		if((atoi((char *)l_limit.force_touch_MIN)<= result2 && result2 <= atoi((char *)l_limit.force_touch_MAX)))
			return TRUE;
	}
	return FALSE;
}



int atmel_04_Node_detection_check(void)
{
	int len = PT_CH_SIZE_X * PT_CH_SIZE_Y * 2 ;
	int page_len = 11;
	int one_page = (2 + 64 + 64);
	int byte_len = one_page * page_len;
		
//	u8 p[3000] = {0,};
//	u8 *read_buf = p;

	u8 write_cmd = 0x22;
	mxt_write_blks_for_dp150(touch_table.t8_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x0A;
	mxt_write_blks_for_dp150(touch_table.t46_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x00;
	mxt_write_blks_for_dp150(touch_table.t56_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x00;
	mxt_write_blks_for_dp150(touch_table.t65_cfg_address + 0, 1, &write_cmd);
	usleep(10000);
	
	backup_reset_for_dp150();

	write_cmd = 0x11;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address + 5, 1, &write_cmd);//ref mode
	usleep(10000);

	mxt_read_page(touch_table.t37_cfg_address, byte_len, read_buf_global_for_dp150);

	usleep(10000);
	int i=0;
	for(i=0; i< (16 * 34 * 2 / 32); i++)
		memmove(read_buf_global_for_dp150 + (i * 32), &(read_buf_global_for_dp150[i * 40]), 32);

	if(!node_data_check_for_dp150(read_buf_global_for_dp150, len, l_limit.nodeDetection_MIN, l_limit.nodeDetection_MAX))
		return FALSE;
	return TRUE;
}

int micro_defect_check_for_dp150(void)
{
	int len = PT_CH_SIZE_X * PT_CH_SIZE_Y * 2;
	int page_len = 11;
	int one_page = (2 + 64 + 64);
	int byte_len = one_page * page_len;

//	u8 p[3000] = {0,};
//	u8 *read_buf = p;

	u8 write_cmd = 0x11;
	mxt_write_blks_for_dp150(touch_table.t8_cfg_address+0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x0A;
	mxt_write_blks_for_dp150(touch_table.t46_cfg_address+0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x00;
	mxt_write_blks_for_dp150(touch_table.t56_cfg_address+0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x00;
	mxt_write_blks_for_dp150(touch_table.t65_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	backup_reset_for_dp150();

	write_cmd = 0x11;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address+5, 1, &write_cmd);//ref mode
	usleep(10000);

	mxt_read_page(touch_table.t37_cfg_address, byte_len, read_buf_global_for_dp150);
	int i=0;
	for(i=0; i< (16 * 34 * 2 / 32); i++)
		memmove(read_buf_global_for_dp150 + (i * 32), &(read_buf_global_for_dp150[i * 40]), 32);

	if(!micro_data_check_for_dp150(read_buf_global_for_dp150, len, (long *)l_limit.micro_defect))
		return FALSE;
	return TRUE;
}

int atmel_04_delta_limit_check(void)
{
	int len = PT_CH_SIZE_X * PT_CH_SIZE_Y * 2 ;
	int page_len = 11;
	int one_page = (2 + 64 + 64);
	int byte_len = one_page * page_len;

//	u8 p[3000] = {0,};
//	u8 *read_buf = p;


	u8 write_cmd = 0x22;
	mxt_write_blks_for_dp150(touch_table.t8_cfg_address + 0, 1, &write_cmd);
	usleep(10000);
	
	write_cmd = 0x02;
	mxt_write_blks_for_dp150(touch_table.t46_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x01;
	mxt_write_blks_for_dp150(touch_table.t56_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x01;
	mxt_write_blks_for_dp150(touch_table.t65_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	backup_reset_for_dp150();

	write_cmd = 0x10;
	mxt_write_blks_for_dp150(touch_table.t6_cfg_address + 5, 1, &write_cmd);//delta mode

	usleep(10000);

	mxt_read_page(touch_table.t37_cfg_address, byte_len, read_buf_global_for_dp150);
	int i=0;
	for(i=0; i< (16 * 34 * 2 / 32); i++)
		memmove(read_buf_global_for_dp150 + (i * 32), &(read_buf_global_for_dp150[i * 40]), 32);

	if(!delta_data_check_for_dp150(read_buf_global_for_dp150, len, l_limit.delta_MIN, l_limit.delta_MAX))
		return FALSE;
	return TRUE;
}

void config_restore_for_dp150(void){
	u8 write_cmd = 0x22;
	mxt_write_blks_for_dp150(touch_table.t8_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x0A;
	mxt_write_blks_for_dp150(touch_table.t46_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x01;
	mxt_write_blks_for_dp150(touch_table.t56_cfg_address + 0, 1, &write_cmd);
	usleep(10000);

	write_cmd = 0x00;
	mxt_write_blks_for_dp150(touch_table.t65_cfg_address + 0, 1, &write_cmd);
	usleep(10000);
	
	backup_reset_for_dp150();
}




int atmel_04_touch_limit_table_parser(MODEL id, char *m_name, struct atmel_04_touch_limit* limit)
{
	char string[500];
	FILE *fp;
	char *token = NULL;
	char file_name[50];
	char name_buf[30]={0,};
	int row = 0;
	int parsing_en = 0;
	int x = 0, y =0;
	int mode = 0;
	FUNC_BEGIN();

	sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_TABLE_FILE);

	if(id != limit->id)
		limit->id = id;

	if((fp=(fopen(file_name,"r"))) == 0 ){
		printf("%s : cannot open %s\n", __func__, file_name);
		FUNC_END();
		return -1;
	}

	sprintf(name_buf,"[%s]",m_name);
	printf("%s : %s searching... \n",__func__,name_buf);

	while((fgets(string, 500, fp)) != NULL){
		token = strtok(string, TOKEN_SEP_COMMA);
		while(token != NULL){
			if(!parsing_en)
			{
				if(!strcmp(token,name_buf))
				{
					printf("Found LIMIT DATA --> %s \n",name_buf);
					printf("%s : %s limit parser START.. \n",__func__,name_buf);
					parsing_en = 1;
				}
				break;
			}
			else
			{
				if(!strcmp(token,"END"))
				{
					parsing_en = 0;
					goto end;
				}

				if(!strcmp(token, "X"))
				{
					token = strtok(NULL, TOKEN_SEP_COMMA);
					x = strtoul(token, NULL,10);
					if(!strcmp(strtok(NULL, TOKEN_SEP_COMMA),"Y"))
					{
						token = strtok(NULL, TOKEN_SEP_COMMA);
						y = strtoul(token, NULL,10);
					}
				}

				switch(mode)
				{	
					case 0:
						if(!strcmp(token, "fw_ver"))
							mode = 1;
						else if(!strcmp(token, "config_ver"))
							mode = 2;
						else if(!strcmp(token, "product_id"))
							mode = 3;
						else if(!strcmp(token, "nodeDetection_MIN"))
							mode = 4;
						else if(!strcmp(token, "nodeDetection_MAX"))
							mode = 5;
						else if(!strcmp(token, "delta_MIN"))
							mode = 6;	
						else if(!strcmp(token, "delta_MAX"))
							mode = 7;
						else if(!strcmp(token, "micro_defect"))
							mode = 8;
						else if(!strcmp(token, "force_touch_MIN"))
							mode = 9;
						else if(!strcmp(token, "force_touch_MAX"))
							mode = 10;
						break;

					case 1:
						if(atmel_04_touch_csv_parser2(&x, &y, &row, token, limit->fw_ver))
							mode = 0; 
						break;
					case 2:
						if(atmel_04_touch_csv_parser2(&x, &y, &row, token, limit->config_ver))
							mode = 0; 
						break;
					case 3:
						if(atmel_04_touch_csv_parser2(&x, &y, &row, token, limit->product_id))
							mode = 0; 
						break;
					case 4:
						if(atmel_04_touch_csv_parser(&x, &y, &row, token, limit->nodeDetection_MIN))
							mode = 0; 
						break;
					case 5:
						if(atmel_04_touch_csv_parser(&x, &y, &row, token, limit->nodeDetection_MAX))
							mode = 0; 
						break;
					case 6:
						if(atmel_04_touch_csv_parser(&x, &y, &row, token, limit->delta_MIN))
							mode = 0; 
						break;
					case 7:
						if(atmel_04_touch_csv_parser(&x, &y, &row, token, limit->delta_MAX))
							mode = 0; 
						break;
					case 8:
						if(atmel_04_touch_csv_parser(&x, &y, &row, token, limit->micro_defect))
							mode = 0; 
						break;
					case 9:
						if(atmel_04_touch_csv_parser2(&x, &y, &row, token, limit->force_touch_MIN))
							mode = 0; 
						break;
					case 10:
						if(atmel_04_touch_csv_parser2(&x, &y, &row, token, limit->force_touch_MAX))
							mode = 0; 
						break;
				}	
			}
			token = strtok(NULL, TOKEN_SEP_COMMA);
		}
	}

end:
	printf("END \n");
	fclose(fp);
	FUNC_END();
	return 0;
}


int atmel_04_touch_csv_parser(int *xx, int *yy, int *row, char *token, long limit_array[][300])
{
	int i = 0, j =0;
	int n = 0;
	int flag = 0;
	int row_result = 0;
	int x, y;
	x = *xx;
	y = *yy;
	row_result = *row;

	if(!strcmp(token,"S"))
	{
		for(n = 0; n < x+1 ; n++) //1 : x
		{
			token = strtok(NULL, TOKEN_SEP_COMMA);
			if(!strcmp(token,"E"))
			{
				row_result++;
				if(row_result > y-1) //2 :y
				{
					row_result = 0;
					flag = 1;

					if(DEBUG_MODE){
						for(i = 0; i< y; i++)
						{
							for(j = 0; j<x ;j++)
							{
								//printf(" %0.4f",limit_array[i][j]);
								printf(" %ld",limit_array[i][j]);
							}
							printf("\n");
						}
					}

				}
				break;
			}
			limit_array[row_result][n] = strtod(token,NULL);
		}
	}
	*row = row_result;
	return flag;
}

int atmel_04_touch_csv_parser2(int *xx, int *yy, int *row, char *token, char *limit_array)
{
	char test_array[10]={0,};
	char * p = test_array;
	if(!strcmp(token,"S"))
	{
		p = strtok(NULL, TOKEN_SEP_COMMA);
		memcpy(limit_array,p,sizeof(test_array));
	}
	return TRUE;
}

void atmel_04_init_limit_data(struct atmel_04_touch_limit *limit)
{
	l_limit = *limit;
}

int atmel_04_TRANS_RESULT(unsigned char *buf)
{
	int result=0;
	result =((buf[1]<<8)|(buf[0]));
	//printf(">>> SSW <<< [%s %d] %s CALL ====== %X %X %d \n", __FILE__, __LINE__, __FUNCTION__, buf[1], buf[0], result);
	return result;
}

signed short atmel_04_TRANS_RESULT2(unsigned char *buf)
{
	signed short result = 0;
	result =((signed char)buf[1]<<8)|((signed char)buf[0]);
	//printf(">>> SSW <<< [%s %d] %s CALL ====== %X %X %d \n", __FILE__, __LINE__, __FUNCTION__, buf[1], buf[0], result2);
	return result;
}



int node_compare_result_for_dp150(long MIN[300][300],long MAX[300][300], int col, int row, unsigned char* buf, int size)
{
	int i,j,size_count = 0;

	FUNC_BEGIN();

	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++)
		{
			if(size_count >= size)
				goto END;

			unsigned char trans_array[2]={0,0};
			trans_array[0]=(unsigned char) *(buf+size_count);
			trans_array[1]=(unsigned char) *(buf+size_count+1);
			int i2c_result = atmel_04_TRANS_RESULT(trans_array);

			if(DEBUG_MODE){
				if(!((size_count/2)%col))
					printf("\n");
				printf("%05d ",i2c_result);
			}

			//			printf("		>>> SSW <<< [%s %d] %s CALL ====== %04d %05d %04d\n", __FILE__, __LINE__, __FUNCTION__,MIN[i][j], i2c_result, MAX[i][j]);
			//printf("%04d \n", i2c_result);
			size_count +=2;
			if(((MIN[i][j] <= i2c_result) && (i2c_result <= MAX[i][j]))){
				FUNC_END();
			}else{
		//		printf("[%s %d] %s FAIL ====== %d ) %04d < %04d < %04d\n", __FILE__, __LINE__, __FUNCTION__, size_count/2, (int)MIN[i][j], i2c_result, (int)MAX[i][j]);
		//		return FALSE;
			}
		}
	}
END:
	FUNC_END();
	return TRUE;
}


int delta_compare_result_for_dp150(long MIN[300][300],long MAX[300][300], int col, int row, unsigned char* buf, int size)
{
	int i,j,size_count = 0;

	FUNC_BEGIN();

	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++)
		{
			if(size_count >= size)
				goto END;

			unsigned char trans_array[2]={0,0};
			trans_array[0]=(unsigned char) *(buf+size_count);
			trans_array[1]=(unsigned char) *(buf+size_count+1);
			signed short i2c_result = atmel_04_TRANS_RESULT2(trans_array);

			if(DEBUG_MODE){
				if(!((size_count/2)%col))
					printf("\n");
				//printf("[%X,%X] \n",*(buf+size_count+1),*(buf+size_count));
				printf("%04d ",i2c_result);
			}

			//			printf("		>>> SSW <<< [%s %d] %s CALL ====== %04d %05d %04d\n", __FILE__, __LINE__, __FUNCTION__,MIN[i][j], i2c_result, MAX[i][j]);
			//printf("%04d \n", i2c_result);
				size_count +=2;
			if((MIN[i][j] <= i2c_result && i2c_result <= MAX[i][j])){
				FUNC_END();
			}else{
				//printf("[%s %d] %s FAIL ====== %d ) %03d < %03d < %03d\n", __FILE__, __LINE__, __FUNCTION__, size_count/2, MIN[i][j], i2c_result, MAX[i][j]);
			//	printf("[%s %d] %s FAIL ====== %d ) [%X %X] = %03d\n", __FILE__, __LINE__, __FUNCTION__, size_count/2, *(buf+size_count+1), *(buf+size_count+0), i2c_result);
			//	return FALSE;
			}
		}
	}
END:
	FUNC_END();
	return TRUE;
}



int micro_compare_result_for_dp150(long MARGIN[300][300], int col, int row, unsigned char* buf, int size)
{
	int i,j,size_count = 0;

	FUNC_BEGIN();

	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++)
		{
			if(size_count >= size)
				goto END;

			unsigned char trans_array[2]={0,0};
			trans_array[0]=(unsigned char) *(buf+size_count);
			trans_array[1]=(unsigned char) *(buf+size_count+1);
			int present_result = atmel_04_TRANS_RESULT(trans_array);
			int next_result = 0;
			if(size_count >= size-2 || j == (col-1)){
				trans_array[0]=(unsigned char) *(buf+size_count-2);
				trans_array[1]=(unsigned char) *(buf+size_count-1);
				next_result = atmel_04_TRANS_RESULT(trans_array);
			}else{
				trans_array[0]=(unsigned char) *(buf+size_count+2);
				trans_array[1]=(unsigned char) *(buf+size_count+3);
				next_result = atmel_04_TRANS_RESULT(trans_array);
			}

			float margin = 0;
			//margin=(present_result - next_result) / present_result * 100;
			margin=(present_result - next_result); 	
			margin = margin/present_result;
			margin = margin * 100;

			if(DEBUG_MODE){
			//if(1){
				if(!((size_count/2)%col))
					printf("\n");
				printf("%05d ",present_result);
				//printf("%d ",(int) margin);
			}
			
			size_count+=2;
			//	printf("		>>> SSW <<< [%s %d] %s CALL ====== %d %04f %d    %05d %05d \n", __FILE__, __LINE__, __FUNCTION__,-MARGIN[i][j], margin, MARGIN[i][j], present_result, next_result);
			if(((-MARGIN[i][j] <= (int)margin) && ((int)margin <= MARGIN[i][j]))){
				FUNC_END();
			}
			else
			{
				//printf("[%s %d] %s FAIL ====== %d )  %d < %d(%d,%d) < %d\n", __FILE__, __LINE__, __FUNCTION__,size_count/2, (int)-MARGIN[i][j], (int)margin,present_result,next_result,(int)MARGIN[i][j]);
				//return FALSE;
			}
		}
	}
END:
	FUNC_END();
	return TRUE;
}

int node_data_check_for_dp150(unsigned char* buf, int size, long MODE0_MIN[300][300], long MODE0_MAX[300][300])
{
	if(node_compare_result_for_dp150(MODE0_MIN, MODE0_MAX, PT_CH_SIZE_X, PT_CH_SIZE_Y, buf, size)){
		return TRUE;
	}else{
		return FALSE;
	}
	return TRUE;
}


int delta_data_check_for_dp150(unsigned char* buf, int size, long MODE0_MIN[300][300], long MODE0_MAX[300][300])
{
	if(delta_compare_result_for_dp150(MODE0_MIN, MODE0_MAX, PT_CH_SIZE_X, PT_CH_SIZE_Y, buf, size)){
		return TRUE;
	}else{
		return FALSE;
	}
	return TRUE;
}


int micro_data_check_for_dp150(unsigned char* buf, int size, long MODE0_MIN[300][300])//case margin % ,ex = Micro_defect
{
	if(micro_compare_result_for_dp150(MODE0_MIN, PT_CH_SIZE_X, PT_CH_SIZE_Y, buf, size))
		return TRUE;
	else
		return FALSE;
	return TRUE;
}
