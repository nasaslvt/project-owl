/*
This program is used to measure the duty cycle of a pulse train signal.
The signal is connected to P8_20.
*/

#include <stdint.h>
#include <pru_ctrl.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

#define P8_27 1 << 8 //pr1_pru1_pru_r31_8 (page 62 BBB_SRM)

volatile register uint32_t __R31;

#define PRU1_DRAM       0x00000         // Offset to DRAM
// Skip the first 0x200 bytes of DRAM since the Makefile allocates
// 0x100 for the STACK and 0x100 for the HEAP.
volatile uint32_t *pru1_dram = (uint32_t *) (PRU1_DRAM + 0x200);

void main(void)
{
    PRU1_CTRL.CTRL_bit.CTR_EN = 1;

    while(1) {
        PRU1_CTRL.CYCLE = 0;

        while(__R31 & P8_27);
        pru1_dram[0] = PRU1_CTRL.CYCLE;

        while((__R31 & P8_27)==0);
        pru1_dram[1] = PRU1_CTRL.CYCLE;
    }
}