#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <type.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <i2c-dev.h>
#include <errno.h>
#include <stdint.h>
#include <siw_touch.h>
#include <model_alpha.h>

int snt_dev;
char i2c_dev[30];
struct siw_touch_limit l_limit;

#if 0
int data_check(unsigned char* buf, int mode, int size, long MODE0_MIN[300][300], long MODE0_MAX[300][300], long MODE1_MIN[300][300], long MODE1_MAX[300][300])
{
	FUNC_BEGIN();
	if(mode == 0)
	{
		if(compare_result(MODE0_MIN, MODE0_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}else{
		if(compare_result(MODE1_MIN, MODE1_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}
	FUNC_END();
	return TRUE;
}
#endif


int open_data_check(unsigned char* buf, int mode, int size)
{
	FUNC_BEGIN();
	if(mode == 0)
	{
		if(compare_result(l_limit.open_test_result1_MIN, l_limit.open_test_result1_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size)){
			return TRUE;
		}else{
			return FALSE;
		}
	}else{
		if(compare_result(l_limit.open_test_result2_MIN, l_limit.open_test_result2_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}
	FUNC_END();
	return TRUE;
}

int short_data_check(unsigned char* buf, int mode, int size)
{
	FUNC_BEGIN();
	if(mode == 0){
		if(compare_result(l_limit.short_test_result1_MIN, l_limit.short_test_result1_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}else{
		if(compare_result(l_limit.short_test_result2_MIN, l_limit.short_test_result2_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}
	FUNC_END();
	return TRUE;
}

int M2Raw_data_check(unsigned char* buf, int mode, int size)
{
	FUNC_BEGIN();
	if(mode == 0){
		if(compare_result(l_limit.m2_raw_mutual_test_MIN, l_limit.m2_raw_mutual_test_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}else{
		//if(compare_result(l_limit.m2_raw_self_test_MIN, l_limit.m2_raw_self_test_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
		if(compare_result(l_limit.m2_raw_self_test_MIN, l_limit.m2_raw_self_test_MAX, PT_CH_SIZE_X, 2, buf, size))
			return TRUE;
		else
			return FALSE;
	}
	FUNC_END();
	return TRUE;
}

int M2Jitter_data_check(unsigned char* buf, int mode, int size)
{
	FUNC_BEGIN();
	if(mode == 0){
		if(compare_result(l_limit.m2_jitter_mutual_test_MIN, l_limit.m2_jitter_mutual_test_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
			return TRUE;
		else
			return FALSE;
	}else{
		//if(compare_result(l_limit.m2_jitter_self_test_MIN, l_limit.m2_jitter_self_test_MAX, PT_CH_SIZE_X, PT_CH_SIZE_y, buf, size))
		if(compare_result(l_limit.m2_jitter_self_test_MIN, l_limit.m2_jitter_self_test_MAX, PT_CH_SIZE_X, 2, buf, size))
			return TRUE;
		else
			return FALSE;
	}
	FUNC_END();
	return TRUE;
}

int m1_data_check(unsigned char* buf, int size)
{
	FUNC_BEGIN();
		if(compare_result(l_limit.m1_raw_test_MIN, l_limit.m1_raw_test_MAX, PT_CH_SIZE_y, 1, buf, size))
			return TRUE;
		else
			return FALSE;
	FUNC_END();
	return TRUE;
}
//190102
int m1Jitter_data_check(unsigned char* buf, int size)
{
    FUNC_BEGIN();
  //  printf("        >>> SSW <<< [%s %d] %s START", __FILE__, __LINE__, __FUNCTION__);
		if(compare_result(l_limit.m1_jitter_test_MIN, l_limit.m1_jitter_test_MAX, PT_CH_SIZE_y, 1, buf, size))
			return TRUE;
		else
			return FALSE;
	FUNC_END();
	return TRUE;
}

int test_log_create(char *fname){
	FILE *out;
	//if ( (out = fopen("/Data/siw_touch_compare_result_raw_data.txt", "a")) == NULL) {
	if ((out = fopen(fname, "w")) == NULL){
		return FALSE;
	}
	return TRUE;
}

int TRANS_RESULT(unsigned char *buf)
{
	unsigned char result_buf[10];
	sprintf(result_buf, "%02x%02x", buf[1], buf[0]);
	return strtoul(result_buf, NULL, 16);
}

int compare_result(long MIN[300][300],long MAX[300][300], int col, int row, unsigned char* buf, int size)
{
	int i,j,size_count = 0;

	FUNC_BEGIN();
	//create_log
	//	FILE *out;
	//	out = fopen(SIW_TOUCH_LOG, "a");

	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++)
		{
			if(size_count >= size)
				goto END;

			unsigned char trans_array[2]={0,0};
			trans_array[0]=(unsigned char) *(buf+size_count);
			trans_array[1]=(unsigned char) *(buf+size_count+1);
			int i2c_result = TRANS_RESULT(trans_array);

			if(DEBUG_MODE){
				if(!((size_count/2)%col))
					printf("\n");
				printf("%04d ",i2c_result);
			}
			//create_log
			//			if(!((size_count/2)%col))
			//				fprintf(out, "\n");
			//			fprintf(out, "%04d ",i2c_result);

			if((MIN[i][j] <= i2c_result && i2c_result <= MAX[i][j])){
             //   printf("        >>> SSW <<< [%s %d] %s CALL ====== %04d < %04d < %04d\n", __FILE__, __LINE__, __FUNCTION__, MIN[i][j] ,i2c_result , MAX[i][j]);
				size_count += 2;
			}else{
				//create_log
				//				fprintf(out, "TEST_FAIL\n");
				//				fclose(out);
				FUNC_END();
				return FALSE;
			}
		}
	}
END:
	//create_log
	//	fprintf(out, " END\n");
	//	fclose(out);
	FUNC_END();
	return TRUE;
}

int init_i2c_set_slvAddr_channel(int ch, int slvAddr)
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
		
    sprintf(i2c_dev, "%s%d", i2c_dev, i2c_line);
    printf("OPEN : %s \n",i2c_dev);

    snt_dev = open(i2c_dev, O_RDWR);
    if(snt_dev < 0)
    {
        printf("I2C Device Open Failed..\n");
		FUNC_END();
		return FALSE;
	}
	if (ioctl(snt_dev, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
				"functionality matrix: %s\n", strerror(errno));
		close(snt_dev);
		snt_dev = 0;
		FUNC_END();
		return FALSE;
	}
	if (ioctl(snt_dev, I2C_SLAVE_FORCE, slvAddr) < 0) {
        fprintf(stderr, "Error: Could not set address[reg:0x%X] \n",slvAddr);
		close(snt_dev);
		snt_dev = 0;
		FUNC_END();
        return FALSE;
    }
	FUNC_END();
	return TRUE;
}

int release_i2c(void)
{
	FUNC_BEGIN();

	if (snt_dev > 0)
	{
		printf("CLOSE : i2c device\n");
		close(snt_dev);
		snt_dev = 0;
	}

	FUNC_END();
	return TRUE;
}

int WriteBurst(UInt16 addr, unsigned char *data, int len)
{
    int ret;
    int i;
    unsigned char i2c_send_buf[255];

    FUNC_BEGIN();
	
	i2c_send_buf[0] = (addr >> 8) & 0xFF;
	i2c_send_buf[1] = (addr >> 0) & 0xFF;
    
    for(i=2; i<len+2; i++){
        i2c_send_buf[i] = data[i-2];
		printf(" - writeBurst [ADDR(%d):0x%02X] \n", i, i2c_send_buf[i]);
	}
    ret = write(snt_dev, i2c_send_buf, len+2);

    memset(i2c_send_buf, 0, len+2);

    if(ret < 0)
    {
        FUNC_END();
        return FALSE;
    }
    FUNC_END();
    return TRUE;
}

int ReadBurst(UInt16 addr, unsigned char *buf, int size)
{
    int rc;
    unsigned char temp_buf[2];
   
	FUNC_BEGIN();

    temp_buf[0] = (addr >> 8) & 0xFF;
    temp_buf[1] = (addr >> 0) & 0xFF;

    rc = write(snt_dev, temp_buf, 2);
	usleep(500);
	rc = read(snt_dev, buf, size);
#if	0	/* debug print */
    printf("\n - ReadBurst_read(%d) [ADDR:0x%X] [SIZE:%d] - ",rc,addr,size);
    int i;
	for(i = 0; i < size; i++)
    {
        if(!(i % 4))
            printf("\n");
        printf("[%d:0x%02X] ",i,*(buf+i));
    }
    printf("\n");
#endif

    if(rc < 0)
    {
        printf("Read Fail.. \n");
        FUNC_END();
        return FALSE;
    }

    FUNC_END();
	return size; // read byte size
}

int	ReadMemory(UInt16 data_reg, UInt16 offset_reg, unsigned char *nWordOffset, unsigned char *pData, int nSize)
{
	int wSize;
    FUNC_BEGIN();
	
	wSize = WriteBurst(offset_reg, nWordOffset, 2);
	if (wSize != TRUE){
		FUNC_END();
		return FALSE;
	}
	wSize = ReadBurst(data_reg, pData, nSize);
    FUNC_END();
	return wSize;
}

int WriteMemory(UInt16 data_reg, UInt16 offset_reg, UInt32 nWordOffset, UInt8 *pData, UInt32 nSize)
{
    int wSize;
    wSize = WriteBurst(offset_reg, nWordOffset, 2);
    if (wSize != TRUE)
        return FALSE;

    wSize = WriteBurst(data_reg, pData, nSize);

    return wSize;
}

void wait_ready()
{
	usleep(200000);
}

void Sleep(UInt32 ms_delay)
{
	usleep(ms_delay);
}

int check_ext_attn(int pin, int ch_num)
{	
	int cmd;
	int err_ret;
    FUNC_BEGIN();

	if(ch_num == 1)
		cmd = _IOCTL_CH1_TE_GET;
	else
		cmd = _IOCTL_CH2_TE_GET;
	
	err_ret = ioctl(dic_dev, cmd, &pin);
	if (err_ret < 0)
	{
		printf("ERR: ioctl-_IOCTL_CHX_TE_GET(ch_num:%d)\n",ch_num);
		FUNC_END();
		return FALSE;
	}
	printf("Attention_test:pin=(%d) ch_num=(%d) cmd=(%d)\n",pin,ch_num,cmd);
    FUNC_END();
	return pin;
}

int ATTN_check(int ch_num)
{
	int i;
	int result = 0;
	
	FUNC_BEGIN();
	
	unsigned char TEST_0[2] = {0x00,0x00};
	WriteBurst(PT_ADDR_SPI_TATTN_OPT, &TEST_0, 2); //ATTN pin Setting
	WriteBurst(SYS_ATTN_OUT, &TEST_0, 2);	// ATTN pin LOW
	Sleep(10000);

    for(i = 0; i<TRY_CNT;i++)
    {
        if(!check_ext_attn(PIN_LOW, ch_num)) // check ATTN of external pin.
        {
            result |= 0x2;
            break;
    	}
        Sleep(10000);
    }

	unsigned char TEST_1[2] = {0x01,0x00};
	WriteBurst(SYS_ATTN_OUT, &TEST_1, 2);	// ATTN pin HIGH
	Sleep(10000);
   
	for(i = 0; i<TRY_CNT;i++)
    {
        if(check_ext_attn(PIN_HIGH, ch_num)) // check ATTN of external pin.
        {
            result |= 0x1;
            break;
    	}
        Sleep(10000);
    }

	FUNC_END();
	if(result == 0x03) 
		return TRUE;		// pass
	else 
		return FALSE;		// fail

}

int connection_check(int ch_num)
{
	unsigned char test_buf[4] = {0};
	unsigned char result_buf[10];
	
	FUNC_BEGIN();
	ReadBurst(CHIP_ID_ADDR_47600, &test_buf, 4);
	sprintf(result_buf, "%02x%02x%02x%02x", test_buf[3],test_buf[2],test_buf[1],test_buf[0]);    
	
	if(atoi(result_buf) != l_limit.chip_id1[0][0])
		return FALSE;
	
	ReadBurst(CHIP_ID_ADDR_47400, &test_buf, 4);
	sprintf(result_buf, "%02x%02x%02x%02x", test_buf[3],test_buf[2],test_buf[1],test_buf[0]);    

	if(atoi(result_buf) != l_limit.chip_id2[0][0])
		return FALSE;
	
	FUNC_END();

	if(ATTN_check(ch_num))
		return TRUE;
	else
		return FALSE;
	
}

int ic_info_check(void)
{
	FUNC_BEGIN();
	t_ic_info ic_info;
	ReadBurst(IC_INFO, &ic_info, 6);
	FUNC_END();
	printf(">>>>>>>>>>>>> %X %X : %X %X\n", ic_info.b.minor_ver, l_limit.fw_version_minor[0][0], ic_info.b.major_ver, l_limit.fw_version_major[0][0]);

	if(ic_info.b.minor_ver == l_limit.fw_version_minor[0][0] && ic_info.b.major_ver == l_limit.fw_version_major[0][0])
        return TRUE; // FW VERSION OK
	else
        return FALSE; // FW VERSION NG
}

int open_test(void)
{
	int try_cnt = 10;
	int nXSize = PT_CH_SIZE_X;
	int nYSize = PT_CH_SIZE_y;
	unsigned char nRead;
	int size = (nXSize * nYSize) * 2;
	unsigned char frameBuf[PT_CH_SIZE_X * PT_CH_SIZE_y * 2]={0,};
	FUNC_BEGIN();

	unsigned char TEST_CMD[2] = {0x01,0x03};     
	WriteBurst(0xC04, &TEST_CMD, 2);
	
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA)	
			break;
		
		wait_ready();
	}while(try_cnt--);

	if(try_cnt <= 0)
		return FALSE; // test fail

	// read pt0
	unsigned char OFFSET_PT0[2] = {0x39, 0x0E};
	if(ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size)
		return FALSE;
	
	if(!open_data_check(frameBuf, 0, size))
		return FALSE;
	// read pt1
	unsigned char OFFSET_PT1[2] = {0xB4, 0x10};
	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT1, frameBuf, size) != size)
		return FALSE;
	
	if(!open_data_check(frameBuf, 1, size))
		return FALSE;

	FUNC_END();
	return TRUE;
}


int short_test(void)
{
	int try_cnt = 10;
	unsigned char nRead;
	int nXSize = PT_CH_SIZE_X;
	int nYSize = PT_CH_SIZE_y;
	int size = (nXSize * nYSize) * 2;
	unsigned char frameBuf[PT_CH_SIZE_X * PT_CH_SIZE_y * 2]={0,};

	FUNC_BEGIN();
	unsigned char TEST_CMD[2]= { 0x02, 0x03 };
	WriteBurst(0xC04, &TEST_CMD, 2);
	
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);

	if(try_cnt <= 0){
		FUNC_END();
		return FALSE; // test fail
	}
	// read pt0
	unsigned char OFFSET_PT0[2] = {0x39, 0x0E};      
	if(ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size){
		FUNC_END();
		return FALSE; // read fail
	}

	if(!short_data_check(frameBuf, 0, size)){
		FUNC_END();
		return FALSE;
	}
	// read pt1
	unsigned char OFFSET_PT1[2] = {0xB4, 0x10};
	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT1, frameBuf, size) != size){
		FUNC_END();
		return FALSE; // read fail
	}

	if(!short_data_check(frameBuf, 1, size)){
		FUNC_END();
		return FALSE;
	}
	FUNC_END();
	return TRUE;
}



int M2_Raw_Mutual_test(void)
{
	int try_cnt = 10;
	unsigned char nRead;
	int nXSize = PT_CH_SIZE_X;
	int nYSize = PT_CH_SIZE_y;
	int size = (nXSize * nYSize) * 2;
	unsigned char frameBuf[PT_CH_SIZE_X * PT_CH_SIZE_y * 2]={0,};

	FUNC_BEGIN();
	unsigned char TEST_CMD[2] = {0x05,0x03}; 
	WriteBurst(0xC04, &TEST_CMD, 2);
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);

	if(try_cnt <= 0) return FALSE; // test fail

	// read mutual data
	unsigned char OFFSET_PT0[2] = {0x39, 0x0E};
	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size)
		return FALSE; // read fail
	
	if(!M2Raw_data_check(frameBuf, 0, size))
		return FALSE;
	
	FUNC_END();
	return TRUE;
}


int M2_Raw_Self_test()
{
	int try_cnt = 10;
	unsigned char nRead;
	int nXSize = PT_CH_SIZE_X;
	int nYSize = PT_CH_SIZE_y;
	int size = (nXSize + nYSize) * 2;
	unsigned char frameBuf[PT_CH_SIZE_X * PT_CH_SIZE_y * 2]={0,};

	FUNC_BEGIN();
    unsigned char TEST_CMD[2] = {0x05, 0x03}; 
	WriteBurst(0xC04, &TEST_CMD, 2);             
	
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);

	if(try_cnt <= 0) return FALSE; // test fail

	// read self data
	 unsigned char OFFSET_PT0[2] = {0x91, 0x10};  
	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size)
		return FALSE; // read fail

	if(!M2Raw_data_check(frameBuf, 1, size))
		return FALSE;
	
	FUNC_END();
	return TRUE;
}

int M2_Jitter_test(void)
{
	int try_cnt = 10;
	unsigned char nRead;
	int nXSize = PT_CH_SIZE_X;
	int nYSize = PT_CH_SIZE_y;
	int mutual_size = (nXSize * nYSize) * 2;
	int self_size = (nXSize + nYSize) * 2;
	unsigned char frameBuf[PT_CH_SIZE_X * PT_CH_SIZE_y * 2]={0,};

	FUNC_BEGIN();
 //   unsigned char TEST_CMD[2] = {0x13,0x03};    
    unsigned char TEST_CMD[2] = {0xD,0x03};    
	WriteBurst(0xC04, &TEST_CMD, 2);      
	
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);

	if(try_cnt <= 0) return 0; // test fail

	// read mutual data
	unsigned char OFFSET_PT0[2] = {0x39, 0x0E};       
	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, mutual_size) != mutual_size) {
		return 0; // read fail
	}

	if(!M2Jitter_data_check(frameBuf, 0, mutual_size)){    //data check for mutual
		return 0;
	}
	
	// read self data
//	unsigned char OFFSET_PT1[2] = {0x91, 0x10};
//	if (ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT1, frameBuf, self_size) != self_size) {
//		return 0; // read fail
//	}

//	if(!M2Jitter_data_check(frameBuf, 1, self_size )){ // data check for self
//		return 0;
//	}
	
	FUNC_END();
	return 1;
}
//m1_raw_mutual
int M1_Raw_test(void)
{
	int try_cnt = 10;
	int size = PT_CH_SIZE_y * 2;
	unsigned char nRead;
	unsigned char frameBuf[PT_CH_SIZE_y * 2]={0,};
	
	FUNC_BEGIN();
	unsigned char TEST_CMD[2] = {0x03, 0x03};
	WriteBurst(0xC04, &TEST_CMD, 2); 
	

    Sleep(50); //wait for 50ms
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);
	if(try_cnt <= 0) return FALSE; // test fail

	unsigned char OFFSET_PT0[2] = {0x39,0x0E};
	if(ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size) {
		return FALSE;
	}

    if(!m1_data_check(frameBuf, size)){
        return FALSE;
	}
	FUNC_END();
	return TRUE;
}
//190108 this test naming is mismatch -> "M2_Jitter_self" 
int M1_Jitter_test(void)
{
	int try_cnt = 10;
	int size = PT_CH_SIZE_y * 2;
	unsigned char nRead;
	unsigned char frameBuf[PT_CH_SIZE_y * 2]={0,};
	
	FUNC_BEGIN();
	unsigned char TEST_CMD[2] = {0xD, 0x03};
	WriteBurst(0xC04, &TEST_CMD, 2); 
	

    Sleep(50); //wait for 50ms
	do{
		ReadBurst(PT_ADDR_PT_TEST_STS, &nRead, 1);
		if(nRead == 0xAA) break;

		wait_ready();
	}while(try_cnt--);
	if(try_cnt <= 0) return FALSE; // test fail

	unsigned char OFFSET_PT0[2] = {0x91,0x10};
	if(ReadMemory(DATA_I2CBASE_ADDR, SERIAL_DATA_OFFSET, &OFFSET_PT0, frameBuf, size) != size) {
		return FALSE;
	}

    if(!m1Jitter_data_check(frameBuf, size)){
        return FALSE;
	}
	FUNC_END();
	return TRUE;
}



int product_id_check(void)
{
	int try_cnt = 10;
	unsigned char nRead;
	unsigned char read_buf[20]={0};

	FUNC_BEGIN();

	unsigned char TEST_CMD[2] = {0x39, 0x0E};
	WriteBurst(SERIAL_DATA_OFFSET, &TEST_CMD, 2);
	ReadBurst(DATA_I2CBASE_ADDR, &read_buf, 20);
#if 0
	if(!strncmp(LGD_PRODUCT_ID, read_buf, 6))
		return TRUE;

	return FALSE;
#endif


	printf(">>>>>>>>>>>>> %X %X %X %X %X %X %X %X %X %X\n", read_buf[0], read_buf[1], read_buf[2], read_buf[3], read_buf[4], read_buf[5]  , read_buf[6], read_buf[7], read_buf[8], read_buf[9]);

	printf(">>>>>>>>>>>>> %X %X %X %X %X %X %X %X %X %X\n", read_buf[10], read_buf[11], read_buf[12], read_buf[13], read_buf[14], read_buf[15]  , read_buf[16], read_buf[17], read_buf[18], read_buf[19]);
	printf(">>>>>>>>>>>>> %X %X %X %X %X %X\n", l_limit.lg_product_id[0], l_limit.lg_product_id[1],l_limit.lg_product_id[2],l_limit.lg_product_id[3], l_limit.lg_product_id[4],l_limit.lg_product_id[5]);
	if(!strncmp(read_buf, l_limit.lg_product_id, 6)){
    	return TRUE; // FW VERSION OK
	}else{
		return FALSE;
    }
	return TRUE;
}

int product_id_write(void)
{
	int try_cnt = 10;
	unsigned char nRead;
	unsigned char read_buf[16]={0x4C,0x30,0x57,0x36,0x31,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
    //unsigned char read_buf[6]={0x4C,0x30,0x57,0x36,0x31,0x41};
    //unsigned char read_buf[16]={0x01,0x02,0x03,0x04,0x05,0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,0x16};
	//unsigned char read_buf[6]={0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

	FUNC_BEGIN();

    sleep(2);
	unsigned char TEST_CMD[2] = {0xB4, 0x10};
	WriteBurst(SERIAL_DATA_OFFSET, &TEST_CMD, 2);
    sleep(2);
	WriteBurst(DATA_I2CBASE_ADDR_FW, &read_buf, 16);//write ram
    sleep(2);
	unsigned char TEST_CMD2[2] = {0x32, 0x00};
	WriteBurst(SAVE_ADDR, &TEST_CMD2, 2);//write rom
    sleep(2);
	
    return TRUE;
}

void init_limit_data_test(struct siw_touch_limit *limit)
{
	l_limit = *limit;
}



int touch_limit_table_parser(MODEL id, char *m_name, struct siw_touch_limit* limit)
{
	char string[500];
	FILE *fp;
	char *token = NULL;
	char file_name[50];
	char name_buf[30]={0,};
	int row = 0;
	int parsing_en = 0;
	int x = 0, y =0;
	int mode = 0;

	FUNC_BEGIN();

	sprintf(file_name, "%s%s", CONFIG_DIR,T_LIMIT_TABLE_FILE);
#if 0
    if(id != limit->id)     
    {     
        printf("%s : model id FAIL [%d/%d] \n",__func__,id, limit->id);     
        printf("%s : plz, excute touch_table_parser func first\n",__func__);     
        FUNC_END();     
        return FAIL;     
    } 
#endif
	if(id != limit->id)
		limit->id = id;

	if((fp=(fopen(file_name,"r"))) == 0 ){
		printf("%s : cannot open %s\n", __func__, file_name);
		FUNC_END();
		return -1;
	}

	sprintf(name_buf,"[%s]",m_name);
	printf("%s : %s searching... \n",__func__,name_buf);

	while((fgets(string, 500, fp)) != NULL){
		token = strtok(string, TOKEN_SEP_COMMA);
		while(token != NULL){
			if(!parsing_en)
			{
				if(!strcmp(token,name_buf))
				{
					printf("Found LIMIT DATA --> %s \n",name_buf);
					printf("%s : %s limit parser START.. \n",__func__,name_buf);
					parsing_en = 1;
				}
				break;
			}
			else
			{
				if(!strcmp(token,"END"))
				{
					parsing_en = 0;
					goto end;
				}

				if(!strcmp(token, "X"))
				{
					token = strtok(NULL, TOKEN_SEP_COMMA);
					x = strtoul(token, NULL,10);
					if(!strcmp(strtok(NULL, TOKEN_SEP_COMMA),"Y"))
					{
						token = strtok(NULL, TOKEN_SEP_COMMA);
						y = strtoul(token, NULL,10);
					}
				}

				switch(mode)
				{	
					case 0:
						if(!strcmp(token, "chip_id1"))
							mode = 1;
						else if(!strcmp(token, "chip_id2"))
							mode = 2;
						else if(!strcmp(token, "fw_version_major"))
							mode = 3;
						else if(!strcmp(token, "fw_version_minor"))
							mode = 4;
						else if(!strcmp(token, "open_test_result1_MIN"))
							mode = 5;
						else if(!strcmp(token, "open_test_result1_MAX"))
							mode = 6;	
						else if(!strcmp(token, "open_test_result2_MIN"))
							mode = 7;
						else if(!strcmp(token, "open_test_result2_MAX"))
							mode = 8;
						else if(!strcmp(token, "short_test_result1_MIN"))
							mode = 9;
						else if(!strcmp(token, "short_test_result1_MAX"))
							mode = 10;	
						else if(!strcmp(token, "short_test_result2_MIN"))
							mode = 11;	
						else if(!strcmp(token, "short_test_result2_MAX"))
							mode = 12;	
						else if(!strcmp(token, "m2_raw_mutual_test_MIN"))
							mode = 13;	
						else if(!strcmp(token, "m2_raw_mutual_test_MAX"))
							mode = 14;	
						else if(!strcmp(token, "m2_raw_self_test_MIN"))
							mode = 15;	
						else if(!strcmp(token, "m2_raw_self_test_MAX"))
							mode = 16;	
						else if(!strcmp(token, "m2_jitter_mutual_test_MIN"))
							mode = 17;
						else if(!strcmp(token, "m2_jitter_mutual_test_MAX"))
							mode = 18;
						else if(!strcmp(token, "m2_jitter_self_test_MIN"))
							mode = 19;	
						else if(!strcmp(token, "m2_jitter_self_test_MAX"))
							mode = 20;	
						else if(!strcmp(token, "m1_raw_test_MIN"))
                            mode = 21;	
                        else if(!strcmp(token, "m1_raw_test_MAX"))
                            mode = 22;
                        else if(!strcmp(token, "m2_jitter_test_MIN"))
                            mode = 23;	
                        else if(!strcmp(token, "m2_jitter_test_MAX"))
                            mode = 24;					
                        else if(!strcmp(token, "lg_product_id"))
                            mode = 25;
                        break;

					case 1:
						if(touch_csv_parser(&x, &y, &row, token, limit->chip_id1))
							mode = 0;
						break;
					case 2:
						if(touch_csv_parser(&x, &y, &row, token, limit->chip_id2))
							mode = 0; 
						break;
					case 3:
						if(touch_csv_parser(&x, &y, &row, token, limit->fw_version_major))
							mode = 0; 
						break;
					case 4:
						if(touch_csv_parser(&x, &y, &row, token, limit->fw_version_minor))
							mode = 0; 
						break;
					case 5:
						if(touch_csv_parser(&x, &y, &row, token, limit->open_test_result1_MIN))
							mode = 0; 
						break;
					case 6:
						if(touch_csv_parser(&x, &y, &row, token, limit->open_test_result1_MAX))
							mode = 0; 
						break;
					case 7:
						if(touch_csv_parser(&x, &y, &row, token, limit->open_test_result2_MIN))
							mode = 0; 
						break;
					case 8:
						if(touch_csv_parser(&x, &y, &row, token, limit->open_test_result2_MAX))
							mode = 0; 
						break;
					case 9:
						if(touch_csv_parser(&x, &y, &row, token, limit->short_test_result1_MIN))
							mode = 0; 
						break;
					case 10:
						if(touch_csv_parser(&x, &y, &row, token, limit->short_test_result1_MAX))
							mode = 0; 
						break;
					case 11:
						if(touch_csv_parser(&x, &y, &row, token, limit->short_test_result2_MIN))
							mode = 0; 
						break;
					case 12:
						if(touch_csv_parser(&x, &y, &row, token, limit->short_test_result2_MAX))
							mode = 0; 
						break;
					case 13:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_raw_mutual_test_MIN))
							mode = 0;
						break;
					case 14:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_raw_mutual_test_MAX))
							mode = 0; 
						break;
					case 15:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_raw_self_test_MIN))
							mode = 0; 
						break;
					case 16:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_raw_self_test_MAX))
							mode = 0; 
						break;
					case 17:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_jitter_mutual_test_MIN))
							mode = 0; 
						break;
					case 18:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_jitter_mutual_test_MAX))
							mode = 0; 
						break;
					case 19:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_jitter_self_test_MIN))
							mode = 0; 
						break;
					case 20:
						if(touch_csv_parser(&x, &y, &row, token, limit->m2_jitter_self_test_MAX))
							mode = 0; 
						break;
					case 21:
						if(touch_csv_parser(&x, &y, &row, token, limit->m1_raw_test_MIN))
							mode = 0; 
						break;
					case 22:
						if(touch_csv_parser(&x, &y, &row, token, limit->m1_raw_test_MAX))
							mode = 0;
						break;
                    case 23://mismatch naming m1 -> m2
						if(touch_csv_parser(&x, &y, &row, token, limit->m1_jitter_test_MIN))
							mode = 0; 
						break;
					case 24://mismatch naming m1 -> m2
						if(touch_csv_parser(&x, &y, &row, token, limit->m1_jitter_test_MAX))
							mode = 0;
						break;
					case 25:
						if(touch_csv_parser2(&x, &y, &row, token, limit->lg_product_id))
							mode = 0;
						break;
				}	
			}
			token = strtok(NULL, TOKEN_SEP_COMMA);
		}
	}

end:
	printf("END \n");
	fclose(fp);
	FUNC_END();
	return 0;
}

int touch_csv_parser(int *xx, int *yy, int *row, char *token, long limit_array[][300])
{
	int i = 0, j =0;
	int n = 0;
	int flag = 0;
	int row_result = 0;
	int x, y;
	x = *xx;
	y = *yy;
	row_result = *row;
	
	if(!strcmp(token,"S"))
	{
		for(n = 0; n < x+1 ; n++) //1 : x
		{
			token = strtok(NULL, TOKEN_SEP_COMMA);
			if(!strcmp(token,"E"))
			{
				row_result++;
				if(row_result > y-1) //2 :y
				{
					row_result = 0;
					flag = 1;
		
					if(DEBUG_MODE){
						for(i = 0; i< y;i++)
						{
							for(j = 0; j<x ;j++)
							{
								//printf(" %0.4f",limit_array[i][j]);
								printf(" %ld",limit_array[i][j]);
							}
							printf("\n");
						}
					}

				}
				break;
			}
			limit_array[row_result][n] = strtod(token,NULL);
		}
	}
	*row = row_result;
	return flag;
}

int touch_csv_parser2(int *xx, int *yy, int *row, char *token, char *limit_array)
{
   // int i = 0, j =0;
   // int n = 0;
   // int row_result = 0;
   // int x, y;
   // x = *xx;
   // y = *yy;
    char test_array[10]={0,};
    char * p = test_array;
    if(!strcmp(token,"S"))
    {
        p = strtok(NULL, TOKEN_SEP_COMMA);
        memcpy(limit_array,p,sizeof(test_array));
    }
    return 1;
}


int touch_csv_parser3(int *xx, int *yy, int *row, char *token, long limit_array[][300])
{
	int i = 0, j =0;
	int n = 0;
	int flag = 0;
	int row_result = 0;
	int x, y;
	x = *xx;
	y = *yy;
	row_result = *row;
	
	if(!strcmp(token,"S"))
	{
		for(n = 0; n < x+1 ; n++) //1 : x
		{
			token = strtok(NULL, TOKEN_SEP_COMMA);
			if(!strcmp(token,"E"))
			{
				row_result++;
				if(row_result > y-1) //2 :y
				{
					row_result = 0;
					flag = 1;
		
					if(DEBUG_MODE){
						for(i = 0; i< y;i++)
						{
							for(j = 0; j<x ;j++)
							{
								//printf(" %0.4f",limit_array[i][j]);
								printf(" %ld",limit_array[i][j]);
							}
							printf("\n");
						}
					}

				}
				break;
			}
			limit_array[row_result][n] = strtok(token,NULL);
			//limit_array[row_result][n] = strtod(token,NULL);
		}
	}
	*row = row_result;
	return flag;
}

