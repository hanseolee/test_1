
/******************************************************************************
* @file    fw_download_ref_for_sw42000.h
* @version $Staste:$
* @date    
* @author
* @brief   SW42000 firmware download reference code
******************************************************************************/



typedef int LXRESULT;
typedef signed char SInt8;
typedef signed short SInt16;
typedef signed int SInt32;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;


typedef union
{
	struct {
		unsigned    minor_ver : 8;
		unsigned    major_ver : 4;
		unsigned    build_ver : 4;
		unsigned    chip_id : 8;
		unsigned    protocol_ver : 4;
		unsigned    reserved : 4;
	} b;
	UInt32 w;
} t_ic_info;

/*FW Version*/
//it depands on FW Version.
#define FW_MAJOR_VER                    0X07 
#define FW_MINOR_VER                    0x03


 /* Reset Control*/
#define SYS_RST_CTL						0x04 
#define CM3_RST                         0x2
#define CM3_RST_RELEASED                0


/* FW Download Parameters */
#define FW_BOOT_LOADER_INIT				(0x74696E69)
#define FW_BOOT_LOADER_CODE				(0x544F4F42)
#define CRC_FIXED_VALUE					0x800D800D

#define SIZEOF_FLASH					(128*1024)

#define MAX_FW_SIZE						(128*1024)
#define BDMA_TRANS_SIZE					(0x8000) // 32kB

/* registers */
#define SYS_OSC_CTL						0x05
#define FW_BOOT_CODE_ADDR				0x1A 
#define FLASH_TPROG						0x65

#define FC_FLASH_PAGE_SIZE				(2048)	
#define FC_ADDR							0x6D 	
#define FC_STAT							0x6C	
#define FC_CTRL							0x6B	
#define FC_CTRL_PAGE_ERASE				0x5		
#define FC_CTRL_MASS_ERASE				0x6		
#define SPI_FLASH_STATUS				0xFE2	

#define DATA_I2CBASE_ADDR				0x2FD1
#define DATA_I2CBASE_ADDR_FW			0x6FD1
#define SERIAL_DATA_OFFSET				0x003C
#define SAVE_ADDR       				0x0C04
#define OFFSET_ADDR_SIZE				4
#define DATASRAM_ADDR					(0x20000000)

// BDMA Defines & Headers
#define BDMA_SADDR						0x72
#define BDMA_DADDR						0x73
#define BDMA_CTL						0x74
#define BDMA_START						0x75
#define BDMA_STS						0x77

#define BDMA_CTL_BDMA_EN				(0x00010000)
#define BDMA_CTL_BDMA_BST_EN			(0x001E0000)
#define BDMA_STS_TR_BUSY				(0x00000040)

// GDMA Defines & Headers
#define GDMA_CTL						0x58 
#define GDMA_CTL_READONLY_EN			(0x1 << 17) 
#define GDMA_CTL_GDMA_EN	        	(0x1 << 26) 
#define GDMA_START						0x59 
#define GDMA_CRC_RESULT					0x5D 
#define GDMA_CRC_PASS					0x5E 
#define GDMA_SADDR                      0x56 

// PT Control

#define PT_ADDR_SPI_TATTN_OPT			0xFF3
#define SYS_ATTN_OUT					0x02

#define IC_INFO							0x642
#define CHIP_ID_ADDR_47600				0X00
#define CHIP_ID_47600					0x37363030
#define CHIP_ID_ADDR_47400				0x100
#define CHIP_ID_47400					0x00047400

#define PT_ADDR_PT_TEST_STS				0x690
#define PT_ADDR_TUNE_CODE				0x6A6
#define	PT_ADDR_TC_PT_TEST_CTL			0xC04

#define PT_DISPLAY_STATUS_U3 			0x300
#define PT_DISPLAY_STATUS_U2 			0x200
#define PT_DISPLAY_STATUS_U0 			0x000

#define PT_CH_SIZE_X					34
#define PT_CH_SIZE_y					15

#define PT_CMD_OPEN 					1
#define PT_CMD_SHORT 					2
#define PT_CMD_M2_RAW 					5
#define PT_CMD_M2_JITTER				13
#define PT_CMD_M1_RAW                   3


//Check Flash Info area
#define PT_CMD_FLASH_INFO_CHECK			0x3C
//#define PT_CMD_FLASH_INFO_CHECK			60
#define PT_CMD_PROD_INFO1_READ			0x33
#define FLASH_INFO_CHECK_RESULT_ADDR	0x6A5			
#define	FLASH_INFO_PASS				    0x31


#define TRY_CNT                         5

#define PIN_HIGH                        1
#define PIN_LOW                         0

#define FALSE                           0
#define TRUE                            1

#define _IOCTL_CH1_TE_GET               0x1004 
#define _IOCTL_CH2_TE_GET               0x1005

#define SIW_TOUCH_LOG					"/Data/siw_touch_log.txt"
#define LGD_PRODUCT_ID					"L0W61A"

//int touch_limit_table_parser(MODEL id, char *m_name, struct synaptics_touch_limit* limit);


int init_i2c_set_slvAddr_channel(int ch, int slvAddr);
int connection_check(int ch_num);
int ic_info_check(void);
int open_test(void);
int short_test(void);
 int M2_Raw_Mutual_test(void);
 int M2_Raw_Self_test();  
	int M2_Jitter_test(void);   
	int M1_Raw_test(void);
