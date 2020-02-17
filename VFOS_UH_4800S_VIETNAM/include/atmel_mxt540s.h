#ifndef	__ATMEL_MXT540S_H__
#define	__ATMEL_MXT540S_H_

#define MXT_540S_SW_RESET_TIME      300 /* msec */
#define MXT_540S_HW_RESET_TIME      300 /* msec */
#define MXT_540S_FW_RESET_TIME      500 /* msec */

#define OBJECT_TABLE_ELEMENT_SIZE   6
#define OBJECT_TABLE_START_ADDRESS  7

#define CMD_RESET_OFFSET        0
#define CMD_BACKUP_OFFSET       1
#define CMD_CALIBRATE_OFFSET        2
#define CMD_REPORTATLL_OFFSET       3
#define CMD_DEBUG_CTRL_OFFSET       4
#define CMD_DIAGNOSTIC_OFFSET       5

#define MXT_T7_IDLE_ACQ_INT 0
#define MXT_T7_ACT_ACQ_INT  1

#define MXT_T9_CTRL 		0
#define	MXT_T9_CFG1			1
#define	MXT_T9_SCRAUX		2
#define	TCHAUX				3
#define	TCHEVENTCFG			4
#define	AKSCFG				5
#define	NUMTCH				6
#define	XYCFG				7
#define	XORIGIN				8
#define	XSIZE				9
#define	XPITCH				10
#define	XLOCLIP				11
#define	XHICLIP				12
#define MXT_T9_XRANGE_LSB   13
#define MXT_T9_XRANGE_MSB   14
#define	XEDGECFG			15
#define	XEDGEDIST			16
#define	DXXEDGECFG			17
#define	DXXEDGEDIST			18
#define	YORIGIN				19
#define	YSIZE				20
#define	YPITCH				21
#define	YLOCLIP				22
#define	YHICLIP				23
#define MXT_T9_YRANGE_LSB   24
#define MXT_T9_YRANGE_MSB   25
#define	YEDGECFG			26
#define	YEDGEDIST			27
#define	GAIN				28
#define	DXGAIN				29
#define	TCHTHR				30
#define	TCHHYST				31
#define	INTTHR				32
#define	NOISESF				33
#define	CUTOFFTHR			34
#define	MRGTHR				35
#define	MRGTHRADJSTR		36
#define	MRGHYST				37
#define	DXTHRSF				38
#define	TCHDIDOWN			39
#define	TCHDIUP				40
#define	NEXTTCHDI			41
#define	RESERVED_0			42
#define	JUMPLIMIT			43
#define	MOVFILTER			44
#define	MOVSMOOTH			45
#define	MOVPRED				46
#define	MOVHYSTI_LSB		47
#define	MOVHYSTI_MSB		48
#define	MOVHYSTN_LSB		49
#define	MOVHYSTN_MSB		50
#define	AMPLHYST			51
#define	SCRAREAHYST			52
#define	INTTHRHYST			53

#define DETECT_MSG_MASK     0x80
#define PRESS_MSG_MASK      0x40
#define RELEASE_MSG_MASK    0x20
#define MOVE_MSG_MASK       0x10
#define VECTOR_MSG_MASK     0x08
#define AMPLITUDE_MSG_MASK  0x04
#define SUPPRESS_MSG_MASK   0x02

/* Slave addresses */
#define MXT_APP_LOW     0x4a
#define MXT_APP_HIGH        0x4b
#define MXT_BOOT_LOW        0x26
#define MXT_BOOT_HIGH       0x27

#define MXT_BOOT_VALUE      0xa5
#define MXT_BACKUP_VALUE    0x55

/* Bootloader mode status */
#define MXT_WAITING_BOOTLOAD_CMD    0xc0
#define MXT_WAITING_FRAME_DATA      0x80
#define MXT_FRAME_CRC_CHECK     0x02
#define MXT_FRAME_CRC_FAIL      0x03
#define MXT_FRAME_CRC_PASS      0x04
#define MXT_APP_CRC_FAIL        0x40
#define MXT_BOOT_STATUS_MASK        0x3f

/* Bootloader ID */
#define MXT_BOOT_EXTENDED_ID        0x20
#define MXT_BOOT_ID_MASK        0x1f

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB      0xaa
#define MXT_UNLOCK_CMD_LSB      0xdc

#define ID_BLOCK_SIZE           7

#define MXT_STATE_INACTIVE      -1
#define MXT_STATE_RELEASE       0
#define MXT_STATE_PRESS         1
#define MXT_STATE_MOVE          2

/* Diagnostic cmds  */
#define MXT_DIAG_PAGE_UP        0x01
#define MXT_DIAG_PAGE_DOWN      0x02
#define MXT_DIAG_DELTA_MODE     0x10
#define MXT_DIAG_REFERENCE_MODE     0x11
#define MXT_DIAG_CTE_MODE       0x31
#define MXT_DIAG_IDENTIFICATION_MODE    0x80
#define MXT_DIAG_TOCH_THRESHOLD_MODE    0xF4

#define MXT_T6_CMD_DELTAS           0x10
#define MXT_T6_CMD_REFS           	0x11 
#define MXT_T6_CMD_SELF_DELTAS    	0xF7 
#define MXT_T6_CMD_SELF_REFS     	0xF8 

#define MXT_DIAG_MODE_MASK  0xFC
#define MXT_DIAGNOSTIC_MODE 0
#define MXT_DIAGNOSTIC_PAGE 1

/* Firmware name */
#define MXT_FW_NAME     "mXT540S.fw"
#define MXT_MAX_FW_PATH     255
#define MXT_CONFIG_VERSION_LENGTH   30

/* Firmware version */
#define MXT_FIRM_VERSION    0x10
#define MXT_FIRM_BUILD      0xAA

/* Touchscreen configuration infomation */
#define MXT_FW_MAGIC        0x4D3C2B1A
#define DUAL_CFG    1

/* Feature */
#define TSP_FIRMUP_ON_PROBE 1
#define TSP_BOOSTER     0
#define TSP_DEBUG_INFO  1
#define TSP_SEC_SYSFS   1
#define CHECK_ANTITOUCH     1
#if DUAL_CFG
#define TSP_INFORM_CHARGER  1
#else
#define TSP_INFORM_CHARGER  0
#endif
/* TSP_ITDEV feature just for atmel tunning app
* so it should be disabled after finishing tunning
* because it use other write permission. it will be cause
* failure of CTS
*/
#define TSP_ITDEV       1
#define TSP_USE_SHAPETOUCH  0

#if CHECK_ANTITOUCH
#define MXT_T61_TIMER_ONESHOT   0
#define MXT_T61_TIMER_REPEAT    1
#define MXT_T61_TIMER_CMD_START     1
#define MXT_T61_TIMER_CMD_STOP      2
#endif

#if TSP_SEC_SYSFS
#define TSP_BUF_SIZE     1024

#define NODE_NUM    540
#define NODE_PER_PAGE   64
#define DATA_PER_NODE   2

#define REF_MIN_VALUE   19744
#define REF_MAX_VALUE   28884

#define TSP_CMD_STR_LEN     32
#define TSP_CMD_RESULT_STR_LEN  512
#define TSP_CMD_PARAM_NUM   8
#endif

enum { RESERVED_T0 = 0,
    RESERVED_T1,
    DEBUG_DELTAS_T2,
    DEBUG_REFERENCES_T3,
    DEBUG_SIGNALS_T4,
    GEN_MESSAGEPROCESSOR_T5,
    GEN_COMMANDPROCESSOR_T6,
    GEN_POWERCONFIG_T7,
    GEN_ACQUISITIONCONFIG_T8,
    TOUCH_MULTITOUCHSCREEN_T9_OLD,
    TOUCH_SINGLETOUCHSCREEN_T10, //10
    TOUCH_XSLIDER_T11,
    TOUCH_YSLIDER_T12,
    TOUCH_XWHEEL_T13,
    TOUCH_YWHEEL_T14,
    TOUCH_KEYARRAY_T15,
    PROCG_SIGNALFILTER_T16,
    PROCI_LINEARIZATIONTABLE_T17,
    SPT_COMCONFIG_T18,
    SPT_GPIOPWM_T19,
    PROCI_GRIPFACESUPPRESSION_T20, //20
    RESERVED_T21,
    PROCG_NOISESUPPRESSION_T22,
    TOUCH_PROXIMITY_T23,
    PROCI_ONETOUCHGESTUREPROCESSOR_T24,
    SPT_SELFTEST_T25, 
    DEBUG_CTERANGE_T26,
    PROCI_TWOTOUCHGESTUREPROCESSOR_T27,
    SPT_CTECONFIG_T28,
    SPT_GPI_T29,
    SPT_GATE_T30,	//30
    TOUCH_KEYSET_T31,
    TOUCH_XSLIDERSET_T32,
    RESERVED_T33,
    GEN_MESSAGEBLOCK_T34,
    SPT_GENERICDATA_T35,
    RESERVED_T36,
    DEBUG_DIAGNOSTIC_T37,
    SPT_USERDATA_T38,
    SPARE_T39,
    PROCI_GRIPSUPPRESSION_T40, //40
    SPARE_T41,
    PROCI_TOUCHSUPPRESSION_T42,
    SPT_DIGITIZER_T43,
    SPARE_T44,
    SPARE_T45,
    SPT_CTECONFIG_T46,
    PROCI_STYLUS_T47,	
    PROCG_NOISESUPPRESSION_T48,
    SPARE_T49,
    SPARE_T50,	//50
    SPARE_T51,
    TOUCH_PROXIMITY_KEY_T52,
    GEN_DATASOURCE_T53,
    PROCG_NOISESUPPRESSION_T54,
    ADAPTIVE_T55,
    PROCI_SHIELDLESS_T56,
    PROCI_EXTRATOUCHSCREENDATA_T57,
    SPARE_T58,
    SPARE_T59,
    SPARE_T60, //60
    SPT_TIMER_T61,
    PROCG_NOISESUPPRESSION_T62,
    PROCI_ACTIVESTYLUS_T63,
	TOUCH_MULTITOUCHSCREEN_T9 = 100,
    RESERVED_T255 = 255,
};

#endif	/* __ATMEL_MXT540S_H_ */

