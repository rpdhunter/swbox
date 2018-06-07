#ifndef ZYNQ_H
#define ZYNQ_H

//基地址
#define AXI_STREAM_BASE0		0x43c10000  //基本读（读）x
#define AXI_STREAM_BASE1		0x83c00000  //基本写（写）x

#define AXI_STREAM_BASE2		0x43c00000  //超声1（写）x
#define AXI_STREAM_BASE3		0x43c20000  //超声2（读）
#define AXI_STREAM_BASE4		0x43c30000  //同步(写) x

#define AXI_STREAM_BASE5		0x43c40000  //HFCT1（读）x
#define AXI_STREAM_BASE6		0x43c50000  //HFCT2（读）x
//#define AXI_STREAM_BASE7		0x43d60000  //PRPD1（读）xX
//#define AXI_STREAM_BASE8		0x43c80000  //PRPD2（读）x
#define AXI_STREAM_BASE9		0x43c60000  //录波（读）xX
#define AXI_STREAM_BASE10		0x83c10000  //ae1（读）X
#define AXI_STREAM_BASE11		0x83c20000  //ae2（读）X


#define AXI_STREAM_SIZE			0x1000
#define ISR						0x00
#define IER						0x01
#define TDFR					0x02
#define TDFV					0x03
#define TDFD					0x04
#define TLR						0x05
#define RDFR					0x06
#define RDFO					0x07
#define RDFD					0x08
#define RLR						0x09

//常规功能设置
#define RAM_RESET				0x0001          //初始化时使用（bool）
#define FREQ_REG				0x0002          //频率（目前只有50,60两个值，以后可能要扩展成浮点数频率）
#define BACKLIGHT_REG			0x0003          //背光（0-7变化，7为最亮，8为全黑）
#define KEYBOARD_BACKLIGHT      0x0004          //0熄灭键盘灯，1打开键盘灯
#define SLEEPING                0x0005          //FPGA休眠(0为休眠,1为唤醒)
#define BUZZER                  0x0006          //蜂鸣器(0不响,1为响)
#define BUZZER_FREQ             0x0007          //蜂鸣器频率,目标频率f, 则写入值为100 000 000/f/2/16 (f范围100-20000)
#define REBOOT                  0x0008          //关机指令(1为关机)
#define FILTER_MODE             0x0009          //滤波模式设定，高八位控制H2,低八位控制H1,0无滤波，2为1.8M高通，1为500K高通
#define L1_CHANNEL_MODE         0x000a          //低频通道模式，0为录波声音使用原始模式，1为录波声音使用包络线模式
#define L2_CHANNEL_MODE         0x000b          //低频通道模式，0为录波声音使用原始模式，1为录波声音使用包络线模式

//通道参数
#define H1_ZERO                 0x0010          //通道1参考零点（0x0000-0xffff）
#define H1_THRESHOLD			0x0011          //通道1脉冲阈值（0x0000-0xffff）
#define H2_ZERO                 0x0012          //通道2参考零点（0x0000-0xffff）
#define H2_THRESHOLD			0x0013          //通道2脉冲阈值（0x0000-0xffff）
#define VOL_L1					0x0018          //超声音量通道1（0-15变化，7为原始音量）
#define AA_RECORD_PLAY			0x0019          //播放声音标志(之后发送声音数据至FPGA)（0直播L1,1重播,2直播L2）
#define VOL_L2					0x001a          //超声音量通道2（0-15变化，7为原始音量）
#define KALMAN_L1               0x001b          //卡尔曼滤波器的开关(低频1)，0为关，1为开
#define KALMAN_L2               0x001c          //卡尔曼滤波器的开关(低频2)，0为关，1为开

//录波
#define REC_ON                  0x001f          //录波开关(0为关闭录波功能,1为开启录波功能,常态关闭)
#define REC_START_H1			0x0020          //TEV1录波控制信号（0为常态，1为录波开始，2为上传开始, 初始化为3）
#define REC_START_H2            0x0021          //TEV2录波控制信号（0为常态，1为录波开始，2为上传开始, 初始化为3）
#define REC_START_L1            0x0024          //AA1录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_L2            0x0025          //AA2录波控制信号（0为常态，1为录波开始，2为上传开始）


//组号，录波时用于数据组标志，范围(0-15)+通道编号(0x100-TEV1,0x200-TEV2,0x400-HFCT1,0x800-HFCT2,0x1000-AA1,0x2000-AA2)
#define GROUP_NUM				0x002a
#define AUTO_REC                0x002b          //自动录波标志位（0为关闭所有自动录波，1为1通道自动，2为2通道自动，3为双通道自动，12为1通道触发同步，20为2通道触发同步，28为双通道触发同步）

//要通道数据信号
#define READ_FPGA_NOMAL         0x0030          //普通
#define READ_FPGA_REC           0x0031          //录波
#define READ_FPGA_HFCT1         0x0034          //HFCT1模式
#define READ_FPGA_HFCT2         0x0035          //HFCT2模式
#define READ_FPGA_AE1           0x0036          //AE模式中包络线数据1
#define READ_FPGA_AE2           0x0037          //AE模式中包络线数据2



enum send_params {
    //常规功能设置
    sp_ram_reset = 0,
    sp_freq_reg,
    sp_backlight_reg,
    sp_keyboard_backlight,
    sp_sleeping,
    sp_buzzer,
    sp_buzzer_freq,
    sp_reboot,
    sp_filter_mode,
    sp_l1_channnel_mode,
    sp_l2_channnel_mode,

    //通道参数
    sp_h1_zero,
    sp_h1_threshold,
    sp_h2_zero,
    sp_h2_threshold,
    sp_vol_l1,
    sp_aa_record_play,
    sp_vol_l2,
    sp_kalman_l1,
    sp_kalman_l2,

    //录波
    sp_rec_on,
    sp_rec_start_h1,
    sp_rec_start_h2,
    sp_rec_start_l1,
    sp_rec_start_l2,
    sp_group_num,
    sp_auto_rec,
    //要通道数据信号
    sp_read_fpga_normal,
    sp_read_fpga_rec,
    sp_read_fpga_hfct1,
    sp_read_fpga_hfct2,
    sp_read_fpga_ae1,
    sp_read_fpga_ae2,

    sp_num
};

/* Register masks. The following constants define bit locations of various
 * control bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/* @name Interrupt bits
 *  These bits are associated with the XLLF_IER_OFFSET and XLLF_ISR_OFFSET
 *  registers.
 * @{
 */
#define XLLF_INT_RPURE_MASK       0x80000000 /**< Receive under-read */
#define XLLF_INT_RPORE_MASK       0x40000000 /**< Receive over-read */
#define XLLF_INT_RPUE_MASK        0x20000000 /**< Receive underrun (empty) */
#define XLLF_INT_TPOE_MASK        0x10000000 /**< Transmit overrun */
#define XLLF_INT_TC_MASK          0x08000000 /**< Transmit complete */
#define XLLF_INT_RC_MASK          0x04000000 /**< Receive complete */
#define XLLF_INT_TSE_MASK         0x02000000 /**< Transmit length mismatch */
#define XLLF_INT_TRC_MASK         0x01000000 /**< Transmit reset complete */
#define XLLF_INT_RRC_MASK         0x00800000 /**< Receive reset complete */
#define XLLF_INT_TFPF_MASK        0x00400000 /**< Tx FIFO Programmable Full,
                        * AXI FIFO MM2S Only */
#define XLLF_INT_TFPE_MASK        0x00200000 /**< Tx FIFO Programmable Empty
                        * AXI FIFO MM2S Only */
#define XLLF_INT_RFPF_MASK        0x00100000 /**< Rx FIFO Programmable Full
                        * AXI FIFO MM2S Only */
#define XLLF_INT_RFPE_MASK        0x00080000 /**< Rx FIFO Programmable Empty
                        * AXI FIFO MM2S Only */
#define XLLF_INT_ALL_MASK         0xfff80000 /**< All the ints */
#define XLLF_INT_ERROR_MASK       0xf2000000 /**< Error status ints */
#define XLLF_INT_RXERROR_MASK     0xe0000000 /**< Receive Error status ints */
#define XLLF_INT_TXERROR_MASK     0x12000000 /**< Transmit Error status ints */
/*@}*/

/* @name Reset register values
 *  These bits are associated with the XLLF_TDFR_OFFSET and XLLF_RDFR_OFFSET
 *  reset registers.
 * @{
 */
#define XLLF_RDFR_RESET_MASK        0x000000a5 /**< receive reset value */
#define XLLF_TDFR_RESET_MASK        0x000000a5 /**< Transmit reset value */
#define XLLF_LLR_RESET_MASK         0x000000a5 /**< Local Link reset value */
/*@}*/

#endif // ZYNQ_H
