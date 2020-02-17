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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "INI_API/iniparser.h"

#include <atmel_mxt540s.h>
#include <i2c-dev.h>
#include <atmel_touch.h>
#include <type.h>

#define	ID_BLOCK_SIZE	7

#define	MAX_TOUCH_FINGER	2

#define MXT_CONFIG_VERSION_LENGTH	30

/* Touchscreen configuration infomation */
#define MXT_FW_MAGIC		0x4D3C2B1A

/* Message type of T100 object */
#define MXT_T100_SCREEN_MSG_FIRST_RPT_ID	0
#define MXT_T100_SCREEN_MSG_SECOND_RPT_ID	1
#define MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID	2

/* Event Types of T100 object */
#define MXT_T100_DETECT_MSG_MASK	7

#define MXT_T100_EVENT_NONE			0
#define MXT_T100_EVENT_MOVE			1
#define MXT_T100_EVENT_UNSUPPRESS	2
#define MXT_T100_EVENT_SUPPESS		3
#define MXT_T100_EVENT_DOWN			4
#define MXT_T100_EVENT_UP			5
#define MXT_T100_EVENT_UNSUPSUP		6
#define MXT_T100_EVENT_UNSUPUP		7
#define MXT_T100_EVENT_DOWNSUP		8
#define MXT_T100_EVENT_DOWNUP		9

/* Tool types of T100 object */
#define MXT_T100_TYPE_RESERVED			0
#define MXT_T100_TYPE_FINGER			1
#define MXT_T100_TYPE_PASSIVE_STYLUS	2
#define MXT_T100_TYPE_ACTIVE_STYLUS		3
#define MXT_T100_TYPE_HOVERING_FINGER	4

#define SELF_TEST_ANALOG       0x01 
#define SELF_TEST_PIN_FAULT    0x11 
#define SELF_TEST_PIN_FAULT_2  0x12 
#define SELF_TEST_AND_GATE     0x13 
#define SELF_TEST_SIGNAL_LIMIT 0x17 
#define SELF_TEST_GAIN         0x20 
#define SELF_TEST_OFFSET       0x21 
#define SELF_TEST_ALL          0xFE 
#define SELF_TEST_INVALID      0xFD 
#define SELF_TEST_TIMEOUT      0xFC 

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define FLOAT_ROUND_OFF(x)  ((int)(x*10))%10 > 4 ? x+1:x

#define DIAGNOSTIC_DEBUG
#define MICRODEFECT_DEBUG

int dev_gpio;
unsigned char ts_id[ID_BLOCK_SIZE];

char lockdownState;
char configState;

struct mxt_info_block {
    unsigned char family_id;
    unsigned char variant_id;
    unsigned char version;
    unsigned char build;
    unsigned char matrix_xsize;
    unsigned char matrix_ysize;
    unsigned char object_num;
};

struct mxt_object 
{
	__u8 	object_type;
	__u8	start_address_lsb;
	__u8	start_address_msb;
	__u8	size;
	__u8	instances;
	__u8	num_report_ids;
};

struct mxt_report_id_map {
    unsigned char object_type;
    unsigned char instance;
};

struct mxt_finger_info {
    short x;
    short y;
    short z;
    unsigned short w;
    char state;
#if TSP_USE_SHAPETOUCH
    short component;
#endif
    unsigned short mcount;
};

struct mxt_data 
{
    struct mxt_info_block info;
    struct mxt_object objects[512];
	struct mxt_report_id_map rid_map[512];

#if CHECK_ANTITOUCH
    unsigned char check_antitouch;
    unsigned char check_timer;
    unsigned char check_autocal;
    unsigned char check_calgood;
#endif
    unsigned char tsp_ctrl;
    unsigned char max_report_id;
	unsigned char min_report_id;
    unsigned char finger_report_id;
    unsigned short msg_proc;
    unsigned short cmd_proc;
    unsigned short msg_object_size;
    unsigned int x_dropbits:2;
    unsigned int y_dropbits:2;
    unsigned int finger_mask;
    int num_fingers;
    int mxt_enabled;
    struct mxt_finger_info fingers[];
};

typedef struct coord_info
{
	unsigned short	x_coord;
	unsigned short	y_coord;
	unsigned short	z_value;
	int				status;
}coord;

struct mxt_data data;
unsigned char i2c_send_buf[255];

uint8_t mxt_i2c_address_table[] =
{
	0x4A,
	0x4B,
	0x4C,
	0x4D,
	0x5A,
	0x5B,
	0x24,
	0x25,
	0x26,
	0x27,
	0x34,
	0x35
};

unsigned short t25_addr = 0;
unsigned char t25_reportid_max = 0;
unsigned char t25_reportid_min = 0;

unsigned short t37_addr = 0;
unsigned short t37_size = 0;

unsigned short t8_addr = 0;
unsigned short t8_size = 0;

unsigned long	cap_limit_low = 5000;
unsigned long   cap_limit_high = 9500;

int t8Data_org[15];
int t8Data_halfChg[15];
int mux_dev = 0;


/////////////////////

unsigned short  l_fw_ver;
unsigned int    l_config_ver;
float			l_md_set_charge;
double			l_md_x_center;
double			l_md_y_center;
double			l_md_x_border;
double			l_md_y_border;

int nodeDetection_MAX[300][300];
int nodeDetection_MIN[300][300];
int delta_MAX[300][300];
int delta_MIN[300][300];
////////////////////
#define	ATMEL_INT_CHG_RET			0
#define	ATMEL_CONFIG_CRC_RET		1
///////////////////

#if	0	/* swchoi - comment to avoid warning message as it is not used */
static int iic_write1(unsigned char val)
{
    int ret;
    char * bfg_data;

	FUNC_BEGIN();
    bfg_data = (char*)malloc(1);
    bfg_data[0] = (val & 0xFF);

    ret = write(mux_dev, bfg_data, 1);
	FUNC_END();
    return ret;
}
#endif	/* swchoi - end */


int mxt_write_direct(unsigned short reg, unsigned char offset, unsigned char val, int mode);

int parse_ini_file(char * ini_name, int mode)
{
    dictionary  *   ini ;
    int t_addr;
    int t_size;
    char temp_buf[100];
    char temp_buf2[100];
    int t_data[200];
    char *tag_name;
    char reg_name[100][150];
    int g,j;
    int count;

    /* Some temporary variables to hold query results */
    int             i ;

	FUNC_BEGIN();
    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        fprintf(stderr, "cannot parse file: %s\n", ini_name);
		FUNC_END();
        return -1 ;
    }
    iniparser_dump(ini, stderr);

	/* DEBUG_DIAGNOSTIC_T37 */
    tag_name = "DEBUG_DIAGNOSTIC_T37 INSTANCE 0:";
    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    t_data[0] = iniparser_getint(ini, "DEBUG_DIAGNOSTIC_T37 INSTANCE 0:0 1 MODE", -1);
    if(t_data[0] < -1) 	printf("T37[0]: %d\n",t_data[0]);
	else		mxt_write_direct(t_addr, 0, t_data[0], mode);
    t_data[1] = iniparser_getint(ini, "DEBUG_DIAGNOSTIC_T37 INSTANCE 0:1 1 PAGE", -1);
    if(t_data[1] < -1)	printf("T37[1]: %d\n",t_data[1]);
	else        mxt_write_direct(t_addr, 1, t_data[1], mode);

    for(g = 2; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s""%d""%s",tag_name,g,1,"DATA[",g-2,"]");
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);

		if(t_data[g] == -1)
        	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_USERDATA_T38 */
    tag_name = "SPT_USERDATA_T38 INSTANCE 0:";
    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for(g = 0; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s""%d""%s",tag_name,g,1,"DATA[",g,"]");
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_DYNAMICCONFIGURATIONCONTAINER_T71 */
    tag_name = "SPT_DYNAMICCONFIGURATIONCONTAINER_T71 INSTANCE 0:";
    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for(g = 0; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s""%d""%s",tag_name,g,1,"DATA[",g,"]");
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* GEN_POWERCONFIG_T7 */
    tag_name = "GEN_POWERCONFIG_T7 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"IDLEACQINT");
    strcpy(reg_name[count++],"ACTVACQINT");
    strcpy(reg_name[count++],"ACTV2IDLETO");
    strcpy(reg_name[count++],"CFG");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* GEN_ACQUISITIONCONFIG_T8 */
    tag_name = "GEN_ACQUISITIONCONFIG_T8 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CHRGTIME");
    strcpy(reg_name[count++],"ATCHDRIFT");
    strcpy(reg_name[count++],"TCHDRIFT");
    strcpy(reg_name[count++],"DRIFTST");
    strcpy(reg_name[count++],"TCHAUTOCAL");
    strcpy(reg_name[count++],"SYNC");
    strcpy(reg_name[count++],"ATCHCALST");
    strcpy(reg_name[count++],"ATCHCALSTHR");
    strcpy(reg_name[count++],"ATCHFRCCALTHR");
    strcpy(reg_name[count++],"ATCHFRCCALRATIO");
    strcpy(reg_name[count++],"MEASALLOW");
    strcpy(reg_name[count++],"MEASIDLEDEF");
    strcpy(reg_name[count++],"MEASACTVDEF");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);
	
	/* TOUCH_KEYARRAY_T15 */
    tag_name = "TOUCH_KEYARRAY_T15 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"XORIGIN");
    strcpy(reg_name[count++],"YORIGIN");
    strcpy(reg_name[count++],"XSIZE");
    strcpy(reg_name[count++],"YSIZE");
    strcpy(reg_name[count++],"AKSCFG");
    strcpy(reg_name[count++],"BLEN");
    strcpy(reg_name[count++],"TCHTHR");
    strcpy(reg_name[count++],"TCHDI");
    strcpy(reg_name[count++],"TCHHYST");
    strcpy(reg_name[count++],"RESERVED[0]");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_COMMSCONFIG_T18 */
    tag_name = "SPT_COMMSCONFIG_T18 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"COMMAND");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_GPIOPWM_T19 */
    tag_name = "SPT_GPIOPWM_T19 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"REPORTMASK");
    strcpy(reg_name[count++],"DIR");
    strcpy(reg_name[count++],"INTPULLUP");
    strcpy(reg_name[count++],"OUT");
    strcpy(reg_name[count++],"WAKE");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
		mxt_write_direct(t_addr, g, t_data[g], mode);
        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* TOUCH_PROXIMITY_T23 */
    tag_name = "TOUCH_PROXIMITY_T23 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"XORIGIN");
    strcpy(reg_name[count++],"YORIGIN");
    strcpy(reg_name[count++],"XSIZE");
    strcpy(reg_name[count++],"YSIZE");
    strcpy(reg_name[count++],"RESERVED[0]");
    strcpy(reg_name[count++],"BLEN");
    strcpy(reg_name[count++],"FXDDTHR");
    count++;
    strcpy(reg_name[count++],"FXDDI");
    strcpy(reg_name[count++],"AVERAGE");
    strcpy(reg_name[count++],"MVNULLRATE");
    count++;
    strcpy(reg_name[count++],"MVDTHR");
    count++;
    strcpy(reg_name[count++],"CFG");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"FXDDTHR"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"MVNULLRATE"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"MVDTHR"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }

	for( g = 0 ; g < t_size; g++)	
		mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);

	/* SPT_SELFTEST_T25 */
    tag_name = "SPT_SELFTEST_T25 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CMD");
    strcpy(reg_name[count++],"SIGLIM[0].UPSIGLIM"); count++;
    strcpy(reg_name[count++],"SIGLIM[0].LOSIGLIM"); count++;
    strcpy(reg_name[count++],"SIGLIM[1].UPSIGLIM"); count++;
    strcpy(reg_name[count++],"SIGLIM[1].LOSIGLIM"); count++;
    strcpy(reg_name[count++],"SIGLIM[2].UPSIGLIM"); count++;
    strcpy(reg_name[count++],"SIGLIM[2].LOSIGLIM"); count++;
    strcpy(reg_name[count++],"PINDWELLUS");
    strcpy(reg_name[count++],"SIGRANGELIM[0]"); count++;
    strcpy(reg_name[count++],"SIGRANGELIM[1]"); count++;
    strcpy(reg_name[count++],"SIGRANGELIM[2]"); count++;

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"SIGLIM[0].UPSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGLIM[0].LOSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGLIM[1].UPSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGLIM[1].LOSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGLIM[2].UPSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGLIM[2].LOSIGLIM"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGRANGELIM[0]"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGRANGELIM[1]"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"SIGRANGELIM[2]"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }

    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);
	
	/* PROCI_GRIPSUPPRESSION_T40 */
    tag_name = "PROCI_GRIPSUPPRESSION_T40 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"XLOGRIP");
    strcpy(reg_name[count++],"XHIGRIP");
    strcpy(reg_name[count++],"YLOGRIP");
    strcpy(reg_name[count++],"YHIGRIP");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }

	printf("Finished : %s\n",tag_name);

	/* PROCI_TOUCHSUPPRESSION_T42 */
    tag_name = "PROCI_TOUCHSUPPRESSION_T42 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"APPRTHR");
    strcpy(reg_name[count++],"MAXAPPRAREA");
    strcpy(reg_name[count++],"MAXTCHAREA");
    strcpy(reg_name[count++],"SUPSTRENGTH");
    strcpy(reg_name[count++],"SUPEXTTO");
    strcpy(reg_name[count++],"MAXNUMTCHS");
    strcpy(reg_name[count++],"SHAPESTRENGTH");
    strcpy(reg_name[count++],"SUPDIST");
    strcpy(reg_name[count++],"DISTHYST");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_CTECONFIG_T46 */
    tag_name = "SPT_CTECONFIG_T46 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"MODE");
    strcpy(reg_name[count++],"IDLESYNCSPERX");
    strcpy(reg_name[count++],"ACTVSYNCSPERX");
    strcpy(reg_name[count++],"ADCSPERSYNC");
    strcpy(reg_name[count++],"PULSESPERADC");
    strcpy(reg_name[count++],"XSLEW");
    strcpy(reg_name[count++],"SYNCDELAY");  count++;
    strcpy(reg_name[count++],"XVOLTAGE");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"SYNCDELAY"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }

    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);

	/* PROCI_STYLUS_T47 */
    tag_name = "PROCI_STYLUS_T47 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CONTMIN");
    strcpy(reg_name[count++],"CONTMAX");
    strcpy(reg_name[count++],"STABILITY");
    strcpy(reg_name[count++],"MAXTCHAREA");
    strcpy(reg_name[count++],"AMPLTHR");
    strcpy(reg_name[count++],"STYSHAPE");
    strcpy(reg_name[count++],"HOVERSUP");
    strcpy(reg_name[count++],"CONFTHR");
    strcpy(reg_name[count++],"SYNCSPERX");
    strcpy(reg_name[count++],"XPOSADJ");
    strcpy(reg_name[count++],"YPOSADJ");
    strcpy(reg_name[count++],"CFG");
    strcpy(reg_name[count++],"RESERVED[0]");
    strcpy(reg_name[count++],"RESERVED[1]");
    strcpy(reg_name[count++],"RESERVED[2]");
    strcpy(reg_name[count++],"RESERVED[3]");
    strcpy(reg_name[count++],"RESERVED[4]");
    strcpy(reg_name[count++],"RESERVED[5]");
    strcpy(reg_name[count++],"RESERVED[6]");
    strcpy(reg_name[count++],"SUPSTYTO");
    strcpy(reg_name[count++],"MAXNUMSTY");
    strcpy(reg_name[count++],"XEDGECTRL");
    strcpy(reg_name[count++],"XEDGEDIST");
    strcpy(reg_name[count++],"YEDGECTRL");
    strcpy(reg_name[count++],"YEDGEDIST");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* PROCI_ADAPTIVETHRESHOLD_T55 */
    tag_name = "PROCI_ADAPTIVETHRESHOLD_T55 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"TARGETTHR");
    strcpy(reg_name[count++],"THRADJLIM");
    strcpy(reg_name[count++],"RESETSTEPTIME");
    strcpy(reg_name[count++],"FORCECHGDIST");
    strcpy(reg_name[count++],"FORCECHGTIME");
    strcpy(reg_name[count++],"LOWESTTHR");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);
	
	/* PROCI_SHIELDLESS_T56 */
    tag_name = "PROCI_SHIELDLESS_T56 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"COMMAND");
    strcpy(reg_name[count++],"OPTINT");
    strcpy(reg_name[count++],"INTTIME");

    for( g = 4 ; g < 34; g++)
    {
        sprintf(temp_buf,"%s""%d""%s","INTDELAY[",g-4,"]");
        strcpy(reg_name[g],temp_buf);
        count++;
    }
    strcpy(reg_name[34],"MULTICUTGC");
    strcpy(reg_name[35],"GCLIMIT");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_TIMER_T61 INSTANCE:0*/
    tag_name = "SPT_TIMER_T61 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CMD");
    strcpy(reg_name[count++],"MODE");
    strcpy(reg_name[count++],"PERIOD"); count++;

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"PERIOD"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }
    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);

    /* SPT_TIMER_T61 INSTANCE:1*/
    tag_name = "SPT_TIMER_T61 INSTANCE 1:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CMD");
    strcpy(reg_name[count++],"MODE");
    strcpy(reg_name[count++],"PERIOD"); count++;

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"PERIOD"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }
    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);

	/* PROCI_LENSBENDING_T65 */
    tag_name = "PROCI_LENSBENDING_T65 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"GRADTHR");
    strcpy(reg_name[count++],"YLONOISEMUL");    count++;
    strcpy(reg_name[count++],"YLONOISEDIV");    count++;
    strcpy(reg_name[count++],"YHINOISEMUL");    count++;
    strcpy(reg_name[count++],"YHINOISEDIV");    count++;
    strcpy(reg_name[count++],"LPFILTCOEF");
    strcpy(reg_name[count++],"FORCESCALE");     count++;
    strcpy(reg_name[count++],"FORCETHR");
    strcpy(reg_name[count++],"FORCETHRHYST");
    strcpy(reg_name[count++],"FORCEDI");
    strcpy(reg_name[count++],"FORCEHYST");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"YLONOISEMUL"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"YLONOISEDIV"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"YHINOISEMUL"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"YHINOISEDIV"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"FORCESCALE"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }
    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);

	/* SPT_GOLDENREFERENCES_T66 */
    tag_name = "SPT_GOLDENREFERENCES_T66 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"FCALFAILTHR");
    strcpy(reg_name[count++],"FCALDRIFTCNT");
    strcpy(reg_name[count++],"UNKNOWN[3]");
    strcpy(reg_name[count++],"UNKNOWN[4]");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* PROCI_PALMGESTUREPROCESSOR_T69 */
    tag_name = "PROCI_PALMGESTUREPROCESSOR_T69 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"LONGDIMTHR");
    strcpy(reg_name[count++],"SHORTDIMTHR");
    strcpy(reg_name[count++],"LONGDIMHYST");
    strcpy(reg_name[count++],"SHORTDIMHYST");
    strcpy(reg_name[count++],"MOVTHR");
    strcpy(reg_name[count++],"MOVTHRTO");
    strcpy(reg_name[count++],"AREATHR");
    strcpy(reg_name[count++],"AREATHRTO");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* SPT_DYNAMICCONFIGURATIONCONTROLLER_T70 INSTAVCE[0:7]*/
    for( j = 0 ; j < 8; j++)
    {
        sprintf(temp_buf2,"%s""%d""%s","SPT_DYNAMICCONFIGURATIONCONTROLLER_T70 INSTANCE ",j,":");
        count = 0;
        strcpy(reg_name[count++],"CTRL");
        strcpy(reg_name[count++],"EVENT");          count++;
        strcpy(reg_name[count++],"OBJTYPE");
        strcpy(reg_name[count++],"RESERVED[0]");
        strcpy(reg_name[count++],"OBJINST");
        strcpy(reg_name[count++],"DSTOFFSET");
        strcpy(reg_name[count++],"SRCOFFSET");      count++;
        strcpy(reg_name[count++],"LENGTH");

        sprintf(temp_buf,"%s""%s",temp_buf2,"OBJECT_ADDRESS");
        t_addr = iniparser_getint(ini, temp_buf, -1);

        sprintf(temp_buf,"%s""%s",temp_buf2,"OBJECT_SIZE");
        t_size = iniparser_getint(ini, temp_buf, -1);

        for( g = 0 ; g < t_size; g++)
        {
            if(!strcmp(reg_name[g],"EVENT"))
            {
                sprintf(temp_buf,"%s""%d"" %d ""%s",temp_buf2,g,2,reg_name[g]);
                i = iniparser_getint(ini,temp_buf, -1);
                if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

                t_data[g]   = i & 0xFF;
                t_data[g+1] = (i>>8) & 0xFF;
                g = g + 1;
            }
            else if(!strcmp(reg_name[g],"SRCOFFSET"))
            {
                sprintf(temp_buf,"%s""%d"" %d ""%s",temp_buf2,g,2,reg_name[g]);
                i = iniparser_getint(ini,temp_buf, -1);
                if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

                t_data[g]   = i & 0xFF;
                t_data[g+1] = (i>>8) & 0xFF;
                g = g + 1;
            }
            else
            {
                sprintf(temp_buf,"%s""%d"" %d ""%s",temp_buf2,g,1,reg_name[g]);
                t_data[g] = iniparser_getint(ini,temp_buf, -1);
                if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
            }
        }
    	for( g = 0 ; g < t_size; g++)   
        	mxt_write_direct(t_addr, g, t_data[g], mode);

		printf("Finished : %s\n",temp_buf2);
    }

	/* PROCG_NOISESUPPRESSION_T72 */
    tag_name = "PROCG_NOISESUPPRESSION_T72 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CALCFG1");
    strcpy(reg_name[count++],"CFG1");
    strcpy(reg_name[count++],"CFG2");
    strcpy(reg_name[count++],"DEBUGCFG");
    strcpy(reg_name[count++],"HOPCNT");
    strcpy(reg_name[count++],"HOPCNTPER");
    strcpy(reg_name[count++],"HOPEVALTO");
    strcpy(reg_name[count++],"HOPST");
    strcpy(reg_name[count++],"NLGAINDUALX");
    strcpy(reg_name[count++],"MINNLTHR");
    strcpy(reg_name[count++],"INCNLTHR");
    strcpy(reg_name[count++],"FALLNLTHR");
    strcpy(reg_name[count++],"NLTHRMARGIN");
    strcpy(reg_name[count++],"MINTHRADJ");
    strcpy(reg_name[count++],"NLTHRLIMIT");
    strcpy(reg_name[count++],"BGSCAN");
    strcpy(reg_name[count++],"NLGAINSINGX");
    strcpy(reg_name[count++],"BLKNLTHR");
    strcpy(reg_name[count++],"RESERVED[0]");
    strcpy(reg_name[count++],"STATE[0]_CTRL");
    strcpy(reg_name[count++],"STATE[0]_FREQ[0]");
    strcpy(reg_name[count++],"STATE[0]_FREQ[1]");
    strcpy(reg_name[count++],"STATE[0]_FREQ[2]");
    strcpy(reg_name[count++],"STATE[0]_FREQ[3]");
    strcpy(reg_name[count++],"STATE[0]_FREQ[4]");
    strcpy(reg_name[count++],"STATE[0]_TCHAPX[0]");
    strcpy(reg_name[count++],"STATE[0]_TCHAPX[1]");
    strcpy(reg_name[count++],"STATE[0]_TCHAPX[2]");
    strcpy(reg_name[count++],"STATE[0]_TCHAPX[3]");
    strcpy(reg_name[count++],"STATE[0]_TCHAPX[4]");
    strcpy(reg_name[count++],"STATE[0]_NOTCHAPX[0]");
    strcpy(reg_name[count++],"STATE[0]_NOTCHAPX[1]");
    strcpy(reg_name[count++],"STATE[0]_NOTCHAPX[2]");
    strcpy(reg_name[count++],"STATE[0]_NOTCHAPX[3]");
    strcpy(reg_name[count++],"STATE[0]_NOTCHAPX[4]");
    strcpy(reg_name[count++],"STATE[0]_PC");
    strcpy(reg_name[count++],"STATE[0]_LOWNLTHR");
    strcpy(reg_name[count++],"STATE[0]_HIGHNLTHR");
    strcpy(reg_name[count++],"STATE[0]_CNT");
    strcpy(reg_name[count++],"STATE[1]_CTRL");
    strcpy(reg_name[count++],"STATE[1]_FREQ[0]");
    strcpy(reg_name[count++],"STATE[1]_FREQ[1]");
    strcpy(reg_name[count++],"STATE[1]_FREQ[2]");
    strcpy(reg_name[count++],"STATE[1]_FREQ[3]");
    strcpy(reg_name[count++],"STATE[1]_FREQ[4]");
    strcpy(reg_name[count++],"STATE[1]_TCHAPX[0]");
    strcpy(reg_name[count++],"STATE[1]_TCHAPX[1]");
    strcpy(reg_name[count++],"STATE[1]_TCHAPX[2]");
    strcpy(reg_name[count++],"STATE[1]_TCHAPX[3]");
    strcpy(reg_name[count++],"STATE[1]_TCHAPX[4]");
    strcpy(reg_name[count++],"STATE[1]_NOTCHAPX[0]");
    strcpy(reg_name[count++],"STATE[1]_NOTCHAPX[1]");
    strcpy(reg_name[count++],"STATE[1]_NOTCHAPX[2]");
    strcpy(reg_name[count++],"STATE[1]_NOTCHAPX[3]");
    strcpy(reg_name[count++],"STATE[1]_NOTCHAPX[4]");
    strcpy(reg_name[count++],"STATE[1]_PC");
    strcpy(reg_name[count++],"STATE[1]_LOWNLTHR");
    strcpy(reg_name[count++],"STATE[1]_HIGHNLTHR");
    strcpy(reg_name[count++],"STATE[1]_CNT");
    strcpy(reg_name[count++],"STATE[2]_CTRL");
    strcpy(reg_name[count++],"STATE[2]_FREQ[0]");
    strcpy(reg_name[count++],"STATE[2]_FREQ[1]");
    strcpy(reg_name[count++],"STATE[2]_FREQ[2]");
    strcpy(reg_name[count++],"STATE[2]_FREQ[3]");
    strcpy(reg_name[count++],"STATE[2]_FREQ[4]");
    strcpy(reg_name[count++],"STATE[2]_TCHAPX[0]");
    strcpy(reg_name[count++],"STATE[2]_TCHAPX[1]");
    strcpy(reg_name[count++],"STATE[2]_TCHAPX[2]");
    strcpy(reg_name[count++],"STATE[2]_TCHAPX[3]");
    strcpy(reg_name[count++],"STATE[2]_TCHAPX[4]");
    strcpy(reg_name[count++],"STATE[2]_NOTCHAPX[0]");
    strcpy(reg_name[count++],"STATE[2]_NOTCHAPX[1]");
    strcpy(reg_name[count++],"STATE[2]_NOTCHAPX[2]");
    strcpy(reg_name[count++],"STATE[2]_NOTCHAPX[3]");
    strcpy(reg_name[count++],"STATE[2]_NOTCHAPX[4]");
    strcpy(reg_name[count++],"STATE[2]_PC");
    strcpy(reg_name[count++],"STATE[2]_LOWNLTHR");
    strcpy(reg_name[count++],"STATE[2]_HIGHNLTHR");
    strcpy(reg_name[count++],"STATE[2]_CNT");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* PROCI_GLOVEDETECTION_T78 */
    tag_name = "PROCI_GLOVEDETECTION_T78 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"MINAREA");
    strcpy(reg_name[count++],"CONFTHR");
    strcpy(reg_name[count++],"MINDIST");
    strcpy(reg_name[count++],"GLOVEMODETO");
    strcpy(reg_name[count++],"SUPTO");
    strcpy(reg_name[count++],"SYNCSPERX");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);
    //printf("%s address : 0x%02X /size : %d\n",tag_name,t_addr, t_size);

    for( g = 0 ; g < t_size; g++)
    {
        sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
        t_data[g] = iniparser_getint(ini,temp_buf, -1);
        mxt_write_direct(t_addr, g, t_data[g], mode);

        if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
    }
	printf("Finished : %s\n",tag_name);

	/* TOUCH_MULTITOUCHSCREEN_T100 */
    tag_name = "TOUCH_MULTITOUCHSCREEN_T100 INSTANCE 0:";
    count = 0;
    strcpy(reg_name[count++],"CTRL");
    strcpy(reg_name[count++],"CFG1");
    strcpy(reg_name[count++],"SCRAUX");
    strcpy(reg_name[count++],"TCHAUX");
    strcpy(reg_name[count++],"TCHEVENTCFG");
    strcpy(reg_name[count++],"AKSCFG");
    strcpy(reg_name[count++],"NUMTCH");
    strcpy(reg_name[count++],"XYCFG");
    strcpy(reg_name[count++],"XORIGIN");
    strcpy(reg_name[count++],"XSIZE");
    strcpy(reg_name[count++],"XPITCH");
    strcpy(reg_name[count++],"XLOCLIP");
    strcpy(reg_name[count++],"XHICLIP");
    strcpy(reg_name[count++],"XRANGE");         count++;
    strcpy(reg_name[count++],"XEDGECFG");
    strcpy(reg_name[count++],"XEDGEDIST");
    strcpy(reg_name[count++],"DXXEDGECFG");
    strcpy(reg_name[count++],"DXXEDGEDIST");
    strcpy(reg_name[count++],"YORIGIN");
    strcpy(reg_name[count++],"YSIZE");
    strcpy(reg_name[count++],"YPITCH");
    strcpy(reg_name[count++],"YLOCLIP");
    strcpy(reg_name[count++],"YHICLIP");
    strcpy(reg_name[count++],"YRANGE");         count++;
    strcpy(reg_name[count++],"YEDGECFG");
    strcpy(reg_name[count++],"YEDGEDIST");
    strcpy(reg_name[count++],"GAIN");
    strcpy(reg_name[count++],"DXGAIN");
    strcpy(reg_name[count++],"TCHTHR");
    strcpy(reg_name[count++],"TCHHYST");
    strcpy(reg_name[count++],"INTTHR");
    strcpy(reg_name[count++],"NOISESF");
    strcpy(reg_name[count++],"CUTOFFTHR");
    strcpy(reg_name[count++],"MRGTHR");
    strcpy(reg_name[count++],"MRGTHRADJSTR");
    strcpy(reg_name[count++],"MRGHYST");
    strcpy(reg_name[count++],"DXTHRSF");
    strcpy(reg_name[count++],"TCHDIDOWN");
    strcpy(reg_name[count++],"TCHDIUP");
    strcpy(reg_name[count++],"NEXTTCHDI");
    strcpy(reg_name[count++],"RESERVED[0]");
    strcpy(reg_name[count++],"JUMPLIMIT");
    strcpy(reg_name[count++],"MOVFILTER");
    strcpy(reg_name[count++],"MOVSMOOTH");
    strcpy(reg_name[count++],"MOVPRED");
    strcpy(reg_name[count++],"MOVHYSTI");       count++;
    strcpy(reg_name[count++],"MOVHYSTN");       count++;
    strcpy(reg_name[count++],"AMPLHYST");
    strcpy(reg_name[count++],"SCRAREAHYST");
    strcpy(reg_name[count++],"INTTHRHYST");

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_ADDRESS");
    t_addr = iniparser_getint(ini, temp_buf, -1);

    sprintf(temp_buf,"%s""%s",tag_name,"OBJECT_SIZE");
    t_size = iniparser_getint(ini, temp_buf, -1);

    for( g = 0 ; g < t_size; g++)
    {
        if(!strcmp(reg_name[g],"XRANGE"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"YRANGE"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"MOVHYSTI"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else if(!strcmp(reg_name[g],"MOVHYSTN"))
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,2,reg_name[g]);
            i = iniparser_getint(ini,temp_buf, -1);
            if(i == -1)	printf("[%d] = %d/ [%d] = %d\n",g,i&0xFF,g+1,(i>>8)&0xFF);

            t_data[g]   = i & 0xFF;
            t_data[g+1] = (i>>8) & 0xFF;
            g = g + 1;
        }
        else
        {
            sprintf(temp_buf,"%s""%d"" %d ""%s",tag_name,g,1,reg_name[g]);
            t_data[g] = iniparser_getint(ini,temp_buf, -1);
            if(t_data[g] == -1)	printf("[%d] = %d\n",g,t_data[g]);
        }
    }
    for( g = 0 ; g < t_size; g++)   
        mxt_write_direct(t_addr, g, t_data[g], mode);

	printf("Finished : %s\n",tag_name);
	FUNC_END();
	return 1;
}


//////////////////////////////////////////////////////////////////////////////
/// I2C Functions
//////////////////////////////////////////////////////////////////////////////
struct i2c_data{
    unsigned char addr;
    unsigned short reg;
    unsigned char *data;
	int len;
};

struct i2c_read_data{
    unsigned char addr;
    unsigned short reg;
    unsigned char data[4096];
};

#define _IOCTL_ADDR 0x01
#define _IOCTL_REG 0x02

int gpio_i2c_write_atmel(unsigned short addr, unsigned char *data, int len)
{
    struct i2c_read_data i2cd;
    int ret;

	FUNC_BEGIN();
    i2cd.addr = TC_SLAVE_ADDR;
    i2cd.reg = addr;

	memcpy(i2cd.data, data, len);
    ret = write(dev_gpio, &i2cd, len);

	FUNC_END();
    return ret;
}

int gpio_i2c_read_atmel(unsigned short addr, unsigned char *buf, int size){
    struct i2c_read_data i2cd;
    int ret;

	FUNC_BEGIN();
    i2cd.addr = TC_SLAVE_ADDR;
    i2cd.reg = addr;

	ret = read(dev_gpio, &i2cd, size);
    memcpy(buf, (char*)i2cd.data, size);

	FUNC_END();
    return ret;
}

int i2c_general_read(unsigned short addr, unsigned char *buf, int size)
{

    int rc;
    unsigned char temp_buf[2];
    temp_buf[0] = addr & 0xFF;
    temp_buf[1] = (addr >> 8) & 0xFF;

	FUNC_BEGIN();

    rc = write(mux_dev, temp_buf ,2);
	usleep(500);
    rc = read(mux_dev,buf,size);

	FUNC_END();
    return rc;

}

int i2c_general_write(unsigned short addr, unsigned char *data, int len)
{
    int ret;
	int i;

	FUNC_BEGIN();
	i2c_send_buf[0] = addr & 0xFF;
    i2c_send_buf[1] = (addr >> 8) & 0xFF;

    for(i=2; i<len+2; i++)
        i2c_send_buf[i] = data[i-2];

	ret = write(mux_dev, i2c_send_buf, len+2);

	memset(i2c_send_buf,0,len+2);

    if(ret < 0)
	{
		FUNC_END();
        return -1;
	}

	FUNC_END();
    return 0;

}
//////////////////////////////////////////////////////////////////////////////

int mxt_write_mem(unsigned short reg, unsigned char len, unsigned char *buf, int mode)
{
    int ret = 0;

	FUNC_BEGIN();
	if(mode)
	{
		ret = i2c_general_write(reg, buf, len);
		if(ret < 0)	
		{
			printf("I2C Write Failed!\n");
			FUNC_END();
			return I2C_ERR;
		}
	}
	else
	{
        ret = gpio_i2c_write_atmel(reg, buf, len);
        if(ret < 0)
        {
			FUNC_END();
            return I2C_ERR;
        }
	}
	FUNC_END();
	return 0;
}

int mxt_read_mem(unsigned short reg, unsigned char len, unsigned char *buf, int mode)
{
    int ret = 0;

	FUNC_BEGIN();
	if(mode)
	{
		ret = i2c_general_read(reg, buf, len);
		if(ret < 0) 
		{
			printf("I2C Read Failed!\n");
			FUNC_END();
			return I2C_ERR;
		}
	}
	else
	{
		ret = gpio_i2c_read_atmel(reg, buf, len);
        if(ret < 0)
        {
			FUNC_END();
            return I2C_ERR;
        }
	}
	FUNC_END();
	return 0;
}

struct mxt_object *mxt_get_object(unsigned char object_type, int mode)
{
    struct mxt_object *object;
    int i;

	FUNC_BEGIN();
    if (!data.objects)
	{
		FUNC_END();
        return NULL;
	}

	for (i = 0; i < data.info.object_num; i++) 
	{
	    object = data.objects + i;
	    if (object->object_type == object_type)
        {
            printf("find object [object_type : %d] \n",object->object_type);
			FUNC_END();
            return object;
        }
		else
			printf("object_type : %d\n",object->object_type);
	}

	printf("Invalid object type T%d\n",object_type);

	FUNC_END();
    return NULL;
}

int mxt_read_object(unsigned char type, unsigned char offset, unsigned char *val, int mode)
{
    struct mxt_object *object;
    unsigned short reg;

	FUNC_BEGIN();
    object = mxt_get_object(type, mode);
    if (!object)
	{
		FUNC_END();
        return -EINVAL;
	}

	reg = (object->start_address_lsb & 0xff) | ((object->start_address_msb << 8) & 0xff00);
	FUNC_END();
    return mxt_read_mem(reg + offset, 1, val, mode);
}


int mxt_write_object(unsigned char type, unsigned char offset, unsigned char val, int mode)
{
    struct mxt_object *object;
    unsigned short reg;

	FUNC_BEGIN();
    object = mxt_get_object(type, mode);
    if (!object)
	{
		FUNC_END();
        return -EINVAL;
	}

	if (offset >= object->size) {
    	printf("Tried to write outside object T%d offset:%d, size:%d\n",type, offset, object->size);
		FUNC_END();
        return -EINVAL;
    }
	reg = (object->start_address_lsb & 0xff) | ((object->start_address_msb << 8) & 0xff00);
	FUNC_END();
    return mxt_write_mem(reg + offset, 1, &val, mode);
}

int mxt_write_direct(unsigned short reg, unsigned char offset, unsigned char val, int mode)
{
	FUNC_BEGIN();
	FUNC_END();
    return mxt_write_mem(reg + offset, 1, &val, mode);
}

static int mxt_start(int mode)
{
    int error;

	FUNC_BEGIN();
    /* Touch report enable */
    error = mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, MXT_T9_CTRL, data.tsp_ctrl, mode);
    if (error < 0)
	{
    	printf("Fail to start touch\n");
	}

	FUNC_END();
    return error;
}

void mxt_stop(int mode)
{
	FUNC_BEGIN();
    /* Touch report disable */
	FUNC_END();
    mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, MXT_T9_CTRL, 0, mode);
}

unsigned int crc24(unsigned int crc, unsigned char byte1, unsigned char byte2)
{
    static const unsigned int crcpoly = 0x80001B;
    unsigned int res;
    unsigned short data_word;

	FUNC_BEGIN();
    data_word = (((unsigned short)byte2) << 8) | byte1;
    res = (crc << 1) ^ (unsigned int)data_word;

    if (res & 0x1000000)
        res ^= crcpoly;

	FUNC_END();
    return res;
}

int mxt_read_info_crc(unsigned int *crc_pointer, int mode)
{
    unsigned short crc_address;
    unsigned char msg[3];
    int ret;

	FUNC_BEGIN();
	/* Read Info block CRC address */
	crc_address = OBJECT_TABLE_START_ADDRESS + data.info.object_num * OBJECT_TABLE_ELEMENT_SIZE;

	ret = mxt_read_mem(crc_address, 3, msg, mode);
	if (ret < 0)
	{
		FUNC_END();
	    return ret;
	}

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	FUNC_END();
    return true;
}

int mxt_calculate_infoblock_crc(unsigned int *crc_pointer, int mode)
{
    unsigned int crc = 0;
    int ret;
    int i;

	unsigned char mem[7 + data.info.object_num * 6];
	FUNC_BEGIN();
	ret = mxt_read_mem(0, sizeof(mem), mem, mode);
	
	if (ret < 0)
	{
		FUNC_END();
	    return ret;
	}
	
	for (i = 0; i < sizeof(mem) - 1; i += 2)
	    crc = crc24(crc, mem[i], mem[i + 1]);
	
	*crc_pointer = crc24(crc, mem[i], 0) & 0x00FFFFFF;

	FUNC_END();
    return true;
}

void mxt_handle_init_data(int mode)
{
    int ret;
    unsigned char val;
	unsigned int max_x;
	unsigned int max_y;

	FUNC_BEGIN();
	parse_ini_file("/mnt/sd/atmel_conf.ini", mode);
	printf("----- configuration finished -----\n");
//////// ??? need modify
    max_x = 1279;
    max_y = 719;
//////// ??? 
    /* Set touchscreen resolution */
    mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9,MXT_T9_XRANGE_LSB, max_x & 0xff, mode);
    mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9,MXT_T9_XRANGE_MSB, max_x >> 8, mode);
    mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9,MXT_T9_YRANGE_LSB, max_y & 0xff, mode);
    mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9,MXT_T9_YRANGE_MSB, max_y >> 8, mode);

	/* Set Oriental */
	mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, MXT_T9_CFG1, 112, mode);

	mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, 47, 5, mode);
	mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, 48, 0, mode);
	mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, 49, 2, mode);
	mxt_write_object(TOUCH_MULTITOUCHSCREEN_T9, 50, 0, mode);
    /* Get acquistion time */
    ret = mxt_read_object(TOUCH_MULTITOUCHSCREEN_T9,MXT_T9_CTRL, &val, mode);

	if (ret<0)
	    data.tsp_ctrl = 0x83;
	else
	    data.tsp_ctrl = (val > 0) ? val : 0x83;
    printf("T9_CTRL : %d\n", data.tsp_ctrl);
	FUNC_END();
}

int mxt_reset(int mode)
{
    unsigned char buf = 1;
    int ret = 0;
	FUNC_BEGIN();
    ret = mxt_write_mem(data.cmd_proc + CMD_RESET_OFFSET, 1, &buf, mode);
	FUNC_END();
	return ret;
}

unsigned char mxt_read_reset(int mode)
{
    unsigned char buf = 1;
    int ret = 0;
	FUNC_BEGIN();
    ret = mxt_read_mem(data.cmd_proc + CMD_RESET_OFFSET, 1, &buf, mode);

    if (ret < 0)
	{
        return ret;
		FUNC_END();
	}

	printf("read reset = %d \n",buf);

	FUNC_END();
    return buf;
}


int mxt_backup(int mode)
{
    unsigned char buf = 0x55;
    int ret = 0;
	FUNC_BEGIN();
    ret = mxt_write_mem(data.cmd_proc + CMD_BACKUP_OFFSET, 1, &buf, mode);

	FUNC_END();
	return ret;
}

unsigned char atmel_check_valid_information_block(int mode)
{
    int readSize = 226;
    unsigned char buffer[readSize];

    int i;
    int ret = -1;

	FUNC_BEGIN();
    memset(buffer, 0, sizeof(buffer));
    {
        ret = mxt_read_mem(0, sizeof(buffer), buffer, mode);
    }
    if (ret < 0)
    {
        printf("Read Fail - Get Information Block\n");
		FUNC_END();
        return 0;
    }

    for (i = 0; i < sizeof(buffer); i++)
    {
        if (buffer[i] == 0xFF)
        {
			FUNC_END();
            return 0;
        }
    }

	FUNC_END();
    return 1;
}

unsigned char atmel_get_id(int first_read, int id, int mode)
{
	int ret = -1;

	FUNC_BEGIN();
	if(first_read)
	{
		ret = mxt_read_mem(0, sizeof(ts_id), ts_id,mode);
	
		if(ret)
	    {
	        printf("Read Fail - IC Information\n");
			FUNC_END();
	        return ret;
	    }
	    data.info.family_id = ts_id[0];
		data.info.variant_id = ts_id[1];
		data.info.version = ts_id[2];
		data.info.build = ts_id[3];
		data.info.matrix_xsize = ts_id[4];
		data.info.matrix_ysize = ts_id[5];
		data.info.object_num = ts_id[6];
		printf("family ID : 0x%x\n",ts_id[0]);
		printf("2 : 0x%x\n",ts_id[1]);
		printf("3 : 0x%x\n",ts_id[2]);
		printf("4 : 0x%x\n",ts_id[3]);
		printf("5 : 0x%x\n",ts_id[4]);
		printf("6 : 0x%x\n",ts_id[5]);
		printf("7 : 0x%x\n",ts_id[6]);
	}

	switch(id)
	{
		case 0:	return data.info.family_id;	break;
		case 1: return data.info.variant_id; break;
		case 2: return data.info.version; 	break;
		case 3: return data.info.build; 	break;
		case 4: return data.info.matrix_xsize; 	break;
		case 5: return data.info.matrix_ysize; 	break;
	}
	FUNC_END();
	return 0;
}

void mxt_make_reportid_table(int mode)
{
    int i, j;
    int cur_id, sta_id;
	struct mxt_object *objects = data.objects;

	FUNC_BEGIN();
	data.rid_map[0].instance = 0;
	data.rid_map[0].object_type = 0;
	cur_id = 1;
	
	for (i = 0; i < data.info.object_num; i++) 
	{
	    if (objects[i].num_report_ids == 0)
	        continue;
	    for (j = 1; j <= objects[i].instances; j++) 
		{
	        for (sta_id = cur_id; cur_id < (sta_id + objects[i].num_report_ids);cur_id++) 
			{
	            data.rid_map[cur_id].instance = j;
	            data.rid_map[cur_id].object_type = objects[i].object_type;
	        }
	    }
	}
#ifdef	MXT_DEBUG
	    printf("maXTouch: %d report ID\n",data.max_report_id);
#endif
	
	FUNC_END();
}


void mxt_get_object_table(int mode)
{
	int i;
	int type_count = 0;
	int ret = 0;

	FUNC_BEGIN();
	//if(mode)
	data.max_report_id = 0;
	ret = mxt_read_mem(OBJECT_TABLE_START_ADDRESS, 
					data.info.object_num * sizeof(*data.objects),
						(unsigned char *)data.objects, mode);
    if (ret < 0)
	{
		FUNC_END();
	}

	for (i = 0; i < data.info.object_num; i++) 
	{
	    /* size and instance values are smaller than atual value */
	    data.objects[i].size += 1;
	    data.objects[i].instances += 1;
	    data.max_report_id += data.objects[i].num_report_ids * (data.objects[i].instances);
		data.min_report_id = data.max_report_id - data.objects[i].instances * data.objects[i].num_report_ids + 1; 

	    switch (data.objects[i].object_type) 
		{
	        case GEN_MESSAGEPROCESSOR_T5:
	            data.msg_object_size = data.objects[i].size;
		        data.msg_proc = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
#ifdef	MXT_DEBUG
				printf("Mesage object size: %d message address: 0x%x\n",data.msg_object_size, data.msg_proc);
#endif
	        break;
		    case GEN_COMMANDPROCESSOR_T6:
				data.cmd_proc = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
#ifdef	MXT_DEBUG
				printf("Command object address: 0x%x\n",data.cmd_proc);
#endif
	        break;
		    case TOUCH_MULTITOUCHSCREEN_T9:
		        data.finger_report_id = type_count + 1;
		            //printf("[CH1]Finger report id: %d\n",data.finger_report_id);
	        break;
			case SPT_SELFTEST_T25:
				t25_addr = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
		        t25_reportid_max = data.max_report_id;
        		t25_reportid_min = data.min_report_id;
#ifdef	MXT_DEBUG
				printf("SELFTEST T25 ADDRESS : 0x%x %d %d\n",t25_addr, t25_reportid_max, t25_reportid_min);
#endif
			break;
            case DEBUG_DIAGNOSTIC_T37:
                t37_addr = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
                t37_size = data.objects[i].size;
//					printf("[CH1] DIAGNOSTIC T37 ADDRESS : 0x%x/ SIZE %d\n",t37_addr_1, t37_size_1);
            break;
			case GEN_ACQUISITIONCONFIG_T8:
				t8_addr = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
				t8_size = data.objects[i].size;
			break;
	    }
	
	    if (data.objects[i].num_report_ids) {
	        type_count += data.objects[i].num_report_ids * (data.objects[i].instances);
	    }
	}
#ifdef	MXT_DEBUG
	printf("\n[Object Table]\n");
	for (i = 0; i < data.info.object_num; i++) 
	{
	    printf("Object:T%d\t\tAddress:0x%x\t\tSize:%d\t\tInstance:%d\t\tReport Id's:%d\n",
	    data.objects[i].object_type,
		(data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00),
	    data.objects[i].size,
	    data.objects[i].instances,
	    data.objects[i].num_report_ids);
	}
	printf("\n");
#endif
	FUNC_END();
}

unsigned short mxt_get_object_address(unsigned char object, int mode)
{
	unsigned short addr;
	int i;

	FUNC_BEGIN();
    for (i = 0; i < data.info.object_num; i++)
    {
        if(data.objects[i].object_type == object)
		{
            addr = (data.objects[i].start_address_lsb & 0xff) | ((data.objects[i].start_address_msb << 8) & 0xff00);
			FUNC_END();
			return addr;
		}
    }
	FUNC_END();
	return 0;
}

int atmel_get_main_packet(unsigned char *temp)
{
	FUNC_BEGIN();
	FUNC_END();
	return 1;
}

int atmel_is_valid(unsigned char *temp)
{
	FUNC_BEGIN();
	FUNC_END();
	return 1;
}

int atmel_free_irq(void)
{
	FUNC_BEGIN();
	FUNC_END();
	return 0;
}

int mxt_reportid_to_type(unsigned char report_id, unsigned char *instance)
{
	FUNC_BEGIN();
	if (report_id <= data.max_report_id) 
	{
	    *instance = data.rid_map[report_id].instance;
		FUNC_END();
	    return data.rid_map[report_id].object_type;
	} 
	else
	{
		FUNC_END();
	    return 0;
	}

	FUNC_END();
	return 0;
}

void disable_noise_suppression(int mode)
{
  	unsigned short addr;
	unsigned char disable = 0;
	int ret = 0;

	FUNC_BEGIN();
  	addr = mxt_get_object_address(PROCG_NOISESUPPRESSION_T22, mode);
  	if (addr)	ret = mxt_write_mem(addr, 1, &disable, mode);
    if (ret < 0)
	{
		FUNC_END();
	}
  	addr = mxt_get_object_address(PROCG_NOISESUPPRESSION_T48, mode);
  	if (addr)	ret = mxt_write_mem(addr, 1, &disable, mode);
    if (ret < 0)
	{
		FUNC_END();
	}
  	addr = mxt_get_object_address(PROCG_NOISESUPPRESSION_T54, mode);
  	if (addr)	ret = mxt_write_mem(addr, 1, &disable, mode);
    if (ret < 0)
	{
		FUNC_END();
	}
  	addr = mxt_get_object_address(PROCG_NOISESUPPRESSION_T62, mode);
  	if (addr)	ret = mxt_write_mem(addr, 1, &disable, mode);
    if (ret < 0)
	{
		FUNC_END();
	}
	FUNC_END();
}

long atmel_config_crc(int mode)
{
    unsigned char msg_1[data.msg_object_size];
    int object_type = 0;
    unsigned char instance;
    unsigned int crc = 0;
    int i,ret = 0;

	FUNC_BEGIN();
    for(i=0; i<64; i++)
    {
        ret = mxt_read_mem(data.msg_proc, sizeof(msg_1), msg_1, mode);
        if(ret < 0)
        {
            printf("atmel_config_crc - mxt_read_mem\n");
			FUNC_END();
            return ret;
        }
		printf("%s:T5 report_id(buf[0]) is 0x%X \n",__func__,msg_1[0]); //may be 0x01 is right
		usleep(10000);
        object_type = mxt_reportid_to_type(msg_1[0] , &instance);

        if(object_type == GEN_COMMANDPROCESSOR_T6)
        {
            crc = msg_1[2] | (msg_1[3] << 8) | (msg_1[4] << 16);
			FUNC_END();
            return crc;
        }
    }

	FUNC_END();
    return false;
}

int atmel_test_about_t5_object_read(int mode, unsigned int *crc) //[int/chg test] [config checksum test]
{

    unsigned char msg_1[data.msg_object_size];
    int object_type = 0;
    unsigned char instance;
	int ret = true;
	int i = 0;
    int int_chg_ret = true;
    int config_crc_ret = false;


	FUNC_BEGIN();
    for(i=0; i<64; i++)
    {
		/* T5 Object Data Read */ 
		ret = mxt_read_mem(data.msg_proc, sizeof(msg_1), msg_1, mode);
		if(ret < 0)
		{
		    printf("atmel_int_chg_test - mxt_read_mem\n");
			int_chg_ret = false;
			config_crc_ret = false;
			printf("%s: result -> INT CHG RET %d / CONFIG CRC RET %d \n",__func__,int_chg_ret,config_crc_ret);
			FUNC_END();
			return (((config_crc_ret << ATMEL_CONFIG_CRC_RET) | (int_chg_ret << ATMEL_INT_CHG_RET)) & 0x03);
		}	
		printf("T5 Report ID : %d \n",msg_1[0]);

		if(msg_1[0] <= data.max_report_id)
		{
			/* for Config CRC Test */
			object_type = mxt_reportid_to_type(msg_1[0] , &instance);

			if(object_type == GEN_COMMANDPROCESSOR_T6)
			{
				config_crc_ret = true;
			    *crc = msg_1[2] | (msg_1[3] << 8) | (msg_1[4] << 16);
				printf("%s:Read Config Checksum 0x%X \n",__func__,*crc);
				printf("%s: result -> INT CHG RET %d / CONFIG CRC RET %d \n",__func__,int_chg_ret,config_crc_ret);
			    FUNC_END();
				return (((config_crc_ret << ATMEL_CONFIG_CRC_RET) | (int_chg_ret << ATMEL_INT_CHG_RET)) & 0x03);
			}

		}
		else if(i == 0)
		{
			/* for INT/CHG Test*/
			int_chg_ret = false;
		}
	}

	/* for Config CRC Test */
	config_crc_ret = false;

	printf("%s: result -> INT CHG RET %d / CONFIG CRC RET %d \n",__func__,int_chg_ret,config_crc_ret);
	FUNC_END();
	return (((config_crc_ret << ATMEL_CONFIG_CRC_RET) | (int_chg_ret << ATMEL_INT_CHG_RET)) & 0x03);

}

int atmel_self_test(int mode, unsigned char cmd)
{
	int ret=0;
	unsigned char enable = 3;
	unsigned char ggg[2];
	unsigned char object_type = 0;
	unsigned char instance;
	unsigned char msg_1[data.msg_object_size];
	int i;

	unsigned char t25_status = 0;

	FUNC_BEGIN();
	ggg[0] = enable;
	ggg[1] = cmd;

	ret = mxt_write_mem(t25_addr, 2, ggg, mode);
    if (ret < 0)
	{
		FUNC_END();
        return ret;
	}

	ret = 0;

	for(i=0; i<64; i++)
	{
		ret = mxt_read_mem(data.msg_proc, sizeof(msg_1), msg_1, mode);
		if(ret < 0)
		{
			printf("SELF TEST FAILED - mxt_read_mem\n");
			FUNC_END();
		    return ret;
		}
		object_type = mxt_reportid_to_type(msg_1[0] , &instance);

		if(object_type == SPT_SELFTEST_T25)
		{
			printf("T25 Status : 0x%02X\n",msg_1[1]);
			t25_status = msg_1[1];
			break;
		}
		usleep(5000);
	}

	printf("SELF_TEST RESULT : ");
    switch (t25_status) 
	{
    	case SELF_TEST_ALL:
    		printf("All tests passed");
    		break;
    	case SELF_TEST_INVALID:
    		printf("Invalid or unsupported test command");
    		break;
    	case SELF_TEST_TIMEOUT:
    		printf("Test timeout");
    		break;
    	case SELF_TEST_ANALOG:
    		printf("AVdd Analog power is not present");
    		break;
    	case SELF_TEST_PIN_FAULT:
    		printf("Pin fault");
    		break;
    	case SELF_TEST_PIN_FAULT_2:
			printf("msg_1[3] = %d , msg_1[4] = %d \n",msg_1[3],msg_1[4]);
    		if (msg_1[3] == 0 && msg_1[4] == 0)
	    		printf("Pin fault SEQ_NUM=%d driven shield line failed",msg_1[2]);
	    	else if (msg_1[3] > 0)
	    		printf("Pin fault SEQ_NUM=%d X%d", msg_1[2], msg_1[3] - 1);
	    	else if (msg_1[4] > 0)
	    		printf("Pin fault SEQ_NUM=%d Y%d", msg_1[2], msg_1[4] - 1);
    		break;
    	case SELF_TEST_AND_GATE:
    		printf("AND Gate Fault");
    		break;
    	case SELF_TEST_SIGNAL_LIMIT:
    		printf("Signal limit fault in T%d[%d]", msg_1[2], msg_1[3]);
    		break;
    	case SELF_TEST_GAIN:
    		printf("Gain error");
    		break;
    }
	printf("\n");
	FUNC_END();
	return t25_status; 
}

int atmel_PDS_inspection(char *pdsData)
{

	int ret = 0;

	FUNC_BEGIN();
	lockdownState = 0;
	configState = 0;

	if(pdsData[0] != 0x42)
	{
		ret = 1;
		printf("pdsData[0] Fail : %X\n",pdsData[0]);
		lockdownState |= (0x01 & 0xff);
	}
	if(pdsData[1] != 0x32)
	{
		ret = 1;
        printf("pdsData[1] Fail : %X\n",pdsData[1]);
		lockdownState |= (0x02 & 0xff);
    }

	if(!isValidCoverGlassInkColor(pdsData[2]))
	{
		ret = 1;
        printf("pdsData[2] Fail : %X\n",pdsData[2]);
		lockdownState |= (0x04 & 0xff);
    }

	if(pdsData[3] != 0x01)
	{
		ret = 1;
        printf("pdsData[3] Fail : %X\n",pdsData[3]);
		lockdownState |= (0x08 & 0xff);
    }

	if(pdsData[4] != 0x0A)
	{
		ret = 1;
        printf("pdsData[4] Fail : %X\n",pdsData[4]);
		lockdownState |= (0x10 & 0xff);
    }

	if(pdsData[5] != 0x04)
	{
		ret = 1;
        printf("pdsData[5] Fail : %X\n",pdsData[5]);
		lockdownState |= (0x20 & 0xff);
    }

	if(!isValidCgMaker(pdsData[6]))
	{
		ret = 1;
        printf("pdsData[6] Fail : %X\n",pdsData[6]);
		lockdownState |= (0x40 & 0xff);
    }

	if(pdsData[8] < 0x02)
	{
		ret = 1;
        printf("pdsData[8] Fail : %X\n",pdsData[8]);
		configState |= (0x01 & 0xff);
    }
	printf("1.lockdownFAILState [0x%X] \n configFAILState [0x%X]\n",lockdownState,configState);
	FUNC_END();
	return !ret;

}

void get_PDSstate(char *lockdown_s, char *config_s)
{
	FUNC_BEGIN();
	printf("lockdownFAILState [0x%X] \n configFAILState [0x%X]\n",lockdownState,configState);
	*lockdown_s = lockdownState;
	*config_s = configState;
	FUNC_END();
}

int atmel_getPDS(int mode, char *ret_str)
{
	char temp[5];
    unsigned char obuf[1000];
    int ret = 1;
    int i;
    unsigned char debug_mode = 0x81;

	memset(obuf, 0, sizeof(obuf));

	char pdsData[16];
	memset(pdsData, 0, sizeof(pdsData));

	int pdsResult = 0;

	FUNC_BEGIN();

	memset(obuf, 0, sizeof(obuf));
	memset(pdsData, 0, sizeof(pdsData));
    ret = mxt_write_mem(data.cmd_proc + CMD_DIAGNOSTIC_OFFSET, 1, &debug_mode, mode);
    if (ret < 0)
	{
		FUNC_END();
        return ret;
	}

	ret = 1;

    usleep(200000);
			
    ret = mxt_read_mem(t37_addr, t37_size, obuf, mode);
    if (ret < 0)
    {
		printf("Get PDS FAILED - mxt_read_mem\n");
		FUNC_END();
		return ret;
    }

	for (i = 4; i < 20; i++)
	{
		pdsData[i - 4] = obuf[i];
		sprintf(temp, "%d ", obuf[i]);
		strcat(ret_str, temp);
		printf("%02X ", obuf[i]);
	}

	printf("\n");

	pdsResult = atmel_PDS_inspection(pdsData);
	FUNC_END();
	return pdsResult;
}

int isValidCoverGlassInkColor(int val)
{
	int idx = 0;
	int colorArr[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40, 0x41};
	int arrSize = sizeof(colorArr) / sizeof(colorArr[0]);

	FUNC_BEGIN();
	for (idx = 0; idx < arrSize; idx++)
	{
		if (colorArr[idx] == val)
		{
			FUNC_END();
			return 1;
		}
	}

	FUNC_END();
	return 0;
}

int isValidCgMaker(int val)
{
	int idx = 0;
	int cgMakerArr[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49};
	int arrSize = sizeof(cgMakerArr) / sizeof(cgMakerArr[0]);

	FUNC_BEGIN();
	for (idx = 0; idx < arrSize; idx++)
	{
		if (cgMakerArr[idx] == val)
		{
			FUNC_END();
			return 1;
		}
	}

	FUNC_END();
	return 0;
}

int isValidDebugVersion(int val)
{
	int idx = 0;

	FUNC_BEGIN();
	for (idx = 0x65; idx <= 0x75; idx++)
	{
		if (idx == val)
		{
			FUNC_END();
			return 1;
		}
	}

	FUNC_END();
	return 0;
}

int setT8(int mode, int isHalfData)
{
	int idx = 0;
	unsigned char buf[15];
    unsigned char obuf[1000];
    int ret = 1;
    int i;

	FUNC_BEGIN();
    memset(obuf, 0, sizeof(obuf));

	if(isHalfData == 0)
	{
		for(idx = 0; idx < 15; idx++)
		{
			buf[idx] = t8Data_org[idx];
		}
	}
	else
	{
        for(idx = 0; idx < 15; idx++)
        {
            buf[idx] = t8Data_halfChg[idx];
        }
	}

	printf("\n============================================\n");

	
	ret = mxt_write_mem(t8_addr, t8_size, buf, mode);
    if (ret < 0)
	{
		FUNC_END();
        return ret;
	}

	ret = 1;
    usleep(20000);

    ret = mxt_read_mem(t8_addr, t8_size, obuf, mode);
    if (ret < 0)
    {
        printf("Get T8 FAILED - mxt_read_mem\n");
		FUNC_END();
        return ret;
    }

    for (i = 0; i < 15; i++)
    {
        printf("%02X ", obuf[i]);
	}

    printf("\n");

	FUNC_END();
	return 1;
}

int getT8(int mode)
{
    unsigned char obuf[1000];
    int ret = 1;
    int i;
    unsigned char debug_mode = 0x00;

	FUNC_BEGIN();
    memset(obuf, 0, sizeof(obuf));

	memset(t8Data_org, 0, sizeof(t8Data_org));
	memset(t8Data_halfChg, 0, sizeof(t8Data_halfChg));

	printf("Micro Defect : Charge rate <%f>  \n",l_md_set_charge);
    ret = mxt_write_mem(t8_addr + t8_size, 1, &debug_mode, mode);
    if (ret < 0)
    {
        printf("Get T8 FAILED - mxt_write_mem\n");
		FUNC_END();
        return ret;
    }
    usleep(20000); // don't touch

    ret = mxt_read_mem(t8_addr, t8_size, obuf, mode);
    if (ret < 0)
    {
        printf("Get T8 FAILED - mxt_read_mem\n");
		FUNC_END();
        return ret;
    }
	printf("change charge [original] > \n");

    for (i = 0; i < 15; i++)
    {
		t8Data_org[i] = obuf[i];
		t8Data_halfChg[i] = obuf[i];

		if (i == 0)
		{
			t8Data_halfChg[i] = (int)FLOAT_ROUND_OFF((float)((float)t8Data_org[i] * l_md_set_charge));
		}

        printf("%02X[%02X] ",t8Data_halfChg[i], t8Data_org[i]);
    }

    printf("\n");
	FUNC_END();
    return 1;
}

int atmel_microdefect(int mode)
{
    short diag_buf[10][64];
    unsigned char obuf[1000];
    int i;
    int j;
    int t37_buf_size = 0;
    int num_page = 0;
    int dia_cnt = 0;
    unsigned char debug_mode = MXT_T6_CMD_REFS;
    int rx_cnt = 0;
	int ret = 1;
	int defectResult = 1;

	// Diagnostic Test Data Table Size For A4 : Row = 30, Col = 18
	// Row[30], Col[18] = For Allocate Avg Data
    // Row[31], Col[19] = For Allocate Percent Data
    int D_MAX_COL = data.info.matrix_ysize + 2;
    int MAX_COL=nodeDetection_MAX[0][1];
	int D_MAX_ROW = data.info.matrix_xsize + 2;
	int MAX_ROW = nodeDetection_MAX[0][2];
    int row, col;

	double defect_buf[D_MAX_ROW][D_MAX_COL];

	FUNC_BEGIN();
	printf("D_MAX_ROW : %d / D_MAX_COL : %d \n",D_MAX_ROW,D_MAX_COL);
	memset(defect_buf, 0, sizeof(defect_buf));
	memset(obuf, 0, sizeof(obuf));
	memset(diag_buf, 0, sizeof(diag_buf));	


//hyelim

	printf("microdefect func in\n");
	if(l_md_set_charge != 1)
	{
		ret = getT8(mode);
		if(!ret)
		{
			printf("getT8 FAIL \n");
			FUNC_END();
			return 0;
		}
		usleep(100000);	
		ret = setT8(mode, 1);
        if(!ret)
        {   
            printf("setT8 FAIL \n");
			FUNC_END();
            return 0;
        }

		usleep(100000);
	}

    {
        t37_buf_size = data.info.matrix_xsize * data.info.matrix_ysize * 2;
        num_page = DIV_ROUND_UP(t37_buf_size, t37_size - 2);
		printf("num_page : %d \n",num_page);

        for (i = 0; i < num_page; i++)
        {
            dia_cnt = 0;
            unsigned char cmd;
            cmd = (i == 0) ? debug_mode : MXT_DIAG_PAGE_UP;
            ret = mxt_write_mem(data.cmd_proc + CMD_DIAGNOSTIC_OFFSET, 1, &cmd, mode);
		    if (ret < 0)
			{
				FUNC_END();
				return ret;
			}

			ret = 1;

            usleep(100000);

            ret = mxt_read_mem(t37_addr, t37_size, obuf, mode);
            if (ret < 0)
            {
                printf("Micro Defect FAILED - mxt_read_mem\n");
				FUNC_END();
                return ret;
            }

            if (obuf[0] == debug_mode )
            {
                for (j = 2; j < t37_size; j += 2)
                {
                    diag_buf[i][dia_cnt] = (short)(obuf[j] + (obuf[j + 1] << 8));
                    dia_cnt++;
                }
            }
        }

		row = 0;
		col = 0;

        for (i = 0; i < num_page; i++)
        {
            for (j = 0; j < (int)((t37_size - 2) / 2); j++)
            {
                if (diag_buf[i][j] != (short)0x8000)
                {
                    defect_buf[row][col] = (double)diag_buf[i][j];
                    col++;
                }

                if (++rx_cnt == data.info.matrix_ysize)
                {
                    col = 0;
                    row++;
                    rx_cnt = 0;
                }
            }
        }

        // Get Avg Exclude 4 border Values by Row and col.
        for (row = 0; row < D_MAX_ROW - 2; row++)
        {
            unsigned int sum = 0;
			if(row < MAX_ROW)
			{
				for (col = 0; col < MAX_COL; col++)
				{
					sum += defect_buf[row][col];
				}
			}

            defect_buf[row][D_MAX_COL - 2] = (sum / MAX_COL);

            if ((row >= 1) && (row < MAX_ROW))
            {
                double prevVal = defect_buf[row - 1][D_MAX_COL - 2];
                double nowVal = defect_buf[row][D_MAX_COL - 2];

                defect_buf[row - 1][D_MAX_COL - 1] = (prevVal - nowVal) / (prevVal + nowVal) * 2;
            }
        }

        for (col = 0; col < (D_MAX_COL - 2); col++)
        {
            unsigned int sum = 0;
			if(col < MAX_COL)
			{
				for (row = 0; row < MAX_ROW; row++)
				{
				    sum += defect_buf[row][col];
				}
			}
            defect_buf[D_MAX_ROW - 2][col] = (sum / MAX_ROW);

            if ((col >= 1) && (col < MAX_COL))
            {
                double prevVal = defect_buf[D_MAX_ROW - 2][col - 1];
                double nowVal = defect_buf[D_MAX_ROW - 2][col];

                defect_buf[D_MAX_ROW - 1][col - 1] = (prevVal - nowVal) / (prevVal + nowVal) * 2;
            }
        }

#ifdef MICRODEFECT_DEBUG
        printf("\n*******************************************************\n");
	    printf("MicroDefect Debug Result [X:%d/ Y:%d] [RdData : X:%d/Y:%d]\n",MAX_COL,MAX_ROW,data.info.matrix_ysize,data.info.matrix_xsize);
        //data.info.matrix_ysize is MAX_COL / data.info.matrix_xsize is MAX_ROW
        printf("*******************************************************\n");
		int k = 0;
        for (row = 0; row < D_MAX_ROW; row++)
        {
            printf("[%02d]  ", row + 1);
            for (col = 0; col < D_MAX_COL; col++)
            {
				if(col == MAX_COL)
					printf(" | ");
				if(col == D_MAX_COL -1)
				{
					if((row == 0) || (row == (MAX_ROW-2)))
					{ 
						if ((defect_buf[row][D_MAX_COL -1] * 100 ) >= (l_md_x_border)) //hyelim
							printf("\033[1;31m%05.2lf\033[m    ", (defect_buf[row][D_MAX_COL -1] * 100));
						else
							printf("\033[1;33m%05.2lf\033[m    ", (defect_buf[row][D_MAX_COL -1] * 100));
					}
					else if(row < (MAX_ROW-2))
					{
                        if ((defect_buf[row][D_MAX_COL -1] * 100 ) >= (l_md_x_center))
							printf("\033[1;31m%05.2lf\033[m    ", (defect_buf[row][D_MAX_COL -1] * 100));
                        else
							printf("\033[1;33m%05.2lf\033[m    ", (defect_buf[row][D_MAX_COL -1] * 100));
					}
					else
						printf("%05.2lf  ", defect_buf[row][D_MAX_COL -1]);

				}
				else if(row == D_MAX_ROW - 1)
				{
                    if((col == 0) || (col == (MAX_COL-2)))
                    { 
                        if ((defect_buf[D_MAX_ROW-1][col] * 100 ) >= (l_md_y_border)) //hyelim
                            printf("\033[1;31m%05.2lf\033[m    ", (defect_buf[D_MAX_ROW-1][col] * 100));
                        else
                            printf("\033[1;33m%05.2lf\033[m    ", (defect_buf[D_MAX_ROW-1][col] * 100));
                    }
                    else if(col < (MAX_COL-2))
                    {
                        if ((defect_buf[D_MAX_ROW-1][col] * 100 ) >= (l_md_y_center))
                            printf("\033[1;31m%05.2lf\033[m    ", (defect_buf[D_MAX_ROW-1][col] * 100));
                        else
                            printf("\033[1;33m%05.2lf\033[m    ", (defect_buf[D_MAX_ROW-1][col] * 100));
					}
					else
						printf("%05.2lf  ", defect_buf[D_MAX_ROW-1][col]);


				}
				else
					printf("%05.2lf  ", defect_buf[row][col]);
            }
			if(row == MAX_ROW-1)
			{
				printf("\n--- -------  ");
				for(k = 0; k < MAX_COL;k++)
				{
				    printf("-------  ");
			    }
		        printf("--- \n");

	        }
			else
				printf("\n");
        }
		
		printf("\n");
#endif
    }

	// Microd(fect
    for (row = 0; row < MAX_ROW-1; row++)
    {
        double value = (defect_buf[row][D_MAX_COL - 1]*100);
        if (value < 0)
        {
            value *= -1;
        }

        if ((row == 0) || (row == MAX_ROW-2)) //border
        {
			//printf("[MAX/%d] %05.2lf : %f \n",row, value, l_md_x_border);
            if (value >= (l_md_x_border))
            {
                //printf("[FAIL] X = %d, Y = %d, Value = %2.2lf, THR = %2.2lf\n", row+1, col+1, value, l_md_x_border);
                printf("[FAIL] X = MAX, Y = %d, Value = %2.2lf, THR = %2.2lf [border]\n", row+1, value, l_md_x_border);
                defectResult = 0;
            }
        }
        else //center
        {
			//printf("[MAX/%d] %05.2lf : %f \n",row, value, l_md_x_center);
            if (value >= (l_md_x_center))
            {
                printf("[FAIL] X = MAX, Y = %d, Value = %2.2lf, THR = %2.2lf [center]\n", row+1, value, l_md_x_center);
                defectResult = 0;
            }
        }
    }

    for (col = 0; col < MAX_COL-1; col++)
    {
        double value = (defect_buf[D_MAX_ROW - 1][col] * 100);
        if (value < 0)
        {
            value *= -1;
        }
        if ((col == 0) || (col == (MAX_COL - 2)))
        {
            if (value > (l_md_y_border))
            {
                printf("[FAIL] X = %d, Y = MAX, Value = %2.2lf, THR = %2.2lf [border]\n", col+1, value, l_md_x_border);
                //printf("[FAIL] X = %d, Y = %d, Value = %2.2lf, THR = %2.2lf\n", row+1, col+1, value, l_md_y_border);
                defectResult = 0;
            }
        }
        else
        {
            if (value > (l_md_y_center))
            {
                printf("[FAIL] X = %d, Y = MAX, Value = %2.2lf, THR = %2.2lf [border]\n", col+1, value, l_md_x_border);
                //printf("[FAIL] X = %d, Y = %d, Value = %2.2lf, THR = %2.2lf\n", row+1, col+1, value, l_md_y_center);
                defectResult = 0;
            }
        }
    }

	if(l_md_set_charge != 1)
	{
		ret = setT8(mode, 0);
		if(!ret)
        {
            printf("setT8 FAIL \n");
			FUNC_END();
            return 0;
        }

	}

	FUNC_END();
    return defectResult;
}



int atmel_diagnostic(int mode, int debug_mode)
{
	short diag_buf[10][64]; //for_debug
	unsigned char obuf[1000];
	int ret, ret2 = 1;
	int i;
	int j,k;
	int t37_buf_size = 0;
	int num_page = 0;
	int dia_cnt = 0;
    int rx_cnt = 0;
	int MAX_ROW = 0,MAX_COL = 0;
	int row = 0, col = 0;
    #ifdef DIAGNOSTIC_DEBUG
    int ch_cnt = 0;
    #endif
	int buf_max[300][300]={{0,},};
	int buf_min[300][300]={{0,},};
	

	FUNC_BEGIN();
	if(!debug_mode)
		debug_mode = MXT_T6_CMD_REFS;

	if(debug_mode == MXT_T6_CMD_REFS)
	{
		printf("reference mode \n");
		memcpy(buf_max,nodeDetection_MAX,sizeof(nodeDetection_MAX));
		memcpy(buf_min,nodeDetection_MIN,sizeof(nodeDetection_MIN));
		MAX_COL = nodeDetection_MAX[0][1];
		MAX_ROW = nodeDetection_MAX[0][2];
	}
	else if(debug_mode == MXT_T6_CMD_DELTAS)
	{
		printf("delta mode \n");
        memcpy(buf_max,delta_MAX,sizeof(delta_MAX));
        memcpy(buf_min,delta_MIN,sizeof(delta_MIN));
        MAX_COL = delta_MAX[0][1];
        MAX_ROW = delta_MAX[0][2];
	}

	memset(obuf, 0, sizeof(obuf));
	memset(diag_buf, 0, sizeof(diag_buf));

	#ifdef LIMIT_DEBUG
	printf("--> diagnostic Limit_data \n");
	for(i = 0; i<MAX_ROW;i++)
	{
		printf("CH_%d : ",i);
		for(j = 0; j<MAX_COL; j++)
		{
			printf("%d(%d/%d), ",j,buf_max[i+1][j+1],buf_min[i+1][j+1]);
		}
		printf("\n");
	}
	#endif

	
	t37_buf_size = data.info.matrix_xsize * data.info.matrix_ysize * 2;
	num_page = DIV_ROUND_UP(t37_buf_size, t37_size - 2);
	printf("num_page : %d \n",num_page);

	for(i=0; i<num_page; i++)
	{
		dia_cnt = 0;
		unsigned char cmd;
		cmd = (i == 0) ? debug_mode : MXT_DIAG_PAGE_UP;
		ret = mxt_write_mem(data.cmd_proc + CMD_DIAGNOSTIC_OFFSET, 1, &cmd, mode);
		if (ret < 0)
		{
			FUNC_END();
	        return ret;
		}
		usleep(20000);

		ret = mxt_read_mem(t37_addr, t37_size, obuf, mode);
        if(ret < 0)
        {
            printf("DIAGNOSTIC FAILED - mxt_read_mem\n");
			FUNC_END();
            return ret;
        }

		if (obuf[0] == debug_mode)
		{
			for(j=2; j<t37_size; j=j+2) //t37_size : data byte size
			{
				diag_buf[i][dia_cnt] = (short)((obuf[j] + (obuf[j+1] << 8))); //diag_buf[pagenum][shortdata]
				dia_cnt++;
			}
		}
	}

#ifdef DIAGNOSTIC_DEBUG
    printf("\n*******************************************************\n");
	if(debug_mode == MXT_T6_CMD_REFS)
		printf("REFERENCE MODE\n");
	else if(debug_mode == MXT_T6_CMD_DELTAS)
		printf("DELTA MODE \n");
    printf("DIAGNOSTIC Debug Result [X:%d/ Y:%d] [RdData : X:%d/Y:%d]\n",MAX_COL,MAX_ROW,data.info.matrix_ysize,data.info.matrix_xsize); 
		//data.info.matrix_ysize is MAX_COL / data.info.matrix_xsize is MAX_ROW
    printf("*******************************************************\n");

    printf("R[0]\t");
    for(i=0; i<num_page; i++)
    {
        for(j=0; j<(int)((t37_size-2)/2); j++) //maybe obuf[0],obuf[1] is data not to use..
        {
		
			if(rx_cnt == MAX_COL)
				printf(" | ");

            if (diag_buf[i][j] == (short)0x8000) 
			{
                printf("----- ");
			}
            else
            {
                if(((buf_min[ch_cnt+1][rx_cnt+1]==-1) && (buf_max[ch_cnt+1][rx_cnt+1]==-1))
					|| ((rx_cnt >= MAX_COL) || (ch_cnt >= MAX_ROW)))  //SKIP
                {
	                printf("%05d ",diag_buf[i][j]);
				}
                else if(diag_buf[i][j] < buf_min[ch_cnt+1][rx_cnt+1]) //MIN FAIL
                {
					printf("\033[0;34m%05d\033[m ",diag_buf[i][j]); //blue
					ret2 = 0;
				}
                else if(diag_buf[i][j] > buf_max[ch_cnt+1][rx_cnt+1]) //MAX FAIL
                {
                    printf("\033[1;31m%05d\033[m ",diag_buf[i][j]); //red
				}
				else
                {
                    printf("%05d ",diag_buf[i][j]); //PASS
                }
            }

            if(++rx_cnt == data.info.matrix_ysize)
            {
	            if(ch_cnt == (MAX_ROW-1))
	            {
	                printf("\n-- ----- ");
	                for(k = 0; k < MAX_COL;k++)
	                {
	                    printf("----- ");
	                }
	                printf("-- \n");
	            }

                rx_cnt = 0;
				if(ch_cnt == data.info.matrix_xsize-1)
					break; //hyelim
                if(++ch_cnt < (int)(((t37_size-2)/4)))
                printf("\nR[%d]\t",ch_cnt);
            }
        }
        if(ch_cnt == data.info.matrix_xsize-1)
            break; //hyelim

    }
    printf("\n");
#endif
	row = 0;
	col = 0;

    for(i=0; i<num_page; i++)
    {
        for(j=0; j<(int)((t37_size-2)/2); j++)
        {
            if(diag_buf[i][j] != (short)0x8000)
            {
				if ((buf_min[row+1][col+1] == -1) &&  (buf_max[row+1][col+1] == -1))
				{
					printf("<DBG>[%02d][%02d] %05d : SKIP\n",row, col, diag_buf[i][j]);
				}
                else if(diag_buf[i][j] < buf_min[row+1][col+1])
				{
					printf("<DBG>[%02d][%02d] %05d < TOO LOW(%05d)\n",row, col, diag_buf[i][j], buf_min[row+1][col+1]);
					ret2 = 0;
				}
                else if(diag_buf[i][j] > buf_max[row+1][col+1])
				{
                    printf("<DBG>[%02d][%02d] %05d > TOO HIGH(%05d)\n",row, col, diag_buf[i][j], buf_max[row+1][col+1]);
                    ret2 = 0;
				}

				col++;
            }
			if (col == MAX_COL)
			{
				col = 0;
				row++;
				if(row == MAX_ROW)
					break; // hyelim
			}
        }
		if(row == MAX_ROW)
			break; // hyelim

    }
	printf("\n");
	FUNC_END();
	return ret2;
}

int atmel_prepare_item(int mode)
{
	unsigned int read_info_crc;
	unsigned int calc_info_crc;
	int ret;
	int	i;

	FUNC_BEGIN();
	read_info_crc = 0;
	calc_info_crc = 0;

	/* Get object table */
    mxt_get_object_table(mode);

	/* Make report id table */
	mxt_make_reportid_table(mode);

	/* Verify the info CRC */
	mxt_read_info_crc(&read_info_crc, mode);
    ret = mxt_calculate_infoblock_crc(&calc_info_crc, mode);
    if (ret)
	{
		printf("Read_Info_CRC ERROR!\n");	
        goto out;
	}

    if (read_info_crc != calc_info_crc) {
        printf("CRC error :[CRC 0x%06X!=0x%06X]\n",read_info_crc, calc_info_crc);
        ret = -EFAULT;
        goto out;
    }
	else
	{
		printf("CRC Match!!\n");
	}

    /* Handle data for init */
    mxt_handle_init_data(mode);

    /* Backup to memory */
    ret = mxt_backup(mode);
    if (ret < 0) 
	{
        printf("Failed backup NV data\n");
    }
	else
	{
		printf("Backup Finished..\n");
	}

    /* Soft reset */
    ret = mxt_reset(mode);
    if (ret) 
	{
        printf("Failed Reset IC\n");
    }
	else
	{
		printf("Reset Finished\n");
	}

	for(i=0; i<1000; i++)
    	usleep(MXT_540S_SW_RESET_TIME);
    mxt_stop(mode);

    printf("Mxt touch controller initialized\n");

    /* Touch report enable */
    mxt_start(mode);
	FUNC_END();
    return 0;
out:
	FUNC_END();
	return ret;
}
	
int atmel_init(int mode)
{
//	unsigned int i2c_address_index = 0;	
	int i ,ret;
	int slave_match = 1;
    int zeroValueCount = 0;

	FUNC_BEGIN();
	ret = mxt_read_mem(0, sizeof(ts_id), ts_id, mode);
	if(ret < 0)
	{
	    printf("i2c Fail\n");
		FUNC_END();
		return I2C_ERR;
	}

    for(i=0; i<sizeof(ts_id); i++)
    {
        if(ts_id[i] == 0)
        {
            zeroValueCount++;
        }
    }

    if (zeroValueCount >= sizeof(ts_id))
    {
        slave_match = 0;
    }

    if(slave_match)
    {
		FUNC_END();
        return 0x4A;//mxt_i2c_address_table[i2c_address_index];
    }
    
	FUNC_END();
    return 0;
}

//void mxt_panel_test(int id,unsigned char *uart_buf, int mode)
int mxt_panel_test(int id,unsigned char *uart_buf, int mode)
{
	int ch_reg = 0;
	unsigned char identity_family = 0;
	unsigned int identity_version = 0;
	unsigned int read_config_crc = 0;
    int result;
	int	eReturn = true;
    unsigned char temp = 0;
	unsigned int    ret_state = 0;

	FUNC_BEGIN();
    printf("\n\n< ATMEL TEST VERSION > \n");
/////////////////////////////////////////////////////////////////////////////////////////// TD01
	printf("\n*******************************************************\n");
    printf("[T01]FIND ADDRESS ...\n");
    printf("*******************************************************\n");
    usleep(300000);
    ch_reg = atmel_init(mode);
	
	if(ch_reg <= 0)
	{
		printf("[ judge : FAIL ]\n");
		if(ch_reg == I2C_ERR)
		{
			uart_buf[4] = 0xFF;
			uart_buf[5] = 0xFF;
			uart_buf[6] = 0xFF;
			uart_buf[7] = 0xFF;
			FUNC_END();
			return I2C_ERR;
		}
		else
			ret_state |= 1<<TOUCH_ATMEL_TD01_FOR_I2C;
		
	}
	else
	{
		printf("[ judge : PASS ]\n");
	}

    printf("\n*******************************************************\n");
    printf("CHIP ID Check ...\n");
    printf("*******************************************************\n");

    identity_family = atmel_get_id(1, 0, mode);

    printf("> Family ID\t: 0x%X\n",identity_family);


    mxt_get_object_table(mode);
    mxt_make_reportid_table(mode);
///need modify
    printf("\n*******************************************************\n");
    printf("[T01] INT/CHG Pin Test \n");
    printf("*******************************************************\n");

	read_config_crc = 0;
    //result = atmel_int_chg_test(mode);
    result = atmel_test_about_t5_object_read(mode,&read_config_crc);
    printf("> INT/CHG PIN?? %d\n",result & (1<<ATMEL_INT_CHG_RET));

	if(!(result&(1<<ATMEL_INT_CHG_RET)))
	{
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_TD01_FOR_INT_CHG;
		eReturn = false;
    }
	else
		printf("[ judge : PASS ]\n");

    printf("\n*******************************************************\n");
    printf("[T03] Confirm Checksum \n");
    printf("*******************************************************\n");

	if(!(result & (1<<ATMEL_CONFIG_CRC_RET)))
	{
		printf("> READ Fail CHECKSUM DATA \n");
        printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_CONFIG_VER;
        eReturn = false;

	}
	else
	{
        printf("> READ CHECKSUM DATA [0x%X] \n",read_config_crc);
        printf(">> [Limit] CONFIG Ver \t:0x%X\n",l_config_ver);

        if(read_config_crc != l_config_ver)
        {
            printf("[ judge : FAIL ]\n");
            ret_state |= 1<<TOUCH_ATMEL_CONFIG_VER;
            eReturn = false;
        }
        else
        {
            printf("[ judge : PASS ]\n");
        }

	}


    printf("\n*******************************************************\n");
    printf("[T01] SELF_TEST - AVDD Test \n");
    printf("*******************************************************\n");

    result = atmel_self_test(mode,SELF_TEST_ANALOG);
    printf("> result = 0x%X \n",result);
    if(result != 0xFE)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_TD01_FOR_AVDD_TEST;
		eReturn = false;
    }
    else
    {
		printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
#if 1
/////////////////////////////////////////////////////////////////////////////////////////// TD02
    printf("\n*******************************************************\n");
    printf("[T02] Confirm Firmware \n");
    printf("*******************************************************\n");
///////////need modify!!!
    temp = atmel_get_id(0, 2, mode);
    identity_version |= ((temp&0xFF) <<8);
    temp = atmel_get_id(0, 3, mode);
    identity_version |= ((temp&0xFF));

    printf("> READ VERSION\t:[0x%X]\n",identity_version);
    printf(">> [Limit] F/W VERSION\t:0x%X\n",l_fw_ver);
	if(identity_version != l_fw_ver)
    {
        printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_FW_VER;
		eReturn = false;
    }	
	else
    {
        printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD03
#endif
#if 0
    printf("\n*******************************************************\n");
    printf("[T03] Confirm Checksum \n");
    printf("*******************************************************\n");
    read_config_crc = 0;

    read_config_crc = atmel_config_crc(mode);
	if(read_config_crc ==false)
    {
		printf("> READ Fail CHECKSUM DATA \n");
        printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_CONFIG_VER;
        eReturn = false;
    }
	else
	{		
		printf("> READ CHECKSUM DATA [0x%X] \n",read_config_crc);
		printf(">> [Limit] CONFIG Ver \t:0x%X\n",l_config_ver);

	    if(read_config_crc != l_config_ver)
	    {
	        printf("[ judge : FAIL ]\n");
			ret_state |= 1<<TOUCH_ATMEL_CONFIG_VER;
			eReturn = false;
	    } 
	    else
	    {
	        printf("[ judge : PASS ]\n");
	    }
	}
#endif
#if 0
//test

	unsigned int	read_info_crc = 0;
	unsigned int	calc_info_crc = 0;

    if(mxt_read_info_crc(&read_info_crc,mode) <= 0)
		printf("FAIL mxt_read_info_crc \n");
	if(mxt_calculate_infoblock_crc(&calc_info_crc,mode) <= 0)
		printf("FAIL mxt_calculate_infoblock_crc \n");

	if(read_info_crc == calc_info_crc)
		printf("PASS R:0x%X/C:0x%X \n",read_info_crc,calc_info_crc);
	else
		printf("FAIL R:0x%X/C:0x%X \n",read_info_crc,calc_info_crc);
	
//test
#endif

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD06


    printf("\n*******************************************************\n");
    printf("[T06] Nodes Detection - DIAGNOSTIC_TEST \n");
    printf("*******************************************************\n");

    result = atmel_diagnostic(mode,MXT_T6_CMD_REFS); //cap check
    if(!result)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_NODE_DETECTION;
		eReturn = false;
    }
    else
    {
		printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD07


    printf("\n*******************************************************\n");
    printf("[T07] Nodes Detection - Delta DIAGNOSTIC_TEST \n");
    printf("*******************************************************\n");

    result = atmel_diagnostic(mode,MXT_T6_CMD_DELTAS); //cap check
    if(!result)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_DELTA_LIMIT;
		eReturn = false;
    }
    else
    {
		printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD08
//////need check

    printf("\n*******************************************************\n");
    printf("[T08] SELF_TEST - SIGNAL LIMIT \n");
    printf("*******************************************************\n");

    result = atmel_self_test(mode,SELF_TEST_SIGNAL_LIMIT);
    printf("> result = 0x%X \n",result);
    if(result != 0xFE)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_SIGNAL_LIMIT;
		eReturn = false;

    }
    else
    {
		printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD17

//////need check
    printf("\n*******************************************************\n");
    printf("[T17] SELF_TEST - Pin Fault Test \n");
    printf("*******************************************************\n");

    result = atmel_self_test(mode,SELF_TEST_PIN_FAULT_2);
    printf("> result = 0x%X \n",result);
    if(result != 0xFE)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_PIN_FAULT;
		eReturn = false;
    }
    else
    {
		printf("[ judge : PASS ]\n");
    }

///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////// TD18

    printf("\n*******************************************************\n");
    printf("[T18] Micro Defect_TEST \n");
    printf("*******************************************************\n");
    result = atmel_microdefect(mode);
    if(!result)
    {
		printf("[ judge : FAIL ]\n");
        ret_state |= 1<<TOUCH_ATMEL_MICRO_DEFECT;
		eReturn = false;
    }
    else
    {
		printf("[ judge : PASS ]\n");
    }
///////////////////////////////////////////////////////////////////////////////////////////

    uart_buf[4] = ret_state & 0xFF;
    uart_buf[5] = (ret_state >> 8) & 0xFF;
    uart_buf[6] = (ret_state >> 16) & 0xFF;
    uart_buf[7] = (ret_state >> 24) & 0xFF;

    int s =0;
    printf("STATE : ");
    for(s = 0; s <4; s++)
        printf("0x%X, ",uart_buf[s+4]);
    printf("/\n");

	FUNC_END();
    return  eReturn;

}

int limit_data_match_v540(int id, struct atmel_touch_limit* limit)
{
    int i =0;

	FUNC_BEGIN();

//////////////// init

    l_fw_ver = 0;
    l_config_ver = 0;
    l_md_set_charge = 0;
    l_md_x_center = 0;
    l_md_y_center = 0;
    l_md_x_border = 0;
    l_md_y_border = 0;

    for(i = 0; i < 300; i++)
    {
        memset(nodeDetection_MAX[i],0,sizeof(nodeDetection_MAX[i]));
        memset(nodeDetection_MIN[i],0,sizeof(nodeDetection_MIN[i]));
        memset(delta_MAX[i],0,sizeof(delta_MAX[i]));
        memset(delta_MIN[i],0,sizeof(delta_MIN[i]));
    }
//////////////////// match
    l_fw_ver = limit->fw_ver;
    l_config_ver = limit->config_ver;
    l_md_set_charge = limit->md_set_charge;
    l_md_x_center = limit->md_x_center;
    l_md_y_center = limit->md_y_center;
    l_md_x_border = limit->md_x_border;
    l_md_y_border = limit->md_y_border;


    memcpy(nodeDetection_MAX,limit->nodeDetection_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(nodeDetection_MIN,limit->nodeDetection_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(delta_MAX,limit->delta_MAX,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..
    memcpy(delta_MIN,limit->delta_MIN,300*300); //[0][1] = XnodeCOUNT, [0][2] = YnodeCOUNT, [1][1]->Data start..

	FUNC_END();
	return 0;

}


void i2c_dev_match_v540(int mode, int i2c_dev)
{
	FUNC_BEGIN();
	if(mode == I2C_MODE_GENERAL)
		mux_dev = i2c_dev;
	else
		dev_gpio = i2c_dev;
    printf("%s : ATMEL API : I2C Device Match OK! \n",__func__);

	FUNC_END();
}

void atmel_close(int mode)
{
	FUNC_BEGIN();
	if(mode == I2C_MODE_GENERAL)
		shutdown(mux_dev, SHUT_RDWR);
	else
		shutdown(dev_gpio, SHUT_RDWR);

	FUNC_END();
}



int atmel_panel_test(unsigned int *result)
{
	int ret = 0;
	unsigned char uart_buf[30] = {0,};
	unsigned int mask = 0xFFFFFFFF;
	
	FUNC_BEGIN();
	ret = mxt_panel_test(0,uart_buf, I2C_MODE_GENERAL);

	*result |= ((uart_buf[4] | (uart_buf[5]<<8) | (uart_buf[6]<<16) | (uart_buf[7]<<24))& mask);

	FUNC_END();
	return	ret;
}

void atmel_init_limit_data(struct atmel_touch_limit* limit)
{
	FUNC_BEGIN();
	limit_data_match_v540(0, limit);
	FUNC_END();
}



bool atmel_init_i2c_set_slvAddr_depending_channel(int ch, int slvAddr, unsigned int *result)
{

	char i2c_dev[30]="/dev/i2c-";
	char i2c_line = 13;
    unsigned long   funcs;

	FUNC_BEGIN();

	printf("CH : %d / Slave Addr : 0x%X \n", ch, slvAddr);

	if(ch == 1)
		i2c_line = 13;
	else
		i2c_line = 9;
		

    sprintf(i2c_dev,"%s%d",i2c_dev,i2c_line);
    printf("OPEN : %s \n",i2c_dev);

    mux_dev = open(i2c_dev, O_RDWR);
    if(mux_dev < 0)
    {
        printf("I2C Device Open Failed..\n");
		*result |= (1 << TOUCH_ATMEL_TD01_FOR_I2C);
		FUNC_END();
        return false;
    }

    if (ioctl(mux_dev, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter "
            "functionality matrix: %s\n", strerror(errno));
        *result |= (1 << TOUCH_ATMEL_TD01_FOR_I2C);
		close(mux_dev);
		mux_dev = 0;
		FUNC_END();
            return false;
    }

    if (ioctl(mux_dev, I2C_SLAVE_FORCE, slvAddr) < 0) {
        fprintf(stderr, "Error: Could not set address[reg:0x%X] \n",slvAddr);
        *result |= (1 << TOUCH_ATMEL_TD01_FOR_I2C);
		close(mux_dev);
		mux_dev = 0;
		FUNC_END();
        return false;
    }

    *result &= ~(1 << TOUCH_ATMEL_TD01_FOR_I2C);

	FUNC_END();
	return true;

}

bool atmel_release_i2c_set(void)
{
	FUNC_BEGIN();

	if (mux_dev > 0)
	{
		printf("CLOSE : i2c device\n");
		close(mux_dev);
		mux_dev = 0;
	}

	FUNC_END();
	return true;
}

void atmel_init_tch_power_set(int on)
{

	int power = 0;

    FUNC_BEGIN();

	power = 0;

    ioctl(dic_dev, _IOCTL_TOUCH_EN_V3_3, &power);
    ioctl(dic_dev, _IOCTL_TOUCH_EN_V1_8, &power);
    ioctl(dic_dev, _IOCTL_1CH_TOUCH_RESET, &power);
    ioctl(dic_dev, _IOCTL_2CH_TOUCH_RESET, &power);

    usleep(1000);

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

    FUNC_END();

}














