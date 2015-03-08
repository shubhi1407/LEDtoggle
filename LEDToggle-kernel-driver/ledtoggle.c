/*
 * Kernel module for toggling LEDs through PRU0 using remoteproc.
 * This module uses remoteproc binding for Beaglelogic developed by:
 * Copyright (C) 2014 Kumar Abhishek <abhishek@theembeddedkitchen.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */


#include <asm/uaccess.h>

#include <linux/module.h>
#include <linux/err.h>
#include <linux/kernel.h> 
#include <linux/init.h>

#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <linux/slab.h>
#include <linux/genalloc.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>

#include <linux/kobject.h>
#include <linux/string.h>

#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_device.h>

#include <linux/sysfs.h>
#include <linux/fs.h>

#include "beaglelogic.h"
#include "beaglelogic_glue.h"

#define DRV_NAME	"ledtoggle"
#define DRV_VERSION	"1.1"
/* PRU Downcall API */

#define BL_DC_SM_TOGGLE		8   /* DC to toggle LED */

/* Forward declration */
static const struct file_operations pru_beaglelogic_fops;

struct beaglelogicdev {
	/* Misc device descriptor */
	struct miscdevice miscdev;

	/* Imported functions */
	int (*downcall_idx)(int, u32, u32, u32, u32, u32, u32);
	void __iomem *(*d_da_to_va)(int, u32);
	int (*pru_start)(int);
	void (*pru_request_stop)(void);

	/* Private data */
	struct device *p_dev; /* Parent platform device */

};

void toggle_start(struct device *dev)
{
	struct beaglelogicdev *bldev = dev_get_drvdata(dev);
	bldev->pru_start(0);
	bldev->downcall_idx(0, BL_DC_SM_TOGGLE, 0, 0, 0, 0, 0);
	dev_info(dev, "Toggled");	
}


static ssize_t bl_ledtoggle_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t bl_ledtoggle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u32 val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
	{
		printk( "Starting" );
		toggle_start(dev);
	}
	
	printk( "Value written : %d\n",val );
	return count;
}
// Registering sysfs file
static DEVICE_ATTR(ledtoggle, S_IWUSR | S_IRUGO,
		bl_ledtoggle_show, bl_ledtoggle_store);


static struct attribute *beaglelogic_attributes[] = {
	&dev_attr_ledtoggle.attr,
	NULL
};

static struct attribute_group beaglelogic_attr_group = {
	.attrs = beaglelogic_attributes
};
/* end sysfs attr */
//PROBE
static int ledtoggle_probe(struct platform_device *pdev)
{
	//struct device_node *node = pdev->dev.of_node;
	int err;
	struct beaglelogicdev *bldev;
	struct device *dev;
	
	printk("LEDToggle loaded and initializing\n");

	/* Allocate memory for our private structure */
	bldev = kzalloc(sizeof(*bldev), GFP_KERNEL);
	if (!bldev)
		goto fail;

	bldev->miscdev.fops = &pru_beaglelogic_fops;
	bldev->miscdev.minor = MISC_DYNAMIC_MINOR;
	bldev->miscdev.mode = S_IRUGO;
	bldev->miscdev.name = "beaglelogic";

	
	/* Link the platform device data to our private structure */
	bldev->p_dev = &pdev->dev;
	dev_set_drvdata(bldev->p_dev, bldev);

	/* Bind to the pru_rproc module */
	err = pruproc_beaglelogic_request_bind((void *)bldev);
	if (err)
		goto fail;

	/* Once done, register our misc device and link our private data */
	err = misc_register(&bldev->miscdev);
	if (err)
		goto fail;
	dev = bldev->miscdev.this_device;
	dev_set_drvdata(dev, bldev);

	/* Once done, create device files */
	err = sysfs_create_group(&dev->kobj, &beaglelogic_attr_group);
	if (err) {
		dev_err(dev, "Registration failed.\n");
		goto faildereg;
	}

	return 0;
faildereg:
	misc_deregister(&bldev->miscdev);
	kfree(bldev);
fail:
	return -1;
}

static int ledtoggle_remove(struct platform_device *pdev)
{
	struct beaglelogicdev *bldev = platform_get_drvdata(pdev);
	struct device *dev = bldev->miscdev.this_device;

	/* Unregister ourselves from the pru_rproc module */
	pruproc_beaglelogic_request_unbind();

	/* Remove the sysfs attributes */
	sysfs_remove_group(&dev->kobj, &beaglelogic_attr_group);

	/* Deregister the misc device */
	misc_deregister(&bldev->miscdev);

	/* Free up memory */
	kfree(bldev);

	/* Print a log message to announce unloading */
	printk("LEDToggle unloaded\n");
	return 0;
}

static const struct of_device_id ledtoggle_dt_ids[] = {
	{ .compatible = "beaglelogic,beaglelogic", .data = NULL, },
	{ /* sentinel */ },
};

static struct platform_driver ledtoggle_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = ledtoggle_dt_ids,
	},
	.probe = ledtoggle_probe,
	.remove = ledtoggle_remove,
};

module_platform_driver(ledtoggle_driver);

MODULE_AUTHOR("Shubhangi Gupta");
MODULE_DESCRIPTION("Kernel Driver for LEDToggle");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
