#ifndef JOX030_H
#define JOX030_H

#include <stdint.h>

#define GPU_IO_DATA 0xc080ffff
#define GPU_IO_REGNO 0xc080fffe
#define GPU_IO_WPROT 0xc080fffd
#define GPU_VRAM_BASE 0xC0800000
#define GPU_REG_MODE 0

void gpuWriteReg(uint8_t reg, uint8_t data);
void installIntHandler(uint8_t autovector, void* handler);



#endif