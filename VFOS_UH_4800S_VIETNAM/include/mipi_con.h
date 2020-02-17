#ifndef	__MIPI_CON_H__
#define	__MIPI_CON_H__

#define DSIM_PIXEL_FORMAT_RGB24     0x7
#define DSIM_PIXEL_FORMAT_RGB18     0x6
#define DSIM_PIXEL_FORMAT_RGB18_PACKED  0x5


extern int	st_is_remain_code_after_next_pt;
enum remain_code_about { /*plus, if which model have a remain code(OUT CODE) after turn on next pattern*/

	B1_AOD_OFF = 0,
	B1_VARIABLE_OFF,
	B1_VR_OFF,

};

enum decon_rgb_order {
    DECON_RGB = 0x0,
    DECON_GBR = 0x1,
    DECON_BRG = 0x2,
    DECON_BGR = 0x4,
    DECON_RBG = 0x5,
    DECON_GRB = 0x6,
};

enum dsim_no_of_data_lane {
        DSIM_DATA_LANE_1 = 0,
        DSIM_DATA_LANE_2 = 1,
        DSIM_DATA_LANE_3 = 2,
        DSIM_DATA_LANE_4 = 3,
};

enum dsim_byte_clk_src {
        DSIM_PLL_OUT_DIV8 = 0,
        DSIM_EXT_CLK_DIV8 = 1,
        DSIM_EXT_CLK_BYPASS = 2,
};

enum dsim_video_mode_type {
        DSIM_NON_BURST_SYNC_EVENT = 0,
        DSIM_NON_BURST_SYNC_PULSE = 2,
        DSIM_BURST = 1,
        DSIM_NON_VIDEO_MODE = 4,
};

enum dsim_interface_type {
        DSIM_COMMAND = 0,
        DSIM_VIDEO = 1,
};

enum rgb_cpu_interface_type {
        RGB_MODE = 0,
        CPU_MODE = 1,
};


enum dsim_pixel_format {
        DSIM_CMD_3BPP = 0,
        DSIM_CMD_8BPP = 1,
        DSIM_CMD_12BPP = 2,
        DSIM_CMD_16BPP = 3,
        DSIM_VID_16BPP_565 = 4,
        DSIM_VID_18BPP_666PACKED = 5,
        DSIM_18BPP_666LOOSELYPACKED = 6,
        DSIM_24BPP_888 = 7,
};

enum mipi_ddi_parameter {
        /* RGB interface parameter */
        V_BACK_PORCH = 0,
        V_FRONT_PORCH = 1,
        H_BACK_PORCH = 2,
        H_FRONT_PORCH = 3,
        VSYNC_AREA = 4,
        HSYNC_AREA = 5,
        VCLK_POL = 6,
        HSYNC_POL = 7,
        VSYNC_POL = 8,
        VDEN_POL = 9,

        /* I80 interfae parameter */
        CS_SETUP_TIME = V_BACK_PORCH,
        WR_SETUP_TIME = V_FRONT_PORCH,
        WR_ACT_TIME = H_BACK_PORCH,
        WR_HOLD_TIME = H_FRONT_PORCH,
        RS_POL = VSYNC_AREA,

        /* DSIM video interface parameter */
        DSI_VIDEO_MODE_SEL = 10,
        DSI_VIRTUAL_CH_ID = 11,
        DSI_CMD_ALLOW_LEN = 12,
        DSI_FORMAT = 13,
};

enum decon_burst_mode {
    NON_BURST_EVENT = 0,
    NON_BURST_PULSE = 1,
    BURST_EVENT     = 2,
};

enum decon_psr_mode {
    DECON_VIDEO_MODE = 0,
    DECON_DP_PSR_MODE = 1,
    DECON_MIPI_COMMAND_MODE = 2,
};

/* Mic ratio: 0: 1/2 ratio, 1: 1/3 ratio */
enum decon_mic_comp_ratio {
    MIC_COMP_RATIO_1_2 = 0,
    MIC_COMP_RATIO_1_3 = 1,
    MIC_COMP_BYPASS
};

enum mic_ver {
    MIC_VER_1_1,
    MIC_VER_1_2,
    MIC_VER_2_0,
};

enum type_of_ddi {
    TYPE_OF_SM_DDI = 0,
    TYPE_OF_MAGNA_DDI,
    TYPE_OF_NORMAL_DDI,
};

struct dsim_pll_param {
    unsigned int p;
    unsigned int m;
    unsigned int s;
    unsigned int pll_freq; /* in/out parameter: Mhz */
};

struct dsim_clks
{
    unsigned int hs_clk;
    unsigned int esc_clk;
    unsigned int byte_clk;
};

struct dphy_timing_value
{
    unsigned int bps;
    unsigned int clk_prepare;
    unsigned int clk_zero;
    unsigned int clk_post;
    unsigned int clk_trail;
    unsigned int hs_prepare;
    unsigned int hs_zero;
    unsigned int hs_trail;
    unsigned int lpx;
    unsigned int hs_exit;
    unsigned int b_dphyctl;
};

struct dsim_clks_param {
    struct dsim_clks clks;
    struct dsim_pll_param pll;
    struct dphy_timing_value t;

    unsigned int esc_div;
};


struct stdphy_pms {
    unsigned int p;
    unsigned int m;
    unsigned int s;
    unsigned int clk_prepare;
    unsigned int clk_zero;
    unsigned int clk_post;
    unsigned int clk_trail;
    unsigned int hs_prepare;
    unsigned int hs_zero;
    unsigned int hs_trail;
    unsigned int lpx;
    unsigned int hs_exit;
    unsigned int dphy_manual;
};


  //=====================================================================================================//
 // Added by iamozzi. 2017.02.16.                                                                       //
//=====================================================================================================//
struct dsim_clkctrl
{
	unsigned int txrequest_hsclk;		// Turn on the HS clock
	unsigned int byteclk_en;			// 0 = disable, 1 = enable
	unsigned int escclk_en;				// 0 = disable, 1 = enable
	unsigned int lane_escclk_en;
	unsigned int esc_prescaler;
};

struct dsim_config
{
	unsigned int per_frame_read_en;		// 0 = disable, 1 = enable
	unsigned int q_channel_en;			// 0 = disable, 1 = enable
	unsigned int multi_pix;				// 0 = 1 pixel/clock, 1 = 2 pixel/clock
	unsigned int pixel_format;			// 0x07 = 24-bit RGB (For Common)
	unsigned int vc_id;					// 0x0 = Default (Virtual Channel ID)
/*
	unsigned int dsc_en;				// 0 = disable, 1 = enable
*/	
	unsigned int hsa_en;				// 0 = disable, 1 = enable (Command Mode Not Support)
	unsigned int hbp_en;				// 0 = disable, 1 = enable (Command Mode Not Support)
	unsigned int hfp_en;				// 0 = disable, 1 = enable (Command Mode Not Support)
	unsigned int hse_en;				// 0 = disable, 1 = enable (Command Mode Not Support)
	unsigned int hsync_preserve;		// 0 = hsync discard, 1 = hsync preserve
	unsigned int burst_mode;			// 0 = non-burst mode, 1 = burst mode (Command Mode & DSC Mode Not Support)
	unsigned int sync_inform;			// 0 = event mode (non-burst/burst), 1 = pulse mode (non-burst only) (Command Mode Not Support)
	unsigned int flush_vs;				// 0 = disable, 1 = enable
	unsigned int clklane_onoff;			// 0 = disable, 1 = enable
	unsigned int non_cont_clklane;		// 0 = disable, 1 = enable
};

struct dsim_cmd_config
{
	unsigned int te_detect_polarity;	// 0 = Rising edge detect, 1 = Falling edge detect
	unsigned int vsync_detect_polarity;	// 0 = Rising edge detect, 1 = Falling edge detect
	unsigned int pkt_go_rdy;			// 1 = Default
	unsigned int pkt_go_en;				// 0 = For during every VFP, 1 = For during 1 frame VFP
	unsigned int cmd_ctrl_sel;			// Video Mode Not Support
	unsigned int multi_cmd_pkt_en;		// 0 = Send Single Cmd Pkt, 1 = Send Multi Cmd Pkt	
};

//=====================================================================================================//
// end Added.
//=====================================================================================================//

/*
	//DSC0_CONTROL_0
*/
struct decon_lcd {
    enum decon_psr_mode mode;
    unsigned int vfp;
    unsigned int vbp;
    unsigned int hfp;
    unsigned int hbp;

    unsigned int vsa;
    unsigned int hsa;

    unsigned int xres;
    unsigned int yres;

    unsigned int width;
    unsigned int height;

    unsigned int hs_clk;
    unsigned int esc_clk;

    unsigned int fps;
    unsigned int mic_enabled;
    enum mic_ver mic_ver;
    enum type_of_ddi ddi_type;
    enum decon_mic_comp_ratio mic_ratio;
    unsigned int dsc_enabled;
    unsigned int dsc_cnt;
    unsigned int dsc_slice_num;

    struct stdphy_pms dphy_pms;
    unsigned int dsi_lanes;     //TUSUKANII

    unsigned int hsa_en;
    unsigned int hbp_en;
    unsigned int hfp_en;
    unsigned int hse_en;
    unsigned int hsync_preserve;
    unsigned int burst_mode;
    unsigned int flush_vsync_en;
    unsigned int pix_format;
    unsigned int multi_pix;
    unsigned int per_frame_read;
	unsigned int cmd_tr_mode;
	unsigned int rgb_order;
};


enum {
	false   = 0,
	true    = 1
};

struct mipi_conf {
    struct decon_lcd lcd_info;
	struct dsim_clks_param clks_param;
    struct dsim_clkctrl clkctrl;
    struct dsim_config cfg;
    struct dsim_cmd_config cmd_cfg;
};


enum {
    DSI_PORT_SEL_A = 0,
    DSI_PORT_SEL_B,
    DSI_PORT_SEL_BOTH,
};

typedef struct mipi_short_command
{
    char packet_type;
    char reg;
    char data;
}short_packet;

typedef struct mipi_long_command
{
    char packet_type;
    int count;
    char data[10000];
}long_packet;

typedef struct mipi_read_command
{
    char packet_type;
    unsigned short reg;
    unsigned short len;
}read_packet;

int mipi_dev_open();
int mipi_dev_close();
int decon_dev_open();
int decon_dev_close();

void kernel_msg_in_mipi_write(int param);
int mipi_hs_clk_change(unsigned int hs_clk);
void configure_dsi_read(void);
int mipi_read(char PacketType, unsigned char RdReg, int len, unsigned char *RdData );

//////////////////////////// DISPLAY

int bist_control(int mode, int id, int model_index, int *gray_scan, int if_prev);
int dbv_control(int mode, int id,int model_index, unsigned char reg_dbv1, unsigned char reg_dbv2,int *dimming_mode, char *pic_cmd, int if_prev); //180120
int aod_control(int id,int model_index, int *onoff, char mode, char *pic_cmd, int if_prev);
int vr_control(int id, int model_index, int *onoff, char mode, char *pic_cmd, int if_prev);
int emcon_control(int id, int *onoff, int mode, char *pic_cmd, int if_prev);
int variable_control(int id,int model_index, int *onoff, char *pic_cmd, int if_prev);
int blackpoint_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);
int bright_line_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);
int black_line_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);

//180111
int power_bright_line_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);
int power_black_line_control(int id, int model_index,int *onoff, char *pic_cmd, int if_prev);
int luminance_50per_power_bright_line_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);
int luminance_50per_power_black_line_control(int id, int model_index,int *onoff, char *pic_cmd, int if_prev);


int border_test_control(int id, int model_index, int *onoff, char *pic_cmd, int if_prev);

/*joan, mv*/
int joan_dsc_err_flag_check(int id, int model_index,  unsigned char *ch1_crc, unsigned char *ch2_crc);

/*A1*/

//////////////////////////// OTP
int otp_func(int  id,int model_index, unsigned char *ch1_pocb, unsigned char *ch2_pocb);

///// Other..
int sleep_control(int id,int model_index, int in, int if_prev);
int pocb_write_control(int id, int ch, unsigned char *onoff);

void found_initial_dbv(int id, int model_index, unsigned char *reg_dbv1, unsigned char *reg_dbv2);

void temp_mipi_video_reset_for_b1(void);

#endif	/* __MIPI_CON_H__ */

