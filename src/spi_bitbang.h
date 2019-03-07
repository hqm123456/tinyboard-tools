#ifndef SPI_BITBANG_H
#define SPI_BITBANG_H

#include "stdint.h"

#ifndef LSBFIRST
#define LSBFIRST 0
#endif 

#ifndef MSBFIRST
#define MSBFIRST 1
#endif

#define SPI_MODE0 0x00 //CPOL=0,CPHA=0
#define SPI_MODE1 0X01 //CPOL=0,CPHA=1
#define SPI_MODE2 0x02 //CPOL=1,CPHA=0
#define SPI_MOED3 0x03 //CPOL=1,CPHA=1

static const uint8_t SCL_H = 0b0001;
static const uint8_t SDO_H = 0b0010;
static const uint8_t SDI_H = 0b0100;
static const uint8_t CS_H  = 0b1000;



uint32_t spi_read(uint8_t *dst);
uint32_t spi_decoder(uint8_t *dst,uint8_t *dat,uint8_t len);
uint32_t spi_write(uint8_t* dst,uint8_t dat);
uint32_t spi_initialize(uint8_t *dst);
uint32_t spi_cs_start(uint8_t *dst);
uint32_t spi_cs_end(uint8_t *dst);

#endif