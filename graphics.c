#include "jox030.h"
#include <stdint.h>
volatile uint8_t* vram = (volatile uint8_t*)GPU_VRAM_BASE;

void setupGraphics(){
    gpuWriteReg(GPU_REG_MODE,1);
}

uint16_t x = 0;

void drawVerticalBar(uint16_t x8, uint8_t value){
    uint16_t yPos = x*320;
    uint8_t y;
    uint8_t color;
    for(y=190;y>0;y--){
        if(y>190-value){
            color = y&0xf;
        }else{
            color = 0;
        }

        *(vram + yPos+y) = color;
    }
    if(x<190)
    x++;
    else
    x=0;
    
}
