/*---------------------------------------------------------------------------*/
/* PCI-RDK version 2 registers  */

/* Main Control Registers */

#define RDK2_IRQENB						0x00
#define RDK2_IRQSTAT						0x04
#define		PB7				23
#define		PB6				22
#define		PB5				21
#define		PB4				20
#define		PB3				19
#define		PB2				18
#define		PB1				17
#define		PB0				16
#define		GP3				23
#define		GP2				23
#define		GP1				23
#define		GP0				23
#define         DMA_RETRY_ABORT                 6
#define         DMA_PAUSE_DONE                  5
#define         DMA_ABORT_DONE                  4
#define         DMA_OUT_FIFO_TRANSFER_DONE      3
#define         DMA_LOCAL_DONE                  2
#define         DMA_PCI_DONE                    1
#define         NET2272_PCI_IRQ                 0

#define RDK2_LOCCTLRDK  					0x08
#define		CHIP_RESET			3
#define		SPLIT_DMA			2
#define		MULTIPLEX_MODE			1
#define		BUS_WIDTH			0

#define RDK2_GPIOCTL					0x10
#define		GP3_OUT_ENABLE					7
#define		GP2_OUT_ENABLE					6
#define		GP1_OUT_ENABLE					5
#define		GP0_OUT_ENABLE					4
#define		GP3_DATA					3
#define		GP2_DATA					2
#define		GP1_DATA					1
#define		GP0_DATA					0

#define RDK2_LEDSW						0x14
#define		LED3				27
#define		LED2				26
#define		LED1				25
#define		LED0				24
#define         PBUTTON                         16
#define         DIPSW                           0

#define RDK2_DIAG						0x18
#define		RDK2_FAST_TIMES				2
#define		FORCE_PCI_SERR				1
#define		FORCE_PCI_INT				0
#define RDK2_FPGAREV						0x1C

// Dma Control registers 
#define RDK2_DMACTL						0x80
#define		ADDR_HOLD				24
#define		RETRY_COUNT				16      // 23:16
#define		FIFO_THRESHOLD				11      // 15:11
#define		MEM_WRITE_INVALIDATE			10
#define		READ_MULTIPLE				9
#define		READ_LINE				8
#define		RDK2_DMA_MODE				6       // 7:6
#define		CONTROL_DACK				5
#define		EOT_ENABLE				4
#define		EOT_POLARITY				3
#define		DACK_POLARITY				2
#define		DREQ_POLARITY				1
#define		DMA_ENABLE				0

#define RDK2_DMASTAT						0x84
#define		GATHER_COUNT				12      // 14:12
#define		FIFO_COUNT				6      // 11:6
#define		FIFO_FLUSH				5
#define		FIFO_TRANSFER				4
#define		PAUSE_DONE			        3
#define		ABORT_DONE				2
#define		DMA_ABORT				1
#define		DMA_START				0

#define RDK2_DMAPCICOUNT					0x88
#define		DMA_DIRECTION				31
#define		DMA_PCI_BYTE_COUNT		        0       // 0:23

#define RDK2_DMALOCCOUNT					0x8C    // 0:23 dma local byte count

#define RDK2_DMAADDR						0x90    // 2:31 PCI bus starting address
