#pragma once
#ifdef CODAL_I2C
#include "I2C.h"
#include "Pin.h"
#endif
#include "inttypes.h"
#include "pxt.h"

#define     SC18IS602_ADDRESS_000     (0x50)
#define     SC18IS602_ADDRESS_001     (0x52)
#define     SC18IS602_ADDRESS_010     (0x54)
#define     SC18IS602_ADDRESS_011     (0x56)
#define     SC18IS602_ADDRESS_100     (0x58)
#define     SC18IS602_ADDRESS_101     (0x5A)
#define     SC18IS602_ADDRESS_110     (0x5C)
#define     SC18IS602_ADDRESS_111     (0x5E)

#define     SC18IS602_CLOCK_1843K     (0x00)
#define     SC18IS602_CLOCK_461K      (0x01)
#define     SC18IS602_CLOCK_115K      (0x02)
#define     SC18IS602_CLOCK_58K       (0x03)

#define     SC18IS602_SPI_MODE0       (0x00)
#define     SC18IS602_SPI_MODE1       (0x01)
#define     SC18IS602_SPI_MODE2       (0x02)
#define     SC18IS602_SPI_MODE3       (0x03)

#define     SC18IS602_SS_0   		 (0x00)
#define     SC18IS602_SS_1  		 (0x01)
#define     SC18IS602_SS_2   		 (0x02)
#define     SC18IS602_SS_3   		 (0x03)

#define LSBFIRST 0
#define MSDFIRST 1
#define INPUT 0
#define OUTPUT 1

class SC18IS602 
{ 
    public:
        SC18IS602(uint8_t addr = SC18IS602_ADDRESS_111);
        void begin();
		void setClockDivider(uint8_t divider);
		uint8_t transfer(uint8_t* buffer, uint8_t length);
        void pinMode(uint8_t pin, uint8_t io);
        uint8_t digitalRead(uint8_t pin);
    
    private:
        uint8_t i2cAddr;
		uint8_t ss_pin;
		uint8_t reg_f0_config;
		uint8_t reg_f4_gpio_w;
		uint8_t reg_f5_gpio_r;
		uint8_t reg_f6_gpio_e;
		uint8_t reg_f7_gpio_d;
	    void ResetDevice(void);
		void GPIOEnable(uint8_t pin);
		void SSEnable(uint8_t pin);
		uint8_t ReadBytes(uint8_t* buffer, uint8_t length);
		void WriteBytes(uint8_t* buffer, uint8_t length);
        void WriteRegister(uint8_t reg_addr, uint8_t val);	
};





    
    
