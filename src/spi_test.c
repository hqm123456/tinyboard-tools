#include "ftd2xx.h"
#include "spi_bitbang.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


#define READ_DATA 0x03
#define WRITE_ENABLE 0x06
#define PAGE_PROGRAM 0x02
#define CHIP_ERASE 0xC7

int main(int argc, char *argv[])
{
  FT_STATUS ftStatus;
  FT_HANDLE handle;
  DWORD numDevs;
  int sel_dev = 0;

  ftStatus = FT_CreateDeviceInfoList(&numDevs);
  if (ftStatus != FT_OK) {
    printf("Can not create device info list.");
    return 1;
  }
  if (numDevs == 0) {
    printf("Not find any FTDI device.");
    return 0;
  }

  if (argc != 1) {
    if (is_integer(argv[1])) {
      sscanf(argv[1], "%d", &sel_dev);
    }
  }else{
    puts("fail");
  }

  if (FT_Open(sel_dev, &handle)) {
    puts("Can't open device");
    return 1;
  }

  ftStatus = FT_SetBitMode(handle, 0x0F, FT_BITMODE_SYNC_BITBANG);
  if (ftStatus) {
    return 1;
  }

  ftStatus = FT_SetBaudRate(handle, 9600);
  if (ftStatus) {
    return 1;
  }

  FT_Purge(handle, FT_PURGE_RX);


  uint8_t dst[1024]={0};
  uint8_t length = 2,restart_point;
  uint8_t dat[10]={0};
  uint32_t len=0;

  //   len  += spi_initialize(dst);

  //   len += spi_cs_start(dst);

  //   len += spi_write(dst+len,0x90);
  //   len += spi_write(dst+len,0x00);
  //   len += spi_write(dst+len,0x10);
  //   len += spi_write(dst+len,0x00);


  //   restart_point = len;
  //   len += spi_read(dst+len);
  //   len += spi_read(dst+len);
  //   len += spi_read(dst+len);

  //  len += spi_cs_end(dst+len);


  //写

  len = spi_cs_start(dst);
  len += spi_write(dst+len,WRITE_ENABLE);

  len += spi_cs_start(dst+len);
  len += spi_write(dst+len,PAGE_PROGRAM);




  len += spi_write(dst+len,0x00);
  len += spi_write(dst+len,0x12);
  len += spi_write(dst+len,0x34);


  len += spi_write(dst+len,0x45);
  len += spi_cs_end(dst+len);

  //读

  len += spi_cs_start(dst+len);
  len += spi_write(dst+len,READ_DATA); //读 cmd

  len += spi_write(dst+len,0x00);   //地址
  len += spi_write(dst+len,0X12);
  len += spi_write(dst+len,0x34); 

  restart_point = len;

  len += spi_read(dst+len);
  len += spi_cs_end(dst+len);



  uint32_t nbytes;

  printf("\n");
  FT_Write(handle, dst, len, (LPDWORD)&nbytes);
  FT_Read(handle, dst, nbytes, (LPDWORD)&nbytes);

  spi_decoder(dst+restart_point,dat,1);

  for(int i=0;i<1;i++)
  {
    printf("%0x\n",dat[i]);
  }



  return 0;
}