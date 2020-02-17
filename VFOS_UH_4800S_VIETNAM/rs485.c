#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <rs485.h>
#include <type.h>

#define SERIAL_FD            "/dev/ttySAC0"
#define BAUDRATE        B9600


struct termios oldtio,newtio;
int serial_fd;

int serial_open(void){
	FUNC_BEGIN();
    serial_fd = open(SERIAL_FD, O_RDWR | O_NOCTTY );
    if (serial_fd <0) {perror(SERIAL_FD); exit(-1); }

    tcgetattr(serial_fd,&oldtio);

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VMIN]     = 0;
    newtio.c_cc[VTIME]    = 5;

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd,TCSANOW,&newtio);

	FUNC_END();
    return serial_fd;
}
int serial_close(void){
	FUNC_BEGIN();
    close(serial_fd);
	FUNC_END();
    return 1;
}

int serial_packet_init(unsigned char *buf, unsigned char command, int dis_ch){
	FUNC_BEGIN();
	buf[0] = 0xa5;
    buf[1] = 0x00;
    buf[2] = command;
	buf[3] = dis_ch;
    buf[MAX_PACKET-2] = 0xa6;
    buf[MAX_PACKET-1] = 0x0d;

	FUNC_END();
	return 1;
}

int serial_write_function(unsigned char *buf){
    int ret;

	FUNC_BEGIN();

	usleep(100);
    ret = write(serial_fd, buf, MAX_PACKET);
	usleep(16000);

	FUNC_END();
    return ret;
}

int serial_read_function(unsigned char *buf){
	int len=0;

	FUNC_BEGIN();
	len = read(serial_fd, buf, MAX_PACKET);	
	FUNC_END();
	return len;
}

int serial_valied_packet(unsigned char *buf){
	FUNC_BEGIN();
	if(buf[0] != 0xa5)
	{
		FUNC_END();
		return 1;
	}
	if(buf[MAX_PACKET-2] != 0xa6)
	{
		FUNC_END();
		return 1;
	}
	if(buf[MAX_PACKET-1] != 0x0d)
	{
		FUNC_END();
		return 1;
	}

	FUNC_END();
	return 0;
}

int serial_send_command(unsigned char *buf){
	int ret;
	unsigned char *cmd_buf;
	int i;

	FUNC_BEGIN();
	cmd_buf = malloc(MAX_PACKET);

	while(1){
		serial_write_function(buf);
		serial_read_function(cmd_buf);
		ret = serial_valied_packet(cmd_buf);
		if(ret)
			continue;
		else{
			for(i=0; i<MAX_PACKET; i++){
				buf[i] = cmd_buf[i];
			}
			free(cmd_buf);
			FUNC_END();
			return 1;
		}
	}
	free(cmd_buf);
	FUNC_END();
}
