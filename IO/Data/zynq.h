#ifndef ZYNQ_H
#define ZYNQ_H

//基地址
#define AXI_STREAM_BASE0		0x43c00000  //基本读（读）
//#define AXI_STREAM_BASE1		0x43c10000  //基本写（写）
#define AXI_STREAM_BASE1		0x83c00000  //1013xwt

#define AXI_STREAM_BASE2		0x43c20000  //超声1（写）
#define AXI_STREAM_BASE3		0x43c30000  //超声2（写）
//#define AXI_STREAM_BASE4		0x83c00000  //预留（写）
#define AXI_STREAM_BASE4		0x43c10000  //1013xwt

#define AXI_STREAM_BASE6		0x43c40000  //HFCT1（读）
#define AXI_STREAM_BASE5		0x43c50000  //HFCT2（读）
#define AXI_STREAM_BASE7		0x43c60000  //PRPD1（读）
#define AXI_STREAM_BASE8		0x43c80000  //PRPD2（读）
#define AXI_STREAM_BASE9		0x43c70000  //录波（读）

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

#define WORKING_MODE            0x0008          //工作模式，对应表格如下
/*
0：物理1——TEV1，物理2——TEV2
1：物理1——TEV1，物理2——HFCT1
2：物理1——TEV1，物理2——HFCT2
3：物理1——TEV2，物理2——TEV1
4：物理1——TEV2，物理2——HFCT1
5：物理1——TEV2，物理2——HFCT2
6：物理1——HFCT1，物理2——TEV1
7：物理1——HFCT1，物理2——TEV2
8：物理1——HFCT1，物理2——HFCT2
9：物理1——HFCT2，物理2——TEV1
10：物理1——HFCT2，物理2——TEV2
11：物理1——HFCT2，物理2——HFCT1
*/
#define FILTER_MODE             0x0009          //滤波模式设定，0无滤波，1为180K高通，2为500K高通，目前仅HFCT模式有效


//通道参数
#define TEV1_ZERO				0x0010          //TEV参考零点(通道1)（0x0000-0xffff）
#define TEV1_THRESHOLD			0x0011          //TEV脉冲阈值(通道1)（0x0000-0xffff）
#define TEV2_ZERO				0x0012          //TEV参考零点(通道2)（0x0000-0xffff）
#define TEV2_THRESHOLD			0x0013          //TEV脉冲阈值(通道2)（0x0000-0xffff）
#define HFCT1_ZERO				0x0014          //HFCT参考零点(通道1)（0x0000-0xffff）
#define HFCT1_THRESHOLD			0x0015          //HFCT脉冲阈值(通道1)（0x0000-0xffff）
#define HFCT2_ZERO				0x0016          //HFCT参考零点(通道2)（0x0000-0xffff）
#define HFCT2_THRESHOLD			0x0017          //HFCT脉冲阈值(通道2)（0x0000-0xffff）
#define AA_VOL					0x0018          //超声音量（0-15变化，7为原始音量）
#define AA_RECORD_PLAY			0x0019          //播放声音标志(之后发送声音数据至FPGA)（bool）


//录波
#define REC_START_TEV1			0x0020          //TEV1录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_TEV2          0x0021          //TEV2录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_HFCT1         0x0022          //HFCT1录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_HFCT2         0x0023          //HFCT2录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_AA1           0x0024          //AA1录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_AA2           0x0025          //AA2录波控制信号（0为常态，1为录波开始，2为上传开始）

//组号，录波时用于数据组标志，范围(0-15)+通道编号(0x100-TEV1,0x200-TEV2,0x400-HFCT1,0x800-HFCT2,0x1000-AA1,0x2000-AA2)
#define GROUP_NUM				0x002a

#define AUTO_REC			0x002b          //自动录波标志位（0为关闭所有自动录波，1为1通道自动，2为2通道自动，3为双通道自动，12为1通道触发同步，20为2通道触发同步，28为双通道触发同步）

//要通道数据信号
#define READ_FPGA_NOMAL         0x0030          //普通
#define READ_FPGA_REC           0x0031          //录波
#define READ_FPGA_PRPD1         0x0032          //PRPD1模式
#define READ_FPGA_PRPD2         0x0033          //PRPD2模式
#define READ_FPGA_HFCT1         0x0034          //HFCT1模式
#define READ_FPGA_HFCT2         0x0035          //HFCT2模式

#define FPGA_SLEEP              0x0036          //FPGAsleep

enum send_params {
    //常规功能设置
    sp_ram_reset = 0,
    sp_freq_reg,
    sp_backlight_reg,
    sp_keyboard_backlight,
    sp_sleeping,
    sp_working_mode,
    sp_filter_mode,

    //通道参数
    sp_tev1_zero,
    sp_tev1_threshold,
    sp_tev2_zero,
    sp_tev2_threshold,
    sp_hfct1_zero,
    sp_hfct1_threshold,
    sp_hfct2_zero,
    sp_hfct2_threshold,
    sp_aa_vol,
    sp_aa_record_play,
    //录波
    sp_rec_start_tev1,
    sp_rec_start_tev2,
    sp_rec_start_hfct1,
    sp_rec_start_hfct2,
    sp_rec_start_aa1,
    sp_rec_start_aa2,
    sp_group_num,
    sp_auto_rec,
    //要通道数据信号
    sp_read_fpga_normal,
    sp_read_fpga_rec,
    sp_read_fpga_prpd1,
    sp_read_fpga_prpd2,
    sp_read_fpga_hfct1,
    sp_read_fpga_hfct2,

    sp_fpga_sleep,  //xwt

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
