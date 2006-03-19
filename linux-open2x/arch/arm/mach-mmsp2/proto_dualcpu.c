#include <linux/module.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_dualcpu.h>
#include <asm/hardware.h>

//Register Declaration..
MMSP20_DUALCPU *DualCPU_REG;

void DualCPU_Initialize(void) 
{
	DualCPU_REG = MMSP20_GetBase_DualCPU();
}

void DualCPU_ClearInterrupt(void)
{
        DualCPU_REG->DUALINT920 = 0; 
        DualCPU_REG->DUALPEND920 = 0xFF;
        DualCPU_REG->DUALINT940 = 0;
        DualCPU_REG->DUALPEND940 = 0xFF;
}

void DualCPU_InitHardware(void)
{
        DualCPU_ClearInterrupt();

        // 940BANK address 
        DualCPU_REG->DUALCTRL940 = (DualCPU_REG->DUALCTRL940 & 0xFF80) | (PA_ARM940_BASE >> 24);
        DualCPU_REG->DUALINT940 = 0x00;
        DualCPU_REG->DUALPEND940 = 0xFF;
}

void DualCPU_operate_mode_on(void)
{
        DualCPU_REG->DUALCTRL940 = (DualCPU_REG->DUALCTRL940 & 0xFF7F) | (0 << 7);            
}

void DualCPU_CloseHardware(void)
{
        DualCPU_ClearInterrupt();
        DualCPU_REG->DUALCTRL940 |= (1 << 7);            
}

void ARM920_ClearInterrupt(void)
{
    DualCPU_REG->DUALPEND920 = 0xFF; // interrupt pending bit all disable
    DualCPU_REG->DUALINT920 = 0x00;  // interrupt status register all clear
}

EXPORT_SYMBOL(DualCPU_Initialize);
EXPORT_SYMBOL(DualCPU_InitHardware);
EXPORT_SYMBOL(DualCPU_operate_mode_on);
EXPORT_SYMBOL(DualCPU_CloseHardware);
EXPORT_SYMBOL(ARM920_ClearInterrupt);
