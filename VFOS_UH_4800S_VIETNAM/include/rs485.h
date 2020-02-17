#ifndef	__RS485_H__
#define	__RS485_H__

#define MAX_PACKET      30

int serial_open(void);
int serial_close(void);
int serial_packet_init(unsigned char *buf, unsigned char command, int dis_ch);
int serial_write_function(unsigned char *buf);
int serial_read_function(unsigned char *buf);
int serial_send_command(unsigned char *buf);
int serial_valied_packet(unsigned char *buf);

#endif	/* __RS485_H__ */
