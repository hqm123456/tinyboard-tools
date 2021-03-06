#include "spi_bitbang.h"
#include "windows.h"
#include <stdio.h>

#define serial_spi_read_size  16
#define serial_spi_write_size 16

#define MODE0 1

uint32_t spi_initialize(uint8_t *dst)
{
#ifdef MODE2
    dst[0] = SCL_H;
#elif MODE3
    dst[0] = SCL_H;
#else
    dst[0] = 0;
#endif
    return 1;
}

uint32_t spi_cs_start(uint8_t *dst)
{
#ifdef MODE2
    dst[0] = CS_H | SCL_H;
    dst[1] = 0 | SCL_H;
#elif MODE3
    dst[0] = CS_H | SCL_H;
    dst[1] = 0 | SCL_H;
#else
    dst[0] = CS_H;
    dst[1] = 0;
#endif
    return 2;
}

uint32_t spi_cs_end(uint8_t *dst)
{
#ifdef MODE2
    dst[0] = CS_H | SCL_H;
#elif  MODE3
    dst[0] = CS_H | SCL_H;
#else
    dst[0] = CS_H;
#endif
    return 1;
}

//CPOL=0,CPHA=0;  第一个跳变沿读取数据
//
uint32_t spi_read(uint8_t *dst)
{
    int i;

#ifdef MODE0     
    for(i=0;i<8;i++)
    {
        dst[2*i] = 0;

        dst[2*i+1] = SCL_H;    
    }
#elif MODE1
    for(i=0;i<8;i++)
    {
        dst[2*i] = SCL_H;
        dst[2*i+1] = 0;    
    }
#elif MODE2
    for(i=0;i<8;i++)
    {
        dst[2*i] = 0;
        dst[2*i+1] = SCL_H;    
    }
#elif MODE3
    for(i=0;i<8;i++)
    {
        dst[2*i] = SCL_H;
        dst[2*i+1] = 0;        
    }
#endif
    return 16;
}

uint32_t spi_write(uint8_t* dst,uint8_t dat)
{
    int i;
#ifdef MODE0
    for(i=0;i<8;i++)
    {
        dst[2*i] = 0;   
        if(dat&0x80)
          dst[2*i+1] = SDO_H | SCL_H;
        else
        {
          dst[2*i+1] = SCL_H;
        }
        dat <<= 1;   
    }
    return serial_spi_write_size;
#elif MODE1
    for(i=0;i<8;i++)
    {
        dst[3*i] = 0;

        dst[3*i + 1] = SCL_H;
        if(dat&0x80)
          dst[3*i+2] = SDO_H;
        else
        {
          dst[3*i +2] = 0;
        }
        dat <<= 1;
        
    }
    return 24;
#elif MOMDE2
    for(i=0;i<8;i++)
    {
        dst[3*i] = SCL_H;
        if(dat&0x80)
          dst[3*i+1] = SDO_H;
        else
        {
          dst[3*i+1] = 0;
        }
        dat <<= 1;

       dst[3*i+2] = SCL_H;
    }
    return 24;
#elif MODE3
    for(i=0;i<8;i++)
    {
        dst[3*i] = SCL_H;

        dst[3*i + 1] = 0;
        if(dat&0x80)
          dst[3*i+2] = SDO_H | SCL_H;
        else
        {
          dst[3*i +2] = 0 | SCL_H;
        }
        dat <<= 1;
    }
    return 24;
#endif

}

uint32_t spi_decoder(uint8_t *dst,uint8_t *dat,uint8_t len)
{
    int i,j;
    uint8_t adat;
    for(j = 0;j<len;j++)
    {
        adat = 0x00;
        for(i=0;i<8;i++)
        {
            adat <<= 1;
            if(dst[2*i+1] & SDI_H)
                adat |= 0x01;
            else
            {
                adat &= 0xfe;
            }
  
        }
        dat[j] = adat;
    }
    return 0;
}