
CC=arm-linux-gnueabihf-gcc
CFLAGS = -mfloat-abi=hard -march=armv7 -O2 -D_GNU_SOURCE -Wall -Wno-unused-result -Wunused-but-set-variable -Wswitch

LDFLAGS := 
IFLAGS  := -Iinclude/ -I../../../../Include/
INCPATH := 
LIBPATH := -lpthread

APPS    := vfos2_4800s
#SRCS    := main.c key_manager.c rs485.c mipi_dsi.c display.c current.c stm_touch.c stm_touch_dev.c
SRCS    := main.c key_manager.c rs485.c mipi_dsi.c display.c current.c touch.c stm_touch.c stm_touch_dev_09.c stm_touch_dev_15.c stm_touch_dev_18.c stm_touch_dev_08.c atmel_touch.c atmel_touch_dev.c synaptics_touch.c ./INI_API/*.c model_storm.c model_dp076.c model_aop.c model_common.c model_alpha.c siw_touch.c model_dp086.c atmel_touch_dev_02.c model_f2.c stm_touch_dev_07.c model_dp116.c atmel_touch_dev_03.c synaptics_touch_02.c libehr.a model_dp150.c atmel_touch_dev_04.c synaptics_touch_03.c model_dp173.c atmel_touch_dev_05.c synaptics_touch_04.c

all: $(APPS)

$(APPS): $(SRCS)
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(SRCS) -o $(APPS) $(INCPATH) $(LIBPATH) $(LDFLAGS)
	#./h
	#./z

main.o:	rs485.h type.h display.h current.h stm_touch.h model_storm.h model_alpha.h model_dp086.h model_f2.h model_dp116.h model_dp150.h model_dp173.h
key_manager.o: rs485.h type.h display.h current.h stm_touch.h mipi_con.h
rs485.o: rs485.h
mipi_dsi.o:	mipi_con.h type.h rs485.h
display.o:	type.h rs485.h display.h mipi_con.h
current.o:	type.h rs485.h display.h mipi_con.h
touch.o: stm_touch.h atmel_touch.h type.h
stm_touch.o: type.h	rs485.h fts_lgd_09.h fts_lgd_15.h fts_lgd_18.h fts_lgd_08.h  i2c-dev.h stm_touch.h
stm_touch_dev_09.o:	type.h rs485.h fts_lgd_09.h i2c-dev.h stm_touch.h
stm_touch_dev_15.o:	type.h rs485.h fts_lgd_15.h i2c-dev.h stm_touch.h
stm_touch_dev_18.o:	type.h rs485.h fts_lgd_18.h i2c-dev.h stm_touch.h
stm_touch_dev_08.o:	type.h rs485.h fts_lgd_08.h i2c-dev.h stm_touch.h
stm_touch_dev_07.o:	type.h rs485.h fts_lgd_07.h i2c-dev.h stm_touch.h
atmel_touch.o: type.h rs485.h i2c-dev.h atmel_touch.h i2c-dev.h 
atmel_touch_dev.o: type.h rs485.h i2c-dev.h atmel_touch.h atmel_mxt540s.h
model_storm.o: model_storm.h
model_dp076.o: model_dp076.h
model_aop.o: model_aop.h
model_common.o: model_common.h
synaptics_touch.o: type.h rs485.h i2c-dev.h synaptics_touch.h
model_alpha.o: model_alpha.h
siw_touch.o: siw_touch.h
model_dp086.o: model_dp086.h atmel_touch_dev_02.h
atmel_touch_dev_02.o: type.h rs485.h i2c-dev.h atmel_touch_dev_02.h
model_f2.o: model_f2.h stm_touch_dev_07.h
model_dp116.o: model_dp116.h atmel_touch_dev_03.h
atmel_touch_dev_03.o: type.h rs485.h i2c-dev.h atmel_touch_dev_03.h
synaptics_touch_02.o: type.h rs485.h i2c-dev.h synaptics_touch_02.h touch_comm_lib.h
model_dp150.o: model_dp150.h atmel_touch_dev_04.h	
atmel_touch_dev_04.o: type.h rs485.h i2c-dev.h atmel_touch_dev_04.h
synaptics_touch_03.o: type.h rs485.h i2c-dev.h synaptics_touch_03.h touch_comm_lib.h
model_dp173.o: model_dp173.h atmel_touch_dev_05.h
atmel_touch_dev_05.o: type.h rs485.h i2c-dev.h atmel_touch_dev_05.h
synaptics_touch_04.o: type.h rs485.h i2c-dev.h synaptics_touch_04.h touch_comm_lib.h

clean:
	rm -rf *.o
	rm -rf $(APPS)
