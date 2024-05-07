#include "jox030.h"
uint32_t intHandlerBackup  = 0;

void gpuWriteReg(uint8_t reg, uint8_t data){
	*((volatile uint8_t*)GPU_IO_WPROT) = 0xaf;
	*((volatile uint8_t*)GPU_IO_REGNO) = reg;
	*((volatile uint8_t*)GPU_IO_DATA) = data;
}


void installIntHandler(uint8_t autovector, void* handler){
    autovector += 24;
    autovector = autovector<<2;
    uint32_t vector = autovector;
    intHandlerBackup = *((uint32_t*)vector);
    *((uint32_t*)vector) = (uint32_t) handler;
}

void restoreIntHandler(uint8_t autovector){
    autovector += 24;
    autovector = autovector<<2;
    uint32_t vector = autovector;
    *((uint32_t*)vector) = intHandlerBackup;
}
