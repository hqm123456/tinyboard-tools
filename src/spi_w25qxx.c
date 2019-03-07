#include "ftd2xx.h"
#include "spi_bitbang.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


#define READ_DATA 0x03
#define READ_STATUS_1 0x05
#define WRITE_ENABLE 0x06
#define PAGE_PROGRAM 0x02
#define CHIP_ERASE_SUSPEND 0x60
#define CHIP_ERASE_RESUME  0x7A
#define SECTOR_ERASE 0x20


#define EER_CREATE 1
#define EER_NODEVICE 2


const char* help = "Usage:w25qxx I2CBUSS\n";

uint8_t ReadDeviceID(FT_HANDLE handle)
{

    uint8_t serial[1024]={0};
    uint8_t restart_point,dat;
    uint32_t len=0;
    DWORD nbytes;

    len += spi_cs_start(serial);

    len += spi_write(serial+len,0x90);
    len += spi_write(serial+len,0x00);
    len += spi_write(serial+len,0x00);
    len += spi_write(serial+len,0x00);


    restart_point = len;
    len += spi_read(serial+len);

    len += spi_cs_end(serial+len);

    FT_Write(handle, serial, len, (LPDWORD)&nbytes);
    FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);

    spi_decoder(serial+restart_point,&dat,1);

    printf("\nw25qxx ID is: %0x\n",dat);
    return dat;
}

void w25qxx_check_busy(FT_HANDLE handle)
{
    uint8_t serial[1024];
    uint32_t len=0,restart_point;
    DWORD nbytes;
    uint8_t dat = 0;

    len = spi_cs_start(serial);
    len += spi_write(serial+len,READ_STATUS_1);

    do
    {
        restart_point = len;
        len += spi_read(serial+len);

        FT_Write(handle, serial, len, (LPDWORD)&nbytes);
        FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);

        spi_decoder(serial+restart_point,&dat,1);
        printf("\n%d\n",dat);
    }while(dat & 0x01);

    len += spi_cs_end(serial);
}

void w25qxx_erase_sector(FT_HANDLE handle,uint32_t addr)
{
    uint8_t serial[1024];
    uint32_t len=0;
    DWORD nbytes;

    len = spi_cs_start(serial);
    len += spi_write(serial+len,WRITE_ENABLE);
    len += spi_cs_end(serial+len);

    len += spi_cs_start(serial+len);

    len += spi_write(serial+len,SECTOR_ERASE);
    len += spi_write(serial+len,(uint8_t)(addr>>16));
    len += spi_write(serial+len,(uint8_t)(addr>>8));
    len += spi_write(serial+len,(uint8_t)addr);

    len += spi_cs_end(serial+len);

    FT_Write(handle, serial, len, (LPDWORD)&nbytes);
    FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);

    Sleep(1000);
}

void w25qxx_erase_chip(FT_HANDLE handle,uint8_t dat)
{
    uint8_t serial[1024];
    uint32_t len=0;
    DWORD nbytes;

    len = spi_cs_start(serial);
    len += spi_write(serial+len,WRITE_ENABLE);

    len += spi_cs_start(serial+len);
    len += spi_write(serial+len,dat);
    len += spi_cs_end(serial+len);


    FT_Write(handle, serial, len, (LPDWORD)&nbytes); 
    FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);
 
    
    w25qxx_check_busy(handle);
    printf("\n Erase completing...!\n");
}

void w25qxx_read(FT_HANDLE handle,uint32_t addr)
{
    uint8_t serial[1024];
    uint32_t len=0,restart_point;
    DWORD nbytes;
    uint8_t dat;

    len += spi_cs_start(serial);
    len += spi_write(serial+len,READ_DATA); //读 cmd

    len += spi_write(serial+len,(uint8_t)(addr>>16));
    len += spi_write(serial+len,(uint8_t)(addr>>8));
    len += spi_write(serial+len,(uint8_t)addr);

    restart_point = len;

    len += spi_read(serial+len);
    len += spi_cs_end(serial+len);


    FT_Write(handle, serial, len, (LPDWORD)&nbytes);
    FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);

    spi_decoder(serial+restart_point,&dat,1);

    printf("\ndat: %0x\n",dat);
}

void w25qxx_write(FT_HANDLE handle,uint32_t addr,uint8_t wrdat)
{
    uint8_t serial[1024];
    uint32_t len=0;
    DWORD nbytes;

    len = spi_cs_start(serial);
    len += spi_write(serial+len,WRITE_ENABLE);

    len += spi_cs_start(serial+len);
    len += spi_write(serial+len,PAGE_PROGRAM);

    len += spi_write(serial+len,(uint8_t)(addr>>16));
    len += spi_write(serial+len,(uint8_t)(addr>>8));
    len += spi_write(serial+len,(uint8_t)addr);

    
    len += spi_write(serial+len,wrdat);

    len += spi_cs_end(serial+len);


    FT_Write(handle, serial, len, (LPDWORD)&nbytes);
    FT_Read(handle, serial, nbytes, (LPDWORD)&nbytes);
  
}


//128k  

FT_HANDLE OpenDevice(int argc,char* argv[])
{  
    FT_STATUS ftStatus;
    FT_HANDLE handle;
    DWORD numDevs;
    int sel_dev;

    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if(ftStatus != FT_OK)
    {
        printf("Can not create device info list.");
        exit(1);
    }

    if(numDevs == 0 )
    {
        printf("Not find any FTDI device.");
        exit(2);
    }

    if (argc != 1) {
        if (is_integer(argv[1])) {
           sscanf(argv[1], "%d", &sel_dev);
        }else{
            puts("Parameter is error.");
            exit(3) ;
        }
    }else{
        puts(help);
        exit(3);
    }

    if (FT_Open(sel_dev, &handle)) {
        puts("Can't open device");
        exit(1);
    }

    ftStatus = FT_SetBitMode(handle, 0x0F, FT_BITMODE_SYNC_BITBANG);
    if (ftStatus) {
     perror("SetBitMode failing...");
     exit(1);
    }

    ftStatus = FT_SetBaudRate(handle, 9600);
    if (ftStatus) {
     perror("SetBaudRate failing...");
    }

    FT_Purge(handle, FT_PURGE_RX);

    return handle;
}

int main(int argc,char* argv[])
{
    
    FT_HANDLE handle;
    uint8_t wrdat = 0x22;

    handle = OpenDevice(argc,argv);

    ReadDeviceID(handle);

    

    w25qxx_erase_chip(handle,0x60);


    FT_Close(handle);

    w25qxx_write(handle,0x0001,wrdat);

    w25qxx_read(handle,0x0000);

    getchar();
    w25qxx_read(handle,0x0001);
    w25qxx_read(handle,0x0002);
    

    return 0;
}