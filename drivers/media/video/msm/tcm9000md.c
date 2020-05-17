/* drivers/media/video/msm/tcm9000md.c 
*
* This software is for APTINA 1.3M sensor 
*  
* Copyright (C) 2010-2011 LGE Inc.  
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

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <linux/byteorder/little_endian.h>

#include "tcm9000md.h"

#undef CAM_MSG
#undef CAM_ERR
#undef CDBG

#define CAM_MSG(fmt, args...)   printk(KERN_INFO "msm_camera: " fmt, ##args)
#define CAM_ERR(fmt, args...)   printk(KERN_INFO "msm_camera: " fmt, ##args)
#define CDBG(fmt, args...)      printk(KERN_INFO "msm_camera: " fmt, ##args)

/* SYSCTL Registers*/
#define TCM9000MD_REG_MODEL_ID      0x00
#define TCM9000MD_MODEL_ID          0x1048

/* chanhee.park@lge.com 
   temp : we define the delay time on MSM as 0xFFFE address 
*/
#define MT9M113_REG_REGFLAG_DELAY  		0xFFFE

DEFINE_MUTEX(tcm9000md_mutex);

enum{
  MT9M113_FPS_NORMAL = 0,
  MT9M113_FPS_5 = 5,
  MT9M113_FPS_7 = 7,
  MT9M113_FPS_10 = 10,
  MT9M113_FPS_12 = 12,
  MT9M113_FPS_15 = 15,
  MT9M113_FPS_20 = 20,
  MT9M113_FPS_24 = 24,
  MT9M113_FPS_25 = 25,
  MT9M113_FPS_30 = 30
};

struct tcm9000md_ctrl_t {
	const struct msm_camera_sensor_info *sensordata;
	int8_t  previous_mode;

   /* for Video Camera */
	int8_t effect;
	int8_t wb;
	unsigned char brightness;

   /* for register write */
	int16_t write_byte;
	int16_t write_word;
};

static struct i2c_client  *tcm9000md_client;

static struct tcm9000md_ctrl_t *tcm9000md_ctrl = NULL;
static atomic_t init_reg_mode;


/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct tcm9000md_reg tcm9000md_regs;

static int32_t tcm9000md_i2c_txdata(unsigned short saddr,
	unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if (i2c_transfer(tcm9000md_client->adapter, msg, 1) < 0 ) {
		CAM_ERR("tcm9000md_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}
static int32_t tcm9000md_i2c_write( uint16_t saddr, uint8_t addr, uint8_t data )
{
    int32_t rc = -EIO;
    unsigned char buf[2];

    memset(buf, 0, sizeof(buf));
    buf[0] = addr;
    buf[1] = data;
    rc = tcm9000md_i2c_txdata(saddr, buf, 2);

    if (rc < 0){
        CAM_ERR("i2c_write failed, addr = 0x%x, val = 0x%x!\n", addr, data);
    }

    return rc;
}

static int32_t tcm9000md_i2c_write_table(
	struct tcm9000md_address_value_pair const *reg_conf_tbl,
	int num_of_items_in_table)
{
    int32_t retry;
    int32_t i;
    int32_t rc = 0;

    for (i = 0; i < num_of_items_in_table; i++) {

    if(reg_conf_tbl->register_address == MT9M113_REG_REGFLAG_DELAY){
        mdelay(reg_conf_tbl->register_value);
    }
    else{
        rc = tcm9000md_i2c_write(tcm9000md_client->addr,
                                    reg_conf_tbl->register_address,
                                    reg_conf_tbl->register_value );
    }

    if(rc < 0){
    for(retry = 0; retry < 3; retry++){
        rc = tcm9000md_i2c_write(tcm9000md_client->addr,
                                    reg_conf_tbl->register_address,
                                    reg_conf_tbl->register_value );
        if(rc >= 0)
            retry = 3;        
        }
        reg_conf_tbl++;
    }else
        reg_conf_tbl++;
    }

    return rc;
}
#if 0
static int tcm9000md_i2c_rxdata( uint16_t saddr, uint8_t *rxdata, int length)
{
	struct i2c_msg msgs[] = {
	{
		.addr   = saddr,
		.flags = 0,
		.len   = 1,
		.buf   = rxdata,
	},
	{
		.addr   = saddr,
		.flags = I2C_M_RD,
		.len   = length,
		.buf   = rxdata,
	},
	};

	if (i2c_transfer(tcm9000md_client->adapter, msgs, 2) < 0) {
		CAM_ERR("tcm9000md_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
	
}
static int32_t tcm9000md_i2c_read( uint16_t saddr,
                                   uint8_t raddr, 
	                               uint8_t *rdata,
	                               enum tcm9000md_width width )
{
	int32_t rc = 0;
	unsigned char buf[4];

	if ((!rdata) || (width > DWORD_LEN))
		return -EIO;

	memset(buf, 0, sizeof(buf));

    buf[0] = raddr;
	rc = tcm9000md_i2c_rxdata(saddr, buf, width+1);
	if (rc < 0) {
        CAM_ERR("tcm9000md_i2c_read failed!\n");
        return rc;
	}

    switch (width) {
#if 1   // Little Endian
        case DWORD_LEN :
            *(rdata+3) = buf[3];
        case BYTE_3_LEN :
            *(rdata+2) = buf[2];
        case WORD_LEN :
            *(rdata+1) = buf[1];
        case BYTE_1_LEN :
            *rdata = buf[0];
            break;
#else
        case BYTE_1_LEN :
            *rdata = buf[0];
            break;
            
        case WORD_LEN :
            *rdata = buf[1];
            *(rdata+1) = buf[0];
            break;
            
        case BYTE_3_LEN :
            *rdata = buf[2];
            *(rdata+1) = buf[1];
            *(rdata+2) = buf[0];
            break;
            
        case DWORD_LEN :
            *rdata = buf[3];
            *(rdata+1) = buf[2];
            *(rdata+2) = buf[1];
            *(rdata+3) = buf[0];
            break;
#endif
	}

	if (rc < 0)
		CAM_ERR("tcm9000md_i2c_read failed!\n");
   
	return rc;
	
}
#endif

static long tcm9000md_set_effect(int effect)
{
    CAM_MSG("tcm9000md_set_effect: effect is %d\n", effect);

    return 0;
}

static long tcm9000md_set_wb(int8_t wb)
{
    CAM_MSG("tcm9000md_set_wb : called, new wb: %d\n", wb);

    tcm9000md_ctrl->wb = wb;

    return 0;
}

/* =====================================================================================*/
/* tcm9000md_set_brightness                                                                        								    */
/* =====================================================================================*/
static long tcm9000md_set_brightness(int8_t brightness)
{
    long rc = 0;

    CAM_MSG("tcm9000md_set_brightness: %d\n", brightness);	

    tcm9000md_ctrl->brightness = brightness;

    return rc;
}

long tcm9000md_set_preview_fps(uint16_t fps)
{
    long rc = 0;

    CAM_MSG("[tcm9000md] fps[%d] \n",fps);

    return rc;
}

static long tcm9000md_set_sensor_mode(int mode)
{
    CAM_MSG("tcm9000md_set_sensor_mode: sensor mode is PREVIEW : %d\n", mode);

	tcm9000md_ctrl->previous_mode = mode;
   
	return 0;
}

/* =====================================================================================*/
/*  tcm9000md_sensor_config                                                                        								    */
/* =====================================================================================*/
int tcm9000md_sensor_config(void __user *argp)
{
    struct sensor_cfg_data cfg_data;
    long   rc = 0;

    CAM_MSG("tcm9000md_sensor_config...................\n");	

    if (copy_from_user(&cfg_data,(void *)argp,sizeof(struct sensor_cfg_data)))
        return -EFAULT;

    CAM_MSG("tcm9000md_ioctl, cfgtype = %d, mode = %d\n", cfg_data.cfgtype, cfg_data.mode);

    mutex_lock(&tcm9000md_mutex);

    msleep(10);

    switch (cfg_data.cfgtype) {
    case CFG_SET_MODE:
        CAM_MSG("tcm9000md_sensor_config: command is CFG_SET_MODE\n");
        rc = tcm9000md_set_sensor_mode(cfg_data.mode); 
        break;

    case CFG_SET_EFFECT:
        CAM_MSG("tcm9000md_sensor_config: command is CFG_SET_EFFECT\n");
        rc = tcm9000md_set_effect(cfg_data.cfg.effect);
        break;

    case CFG_SET_WB:
        CAM_MSG("tcm9000md_sensor_config: command is CFG_SET_WB\n");
        //rc = tcm9000md_set_wb(cfg_data.wb);
        rc = tcm9000md_set_wb(0);
        break;

    case CFG_SET_BRIGHTNESS:
        CAM_MSG("tcm9000md_sensor_config: command is CFG_SET_BRIGHTNESS\n");
        //rc = tcm9000md_set_brightness(cfg_data.brightness);
        rc = tcm9000md_set_brightness(0);
        break;

    case CFG_SET_FPS:
        CAM_MSG("%s:CFG_SET_FPS : user fps[%d]\n",__func__, cfg_data.cfg.fps.fps_div);
        rc = tcm9000md_set_preview_fps(cfg_data.cfg.fps.fps_div);
        break;

    default:	 	
        CAM_MSG("tcm9000md_sensor_config:cfg_data.cfgtype[%d]\n",cfg_data.cfgtype);
        rc = 0;//-EINVAL; 
        break;
    }

    mutex_unlock(&tcm9000md_mutex);

    if (rc < 0){
  	    CAM_ERR("tcm9000md: ERROR in sensor_config, %ld\n", rc);
    }

    return rc;
}

/* =====================================================================================*/
/* 	tcm9000md sysfs                                                                          									   */
/* =====================================================================================*/
static ssize_t tcm9000md_mclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	
	//vt_cam_mclk_rate = value;

	printk("[tcm9000md] mclk_rate = %d\n", value);

	return size;
}
static DEVICE_ATTR(mclk, S_IRWXUGO, NULL, tcm9000md_mclk_store);

static ssize_t tcm9000md_init_code_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;

	if (tcm9000md_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	CAM_ERR("tcm9000md: init code type [0x%x] \n",val);

#if 0
    /* 0 : main camera to support the YUV snapshot
           1 : secondary camera
           2 : main camera to support the preview with 720p, 30 fps
           3 : secondary camera 30 fps
      */
	if(val != CAMERA_TYPE_SECONDARY && val != CAMERA_TYPE_SECONDARY_FOR_VT) {
		CAM_ERR("tcm9000md: invalid init. type[%d] \n",val);
		val = CAMERA_TYPE_SECONDARY;
	}
#endif

	atomic_set(&init_reg_mode,0);
	
	return n;
}

static DEVICE_ATTR(init_code, S_IRUGO|S_IWUGO, NULL, tcm9000md_init_code_store);

static struct attribute* tcm9000md_sysfs_attrs[] = {
	&dev_attr_mclk.attr,
	&dev_attr_init_code.attr,
	NULL
};

static int tcm9000md_sysfs_add(struct kobject* kobj)
{
	int i, n, ret;
	
	n = ARRAY_SIZE(tcm9000md_sysfs_attrs);
	for(i = 0; i < n; i++){
		if(tcm9000md_sysfs_attrs[i]){
			ret = sysfs_create_file(kobj, tcm9000md_sysfs_attrs[i]);
			if(ret < 0){
				CAM_ERR("tcm9000md sysfs is not created\n");
			}
		}
	}

	return 0;
}
/*======================================================================================*/
/*  end :  sysfs                                                                         										    */
/*======================================================================================*/


/* =====================================================================================*/
/*  tcm9000md_sensor_init                                                                        								    */
/* =====================================================================================*/
int tcm9000md_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc =0;

    CDBG("tcm9000md_sensor_init\n");
    
	tcm9000md_ctrl = (struct tcm9000md_ctrl_t *)kzalloc(sizeof(struct tcm9000md_ctrl_t), GFP_KERNEL);  
	if (!tcm9000md_ctrl) {
		CAM_ERR("tcm9000md_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}
	
	if (data)
		tcm9000md_ctrl->sensordata = data;
	else
		goto init_done;

	data->pdata->camera_power_on();

	printk("[tcm9000md_sensor_init] init_reg_mode NORMARL.....\n");
	rc = tcm9000md_i2c_write_table(&tcm9000md_regs.init_reg_settings[0],
		 tcm9000md_regs.init_reg_settings_size);
	
	if(rc < 0 ){
	   CAM_ERR("tcm9000md: tcm9000md writing fail\n");
	   goto init_done; 
	}
	
	tcm9000md_ctrl->previous_mode = SENSOR_PREVIEW_MODE;

	return rc;

init_done:
	CAM_ERR("tcm9000md:tcm9000md_sensor_init failed\n");
	return rc;

}
/* =====================================================================================*/
/*  tcm9000md_sensor_release                                                                        								    */
/* =====================================================================================*/
int tcm9000md_sensor_release(void)
{
    CDBG("tcm9000md_sensor_release\n");
    
    if(tcm9000md_ctrl)
        tcm9000md_ctrl->sensordata->pdata->camera_power_off();

    if(tcm9000md_ctrl)
        kfree(tcm9000md_ctrl);

	return 0;	
}

/* =====================================================================================*/
/*  tcm9000md_sensor_prove function                                                                        								    */
/* =====================================================================================*/
#if 0
static int tcm9000md_probe_init_done(const struct msm_camera_sensor_info *data)
{
	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
	return 0;
}

static int tcm9000md_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	unsigned short chipid;
	CDBG("%s: %d\n", __func__, __LINE__);
	rc = gpio_request(data->sensor_reset, "tcm9000md");
	CDBG(" tcm9000md_probe_init_sensor \n");

	if (rc){
		CDBG(" tcm9000md_probe_init_sensor 2\n");
		goto init_probe_done;
	}
	
	CDBG(" tcm9000md_probe_init_sensor TCM9000MD_REG_MODEL_ID is read  \n");

    CDBG("tcm9000md_probe_init_sensor Power On\n");
	data->pdata->camera_power_on();
	    
	/* 3. Read sensor Model ID: */
	rc = tcm9000md_i2c_read(tcm9000md_client->addr, TCM9000MD_REG_MODEL_ID,
	                        (uint8_t*)&chipid, WORD_LEN );
	
    CDBG("tcm9000md_probe_init_sensor Power Off\n");
	data->pdata->camera_power_off();
	    
	if (rc < 0)
		goto init_probe_fail;
	CDBG("tcm9000md model_id = 0x%x\n", chipid);

	/* 4. Compare sensor ID to TCM9000MD ID: */
	if (chipid != TCM9000MD_MODEL_ID) {
		rc = -ENODEV;
		goto init_probe_fail;
	}

	goto init_probe_done;
init_probe_fail:
	CDBG(" tcm9000md_probe_init_sensor fails\n");
init_probe_done:
	CDBG(" tcm9000md_probe_init_sensor finishes\n");
	tcm9000md_probe_init_done(data);
	return rc;
}
#endif

static int tcm9000md_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int rc = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	tcm9000md_client = client;

	rc = tcm9000md_sysfs_add(&client->dev.kobj);

	return rc;

probe_failure:	
	CAM_ERR("tcm9000md_i2c_probe.......rc[%d]....................\n",rc);
	return rc;		
	
}

static int tcm9000md_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id tcm9000md_i2c_ids[] = {
	{"tcm9000md", 0},
	{ /* end of list */},
};

static struct i2c_driver tcm9000md_i2c_driver = {
	.probe  = tcm9000md_i2c_probe,
	.remove = tcm9000md_i2c_remove, 
	.id_table = tcm9000md_i2c_ids,	
	.driver   = {
		.name =  "tcm9000md",
		.owner= THIS_MODULE,	
    },
};

static int tcm9000md_sensor_probe(const struct msm_camera_sensor_info *info,
	                                 	struct msm_sensor_ctrl *s)
{
	int rc = 0;

	CAM_MSG("tcm9000md_sensor_probe...................\n");

	rc = i2c_add_driver(&tcm9000md_i2c_driver);
	if (rc < 0) {
		rc = -ENOTSUPP;
		CAM_ERR("[tcm9000md_sensor_probe] return value :ENOTSUPP\n");
		goto probe_fail;
	}

	atomic_set(&init_reg_mode, 0);

#if 0   // Check Sensor ID
    rc = tcm9000md_probe_init_sensor(info);
    if (rc < 0)
        goto probe_fail;
#endif

	s->s_init    = tcm9000md_sensor_init;
	s->s_release = tcm9000md_sensor_release;
	s->s_config  = tcm9000md_sensor_config;
    s->s_camera_type = FRONT_CAMERA_2D;
	s->s_mount_angle = 0;
	
    CAM_MSG("tcm9000md.c : tcm9000md_sensor_probe - complete : %d \n", rc);
    return 0;

probe_fail:
    CAM_MSG("tcm9000md.c : tcm9000md_sensor_probe - Fail : %d \n", rc);
	return rc;
}
static int __tcm9000md_probe(struct platform_device *pdev)
{	
	return msm_camera_drv_start(pdev,tcm9000md_sensor_probe);	
}

static struct platform_driver msm_camera_driver = {
	.probe = __tcm9000md_probe,
	.driver = {
		.name = "msm_camera_tcm9000md",
		.owner = THIS_MODULE,
	},
};
static int __init tcm9000md_init(void)
{
	CAM_MSG("tcm9000md_init...................\n");
		
	return platform_driver_register(&msm_camera_driver);  
}

module_init(tcm9000md_init);

