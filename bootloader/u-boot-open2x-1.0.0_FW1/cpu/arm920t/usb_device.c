/*
 * USB device controller driver on the S3C2410.
 *
 * (C) Copyright 2003
 * Kim Sungho, Dignsys Inc. <shkim@dignsys.com>
 *
 */

/* include file */
#include <common.h>

#ifdef CONFIG_DRIVER_USB_DEVICE

#if defined(CONFIG_S3C2410)
#include <s3c2410.h>
#endif
#if defined(CONFIG_MMSP20)
#include <mmsp20.h>
#endif

//#define DEBUG_USB
#ifdef DEBUG_USB
	#define DbgPrintf(args...) printf(args)
#else
	#define DbgPrintf(args...) { }
#endif

//#define DEBUG_USB2
#ifdef DEBUG_USB2
	#define DbgPrintf2(args...) printf(args)
#else
	#define DbgPrintf2(args...) { }
#endif

#define U32 unsigned int
#define U16 unsigned short
#define S32 int
#define S16 short int
#define U8  unsigned char
#define	S8  char

#define TRUE 	1   
#define FALSE 	0

// USB Device Options
#define BULK_PKT_SIZE	64

#if defined(CONFIG_S3C2410)
#define USB_DEVICE_BASE S3C24X0_USB_DEVICE_BASE
#endif
#if defined(CONFIG_MMSP20)
#define USB_DEVICE_BASE MMSP20_USB_DEVICE_BASE
#endif

// USB DEVICE
#ifdef __BIG_ENDIAN
<ERROR IF BIG_ENDIAN>

#else  // Little Endian

#if defined(CONFIG_S3C2410)

#define rFUNC_ADDR_REG     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x000)) //Function address
#define rPWR_REG           (*(volatile unsigned char *)(USB_DEVICE_BASE+0x004)) //Power management
#define rEP_INT_REG        (*(volatile unsigned char *)(USB_DEVICE_BASE+0x008)) //EP Interrupt pending and clear
#define rUSB_INT_REG       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x018)) //USB Interrupt pending and clear
#define rEP_INT_EN_REG     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x01c)) //Interrupt enable
#define rUSB_INT_EN_REG    (*(volatile unsigned char *)(USB_DEVICE_BASE+0x02c))
#define rFRAME_NUM1_REG    (*(volatile unsigned char *)(USB_DEVICE_BASE+0x030)) //Frame number lower byte
#define rFRAME_NUM2_REG    (*(volatile unsigned char *)(USB_DEVICE_BASE+0x034)) //Frame number higher byte
#define rINDEX_REG         (*(volatile unsigned char *)(USB_DEVICE_BASE+0x038)) //Register index
#define rMAXP_REG          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x040)) //Endpoint max packet
#define rEP0_CSR           (*(volatile unsigned char *)(USB_DEVICE_BASE+0x044)) //Endpoint 0 status
#define rIN_CSR1_REG       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x044)) //In endpoint control status
#define rIN_CSR2_REG       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x048))
#define rOUT_CSR1_REG      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x050)) //Out endpoint control status
#define rOUT_CSR2_REG      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x054))
#define rOUT_FIFO_CNT1_REG (*(volatile unsigned char *)(USB_DEVICE_BASE+0x058)) //Endpoint out write count
#define rOUT_FIFO_CNT2_REG (*(volatile unsigned char *)(USB_DEVICE_BASE+0x05c))
#define rEP0_FIFO          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x080)) //Endpoint 0 FIFO
#define rEP1_FIFO          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x084)) //Endpoint 1 FIFO
#define rEP2_FIFO          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x088)) //Endpoint 2 FIFO
#define rEP3_FIFO          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x08c)) //Endpoint 3 FIFO
#define rEP4_FIFO          (*(volatile unsigned char *)(USB_DEVICE_BASE+0x090)) //Endpoint 4 FIFO
#define rEP1_DMA_CON       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0c0)) //EP1 DMA interface control
#define rEP1_DMA_UNIT      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0c4)) //EP1 DMA Tx unit counter
#define rEP1_DMA_FIFO      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0c8)) //EP1 DMA Tx FIFO counter
#define rEP1_DMA_TTC_L     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0cc)) //EP1 DMA total Tx counter
#define rEP1_DMA_TTC_M     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0d0))
#define rEP1_DMA_TTC_H     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0d4))
#define rEP2_DMA_CON       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0d8)) //EP2 DMA interface control
#define rEP2_DMA_UNIT      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0dc)) //EP2 DMA Tx unit counter
#define rEP2_DMA_FIFO      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0e0)) //EP2 DMA Tx FIFO counter
#define rEP2_DMA_TTC_L     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0e4)) //EP2 DMA total Tx counter
#define rEP2_DMA_TTC_M     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0e8))
#define rEP2_DMA_TTC_H     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x0ec))
#define rEP3_DMA_CON       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x100)) //EP3 DMA interface control
#define rEP3_DMA_UNIT      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x104)) //EP3 DMA Tx unit counter
#define rEP3_DMA_FIFO      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x108)) //EP3 DMA Tx FIFO counter
#define rEP3_DMA_TTC_L     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x10c)) //EP3 DMA total Tx counter
#define rEP3_DMA_TTC_M     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x110))
#define rEP3_DMA_TTC_H     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x114))
#define rEP4_DMA_CON       (*(volatile unsigned char *)(USB_DEVICE_BASE+0x118)) //EP4 DMA interface control
#define rEP4_DMA_UNIT      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x11c)) //EP4 DMA Tx unit counter
#define rEP4_DMA_FIFO      (*(volatile unsigned char *)(USB_DEVICE_BASE+0x120)) //EP4 DMA Tx FIFO counter
#define rEP4_DMA_TTC_L     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x124)) //EP4 DMA total Tx counter
#define rEP4_DMA_TTC_M     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x128))
#define rEP4_DMA_TTC_H     (*(volatile unsigned char *)(USB_DEVICE_BASE+0x12c))

#elif defined(CONFIG_MMSP20)

#define rFUNC_ADDR_REG     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x000)) //Function address
#define rPWR_REG           (*(volatile unsigned short *)(USB_DEVICE_BASE+0x002)) //Power management
#define rEP_INT_REG        (*(volatile unsigned short *)(USB_DEVICE_BASE+0x004)) //EP Interrupt pending and clear
#define rUSB_INT_REG       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x00c)) //USB Interrupt pending and clear
#define rEP_INT_EN_REG     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x00e)) //Interrupt enable
#define rUSB_INT_EN_REG    (*(volatile unsigned short *)(USB_DEVICE_BASE+0x016))
#define rFRAME_NUM1_REG    (*(volatile unsigned short *)(USB_DEVICE_BASE+0x018)) //Frame number lower byte
#define rFRAME_NUM2_REG    (*(volatile unsigned short *)(USB_DEVICE_BASE+0x01a)) //Frame number higher byte
#define rINDEX_REG         (*(volatile unsigned short *)(USB_DEVICE_BASE+0x01c)) //Register index
#define rMAXP_REG          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x020)) //Endpoint max packet
#define rEP0_CSR           (*(volatile unsigned short *)(USB_DEVICE_BASE+0x022)) //Endpoint 0 status
#define rOUT_MAXP_REG      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x026)) //OUT Endpoint max packet : MMSP20 only
#define rIN_CSR1_REG       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x022)) //In endpoint control status
#define rIN_CSR2_REG       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x024))
#define rOUT_CSR1_REG      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x028)) //Out endpoint control status
#define rOUT_CSR2_REG      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x02a))
#define rOUT_FIFO_CNT1_REG (*(volatile unsigned short *)(USB_DEVICE_BASE+0x02c)) //Endpoint out write count
#define rOUT_FIFO_CNT2_REG (*(volatile unsigned short *)(USB_DEVICE_BASE+0x02e))
#define rEP0_FIFO          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x040)) //Endpoint 0 FIFO
#define rEP1_FIFO          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x042)) //Endpoint 1 FIFO
#define rEP2_FIFO          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x044)) //Endpoint 2 FIFO
#define rEP3_FIFO          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x046)) //Endpoint 3 FIFO
#define rEP4_FIFO          (*(volatile unsigned short *)(USB_DEVICE_BASE+0x048)) //Endpoint 4 FIFO
#define rEP1_DMA_CON       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x060)) //EP1 DMA interface control
#define rEP1_DMA_FIFO      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x064)) //EP1 DMA Tx FIFO counter
#define rEP1_DMA_TTC_L     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x066)) //EP1 DMA total Tx counter
#define rEP1_DMA_TTC_M     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x068))
#define rEP1_DMA_TTC_H     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x06a))
#define rEP2_DMA_CON       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x06c)) //EP2 DMA interface control
#define rEP2_DMA_FIFO      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x070)) //EP2 DMA Tx FIFO counter
#define rEP2_DMA_TTC_L     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x072)) //EP2 DMA total Tx counter
#define rEP2_DMA_TTC_M     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x074))
#define rEP2_DMA_TTC_H     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x076))
#define rEP3_DMA_CON       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x080)) //EP3 DMA interface control
#define rEP3_DMA_FIFO      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x084)) //EP3 DMA Tx FIFO counter
#define rEP3_DMA_TTC_L     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x086)) //EP3 DMA total Tx counter
#define rEP3_DMA_TTC_M     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x088))
#define rEP3_DMA_TTC_H     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x08a))
#define rEP4_DMA_CON       (*(volatile unsigned short *)(USB_DEVICE_BASE+0x08c)) //EP4 DMA interface control
#define rEP4_DMA_FIFO      (*(volatile unsigned short *)(USB_DEVICE_BASE+0x090)) //EP4 DMA Tx FIFO counter
#define rEP4_DMA_TTC_L     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x092)) //EP4 DMA total Tx counter
#define rEP4_DMA_TTC_M     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x094))
#define rEP4_DMA_TTC_H     (*(volatile unsigned short *)(USB_DEVICE_BASE+0x096))

#endif

#endif   // __BIG_ENDIAN

#define PWR_REG_DEFAULT_VALUE (DISABLE_SUSPEND)

//*************************************************************************
// USB definitons
//*************************************************************************

/* Power Management Register */
#define DISABLE_SUSPEND          0x00   
#define ENABLE_SUSPEND           0x01
#define SUSPEND_MODE		 0x02
#define MCU_RESUME               0x04
#define ISO_UPDATE		 (1<<7)

/* MAXP Register */
#define FIFO_SIZE_0              0x00  /* 0x00 * 8 = 0  */
#define FIFO_SIZE_8              0x01  /* 0x01 * 8 = 8  */
#define FIFO_SIZE_16             0x02  /* 0x02 * 8 = 16 */
#define FIFO_SIZE_32             0x04  /* 0x04 * 8 = 32 */
#define FIFO_SIZE_64             0x08  /* 0x08 * 8 = 64 */

/* ENDPOINT0 CSR (Control Status Register) : Mapped to IN CSR1 */
#define EP0_OUT_PKT_READY        0x01  /* USB sets, MCU clears by setting SERVICED_OUT_PKT_RDY */
#define EP0_IN_PKT_READY         0x02  /* MCU sets, USB clears after sending FIFO */
#define EP0_SENT_STALL           0x04  /* USB sets */       
#define EP0_DATA_END             0x08  /* MCU sets */
#define EP0_SETUP_END            0x10  /* USB sets, MCU clears by setting SERVICED_SETUP_END */
#define EP0_SEND_STALL           0x20  /* MCU sets */
#define EP0_SERVICED_OUT_PKT_RDY 0x40  /* MCU writes 1 to clear OUT_PKT_READY */
#define EP0_SERVICED_SETUP_END   0x80  /* MCU writes 1 to clear SETUP_END        */

#define EP0_WR_BITS              0xc0  

//EP_INT_REG / EP_INT_EN_REG
#define EP0_INT                	 0x01  // Endpoint 0, Control   
#define EP1_INT                  0x02  // Endpoint 1, (Bulk-In) 
#define EP2_INT                  0x04  // Endpoint 2 
#define EP3_INT			 0x08  // Endpoint 3, (Bulk-Out)   
#define EP4_INT			 0x10  // Endpoint 4

//USB_INT_REG / USB_INT_EN_REG
#define SUSPEND_INT            	 0x01  
#define RESUME_INT               0x02  
#define RESET_INT                0x04  

//IN_CSR1
#define EPI_IN_PKT_READY         0x01  
#define EPI_UNDER_RUN		 0x04
#define EPI_FIFO_FLUSH		 0x08
#define EPI_SEND_STALL           0x10  
#define EPI_SENT_STALL           0x20  
#define EPI_CDT			 0x40	
#define EPI_WR_BITS              (EPI_FIFO_FLUSH|EPI_IN_PKT_READY|EPI_CDT) 
					//(EPI_FIFO_FLUSH) is preferred  (???)
//IN_CSR2
#define EPI_IN_DMA_INT_MASK	(1<<4)
#define EPI_MODE_IN		(1<<5)
#define EPI_MODE_OUT		(0<<5)
#define EPI_ISO			(1<<6)
#define EPI_BULK		(0<<6)
#define EPI_AUTO_SET		(1<<7)

//OUT_CSR1
#define EPO_OUT_PKT_READY        0x01  
#define EPO_OVER_RUN		 0x04  
#define EPO_DATA_ERROR		 0x08  
#define EPO_FIFO_FLUSH		 0x10
#define EPO_SEND_STALL           0x20  
#define EPO_SENT_STALL           0x40
#define EPO_CDT			 0x80	
#define EPO_WR_BITS              (EPO_FIFO_FLUSH|EPO_SEND_STALL|EPO_CDT)
					//(EPO_FIFO_FLUSH) is preferred (???)
//OUT_CSR2
#define EPO_OUT_DMA_INT_MASK	(1<<5)
#define EPO_ISO		 	(1<<6)
#define EPO_BULK	 	(0<<6)
#define EPO_AUTO_CLR		(1<<7)

//USB DMA control register
#define UDMA_IN_RUN_OB		(1<<7)
#define UDMA_IGNORE_TTC		(1<<7)
#define UDMA_DEMAND_MODE	(1<<3)
#define UDMA_OUT_RUN_OB		(1<<2)
#define UDMA_OUT_DMA_RUN	(1<<2)
#define UDMA_IN_DMA_RUN		(1<<1)
#define UDMA_DMA_MODE_EN	(1<<0)

#define rEP1_DMA_TTC	(rEP1_DMA_TTC_L+(rEP1_DMA_TTC_M<<8)+(rEP1_DMA_TTC_H<<16))
#define rEP2_DMA_TTC	(rEP2_DMA_TTC_L+(rEP2_DMA_TTC_M<<8)+(rEP2_DMA_TTC_H<<16))
#define rEP3_DMA_TTC	(rEP3_DMA_TTC_L+(rEP3_DMA_TTC_M<<8)+(rEP3_DMA_TTC_H<<16))
#define rEP4_DMA_TTC	(rEP4_DMA_TTC_L+(rEP4_DMA_TTC_M<<8)+(rEP4_DMA_TTC_H<<16))

#define ADDR_EP0_FIFO 		((USB_DEVICE_BASE+0x080) //Endpoint 0 FIFO
#define ADDR_EP1_FIFO		((USB_DEVICE_BASE+0x084) //Endpoint 1 FIFO
#define ADDR_EP2_FIFO		((USB_DEVICE_BASE+0x088) //Endpoint 2 FIFO
#define ADDR_EP3_FIFO		((USB_DEVICE_BASE+0x08c) //Endpoint 3 FIFO
#define ADDR_EP4_FIFO		((USB_DEVICE_BASE+0x090) //Endpoint 4 FIFO

//If you chane the packet size, the source code should be changed!!!
#define EP0_PKT_SIZE             8	
#define EP1_PKT_SIZE             BULK_PKT_SIZE
#define EP3_PKT_SIZE             BULK_PKT_SIZE

#define EP0_STATE_INIT 			(0)  

//NOTE: The ep0State value in a same group should be added by 1.
#define EP0_STATE_GD_DEV_0	 	(10)  //10-10=0 
#define EP0_STATE_GD_DEV_1 		(11)  //11-10=1
#define EP0_STATE_GD_DEV_2 		(12)  //12-10=2

#define EP0_STATE_GD_CFG_0	 	(20)
#define EP0_STATE_GD_CFG_1 		(21)
#define EP0_STATE_GD_CFG_2 		(22)
#define EP0_STATE_GD_CFG_3 		(23)
#define EP0_STATE_GD_CFG_4 		(24)

#define EP0_STATE_GD_CFG_ONLY_0		(40)
#define EP0_STATE_GD_CFG_ONLY_1		(41)
#define EP0_STATE_GD_IF_ONLY_0 		(42)
#define EP0_STATE_GD_IF_ONLY_1 		(43)
#define EP0_STATE_GD_EP0_ONLY_0		(44)
#define EP0_STATE_GD_EP1_ONLY_0		(45)
#define EP0_INTERFACE_GET		(46)

#define EP0_STATE_GD_STR_I0	 	(30)  
#define EP0_STATE_GD_STR_I1	 	(31)  
#define EP0_STATE_GD_STR_I2	 	(32)  
#define EP0_STATE_GD_STR_I3	 	(70)  

#define EP0_CONFIG_SET    (33)
#define EP0_GET_STATUS0  (35)
#define EP0_GET_STATUS1  (36)
#define EP0_GET_STATUS2  (37)
#define EP0_GET_STATUS3  (38)
#define EP0_GET_STATUS4  (39)

//************************
//       Endpoint 0      
//************************

// Standard bmRequestTyje (Direction) 
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)    

// Standard bmRequestType (Type) 
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

// Standard bmRequestType (Recipient) 
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

// Feature Selectors 
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

// Standard Request Codes 
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

// Class-specific Request Codes 
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

// Descriptor Types
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)

//configuration descriptor: bmAttributes 
#define CONF_ATTR_DEFAULT	    (0x80) //Spec 1.0 it was BUSPOWERED bit.
#define CONF_ATTR_REMOTE_WAKEUP     (0x20)
#define CONF_ATTR_SELFPOWERED       (0x40)

//endpoint descriptor
#define EP_ADDR_IN		    (0x80)	
#define EP_ADDR_OUT		    (0x00)

#define EP_ATTR_CONTROL		    (0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK		    (0x2)
#define EP_ATTR_INTERRUPT	    (0x3)	

//string descriptor
#define LANGID_US_L 		    (0x09)  
#define LANGID_US_H 		    (0x04)

#define CLR_EP3_OUT_PKT_READY() rOUT_CSR1_REG= ( (out_csr3 &(~ EPO_WR_BITS)) &(~EPO_OUT_PKT_READY) )
#define SET_EP3_SEND_STALL()	rOUT_CSR1_REG= ( (out_csr3 & (~EPO_WR_BITS)) | EPO_SEND_STALL) )
#define CLR_EP3_SENT_STALL()	rOUT_CSR1_REG= ( (out_csr3 & (~EPO_WR_BITS)) &(~EPO_SENT_STALL) )
#define FLUSH_EP3_FIFO() 	rOUT_CSR1_REG= ( (out_csr3 & (~EPO_WR_BITS)) | EPO_FIFO_FLUSH) )

#define SET_EP1_IN_PKT_READY()  rIN_CSR1_REG= ( (in_csr1 &(~ EPI_WR_BITS)) | EPI_IN_PKT_READY )
#define SET_EP1_SEND_STALL()	rIN_CSR1_REG= ( (in_csr1 & (~EPI_WR_BITS)) | EPI_SEND_STALL) )
#define CLR_EP1_SENT_STALL()	rIN_CSR1_REG= ( (in_csr1 & (~EPI_WR_BITS)) &(~EPI_SENT_STALL) )
#define FLUSH_EP1_FIFO() 	rIN_CSR1_REG= ( (in_csr1 & (~EPI_WR_BITS)) | EPI_FIFO_FLUSH) )

#define CLR_EP0_OUT_PKT_RDY() 		rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) | EP0_SERVICED_OUT_PKT_RDY ) 
#define CLR_EP0_OUTPKTRDY_DATAEND() 	rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) | (EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END) )
#define SET_EP0_IN_PKT_RDY() 		rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) | (EP0_IN_PKT_READY) )
#define SET_EP0_INPKTRDY_DATAEND() 	rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) | (EP0_IN_PKT_READY|EP0_DATA_END) )
#define CLR_EP0_SETUP_END() 		rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) | (EP0_SERVICED_SETUP_END) )
#define CLR_EP0_SENT_STALL() 		rEP0_CSR=( (ep0_csr & (~EP0_WR_BITS)) & (~EP0_SENT_STALL) )

#define FLUSH_EP0_FIFO() 		{while(rOUT_FIFO_CNT1_REG)rEP0_FIFO;}

struct USB_SETUP_DATA{
	U8 bmRequestType;
	U8 bRequest;
	U8 bValueL;
	U8 bValueH;
	U8 bIndexL;
	U8 bIndexH;
	U8 bLengthL;
	U8 bLengthH;
};

struct USB_DEVICE_DESCRIPTOR{
	U8 bLength;
	U8 bDescriptorType;
	U8 bcdUSBL;
	U8 bcdUSBH;
	U8 bDeviceClass;
	U8 bDeviceSubClass;
	U8 bDeviceProtocol;
	U8 bMaxPacketSize0;
	U8 idVendorL;
	U8 idVendorH;
	U8 idProductL;
	U8 idProductH;
	U8 bcdDeviceL;
	U8 bcdDeviceH;
	U8 iManufacturer;
	U8 iProduct;
	U8 iSerialNumber;
	U8 bNumConfigurations;
};

struct USB_CONFIGURATION_DESCRIPTOR{
	U8 bLength;
	U8 bDescriptorType;
	U8 wTotalLengthL;
	U8 wTotalLengthH;
	U8 bNumInterfaces;
	U8 bConfigurationValue;
	U8 iConfiguration;
	U8 bmAttributes;
	U8 maxPower;
};

struct USB_INTERFACE_DESCRIPTOR{
	U8 bLength;
	U8 bDescriptorType;
	U8 bInterfaceNumber;
	U8 bAlternateSetting;
	U8 bNumEndpoints;
	U8 bInterfaceClass;
	U8 bInterfaceSubClass;
	U8 bInterfaceProtocol;
	U8 iInterface;
};

struct USB_ENDPOINT_DESCRIPTOR{
	U8 bLength;
	U8 bDescriptorType;
	U8 bEndpointAddress;
	U8 bmAttributes;
	U8 wMaxPacketSizeL;
	U8 wMaxPacketSizeH;
	U8 bInterval;
};

struct USB_CONFIGURATION_SET{
	U8 ConfigurationValue;
};

struct USB_GET_STATUS{
	U8 Device;
	U8 Interface;
	U8 Endpoint0;
	U8 Endpoint1;
	U8 Endpoint3;
};

struct USB_INTERFACE_GET{
	U8 AlternateSetting;
};
 
static const U8 descStr0[]={
	4,STRING_TYPE,LANGID_US_L,LANGID_US_H,  //codes representing languages
};

static const U8 descStr1[]={  //Manufacturer
	(0x14+2),STRING_TYPE, 
	'S',0x0,'y',0x0,'s',0x0,'t',0x0,'e',0x0,'m',0x0,' ',0x0,'M',0x0,
	'C',0x0,'U',0x0,
};

  
static const U8 descStr2[]={  //Product
	(0x2a+2),STRING_TYPE, 
	'S',0x0,'E',0x0,'C',0x0,' ',0x0,'S',0x0,'3',0x0,'C',0x0,'2',0x0,
	'4',0x0,'1',0x0,'0',0x0,'X',0x0,' ',0x0,'T',0x0,'e',0x0,'s',0x0,
	't',0x0,' ',0x0,'B',0x0,'/',0x0,'D',0x0
};

#define DESCSTR3_SIZE_MAX	128
static U8 descStr3[DESCSTR3_SIZE_MAX+2]={    //file name
	0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

U32 tempDownloadAddress;
U8 ep3Buf[EP3_PKT_SIZE];
U8 ep1Buf[EP1_PKT_SIZE];
int transferIndex=0;
U32 ep0State;
U32 ep0SubState;
int download_run = 0;
unsigned long cmd_offset;

volatile U8 Rwuen;
volatile U32 downloadAddress;
volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err=0;
volatile U32 totalDmaCount;
volatile int isUsbdSetConfiguration;
volatile U8 Configuration=1;
volatile U8 AlterSetting;
volatile U8 Selfpwr=TRUE;
volatile U8 device_status;
volatile U8 interface_status;
volatile U8 endpoint0_status;
volatile U8 endpoint1_status;
volatile U8 endpoint3_status;

struct USB_SETUP_DATA descSetup;
struct USB_DEVICE_DESCRIPTOR descDev;
struct USB_CONFIGURATION_DESCRIPTOR descConf;
struct USB_INTERFACE_DESCRIPTOR descIf;
struct USB_ENDPOINT_DESCRIPTOR descEndpt0;
struct USB_ENDPOINT_DESCRIPTOR descEndpt1;
struct USB_CONFIGURATION_SET ConfigSet;
struct USB_INTERFACE_GET InterfaceGet;
struct USB_GET_STATUS StatusGet;   //={0,0,0,0,0};

/* function prototype */
void InitUsbd(void);
void IsrUsbd(void);
void Ep3Handler(void);
void Ep3HandlerOptimized(void);
void ClearEp3OutPktReady(void);
void ConfigUsbd(void);
void ReconfigUsbd(void);
void RdPktEp0(U8 *buf,int num);
void WrPktEp0(U8 *buf,int num);
void WrByteEp0(U8 value);
void WrPktEp1(U8 *buf,int num);
void WrPktEp2(U8 *buf,int num);
void RdPktEp3(U8 *buf,int num);
void RdPktEp4(U8 *buf,int num);
void Ep1Handler(void);
void PrepareEp1Fifo(void);
void RdPktEp3_CheckSum(U8 *buf,int num);
void PrintEpiPkt(U8 *pt,int cnt);
void Ep0Handler(void);
void InitDescriptorTable(void);
void PrintEp0Pkt(U8 *pt);
unsigned int WaitDownload(unsigned long);

/* external function prototype */
extern void ChangeUPllValue(int mdiv,int pdiv,int sdiv);
extern void pullup_en(void);
extern void pullup_dis(void);
extern void usbd_en(void);
extern void usbd_dis(void);

unsigned int usbd_recv(unsigned long offset, char *file_name)
{
	int cnt;
	int len;
	unsigned int down_size;
	
	cmd_offset = offset;
	/* make string descriptor */
	if((len=strlen(file_name)) > (DESCSTR3_SIZE_MAX/2)) return 0;
	for(cnt=0;cnt<DESCSTR3_SIZE_MAX+2;cnt++) {
		descStr3[cnt] = 0;
	}
	descStr3[0] = len*2 + 2;
	descStr3[1] = STRING_TYPE;
	for(cnt=0; cnt<len; cnt++) {
		descStr3[cnt*2+2] = file_name[cnt];
		descStr3[cnt*2+3] = 0;
	}

	InitUsbd(); 
	udelay(0);  //calibrate udelay()

	pullup_dis();
	udelay(500000);

	usbd_en();

	download_run = 1;
	isUsbdSetConfiguration=0;

	down_size = WaitDownload(offset);

	pullup_dis();
	usbd_dis();
	udelay(500000);

	return down_size;
}

unsigned int WaitDownload(unsigned long offset)
{
	U32 j;
	U16 dnCS;
	int first=1;
	U8 tempMem[16];
	int cnt;
	
	DbgPrintf("****[%s:%d]->%s():\n", __FILE__, __LINE__, __FUNCTION__);

	checkSum=0;
	downloadAddress=(U32)tempMem;  
	downPt=(unsigned char *)downloadAddress;
	//This address is used for receiving first 8 byte.
	downloadFileSize=0;

	j=0;

	pullup_en();

	while(downloadFileSize==0)
	{
		IsrUsbd();

		if(first==1 && isUsbdSetConfiguration!=0)
		{
			printf("USB host is connected. Waiting a download.\n");
			first=0;
		}

	}

	printf("\nNow, Downloading [ADDRESS:%xh,TOTAL:%xh]\n",
		(U32)offset,downloadFileSize - 10);
	printf("RECEIVED FILE SIZE: 0x%8x",0);

	j=0x10000;

	while(((U32)downPt-downloadAddress)<(downloadFileSize-8))
	{
		IsrUsbd();

		if( ((U32)downPt-downloadAddress)>=j)
		{
			printf("\b\b\b\b\b\b\b\b%08x",j);
			j+=0x1000;
		}
	}
	printf("\b\b\b\b\b\b\b\b%08x\n",downloadFileSize - 10);

	//checkSum was calculated including dnCS. So, dnCS should be subtracted.
	checkSum=checkSum - *((unsigned char *)(downloadAddress+downloadFileSize-8-2))
		- *( (unsigned char *)(downloadAddress+downloadFileSize-8-1) );

	dnCS=*((unsigned char *)(downloadAddress+downloadFileSize-8-2))+
	(*( (unsigned char *)(downloadAddress+downloadFileSize-8-1) )<<8);

	if(checkSum!=dnCS)
	{
		printf("Checksum Error!!! (MEM:%xh DN:%xh)\n",checkSum,dnCS);
		return 0;
	}

	printf("Checksum O.K. (MEM:%xh DN:%xh)\n",checkSum,dnCS);
	printf("Download O.K.\n\n");

	for(cnt=0;cnt<64;cnt++) {
		descStr3[cnt] = 0;
	}

	return downloadFileSize - 10;
}

void init_usb_regs(void)
{
	rFUNC_ADDR_REG = 0x00;
	rPWR_REG = 0x00; 
	rEP_INT_REG = 0x00; 
	rUSB_INT_REG = 0x00;  
	rEP_INT_EN_REG = 0xff;   
	rUSB_INT_EN_REG = 0x04;   
	rFRAME_NUM1_REG = 0x00;   
	rFRAME_NUM2_REG = 0x00;   
	rINDEX_REG = 0x00;   
	rMAXP_REG = 0x01;   
	rEP0_CSR = 0x00; 
	rIN_CSR1_REG = 0x00;
	rIN_CSR2_REG = 0x00; 
	rOUT_CSR1_REG = 0x00;
	rOUT_CSR2_REG = 0x00; 
	rOUT_FIFO_CNT1_REG = 0x00;
	rOUT_FIFO_CNT2_REG = 0x00;
	rEP0_FIFO = 0x00; 
	rEP1_FIFO = 0x00; 
	rEP2_FIFO = 0x00;
	rEP3_FIFO = 0x00;   
	rEP4_FIFO = 0x00;  
	rEP1_DMA_CON = 0x00;
#if defined(CONFIG_S3C2410)
	rEP1_DMA_UNIT = 0x00;  
#endif
	rEP1_DMA_FIFO = 0x00;  
	rEP1_DMA_TTC_L = 0x00;   
	rEP1_DMA_TTC_M = 0x00;  
	rEP1_DMA_TTC_H = 0x00; 
	rEP2_DMA_CON = 0x00;  
#if defined(CONFIG_S3C2410)
	rEP2_DMA_UNIT = 0x00;
#endif
	rEP2_DMA_FIFO = 0x00;
	rEP2_DMA_TTC_L = 0x00;  
	rEP2_DMA_TTC_M = 0x00; 
	rEP2_DMA_TTC_H = 0x00;
	rEP3_DMA_CON = 0x00;  
#if defined(CONFIG_S3C2410)
	rEP3_DMA_UNIT = 0x00;
#endif
	rEP3_DMA_FIFO = 0x00;   
	rEP3_DMA_TTC_L = 0x00; 
	rEP3_DMA_TTC_M = 0x00;
	rEP3_DMA_TTC_H = 0x00;
	rEP4_DMA_CON = 0x00; 
#if defined(CONFIG_S3C2410)
	rEP4_DMA_UNIT = 0x00;   
#endif
	rEP4_DMA_FIFO = 0x00;
	rEP4_DMA_TTC_L = 0x00;  
	rEP4_DMA_TTC_M = 0x00;  
	rEP4_DMA_TTC_H = 0x00;  
}

void InitUsbd(void)
{
	DbgPrintf("****[%s:%d]->%s():\n", __FILE__, __LINE__, __FUNCTION__);

	ChangeUPllValue(40,1,2);  //UCLK=48Mhz
	InitDescriptorTable();
	//ResetUsbd();
	
	ConfigUsbd(); 

	//DetectVbus(); //not used in S3C2400X

	PrepareEp1Fifo(); 
}

void IsrUsbd(void)
{
	U8 usbdIntpnd,epIntpnd;
	U8 saveIndexReg = rINDEX_REG;

	usbdIntpnd = rUSB_INT_REG;
	epIntpnd = rEP_INT_REG;
	//DbgPrintf( "[INT:EP_I=%x,USBI=%x]",epIntpnd,usbIntpnd );

	if(usbdIntpnd&SUSPEND_INT) {
		rUSB_INT_REG = SUSPEND_INT;
		DbgPrintf( "<SUS]");
	}
	
	if(usbdIntpnd&RESUME_INT) {
		rUSB_INT_REG = RESUME_INT;
		DbgPrintf("<RSM]");
	}

	if(usbdIntpnd&RESET_INT) {
		DbgPrintf( "<RST]");  
		//ResetUsbd();
		ReconfigUsbd();
		rUSB_INT_REG = RESET_INT;  //RESET_INT should be cleared after ResetUsbd().   
		PrepareEp1Fifo(); 
	}

	if(epIntpnd&EP0_INT) {
		rEP_INT_REG = EP0_INT;  
		Ep0Handler();
	}
	if(epIntpnd&EP1_INT) {
		rEP_INT_REG = EP1_INT;  
		Ep1Handler();
	}

	if(epIntpnd&EP2_INT) {
		rEP_INT_REG = EP2_INT;  
		DbgPrintf("<2:TBD]");   //not implemented yet
		//Ep2Handler();
	}

	if(epIntpnd&EP3_INT) {
		rEP_INT_REG = EP3_INT;
		Ep3Handler();
	}

	if(epIntpnd&EP4_INT) {
		rEP_INT_REG = EP4_INT;
		DbgPrintf("<4:TBD]");   //not implemented yet
		//Ep4Handler();
	}

	rINDEX_REG = saveIndexReg;
}

void Ep3Handler(void)
{
	U8 out_csr3;
	int fifoCnt;
	rINDEX_REG=3;

	out_csr3=rOUT_CSR1_REG;
	
	DbgPrintf("<3:%x]",out_csr3);

	if(out_csr3 & EPO_OUT_PKT_READY)
	{   
		fifoCnt=rOUT_FIFO_CNT1_REG; 
#if 0
	RdPktEp3(ep3Buf,fifoCnt);
	PrintEpoPkt(ep3Buf,fifoCnt);
#else

		if(downloadFileSize==0)
		{
			RdPktEp3((U8 *)downPt,8); 
		
			if(download_run==0)
			{
				downloadAddress=tempDownloadAddress;
			}
			else
			{
#if 0			
				downloadAddress=
					*((U8 *)(downPt+0))+
					(*((U8 *)(downPt+1))<<8)+
					(*((U8 *)(downPt+2))<<16)+
					(*((U8 *)(downPt+3))<<24);
#else
				downloadAddress=cmd_offset;
#endif		
			}
			downloadFileSize=
				*((U8 *)(downPt+4))+
				(*((U8 *)(downPt+5))<<8)+
				(*((U8 *)(downPt+6))<<16)+
				(*((U8 *)(downPt+7))<<24);
			checkSum=0;
			downPt=(U8 *)downloadAddress;

			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt-8); //The first 8-bytes are deleted.
			downPt+=fifoCnt-8;  
		}
		else
		{
			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt); 
			downPt+=fifoCnt;  //fifoCnt=64
		}
#endif
	CLR_EP3_OUT_PKT_READY();
	return;
	}

	
	//I think that EPO_SENT_STALL will not be set to 1.
	if(out_csr3 & EPO_SENT_STALL)
	{
		DbgPrintf("[STALL]");
		CLR_EP3_SENT_STALL();
		return;
	}
}

void PrintEpoPkt(U8 *pt,int cnt)
{
	int i;
	DbgPrintf("[BOUT:%d:",cnt);
	for(i=0;i<cnt;i++)
		DbgPrintf("%x,",pt[i]);
	DbgPrintf("]");
}

void RdPktEp3_CheckSum(U8 *buf,int num)
{
	int i;
		
	for(i=0;i<num;i++)
	{
		buf[i]=(U8)rEP3_FIFO;
		checkSum+=buf[i];
	}
}


void ClearEp3OutPktReady(void)
{
	U8 out_csr3;
	rINDEX_REG=3;
	out_csr3=rOUT_CSR1_REG;
	CLR_EP3_OUT_PKT_READY();
}

void PrepareEp1Fifo(void) 
{
	int i;
	U8 in_csr1;
	rINDEX_REG=1;
	in_csr1=rIN_CSR1_REG;
	
	for(i=0;i<EP1_PKT_SIZE;i++) ep1Buf[i]=(U8)(transferIndex+i);
	WrPktEp1(ep1Buf,EP1_PKT_SIZE);
	SET_EP1_IN_PKT_READY(); 
}

void Ep1Handler(void)
{
	U8 in_csr1;
	rINDEX_REG=1;
	in_csr1=rIN_CSR1_REG;
	
	DbgPrintf("<1:%x]",in_csr1);

	//I think that EPI_SENT_STALL will not be set to 1.
	if(in_csr1 & EPI_SENT_STALL)
	{
		DbgPrintf("[STALL]");
		CLR_EP1_SENT_STALL();
		return;
	}

	//IN_PKT_READY is cleared
	
	//The data transfered was ep1Buf[] which was already configured 

	PrintEpiPkt(ep1Buf,EP1_PKT_SIZE); 
	
	transferIndex++;

	PrepareEp1Fifo(); 
	//IN_PKT_READY is set   
	//This packit will be used for next IN packit.

	return;
}

void PrintEpiPkt(U8 *pt,int cnt)
{
	int i;
	DbgPrintf("[B_IN:%d:",cnt);
	for(i=0;i<cnt;i++)
		DbgPrintf("%x,",pt[i]);
	DbgPrintf("]");
}

void Ep0Handler(void)
{
	static int ep0SubState;
	U8 ep0_csr;

	rINDEX_REG=0;
	ep0_csr=rEP0_CSR;
	
	DbgPrintf("<0:%x]",ep0_csr);

	//DATAEND interrupt(ep0_csr==0x0) will be ignored 
	//because ep0State==EP0_STATE_INIT when the DATAEND interrupt is issued.

	
	DbgPrintf2("****[%s:%d]->%s():\n", __FILE__, __LINE__, __FUNCTION__);
	if(ep0_csr & EP0_SETUP_END) {   
		// Host may end GET_DESCRIPTOR operation without completing the IN data stage.
		// If host does that, SETUP_END bit will be set.
		// OUT_PKT_RDY has to be also cleared because status stage sets OUT_PKT_RDY to 1.
		DbgPrintf("[SETUPEND]");
		CLR_EP0_SETUP_END();

		if(ep0_csr & EP0_OUT_PKT_READY) {
			FLUSH_EP0_FIFO(); //(???)
			//I think this isn't needed because EP0 flush is done automatically.   
			CLR_EP0_OUT_PKT_RDY();
		}
	
		ep0State=EP0_STATE_INIT;
		return;
	}

	//I think that EP0_SENT_STALL will not be set to 1.
	if(ep0_csr & EP0_SENT_STALL) {   
		DbgPrintf("[STALL]");
		CLR_EP0_SENT_STALL();
		if(ep0_csr & EP0_OUT_PKT_READY) {
			CLR_EP0_OUT_PKT_RDY();
		}

		ep0State=EP0_STATE_INIT;
		return;
	}

	if((ep0_csr & EP0_OUT_PKT_READY) && (ep0State==EP0_STATE_INIT)) {
		RdPktEp0((U8 *)&descSetup,EP0_PKT_SIZE);

		PrintEp0Pkt((U8 *)(&descSetup)); //DEBUG

		switch(descSetup.bRequest) {
			DbgPrintf2("****[%s:%d]->%s(): descSetup.bRequest(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, descSetup.bRequest);
		case GET_DESCRIPTOR:
			DbgPrintf2("****[%s:%d]->%s(): descSetup.bValueH(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, descSetup.bValueH);
			switch(descSetup.bValueH) {
			case DEVICE_TYPE:
				DbgPrintf("[GDD]");
				CLR_EP0_OUT_PKT_RDY();
				ep0State=EP0_STATE_GD_DEV_0;
				break;
			case CONFIGURATION_TYPE:
				DbgPrintf("[GDC]");
				CLR_EP0_OUT_PKT_RDY();
				if((descSetup.bLengthL+(descSetup.bLengthH<<8))>0x9)
					//bLengthH should be used for bLength=0x209 at WIN2K.
					ep0State=EP0_STATE_GD_CFG_0; //for WIN98,WIN2K
				else
					ep0State=EP0_STATE_GD_CFG_ONLY_0; //for WIN2K
				break;
			case STRING_TYPE:
				DbgPrintf("[GDS]");
				CLR_EP0_OUT_PKT_RDY();
				switch(descSetup.bValueL) {
				case 0:
					ep0State=EP0_STATE_GD_STR_I0;
					break;
				case 1:
					ep0State=EP0_STATE_GD_STR_I1;
					break;
				case 2:
					ep0State=EP0_STATE_GD_STR_I2;
					break;
				case 3:
					ep0State=EP0_STATE_GD_STR_I3;
					break;
				default:
					DbgPrintf("[UE:STRI?]");
					break;
				}
				ep0SubState=0;
				break;
			case INTERFACE_TYPE:
				DbgPrintf("[GDI]");
				CLR_EP0_OUT_PKT_RDY();
				ep0State=EP0_STATE_GD_IF_ONLY_0; //for WIN98
				break;
			case ENDPOINT_TYPE:
				DbgPrintf("[GDE]");
				CLR_EP0_OUT_PKT_RDY();
				switch(descSetup.bValueL&0xf) {
				case 0:
					ep0State=EP0_STATE_GD_EP0_ONLY_0;
					break;
				case 1:
					ep0State=EP0_STATE_GD_EP1_ONLY_0;
					break;
				default:
					DbgPrintf("[UE:GDE?]");
					break;
				}
				break;
			default:
				DbgPrintf("[UE:GD?]");
				break;
			}
			break;

		case SET_ADDRESS:
			DbgPrintf("[SA:%d]",descSetup.bValueL);
			rFUNC_ADDR_REG=descSetup.bValueL | 0x80;
			CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
			ep0State=EP0_STATE_INIT;
			break;

		case SET_CONFIGURATION:
			DbgPrintf("[SC]");
			ConfigSet.ConfigurationValue=descSetup.bValueL;
			CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
			ep0State=EP0_STATE_INIT;

			isUsbdSetConfiguration=1; 
			break;

	//////////////////////// For chapter 9 test ////////////////////
		case CLEAR_FEATURE:
			switch (descSetup.bmRequestType) {
			case DEVICE_RECIPIENT:
				if (descSetup.bValueL == 1)
					Rwuen = FALSE;
					break;

			case ENDPOINT_RECIPIENT:
				if (descSetup.bValueL == 0) {
					if((descSetup.bIndexL & 0x7f) == 0x00) {
						StatusGet.Endpoint0= 0;
					}
					if((descSetup.bIndexL & 0x8f) == 0x81) {           // IN  Endpoint 1
						StatusGet.Endpoint1= 0;
					}
					if((descSetup.bIndexL & 0x8f) == 0x03){          // OUT Endpoint 3
						StatusGet.Endpoint3= 0;  
					}
				}
				break;

			default:
				break;
			}
			CLR_EP0_OUTPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;
			break;

		case GET_CONFIGURATION:
			CLR_EP0_OUT_PKT_RDY();
			ep0State=EP0_CONFIG_SET;
			break;

		case GET_INTERFACE:
			CLR_EP0_OUT_PKT_RDY();
			ep0State=EP0_INTERFACE_GET;
			break;

		case GET_STATUS:
			switch(descSetup.bmRequestType) {
			case  (0x80):
				CLR_EP0_OUT_PKT_RDY();
				StatusGet.Device=((U8)Rwuen<<1)|(U8)Selfpwr;
				ep0State=EP0_GET_STATUS0;
				break;

			case  (0x81):
				CLR_EP0_OUT_PKT_RDY();
				StatusGet.Interface=0;
				ep0State=EP0_GET_STATUS1;
				break;

			case  (0x82):
				CLR_EP0_OUT_PKT_RDY();
				if((descSetup.bIndexL & 0x7f) == 0x00) {
					ep0State=EP0_GET_STATUS2;
				}

				if((descSetup.bIndexL & 0x8f) == 0x81){
					ep0State=EP0_GET_STATUS3;
				}
  
				if((descSetup.bIndexL & 0x8f) == 0x03){
					ep0State=EP0_GET_STATUS4;
				}
				break;

			default:
				break;
			}
			break;

		case SET_DESCRIPTOR:
			CLR_EP0_OUTPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;
			break;

		case SET_FEATURE:
			switch (descSetup.bmRequestType) {
			case DEVICE_RECIPIENT:
				if (descSetup.bValueL == 1)
					Rwuen = TRUE;
				break;

			case ENDPOINT_RECIPIENT:
				if (descSetup.bValueL == 0) {
					if((descSetup.bIndexL & 0x7f) == 0x00) {
						StatusGet.Endpoint0= 1;
					}
					if((descSetup.bIndexL & 0x8f) == 0x81) {
						StatusGet.Endpoint1= 1;
					}
					if((descSetup.bIndexL & 0x8f) == 0x03) {
						StatusGet.Endpoint3= 1;
					}
				}
				break;

			default:
				break;
			}
			CLR_EP0_OUTPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;
			break;

		case SET_INTERFACE:
			InterfaceGet.AlternateSetting= descSetup.bValueL;
			CLR_EP0_OUTPKTRDY_DATAEND(); 
			ep0State=EP0_STATE_INIT;
			break;

		case SYNCH_FRAME:
			ep0State=EP0_STATE_INIT;
			break;

		default:
			DbgPrintf("[UE:SETUP=%x]",descSetup.bRequest);
			CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
			ep0State=EP0_STATE_INIT;
			break;
		}
	}
	
	DbgPrintf2("****[%s:%d]->%s(): ep0State(%d)\n", __FILE__, __LINE__, __FUNCTION__, ep0State);
	switch(ep0State) {
	case EP0_STATE_INIT:
		break; 

	//=== GET_DESCRIPTOR:DEVICE ===

	case EP0_STATE_GD_DEV_0:
		DbgPrintf("[GDD0]");
		WrPktEp0((U8 *)&descDev+0,8); //EP0_PKT_SIZE
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_DEV_1;
		break;

	case EP0_STATE_GD_DEV_1:
		DbgPrintf("[GDD1]");
		WrPktEp0((U8 *)&descDev+0x8,8); 
		SET_EP0_IN_PKT_RDY();

		ep0State=EP0_STATE_GD_DEV_2;
		break;

	case EP0_STATE_GD_DEV_2:
		DbgPrintf("[GDD2]");
		WrPktEp0((U8 *)&descDev+0x10,2);   //8+8+2=0x12
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

	//=== GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 ===
	//Windows98 gets these 4 descriptors all together by issuing only a request.
	//Windows2000 gets each descriptor seperately.
	case EP0_STATE_GD_CFG_0:
		DbgPrintf("[GDC0]");
		WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_1;
		break;

	case EP0_STATE_GD_CFG_1:
		DbgPrintf("[GDC1]");
		WrPktEp0((U8 *)&descConf+8,1); 
		WrPktEp0((U8 *)&descIf+0,7); 
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_2;
		break;

	case EP0_STATE_GD_CFG_2:
		DbgPrintf("[GDC2]");
		WrPktEp0((U8 *)&descIf+7,2); 
		WrPktEp0((U8 *)&descEndpt0+0,6); 
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_3;
		break;

	case EP0_STATE_GD_CFG_3:
		DbgPrintf("[GDC3]");
		WrPktEp0((U8 *)&descEndpt0+6,1); 
		WrPktEp0((U8 *)&descEndpt1+0,7); 
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_4;
		break;

	case EP0_STATE_GD_CFG_4:
		DbgPrintf("[GDC4]");
		//zero length data packit 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

	//=== GET_DESCRIPTOR:CONFIGURATION ONLY===
	case EP0_STATE_GD_CFG_ONLY_0:
		DbgPrintf("[GDCO0]");
		WrPktEp0((U8 *)&descConf+0,8); //EP0_PKT_SIZE
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_ONLY_1;
		break;

	case EP0_STATE_GD_CFG_ONLY_1:
		DbgPrintf("[GDCO1]");
		WrPktEp0((U8 *)&descConf+8,1); 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

	//=== GET_DESCRIPTOR:INTERFACE ONLY===
	case EP0_STATE_GD_IF_ONLY_0:
		DbgPrintf("[GDI0]");
		WrPktEp0((U8 *)&descIf+0,8); 
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_IF_ONLY_1;
		break;
	case EP0_STATE_GD_IF_ONLY_1:
		DbgPrintf("[GDI1]");
		WrPktEp0((U8 *)&descIf+8,1); 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

	//=== GET_DESCRIPTOR:ENDPOINT 0 ONLY===
	case EP0_STATE_GD_EP0_ONLY_0:
		DbgPrintf("[GDE00]");
		WrPktEp0((U8 *)&descEndpt0+0,7); 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

	//=== GET_DESCRIPTOR:ENDPOINT 1 ONLY===
	case EP0_STATE_GD_EP1_ONLY_0:
		DbgPrintf("[GDE10]");
		WrPktEp0((U8 *)&descEndpt1+0,7); 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;

////////////////////////////////////////////

	 case EP0_INTERFACE_GET:
		WrPktEp0((U8 *)&InterfaceGet+0,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;


	//=== GET_DESCRIPTOR:STRING ===
	case EP0_STATE_GD_STR_I0:
		DbgPrintf("[GDS0_0]");
		WrPktEp0((U8 *)descStr0, 4 );  
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT; 
		ep0SubState=0;
		break;

	case EP0_STATE_GD_STR_I1:
		DbgPrintf("[GDS1_%d]",ep0SubState);
		if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr1) ) {
			WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
			SET_EP0_IN_PKT_RDY();
			ep0State=EP0_STATE_GD_STR_I1;
			ep0SubState++;
		} else {
			WrPktEp0((U8 *)descStr1+(ep0SubState*EP0_PKT_SIZE),
			sizeof(descStr1)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT; 
			ep0SubState=0;
		}
		break;

	case EP0_STATE_GD_STR_I2:
		DbgPrintf("[GDS2_%d]",ep0SubState);
		if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr2) ) {
			WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
			SET_EP0_IN_PKT_RDY();
			ep0State=EP0_STATE_GD_STR_I2;
			ep0SubState++;
		} else {
			DbgPrintf("[E]");
			WrPktEp0((U8 *)descStr2+(ep0SubState*EP0_PKT_SIZE),
			sizeof(descStr2)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT; 
			ep0SubState=0;
		}
		break;

	case EP0_STATE_GD_STR_I3:
		DbgPrintf("[GDS2_%d]",ep0SubState);
		if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr3) ) {
			WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
			SET_EP0_IN_PKT_RDY();
			ep0State=EP0_STATE_GD_STR_I3;
			ep0SubState++;
		} else {
			DbgPrintf("[E]");
			WrPktEp0((U8 *)descStr3+(ep0SubState*EP0_PKT_SIZE),
			sizeof(descStr3)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT; 
			ep0SubState=0;
		}
		break;
	case EP0_CONFIG_SET:
		WrPktEp0((U8 *)&ConfigSet+0,1); 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	case EP0_GET_STATUS0:
		WrPktEp0((U8 *)&StatusGet+0,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	case EP0_GET_STATUS1:
		WrPktEp0((U8 *)&StatusGet+1,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	case EP0_GET_STATUS2:
		WrPktEp0((U8 *)&StatusGet+2,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	case EP0_GET_STATUS3:
		WrPktEp0((U8 *)&StatusGet+3,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	case EP0_GET_STATUS4:
		WrPktEp0((U8 *)&StatusGet+4,1);
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;  
		break;

	default:
		DbgPrintf("UE:G?D");
		break;
	}
}

void PrintEp0Pkt(U8 *pt)
{
	int i;
	DbgPrintf("[RCV:");
	for(i=0;i<EP0_PKT_SIZE;i++)
		DbgPrintf("%x,",pt[i]);
	DbgPrintf("]");
}

void InitDescriptorTable(void)
{	
	//Standard device descriptor
	descDev.bLength=0x12;	//EP0_DEV_DESC_SIZE=0x12 bytes
	descDev.bDescriptorType=DEVICE_TYPE;
	descDev.bcdUSBL=0x10;
	descDev.bcdUSBH=0x01; 	//Ver 1.10
	descDev.bDeviceClass=0xFF; //0x0
	descDev.bDeviceSubClass=0x0;
	descDev.bDeviceProtocol=0x0;
	descDev.bMaxPacketSize0=0x8;
	descDev.idVendorL=0x78;
	descDev.idVendorH=0x56;
	descDev.idProductL=0x34;
	descDev.idProductH=0x12;
	descDev.bcdDeviceL=0x00;
	descDev.bcdDeviceH=0x01;
	descDev.iManufacturer=0x1;  //index of string descriptor
	descDev.iProduct=0x2;	//index of string descriptor 
	descDev.iSerialNumber=0x0;
	descDev.bNumConfigurations=0x1;

	//Standard configuration descriptor
	descConf.bLength=0x9;
	descConf.bDescriptorType=CONFIGURATION_TYPE;
	descConf.wTotalLengthL=0x20; //<cfg desc>+<if desc>+<endp0 desc>+<endp1 desc>
	descConf.wTotalLengthH=0;
	descConf.bNumInterfaces=1;
	//dbg descConf.bConfigurationValue=2;  //why 2? There's no reason.
	descConf.bConfigurationValue=1;  
	descConf.iConfiguration=0;
	descConf.bmAttributes=CONF_ATTR_DEFAULT|CONF_ATTR_SELFPOWERED;  //bus powered only.
	descConf.maxPower=25; //draws 50mA current from the USB bus.

	//Standard interface descriptor
	descIf.bLength=0x9;
	descIf.bDescriptorType=INTERFACE_TYPE;
	descIf.bInterfaceNumber=0x0;
	descIf.bAlternateSetting=0x0; //?
	descIf.bNumEndpoints=2;	//# of endpoints except EP0
	descIf.bInterfaceClass=0xff; //0x0 ?
	descIf.bInterfaceSubClass=0x0;  
	descIf.bInterfaceProtocol=0x0;
	descIf.iInterface=0x0;

	//Standard endpoint0 descriptor
	descEndpt0.bLength=0x7;
	descEndpt0.bDescriptorType=ENDPOINT_TYPE;
	descEndpt0.bEndpointAddress=1|EP_ADDR_IN;   // 2400Xendpoint 1 is IN endpoint.
	descEndpt0.bmAttributes=EP_ATTR_BULK;
	descEndpt0.wMaxPacketSizeL=EP1_PKT_SIZE; //64
	descEndpt0.wMaxPacketSizeH=0x0;
	descEndpt0.bInterval=0x0; //not used

	//Standard endpoint1 descriptor
	descEndpt1.bLength=0x7;
	descEndpt1.bDescriptorType=ENDPOINT_TYPE;
	descEndpt1.bEndpointAddress=3|EP_ADDR_OUT;   // 2400X endpoint 3 is OUT endpoint.
	descEndpt1.bmAttributes=EP_ATTR_BULK;
	descEndpt1.wMaxPacketSizeL=EP3_PKT_SIZE; //64
	descEndpt1.wMaxPacketSizeH=0x0;
	descEndpt1.bInterval=0x0; //not used 
}

void ConfigUsbd(void)
{

	ReconfigUsbd();
}


void ReconfigUsbd(void)
{
// *** End point information ***
//   EP0: control
//   EP1: bulk in end point
//   EP2: not used
//   EP3: bulk out end point
//   EP4: not used
	
	rPWR_REG=PWR_REG_DEFAULT_VALUE;	//disable suspend mode

	rINDEX_REG=0;
	rMAXP_REG=FIFO_SIZE_8;   	//EP0 max packit size = 8 
	rEP0_CSR=EP0_SERVICED_OUT_PKT_RDY|EP0_SERVICED_SETUP_END;
				//EP0:clear OUT_PKT_RDY & SETUP_END
	/* Endpoint1 Initialize */
	rINDEX_REG=1;
	#if (EP1_PKT_SIZE==32)
		rMAXP_REG=FIFO_SIZE_32;	//EP1:max packit size = 32
	#else
		rMAXP_REG=FIFO_SIZE_64;	//EP1:max packit size = 64
	#endif
	rIN_CSR1_REG=EPI_FIFO_FLUSH|EPI_CDT;
	rIN_CSR2_REG=EPI_MODE_IN|EPI_IN_DMA_INT_MASK|EPI_BULK; //IN mode, IN_DMA_INT=masked
	rOUT_CSR1_REG=EPO_CDT;
	rOUT_CSR2_REG=EPO_BULK|EPO_OUT_DMA_INT_MASK;

	/* Endpoint2 Initialize */
	rINDEX_REG=2;
	rMAXP_REG=FIFO_SIZE_64;	//EP2:max packit size = 64
	rIN_CSR1_REG=EPI_FIFO_FLUSH|EPI_CDT|EPI_BULK;
	rIN_CSR2_REG=EPI_MODE_IN|EPI_IN_DMA_INT_MASK; //IN mode, IN_DMA_INT=masked
	rOUT_CSR1_REG=EPO_CDT;
	rOUT_CSR2_REG=EPO_BULK|EPO_OUT_DMA_INT_MASK;

	/* Endpoint3 Initialize */
	rINDEX_REG=3;
	#if (EP3_PKT_SIZE==32)
		rMAXP_REG=FIFO_SIZE_32;	//EP3:max packit size = 32
	#else
		rMAXP_REG=FIFO_SIZE_64;	//EP3:max packit size = 64
	#endif
	rIN_CSR1_REG=EPI_FIFO_FLUSH|EPI_CDT|EPI_BULK;
	rIN_CSR2_REG=EPI_MODE_OUT|EPI_IN_DMA_INT_MASK; //OUT mode, IN_DMA_INT=masked
	rOUT_CSR1_REG=EPO_CDT;
	//clear OUT_PKT_RDY, data_toggle_bit.
	//The data toggle bit should be cleared when initialization.
	rOUT_CSR2_REG=EPO_BULK|EPO_OUT_DMA_INT_MASK;

	/* Endpoint4 Initialize */
	rINDEX_REG=4;
	rMAXP_REG=FIFO_SIZE_64;	//EP4:max packit size = 64
	rIN_CSR1_REG=EPI_FIFO_FLUSH|EPI_CDT|EPI_BULK;
	rIN_CSR2_REG=EPI_MODE_OUT|EPI_IN_DMA_INT_MASK; //OUT mode, IN_DMA_INT=masked
	rOUT_CSR1_REG=EPO_CDT;
	//clear OUT_PKT_RDY, data_toggle_bit.
	//The data toggle bit should be cleared when initialization.
	rOUT_CSR2_REG=EPO_BULK|EPO_OUT_DMA_INT_MASK;
	
	rEP_INT_REG=EP0_INT|EP1_INT|EP2_INT|EP3_INT|EP4_INT;
	rUSB_INT_REG=RESET_INT|SUSPEND_INT|RESUME_INT; 
	//Clear all usbd pending bits

	//EP0,1,3 & reset interrupt are enabled
	rEP_INT_EN_REG=EP0_INT|EP1_INT|EP3_INT;
	rUSB_INT_EN_REG=RESET_INT;
	ep0State=EP0_STATE_INIT;
}

void RdPktEp0(U8 *buf,int num)
{
	int i;

	for(i=0;i<num;i++)
	{
		buf[i]=(U8)rEP0_FIFO;
	}
}

void WrPktEp0(U8 *buf,int num)
{
	int i;
		
	for(i=0;i<num;i++)
	{
		rEP0_FIFO=buf[i];
	}
}

void WrByteEp0(U8 value)
{
	rEP0_FIFO= value;
}

void WrPktEp1(U8 *buf,int num)
{
	int i;

	for(i=0;i<num;i++) rEP1_FIFO=buf[i];
}

void WrPktEp2(U8 *buf,int num)
{
	int i;

	for(i=0;i<num;i++) rEP2_FIFO=buf[i];
}

void RdPktEp3(U8 *buf,int num)
{
	int i;

	for(i=0;i<num;i++) buf[i]=(U8)rEP3_FIFO;
}

void RdPktEp4(U8 *buf,int num)
{
	int i;

	for(i=0;i<num;i++) buf[i]=(U8)rEP4_FIFO;
}


#endif /* CONFIG_DRIVER_USB_DEVICE */
