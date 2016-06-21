/****************************************************************
 * C o*pyright 	Novatek (Shanghai) Co., Ltd.
 * All rights reserved.
 * 
 * Panel Model:		IPS 196 ppi
 * 
 * Project:		DEXP Ixion ES2 mt6582	
 * 
 * Driver :		NT35512+CMI5.0HD TFT LCD panel;
 * 
 * Interface :	MIPI-2LANE;   
 * 
 * Reverse Engineering Code: © 4PDA® team: croc2008 & Co.
 * 
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

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

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

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[67];
};

/*
 *  Structure Format :
 *  {DCS command, count of parameters, {parameter list}}
 *  {REGFLAG_DELAY, milliseconds of time, {}},
 *  ...
 *  Setting ending by predefined flag
 *  {REGFLAG_END_OF_TABLE, 0x00, {}}
 */
// 41E

static struct LCM_setting_table lcm_id_setting[] = {
    { 0xf0, 0x05, {0x55, 0xaa, 0x52, 0x08, 0x01 }},
    { REGFLAG_DELAY, 0x0a, {}},
    { REGFLAG_END_OF_TABLE, 0, {}},
};

static struct LCM_setting_table lcm_initialization_setting[] = {
    { 0xf0, 0x05, {0x55, 0xaa, 0x52, 0x08, 0x01 }},
    { 0xb6, 0x03, {0x54, 0x54, 0x54 }},
    { 0xb0, 0x03, {0x0a, 0x0a, 0x0a }},
    { 0xb7, 0x03, {0x34, 0x34, 0x34 }},
    { 0xb1, 0x03, {0x0a, 0x0a, 0x0a }},
    { 0xb3, 0x03, {0x0b, 0x0b, 0x0b }},
    { 0xb9, 0x03, {0x34, 0x34, 0x34 }},
    { 0xb5, 0x03, {0x0b, 0x0b, 0x0b }},
    { 0xb8, 0x03, {0x24, 0x24, 0x24 }},
    { 0xba, 0x03, {0x14, 0x14, 0x14 }},
    { 0xbc, 0x03, {0x00, 0x93, 0x00 }},
    { 0xbd, 0x03, {0x00, 0x93, 0x00 }},
    { 0xbe, 0x02, {0x00, 0x28 }},
    { 0xbf, 0x01, {0x01 }},
    { 0xd1, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xd2, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xd3, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xd4, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xd5, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xd6, 0x34, {0x00, 0x77, 0x00, 0xa9, 0x00, 0xd1, 0x00, 0xeb, 0x00, 0xff, 0x01, 0x1f, 0x01, 0x39, 0x01, 0x62, 0x01, 0x82, 0x01, 0xb4, 0x01, 0xda, 0x02, 0x18, 0x02, 0x4a, 0x02, 0x4b, 0x02, 0x79, 0x02, 0xa9, 0x02, 0xc6, 0x02, 0xef, 0x03, 0x0c, 0x03, 0x31, 0x03, 0x48, 0x03, 0x69, 0x03, 0x7b, 0x03, 0xa9, 0x03, 0xaa, 0x03, 0xab }},
    { 0xf0, 0x05, {0x55, 0xaa, 0x52, 0x08, 0x00 }},
    { 0xb0, 0x05, {0x00, 0x18, 0x0a, 0x36, 0x0a }},
    { 0xb1, 0x02, {0xfc, 0x00 }},
    { 0xb4, 0x01, {0x00 }},
    { 0xb5, 0x01, {0x6a }},
    { 0xb6, 0x01, {0x02 }},
    { 0xb7, 0x02, {0x50, 0x50 }},
    { 0xb8, 0x04, {0x01, 0x08, 0x08, 0x08 }},
    { 0xbc, 0x03, {0x00, 0x00, 0x00 }},
    { 0xba, 0x01, {0x01 }},
    { 0xcc, 0x03, {0x03, 0x00, 0x00 }},
    { 0xff, 0x04, {0xcc, 0x33, 0x12, 0x82 }},
    { 0xf7, 0x12, {0x65, 0xc0, 0x40, 0x00, 0x01, 0x02, 0xb0, 0x98, 0x00, 0x02, 0x40, 0x54, 0x00, 0x00, 0xc0, 0x00, 0x42, 0x28 }},
    { 0x3a, 0x01, {0x77 }},
    { 0x11, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0x78, {}},
    { 0x29, 0x01, {0x00 }},
    { REGFLAG_DELAY, 0x32, {}},
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
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type = 2;
    params->dsi.LANE_NUM = 2;
    params->dsi.data_format.format = 2;
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS = 2;
    params->dsi.word_count = 1440;
    params->dbi.te_edge_polarity = 0;
    params->dsi.data_format.color_order = 0;
    params->dsi.data_format.trans_seq = 0;
    params->dsi.data_format.padding = 0;
    params->dsi.vertical_backporch = 20;
    params->dsi.pll_div2 = 0;
    params->width = 480;
    params->height = 854;
    params->dbi.te_mode = 1;
    params->dsi.mode = 3;
    params->dsi.vertical_active_line = 854;
    params->dsi.vertical_sync_active = 4;
    params->dsi.vertical_frontporch = 10;
    params->dsi.horizontal_sync_active = 4;
    params->dsi.horizontal_backporch = 50;
    params->dsi.horizontal_frontporch = 10;
    params->dsi.horizontal_active_pixel = 480;
    params->dsi.pll_div1 = 1;
    params->dsi.fbk_div = 15;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(150);
    
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(20);
    
}

static void lcm_resume(void)
{
    lcm_init();
    
}

static unsigned int lcm_compare_id(void)
{	
    int id_high;
    int id_low; 
    unsigned char buffer[2]; 
    int data[8]; 
    
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(150);
    push_table(lcm_id_setting, sizeof(lcm_id_setting) / sizeof(struct LCM_setting_table), 1);
    data[0] = 0x23700;
    dsi_set_cmdq(data, 1, 1);
    read_reg_v2(0xC6, buffer, 2);                  
    id_high = buffer[0];
    id_low = buffer[1];
    LCM_PRINT("luke: sh1282 %d  id_high:0x%x  id_low:0x%x \n", 0x16F, buffer[0], buffer[1]);
    if (id_low == 2 && id_high == 18)
        return TRUE;
    else
        return FALSE;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER sh1282_fwvga_boe5p0_xld_a26_lcm_drv = 
{
    .name			= "sh1282_fwvga_boe5p0_xld_a26",
    .set_util_funcs = lcm_set_util_funcs, // static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
    .get_params     = lcm_get_params,     // static void lcm_get_params(LCM_PARAMS *params)
    .init           = lcm_init,           // static void lcm_init(void)
    .suspend        = lcm_suspend,        // static void lcm_suspend(void)
    .resume         = lcm_resume,         // static void lcm_resume(void)
    .compare_id		= lcm_compare_id,     // static unsigned int lcm_compare_id(void)
};
