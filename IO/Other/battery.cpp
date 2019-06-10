#include "battery.h"
#include <QtDebug>
#include "Gui/Common/common.h"
#include "IO/Com/rdb/rdb.h"

#define  mA5000   1
#define  mA4000   0
#define  mA2800   0
#define  mA3000   0
/* 5%一个电压 */
static float bat_pwr_percent [BAT_PWR_PER_NUM] = {
    4.00f * 2 - DELT_VOL ,		/* 90% */
    3.94f * 2 - DELT_VOL,		/* 80% */
    3.88f * 2 - DELT_VOL,		/* 70% */
    3.82f * 2 - DELT_VOL,		/* 60% */
    3.76f * 2 - DELT_VOL,		/* 50% */
    3.70f * 2 - DELT_VOL,		/* 40% */
    3.64f * 2 - DELT_VOL,		/* 30% */
    3.58f * 2 - DELT_VOL,		/* 20% */
    3.52f * 2 - DELT_VOL,		/* 10% */
//    6.7,
    3.46f * 2 - DELT_VOL,		/* 1% */
//    6.6,
};

static float bat_charge_delt_vol [BAT_PWR_PER_NUM + 1] = {
#if mA5000
    0.05,						/* 90% */
    0.05,						/* 80% */
    0.08,						/* 70% */
    0.08,						/* 60% */
    0.08,						/* 50% */
    0.10,						/* 40% */
    0.10,						/* 30% */
    0.10,						/* 20% */
    0.12,						/* 10% */
    0.12,						/* 1% */
    0.12
#elif mA4000
    0.10,						/* 90% */
    0.10,						/* 80% */
    0.10,						/* 70% */
    0.15,						/* 60% */
    0.15,						/* 50% */
    0.15,						/* 40% */
    0.15,						/* 30% */
    0.15,						/* 20% */
    0.15,						/* 10% */
    0.15,						/* 1% */
    0.15
#elif  mA2800
    0.10,						/* 90% */
    0.10,						/* 80% */
    0.10,						/* 70% */
    0.15,						/* 60% */
    0.15,						/* 50% */
    0.15,						/* 40% */
    0.15,						/* 30% */
    0.15,						/* 20% */
    0.15,						/* 10% */
    0.15,						/* 1% */
    0.15
#elif  mA3000
    0.10,						/* 90% */
    0.10,						/* 80% */
    0.10,						/* 70% */
    0.10,						/* 60% */
    0.10,						/* 50% */
    0.10,						/* 40% */
    0.10,						/* 30% */
    0.10,						/* 20% */
    0.10,						/* 10% */
    0.10,						/* 1% */
    0.10
#endif
};


static float bat_charge_delt_vol_1 [BAT_PWR_PER_NUM + 1] = {
#if  mA5000
    0.15,						/* 90% */
    0.15,						/* 80% */
    0.18,						/* 70% */
    0.18,						/* 60% */
    0.18,						/* 50% */
    0.18,						/* 40% */
    0.18,						/* 30% */
    0.18,						/* 20% */
    0.18,						/* 10% */
    0.20,						/* 1% */
    0.30
#elif mA4000
    0.15,						/* 90% */
    0.15,						/* 80% */
    0.20,						/* 70% */
    0.25,						/* 60% */
    0.35,						/* 50% */
    0.35,						/* 40% */
    0.35,						/* 30% */
    0.35,						/* 20% */
    0.42,						/* 10% */
    0.42,						/* 1% */
    0.42
#elif  mA2800
    0.20,						/* 90% */
    0.20,						/* 80% */
    0.20,						/* 70% */
    0.25,						/* 60% */
    0.45,						/* 50% */
    0.45,						/* 40% */
    0.45,						/* 30% */
    0.55,						/* 20% */
    0.55,						/* 10% */
    0.45,						/* 1% */
    0.45
#elif  mA3000
    0.20,						/* 90% */
    0.20,						/* 80% */
    0.20,						/* 70% */
    0.20,						/* 60% */
    0.20,						/* 50% */
    0.20,						/* 40% */
    0.20,						/* 30% */
    0.20,						/* 20% */
    0.20,						/* 10% */
    0.20,						/* 1% */
    0.20
#endif
};

Battery::Battery(QObject *parent) : QObject(parent)
{
    bat_tim_init = 0;
    init_battery_power (&battery_power);

    check_battery_power (&battery_power);
    _powerPercent = battery_power.percent_power;
    startTimer(1000);           //内部计时器，用于监控电量趋势，以获得充放电、稳定的百分比电量等高阶数据
}

//电量百分比
//引入平均电量+磁滞回线机制，保证稳定
/*******************************
 *  100%:   8.00 ±0.1 v
 *  90% :   7.88 ±0.1 v
 *  70% :   7.76 ±0.1 v
 *  60% :   7.64 ±0.1 v
 *  50% :   7.52 ±0.1 v
 *  40% :   7.40 ±0.1 v
 *  30% :   7.28 ±0.1 v
 *  20% :   7.16 ±0.1 v
 *  10% :   7.04 ±0.1 v
 *  1%  :   6.92 ±0.1 v
 *  alarm:  6.6v
 * ********************************/
int Battery::battPercentValue()
{
    int k;

    if(battery_power.percent_power < 0){
        return -1;
    }

    battery_power.percent.append(battery_power.percent_power);
    if(battery_power.percent.count() >= 20){
        battery_power.percent.removeFirst();
    }
    _powerPercent = Compute::avrage(battery_power.percent);

    k = _powerPercent/10;
    if(_powerPercent%10 > 7){
        k = k+1;
    }else if(_powerPercent%10 < 3){
        k = k;
    }else{
        k = battery_power.last_percent;
    }
    battery_power.last_percent = k;

    return k*10;
}

bool Battery::is_low_power()
{
    return battery_power.pwr_loss_alarm;
}

bool Battery::is_charging()
{
    return battery_power._isCharging;
}

float Battery::battVcc()
{
    return battery_power.vol;
}

float Battery::battCur()
{
    return battery_power.cur;
}

void Battery::timerEvent(QTimerEvent *)
{
    check_battery_power (&battery_power);
}

void Battery::get_screen_state(bool sta)
{
    battery_power.screen_light = sta;
//    qDebug()<<"screen : " << sta;
}

int Battery::adm1191_conv_init()
{
    return gpio_open (ADM1191_CONV_PIN, (char*)"out");
}

int Battery::adm1191_conv_set(int value)
{
    return gpio_set (ADM1191_CONV_PIN, value);
}

int Battery::i2c_adm1191_init(unsigned char addr)
{
    int fd, ret;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg;
    unsigned char msg_buf;

    fd = open (I2C_DEV_0, O_RDWR);
    if (fd < 0) {
        //        printf ("open i2c dev error\n");
        return -1;
    }
    ioctl (fd, I2C_TIMEOUT, 100);
    ioctl (fd, I2C_M_TEN, 0);
    ioctl (fd, I2C_RETRIES, 2);

    msg_buf = 0x01 | 0x04;		/* V_CONT | I_CONT | VRANGE = 26.52V */

    i2c_data.nmsgs = 1;
    i2c_data.msgs = &msg;

    i2c_data.msgs [0].addr = addr;
    i2c_data.msgs [0].flags = 0;			/* write */
    i2c_data.msgs [0].len = 1;
    i2c_data.msgs [0].buf = &msg_buf;

    ret = ioctl (fd, I2C_RDWR, (unsigned long) &i2c_data);

    close (fd);

    return ret;
}

int Battery::i2c_adm1191_read(unsigned char addr, unsigned short *voltage, unsigned short *current)
{
    int fd, ret;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg;
    unsigned char msg_buf [3];

    fd = open (I2C_DEV_0, O_RDWR);
    if (fd < 0) {
        //        printf ("open i2c dev error\n");
        return -1;
    }
    ioctl (fd, I2C_TIMEOUT, 100);
    ioctl (fd, I2C_M_TEN, 0);
    ioctl (fd, I2C_RETRIES, 2);

    i2c_data.nmsgs = 1;
    i2c_data.msgs = &msg;

    i2c_data.msgs [0].addr = addr;
    i2c_data.msgs [0].flags = 1;			/* read */
    i2c_data.msgs [0].len = 3;
    i2c_data.msgs [0].buf = msg_buf;

    ret = ioctl (fd, I2C_RDWR, (unsigned long) &i2c_data);

    close (fd);

    if (ret >= 0) {
        * voltage = (msg_buf [0] << 4) | (msg_buf [2] >> 4);
        * current = (msg_buf [1] << 4) | (msg_buf [2] & 0xf);
    }

    return ret;
}

int Battery::i2c_adm1191_probe(Battery::battery_power_t *bp)
{
    unsigned char addr;
    int ret;

    if (adm1191_conv_init () != 0) {
        //        printf ("adm1191_conv_init failled\n");
    }

    if (adm1191_conv_set (1) != 0) {
        //        printf ("adm1191_conv_set failed\n");
    }

    for (addr = ADM1191_START_ADDR; addr <= ADM1191_END_ADDR; addr++) {
        //printf ("probe adm 0x%x...\n", addr);
        if ((ret = i2c_adm1191_init (addr)) >= 0) {
            //printf ("adm1191 probe ok at 0x%x\n", addr);
            break;
        }
        usleep (100000);
    }

    bp->adm1191_addr = addr;

    if (addr > ADM1191_END_ADDR) {
        return -1;
    }
    else {
        return 0;
    }
}

int Battery::conv_vol(unsigned short vol, float *vol_f, int hi)
{
    if (hi) {
        * vol_f = ADM_VOL_FULLSCALE_HI * vol / 4096.0f;
    }
    else {
        * vol_f = ADM_VOL_FULLSCALE_LO * vol / 4096.0f;
    }

    return 0;
}

int Battery::conv_cur(unsigned short cur, float *cur_f)
{
    * cur_f = (ADM_CUR_FULLSCALE * cur) / (4096.0f * ADM_SENSE_RES);

    return 0;
}

int Battery::init_battery_power(Battery::battery_power_t *bp)
{
    bp->cur = 400.0f;			/* 400ma */
    bp->vol = 8.2f;				/* 8.2v */
    bp->percent_power = 100;			/* 100% */
	bp->_isCharging = false;
    bp->lo_pwr_alarm = 0;		/* 低电压告警 */
    bp->pwr_loss_alarm = 0;
    bp->force_pwr_off = 0;

	bp->bat_value_list_init = false;

    if (i2c_adm1191_probe (bp) < 0) {
        bp->adm1191_addr = ADM1191_ADDR;
        //        printf ("failed to probe adm\n");

        return -1;
    }
    else {
        //        printf ("adm1191 is probed at 0x%x\n", bp->adm1191_addr);

        return 0;
    }
}

int Battery::check_battery_power(Battery::battery_power_t *bp)
{
    unsigned short vol, cur;
	int i;
	float old_val, new_val, del_val;

    if (i2c_adm1191_read (bp->adm1191_addr, &vol, &cur) >= 0) {
        conv_vol (vol, &bp->vol, 1);
        conv_cur (cur, &bp->cur);

		/* 初始化list */
		if (!bp->bat_value_list_init) {


            bat_tim_init++;
            if(bat_tim_init > 1){                           //前5次数据采样不正确
                for (i = 0; i < VCC_LIST_NUM; i++) {
                    bp->vcc_list.append(bp->vol);
                }
                for (i = 0; i < VCC_DELTA_LIST_NUM; i++) {
                    bp->vcc_delta.append(0);
                }

                new_val = Compute::avrage(bp->vcc_list);
//                qDebug()<<"new_val  "<< new_val;

                bp->bat_value_list_init = true;
            }else{
                bp->percent_power = -1;
                return 0;
            }
		}

		//记录10s的vcc值，用以判断充放电
		bp->vcc_list.append(bp->vol);
    	if (bp->vcc_list.count() > VCC_LIST_NUM){
        	bp->vcc_list.removeFirst();
    	}

        old_val = Compute::avrage(bp->vcc_list.mid(0,VCC_LIST_OLD_VALUE_NUM) );
        new_val = Compute::avrage(bp->vcc_list.mid(VCC_LIST_OLD_VALUE_NUM,VCC_LIST_NEW_VALUE_NUM) );
        del_val = new_val - old_val;
		bp->vcc_delta.append(del_val);
       	if(bp->vcc_delta.count() > VCC_DELTA_LIST_NUM){
           	bp->vcc_delta.removeFirst();
       	}

//        qDebug()<< "backlight" << sqlcfg->get_para()->backlight;


	    for (i = 0; i < BAT_PWR_PER_NUM; i++) {
			if (bp->_isCharging) {       
                del_val = bat_charge_delt_vol_1 [i];
			}
			else {
				del_val = 0.0f;
            }


            if(battery_power.screen_light == false){
                del_val = del_val + sqlcfg->get_para()->backlight * i * 0.0010;
            }
//            qDebug()<<"del_val"<<del_val;

	        if (new_val > (bat_pwr_percent [i] + del_val)) {
	            break;
	        }
	    }
//        qDebug()<<"_________________";
//        qDebug()<<"i"<<i;
//        qDebug()<<"new_val"<<new_val;
		bp->percent_power = 100 - i * 10;
//        qDebug()<<"percent_power"<<bp->percent_power;

        del_val = Compute::avrage(bp->vcc_delta);



//        qDebug()<<"_isCharging"<<bp->_isCharging;

        if(del_val > bat_charge_delt_vol [i]){       //认定充电
            bp->_isCharging = true;
        }
        else if(del_val < (-bat_charge_delt_vol [i])){
            bp->_isCharging = false;
        }

	    if (bp->percent_power <= BAT_PWR_ALARM_PER) {
	        bp->lo_pwr_alarm = 1;
	    }
	    else {
	        bp->lo_pwr_alarm = 0;
	    }

	    if (bp->percent_power <= BAT_PWR_LOSS_PER) {
	        bp->pwr_loss_alarm = 1;
	    }
	    else {
	        bp->pwr_loss_alarm = 0;
	    }

	    if (new_val < SHUTDOWN_VOL) {
	        bp->force_pwr_off = 1;
			qDebug()<<"is batt!!!! batt = "<< battery_power.percent_power;
            system ("reboot");
	    }
	    else {
	        bp->force_pwr_off = 0;
	    }

		//电压电流传入RDB
        Common::rdb_set_yc_value(Battery_vcc_yc, /*bp->vol*/new_val );
        Common::rdb_set_yc_value(Battery_cur_yc, bp->cur );
	
//        printf ("vol %1.2f; cur %3.2f; pwr %d; lo pwr %d; loss pwr %d\n",
//                bp->vol, bp->cur, bp->power, bp->lo_pwr_alarm, bp->pwr_loss_alarm);
    }

    return 0;
}

int Battery::vcc_to_percent(float v)
{
    return 100*v - 700;
}
