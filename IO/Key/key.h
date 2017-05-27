#ifndef KEY_H
#define KEY_H

#define PS_GPIO_BASE        0xe000a000
#define PS_GPIO_SIZE        0x00001000

#define KEY_NUM             7
#define KEY_MS_DLY          10000

#define PIN_BASE            906
#define PIN_START           54

#define PIN_POWER           54

/*
 * HD_V1 : hardware made in 2016
 * HD_V2 : hardware made in March, 2017
 */
//#define HD_V1
#ifdef HD_V1
#define PIN_DOWN            55
#define PIN_RIGHT           56
#define PIN_OK              57
#define PIN_CANCEL          58
#define PIN_UP              59
#define PIN_LEFT            60
#else
#define PIN_CANCEL          55
#define PIN_UP              56
#define PIN_RIGHT           57
#define PIN_OK              58
#define PIN_LEFT            59
#define PIN_DOWN            60
#endif

/*
 * This structure defines the mapping of the pin numbers to the banks when
 * the driver APIs are used for working on the individual pins.
 */
const int gpio_pin_table[] = {
    31, /* 0 - 31, Bank 0 */
    53, /* 32 - 53, Bank 1 */
    85, /* 54 - 85, Bank 2 */
    117 /* 86 - 117 Bank 3 */
};

/** @name Register offsets for each Bank.
 *  @{
 */
#define XGPIOPS_DATA_MASK_OFFSET 0x8  /* Data/Mask Registers offset */
#define XGPIOPS_DATA_BANK_OFFSET 0x4  /* Data Registers offset */
#define XGPIOPS_REG_MASK_OFFSET 0x40  /* Registers offset */
/* @} */

/** @name Register offsets for the GPIO. Each register is 32 bits.
 *  @{
 */
#define XGPIOPS_DATA_LSW_OFFSET 0x000  /* Mask and Data Register LSW, WO */
#define XGPIOPS_DATA_MSW_OFFSET 0x004  /* Mask and Data Register MSW, WO */
#define XGPIOPS_DATA_OFFSET	    0x040  /* Data Register, RW */
#define XGPIOPS_DATA_RO_OFFSET	0x060  /* Data Register - Input, RO */
#define XGPIOPS_DIRM_OFFSET     0x204  /* Direction Mode Register, RW */
#define XGPIOPS_OUTEN_OFFSET	0x208  /* Output Enable Register, RW */
#define XGPIOPS_INTMASK_OFFSET  0x20C  /* Interrupt Mask Register, RO */
#define XGPIOPS_INTEN_OFFSET	0x210  /* Interrupt Enable Register, WO */
#define XGPIOPS_INTDIS_OFFSET   0x214  /* Interrupt Disable Register, WO*/
#define XGPIOPS_INTSTS_OFFSET   0x218  /* Interrupt Status Register, RO */
#define XGPIOPS_INTTYPE_OFFSET  0x21C  /* Interrupt Type Register, RW */
#define XGPIOPS_INTPOL_OFFSET	0x220  /* Interrupt Polarity Register, RW */
#define XGPIOPS_INTANY_OFFSET	0x224  /* Interrupt On Any Register, RW */
/* @} */

#define PIN_DIR_OUT     1
#define PIN_DIR_IN      0
#define PIN_OUT_ENABLE  1
#define PIN_OUT_DISABLE 0
#define PIN_HIGH        1
#define PIN_LOW         0

/* key value define by self */
enum KEY_VALUE {
    KEY_NONE    = 0x00,
    KEY_POWER   = 0x10,
    KEY_CANCEL  = 0x20,
    KEY_OK      = 0x30,
    KEY_UP      = 0xa0,
    KEY_DOWN    = 0xa1,
    KEY_LEFT    = 0xb0,
    KEY_RIGHT   = 0xb1,
};

union CURRENT_KEY_VALUE {
    unsigned long long val;
    struct KEY {
        unsigned char val7;
        unsigned char val6;
        unsigned char val5;
        unsigned char val4;
        unsigned char val3;
        unsigned char val2;
        unsigned char val1;
        unsigned char val0;
    } grade;
};

/* grade menu range */
#define GRADE0_MENU_MIN_NUM         0
#define GRADE0_MENU_MAX_NUM         6
#endif // KEY_H
