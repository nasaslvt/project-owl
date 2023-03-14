/*
This program is used to measure the duty cycle of a pulse train signal.
The signal is connected to P8_15.
*/

#include <stdint.h>
#include <pru_ctrl.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

#define P8_15 1 << 15 //pr1_pru0_pru_r31_15 (page 62 BBB_SRM)

volatile register uint32_t __R31;

#define PRU0_DRAM       0x00000         // Offset to DRAM
// Skip the first 0x200 bytes of DRAM since the Makefile allocates
// 0x100 for the STACK and 0x100 for the HEAP.
volatile uint32_t *pru0_dram = (uint32_t *) (PRU0_DRAM + 0x200);

void main(void)
{
    PRU0_CTRL.CTRL_bit.CTR_EN = 1;

    while(1) {
        PRU0_CTRL.CYCLE = 0;

        while(__R31 & P8_15);        
        pru0_dram[0] = PRU0_CTRL.CYCLE;

        while((__R31 & P8_15)==0);
        pru0_dram[1] = PRU0_CTRL.CYCLE;
    }
}