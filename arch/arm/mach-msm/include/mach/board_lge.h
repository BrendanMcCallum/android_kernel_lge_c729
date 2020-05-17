
/*
  * arch/arm/mach-msm/include/mach/board_lge.h
  * Copyright (C) 2010 LGE Corporation.
  *
  * This software is licensed under the terms of the GNU General Public
  * License version 2, as published by the Free Software Foundation, and
  * may be copied, distributed, and modified under those terms.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  */

#ifndef __ASM_ARCH_MSM_BOARD_LGE_H
#define __ASM_ARCH_MSM_BOARD_LGE_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/rfkill.h>
#include <linux/platform_device.h>
#include <asm/setup.h>

#if __GNUC__
#define __WEAK __attribute__((weak))
#endif

#define VENDOR_LGE	0x1004

#ifdef CONFIG_ARCH_MSM7X30

#if 0 // original

#define MSM_PMEM_SF_SIZE	0x1700000
#define MSM_FB_SIZE		0x500000
#define MSM_GPU_PHYS_SIZE       SZ_2M
   
#define MSM_PMEM_ADSP_SIZE      0x2000000
//#define MSM_PMEM_ADSP_SIZE      0x1800000
 
#define MSM_FLUID_PMEM_ADSP_SIZE	0x2800000
#define PMEM_KERNEL_EBI1_SIZE   0x600000
#define MSM_PMEM_AUDIO_SIZE     0x200000

#endif

 
#ifdef CONFIG_MACH_MSM8X55_FLIP

#define MSM_FB_SIZE					0x1C2000 // (= 320*720*4(32bpp)*2(double buffering)) 
#define MSM_PMEM_ADSP_SIZE      	0x1D1A000
#define MSM_PMEM_SF_SIZE			0x1700000// (this can be zero if TARGET_GRALLOC_USES_ASHMEM enabled) 
#define PMEM_KERNEL_EBI1_SIZE   	0x600000
#define MSM_PMEM_AUDIO_SIZE     	0x200000
#define MSM_FLUID_PMEM_ADSP_SIZE	0x2800000
#define MSM_GPU_PHYS_SIZE       	SZ_2M

#else

#define MSM_FB_SIZE           		0x12C000 // (= 320*480*4(32bpp)*2(double buffering)) 
#define MSM_PMEM_ADSP_SIZE    		0x1D1A000 
#define MSM_PMEM_SF_SIZE      		0x1000000 //(this can be zero if TARGET_GRALLOC_USES_ASHMEM enabled) 
#define PMEM_KERNEL_EBI1_SIZE 		0x600000 
#define MSM_PMEM_AUDIO_SIZE   		0x200000 
#define MSM_FLUID_PMEM_ADSP_SIZE	0x2800000
#define MSM_GPU_PHYS_SIZE     		SZ_2M

#endif
 


#ifdef CONFIG_ANDROID_RAM_CONSOLE
#define LGE_RAM_CONSOLE_SIZE    (124 * SZ_1K)
#endif

#ifdef CONFIG_LGE_HANDLE_PANIC
#define LGE_CRASH_LOG_SIZE		(4 * SZ_1K)
#endif

/* board revision information */
enum {
	EVB         = 0,
	LGE_REV_A,
	LGE_REV_B,
	LGE_REV_C,
	LGE_REV_D,
	LGE_REV_E,
	LGE_REV_F,
	LGE_REV_10,
	LGE_REV_11,
	LGE_REV_12,
	LGE_REV_13,
	LGE_REV_TOT_NUM,
};

extern int lge_bd_rev;
extern int lge_get_hw_rev(void);


/* define gpio pin number of i2c-gpio */
struct gpio_i2c_pin {
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int reset_pin;
	unsigned int irq_pin;
};

/* atcmd virtual keyboard platform data */
struct atcmd_virtual_platform_data {
	unsigned int keypad_row;
	unsigned int keypad_col;
	unsigned int *keycode;
};

struct key_touch_platform_data {
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
	unsigned int *keycode;
	int keycodemax;
};

/* touch screen platform data */
#ifdef CONFIG_TOUCHSCREEN_QT602240
struct qt602240_platform_data {
	unsigned int x_line;
	unsigned int y_line;
	unsigned int x_size;
	unsigned int y_size;
	unsigned int blen;
	unsigned int threshold;
	unsigned int voltage;
	unsigned char orient;
	int (*power)(unsigned char onoff);
	int gpio_int;	
	int irq;
	int scl;
	int sda;
	int hw_i2c;	
};
#else  // univaQ uses here
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_x_scrn_max;
	int ts_y_min;
	int ts_y_max;
	int ts_y_start;
	int ts_y_scrn_max;
	int (*power)(unsigned char onoff);
	int irq;
	int gpio_int;
	int hw_i2c;
	int scl;
	int sda;
	int ce;
	int reset;
};
#endif


/* acceleration platform data */
struct acceleration_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
};
/* kr3dh acceleration platform data */
#ifdef CONFIG_MACH_MSM8X55_FLIP
struct kr3dh_platform_data {
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*kr_init)(void);
	void (*kr_exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
	int (*gpio_config)(int config);
};
#endif


/* proximity platform data */
struct proximity_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
	int methods;
	int operation_mode;
	int debounce;
	u8 cycle;
};
#ifdef CONFIG_MACH_MSM8X55_FLIP
struct apds9900_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
};
#endif


/* ecompass platform data */ 
struct ecom_platform_data {
	int pin_int;
	int pin_rst;
	int (*power)(unsigned char onoff);
	char accelerator_name[20];
	int fdata_sign_x;
        int fdata_sign_y;
        int fdata_sign_z;
	int fdata_order0;
	int fdata_order1;
	int fdata_order2;
	int sensitivity1g;
	s16 *h_layout;
	s16 *a_layout;
};
// cg.kim@lge.com ecompass akm8975C
#ifdef CONFIG_MACH_MSM8X55_FLIP
struct akm8975_platform_data {
	int     (*power)(unsigned char onoff); // TODO : power off implement
	char 	accelerator_name[20];
	//under 7 item is for motion sensor's data (KR3DH)
	int 	fdata_sign_x;
    int 	fdata_sign_y;
    int 	fdata_sign_z;
	int 	fdata_order0;
	int 	fdata_order1;
	int 	fdata_order2;
	int 	sensitivity1g;	
};
#endif


/* android vibrator platform data */
struct lge_vibrator_platform_data {
	int enable_status;
	int (*power_set)(int enable); 		/* LDO Power Set Function */
	int (*pwm_set)(int enable, int gain); 		/* PWM Set Function */
	int (*ic_enable_set)(int enable); 	/* Motor IC Set Function */
	int amp_value;				/* PWM tuning value */
};

/* bt platform data */
struct bluetooth_platform_data {
	int (*bluetooth_power)(int on);
	int (*bluetooth_toggle_radio)(void *data, bool blocked);
};

struct bluesleep_platform_data {
	int bluetooth_port_num;
};

//[sumeet.gupta ONE_CODE 040411
struct msm_panel_ebi2lcd_pdata {
        int gpio;
        int (*backlight_level)(int level, int max, int min);
        int (*pmic_backlight)(int level);
        int (*panel_num)(void);
        void (*panel_config_gpio)(int);
        int *gpio_num;
        int initialized;
};
//]sumeet.gupta ONE_CODE 040411

typedef void (gpio_i2c_init_func_t)(int bus_num);
int __init init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data);

void __init msm_add_fb_device(void);
void __init msm_add_pmem_devices(void);
void __init msm_add_kgsl_device(void);
void __init msm7x30_allocate_memory_regions(void);
void __init msm_add_usb_devices(void);
void __init msm7x30_init_marimba(void);
void __init register_board_info(void);

/* implement in board-victor-pm.c */
int __init pmic8058_buses_init(void);
void __init pmic8058_leds_init(void);
void __init lge_pm_set_platform_data(void);

/* implement in board-victor-misc.c */
void __init lge_add_mmc_devices(void);
void __init lge_add_misc_devices(void);

/* implement in board-victor-sound.c */
int __init aux_pcm_gpio_init(void);
void __init lge_victor_audio_init(void);

/* implement in board-victor-mmc.c */
void __init lge_add_mmc_devices(void);

/* implement in board-victor-input.c */
void __init lge_add_input_devices(void);

/* implement in board-victor-bt.c */
void __init lge_add_btpower_devices(void);

void __init lge_add_gpio_i2c_device(gpio_i2c_init_func_t *init_func);
void __init lge_add_gpio_i2c_devices(void);

void __init lge_add_lcd_devices(void);
void __init msm_qsd_spi_init(void);

/* implement in board-victor-camera.c */
void __init lge_add_camera_devices(void);

/* for interaction with LK loader */
int __init lge_get_uart_mode(void);

#ifdef CONFIG_ANDROID_RAM_CONSOLE
void __init lge_add_ramconsole_devices(void);
#endif

#if defined(CONFIG_ANDROID_RAM_CONSOLE) && defined(CONFIG_LGE_HANDLE_PANIC)
void __init lge_add_panic_handler_devices(void);
void lge_set_reboot_reason(unsigned int reason);
#endif

#ifdef CONFIG_LGE_DETECT_PIF_PATCH
unsigned lge_get_pif_info(void);
#endif

unsigned lge_get_nv_qem(void);

unsigned lge_get_nv_frststatus(void);
void lge_set_nv_frststatus(unsigned char flag);

 
#ifdef CONFIG_LGE_HIDDEN_RESET_PATCH
extern int hidden_reset_enable;
extern int on_hidden_reset;
void *lge_get_fb_addr(void);
void *lge_get_fb_copy_virt_addr(void);
void *lge_get_fb_copy_phys_addr(void);
void *lge_get_fb_copy_virt_rgb888_addr(void);
unsigned int lge_get_fb_phys_addr(void);
#endif // CONFIG_LGE_HIDDEN_RESET_PATCH
 

#endif

#endif
