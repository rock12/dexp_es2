/****************************************************************
  Copyright 	Novatek (Shanghai) Co., Ltd.
				All rights reserved.

  Panel Model:		IPS 196 ppi
  
  Project:		DEXP Ixion ES2 mt6582	

  Driver :		NT35512+CMI5.0HD TFT LCD panel;

  Interface :	MIPI-2LANE;   

  Reverse Engineering Code: © 4PDA® team: croc2008 & Co.

 ***************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[120];
};

	/*
	Structure Format :
	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},
	...
	Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
// 41E
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF,	4,	{0xAA, 0x55, 0xA5, 0x80}},
	{0xF7,	15,	{0x63, 0x40, 0x00, 0x00, 0x00, 0x01, 0xC4, 0xA2, 0x00, 0x02,0x64,0x54,0x48,0x00,0xD0}},
	{0xFF,	4,	{0xAA, 0x55, 0xA5, 0x00}},

	
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x01}},
	//{0xc0,	2,	{0x04,0x08}},
	{0xB0,	1,	{0x12}},
	{0xB1,	1,	{0x12}},
	{0xB2,	1,	{0x00}},
	{0xB3,	1,	{0x07}},
	
	{0xB6,	1,	{0x14}},
	{0xB7,	1,	{0x14}},
	{0xB8,	1,	{0x24}},
	{0xB9,	1,	{0x34}},
	{0xBA,	1,	{0x14}},
	{0xBF,	1,	{0x01}},

	{0xC3,	1,	{0x04}},
	{0xC2,	1,	{0x00}},
	{0xC0,	2,	{0x04,0x08}},

	{0xBC,	3,	{0x00,0x70,0x00}},
	{0xBD,	3,	{0x00,0x70,0x00}},
	{0xBE,	2,	{0x00,0x4B}},
		

	{0xD1,	52,	{0x00,0x00,0x00,0x0C,0x00,0x22,0x00,0x36,0x00,0x47,0x00,0x66,0x00,0x82,0x00,0xB2,0x00,0xDA,0x01,0x1B,0x01,0x51,0x01,0xA5,0x01,0xED,0x01,0xEE,0x02,0x30,0x02,0x75,0x02,0x9E,0x02,0xD7,0x02,0xF9,0x03,0x28,0x03,0x47,0x03,0x6C,0x03,0x81,0x03,0x90,0x03,0xC0,0x03,0xF8}},
	{0xD2,	52, {0x00,0x48,0x00,0x50,0x00,0x5F,0x00,0x6D,0x00,0x7A,0x00,0x94,0x00,0xAB,0x00,0xD4,0x00,0xF7,0x01,0x31,0x01,0x61,0x01,0xB2,0x01,0xF7,0x01,0xF9,0x02,0x38,0x02,0x7C,0x02,0xA4,0x02,0xDE,0x02,0xFF,0x03,0x30,0x03,0x4D,0x03,0x72,0x03,0x8A,0x03,0xA6,0x03,0xC0,0x03,0xF8}},
	{0xD3,	52, {0x00,0x00,0x00,0x0C,0x00,0x23,0x00,0x37,0x00,0x4A,0x00,0x69,0x00,0x85,0x00,0xB7,0x00,0xE0,0x01,0x21,0x01,0x57,0x01,0xAC,0x01,0xF3,0x01,0xF5,0x02,0x31,0x02,0x78,0x02,0xA1,0x02,0xD7,0x02,0xF8,0x03,0x26,0x03,0x48,0x03,0x6A,0x03,0x71,0x03,0x76,0x03,0xF7,0x03,0xF8}},
	{0xD4,	52, {0x00,0x00,0x00,0x0C,0x00,0x22,0x00,0x36,0x00,0x47,0x00,0x66,0x00,0x82,0x00,0xB2,0x00,0xDA,0x01,0x1B,0x01,0x51,0x01,0xA5,0x01,0xED,0x01,0xEE,0x02,0x30,0x02,0x75,0x02,0x9E,0x02,0xD7,0x02,0xF9,0x03,0x28,0x03,0x47,0x03,0x6C,0x03,0x81,0x03,0x90,0x03,0xC0,0x03,0xF8}},
	{0xD5,	52, {0x00,0x48,0x00,0x50,0x00,0x5F,0x00,0x6D,0x00,0x7A,0x00,0x94,0x00,0xAB,0x00,0xD4,0x00,0xF7,0x01,0x31,0x01,0x61,0x01,0xB2,0x01,0xF7,0x01,0xF9,0x02,0x38,0x02,0x7C,0x02,0xA4,0x02,0xDE,0x02,0xFF,0x03,0x30,0x03,0x4D,0x03,0x72,0x03,0x8A,0x03,0xA6,0x03,0xC0,0x03,0xF8}},
	{0xD6,	52, {0x00,0x00,0x00,0x0C,0x00,0x23,0x00,0x37,0x00,0x4A,0x00,0x69,0x00,0x85,0x00,0xB7,0x00,0xE0,0x01,0x21,0x01,0x57,0x01,0xAC,0x01,0xF3,0x01,0xF5,0x02,0x31,0x02,0x78,0x02,0xA1,0x02,0xD7,0x02,0xF8,0x03,0x26,0x03,0x48,0x03,0x6A,0x03,0x71,0x03,0x76,0x03,0xF7,0x03,0xF8}},


	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x00}},
	
	{0xB6,	1,	{0x03}},//SDT: 
	{0xB7,	2,	{0x73,0x73}},
	{0xB8,	4,	{0x01,0x06,0x06,0x06}},//Source EQ:
	{0xBC,	1,	{0x00}},
	{0xB1,	3,	{0x63, 0x00, 0x01}},
	{0xB4,	1,	{0x10}},


	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
//    {0x11,1,{0x00}}, //SLEEP OUT
//    {REGFLAG_DELAY,120,{}},
                                 				                                                                                
//    {0x29,1,{0x00}}, //Display ON 
//    {REGFLAG_DELAY,120,{}},
    
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},

    // Sleep Mode On
	{0x10, 0, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//MDELAY(10);//soso add or it will fail to send register
       	}
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

// 81E1F6EC
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

// 81E1F674
static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS)); //536 //0x218
	
		params->type   = LCM_TYPE_DSI; //2

		params->width  = FRAME_WIDTH;  //480 //0x1E0
		params->height = FRAME_HEIGHT; //854 //0x356


		params->dsi.mode   = SYNC_EVENT_VDO_MODE; //2
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM 				= LCM_TWO_LANE; //2
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB; //0
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST; //0
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB; //0
		params->dsi.data_format.format 		= LCM_DSI_FORMAT_RGB888; //2

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size				= 256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num 	= 2; //because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS						= LCM_PACKED_PS_24BIT_RGB888; //2
//		params->dsi.word_count					= 480*3; //1440 //0x5A0

		params->dsi.vertical_sync_active 	= 2;
		params->dsi.vertical_backporch 				= 50; //0x32
		params->dsi.vertical_frontporch 			= 20; //0x14
		params->dsi.vertical_active_line 			= FRAME_HEIGHT; //854 

		params->dsi.horizontal_sync_active 			= 2;
		params->dsi.horizontal_backporch 			= 100; //0x64
		params->dsi.horizontal_frontporch 			= 100; //0x64
		params->dsi.horizontal_active_pixel 		= FRAME_WIDTH; //480


		// Bit rate calculation
//		params->dsi.pll_div1=30;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
//		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

		//improve clk quality
		//this value must be in MTK suggested table
		params->dsi.PLL_CLOCK = 240; //0xF0 //dsi clock customization: should config clock value directly

		/* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response. 
//		params->dsi.lcm_int_te_monitor = FALSE; 
//		params->dsi.lcm_int_te_period = 1; // Unit : frames 
 
		// Need longer FP for more opportunity to do int. TE monitor applicably. 
//		if(params->dsi.lcm_int_te_monitor) 
//			params->dsi.vertical_frontporch *= 2; 
 
		// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.) 
//		params->dsi.lcm_ext_te_monitor = FALSE; 
		// Non-continuous clock 
//		params->dsi.noncont_clock = TRUE; 
//		params->dsi.noncont_clock_period = 2; // Unit : frames		
}

// 81E1F790
static void lcm_init(void)
{
	unsigned int data_array[64];

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(120);//Must > 120ms
    
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

// 81E1F754
static void lcm_suspend(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(20);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(150);//Must > 120ms

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

// 81E1F7CC
static void lcm_resume(void)
{
	//lcm_compare_id();
	lcm_init();
	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

// 81E1F5D8
static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[3];
	unsigned int array[16];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x04, buffer, 3);
	id = buffer[1]; //we only need ID
    return 1;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35512_fwvga_boe5p0_xingld_dg_a26_lcm_drv = 
{
    .name			= "nt35512_fwvga_boe5p0_xingld_dg_a26",
	.set_util_funcs = lcm_set_util_funcs, // static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
	.get_params     = lcm_get_params,     // static void lcm_get_params(LCM_PARAMS *params)
	.init           = lcm_init,           // static void lcm_init(void)
	.suspend        = lcm_suspend,        // static void lcm_suspend(void)
	.resume         = lcm_resume,         // static void lcm_resume(void)
//	.compare_id		= lcm_compare_id,     // static unsigned int lcm_compare_id(void)
};
