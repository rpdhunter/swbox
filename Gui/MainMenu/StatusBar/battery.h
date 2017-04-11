#ifndef BATTERY_H
#define BATTERY_H

/********************************************************************
    created:	2017/02/27
    created:	27:2:2017   10:53
    filename: 	Z:\pd\uart_test\uart_test.c
    file path:	Z:\pd\uart_test
    file base:	uart_test
    file ext:	c
    author:
    purpose:	uart test programme
*********************************************************************/

//串口相关的头文件
#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "IO/Modbus/gpio_oper.h"

#define I2C_DEV_0		"/dev/i2c-0"

#define ADM1191_START_ADDR		0x30
#define ADM1191_END_ADDR		0x3f

#define ADM1191_ADDR			0x3f

#define ADM_VOL_FULLSCALE_HI	26.52f
#define ADM_VOL_FULLSCALE_LO	6.65f
#define ADM_CUR_FULLSCALE		105.84f
#define ADM_SENSE_RES			0.05f

#define ADM1191_CONV_PIN		61
#define ADM1191_ALAM_PIN		62

#define BAT_PWR_ALARM_PER		20
#define BAT_PWR_LOSS_PER		0

#define BAT_PWR_PER_NUM			10

/* 5%一个电压 */
static float bat_pwr_percent [BAT_PWR_PER_NUM] = {
    //4.12f * 2,		/* 95% */
    4.08f * 2,		/* 90% */
    //4.03f * 2,		/* 85% */
    3.97f * 2,		/* 80% */
    //3.93f * 2,		/* 75% */
    3.90f * 2,		/* 70% */
    //3.87f * 2,		/* 65% */
    3.84f * 2,		/* 60% */
    //3.81f * 2,		/* 55% */
    3.79f * 2,		/* 50% */
    //3.77f * 2,		/* 45% */
    3.75f * 2,		/* 40% */
    //3.74f * 2,		/* 35% */
    3.73f * 2,		/* 30% */
    //3.72f * 2,		/* 25% */
    3.71f * 2,		/* 20% */
    //3.69f * 2,		/* 15% */
    3.65f * 2,		/* 10% */
    //3.63f * 2,		/* 5% */
    3.58f * 2,		/* 1% */
};

//class Battery : public QObject
class Battery
{
public:
    explicit Battery();

    int battValue();        //返回当前电量


private:
    typedef struct battery_power_s {
        unsigned char adm1191_addr;		/* 电源监视芯片地址 */
        float vol;						/* 电池电压 */
        float cur;						/* 电池输出电流 */
        unsigned int power;				/* 0-100 */
        unsigned int lo_pwr_alarm;		/* 低电量告警 */
        unsigned int pwr_loss_alarm;	/* 失电告警 */
    } battery_power_t;

    battery_power_t battery_power;

    int adm1191_conv_init ();

    int adm1191_conv_set (int value);

    int i2c_adm1191_init (unsigned char addr);

    int i2c_adm1191_read (unsigned char addr, unsigned short * voltage, unsigned short * current);

    int i2c_adm1191_probe (battery_power_t * bp);

    int conv_vol (unsigned short vol, float * vol_f, int hi);

    int conv_cur (unsigned short cur, float * cur_f);

    int calc_bat_pwr_percent (battery_power_t * bp);

    int init_battery_power (battery_power_t * bp);

    int check_battery_power (battery_power_t * bp);



};

#endif // BATTERY_H
