#ifndef	__ATMEL_TOUCH_H__
#define	__ATMEL_TOUCH_H__

#include <type.h>
#include <stdbool.h>

#define	I2C_MODE_GENERAL	1
#define	I2C_MODE_GPIO		0

#define TC_SLAVE_ADDR    0x4A

enum {
    TOUCH_ATMEL_TD01_FOR_I2C = 0, //TD01
    TOUCH_ATMEL_TD01_FOR_INT_CHG, //TD01
    TOUCH_ATMEL_TD01_FOR_AVDD_TEST, //TD01
    TOUCH_ATMEL_FW_VER, //TD02
    TOUCH_ATMEL_CONFIG_VER, //TD03
    TOUCH_ATMEL_NODE_DETECTION, //TD06
    TOUCH_ATMEL_DELTA_LIMIT, //TD07
    TOUCH_ATMEL_SIGNAL_LIMIT, //TD08

    TOUCH_ATMEL_PIN_FAULT, //TD17
    TOUCH_ATMEL_MICRO_DEFECT, //TD18
};

int atmel_init(int mode);
void atmel_close(int mode);
unsigned char   atmel_get_id(int first_read, int id, int mode);
void mxt_get_object_table(int mode);
void mxt_make_reportid_table(int mode);
int mxt_read_info_crc(unsigned int *crc_pointer, int mode);
int mxt_calculate_infoblock_crc(unsigned int *crc_pointer, int mode);
int atmel_self_test(int mode, unsigned char cmd);
int mxt_reset(int mode);
int atmel_diagnostic(int mode, int debug_mode);

int touch_iic_mux(int slave_addr);


int isValidCoverGlassInkColor(int val);
int isValidCgMaker(int val);
int isValidDebugVersion(int val);
int atmel_microdefect(int mode);
int atmel_getPDS(int mode,char *ret_str);
void get_PDSstate(char *lockdown_s, char *config_s);
//unsigned int atmel_config_crc(int mode);
long atmel_config_crc(int mode);
unsigned char atmel_check_valid_information_block(int mode);


//made
//int atmel_int_chg_test(int mode);
int atmel_test_about_t5_object_read(int mode, unsigned int *crc);
void i2c_dev_match_v540(int mode, int i2c_dev);
int mxt_panel_test(int id,unsigned char *uart_buf, int mode);

int atmel_touch_limit_parser(int,char *,struct atmel_touch_limit *);
int atmel_touch_limit_table_parser(int, char *, struct atmel_touch_limit *);
///////////////

int atmel_panel_test(unsigned int *);
void atmel_init_limit_data(struct atmel_touch_limit*);

void atmel_init_tch_power_set(int);
bool atmel_init_i2c_set_slvAddr_depending_channel(int, int, unsigned int *);
bool atmel_release_i2c_set();
int limit_data_match_v540(int, struct atmel_touch_limit* );


#endif	/* __ATMEL_TOUCH_H__ */

