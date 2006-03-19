#ifndef _PROTO_PPM_H_
#define _PROTO_PPM_H_

typedef char                    S8;
typedef short                   S16;
typedef long                    S32;

typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned long   U32;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Start For Brian
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef char                    CHAR, SCHAR;
typedef short                   SHORT, SSHORT;
typedef int                             INT, SINT;
typedef long                    LONG, SLONG;
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;
typedef void                    VOID;
typedef unsigned char   BOOL;

#define PPM_ENB                         0x8000
#define PPM_INPOL_NEG           0
#define PPM_INPOL_BYPASS        0x4000

#define PPM_INT_RISEEDGE        0x0001
#define PPM_INT_FALLEDGE        0x0002
#define PPM_INT_OVERFLOW        0x0004

#define PPM_PEND_RISEEDGE       0x0001
#define PPM_PEND_FALLEDGE       0x0002
#define PPM_PEND_OVERFLOW       0x0004

#define PPM_CLR_RISEEDGE        0x0001
#define PPM_CLR_FALLEDGE        0x0002
#define PPM_CLR_OVERFLOW        0x0004

typedef struct tPPMPerid {
                BOOL    OverflowHigh;
                USHORT  High;
                BOOL    OverflowLow;
                USHORT  Low;
} PPMPERD;

extern UINT     gPPM_IOBaseOffset;

#define PPMCTRL         *(volatile USHORT *)( gPPM_IOBaseOffset + 0x00 )
#define PPMSTATUS       *(volatile USHORT *)( gPPM_IOBaseOffset + 0x04 )
#define PPMLOWPERD      *(volatile USHORT *)( gPPM_IOBaseOffset + 0x06 )
#define PPMHIGHPERD     *(volatile USHORT *)( gPPM_IOBaseOffset + 0x08 )

//-----------------------------------------------------------------------------
//Sunsung Remocon
//-----------------------------------------------------------------------------
#define Custom_Code_0           0xE0
#define Custom_Code_1           0xA0

#define Button_Power    0x40
#define Button_VOL_UP   0xE0
#define Button_VOL_DOWN 0xD0
#define Button_CH_UP    0x48
#define Button_CH_DOWN  0x08

#define NUM_FIRST_LOW   0X7000          // 9.2ms
#define NUM_FIRST_HIGH  0x7000          // 4.32ms
#define NUM_LOW                 0x0800// 500us
#define NUM_0_HIGH              0x1300// 500us
#define NUM_1_HIGH              0x3500// 500us

#define PPM_START_CODE_DETECTION        0
#define PPM_DATA_INPUT                          1
//#define       PPM_END_DETECTION                       3


//VOID    PPM_GetDeviceInfo(      MES_DEVINFO *pDevInfo);
VOID    PPM_Initialize( UINT    VirIOAddr,      USHORT  SetInputPol);
VOID    PPM_Close(VOID);
VOID    PPM_SetInterruptEnb(    UINT    IntEnb);
UINT    PPM_GetInterruptEnb(VOID);
UINT    PPM_GetInterruptPend(VOID);
VOID    PPM_ClrInterruptPend(   UINT ClrPend);
VOID    PPM_GetPeriod(  PPMPERD         *pPeriod);

#endif  // _PROTO_PPM_H_

