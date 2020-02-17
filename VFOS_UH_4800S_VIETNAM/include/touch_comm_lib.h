/////////////////// SYNAPTICS TOUCH COMM PROTOCOL FUNCTIONS //////////////////////////

// STANDARD COMMANDS
#define IDENTIFY		0x02
#define RESET			0x04

// BOOTLOADER COMMANDS (not usable in application firmware mode!)
#define GET_BOOT_INFO	0x10
#define READ_FLASH		0x13
#define RUN_APPLICATION_FIRMWARE	0X14

// APPLICATION FIRMWARE COMMANDS (current mode)
#define ENTER_BOOTLOADER_MODE		0x1f
#define GET_APP_INFO				0x20
#define GET_STATIC_CONFIG			0x21
#define SET_STATIC_CONFIG			0x22
#define GET_DYNAMIC_CONFIG			0x23
#define SET_DYNAMIC_CONFIG			0x24
#define GET_REPORT_CONFIG			0x25
#define SET_REPORT_CONFIG			0x26
#define REZERO						0x27
#define COMMIT_CONFIG				0x28
#define DESCRIBE_DYNAMIC_CONFIG		0x29
#define PRODUCTION_TEST				0x2a
#define SET_CONFIG_ID				0x2b
#define ENABLE_REPORT				0x05
#define DISABLE_REPORT				0x06
#define ENTER_DEEP_SLEEP			0x2c
#define EXIT_DEEP_SLEEP				0x2d
#define GET_TOUCH_INFO				0x2e
#define GET_DATA_LOCATION			0x2f
#define HOST_DOWNLOAD				0x30
#define ENTER_PRODUCTION_TEST_MODE	0x31
#define GET_FEATURES				0x32
#define CALIBRATE					0x33


/////////////////// SYNAPTICS PRODUCTIONID /////////////////////
#define    TEST_TRX_TRX_SHORTS       0x00
#define    TEST_TRX_SENSOR_OPEN      0x01 // discrete TRx Short
#define    TEST_TRX_OPEN             0x02
#define    TEST_TRX_GND_SHORTS       0x03
#define    TEST_GPIO_SHORT           0x04
#define    TEST_FULLRAW              0x05
#define    TEST_DRT                  0x07
#define    TEST_OPEN_SHORT           0x08 // discrete TEST_HIGH_RESISTANCE
#define    TEST_BUMP_SHORT           0x09
#define    TEST_NOISE                0x0A
#define    TEST_PT11                 0x0B
#define    TEST_PT12                 0x0C
#define    TEST_PT13                 0x0D // h
#define    TEST_DRT_DOZE             0x0E
#define    TEST_NOISE_DOZE           0x0F
#define    TEST_SENSORSPEED          0x10
#define    TEST_ADCRANGE             0x11
#define    TEST_ABSRAW               0x12
#define    TEST_13                   0x13
#define    TEST_SYNC_SHORT			 0x14			// 190430 NEW FW	
#define    TEST_RAW_CAP              0x16 // RT3
#define    TEST_TRANS_RX_SHORT       0x19 // RT133
#define    TEST_HYBRID_ABS_WITH_CBC  0x1A // RT252(RT150)
#define    TEST_HYBRID_ABS_NOISE     0x1D // RT155
#define    TEST_BSC_CAL              0x1E
#define    TEST_EXTERNAL_OSC         0x1F
#define    TEST_RT100                0xC4
#define	   TEST_PID21				 0x21

/* Table 24. Possible field IDs */
enum FieldId{	
	FIELD_DISABLE_DOZE = 1,
    FIELD_DISABLE_NOISE_MITIGATION,
    FIELD_DISABLE_FREQ_SHIFTING,
    FIELD_REDUCE_SENSING_FREQ_INDEX,
    FIELD_H_SYNC,
    FIELD_ENABLE_REZERO,
    FIELD_ENABLE_CHARGER_MODE,
    FIELD_DISABLE_BASELINE_RELAXATION,
    FIELD_ENABLE_WAKEUP_GESTURE,
    FIELD_NUM_OF_FAKE_FINGERS,
    FIELD_ENABLE_GRIP_SUPPRESSION,
    FIELD_ENABLE_THICK_GLOVE,
    FIELD_ENABLE_GLOVE,
    FIELD_INT2 = 0xD2, //for int2 test
    FIELD_NONE = 0xFF,
};
