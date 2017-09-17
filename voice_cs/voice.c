
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/input/matrix_keypad.h>
#include <linux/gpio_keys.h>
#include <linux/usb/android_composite.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>

#include <linux/regulator/machine.h>
#include <linux/i2c/twl.h>

#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>
#include <linux/poll.h>

#include <linux/interrupt.h>

//#define DEBUGDRV 0


#ifdef DEBUGDRV
#define dbg(format,...) printk("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define dbg(format,...)
#endif



#define 	GPIO_BASE1 	0
#define 	GPIO_BASE2 	34
#define 	GPIO_BASE3 	64
#define 	GPIO_BASE4 	96
#define 	GPIO_BASE5 	128
#define 	GPIO_BASE6 	160

/*
GPIO6 --------------- gpio_[191:160]
GPIO5 --------------- gpio_[159:128]
GPIO4 --------------- gpio_[127:96]
GPIO3 --------------- gpio_[95:64]
GPIO2 --------------- gpio_[63:34]
GPIO1 --------------- gpio_[31:0]
*/
//使能基地址
#define 	GPIO_OE_BASE1 		0x48310034
#define 	GPIO_OE_BASE2 		0x49050034
#define 	GPIO_OE_BASE3 		0x49052034
#define 	GPIO_OE_BASE4 		0x49054034
#define 	GPIO_OE_BASE5 		0x49056034
#define 	GPIO_OE_BASE6 		0x49058034


//输出基地址
#define 	GPIO_DATAOUT_BASE1 	0x4831003C
#define 	GPIO_DATAOUT_BASE2 	0x4905003C
#define 	GPIO_DATAOUT_BASE3 	0x4905203C
#define 	GPIO_DATAOUT_BASE4 	0x4905403C
#define 	GPIO_DATAOUT_BASE5 	0x4905603C
#define 	GPIO_DATAOUT_BASE6 	0x4905803C

//输入基地址
#define 	GPIO_DATAIN_BASE1 	0x48310038
#define 	GPIO_DATAIN_BASE2 	0x49050038
#define 	GPIO_DATAIN_BASE3 	0x49052038
#define 	GPIO_DATAIN_BASE4 	0x49054038
#define 	GPIO_DATAIN_BASE5 	0x49056038
#define 	GPIO_DATAIN_BASE6 	0x49058038




//step one : 查看引脚配置

//IO引脚实际配置地址，datasheet:P2454，
#define GPIO161_MODE_CONF 0x48002194 //bit31:16			



volatile unsigned long* gpio_oe_base6 			= NULL;
volatile unsigned long* gpio_dataout_base6 		= NULL; //read only
volatile unsigned long* gpio161_mode 			= NULL;


static struct class * voice_class;
static struct device* voice_class_dev;

//static int irq;

int  voice_drv_open(struct inode *inode, struct file * file)
{

	dbg("open\n");
	//step three : IO模式选择及配置
	//在数据手册的2444页，低三位为模式选择
	//配置 gpio161 复用模式为输入
	*gpio161_mode &= ~(0x00070000);		//将低三位模式选择清零
	*gpio161_mode |= (0x00040000);			//选择mode 4 for gpio

	*gpio_oe_base6 &= ~(0x1<<(161 - GPIO_BASE6));	//GPIO_OE 使能输出，datasheet p3512

	return 0;
}



ssize_t voice_drv_read(struct file *file, char __user *buf, size_t size, loff_t *loff_t)
{
	return 0;
}

ssize_t voice_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val=0;
	int ret;
	if(count != 1)
		return -1;


	printk(KERN_INFO "voice_drv_write");

	//dbg("count = %d\n",count);
	ret = copy_from_user(&val,buf,count);
	//dbg("ret = %d\n",ret);
	printk(KERN_INFO "wirte val  = %d\n",val);
	if(val > 0)
	{
		printk(KERN_INFO "wirte val  = %d\n",val);

		*gpio_dataout_base6 |= (1<<(161 - GPIO_BASE6));
	}
	else
	{
		printk(KERN_INFO "write pwrkey %d\n",val);

	    *gpio_dataout_base6 &= ~(1<<(161 - GPIO_BASE6));
	}

	return 0;
}


int voice_drv_release(struct inode *inode, struct file *file)
{
	return 0;

}

static void GpioMap(void)
{
	//remap mode address
	//映射复用模式配置寄存器
	//step two : IO映射
	gpio161_mode = (unsigned long*)ioremap(GPIO161_MODE_CONF,4);
	gpio_oe_base6 = (unsigned long*)ioremap(GPIO_OE_BASE6,4);
	gpio_dataout_base6 = (unsigned long*)ioremap(GPIO_DATAOUT_BASE6,4);

	printk(KERN_INFO "remap io \n");
}

static void GpioUnMap(void)
{

	iounmap(gpio_dataout_base6);
	iounmap(gpio_oe_base6);
	iounmap(gpio161_mode);

	dbg("remap io \n");
}



static struct file_operations voice_drv_ops = {

		.owner   = THIS_MODULE,
		.open    = voice_drv_open,
		.read    = voice_drv_read,
		.write   = voice_drv_write,
		.release = voice_drv_release,
};

int major;
static int voice_drv_init(void)
{

	major = register_chrdev(0,"voice_cs",&voice_drv_ops);

	voice_class = class_create(THIS_MODULE,"voice_cs");
	voice_class_dev = device_create(voice_class,NULL,MKDEV(major,0),NULL,"voice_cs");// dev/key
	GpioMap();

	printk(KERN_INFO "voice_cs driver init\n");

	return 0;
}

static void voice_drv_exit(void)
{

	printk(KERN_INFO "voice_cs driver exit\n");
	unregister_chrdev(major,"voice_cs");

	class_destroy(voice_class);
	device_unregister(voice_class_dev);
	GpioUnMap();
}



module_init(voice_drv_init);
module_exit(voice_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("caredrive, Inc.");
MODULE_AUTHOR("leozlee <leozlee@163.com>");





