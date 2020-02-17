#ifndef	__CURRENT_H__
#define	__CURRENT_H__

/* Registers */
#define    INA219_REG_CONF      0x00
#define    INA219_REG_SHUNT     0x01 
#define    INA219_REG_BUS       0x02
#define    INA219_REG_POWER     0x03
#define    INA219_REG_CURRENT   0x04
#define    INA219_REG_CAL       0x05

/* Current UART buffer offset define */
#define	CURRENT_COMMAND_CURRENT_OFFSET		2
#define	CURRENT_CHANNEL_NUM_OFFSET			3
#define	CURRENT_VPNL_HIGH_8_BITS_OFFSET		4
#define	CURRENT_VPNL_LOW_8_BITS_OFFSET		5
#define	CURRENT_VDDI_HIGH_8_BITS_OFFSET		6
#define	CURRENT_VDDI_LOW_8_BITS_OFFSET		7
#define	CURRENT_VDDVDH_HIGH_8_BITS_OFFSET	8
#define	CURRENT_VDDVDH_LOW_8_BITS_OFFSET	9
#define	CURRENT_VDDEL_HIGH_8_BITS_OFFSET	10
#define	CURRENT_VDDEL_LOW_8_BITS_OFFSET		11
#define	CURRENT_OCP_VPNL_OFFSET				12
#define	CURRENT_OCP_VDDI_OFFSET				13
#define	CURRENT_OCP_VDDVDH_OFFSET			14
#define	CURRENT_OCP_VDDEL_OFFSET			15
#define	CURRENT_PTN_NUMBER_OFFSET			16
#define	CURRENT_TOTAL_PTN_COUNT_OFFSET		17
#define	CURRENT_TTL_HIGH_8_BITS_OFFSET		19
#define	CURRENT_TTL_LOW_8_BITS_OFFSET		20
#define	CURRENT_OCP_TTL_OFFSET				21

int current_limit_parser(MODEL id, struct current_limit *cl);
int current_func(MODEL id, int model_index, struct current_limit *c_limit, char dir);
unsigned short i2c_slv_addr(int index);
unsigned long Measure_Voltage(void);
unsigned long Measure_Current(void);

#endif	/* __CURRENT_H__ */

