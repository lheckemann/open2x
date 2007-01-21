#ifndef _PROTO_DUALCPU_H_
#define _PROTO_DUALCPU_H_

#include <asm/arch/wincetype.h>

#ifdef __cplusplus
extern "C" {
#endif

void DualCPU_Initialize(void); 
void DualCPU_ClearInterrupt(void);
void DualCPU_InitHardware(void);
void DualCPU_operate_mode_on(void);
void DualCPU_CloseHardware(void);
void ARM920_ClearInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif // _PROTO_DUALCPU_H_

