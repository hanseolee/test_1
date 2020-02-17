#ifndef	__SYNAPTICS_TOUCH_H__
#define	__SYNAPTICS_TOUCH_H__

#define TOKEN_SEP_COMMA  ", \t\r\n"
#define TOKEN_SEP  " \t\r\n"


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

#include <stdbool.h>

/* number define for Synaptics touch test, test error codes are different between STORM and DP076 */
enum{
    TOUCH_SNT_I2C_CHECK = 0,
    TOUCH_SNT_ATTENTION,
	TOUCH_SNT_FW_VERSION,
    TOUCH_SNT_CONFIG,
	TOUCH_SNT_DEVICE_PACKAGE,
    TOUCH_SNT_FULL_RAW_CAP,
    TOUCH_SNT_NOISE,
    TOUCH_SNT_HYBRID_RAW_CAP_TX,
    TOUCH_SNT_HYBRID_RAW_CAP_RX,
    TOUCH_SNT_EXTENDED_TRX_SHORT,
	TOUCH_SNT_SCAN_PDT,
    TOUCH_SNT_RESET_PIN,
    TOUCH_SNT_PROJECT_ID,
    TOUCH_SNT_EXTENDED_HIGH_RESISTANCE,
    TOUCH_SNT_LOCKDOWN_CHECK,
    TOUCH_SNT_SIDE_TOUCH_RAW_CAP,
    TOUCH_SNT_SIDE_TOUCH_NOISE,
    TOUCH_SNT_OTP_WRITE,
    TOUCH_SNT_OTP_COUNT_CHECK,
	TOUCH_SNT_PA2_GND,
};

//////////////////////GLOBAL

typedef unsigned int    uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char   uint8_t;
typedef signed int      int32_t;
typedef signed short    int16_t;
typedef signed char     int8_t;

#define CHIP_T1324  0
#define CHIP_T1326  1
#define CHIP_T1321  2
#define CHIP_TH2411  3
#define CHIP_STOCKHOLM 4
#define CHIP_PROTON 5
////////////////////////////////

#define CHIP_SELECT CHIP_TH2411
#if CHIP_SELECT == CHIP_TH2411
#define TRX_MAX             34
#define TRX_MAPPING_MAX     64
#define TRX_BITMAP_LENGTH   8
#elif CHIP_SELECT == CHIP_T1324
#define TRX_MAX             32
#define TRX_MAPPING_MAX     54
#define TRX_BITMAP_LENGTH   7
#elif CHIP_SELECT == CHIP_T1326
#define TRX_MAX             64  
#define TRX_MAPPING_MAX     100
#define TRX_BITMAP_LENGTH   13
#elif CHIP_SELECT == CHIP_T1321
#define TRX_MAX             28
#define  TRX_MAPPING_MAX    54
#define TRX_BITMAP_LENGTH   4
#elif CHIP_SELECT == CHIP_STOCKHOLM
#define TRX_MAX             72
#define TRX_MAPPING_MAX     118
#define TRX_BITMAP_LENGTH   15
#elif CHIP_SELECT == CHIP_PROTON
#define TRX_MAX             28
#define TRX_MAPPING_MAX     42
#define TRX_BITMAP_LENGTH   6
#endif

#define FORCE_COUNT TRX_MAX

#define COMMUNICATION_DEFINE    1
#define ADDRESSING_MODE       RMI_ADDRESS_8_BIT
#define LOAD_LIBRARY          LIBRARY_CDCIAPI
#define PROTOCOL              PROTOCOL_I2C
#define RESUME_TIMEOUT        1000 /* millisecond */

///////////FOR LIST

#define TIMEOUT_ENABLE 5000
#define TIMEOUT_READ 1000
#define TIMEOUT_WRITE 1000
#define TIMEOUT_ERASE 3000
#define TIMEOUT_FINALIZE 5000

#define     LIST_MAX    100
//////////////////
#define RMI_PAGE_SELECT_REGISTER 0xff
//////////////////////////////////////////

typedef enum FunctionInterrupt
{
  eF34 = 0x01,
  eF01 = 0x02,
  eF11 = 0x04,
  eF54 = 0x08
}FunctionInterrupt;
////////////////////////////////////////

struct attn_context
{
  uint32_t _handle;
  bool attn;
  uint16_t addr;
};

/////////////////////////////////////////

typedef enum PartitionID
{
    Reserved        = 0,
    Bootloader      = 1,
    DeviceConfig    = 2,
    FlashConfig     = 3,
    ManufacturingBlock      = 4,
    CustomerSerialization   = 5,
    GlobalParameters        = 6,
    CoreCode        = 7,
    CoreConfig      = 8,
    GuestCode       = 9,
    DisplayConfig   = 10,
    AFE             = 11,
    PartitionCount
}PartitionID;

typedef enum BL7Command
{
    m_Idle = 0x00,
    m_EnterBootloader = 0x01,
    m_Read = 0x02,
    m_Write = 0x03,
    m_Erase = 0x04,
    m_EraseApplication = 0x05,
    m_SensorID = 0x06,
}BL7Command;

typedef struct PartitionInfo
{
    PartitionID ID;
    unsigned short Length; //block
    unsigned short StartPhysicalBlockAddress;
    unsigned short Properties;
}PartitionInfo;

////////////////////


typedef struct PartitionTask
{
    PartitionID ID;
    unsigned short BlockOffset;
    unsigned short TransferLen;
    BL7Command command;
    unsigned char Payload[2];
    int size; //bytes
    unsigned char *data;
}PartitionTask;


typedef struct SFunctionDescriptor
{
  // note that the address is 16 bit
  uint16_t QueryBase;
  uint16_t CommandBase;
  uint16_t ControlBase;
  uint16_t DataBase;
  uint8_t Version;
  uint8_t InterruptSourceCount;
  uint8_t ID;

}SFunctionDescriptor;

#if 0 //temp
const char OperationStatus[11][35] =
{
    "Success",
    "Device Not In Bootloader Mode",//0x01
    "Invalid Partition",
    "Invalid Command",
    "Invalid Block Offset",
    "Invalid Transfer",
    "Not Erased",
    "Flash Programming Key Incorrect",
    "Bad Partition Table",
    "Checksum Failed",
    "Flash Hardware Failure"
};

const char Dev_ConfigStatus[4][20] =
{
    "Default",
    "Temporary",
    "Fixed",
    "Reserved"
};

const char BL7_CommandString[7][20] =
{
    "Idle",
    "Enter Bootloader",
    "Read",
    "Write",
    "Erase",
    "Erase Application",
    "Sensor ID",
};
const char PartitionName[PartitionCount][30] =
{
    "Reserved",
    "Bootloader",
    "Device Config",
    "Flash Config",
    "Manufacturing Block",
    "Customer Serialization",
    "Global Parameters",
    "Core Code",
    "Core Config",
    "Guest Code",
    "Display Config",
    "External Touch AFE"
};
#endif
///////////////////////////////////////////////////

typedef enum EReportType_s
{
  eRT_DataCollect = 99,
  eRT_ExtendedHighResistance = 98,
  eRT_Configuration = 97,
  eRT_Attention = 96,
  eRT_Package = 95,
  eRT_ExternalReset = 94,
  eRT_Normalized16BitImageReport = 2,
  eRT_RawImageRT3 = 3, //Raw 16-Bit Image Report
  eRT_ExtendCap = 31,
  eRT_FullRawCBC_on = 201,
  eRT_FullRawCBC_off = 202,
  eRT_FullRawCapacitance = 20,   // Full Raw Capacitance with Receiver Offset Removed
  eRT_ADCRange = 23,
  eRT_SensorSpeed = 22,
  eRT_TRexOpen = 24, // no sensor
  eRT_TRexGround = 25, // no sensor
  eRT_TRexShort = 26, // no sensor
  eRT_ExtendedTRexShort = 261,
  eRT_ExtendedTRexShortRT100 = 262,
  eRT_ExtendedTRexShort_3508 = 263,
  eRT_HighResistance = 4,
  eRT_FullRawCapacitanceMaxMin = 13,
  eRT_AbsADCRange = 42,
  eRT_AbsDelta = 40,
  eRT_AbsRaw = 38,//38,
  eRT_AbsOpenShort = 61,
  eRT_AbsOpen = 611,
  eRT_AbsShort = 612,
  eRT_GpioShortTest = 27,
  eRT_GpioOpenTest = 28,
  eRT_RxRxShort = 7,
  eRT_RxRxShort1 = 17,
  eRT_RxRxShort2 = 33,
  eRT_TxTxShort = 5,
  eRT_TxGndShort = 16,
  eRT_RxOpen = 14,
  eRT_RxOpen1 = 18,
  eRT_RxOpen2 = 35,
  eRT_TxOpen = 15,
  eRT_Bonding_Pad_Active_Guard_Opens = 48,
  eRT_GuardPinShort = 50,
  eRT_RawImageRT100 = 100,
  eRT_HybridAbsDeltaCap = 59,
  eRT_HybridSensingRawCap = 63,
  eRT_ExtendHybridTXShort = 631,
  eRT_ExtendHybridTXShort2 = 632,
  eRT_ForceOpen = 633,
  eRT_TagsMoistureCBC_on = 761,
  eRT_TagsMoistureCBC_off = 762,
  eRT_TagsMoisture = 76,
  eRT_CrefCapacitor = 57,
  eRT_FOpen = 106,
  eRT_CID_IM = 93,
  eRT_HICFullRaw = 80,
  eRT_HICNoise = 81,
  eRT_HICTxShort = 126,
  eRT_HICRxShort = 125,
  eRT_HICHybridSensingRawCap = 49,
  eRT_PixelRawCap = 47,
  eRT_HybridAbsWOCBC = 120,
  eRT_IncellAbsDozeWOCBC = 121,
  eRT_LPWG_RxFingerAbsCapWOCBC = 122,
  eRT_AbsGestureLPWG_WOCBC = 123,
  eRT_AmpAbsSensingRawCap = 92,
  eRT_RT130 = 130,
  eRT_RT131 = 131,
  eRT_CBC = 53,
  eRT_TransRxShortUsingCBCVariation = 133,
  eRT_HybridABSwithCBC   = 150,
  eRT_RT127 = 127,
  eRT_RT128 = 128,
  eRT_PA2_GND = 62,
  eRT_SyncShorts = 85,
}EReportType;


#define REPORT_DATA_OFFSET	3
#define DO_TEST_PREPARATION 1
#define EXTENDED_TRX_SHORT_CBC 0x0f
#define MEIZU_TX 25
#define MEIZU_RX 32

//EReportType e_inputRT;

////////////////////////////////////////////////////
typedef struct F34_Query
{
  unsigned char FlashKey[2];
  unsigned int BootloaderFWID;
  unsigned char MWS; // Minium Wirte Size
  unsigned short BlockSize;
  unsigned short FlashPageSize;
  unsigned short AdjPartitionSize;
  unsigned short FlashConfLen;
  unsigned short PayloadLen;
  unsigned char PartitionSupport[4];
  unsigned int PartitionCount;

}F34_Query;

typedef struct PDT {
  SFunctionDescriptor g_F01Descriptor;
  SFunctionDescriptor g_F11Descriptor;
  SFunctionDescriptor g_F12Descriptor;
  SFunctionDescriptor g_F1ADescriptor;
  SFunctionDescriptor g_F21Descriptor;
  SFunctionDescriptor g_F34Descriptor;
  SFunctionDescriptor g_F54Descriptor;
  SFunctionDescriptor g_F55Descriptor;

  bool bHaveF12Ctrl20;
  bool bHaveF12Ctrl27;
  bool bBoodloaderVersion6; // true:v6: , false: v5.1 and below
  bool bSignalClarityOn;
  bool bHaveF54Ctrl07;
  bool bHaveF54Ctrl57;
  bool bHavePixelTouchThresholdTuning;
  bool bHaveInterferenceMetric;
  bool bHaveCtrl11;
  bool bHaveRelaxationControl;
  bool bHaveSensorAssignment;
  bool bHaveSenseFrequencyControl;
  bool bHaveFirmwareNoiseMitigation;
  bool bHaveIIRFilter;
  bool bHaveCmnRemoval;
  bool bHaveCmnMaximum;
  bool bHaveTouchHysteresis;
  bool bHaveEdgeCompensation;
  bool bHavePerFrequencyNoiseControl;
  bool bHaveSignalClarity;
  bool bHaveMultiMetricStateMachine;
  bool bHaveVarianceMetric;
  bool bHave0DRelaxationControl;
  bool bHave0DAcquisitionControl;
  bool bHaveSlewMetric;
  bool bHaveHBlank;
  bool bHaveVBlank;
  bool bHaveLongHBlank;
  bool bHaveNoiseMitigation2;
  bool bHaveSlewOption;
  bool bHaveEnhancedStretch;
  bool bHaveStartupFastRelaxation;
  bool bHaveESDControl;
  bool bHaveEnergyRatioRelaxation;
  bool bHaveF54CIDIM;
  bool bHasInCellSideTouchBottom;
  bool bHasInCellSideTouchTop;
  bool bHasInCellSideTouchRight;
  bool bHasInCellSideTouchLeft;

  bool bHaveF54Data00;
  bool bHaveF54Data01;
  bool bHaveF54Data02;
  bool bHaveF54Data03;
  bool bHaveF54Data04;
  bool bHaveF54Data05;
  bool bHaveF54Data0601;
  bool bHaveF54Data0602;
  bool bHaveF54Data0701;
  bool bHaveF54Data0702;
  bool bHaveF54Data0801;
  bool bHaveF54Data0802;
  bool bHaveF54Data0901;
  bool bHaveF54Data0902;
  bool bHaveF54Data10;
  bool bHaveF54Data11;
  bool bHaveF54Data12;
  bool bHaveF54Data13;
  bool bHaveF54Data14;
  bool bHaveF54Data15;
  bool bHaveF54Data16;
  bool bHaveF54Data17;
  bool bHaveF54Data18;
  bool bHaveF54Data19;
  bool bHaveF54Data20;
  bool bHaveF54Data21;
  bool bHaveF54Data22;
  bool bHaveF54Data23;
  bool bHaveF54Data24;
  bool bHaveF54Data25;
  bool bHaveF54Data26;
  bool bHaveF54Data27;
  bool bHaveF54Data28;
  bool bHaveF54Data29;
  bool bHaveF54Data30;
  bool bHaveF54Data31;
  bool bHaveF54Ctrl86;
  bool bHaveF54Ctrl87;
  bool bHaveF54Ctrl88;
  bool bHaveF54Ctrl89;
  bool bHaveF54Ctrl90;
  bool bHaveF54Ctrl91;
  bool bHaveF54Ctrl92;
  bool bHaveF54Ctrl93;
  bool bHaveF54Ctrl94;
  bool bHaveF54Ctrl95;
  bool bHaveF54Ctrl96;
  bool bHaveF54Ctrl97;
  bool bHaveF54Ctrl98;
  bool bHaveF54Ctrl99;
  bool bHaveF54Ctrl100;
  bool bHaveF54Ctrl101;
  bool bHaveF54Ctrl102;
  bool bHaveF54Ctrl103;
  bool bHaveF54Ctrl104;
  bool bHaveF54Ctrl105;
  bool bHaveF54Ctrl106;
  bool bHaveF54Ctrl107;
  bool bHaveF54Ctrl108;
  bool bHaveF54Ctrl109;
  bool bHaveF54Ctrl110;
  bool bHaveF54Ctrl111;
  bool bHaveF54Ctrl112;
  bool bHaveF54Ctrl113;
  bool bHaveF54Ctrl114;
  bool bHaveF54Ctrl115;
  bool bHaveF54Ctrl116;
  bool bHaveF54Ctrl117;
  bool bHaveF54Ctrl118;
  bool bHaveF54Ctrl119;
  bool bHaveF54Ctrl120;
  bool bHaveF54Ctrl121;
  bool bHaveF54Ctrl122;
  bool bHaveF54Ctrl123;
  bool bHaveF54Ctrl124;
  bool bHaveF54Ctrl125;
  bool bHaveF54Ctrl126;
  bool bHaveF54Ctrl127;
  bool bHaveF54Ctrl128;
  bool bHaveF54Ctrl129;
  bool bHaveF54Ctrl130;
  bool bHaveF54Ctrl131;
  bool bHaveF54Ctrl132;
  bool bHaveF54Ctrl133;
  bool bHaveF54Ctrl134;
  bool bHaveF54Ctrl135;
  bool bHaveF54Ctrl136;
  bool bHaveF54Ctrl137;
  bool bHaveF54Ctrl138;
  bool bHaveF54Ctrl139;
  bool bHaveF54Ctrl140;
  bool bHaveF54Ctrl141;
  bool bHaveF54Ctrl142;
  bool bHaveF54Ctrl143;
  bool bHaveF54Ctrl144;
  bool bHaveF54Ctrl145;
  bool bHaveF54Ctrl146;
  bool bHaveF54Ctrl147;
  bool bHaveF54Ctrl148;
  bool bHaveF54Ctrl149;
  bool bHaveF54Ctrl150;
  bool bHaveF54Ctrl151;
  bool bHaveF54Ctrl152;
  bool bHaveF54Ctrl153;
  bool bHaveF54Ctrl154;
  bool bHaveF54Ctrl155;
  bool bHaveF54Ctrl156;
  bool bHaveF54Ctrl157;
  bool bHaveF54Ctrl158;
  bool bHaveF54Ctrl159;
  bool bHaveF54Ctrl160;
  bool bHaveF54Ctrl161;
  bool bHaveF54Ctrl162;
  bool bHaveF54Ctrl163;
  bool bHaveF54Ctrl164;
  bool bHaveF54Ctrl165;
  bool bHaveF54Ctrl166;
  bool bHaveF54Ctrl167;
  bool bHaveF54Ctrl168;
  bool bHaveF54Ctrl169;
  bool bHaveF54Ctrl170;
  bool bHaveF54Ctrl171;
  bool bHaveF54Ctrl172;
  bool bHaveF54Ctrl173;
  bool bHaveF54Ctrl174;
  bool bHaveF54Ctrl175;
  bool bHaveF54Ctrl176;
  bool bHaveF54Ctrl177;
  bool bHaveF54Ctrl178;
  bool bHaveF54Ctrl179;
  bool bHaveF54Ctrl180;
  bool bHaveF54Ctrl181;
  bool bHaveF54Ctrl182;
  bool bHaveF54Ctrl183;
  bool bHaveF54Ctrl184;
  bool bHaveF54Ctrl185;
  bool bHaveF54Ctrl186;
  bool bHaveF54Ctrl187;
  bool bHaveF54Ctrl188;
  bool bHaveF54Ctrl189;
  bool bHaveF54Ctrl190;
  bool bHaveF54Ctrl191;
  bool bHaveF54Ctrl192;
  bool bHaveF54Ctrl193;
  bool bHaveF54Ctrl194;
  bool bHaveF54Ctrl195;
  bool bHaveF54Ctrl196;
  bool bHaveF54Ctrl197;
  bool bHaveF54Ctrl198;
  bool bHaveF54Ctrl199;
  bool bHaveF54Ctrl200;
  bool bHaveF54Ctrl201;
  bool bHaveF54Ctrl202;
  bool bHaveF54Ctrl203;
  bool bHaveF54Ctrl204;
  bool bHaveF54Ctrl205;
  bool bHaveF54Ctrl206;
  bool bHaveF54Ctrl207;
  bool bHaveF54Ctrl208;
  bool bHaveF54Ctrl209;
  bool bHaveF54Ctrl210;
  bool bHaveF54Ctrl211;
  bool bHaveF54Ctrl212;
  bool bHaveF54Ctrl213;
  bool bHaveF54Ctrl214;
  bool bHaveF54Ctrl215;
  bool bHaveF54Ctrl216;
  bool bHaveF54Ctrl217;
  bool bHaveF54Ctrl218;
  bool bHaveF54Ctrl219;
  bool bHaveF54Ctrl220;
  bool bHaveF54Ctrl221;
  bool bHaveF54Ctrl222;
  bool bHaveF54Ctrl223;
  bool bHaveF54Ctrl224;
  bool bHaveF54Ctrl225;
  bool bHaveF54Ctrl226;
  bool bHaveF54Ctrl227;
  bool bHaveF54Ctrl228;
  bool bHaveF54Ctrl229;
  bool bHaveF54Ctrl230;
  bool bHaveF54Ctrl231;
  bool bHaveF54Ctrl232;
  bool bHaveF54Ctrl233;
  bool bHaveF54Ctrl234;
  bool bHaveF54Ctrl235;
  bool bHaveF54Ctrl236;
  bool bHaveF54Ctrl237;
  bool bHaveF54Ctrl238;
  bool bHaveF54Ctrl239;
  bool bHaveF54Ctrl240;
  bool bHaveF54Ctrl241;
  bool bHaveF54Ctrl242;
  bool bHaveF54Ctrl243;
  bool bHaveF54Ctrl244;
  bool bHaveF54Ctrl245;
  bool bHaveF54Ctrl246;

  bool bIsTDDIHIC;
  bool bHaveF54Query13;
  bool bHaveF54Query15;
  bool bHaveF54Query16;
  bool bHaveF54Query17;
  bool bHaveF54Query18;
  bool bHaveF54Query19;
  bool bHaveF54Query20;
  bool bHaveF54Query21;
  bool bHaveF54Query22;
  bool bHaveF54Query23;
  bool bHaveF54Query24;
  bool bHaveF54Query25;
  bool bHaveF54Query26;
  bool bHaveF54Query27;
  bool bHaveF54Query28;
  bool bHaveF54Query29;
  bool bHaveF54Query30;
  bool bHaveF54Query31;
  bool bHaveF54Query32;
  bool bHaveF54Query33;
  bool bHaveF54Query34;
  bool bHaveF54Query35;
  bool bHaveF54Query36;
  bool bHaveF54Query37;
  bool bHaveF54Query38;
  bool bHaveF54Query39;
  bool bHaveF54Query40;
  bool bHaveF54Query41;
  bool bHaveF54Query42;
  bool bHaveF54Query43;
  bool bHaveF54Query44;
  bool bHaveF54Query45;
  bool bHaveF54Query46;
  bool bHaveF54Query47;
  bool bHaveF54Query48;
  bool bHaveF54Query49;
  bool bHaveF54Query50;
  bool bHaveF54Query51;
  bool bHaveF54Query52;
  bool bHaveF54Query53;
  bool bHaveF54Query54;
  bool bHaveF54Query55;
  bool bHaveF54Query56;
  bool bHaveF54Query57;
  bool bHaveF54Query58;
  bool bHaveF54Query59;
  bool bHaveF54Query60;
  bool bHaveF54Query61;
  bool bHaveF54Query62;
  bool bHaveF54Query63;
  bool bHaveF54Query64;
  bool bHaveF54Query65;
  bool bHaveF54Query66;
  bool bHaveF54Query67;
  bool bHaveF54Query68;
  bool bHaveF54Query69;
  bool bHaveF54Query70;
  bool bHaveF54Query71;
  // Function 55
  bool bHaveF55Query03;
  bool bHaveF55Query04;
  bool bHaveF55Query05;
  bool bHaveF55Query06;
  bool bHaveF55Query07;
  bool bHaveF55Query08;
  bool bHaveF55Query09;
  bool bHaveF55Query10;
  bool bHaveF55Query11;
  bool bHaveF55Query12;
  bool bHaveF55Query13;
  bool bHaveF55Query14;
  bool bHaveF55Query15;
  bool bHaveF55Query16;
  bool bHaveF55Query17;
  bool bHaveF55Query18;
  bool bHaveF55Query19;
  bool bHaveF55Query20;
  bool bHaveF55Query21;
  bool bHaveF55Query22;
  bool bHaveF55Query23;
  bool bHaveF55Query24;
  bool bHaveF55Query25;
  bool bHaveF55Query26;
  bool bHaveF55Query27;
  bool bHaveF55Query28;
  bool bHaveF55Query29;
  bool bHaveF55Query30;
  bool bHaveF55Query31;
  bool bHaveF55Query32;
  bool bHaveF55Query33;

  bool bHaveF55Ctrl00;
  bool bHaveF55Ctrl01;
  bool bHaveF55Ctrl02;
  bool bHaveF55Ctrl03;
  bool bHaveF55Ctrl04;
  bool bHaveF55Ctrl05;
  bool bHaveF55Ctrl06;
  bool bHaveF55Ctrl07;
  bool bHaveF55Ctrl08;
  bool bHaveF55Ctrl09;
  bool bHaveF55Ctrl10;
  bool bHaveF55Ctrl11;
  bool bHaveF55Ctrl12;
  bool bHaveF55Ctrl13;
  bool bHaveF55Ctrl14;
  bool bHaveF55Ctrl15;
  bool bHaveF55Ctrl16;
  bool bHaveF55Ctrl17;
  bool bHaveF55Ctrl18;
  bool bHaveF55Ctrl19;
  bool bHaveF55Ctrl20;
  bool bHaveF55Ctrl21;
  bool bHaveF55Ctrl22;
  bool bHaveF55Ctrl23;
  bool bHaveF55Ctrl24;
  bool bHaveF55Ctrl25;
  bool bHaveF55Ctrl26;
  bool bHaveF55Ctrl27;
  bool bHaveF55Ctrl28;
  bool bHaveF55Ctrl29;
  bool bHaveF55Ctrl30;
  bool bHaveF55Ctrl31;
  bool bHaveF55Ctrl32;
  bool bHaveF55Ctrl33;
  bool bHaveF55Ctrl34;
  bool bHaveF55Ctrl35;
  bool bHaveF55Ctrl36;
  bool bHaveF55Ctrl37;
  bool bHaveF55Ctrl38;
  bool bHaveF55Ctrl39;
  bool bHaveF55Ctrl40;
  bool bHaveF55Ctrl41;
  bool bHaveF55Ctrl42;
  bool bHaveF55Ctrl43;
  bool bHaveF55Ctrl44;
  bool bHaveF55Ctrl45;
  bool bHaveF55Ctrl46;

  bool ButtonShared;
  bool bIncellDevice;

  uint8_t RxChannelCount;
  uint8_t TxChannelCount;

  uint8_t TouchControllerFamily;
  uint8_t CurveCompensationMode;
  uint8_t NumberOfSensingFrequencies;
  uint8_t F12Ctrl20Size;
  uint8_t F12Ctrl27Size;
  uint8_t F12Ctrl20Offset;
  uint8_t F12Ctrl27Offset;

  uint8_t F21Ctrl04Offset;
  uint8_t F21Ctrl23Offset;

  uint8_t F54Ctrl07Offset;
  uint8_t F54Ctrl08Offset;
  uint8_t F54Ctrl20Offset;
  uint8_t F54Ctrl41Offset;
  uint8_t F54Ctrl57Offset;
  uint8_t F54Ctrl86Offset;
  uint8_t F54Ctrl88Offset;
  uint8_t F54Ctrl89Offset;
  uint8_t F54Ctrl91Offset;
  uint8_t F54Ctrl95Offset;
  uint8_t F54Ctrl96Offset;
  uint8_t F54Ctrl97Offset;
  uint8_t F54Ctrl98Offset;
  uint8_t F54Ctrl99Offset;
  uint8_t F54Ctrl102Offset;
  uint8_t F54Ctrl132Offset;
  uint8_t F54Ctrl146Offset;
  uint8_t F54Ctrl147Offset;
  uint8_t F54Ctrl149Offset;
  uint8_t F54Ctrl182Offset;
  uint8_t F54Ctrl186Offset;
  uint8_t F54Ctrl188Offset;
  uint8_t F54Ctrl189Offset;
  uint8_t F54Ctrl190Offset;
  uint8_t F54Ctrl215Offset;
  uint8_t F54Ctrl225Offset;
  uint8_t F54Ctrl246Offset;

  uint8_t F54Data04Offset;
  uint8_t F54Data06Offset;
  uint8_t F54Data07Offset;
  uint8_t F54Data10Offset;
  uint8_t F54Data14Offset;
  uint8_t F54Data17Offset;
  uint8_t F54Data24Offset;
  uint8_t F54Data31Offset;

  uint8_t F55Ctrl14Offset;
  uint8_t F55Ctrl45Offset;
  uint8_t ForceTRxMAXCount;
  uint8_t ForcePhysicalCount;
  uint8_t _2DTxCount;
  uint8_t _2DRxCount;
  uint8_t ButtonTx[8];
  uint8_t ButtonRx[8];
  uint8_t ButtonCount;
  uint8_t GuardTx[4];
  uint8_t GuardRx[4];
  uint8_t ForceTx[FORCE_COUNT];
  uint8_t ForceRx[FORCE_COUNT];
  uint8_t F12Support;
  uint8_t F12ControlRegisterPresence;

  int32_t MaxArrayLength;
  uint8_t TRxPhysical[TRX_MAPPING_MAX]; // for DS5
  uint8_t TRxPhysical_bit[TRX_BITMAP_LENGTH];
  uint8_t TxPhysical[TRX_MAPPING_MAX]; // for DS4
  uint8_t RxPhysical[TRX_MAPPING_MAX]; // for DS4
  uint8_t TxPhysicalMax;
  //incell pin
  uint8_t tsvd;
  uint8_t tshd;
  uint8_t tsstb;
  uint8_t tsfst;
  uint8_t tsfrq;
  uint8_t exvcom_pin_type;
  uint8_t exvcom1;
  uint8_t exvcom2;
  uint8_t exvcom_sel;
  uint8_t enable_guard;
  uint8_t guard_ring;
  uint8_t enable_verf;
  uint8_t verf;
  uint16_t F01_interrupt;
  uint8_t AbsRxRefLo;
  uint8_t AbsTxRefLo;
  uint8_t maxRxCount;
  uint8_t maxTxCount;

}PDT;




//////////////////////////////////////////

int synaptics_touch_limit_parser(MODEL id, char *m_name, struct synaptics_touch_limit* limit);
int synaptics_touch_limit_table_parser(MODEL id, char *m_name, struct synaptics_touch_limit* limit);
void init_synaptics_data_for_start_test(void);

void init_tch_power_set(int);
void init_limit_data(struct synaptics_touch_limit *);
bool init_i2c_set_slvAddr_depending_channel(int, int, unsigned int *);
bool release_i2c_set();
bool synaptics_ScanPDT(unsigned int *);
bool synaptics_FW_Version_test(unsigned int *);
bool synaptics_ConfigID_test(unsigned int *);
bool synaptics_Noise_test(unsigned int *);
bool synaptics_Full_Raw_Capacitance_test(unsigned int *);
int synaptics_Hybrid_Raw_Capacitance_TX_RX_test(unsigned int *);
int synaptics_Extended_TRX_Short_test(unsigned int *);
bool synaptics_SideTouchRaw_test(unsigned int *);
int synaptics_Extended_High_Resistance_test(unsigned int *);
bool synaptics_SideTouchNoise_test(unsigned int *);
int synaptics_ResetPin_test(int,unsigned int *);
int synaptics_Attention_test(int,unsigned int *);
int synaptics_Lockdown_Check_test(unsigned int *);
int synaptics_Device_Package_test(unsigned int *);
int synaptics_ProjectID_test(MODEL,unsigned int *);
int synaptics_Extended_High_Resistance_test(unsigned int *);
int synaptics_Extended_TRX_Short_test_mode_select(int, unsigned int *);
int synaptics_TouchOTP_test(unsigned int *); //temp
int synaptics_PA2_GND_test(unsigned int *);
#endif	/* __STM_TOUCH_H__ */


