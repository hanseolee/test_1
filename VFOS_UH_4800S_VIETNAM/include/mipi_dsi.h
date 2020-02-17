#ifndef	__MIPI_DSI_H__
#define	__MIPI_DSI_H__

int mipi_write(char PacketType, unsigned char *reg_buf, int len);
void mipi_port_set(int dsi);
void OTP_uart_write(char ch,char ret, char max, unsigned char crc_ret, unsigned char arg1);
void write_pps_0x0A(void);

#endif	/* __MIPI_DSI_H__ */

