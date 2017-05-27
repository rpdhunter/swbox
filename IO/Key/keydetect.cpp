#include "keydetect.h"


//线程的初始化工作，建立设备连接
KeyDetect::KeyDetect(QObject *parent) : QThread(parent)
{
    int i;
    int ret;
    char *str = NULL;
    const char *path = "/sys/class/gpio/gpio";
    const char *pcmd = "/sys/class/gpio/export";

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("[ERROR] Open %s failed.\n", "/dev/mem");
    }

    /* get virt_addr from phy_addr */
    base_addr = (char *)mmap(NULL, PS_GPIO_SIZE, (PROT_READ | PROT_WRITE), \
        MAP_SHARED, fd, PS_GPIO_BASE);
    if (base_addr == MAP_FAILED) {
        printf("[ERROR] Mmap virt_addr for phy_addr of 0x%08x faild!", \
            PS_GPIO_BASE);
        return;
    }

    str = (char *)malloc(0x100);
    if (str == NULL) {
        printf("[ERROR] malloc failed!");
        munmap(base_addr, PS_GPIO_SIZE);
        return;
    }

    for (i = 0; i < KEY_NUM; i++) {
        sprintf(str, "%s%d", path, PIN_START + PIN_BASE + i);

        /* Check gpio exist or no */
        ret = access(str, F_OK);
        if (ret) {
            sprintf(str, "%s %d %s %s", "echo", PIN_START + PIN_BASE + i, ">", pcmd);
            if (system(str)){
                printf("run cmd[str] failed.");
            }
        }

        /* set pin output */
        set_dir_pin(PIN_START + i, PIN_DIR_IN);

        /* disable pin output */
        set_output_enable(PIN_START + i, PIN_OUT_DISABLE);
    }

    /* start pthread */
    this->start();

}

//监测键盘事件
//一旦按下某键，便发送对应信号
void KeyDetect::run(void)
{
//    quint64 i=0;
//    while(1){
//        gpio_read_pin(PIN_OK);
//        i++;
//        if(i%1000000==0){
//            qDebug()<<i/1000000;
//        }
//    }
    while (true) {
        if (!gpio_read_pin(PIN_POWER)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_POWER)) {
                emit sendkey(KEY_POWER);
                while (!gpio_read_pin(PIN_POWER)){
                    usleep(KEY_MS_DLY);
                }
            }
        }
        if (!gpio_read_pin(PIN_OK)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_OK)) {
                emit sendkey(KEY_OK);
                while (!gpio_read_pin(PIN_OK)){
                    usleep(KEY_MS_DLY);
                }
            }
        }
        if (!gpio_read_pin(PIN_CANCEL)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_CANCEL)) {
                emit sendkey(KEY_CANCEL);
                while (!gpio_read_pin(PIN_CANCEL)){
                    usleep(KEY_MS_DLY);
                }
            }
        }
        if (!gpio_read_pin(PIN_UP)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_UP)) {
                emit sendkey(KEY_UP);
                cnt = 0;
                while (!gpio_read_pin(PIN_UP)) {
                    usleep(100000);
                    cnt++;
                    if (cnt > 5) {
                        emit sendkey(KEY_UP);
                        cnt = 5;
                    }
                }
            }
        }
        if (!gpio_read_pin(PIN_DOWN)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_DOWN)) {
                emit sendkey(KEY_DOWN);
                cnt = 0;
                while (!gpio_read_pin(PIN_DOWN)) {
                    usleep(100000);
                    cnt++;
                    if (cnt > 5) {
                        emit sendkey(KEY_DOWN);
                        cnt = 5;
                    }
                }
            }
        }
        if (!gpio_read_pin(PIN_LEFT)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_LEFT)) {
                emit sendkey(KEY_LEFT);
                cnt = 0;
                while (!gpio_read_pin(PIN_LEFT)) {
                    usleep(100000);
                    cnt++;
                    if (cnt > 5) {
                        emit sendkey(KEY_LEFT);
                        cnt = 5;
                    }
                }
            }
        }
        if (!gpio_read_pin(PIN_RIGHT)) {
            usleep(KEY_MS_DLY);
            if (!gpio_read_pin(PIN_RIGHT)) {
                emit sendkey(KEY_RIGHT);
                cnt = 0;
                while (!gpio_read_pin(PIN_RIGHT)) {
                    usleep(100000);
                    cnt++;
                    if (cnt > 5) {
                        emit sendkey(KEY_RIGHT);
                        cnt = 5;
                    }
                }
            }
        }
        usleep(KEY_MS_DLY);
    }
    exit(0);
}

/*
    func_name  : set_output_enable
    input para : set pin, set enable value
    out para   : none
    func_dscr  : set pin output enable
*/
void KeyDetect::set_output_enable(int pin, int enable)
{
    char *reg;
    int bank;
    int pin_num;
    unsigned int reg_val;

    /* Get bank from pin */
    get_bank(pin, &bank);
    if (bank < 0) {
        printf("[ERROR] Get bank from pin failed!");
        return;
    }
    else if (bank == 0) {       //这样会造成数组越界
        printf("[ERROR] bank == 0, Get bank from pin failed!");
        return;
    }

    /* Real pin number */
    pin_num = pin % (gpio_pin_table[bank - 1] + 1);

    /* Output enable reg addr */
    reg = base_addr + (XGPIOPS_REG_MASK_OFFSET * (bank)) + XGPIOPS_OUTEN_OFFSET;

    /* Read value from output enable reg */
    reg_val = *(volatile unsigned int *)reg;

    /* Fill enable value */
    if (enable) {
        /* Enable output */
        reg_val |= (1 << pin_num);
    } else {
        /* Disable output */
        reg_val &= ~(1 << pin_num);
    }

    /* Write value to output enable reg */
    *(volatile unsigned int *)reg = reg_val;
}

/*
    func_name  : set_dir_pin
    input para : set pin, set pin dir value
    out para   : none
    func_dscr  : set pin dir
*/
void KeyDetect::set_dir_pin(int pin, int dir)
{
    char *reg;
    int bank;
    int pin_num;
    unsigned int reg_val;

    /* Get bank from pin */
    get_bank(pin, &bank);
    if (bank < 0) {
        printf("[ERROR] Get bank from pin failed!");
        return;
    }
    else if (bank == 0) {       //这样会造成数组越界
        printf("[ERROR] bank == 0, Get bank from pin failed!");
        return;
    }

    /* Real pin number */
    pin_num = pin % (gpio_pin_table[bank - 1] + 1);

    /* Direct reg addr */
    reg = base_addr + (XGPIOPS_REG_MASK_OFFSET * (bank)) + XGPIOPS_DIRM_OFFSET;

    /* Read value from direct reg */
    reg_val = *(volatile unsigned int *)reg;

    /* Fill dir value */
    if (dir) {
        /* Output */
        reg_val |= (1 << pin_num);
    } else {
        /* Input */
        reg_val &= ~(1 << pin_num);
    }

    /* Write value to direct reg */
    *(volatile unsigned int *)reg = reg_val;
}

/*
    func_name  : gpio_read_pin
    input para : read pin
    out para   : key_val
    func_dscr  : read pin status
*/
bool KeyDetect::gpio_read_pin(int pin)
{
    int bank;
    int pin_num;
    volatile unsigned int *reg;

    /* Get bank from pin */
    get_bank(pin, &bank);

    if (bank < 0) {
        printf("[ERROR] Get bank from pin failed!");
//        return;
    }
    else if (bank == 0) {       //这样会造成数组越界
        printf("[ERROR] bank == 0, Get bank from pin failed!");
//        return;
    }

    /* Real pin number */
    pin_num = pin % (gpio_pin_table[bank - 1] + 1);

    /* reg */
    reg = (volatile unsigned int *)(base_addr + (XGPIOPS_DATA_BANK_OFFSET * (bank) + XGPIOPS_DATA_OFFSET));
    return (*reg >> pin_num) & 1;
}

/*
    func_name  : get_bank
    input para : pin, bank pointer
    out para   : none
    func_dscr  : set pin dir
*/
void KeyDetect::get_bank(int pin, int *bank)
{
    /* Get bank from pin */
    if (pin <= gpio_pin_table[0]) {
        *bank = 0;
    } else if (pin <= gpio_pin_table[1]) {
        *bank = 1;
    } else if(pin <= gpio_pin_table[2]) {
        *bank = 2;
    } else if(pin <= gpio_pin_table[3]) {
        *bank = 3;
    } else {
        *bank = -1;
        printf("[ERROR] Pin number %d invalid!\n", pin);
    }
}
