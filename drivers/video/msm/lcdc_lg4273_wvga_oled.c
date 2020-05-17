/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#ifdef CONFIG_SPI_QSD
#include <linux/spi/spi.h>
#endif
#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"

#ifdef CONFIG_FB_MSM_TRY_MDDI_CATCH_LCDC_PRISM
#include "mddihosti.h"
#endif

#include <mach/vreg.h>
#include <mach/board_lge.h>

#define CONFIG_BACKLIGHT_LEDS_CLASS

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#endif

/* enable or disable the PLC Control  */
#define FEATURE_LGD_PLC_CONTROL

/* enable disable the rev.C OLED */
//#define CONFIG_LGE_OLED_REV_C

#ifdef CONFIG_SPI_QSD
#define LCDC_LGDISPLAY_SPI_DEVICE_NAME "lcdc_lgdislpay_lg4273_oled"
static struct spi_device *lcdc_lgdisplay_spi_client;
#else
static int spi_cs;
static int spi_sclk;
static int spi_mosi;
static int spi_miso;
#endif
struct lgdisplay_state_type{
	boolean disp_initialized;
	boolean display_on;
	boolean disp_powered_up;
};

 
static struct lgdisplay_state_type lgdisplay_state = { 1, 1, 1 };
static struct msm_panel_common_pdata *lcdc_lgdisplay_pdata;

#define LGDISPLAY_DATA  	0
#define LGDISPLAY_ADDR  	1
#define LGDISPLAY_DELAY		2
#define LGDISPLAY_SLEEP		3
#define	LGDISPLAY_EOF		4

struct lgdisplay_spi_data {
	u8 type;
	u8 data;
};

static void lgdisplay_write_table(struct lgdisplay_spi_data *seq);

#if !defined (FEATURE_LGD_PLC_CONTROL)
static struct lgdisplay_spi_data init_sequence[] = {
	{LGDISPLAY_ADDR, 0xe9},
	{LGDISPLAY_DATA, 0x30}, /*RGB I/F */

	{LGDISPLAY_ADDR, 0xc3},
	{LGDISPLAY_DATA, 0x00}, /*VCI1=VCI=2.8V, NOC;4clk(step-*/
	{LGDISPLAY_DATA, 0x28}, /*AP=2(x1), DDVDH=VCI1X2=5.6V,*/
	{LGDISPLAY_DATA, 0x32}, /*Step-up2;line freq x 2, Step- */

	{LGDISPLAY_ADDR, 0xc4},
	{LGDISPLAY_DATA, 0x18}, /*VREG1= 3.77V */
	{LGDISPLAY_DATA, 0xff}, /*VREG2=VREG1 */
	{LGDISPLAY_DATA, 0x00}, /*VREF=1.7V(@VCI=2.8V) */
	{LGDISPLAY_DATA, 0x44}, /*VDDOUT=1.82V, VREF_DAC ratio */
	{LGDISPLAY_DATA, 0x44}, /*RI, RSET */

	{LGDISPLAY_ADDR, 0xbf},
	{LGDISPLAY_DATA, 0x00}, /*Power Down Off */

	{LGDISPLAY_ADDR, 0xba},
	{LGDISPLAY_DATA, 0x00}, /*GPO Initail[GPO<0>: OLED_EN,G */

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0x00}, /*DDVDH, VGL,VGH,VGLDC OFF, STB */

	{LGDISPLAY_DELAY, 1}, /*1ms */

	{LGDISPLAY_ADDR, 0xba},
	{LGDISPLAY_DATA, 0x04}, /*GPO<2>=1 (P-IC=ON) */

	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0x08}, /*VGLDC ON*/

	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0x88}, /*DDVDH ON, VGLDC ON*/

	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0x80}, /*DDVDH ON, VGLDC Off*/

	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0xa0}, /*DDVDH ON, VGL ON*/

	{LGDISPLAY_DELAY, 16}, /*16ms*/

	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0xb0}, /*DDVDH ON, VGL ON, VGH ON*/

	{LGDISPLAY_DELAY, 16}, /*16ms*/

	{LGDISPLAY_ADDR, 0xb9},
	{LGDISPLAY_DATA, 0x30}, /*DTE;Eclk enable,DTW;Gclk enab*/

	{LGDISPLAY_DELAY, 16/* 10 */}, /*10ms*/

	{LGDISPLAY_ADDR, 0xcd},
	{LGDISPLAY_DATA, 0xc2}, /*TEGW[7:0]*/
	{LGDISPLAY_DATA, 0x91}, /*TEGS[3:0],TEGW[11:8]*/
	{LGDISPLAY_DATA, 0x01}, /*TEGS[11:4]*/
	{LGDISPLAY_DATA, 0x19}, /*TGW1[7:0]*/
	{LGDISPLAY_DATA, 0x90}, /*TGS1[3:0],TGW1[11:8]*/
	{LGDISPLAY_DATA, 0x01}, /*TGS1[11:4]*/

	{LGDISPLAY_ADDR, 0x29},
	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xb8},
	{LGDISPLAY_DELAY, 16}, /*10ms*/

	{LGDISPLAY_ADDR, 0xc4},
	{LGDISPLAY_DATA, 0x1f}, /*VREG1= 3.77V*/
	{LGDISPLAY_DATA, 0xff}, /*VREG2=VREG1*/
	{LGDISPLAY_DATA, 0x0f}, /*VREF=1.7V(@VCI=2.8V)*/
	{LGDISPLAY_DATA, 0x44}, /*VDDOUT=1.82V, VREF_DAC ratio*/
	{LGDISPLAY_DATA, 0x44}, /*RI, RSET*/

	{LGDISPLAY_DELAY, 16}, /*20ms*/

	{LGDISPLAY_ADDR, 0xba},
	{LGDISPLAY_DATA, 0x05}, /*Panel On(GPO[0]:OLED_EN)*/

   /* disable temporary
    * cheongil.hyun@lge.com
	{LGDISPLAY_ADDR, 0x51},
	{LGDISPLAY_DATA, 0xb0},
	{LGDISPLAY_DELAY, 10}, *//* 10ms */

	{LGDISPLAY_EOF,  0x00},
};
#else
static struct lgdisplay_spi_data init_sequence[] = {
	/****** RGB I/F Display Mode Set *******/
	{LGDISPLAY_ADDR,0xE9},
	{LGDISPLAY_DATA,0x30},
	
	/*************Power Control***********/
	{LGDISPLAY_ADDR,0xC3},
	{LGDISPLAY_DATA,0x00}, 	/*VCI1=VCI=2.8V, NOC;4clk(step-up2)*/
	{LGDISPLAY_DATA,0x24}, 	/*AP=2(x1), DDVDH=VCI1X2=5.6V, VGH=VDD_OLED+VCI1(2.8V)=12.3V, VGL=-2XVCI1=-5.6V*/
	{LGDISPLAY_DATA,0x32}, 	/*Step-up2;line freq x 2, Step-up1; line freq x 4 */
	
	{LGDISPLAY_ADDR,0xC4},  
	{LGDISPLAY_DATA,0x1F}, 	/*VREG1= 3.77V*/
	{LGDISPLAY_DATA,0xFF}, 	/*VREG2=VREG1*/
	{LGDISPLAY_DATA,0x00}, 	/*VREF=1.7V(@VCI=2.8V)  */
	{LGDISPLAY_DATA,0x44}, 	/*VDDOUT=1.82V, VREF_DAC ratio x1*/
	{LGDISPLAY_DATA,0x44}, 	/*RI, RSET */
	
	/***********Power Sequence On***********/
	{LGDISPLAY_ADDR,0xBA},
	{LGDISPLAY_DATA,0x00}, /*GPO Initail[GPO<0>: OLED_EN,GPO<2>: P-IC_EN]*/
	{LGDISPLAY_ADDR,0xBE},
	{LGDISPLAY_DATA,0x00}, /*DDVDH, VGL,VGH,VGLDC OFF, STB OFF, DSTB OFF   */
	{LGDISPLAY_DELAY,1},   /*1ms*/
	{LGDISPLAY_ADDR,0xBA},  
	{LGDISPLAY_DATA,0x04}, /*GPO<2>=1 (P-IC=ON)*/
	{LGDISPLAY_DELAY,16},  /*10ms  */
	{LGDISPLAY_ADDR,0xBE},
	{LGDISPLAY_DATA,0x08}, /*VGLDC ON	*/
	{LGDISPLAY_DELAY,16},  /*10ms	*/
	{LGDISPLAY_ADDR,0xBE},
	{LGDISPLAY_DATA,0x88}, /*DDVDH ON, VGLDC ON	*/
	{LGDISPLAY_DELAY,16},  /*10ms	*/
	{LGDISPLAY_ADDR,0xBE},
	{LGDISPLAY_DATA,0x80}, /*DDVDH ON, VGLDC Off*/
	{LGDISPLAY_DELAY,16},   /*10ms*/
	{LGDISPLAY_ADDR,0xBE}, 
	{LGDISPLAY_DATA,0xA0}, /*DDVDH ON, VGL ON*/
	{LGDISPLAY_DELAY,16},  /*16ms*/
	{LGDISPLAY_ADDR,0xBE}, 
	{LGDISPLAY_DATA,0xb0}, /*DDVDH ON, VGL ON, VGH ON*/
	{LGDISPLAY_DELAY,16},  /*16ms*/
	{LGDISPLAY_ADDR,0xB9}, 
	{LGDISPLAY_DATA,0x30}, /*DTE;Eclk enable,DTW;Gclk enable, CM disable*/
	{LGDISPLAY_DELAY,16},  /*10ms  */
	
	/***********Power Down Register************/
	{LGDISPLAY_ADDR,0xBF},
	{LGDISPLAY_DATA,0x00}, 	/*Power Down Off*/
	
	/************Initial Gate Timing************/
	{LGDISPLAY_ADDR,0xCD},	                              
	{LGDISPLAY_DATA,0xC2},  	/*TEGW[7:0]*/
	{LGDISPLAY_DATA,0x91},  	/*TEGS[3:0],TEGW[11:8]*/
	{LGDISPLAY_DATA,0x01},  	/*TEGS[11:4]*/
	{LGDISPLAY_DATA,0x19},  	/*TGW1[7:0]*/
	{LGDISPLAY_DATA,0x90},  	/*TGS1[3:0],TGW1[11:8]*/
	{LGDISPLAY_DATA,0x01},  	/*TGS1[11:4]*/
	
	/************Display On************/
	{LGDISPLAY_ADDR,0x29},
	{LGDISPLAY_DELAY,16}, /*10ms*/
	
	/************OTP Read************/
	{LGDISPLAY_ADDR, 0xB8},
	{LGDISPLAY_DELAY, 16},/*10ms*/

	/****** AGPC *****/
	{LGDISPLAY_ADDR, 0xc0},       /*AGPC EN*/
	{LGDISPLAY_DATA, 0x01},

	{LGDISPLAY_ADDR, 0xce},       /*AGPC CONTROL1*/
	{LGDISPLAY_DATA, 0x00},
	{LGDISPLAY_DATA, 0x46},
	{LGDISPLAY_DATA, 0x00},

	{LGDISPLAY_ADDR, 0xcf},       /*AGPC CONTROL2*/
	{LGDISPLAY_DATA, 0x72},
	{LGDISPLAY_DATA, 0x11},
	{LGDISPLAY_DATA, 0x05},
	{LGDISPLAY_DATA, 0x05},

	{LGDISPLAY_ADDR, 0xe1},       /*LUT START*/
	{LGDISPLAY_DATA, 0x00},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xff},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x01},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xfe},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x02},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xfd},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x03},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xfc},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x04},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xfb},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x05},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xfa},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x06},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf9},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x07},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf8},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x08},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf7},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x09},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf6},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0a},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf5},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0b},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf4},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0c},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf3},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0d},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf2},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0e},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf1},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x0f},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xf0},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x10},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xef},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x11},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xee},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x12},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xed},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x13},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xec},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x14},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xeb},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x15},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xea},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x16},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe9},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x17},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe8},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x18},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe7},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x19},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe6},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1a},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe5},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1b},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe4},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1c},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe3},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1d},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe2},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1e},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe1},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x1f},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xe0},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x20},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xdf},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x21},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xde},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x22},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xdd},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x23},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xdc},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x24},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xdb},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x25},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xda},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x26},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd9},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x27},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd8},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x28},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd7},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x29},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd6},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2a},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd5},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2b},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd4},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2c},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd3},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2d},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd2},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2e},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd1},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x2f},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xd0},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x30},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xcf},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x31},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xce},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x32},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xcd},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x33},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xcc},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x34},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xcb},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x35},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xca},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x36},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc9},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x37},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc8},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x38},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc7},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x39},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc6},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3a},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc5},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3b},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc4},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3c},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc3},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3d},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc2},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3e},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc1},
	{LGDISPLAY_ADDR, 0xe1},
	{LGDISPLAY_DATA, 0x3f},
	{LGDISPLAY_ADDR, 0xe2},
	{LGDISPLAY_DATA, 0xc0},       /*LUT END*/

	/************Vref Returning************/
	{LGDISPLAY_ADDR,0xC4},  
	{LGDISPLAY_DATA,0x1F}, /*VREG1= 3.77V*/
	{LGDISPLAY_DATA,0xFF}, /*VREG2=VREG1*/
	{LGDISPLAY_DATA,0x0f}, /*VREF=1.7V(@VCI=2.8V)  */
	{LGDISPLAY_DATA,0x44}, /*VDDOUT=1.82V, VREF_DAC ratio x1*/
	{LGDISPLAY_DATA,0x44}, /*RI, RSET*/
	{LGDISPLAY_DELAY,16}, /*10ms*/

	/****** EL GND SW On *******/
	{LGDISPLAY_ADDR,0xBA},
	{LGDISPLAY_DATA,0x05},  /*Panel On(GPO[0]:OLED_EN)*/

	/************Brightness control************/
/*	{LGDISPLAY_ADDR,0x51},  
	{LGDISPLAY_DATA,0xE1},  */
	{LGDISPLAY_EOF, 0x00},
};

#endif

static struct lgdisplay_spi_data off_sequence[] = {
	{LGDISPLAY_ADDR, 0x51},
	{LGDISPLAY_DATA, 0xF1},
	{LGDISPLAY_DELAY,0xFA}, /*250ms*/
	{LGDISPLAY_ADDR, 0x28}, /*display off*/
	{LGDISPLAY_ADDR, 0xb9},
	{LGDISPLAY_DATA, 0x00}, /* DTE, DTW off */
	{LGDISPLAY_ADDR, 0xbe},
	{LGDISPLAY_DATA, 0x01}, /* DDVDH ON, VGL ON, VGH Off */

	{LGDISPLAY_DELAY, 4}, /*2ms*/

	{LGDISPLAY_ADDR, 0xBA},
	{LGDISPLAY_DATA, 0x01}, /* GPI<2>=1 (P_IC=ON)*/
	{LGDISPLAY_DELAY, 4}, /*2ms*/

	{LGDISPLAY_ADDR, 0xba}, /*sleep mode in*/
	{LGDISPLAY_DATA, 0x00}, /* GPI<2>=0 (P_IC=OFF)*/
	{LGDISPLAY_EOF,  0x00},
};

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#ifdef CONFIG_LGE_OLED_REV_C
static enum led_brightness cur_brightness_value = 14;
#if 1
static void oled_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lgdisplay_spi_data bl_data[] = {
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xE0},
		{LGDISPLAY_ADDR, 0xCA},
		{LGDISPLAY_DATA, 0x10},
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xC0},
		{LGDISPLAY_EOF,  0x00},
	};

	if (!lgdisplay_state.display_on)
		return ;

	value = value / 16 ;
	if (value > 15)
		value = 15 ;

	if (cur_brightness_value == value) {
		printk(KERN_INFO "oled_brightness_set : same value (value[%d])\n", value);
		return ;
	}

	if (cur_brightness_value == 0)
		bl_data[1].data = 0xF0;
	else if (cur_brightness_value > 0 && cur_brightness_value <= 4)
		bl_data[1].data = 0xC0;
	else if (cur_brightness_value > 4 && cur_brightness_value < 0x10 )
		bl_data[1].data = 0xE0;

	if( value == 0 ) 
		bl_data[3].data = 0x0;
	else
		bl_data[3].data = 0x10 + value;
	lgdisplay_write_table(bl_data);
	if (value == 0)
		mdelay(16);
	cur_brightness_value = value ;
	printk(KERN_INFO "oled_brightness_set(value[%d])\n", value);
}
#else
static void oled_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lgdisplay_spi_data bl_off_data[] = {
		{LGDISPLAY_ADDR, 0xBA},
		{LGDISPLAY_DATA, 0x05},
		{LGDISPLAY_ADDR, 0xCC},
		{LGDISPLAY_DATA, 0x10},
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xE1},
		{LGDISPLAY_EOF,  0x00},
	};

	struct lgdisplay_spi_data bl_on_data[] = {
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xE0},
		{LGDISPLAY_ADDR, 0xCC},
		{LGDISPLAY_DATA, 0x10},
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xE0},
		{LGDISPLAY_EOF,  0x00},
	};
	struct lgdisplay_spi_data *bl_data;

	if (!lgdisplay_state.display_on)
		return ;

	value = value / 16 ;
	if (value > 15)
		value = 15 ;

#if 1
		/* following code is just temporal!
		 * this code should be removed after brightness tuning is finished
		 * 2010-01-30, cleaneye.kim@lge.com
		 */
		if (value > 0)
			value = 0xF;
#endif

	if (cur_brightness_value == value) {
		printk(KERN_INFO "oled_brightness_set : same value (value[%d])\n", value);
		return ;
	}
	if (cur_brightness_value == 0)
		bl_data = bl_on_data;
	else 
		bl_data = bl_off_data;
	if (cur_brightness_value > 0 && cur_brightness_value <= 4) {
		bl_data[2].data = 0xCB;
		bl_data[5].data = 0xC1;
	}

	if( value == 0 ) 
		bl_data[3].data = 0x0;
	else
		bl_data[3].data = 0x10 + value;
	lgdisplay_write_table(bl_data);
	cur_brightness_value = value ;
	printk(KERN_INFO "oled_brightness_set(value[%d])\n", value);
}

#endif
#else
static void oled_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	static enum led_brightness cur_value = LED_OFF;
	struct lgdisplay_spi_data bl_1e_18[] = {
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xE1},
		{LGDISPLAY_ADDR, 0xCC},
		{LGDISPLAY_DATA, 0x10},
		{LGDISPLAY_DELAY, 0xFA},
		{LGDISPLAY_DELAY, 0xFA},
		{LGDISPLAY_EOF,  0x00},
	};

	struct lgdisplay_spi_data bl_18_11[] = {
		{LGDISPLAY_ADDR, 0x51},
		{LGDISPLAY_DATA, 0xC1},
		{LGDISPLAY_ADDR, 0xCA},
		{LGDISPLAY_DATA, 0x10},
		{LGDISPLAY_DELAY, 0xFA},
		{LGDISPLAY_DELAY, 0xFA},
		{LGDISPLAY_EOF,  0x00},
	};
	struct lgdisplay_spi_data *bl;

	if (!lgdisplay_state.display_on)
		return ;

	value = value / 16 ;
	if (value > 15)
		value = 15 ;

#if 1
	/* following code is just temporal!
	 * this code should be removed after brightness tuning is finished
	 * 2010-01-30, cleaneye.kim@lge.com
	 */
	if (value > 0)
		value = 0xF;
#endif

	if (cur_value == value) {
		printk(KERN_INFO "oled_brightness_set : same value (value[%d])\n", value);
		return ;
	}
	if (value < 0x8)
		bl = bl_18_11 ;
	else
		bl = bl_1e_18 ;

	if( value == 0 ) 
		bl[3].data = 0x0;
	else
		bl[3].data = 0x10 + value;
	lgdisplay_write_table(bl);
	cur_value = value ;
	printk(KERN_INFO "oled_brightness_set(value[%d])\n", value);
}

#endif



static struct led_classdev oled_lg4273_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.max_brightness = 255,
	.brightness_set = oled_brightness_set,
};
#endif

#ifndef CONFIG_SPI_QSD
static void lgdisplay_spi_write_byte(char dc, uint8 data)
{
	uint32 bit;
	int bnum;

	bnum = 8;	/* 8 data bits */
	bit = 0x80;
	while (bnum) {
		gpio_set_value(spi_sclk, 0); /* clk low */
		if (data & bit)
			gpio_set_value(spi_mosi, 1);
		else
			gpio_set_value(spi_mosi, 0);
		udelay(1);
		gpio_set_value(spi_sclk, 1); /* clk high */
		udelay(1);
		bit >>= 1;
		bnum--;
	}
}
#endif

static int lgdisplay_spi_write(u8 cmd, u8 data, int num)
{
#ifdef CONFIG_SPI_QSD
	u8                tx_buf[20];
	int                 rc;
	struct spi_message  m;
	struct spi_transfer t;

	if (!lcdc_lgdisplay_spi_client) {
		printk(KERN_ERR "%s lcdc_lgdisplay_spi_client is NULL\n",
			__func__);
		return -EINVAL;
	}

	memset(&t, 0, sizeof t);
	t.tx_buf = tx_buf;
	spi_setup(lcdc_lgdisplay_spi_client);
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	/* command byte first */
#if defined(SPI_MODE_16BIT_INTERFACE)
	t.len = 3;

	tx_buf[0] = cmd ;
	tx_buf[1] = 0xFF;
	tx_buf[2] = data ;
	tx_buf[3] = NULL ;
#else
	t.len = 2;

	tx_buf[0] = cmd ;
	tx_buf[1] = data ;
	tx_buf[2] = NULL ;
#endif
	t.rx_buf = NULL;

	rc = spi_sync(lcdc_lgdisplay_spi_client, &m);
	if (rc)
		printk(KERN_ERR "spi_sync _write failed %d\n", rc);
	return rc;
#else
	gpio_set_value(spi_cs, 0);	/* cs low */

	/* command byte first */
	lgdisplay_spi_write_byte(0, cmd);
	/* followed by parameter bytes */
	#if defined(SPI_MODE_16BIT_INTERFACE)
	lgdisplay_spi_write_byte(1, 0x00);
	#endif
	lgdisplay_spi_write_byte(1, data);

	gpio_set_value(spi_cs, 1);	/* cs high*/
	gpio_set_value(spi_mosi, 0);	/* mosi low*/
	udelay(2);
	return 0;
#endif
}

#ifndef CONFIG_SPI_QSD
static void spi_pin_assign(void)
{
	/* Setting the Default GPIO's */
	spi_sclk = *(lcdc_lgdisplay_pdata->gpio_num);
	spi_cs   = *(lcdc_lgdisplay_pdata->gpio_num + 1);
	spi_mosi  = *(lcdc_lgdisplay_pdata->gpio_num + 2);
	spi_miso  = *(lcdc_lgdisplay_pdata->gpio_num + 3);
}
#endif

static void lgdisplay_disp_powerup(void)
{
	int gpio = lcdc_lgdisplay_pdata->gpio;

	if (!lgdisplay_state.disp_powered_up && !lgdisplay_state.display_on) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
		gpio_set_value(gpio, 0);	/* bring reset line low to hold reset*/
		msleep(10);
		gpio_set_value(gpio, 1);
		msleep(20);
		lgdisplay_state.disp_powered_up = TRUE;
	}
}

static void lgdisplay_disp_poweroff(void)
{
	int gpio = lcdc_lgdisplay_pdata->gpio;

	if (lgdisplay_state.disp_powered_up) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
		gpio_set_value(gpio, 0);	/* bring reset line low to hold reset*/
		msleep(30);
		lgdisplay_state.disp_powered_up = FALSE;
	}
}

static void lgdisplay_write_table(struct lgdisplay_spi_data *seq)
{
	/* write the initial code for OLED*/
	while (seq->type != LGDISPLAY_EOF) {
		switch (seq->type) {
		case LGDISPLAY_ADDR:
			lgdisplay_spi_write(0x70, seq->data, 1);
			break ;

		case LGDISPLAY_DATA:
			lgdisplay_spi_write(0x72, seq->data, 1);
			break ;

		case LGDISPLAY_DELAY:
			mdelay(seq->data);
			break ;

		case LGDISPLAY_SLEEP:
			msleep_interruptible(seq->data);
			break ;
		}
		seq++ ;
	}
}

static void lgdisplay_disp_on(void)
{
	printk(KERN_INFO "lgdisplay_disp_on\n");
#ifndef CONFIG_SPI_QSD
	gpio_set_value(spi_cs, 1);	/* low */
	gpio_set_value(spi_sclk, 1);	/* high */
	gpio_set_value(spi_mosi, 0);
	gpio_set_value(spi_miso, 0);
#endif

	if (lgdisplay_state.disp_powered_up && !lgdisplay_state.display_on) {
		lgdisplay_write_table(init_sequence);
		lgdisplay_state.display_on = TRUE;
	}
}

static int lcdc_lgdisplay_panel_on(struct platform_device *pdev)
{
	if (!lgdisplay_state.disp_initialized) {
		/* Configure reset GPIO that drives DAC */
		if (lcdc_lgdisplay_pdata->panel_config_gpio)
			lcdc_lgdisplay_pdata->panel_config_gpio(1);
		lgdisplay_disp_powerup();
		lgdisplay_disp_on();
		lgdisplay_state.disp_initialized = TRUE;
	}
	return 0;
}

static int lcdc_lgdisplay_panel_off(struct platform_device *pdev)
{
	if (lgdisplay_state.disp_powered_up && lgdisplay_state.display_on) {
#ifndef CONFIG_SPI_QSD
		gpio_set_value(spi_cs, 1);	/* low */
		gpio_set_value(spi_sclk, 1);	/* high */
		gpio_set_value(spi_mosi, 0);
		gpio_set_value(spi_miso, 0);
#endif

		/* Main panel power off (Deep standby in) */
		lgdisplay_write_table(off_sequence);

		if (lcdc_lgdisplay_pdata->panel_config_gpio)
			lcdc_lgdisplay_pdata->panel_config_gpio(0);
		lgdisplay_state.display_on = FALSE;
		lgdisplay_state.disp_initialized = FALSE;
		lgdisplay_disp_poweroff();
	}
	return 0;
}

static struct msm_fb_panel_data lgdisplay_panel_data = {
	.on = lcdc_lgdisplay_panel_on,
	.off = lcdc_lgdisplay_panel_off,
/*	.set_backlight = lcdc_lgdisplay_set_backlight,*/
};

static struct platform_device this_device = {
	.name   = "lcdc_lgdisplay_wvga",
	.id	= 1,
	.dev	= {
		.platform_data = &lgdisplay_panel_data,
	}
};

ssize_t lgdisplay_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", lgdisplay_state.display_on);
}

ssize_t lgdisplay_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int onoff;
	sscanf(buf, "%d", &onoff);
	
	if (onoff) {
		lgdisplay_panel_data.on(&this_device);
	} else {
		lgdisplay_panel_data.off(&this_device);
	}

	return count;
}

DEVICE_ATTR(lcd_onoff, 0666, lgdisplay_show_onoff, lgdisplay_store_onoff);

static int lgdisplay_probe(struct platform_device *pdev)
{
	int ret ;
	if (pdev->id == 0) {
		lcdc_lgdisplay_pdata = pdev->dev.platform_data;
#ifndef CONFIG_SPI_QSD
		spi_pin_assign();
#endif
		return 0;
	}
	msm_fb_add_device(pdev);
	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	if (ret) {
		printk("lgdisplay_probe device_creat_file failed!!!\n");
	}

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
	if (led_classdev_register(&pdev->dev, &oled_lg4273_led_dev) == 0)
		printk(KERN_ERR"Registering led class dev successfully.\n");
#endif
	return 0;
}

#ifdef CONFIG_SPI_QSD
static int __devinit lcdc_lgdisplay_spi_probe(struct spi_device *spi)
{
	printk(KERN_ERR "%s: new client\n", __func__);
	lcdc_lgdisplay_spi_client = spi;
	lcdc_lgdisplay_spi_client->bits_per_word = 32;
	return 0;
}
static int __devexit lcdc_lgdisplay_spi_remove(struct spi_device *spi)
{
	lcdc_lgdisplay_spi_client = NULL;
	return 0;
}

static struct spi_driver lcdc_lgdisplay_spi_driver = {
	.driver = {
		.name  = LCDC_LGDISPLAY_SPI_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe         = lcdc_lgdisplay_spi_probe,
	.remove        = __devexit_p(lcdc_lgdisplay_spi_remove),
};
#endif
static struct platform_driver this_driver = {
	.probe  = lgdisplay_probe,
	.driver = {
		.name   = "lcdc_lgdisplay_wvga",
	},
};

static int __init lcdc_lgdisplay_panel_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;
#ifdef CONFIG_FB_MSM_TRY_MDDI_CATCH_LCDC_PRISM
	if (mddi_get_client_id() != 0)
		return 0;

	ret = msm_fb_detect_client("lcdc_lgdisplay_wvga_pt");
	if (ret)
		return 0;

#endif

	ret = platform_driver_register(&this_driver);
	if (ret)
		return ret;

	pinfo = &lgdisplay_panel_data.panel_info;
	pinfo->xres = 480;
	pinfo->yres = 800;
	MSM_FB_SINGLE_MODE_PANEL(pinfo);
	pinfo->type = LCDC_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 24;
	pinfo->fb_num = 2;
	/* 25Mhz mdp_lcdc_pclk and mdp_lcdc_pad_pcl */
	pinfo->clk_rate = 26214400; /*24576000; *//*32768000;*/
	pinfo->bl_max = 15;
	pinfo->bl_min = 1;

	pinfo->lcdc.h_back_porch = 8;	/* hsw = 8 + hbp=184 */
	pinfo->lcdc.h_front_porch = 8;
	pinfo->lcdc.h_pulse_width = 8;
	pinfo->lcdc.v_back_porch = 4;	/* vsw=1 + vbp = 2 */
	pinfo->lcdc.v_front_porch = 4;
	pinfo->lcdc.v_pulse_width = 4;
	pinfo->lcdc.border_clr = 0;     /* blk */
	pinfo->lcdc.underflow_clr = 0x0;       /* black */
	pinfo->lcdc.hsync_skew = 0;

	ret = platform_device_register(&this_device);
	if (ret) {
		printk(KERN_ERR "%s not able to register the device\n",
			 __func__);
		goto fail_driver;
	}
#ifdef CONFIG_SPI_QSD
	ret = spi_register_driver(&lcdc_lgdisplay_spi_driver);

	if (ret) {
		printk(KERN_ERR "%s not able to register spi\n", __func__);
		goto fail_device;
	}
#endif
	return ret;

#ifdef CONFIG_SPI_QSD
fail_device:
	platform_device_unregister(&this_device);
#endif
fail_driver:
	platform_driver_unregister(&this_driver);
	return ret;
}

module_init(lcdc_lgdisplay_panel_init);
