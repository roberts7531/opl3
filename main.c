#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "jox030.h"
#include "graphics.h"
uint32_t parseLittleEndian(uint8_t* valLE){
    uint32_t size;
    size = *valLE;
    valLE++;
    size |= *valLE<<8;
    valLE++;
    size |= *valLE<<16;
    valLE++;
    size |= *valLE<<24;
    return size;

}
volatile uint8_t* duartSR = (volatile uint8_t*)0xc0000005;
volatile uint8_t* duartRB = (volatile uint8_t*)0xc000000b;
volatile uint8_t* duartSC = (volatile uint8_t*)0xc000000f;
volatile uint8_t* duartCU = (volatile uint8_t*)0xc0000006;
volatile uint8_t* duartCL = (volatile uint8_t*)0xc0000007;


volatile uint8_t tick = 0;
uint8_t oldTick = 0;
void __attribute__((interrupt)) duartInt(){
	uint8_t status = *duartSR;
	*duartSR = 0x00;
    tick++;


  status = *duartSC;
	*duartSR = 0x08;

}

void setupTimer(uint16_t delay){
  *duartSR = 0x08;
  *duartCU = delay>>8;
  *duartCL = delay&0xff;
  installIntHandler(6,duartInt);

}
volatile uint8_t* primInd = (volatile uint8_t*)0XC0600000;
volatile uint8_t* primData = (volatile uint8_t*)0XC0600001;
volatile uint8_t* secInd = (volatile uint8_t*)0XC0600002;
volatile uint8_t* secData = (volatile uint8_t*)0XC0600003;
uint8_t readStatus(){
  return *primInd;
}
void delayLoop(){
long i;
  for(i=0;i<3;i++){
    readStatus();
  }

}


uint8_t writeReg(uint16_t reg, uint8_t data){
    volatile uint8_t status;

  if(reg&0xff00){
    *secInd = (uint8_t)reg;
      status += readStatus();
    *secData = data;
  }else{
    *primInd = (uint8_t)reg;
      status += readStatus();
    *primData = data;
  }
  status += readStatus();
  status += readStatus();
  status += readStatus();
  drawVerticalBar(reg,data);
  return status;
}

uint8_t isTimerElapsed(){
  if(oldTick!=tick){
    oldTick = tick;
    return 1;
  }
  return 0;
}
void __attribute__((interrupt)) soundIRQHandler(){
  printf("Interrupt!!!");
  writeReg(0x4,0x80);
  writeReg(0x4,0x60);
}

void writeChannel(uint8_t regbase, uint8_t channel, uint8_t data1,uint8_t data2){

    static uint8_t sbpro_op[] = { 0,  1,  2,   6,  7,  8,  12, 13, 14,
			      18, 19, 20,  24, 25, 26,  30, 31, 32};
    static uint16_t rg[] = {0x000,0x001,0x002,0x003,0x004,0x005,
			0x008,0x009,0x00A,0x00B,0x00C,0x00D,
			0x010,0x011,0x012,0x013,0x014,0x015,
			0x100,0x101,0x102,0x103,0x104,0x105,
			0x108,0x109,0x10A,0x10B,0x10C,0x10D,
			0x110,0x111,0x112,0x113,0x114,0x115};


	uint8_t reg = sbpro_op[channel];
	writeReg(rg[reg]+regbase, data1);
	writeReg(rg[reg+3]+regbase, data2);

}

void writeValue(uint8_t regbase, uint8_t channel, uint8_t value)
{
    static uint16_t ch[] = {0x000,0x001,0x002,0x003,0x004,0x005,0x006,0x007,0x008,
			0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108};
    uint16_t chan;


	chan = ch[channel];
    writeReg(regbase + chan, value);
}

int main(int argc, char* argv[]){
    if(argc!=2){
        printf("Must have only one argument!\r\n");
        return 1;	
    }
	*((uint32_t*)0x74) = soundIRQHandler;  
    FILE* file = fopen(argv[1],"rb");
    if(!file){
    	printf("Failed to open: %s\r\n",argv[1]);
	return 1;
    }
    char header[4];
    fread(&header,1,4,file);
    if(header[0]!='V' | header[1]!='g' | header[2]!='m'){
    	printf("Bad VGM header!\r\n");
	return 1;
    }
    uint8_t sizeLE[4];
    fread(&sizeLE,1,4,file);

    uint32_t size = parseLittleEndian((uint8_t*)&sizeLE)+4;

    printf("Size: %d\r\n",size);
    uint8_t* vgmAlloc = (uint8_t*) malloc(size);
    if(!vgmAlloc){
    	printf("Failed to allocate!\r\n");
	return 1;
    }
    rewind(file);
    fread(vgmAlloc,1,size,file);
    uint32_t VGMDataOffset = parseLittleEndian(vgmAlloc+0x34)+0x34;
    uint32_t YMF262clk = parseLittleEndian(vgmAlloc+0x5c);
    uint32_t YM3812clk = parseLittleEndian(vgmAlloc+0x50);
    printf("Vgm data offset:%x\r\n",VGMDataOffset);
    printf("YMF262 clock: %d\r\n",YMF262clk);
    printf("YM3812 clock: %d\r\n",YM3812clk);
    printf("Size: %d\r\n",size);
    if(YMF262clk==0 && YM3812clk==0){
    	printf("Only OPL2 and OPL3 vgm's are supported!\r\n");
	return 1;
    }
    uint8_t data;
    uint16_t reg;
    uint16_t samplesToDelay = 0;
    long pointer;
    setupTimer(0x3);
    setupGraphics();
    for(pointer=VGMDataOffset;pointer<size;pointer++){
      if(!isTimerElapsed()){
        pointer--;
        continue;
      }
	
	if(samplesToDelay>0){
	    samplesToDelay--;
	    pointer--;
	    continue;
	}

	  if(vgmAlloc[pointer]==0x5e){
          pointer++;
          reg = vgmAlloc[pointer];
          pointer++;
          data = vgmAlloc[pointer];
          writeReg(reg,data);
        }
        else if(vgmAlloc[pointer]==0x5f){
          pointer++;
          reg = vgmAlloc[pointer]|0x100;
          pointer++;
          data = vgmAlloc[pointer];
          writeReg(reg,data);
        }
        else if(vgmAlloc[pointer]==0x61){
          pointer++;
          samplesToDelay = vgmAlloc[pointer];
          pointer++;
          samplesToDelay = (samplesToDelay)|vgmAlloc[pointer]<<8;
        }

        else if(vgmAlloc[pointer]==0x5a){
          pointer++;
          reg = vgmAlloc[pointer];
          pointer++;
          data = vgmAlloc[pointer];
          writeReg(reg,data);
        }
        else if(vgmAlloc[pointer]==0x63){
          	//printf("Opcode 0x63\r\n");
		samplesToDelay=882;
	 
	}
           
    }


    
}
