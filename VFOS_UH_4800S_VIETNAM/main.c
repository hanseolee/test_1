#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <rs485.h>
#include <type.h>
#include <display.h>
#include <current.h>
#include <stm_touch.h>
#include <model_storm.h>
#include <model_dp076.h>
#include <model_aop.h>
#include <model_alpha.h>
#include <model_dp086.h>
#include <model_f2.h>
#include <model_dp116.h>
#include <model_dp150.h>
#include <model_dp173.h>
#include <key_manager.h>
#include <atmel_touch.h>
#include <synaptics_touch.h>

int DEBUG_MODE;
char display_version[10]={'1',};
char touch_version[10]={'2',};
char site_version = 0;
int vfos_dev_open(void)
{

    int count = 0;

	FUNC_BEGIN();
	if(!dicOpen)
	    dic_dev = open("/dev/DIC_8890",O_RDWR);

    if(dic_dev < 0)
    {
        printf("DIC Device Open Failed..\n");
        dicOpen = 0;
		dic_dev = 0;
    }
    else
    {
        printf("DIC Device Open\n");
        dicOpen = 1;
        count++;
    }

	serial_open();
    printf("Serial Device Open\n");
    count++;

	FUNC_END();
    return count;

}

int vfos_dev_close(void)
{
    int count = 0;

	FUNC_BEGIN();
    if(dicOpen)
    {
        close(dic_dev);
        dicOpen = 0;
		dic_dev = 0;
        count++;
    }

	serial_close();
    printf("Serial Device Close\n");
    count++;

	FUNC_END();
    return count;
}

int manager_init(void){
	int count = 0;

	FUNC_BEGIN();
///////////////////////////////////////////		JOAN
	Joan.id = JOAN;
	memset(&Joan.limit,0,sizeof(struct Limit));
	//Joan.dir = 'B';
	Joan.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Joan.en = DISABLE;
	if(Joan.en == ENABLE)
		count++;
	printf("%s : JOAN > INIT[%d] buf:%d\n",__func__,Joan.en,Joan.buf_index);


///////////////////////////////////////////		MV
    Mv.id = MV;
    memset(&Mv.limit,0,sizeof(struct Limit));
	//Mv.dir = 'C';
	Mv.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Mv.en = DISABLE;
	if(Mv.en == ENABLE)
		count++;
	printf("%s : MV > INIT[%d] buf:%d\n",__func__,Mv.en,Mv.buf_index);

///////////////////////////////////////////     JOAN_REL
    Joan_r.id = JOAN_REL;
    memset(&Joan_r.limit,0,sizeof(struct Limit));
    //Joan_r.dir = 'D';
    Joan_r.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Joan_r.en = DISABLE;
	if(Joan_r.en == ENABLE)
		count++;
    printf("%s : JOAN_REL > INIT[%d] buf:%d\n",__func__,Joan_r.en,Joan_r.buf_index);


///////////////////////////////////////////     JOAN_MANUAL
    Joan_m.id = JOAN_MANUAL;
    memset(&Joan_m.limit,0,sizeof(struct Limit));
    //Joan_m.dir = 'E';
    Joan_m.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Joan_m.en = DISABLE;
	if(Joan_m.en == ENABLE)
		count++;
    printf("%s : JOAN_MANUAL > INIT[%d] buf:%d\n",__func__,Joan_m.en,Joan_m.buf_index);


///////////////////////////////////////////     MV_MANUAL
    Mv_m.id = MV_MANUAL;
    memset(&Mv_m.limit,0,sizeof(struct Limit));
    //Mv_m.dir = 'F';
    Mv_m.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Mv_m.en = DISABLE;
	if(Mv_m.en == ENABLE)
	    count++;
    printf("%s : MV_MANUAL > INIT[%d] buf:%d\n",__func__,Mv_m.en,Mv_m.buf_index);


///////////////////////////////////////////

///////////////////////////////////////////     A1_AUTO
    a1.id = A1;
    memset(&a1.limit,0,sizeof(struct Limit));
    //a1.dir = 'G';
    a1.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	a1.en = DISABLE;
	if(a1.en == ENABLE)
		count++;
    printf("%s : A1 > INIT[%d] buf:%d\n",__func__,a1.en,a1.buf_index);

///////////////////////////////////////////
///////////////////////////////////////////     JOAN_E5
    JoanE5.id = JOAN_E5;
    memset(&JoanE5.limit,0,sizeof(struct Limit));
    //JoanE5.dir = 'H';
    JoanE5.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	JoanE5.en = DISABLE;
	if(JoanE5.en == ENABLE)
	    count++;
    printf("%s : JOAN_E5 > INIT[%d] buf:%d\n",__func__,JoanE5.en,JoanE5.buf_index);

///////////////////////////////////////////     TAIMEN (MQA) - Vietnam MV(Taimen)
    Mv_mqa.id = MV_MQA;
    memset(&Mv_mqa.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Mv_mqa.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Mv_mqa.en = DISABLE;
    if(Mv_mqa.en == ENABLE)
        count++;
    printf("%s : MV_MQA > INIT[%d] buf:%d\n",__func__,Mv_mqa.en,Mv_mqa.buf_index);
#if 0
///////////////////////////////////////////     TAIMEN (DQA) - Vietnam MV(Taimen)
    Mv_dqa.id = MV_DQA;
    memset(&Mv_dqa.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Mv_dqa.buf_index = count;
	#ifdef VFOS_SITE_VIETNAM
    Mv_dqa.en = ENABLE;
	#endif
	#ifdef VFOS_SITE_GUMI
    Mv_dqa.en = DISABLE; //180104_temp
	#endif
    if(Mv_dqa.en == ENABLE)
        count++;
    printf("%s : MV_DQA > INIT[%d] buf:%d\n",__func__,Mv_dqa.en,Mv_dqa.buf_index);
#endif
	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
///////////////////////////////////////////     DP116
    Dp116.id = DP116;
    memset(&Dp116.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Dp116.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Dp116.en = ENABLE;
    if(Dp116.en == ENABLE)
        count++;
    printf("%s : DP116 > INIT[%d] buf:%d\n",__func__,Dp116.en,Dp116.buf_index);
///////////////////////////////////////////		DP049
    Dp049.id = DP049;
    memset(&Dp049.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Dp049.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Dp049.en = DISABLE;
    if(Dp049.en == ENABLE)
        count++;
    printf("%s : DP049 > INIT[%d] buf:%d\n",__func__,Dp049.en,Dp049.buf_index);

///////////////////////////////////////////////     DP150		// 190923 LWG
    Dp150.id = DP150;
    memset(&Dp150.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Dp150.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Dp150.en = ENABLE;
    if(Dp150.en == ENABLE)
        count++;
    printf("%s : DP150 > INIT[%d] buf:%d\n",__func__,Dp150.en,Dp150.buf_index);

///////////////////////////////////////////////		DP173		// 200129 LWG
    Dp173.id = DP173;
    memset(&Dp173.limit,0,sizeof(struct Limit));
    //Dp049.dir = 'I';
    Dp173.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Dp173.en = ENABLE;
	//Dp173.en = DISABLE;
    if(Dp173.en == ENABLE)
        count++;
    printf("%s : DP173 > INIT[%d] buf:%d\n",__func__,Dp173.en,Dp173.buf_index);

///////////////////////////////////////////     AKATSUKI
    Akatsuki.id = AKATSUKI;
    memset(&Akatsuki.limit,0,sizeof(struct Limit));
    //Akatsuki.dir = 'K';
    Akatsuki.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	Akatsuki.en = DISABLE;
    if(Akatsuki.en == ENABLE)
        count++;
    printf("%s : AKATSUKI > INIT[%d] buf:%d\n",__func__,Akatsuki.en,Akatsuki.buf_index);



///////////////////////////////////////////		B1
    b1.id = B1;
    memset(&b1.limit,0,sizeof(struct Limit));
    //b1.dir = 'J';
    b1.buf_index = count;
	//#ifdef VFOS_SITE_VIETNAM_2
	b1.en = DISABLE;
    if(b1.en == ENABLE)
        count++;
    printf("%s : B1 > INIT[%d] buf:%d\n",__func__,b1.en,b1.buf_index);
///////////////////////////////////////////

///////////////////////////////////////////		STORM
    Storm.id = STORM;
    memset(&Storm.limit,0,sizeof(struct Limit));
    //Storm.dir = 'J';
    Storm.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	Storm.en = DISABLE;
    if(Storm.en == ENABLE)
        count++;
    printf("%s : STORM > INIT[%d] buf:%d\n",__func__,Storm.en,Storm.buf_index);
///////////////////////////////////////////

///////////////////////////////////////////		DP076
    Dp076.id = DP076;
    memset(&Dp076.limit,0,sizeof(struct Limit));
    //Dp076.dir = 'J';
    Dp076.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	Dp076.en = DISABLE;
    if(Dp076.en == ENABLE)
	{
        count++;
	}
    printf("%s : DP076 > INIT[%d] buf:%d\n",__func__,Dp076.en,Dp076.buf_index);
///////////////////////////////////////////

///////////////////////////////////////////		AOP
    Aop.id = AOP;
    memset(&Aop.limit,0,sizeof(struct Limit));
    Aop.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	Aop.en = DISABLE;
    if(Aop.en == ENABLE)
        count++;
	printf("%s : AOP > INIT[%d] buf:%d\n",__func__,Aop.en,Aop.buf_index);
	///////////////////////////////////////////
	
	///////////////////////////////////////////     ALPHA
	Alpha.id = ALPHA;
	memset(&Alpha.limit,0,sizeof(struct Limit));
	//Alpha.dir = 'K';
	Alpha.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	Alpha.en = DISABLE;
	if(Alpha.en == ENABLE)
		count++;
	printf("%s : ALPHA > INIT[%d] buf:%d\n",__func__,Alpha.en,Alpha.buf_index); 
	/////////////////////////////////////////// 	

	///////////////////////////////////////////		DP086
	Dp086.id = DP086;
	memset(&Dp086.limit,0,sizeof(struct Limit));
	//Dp086.dir = 'L';
	Dp086.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	Dp086.en = DISABLE;
	if(Dp086.en == ENABLE)
	{
		count++;
	}
	printf("%s : DP086 > INIT[%d] buf:%d\n",__func__,Dp086.en,Dp086.buf_index);
	///////////////////////////////////////////

#if 1 //F2 DISABLE
	///////////////////////////////////////////		F2
	f2.id = F2;
	memset(&f2.limit,0,sizeof(struct Limit));
	f2.buf_index = count;
	//#if defined(VFOS_SITE_VIETNAM_2)
	f2.en = DISABLE;
	if(f2.en == ENABLE)
	{
		count++;
	}
	printf("%s : F2 > INIT[%d] buf:%d\n",__func__,f2.en,f2.buf_index);
	///////////////////////////////////////////
#endif

	en_model_count = count;
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	printf("TOTAL MODEL NUM : %d \n",en_model_count);
	FUNC_END();
	return count;
}


void limit_parser(void){

	char tmp_name[30] ={0,};

	FUNC_BEGIN();
	if(Joan.en == ENABLE)
	{
		Joan.limit.id = JOAN;
		Joan.dir = 'B'+ Joan.buf_index;
		sprintf(tmp_name,"JOAN");
		strcpy(Joan.name,tmp_name);

		Joan.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Joan.id,Joan.name, Joan.limit.ptouch);
        stm_touch_limit_table_parser(Joan.id,Joan.name, Joan.limit.ptouch);

        current_limit_parser(Joan.id,&Joan.limit.current);
		display_limit_parser(Joan.id,Joan.buf_index,&Joan.limit.display,Joan.dir);

		model_list[Joan.buf_index] = Joan;
		printf("%s : [JOAN] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Joan.limit.id, Joan.buf_index +1, Joan.dir);
	}
    else
        printf("%s : JOAN > NO USE \n",__func__);


    if(Mv.en == ENABLE)
    {
		memset(tmp_name,0,sizeof(tmp_name));
        Mv.limit.id = MV;
		Mv.dir = 'B'+ Mv.buf_index;
        sprintf(tmp_name,"MV");
        strcpy(Mv.name,tmp_name);

        Mv.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Mv.id, Mv.name, Mv.limit.ptouch);
        stm_touch_limit_table_parser(Mv.id, Mv.name, Mv.limit.ptouch);

        current_limit_parser(Mv.id,&Mv.limit.current);
        display_limit_parser(Mv.id,Mv.buf_index,&Mv.limit.display,Mv.dir);

        model_list[Mv.buf_index] = Mv;
        printf("%s : [MV] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Mv.limit.id, Mv.buf_index+1, Mv.dir);
    }
    else
        printf("%s : MV > NO USE \n",__func__);

    if(Joan_r.en == ENABLE)
    {
		memset(tmp_name,0,sizeof(tmp_name));
        Joan_r.limit.id = JOAN_REL;
		Joan_r.dir = 'B'+ Joan_r.buf_index;
		sprintf(tmp_name,"JOAN"); //180221
        strcpy(Joan_r.name,tmp_name);

        Joan_r.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));

        stm_touch_limit_parser(Joan_r.id,Joan_r.name, Joan_r.limit.ptouch);

        stm_touch_limit_table_parser(Joan_r.id,Joan_r.name, Joan_r.limit.ptouch);

        current_limit_parser(Joan_r.id,&Joan_r.limit.current);

        display_limit_parser(Joan_r.id,Joan_r.buf_index,&Joan_r.limit.display,Joan_r.dir);

        model_list[Joan_r.buf_index] = Joan_r;
        printf("%s : [JOAN_REL] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Joan_r.limit.id, Joan_r.buf_index+1, Joan_r.dir);
    }
    else
        printf("%s : JOAN_REL > NO USE \n",__func__);

    if(Joan_m.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Joan_m.limit.id = JOAN_MANUAL;
		Joan_m.dir = 'B'+ Joan_m.buf_index;
        sprintf(tmp_name,"JOAN_MANUAL");
        strcpy(Joan_m.name,tmp_name);


        Joan_m.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Joan_m.id,Joan_m.name, Joan_m.limit.ptouch);
        stm_touch_limit_table_parser(Joan_m.id,Joan_m.name, Joan_m.limit.ptouch);

        current_limit_parser(Joan_m.id,&Joan_m.limit.current);
        display_limit_parser(Joan_m.id,Joan_m.buf_index,&Joan_m.limit.display,Joan_m.dir);

        model_list[Joan_m.buf_index] = Joan_m;
        printf("%s : [JOAN_MANUAL] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Joan_m.limit.id, Joan_m.buf_index+1, Joan_m.dir);
    }
    else
        printf("%s : JOAN_MANUAL > NO USE \n",__func__);


    if(Mv_m.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Mv_m.limit.id = MV_MANUAL;
		Mv_m.dir = 'B'+ Mv_m.buf_index;
        sprintf(tmp_name,"MV_MANUAL");
        strcpy(Mv_m.name,tmp_name);

        Mv_m.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Mv_m.id,Mv_m.name, Mv_m.limit.ptouch);
        stm_touch_limit_table_parser(Mv_m.id,Mv_m.name, Mv_m.limit.ptouch);

        current_limit_parser(Mv_m.id,&Mv_m.limit.current);
        display_limit_parser(Mv_m.id,Mv_m.buf_index,&Mv_m.limit.display,Mv_m.dir);

        model_list[Mv_m.buf_index] = Mv_m;
        printf("%s : [MV_MANUAL] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Mv_m.limit.id, Mv_m.buf_index+1, Mv_m.dir);
    }
    else
        printf("%s : MV_MANUAL > NO USE \n",__func__);

    if(a1.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        a1.limit.id = A1;
		a1.dir = 'B'+ a1.buf_index;
        sprintf(tmp_name,"A1");
        strcpy(a1.name,tmp_name);

        a1.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(a1.id,a1.name, a1.limit.ptouch);
        stm_touch_limit_table_parser(a1.id,a1.name, a1.limit.ptouch);

        current_limit_parser(a1.id,&a1.limit.current);
        display_limit_parser(a1.id,a1.buf_index,&a1.limit.display,a1.dir);

        model_list[a1.buf_index] = a1;
        printf("%s : [A1] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,a1.limit.id, a1.buf_index+1, a1.dir);
    }
    else
        printf("%s : A1 > NO USE \n",__func__);

    if(JoanE5.en == ENABLE)
    {
		memset(tmp_name,0,sizeof(tmp_name));
        JoanE5.limit.id = JOAN;
		JoanE5.dir = 'B'+ JoanE5.buf_index;
        sprintf(tmp_name,"JOAN_E5");
        strcpy(JoanE5.name,tmp_name);

        JoanE5.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(JoanE5.id,JoanE5.name, JoanE5.limit.ptouch);
        stm_touch_limit_table_parser(JoanE5.id,JoanE5.name, JoanE5.limit.ptouch);

        current_limit_parser(JoanE5.id,&JoanE5.limit.current);
        display_limit_parser(JoanE5.id,JoanE5.buf_index,&JoanE5.limit.display,JoanE5.dir);

        model_list[JoanE5.buf_index] = JoanE5;
        printf("%s : [JOAN_E5] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,JoanE5.limit.id, JoanE5.buf_index+1, JoanE5.dir);
    }
    else
        printf("%s : JOAN_E5 > NO USE \n",__func__);


    if(Mv_mqa.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Mv_mqa.limit.id = MV_MQA;
        Mv_mqa.dir = 'B'+ Mv_mqa.buf_index;
        sprintf(tmp_name,"MV_MQA");
        strcpy(Mv_mqa.name,tmp_name);

        Mv_mqa.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Mv_mqa.id,Mv_mqa.name, Mv_mqa.limit.ptouch);
        stm_touch_limit_table_parser(Mv_mqa.id,Mv_mqa.name, Mv_mqa.limit.ptouch);

        current_limit_parser(Mv_mqa.id,&Mv_mqa.limit.current);
        display_limit_parser(Mv_mqa.id,Mv_mqa.buf_index,&Mv_mqa.limit.display,Mv_mqa.dir);

        model_list[Mv_mqa.buf_index] = Mv_mqa;
        printf("%s : [MV_MQA] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Mv_mqa.limit.id, Mv_mqa.buf_index+1, Mv_mqa.dir);
    }
    else
        printf("%s : MV_MQA > NO USE \n",__func__);
	if(Dp116.en == ENABLE)
	{
		memset(tmp_name,0,sizeof(tmp_name));
        Dp116.limit.id = DP116;
        Dp116.dir = 'B'+ Dp116.buf_index;
        sprintf(tmp_name,"DP116");
        strcpy(Dp116.name,tmp_name);

        Dp116.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Dp116.id,Dp116.name, Dp116.limit.ptouch);
        stm_touch_limit_table_parser(Dp116.id,Dp116.name, Dp116.limit.ptouch);

        current_limit_parser(Dp116.id,&Dp116.limit.current);
        display_limit_parser(Dp116.id,Dp116.buf_index,&Dp116.limit.display,Dp116.dir);

        model_list[Dp116.buf_index] = Dp116;
        printf("%s : [DP116] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp116.limit.id, Dp116.buf_index+1, Dp116.dir);
	}
	else
		printf("%s : DP116 > NO USE \n",__func__);
    if(Mv_dqa.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Mv_dqa.limit.id = MV_DQA;
        Mv_dqa.dir = 'B'+ Mv_dqa.buf_index;
        sprintf(tmp_name,"MV_DQA");
        strcpy(Mv_dqa.name,tmp_name);

        Mv_dqa.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Mv_dqa.id,Mv_dqa.name, Mv_dqa.limit.ptouch);
        stm_touch_limit_table_parser(Mv_dqa.id,Mv_dqa.name, Mv_dqa.limit.ptouch);

        current_limit_parser(Mv_dqa.id,&Mv_dqa.limit.current);
        display_limit_parser(Mv_dqa.id,Mv_dqa.buf_index,&Mv_dqa.limit.display,Mv_dqa.dir);

        model_list[Mv_dqa.buf_index] = Mv_dqa;
        printf("%s : [MV_DQA] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Mv_dqa.limit.id, Mv_dqa.buf_index+1, Mv_dqa.dir);
    }
    else
        printf("%s : MV_DQA > NO USE \n",__func__);


    if(Dp049.en == ENABLE)
    {
		memset(tmp_name,0,sizeof(tmp_name));
        Dp049.limit.id = DP049;
		Dp049.dir = 'B'+ Dp049.buf_index;
        sprintf(tmp_name,"DP049");
        strcpy(Dp049.name,tmp_name);

        Dp049.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(Dp049.id,Dp049.name, Dp049.limit.ptouch);
        stm_touch_limit_table_parser(Dp049.id,Dp049.name, Dp049.limit.ptouch);

        current_limit_parser(Dp049.id,&Dp049.limit.current);
        display_limit_parser(Dp049.id,Dp049.buf_index,&Dp049.limit.display,Dp049.dir);

        model_list[Dp049.buf_index] = Dp049;
        printf("%s : [DP049] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp049.limit.id, Dp049.buf_index+1, Dp049.dir);
    }
    else
        printf("%s : DP049 > NO USE \n",__func__);

// 190924 LWG : sequence is important
#ifdef VFOS_SITE_VIETNAM
     if(Dp150.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp150.limit.id = DP150;
        Dp150.dir = 'B'+ Dp150.buf_index;
        sprintf(tmp_name,"DP150");
        strcpy(Dp150.name,tmp_name);

        Dp150.limit.ptouch = (struct atmel_03_touch_limit*)malloc(sizeof(struct atmel_03_touch_limit));
       // synaptics_touch_limit_parser(Dp150.id,Dp150.name, Dp150.limit.ptouch);
        atmel_03_touch_limit_table_parser(Dp150.id, Dp150.name, Dp150.limit.ptouch);

        current_limit_parser(Dp150.id,&Dp150.limit.current);
        display_limit_parser(Dp150.id,Dp150.buf_index,&Dp150.limit.display,Dp150.dir);

        model_list[Dp150.buf_index] = Dp150;
        printf("%s : [DP150] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp150.limit.id, Dp150.buf_index+1, Dp150.dir);
    }
    else
        printf("%s : DP150 > NO USE \n",__func__);
#endif

    if(Akatsuki.en == ENABLE)
    {
		memset(tmp_name,0,sizeof(tmp_name));
        Akatsuki.limit.id = AKATSUKI;
		Akatsuki.dir = 'B'+ Akatsuki.buf_index;
        sprintf(tmp_name,"AKATSUKI");
        strcpy(Akatsuki.name,tmp_name);

        Akatsuki.limit.ptouch = (struct atmel_touch_limit*)malloc(sizeof(struct atmel_touch_limit));
        atmel_touch_limit_parser(Akatsuki.id,Akatsuki.name, Akatsuki.limit.ptouch);
        atmel_touch_limit_table_parser(Akatsuki.id,Akatsuki.name, Akatsuki.limit.ptouch);

        current_limit_parser(Akatsuki.id,&Akatsuki.limit.current);
        display_limit_parser(Akatsuki.id,Akatsuki.buf_index,&Akatsuki.limit.display,Akatsuki.dir);

        model_list[Akatsuki.buf_index] = Akatsuki;
        printf("%s : [AKATSUKI] > ENABLE STATE, LIMIT INIT.. \n",__func__);
		printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Akatsuki.limit.id, Akatsuki.buf_index+1, Akatsuki.dir);
    }
    else
        printf("%s : AKATSUKI > NO USE \n",__func__);


    if(b1.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        b1.limit.id = B1;
        b1.dir = 'B'+ b1.buf_index;
        sprintf(tmp_name,"B1");
        strcpy(b1.name,tmp_name);

        b1.limit.ptouch = (struct stm_touch_limit*)malloc(sizeof(struct stm_touch_limit));
        stm_touch_limit_parser(b1.id,b1.name, b1.limit.ptouch);
        stm_touch_limit_table_parser(b1.id,b1.name, b1.limit.ptouch);

        current_limit_parser(b1.id,&b1.limit.current);
        display_limit_parser(b1.id,b1.buf_index,&b1.limit.display,b1.dir);

        model_list[b1.buf_index] = b1;
        printf("%s : [B1] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,b1.limit.id, b1.buf_index+1, b1.dir);
    }
    else
        printf("%s : B1 > NO USE \n",__func__);

    if(Storm.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Storm.limit.id = STORM;
        Storm.dir = 'B'+ Storm.buf_index;
        sprintf(tmp_name,"STORM");
        strcpy(Storm.name,tmp_name);

        Storm.limit.ptouch = (struct synaptics_touch_limit*)malloc(sizeof(struct synaptics_touch_limit));
        synaptics_touch_limit_parser(Storm.id,Storm.name, Storm.limit.ptouch);
        synaptics_touch_limit_table_parser(Storm.id,Storm.name, Storm.limit.ptouch);

        current_limit_parser(Storm.id,&Storm.limit.current);
        display_limit_parser(Storm.id,Storm.buf_index,&Storm.limit.display,Storm.dir);

        model_list[Storm.buf_index] = Storm;
        printf("%s : [STORM] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Storm.limit.id, Storm.buf_index+1, Storm.dir);
    }
    else
        printf("%s : STORM > NO USE \n",__func__);

    if(Dp076.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp076.limit.id = DP076;
        Dp076.dir = 'B'+ Dp076.buf_index;
        sprintf(tmp_name,"DP076");
        strcpy(Dp076.name,tmp_name);

        Dp076.limit.ptouch = (struct synaptics_touch_limit*)malloc(sizeof(struct synaptics_touch_limit));
        synaptics_touch_limit_parser(Dp076.id,Dp076.name, Dp076.limit.ptouch);
        synaptics_touch_limit_table_parser(Dp076.id,Dp076.name, Dp076.limit.ptouch);

        current_limit_parser(Dp076.id,&Dp076.limit.current);
        display_limit_parser(Dp076.id,Dp076.buf_index,&Dp076.limit.display,Dp076.dir);

        model_list[Dp076.buf_index] = Dp076;
        printf("%s : [DP076] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp076.limit.id, Dp076.buf_index+1, Dp076.dir);
    }
    else
        printf("%s : DP076 > NO USE \n",__func__);

    if(Aop.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Aop.limit.id = AOP;
        Aop.dir = 'B'+ Aop.buf_index;
        sprintf(tmp_name,"AOP");
        strcpy(Aop.name,tmp_name);

        Aop.limit.ptouch = (struct atmel_touch_limit*)malloc(sizeof(struct atmel_touch_limit));
        atmel_touch_limit_parser(Aop.id,Aop.name, Aop.limit.ptouch);
        atmel_touch_limit_table_parser(Aop.id,Aop.name, Aop.limit.ptouch);

        current_limit_parser(Aop.id,&Aop.limit.current);
        display_limit_parser(Aop.id,Aop.buf_index,&Aop.limit.display,Aop.dir);

        model_list[Aop.buf_index] = Aop;
        printf("%s : [AOP] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Aop.limit.id, Aop.buf_index+1, Aop.dir);
    }
    else
        printf("%s : AOP > NO USE \n",__func__);

	if(Alpha.en == ENABLE)
	{
		memset(tmp_name,0,sizeof(tmp_name));
		Alpha.limit.id = ALPHA;
		Alpha.dir = 'B'+ Alpha.buf_index;
		sprintf(tmp_name,"ALPHA");
		strcpy(Alpha.name,tmp_name);

		Alpha.limit.ptouch = (struct siw_touch_limit*)malloc(sizeof(struct siw_touch_limit));
		touch_limit_table_parser(Alpha.id, Alpha.name, Alpha.limit.ptouch);

		current_limit_parser(Alpha.id,&Alpha.limit.current);
		display_limit_parser(Alpha.id,Alpha.buf_index,&Alpha.limit.display,Alpha.dir);

        model_list[Alpha.buf_index] = Alpha;
        printf("%s : [ALPHA] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Alpha.limit.id, Alpha.buf_index+1, Alpha.dir);
    }
    else
        printf("%s : ALPHA > NO USE \n",__func__);

    if(Dp086.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp086.limit.id = DP086;
        Dp086.dir = 'B'+ Dp086.buf_index;
        sprintf(tmp_name,"DP086");
        strcpy(Dp086.name,tmp_name);

        Dp086.limit.ptouch = (struct atmel_02_touch_limit*)malloc(sizeof(struct atmel_02_touch_limit));
       // synaptics_touch_limit_parser(Dp086.id,Dp086.name, Dp086.limit.ptouch);
        atmel_02_touch_limit_table_parser(Dp086.id, Dp086.name, Dp086.limit.ptouch);

        current_limit_parser(Dp086.id,&Dp086.limit.current);
        display_limit_parser(Dp086.id,Dp086.buf_index,&Dp086.limit.display,Dp086.dir);

        model_list[Dp086.buf_index] = Dp086;
        printf("%s : [DP086] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp086.limit.id, Dp086.buf_index+1, Dp086.dir);
    }
    else
        printf("%s : DP086 > NO USE \n",__func__);

    if(f2.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        f2.limit.id = F2;
        f2.dir = 'B'+ f2.buf_index;
        sprintf(tmp_name,"F2");
        strcpy(f2.name,tmp_name);

        //f2.limit.ptouch = (struct siw_touch_limit*)malloc(sizeof(struct siw_touch_limit));
        f2.limit.ptouch = (struct stm_07_touch_limit*)malloc(sizeof(struct stm_07_touch_limit));
//      touch_limit_table_parser(f2.id, f2.name, f2.limit.ptouch);
        stm_07_touch_limit_table_parser(f2.id, f2.name, f2.limit.ptouch);

        current_limit_parser(f2.id,&f2.limit.current);
        display_limit_parser(f2.id,f2.buf_index,&f2.limit.display,f2.dir);

        model_list[f2.buf_index] = f2;
        printf("%s : [F2] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,f2.limit.id, f2.buf_index+1, f2.dir);
    }
    else
        printf("%s : F2 > NO USE \n",__func__);

     if(Dp116.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp116.limit.id = DP116;
        Dp116.dir = 'B'+ Dp116.buf_index;
        sprintf(tmp_name,"DP116");
        strcpy(Dp116.name,tmp_name);

        Dp116.limit.ptouch = (struct atmel_03_touch_limit*)malloc(sizeof(struct atmel_03_touch_limit));
       // synaptics_touch_limit_parser(Dp116.id,Dp116.name, Dp116.limit.ptouch);
        atmel_03_touch_limit_table_parser(Dp116.id, Dp116.name, Dp116.limit.ptouch);

        current_limit_parser(Dp116.id,&Dp116.limit.current);
        display_limit_parser(Dp116.id,Dp116.buf_index,&Dp116.limit.display,Dp116.dir);

        model_list[Dp116.buf_index] = Dp116;
        printf("%s : [DP116] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp116.limit.id, Dp116.buf_index+1, Dp116.dir);
    }
    else
        printf("%s : DP116 > NO USE \n",__func__);

// 190924 LWG : sequence is important
//#ifdef VFOS_SITE_VIETNAM_2
     if(Dp150.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp150.limit.id = DP150;
        Dp150.dir = 'B'+ Dp150.buf_index;
        sprintf(tmp_name,"DP150");
        strcpy(Dp150.name,tmp_name);

        Dp150.limit.ptouch = (struct atmel_03_touch_limit*)malloc(sizeof(struct atmel_03_touch_limit));
       // synaptics_touch_limit_parser(Dp150.id,Dp150.name, Dp150.limit.ptouch);
        atmel_03_touch_limit_table_parser(Dp150.id, Dp150.name, Dp150.limit.ptouch);

        current_limit_parser(Dp150.id,&Dp150.limit.current);
        display_limit_parser(Dp150.id,Dp150.buf_index,&Dp150.limit.display,Dp150.dir);

        model_list[Dp150.buf_index] = Dp150;
        printf("%s : [DP150] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp150.limit.id, Dp150.buf_index+1, Dp150.dir);
    }
    else
        printf("%s : DP150 > NO USE \n",__func__);
//#endif

// 200129 LWG
//#ifdef VFOS_SITE_VIETNAM_2
     if(Dp173.en == ENABLE)
    {
        memset(tmp_name,0,sizeof(tmp_name));
        Dp173.limit.id = DP173;
        Dp173.dir = 'B'+ Dp173.buf_index;
        sprintf(tmp_name,"DP150");
        strcpy(Dp173.name,tmp_name);

        Dp173.limit.ptouch = (struct atmel_03_touch_limit*)malloc(sizeof(struct atmel_03_touch_limit));
       // synaptics_touch_limit_parser(Dp173.id,Dp173.name, Dp173.limit.ptouch);
        atmel_03_touch_limit_table_parser(Dp173.id, Dp173.name, Dp173.limit.ptouch);

        current_limit_parser(Dp173.id,&Dp173.limit.current);
        display_limit_parser(Dp173.id,Dp173.buf_index,&Dp173.limit.display,Dp173.dir);

        model_list[Dp173.buf_index] = Dp173;
        printf("%s : [DP173] > ENABLE STATE, LIMIT INIT.. \n",__func__);
        printf("%s :        >  id:%d / index:%d / dir:%c\n",__func__,Dp173.limit.id, Dp173.buf_index+1, Dp173.dir);
    }
    else
        printf("%s : DP173 > NO USE \n",__func__);
//#endif

	FUNC_END();
}


int limit_mem_free(void)
{
    int i = 0;
    int count = 0;

    FUNC_BEGIN();
	for(i=0;i<TOTAL_MODEL;i++)
	{
		if(model_list[i].en == ENABLE)
		{
			free(model_list[i].limit.ptouch);
			printf("%s : [%s] MEM Free \n",__func__,model_list[i].name);
			count++;
		}
	}
	FUNC_END();
	return count;
}

#include <i2c-dev.h>
#define SLV_ADDR_VCC1						0x40
#define SLV_ADDR_VCC2						0x41
#define SLV_ADDR_VDDVDH						0x44
#define SLV_ADDR_VDDEL						0x45

#define SLV_ADDR_ADS1						0x48
#define CONFIG_REGISTER						0x01
#if 0
void all_ina219_power_down_mode2(void){
    unsigned short conf = 0x8483;   // power down mode 
    unsigned short conf_swap = (((int)conf & 0xFF00) >> 8 | ((int)conf & 0xFF)  << 8);

    int i2c_fd = open(CURRENT_TEST_I2C_1_DEV, O_RDWR);

    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, SLV_ADDR_ADS1) < 0) 
        DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_ADS1);
    i2c_smbus_write_word_data(i2c_fd, CONFIG_REGISTER, conf_swap);
    
    while(1)
    {
        sleep(1);
        int result = i2c_smbus_read_word_data(i2c_fd, 0x00);
        printf("ADS RESULT = %d\n",result);
    }   
#if 0
    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC2) < 0) 
        DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC2);
    i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf_swap);
    usleep(30000);

    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDVDH) < 0) 
        DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDVDH);
    i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf_swap);
    usleep(30000);

    if (ioctl(i2c_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDEL) < 0) 
        DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDEL);
    i2c_smbus_write_word_data(i2c_fd, INA219_REG_CONF, conf_swap);
    usleep(30000);
#endif
    close(i2c_fd);
}
#endif


#define _IOCTL_I2C_13_9_LOW         0x1130
void i2c_converter_off(void){
	// LWG 191126 I2C LOW
	int value = 0;		//  0 : OFF
	ioctl(dic_dev, _IOCTL_I2C_13_9_LOW, &value);
}

void all_ina219_power_down_mode3(void){
	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	unsigned short conf = 0x3C18;   // power down mode 
	unsigned short conf_swap = (((int)conf & 0xFF00) >> 8 | ((int)conf & 0xFF)  << 8);

	int i2c_1_fd, i2c_2_fd, i2c_13_fd, i2c_9_fd;

	i2c_13_fd = open(CURRENT_TEST_I2C_3_DEV, O_RDWR);
	i2c_9_fd = open(CURRENT_TEST_I2C_4_DEV, O_RDWR);

		if (ioctl(i2c_13_fd, I2C_SLAVE_FORCE, 0x41) < 0)		// DVDD 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x41);
		i2c_smbus_write_word_data(i2c_13_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);

		if (ioctl(i2c_13_fd, I2C_SLAVE_FORCE, 0x45) < 0) 		// AVDD
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x45);
		i2c_smbus_write_word_data(i2c_13_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);
		
		if (ioctl(i2c_9_fd, I2C_SLAVE_FORCE, 0x41) < 0)		// DVDD 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x41);
		i2c_smbus_write_word_data(i2c_9_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);

		if (ioctl(i2c_9_fd, I2C_SLAVE_FORCE, 0x45) < 0) 		// AVDD
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x45);
		i2c_smbus_write_word_data(i2c_9_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);

	close(i2c_13_fd);
	close(i2c_9_fd);
}




void all_ina219_power_down_mode(void){
	printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
	unsigned short conf = 0x3C18;   // power down mode 
	unsigned short conf_swap = (((int)conf & 0xFF00) >> 8 | ((int)conf & 0xFF)  << 8);

	int i2c_1_fd, i2c_2_fd, i2c_13_fd, i2c_9_fd;

	i2c_1_fd = open(CURRENT_TEST_I2C_1_DEV, O_RDWR);
	i2c_13_fd = open(CURRENT_TEST_I2C_3_DEV, O_RDWR);
	i2c_2_fd = open(CURRENT_TEST_I2C_2_DEV, O_RDWR);
	i2c_9_fd = open(CURRENT_TEST_I2C_4_DEV, O_RDWR);

#if 1
	// VPNL, VDDI는 INA 안쓰고 ADS 사용
#if 0
	if (ioctl(i2c_1_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);
	i2c_smbus_write_word_data(i2c_1_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);

	if (ioctl(i2c_1_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC2) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC2);
	i2c_smbus_write_word_data(i2c_1_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);
#endif
	
	if (ioctl(i2c_1_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDVDH) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDVDH);
	i2c_smbus_write_word_data(i2c_1_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);

	if (ioctl(i2c_1_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDEL) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDEL);
	i2c_smbus_write_word_data(i2c_1_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);
#endif
	// 1CH TTL(VDDD)
	{
#if 1
		if (ioctl(i2c_13_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);
		i2c_smbus_write_word_data(i2c_13_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);
#endif
#if 0
		if (ioctl(i2c_13_fd, I2C_SLAVE_FORCE, 0x41) < 0)		// DVDD 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x41);
		i2c_smbus_write_word_data(i2c_13_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);

		if (ioctl(i2c_13_fd, I2C_SLAVE_FORCE, 0x45) < 0) 		// AVDD
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x45);
		i2c_smbus_write_word_data(i2c_13_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);
#endif
	}
	// VPNL, VDDI는 INA 안쓰고 ADS 사용
#if 0
	if (ioctl(i2c_2_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);
	i2c_smbus_write_word_data(i2c_2_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);

	if (ioctl(i2c_2_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC2) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC2);
	i2c_smbus_write_word_data(i2c_2_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);
#endif
	
	if (ioctl(i2c_2_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDVDH) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDVDH);
	i2c_smbus_write_word_data(i2c_2_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);

	if (ioctl(i2c_2_fd, I2C_SLAVE_FORCE, SLV_ADDR_VDDEL) < 0) 
		DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VDDEL);
	i2c_smbus_write_word_data(i2c_2_fd, INA219_REG_CONF, conf_swap);
	usleep(30000);

	// 2CH TTL(VDDD)
#if 1
		if (ioctl(i2c_9_fd, I2C_SLAVE_FORCE, SLV_ADDR_VCC1) < 0) 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",SLV_ADDR_VCC1);
		i2c_smbus_write_word_data(i2c_9_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);
#endif
#if 0

		if (ioctl(i2c_9_fd, I2C_SLAVE_FORCE, 0x41) < 0)		// DVDD 
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x41);
		i2c_smbus_write_word_data(i2c_9_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);

		if (ioctl(i2c_9_fd, I2C_SLAVE_FORCE, 0x45) < 0) 		// AVDD
			DERRPRINTF("Error: Could not set address[reg:0x%X] \n",0x45);
		i2c_smbus_write_word_data(i2c_9_fd, INA219_REG_CONF, conf_swap);
		usleep(30000);
#endif
	close(i2c_1_fd);
	close(i2c_2_fd);
	close(i2c_13_fd);
	close(i2c_9_fd);
}

extern int flag_interlock;
extern int password[10];

	// LWG 191106 CHECK FOR CUR_MODEL_ID IN parsing_pattern_mode(MODEL_COMMON.C)
int cur_model_id = 0;
int next_model_id = 0;


void ucurrent_en(int en)
{
	//en 1 = mA, 0 = uA
	#define _IOCTL_UCURRENT_EN      0x2080
	int ad_dev = open("/dev/ad5293",O_RDWR);
	ioctl(ad_dev, _IOCTL_UCURRENT_EN, &en);
    usleep(100);
}




int main(int argc, char** argv){
	int ret = 0;
	int dev;
	int buf_index = 0;
	// LWG 191106 CHECK FOR CUR_MODEL_ID IN parsing_pattern_mode(MODEL_COMMON.C)
//	int cur_model_id = 0;
//	int next_model_id = 0;
	KEY_EVENT   ev_key;
    unsigned char uart_buf[30];
	struct status_flag *sf = (struct status_flag *)malloc(sizeof(struct status_flag));

	FUNC_BEGIN();
	#define __FTS_LGD_09_H__

	if(argc == 2)
	{
		if(!strcmp(argv[1],"DEBUG"))
		{
			DEBUG_MODE = 1;			
			PACKEY_DEBUG = 1;			
			printf("------------------------[ DEBUG_MODE ]----------------------------\n");
		}
		else
	    {
		    DEBUG_MODE = 0;     
			PACKEY_DEBUG = 0;			
            printf("------------------------[ NOMAL_MODE ]----------------------------\n");
		}
	}
	else
	{
		DEBUG_MODE = 0;		
		printf("------------------------[ NOMAL_MODE ]----------------------------\n");
	}
	
	memset(uart_buf,0,sizeof(uart_buf));
    memset(sf,0,sizeof(struct status_flag));

	//#ifdef VFOS_SITE_VIETNAM_2
	site_version = VIETNAM_2;
		
    printf("\n## VFOS Ver%02d Start[Rev.%02d.%02d] ##\n",VFOS_VER,VFOS_REV,VFOS_REV_MINOR);
    printf("\n## [%s] ##\n",VFOS_MAIN_4800S_VERSION);
	if(site_version == GUMI)
	    printf("\n## FOR GUMI Site[0x%X] ##\n",site_version);
	else if(site_version == VIETNAM)
	    printf("\n## FOR VIETNAM Site[0x%X] ##\n",site_version);
	else if(site_version == VIETNAM_2)
		printf("\n## FOR VIETNAM_2 Site[0x%X] ##\n",site_version);
	else
	{
	    printf("\n## FOR UnKnown Site[0x%X] ##\n",site_version);
		site_version = GUMI;	
	    printf("\n## Default : GUMI Site[0x%X] ##\n",site_version);
	}
	
	if(!version_check_touch_limit())
		printf("## touch limit VERSION : %s ##\n",touch_version);
	else
		printf("## touch limit VERSION parsing FAIL.. ##");
	
	vfos_dev_open();
	manager_init();
	limit_parser();

	/* For new model - from STORM */
	if (site_version == VIETNAM)
	{
		/* for STORM */
		ret = init_model_storm();
		if (ret < 0)
		{
			printf("ERR: init_model_storm()\n");
			return -1;
		}
		/* for DP076 */
		ret = init_model_dp076();
		if (ret < 0)
		{
			printf("ERR: init_model_dp076()\n");
			return -1;
		}
		/* for ALPHA */
		ret = init_model_alpha();
		if (ret < 0)
		{
			printf("ERR: init_model_alpha()\n");
			return -1;
		}
		/* for DP086 */
		ret = init_model_dp086();
		if (ret < 0)
		{
			printf("ERR: init_model_dp086()\n");
			return -1;
        }
        /* for F2 */
        ret = init_model_f2();
        if (ret < 0)
        {
            printf("ERR: init_model_f2()\n");
            return -1;
        }
		/* for DP116 */
	   	ret = init_model_dp116();
        if (ret < 0)
        {
            printf("ERR: init_model_dp116()\n");
            return -1;
        }
		/* for DP150 */
	   	ret = init_model_dp150();
        if (ret < 0)
        {
            printf("ERR: init_model_dp150()\n");
            return -1;
        }
    }
    else if (site_version == GUMI)
    {
        /* for AOP */
        ret = init_model_aop();
        if (ret < 0)
		{
			printf("ERR: init_model_aop()\n");
			return -1;
		}
	}
	else if (site_version == VIETNAM_2)
	{
		/* for DP116 */
	   	ret = init_model_dp116();
        if (ret < 0)
        {
            printf("ERR: init_model_dp116()\n");
            return -1;
        }
		/* for DP150 */
	   	ret = init_model_dp150();
        if (ret < 0)
        {
            printf("ERR: init_model_dp150()\n");
            return -1;
        }
		/* for DP173 */
	   	ret = init_model_dp173();
        if (ret < 0)
        {
            printf("ERR: init_model_dp173()\n");
            return -1;
        }
		
	}

	uart_buf[VER_INFO_MODEL_ID_BUF_NUM] = model_list[0].id;
	uart_buf[VER_INFO_VFOS_VER_BUF_NUM] = VFOS_VER;
	uart_buf[VER_INFO_VFOS_REV_BUF_NUM] = VFOS_REV;
	uart_buf[VER_INFO_VFOS_REV_MINOR_BUF_NUM] = VFOS_REV_MINOR;
	uart_buf[VER_INFO_TOUCH_VER_BYTE_7_BUF_NUM] = touch_version[7];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_6_BUF_NUM] = touch_version[6];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_5_BUF_NUM] = touch_version[5];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_4_BUF_NUM] = touch_version[4];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_3_BUF_NUM] = touch_version[3];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_2_BUF_NUM] = touch_version[2];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_1_BUF_NUM] = touch_version[1];
	uart_buf[VER_INFO_TOUCH_VER_BYTE_0_BUF_NUM] = touch_version[0];
	uart_buf[VER_INFO_SITE_VER_BUF_NUM] = site_version;

	sf->key_pre = FUNC;
	sleep(3); //what..need..becouse kernel modify

	serial_packet_init(uart_buf, FUNC, 0x00); //test

    serial_write_function(uart_buf);
    printf("sendOK\n");
	
	// 4. 파일에 기록된 변경내역 읽어서 interlock 유무 확인 (재부팅시 반영)
	{
		FILE *pFile =  fopen("/mnt/sd/initial/interlock.tty", "rt");		
		char buffer[100];
		//size_t len = 0;
		if(pFile != NULL){
			//getline(&buffer, &len, pFile);
			fscanf(pFile, "%[^\n]", buffer);
			printf("LINE IS %s\n", buffer);
		
			if(!strncmp(buffer, "ENABLE", 6)){
				printf("INTERLOCK ENABLED\n");
				flag_interlock = 1;
			}else{
				printf("INTERLOCK DISABLED\n");
				//send_interlock_key_to_uart();		// toggle
				flag_interlock = 0;
			}
			
			fclose(pFile);

			
		}else{
			printf("NO PREVIOUS INTERLOCK SETTINGS\n");
		}
	}

	// 5. password.tty 에 인터락 비밀번호를 설정할수 있게 추가
		FILE *pFile2 =  fopen("/mnt/sd/initial/password.tty", "rt");
		char key[10];
		int i;
		int len;

		for(i=0;i<10;i++){
			fscanf(pFile2, "%s,",key);
			len = strlen(key);
			printf("key is %s\n", key);
			printf("last is %c\n",key[len-1]);
			if(key[len-1] == ',')		// 문자열 마지막 기호가 , 이면
				key[len-1] = '\0';		// 널문자로 변경

			printf("before : %d\n", password[i]);	
			// 문자열을 열거형 상수로 변환
			if(!strcmp(key, "OTP")){	
				password[i] = OTP;	
			}else if(!strcmp(key, "TOUCH")){
				password[i] = TOUCH;	
			}else if(!strcmp(key, "CURRENT")){
				password[i] = CURRENT;	
			}else if(!strcmp(key, "FUNC")){
				password[i] = FUNC;	
			}else if(!strcmp(key, "TURN")){			// NEXT
				password[i] = NEXT;	
			}else if(!strcmp(key, "RETURN")){		// PREV
				password[i] = PREV;	
			}else if(!strcmp(key, "RESET")){
				password[i] = RESET;	
			}else if(!strcmp(key, "FUNC2")){
				password[i] = FUNC2;	
			}
			printf("after : %d\n", password[i]);	
		}
		
		fclose(pFile2);

    if((dev = open("/dev/input/event0", O_RDWR )) < 0){
        perror("open fail /dev/input/event0\n");
        exit(-1);
    }

	system("/Data/reg_init /mnt/sd/initial/register_sleep_data1.tty");
	all_ina219_power_down_mode();
	i2c_converter_off();





	//goto DP116;
    while(1){
#ifdef VFOS_SITE_VIETNAM
        if(read (dev, &ev_key, sizeof (ev_key)))
        {
			usleep(50000);
            if(ev_key.code != 0)
            {
                if(ev_key.value)
                {
                    sf->key_act = ev_key.code -1;
					if(buf_index < en_model_count-1)
					{
						sf->next_model_index = buf_index+1;
						sf->next_model_id = model_list[buf_index+1].id;
					}
					else if(buf_index == en_model_count-1)
					{
						sf->next_model_index = 0;
						sf->next_model_id = model_list[0].id;
					}
					printf("buf_index %d en_model_count-1 %d\n", buf_index, en_model_count-1);
					DPRINTF("###buf_index=(%d)###\n", buf_index);
					DPRINTF("###next_model_id=(%d)###\n", sf->next_model_id);
                   	buf_index = key_counter(model_list[buf_index].id,&model_list[buf_index], sf);
					/* For STORM model */
					if (model_list[buf_index].id == STORM)
					{
						model_storm_info_t storm_info;

						DPRINTF("###Selected STORM###\n");

						memset(&storm_info,0,sizeof(model_storm_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						storm_info.key_dev = dev;
						storm_info.model_storm_id = cur_model_id;
						storm_info.next_model_id = next_model_id;
						storm_info.buf_index = buf_index;
						memcpy(&storm_info.storm_manager,&Storm,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&storm_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Storm.dir,storm_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_storm_thread(&storm_info);
						start_storm_thread();	/* waiting for storm thread finish */

						if (get_last_key_value_for_storm() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_storm());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_storm() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_storm());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_storm());
						}
					}
					/* For DP076 model */
					if (model_list[buf_index].id == DP076)
					{
						model_dp076_info_t dp076_info;

						DPRINTF("###Selected DP076###\n");

						memset(&dp076_info,0,sizeof(model_dp076_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						dp076_info.key_dev = dev;
						dp076_info.model_dp076_id = cur_model_id;
						dp076_info.next_model_id = next_model_id;
						dp076_info.buf_index = buf_index;
						memcpy(&dp076_info.dp076_manager,&Dp076,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&dp076_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Dp076.dir,dp076_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_dp076_thread(&dp076_info);
						start_dp076_thread();	/* waiting for dp076 thread finish */

						if (get_last_key_value_for_dp076() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp076());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_dp076() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp076());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp076());
						}
					}
					/* For AOP model */
					if (model_list[buf_index].id == AOP)
					{
						model_aop_info_t aop_info;

						DPRINTF("###Selected AOP###\n");

						memset(&aop_info,0,sizeof(model_aop_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						aop_info.key_dev = dev;
						aop_info.model_aop_id = cur_model_id;
						aop_info.next_model_id = next_model_id;
						aop_info.buf_index = buf_index;
						memcpy(&aop_info.aop_manager,&Aop,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&aop_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Aop.dir,aop_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_aop_thread(&aop_info);
						start_aop_thread();	/* waiting for aop thread finish */

						if (get_last_key_value_for_aop() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_aop());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

							//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_aop() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_aop());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_aop());
						}
					}
					/* For ALPHA model */
					if (model_list[buf_index].id == ALPHA)
					{
						model_alpha_info_t alpha_info;

						DPRINTF("###Selected ALPHA###\n");

						memset(&alpha_info,0,sizeof(model_alpha_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						alpha_info.key_dev = dev;
						alpha_info.model_alpha_id = cur_model_id;
						alpha_info.next_model_id = next_model_id;
						alpha_info.buf_index = buf_index;
						memcpy(&alpha_info.alpha_manager,&Alpha,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&alpha_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Alpha.dir,alpha_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_alpha_thread(&alpha_info);
						start_alpha_thread();   /* waiting for alpha thread finish */
						if (get_last_key_value_for_alpha() == FUNC) /* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_alpha());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

							//                          printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_alpha() == RESET)   /* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_alpha());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_alpha());
						}
					}
//DP086:
//model_list[buf_index].id = DP086;
					/* For DP086 model */
					if (model_list[buf_index].id == DP086)
					{
						model_dp086_info_t dp086_info;

						DPRINTF("###Selected DP086###\n");

						memset(&dp086_info,0,sizeof(model_dp086_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						dp086_info.key_dev = dev;
						dp086_info.model_dp086_id = cur_model_id;
						dp086_info.next_model_id = next_model_id;
						dp086_info.buf_index = buf_index;
						memcpy(&dp086_info.dp086_manager,&Dp086,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&dp086_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Dp086.dir,dp086_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_dp086_thread(&dp086_info);
						start_dp086_thread();	/* waiting for dp086 thread finish */

						if (get_last_key_value_for_dp086() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp086());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_dp086() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp086());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp086());
						}
                    }
//F2:
//model_list[buf_index].id = F2;
                    /* For F2 model */
                    if (model_list[buf_index].id == F2)
                    {
                        model_f2_info_t f2_info;

                        DPRINTF("###Selected F2###\n");

                        memset(&f2_info,0,sizeof(model_f2_info_t));

                        cur_model_id = model_list[buf_index].id;
                        next_model_id = model_list[((buf_index+1)%en_model_count)].id;

                        f2_info.key_dev = dev;
                        f2_info.model_f2_id = cur_model_id;
                        f2_info.next_model_id = next_model_id;
                        f2_info.buf_index = buf_index;
                        memcpy(&f2_info.f2_manager,&f2,sizeof(MODEL_MANAGER));
                        /* get version information */
                        get_vfos_version_info(next_model_id,buf_index+1,&f2_info.version_info);
                        /* get display image information */
                        ret = get_display_image_info(f2.dir,f2_info.display_image_file_name);
                        if (ret < 0)
                        {
                            printf("ERR:get_display_image_info\n");
                            FUNC_END();
                            return ret;
                        }
                        put_info_for_f2_thread(&f2_info);
                        start_f2_thread();	/* waiting for f2 thread finish */

                        if (get_last_key_value_for_f2() == FUNC)	/* MODEL key */
                        {
                            DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_f2());
                            buf_index = (buf_index + 1) % en_model_count;
                            sf->key_act = FUNC;
                            sf->next_model_index = (buf_index+1) % en_model_count;
                            sf->next_model_id = next_model_id;

                            //                			printf("[MAIN]model %s \n",model_list[buf_index].name);
                        }
                        else if (get_last_key_value_for_f2() == RESET)	/* RESET key */
                        {
                            DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_f2());
                        }
                        else
                        {
                            DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_f2());
                        }
                    }
#endif
//DP116:
//model_list[buf_index].id = DP116;
					/* For DP116 model */
					if (model_list[buf_index].id == DP116)
					{
						model_dp116_info_t dp116_info;

						DPRINTF("###Selected DP116###\n");
						memset(&dp116_info,0,sizeof(model_dp116_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						dp116_info.key_dev = dev;
						dp116_info.model_dp116_id = cur_model_id;
						dp116_info.next_model_id = next_model_id;
						dp116_info.buf_index = buf_index;
						memcpy(&dp116_info.dp116_manager,&Dp116,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&dp116_info.version_info);
						/* get display image information */
						ret = get_display_image_info(Dp116.dir,dp116_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_dp116_thread(&dp116_info);
						start_dp116_thread();	/* waiting for dp116 thread finish */
						//// LWG 190403 model key need to change?
						//DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp116());
						if (get_last_key_value_for_dp116() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp116());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_dp116() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp116());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp116());
						}
						////
                    }
					/* For DP150 model */
					if (model_list[buf_index].id == DP150)
					{
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						model_dp150_info_t dp150_info;

						DPRINTF("###Selected DP150###\n");
						memset(&dp150_info,0,sizeof(model_dp150_info_t));

						cur_model_id = model_list[buf_index].id;
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;

						dp150_info.key_dev = dev;
						dp150_info.model_dp150_id = cur_model_id;
						dp150_info.next_model_id = next_model_id;
						dp150_info.buf_index = buf_index;
						memcpy(&dp150_info.dp150_manager,&Dp150,sizeof(MODEL_MANAGER));
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&dp150_info.version_info);				// buf_index+1 ??
						/* get display image information */
						ret = get_display_image_info(Dp150.dir,dp150_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						put_info_for_dp150_thread(&dp150_info);												// LWG 190924 CRASH HERE
						start_dp150_thread();	/* waiting for dp150 thread finish */
						//// LWG 190403 model key need to change?
						//DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp150());
						if (get_last_key_value_for_dp150() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp150());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_dp150() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp150());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp150());
						}
						////
                    }
					/* For DP173 model */
					if (model_list[buf_index].id == DP173)
					{
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						model_dp173_info_t dp173_info;

						DPRINTF("###Selected DP173###\n");
						memset(&dp173_info,0,sizeof(model_dp173_info_t));

						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						cur_model_id = model_list[buf_index].id;
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						next_model_id = model_list[((buf_index+1)%en_model_count)].id;
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);

						dp173_info.key_dev = dev;
						dp173_info.model_dp173_id = cur_model_id;
						dp173_info.next_model_id = next_model_id;
						dp173_info.buf_index = buf_index;
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						memcpy(&dp173_info.dp173_manager,&Dp173,sizeof(MODEL_MANAGER));
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						/* get version information */
						get_vfos_version_info(next_model_id,buf_index+1,&dp173_info.version_info);				// buf_index+1 ??
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						/* get display image information */
						ret = get_display_image_info(Dp173.dir,dp173_info.display_image_file_name);
						if (ret < 0)
						{
							printf("ERR:get_display_image_info\n");
							FUNC_END();
							return ret;
						}
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						put_info_for_dp173_thread(&dp173_info);												// LWG 190924 CRASH HERE
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						start_dp173_thread();	/* waiting for dp173 thread finish */
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						//// LWG 190403 model key need to change?
						//DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp173());
						printf("                >>> LWG <<< [%s %d] %s CALL ====== \n", __FILE__, __LINE__, __FUNCTION__);
						if (get_last_key_value_for_dp173() == FUNC)	/* MODEL key */
						{
							DPRINTF("###Last key value = (%d)###\n", get_last_key_value_for_dp173());
							buf_index = (buf_index + 1) % en_model_count;
							sf->key_act = FUNC;
							sf->next_model_index = (buf_index+1) % en_model_count;
							sf->next_model_id = next_model_id;

//                			printf("[MAIN]model %s \n",model_list[buf_index].name);
						}
						else if (get_last_key_value_for_dp173() == RESET)	/* RESET key */
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp173());
						}
						else
						{
							DPRINTF("###Last key value = (%d),ERR: This must not be run###\n", get_last_key_value_for_dp173());
						}
						////
                    }


                    ///////////////////////////////////////////////////////
                    printf("[MAIN]model %s \n",model_list[buf_index].name);

#ifdef VFOS_SITE_VIETNAM
                }
            }
        }
#endif

    }

    if(en_model_count == limit_mem_free())
        printf("SUCCESS, ALL MODEL MEM FREE.. \n");
    else
        printf("FAIL, ALL MODEL MEM FREE.. \n");
	
	free(sf);
	if (site_version == VIETNAM)
	{
		/* for STORM */
		release_model_storm();
		/* for DP076 */
		release_model_dp076();
		/* for ALPHA */
    	release_model_alpha();
		/* for DP086 */
        release_model_dp086();
        /* for F2 */
        release_model_f2();
		/* for DP116 */
		release_model_dp116();
    }
	else if (site_version == GUMI)
	{
		/* for AOP */
		release_model_aop();
	}
	vfos_dev_close();
    close(dev);
	FUNC_END();
	return 0;
}






