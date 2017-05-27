#ifndef ZYNQ_H
#define ZYNQ_H

#define AXI_STREAM_BASE0		0x43c00000
//#define AXI_STREAM_BASE1		0x83c00000
#define AXI_STREAM_BASE1		0x43c10000

#define AXI_STREAM_BASE2		0x43c20000
#define AXI_STREAM_BASE3		0x43c30000

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


/* Register masks. The following constants define bit locations of various
 * control bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/** @name Interrupt bits
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

/** @name Reset register values
 *  These bits are associated with the XLLF_TDFR_OFFSET and XLLF_RDFR_OFFSET
 *  reset registers.
 * @{
 */
#define XLLF_RDFR_RESET_MASK        0x000000a5 /**< receive reset value */
#define XLLF_TDFR_RESET_MASK        0x000000a5 /**< Transmit reset value */
#define XLLF_LLR_RESET_MASK         0x000000a5 /**< Local Link reset value */
/*@}*/

#endif // ZYNQ_H
