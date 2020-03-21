
#ifndef xRL0x_h
#define xRL0x_h

#include "SC18IS602.h"

#ifdef CODAL_I2C
#define radioSendTimeout 100
#else
#define radioSendTimeout 3000
#endif

#define SC18IS603_FIFO_SIZE 195
#define RH_RF95_MAX_PAYLOAD_LEN SC18IS603_FIFO_SIZE
#define RH_RF95_HEADER_LEN 4
#ifndef RH_RF95_MAX_MESSAGE_LEN
#define RH_RF95_MAX_MESSAGE_LEN (RH_RF95_MAX_PAYLOAD_LEN - RH_RF95_HEADER_LEN)
#endif
#define RH_RF95_FXOSC 32000000.0
#define RH_RF95_FSTEP (RH_RF95_FXOSC / 524288)
#define RH_BROADCAST_ADDRESS 0xff
#define SPI_WRITE_MASK 0x80
#define RH_RF95_REG_00_FIFO 0x00
#define RH_RF95_REG_01_OP_MODE 0x01
#define RH_RF95_REG_06_FRF_MSB 0x06
#define RH_RF95_REG_07_FRF_MID 0x07
#define RH_RF95_REG_08_FRF_LSB 0x08
#define RH_RF95_REG_09_PA_CONFIG 0x09
#define RH_RF95_REG_0D_FIFO_ADDR_PTR 0x0d
#define RH_RF95_REG_0E_FIFO_TX_BASE_ADDR 0x0e
#define RH_RF95_REG_0F_FIFO_RX_BASE_ADDR 0x0f
#define RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR 0x10
#define RH_RF95_REG_12_IRQ_FLAGS 0x12
#define RH_RF95_REG_13_RX_NB_BYTES 0x13
#define RH_RF95_REG_19_PKT_SNR_VALUE 0x19
#define RH_RF95_REG_1A_PKT_RSSI_VALUE 0x1a
#define RH_RF95_REG_1D_MODEM_CONFIG1 0x1d
#define RH_RF95_REG_1E_MODEM_CONFIG2 0x1e
#define RH_RF95_REG_20_PREAMBLE_MSB 0x20
#define RH_RF95_REG_21_PREAMBLE_LSB 0x21
#define RH_RF95_REG_22_PAYLOAD_LENGTH 0x22
#define RH_RF95_REG_26_MODEM_CONFIG3 0x26
#define RH_RF95_REG_40_DIO_MAPPING1 0x40
#define RH_RF95_REG_4D_PA_DAC 0x4d
#define RH_RF95_LONG_RANGE_MODE 0x80
#define RH_RF95_MODE_SLEEP 0x00
#define RH_RF95_MODE_STDBY 0x01
#define RH_RF95_MODE_TX 0x03
#define RH_RF95_MODE_RXCONTINUOUS 0x05
#define RH_RF95_PA_SELECT 0x80
#define RH_RF95_MAX_POWER 0x70
#define RH_RF95_RX_TIMEOUT 0x80
#define RH_RF95_RX_DONE 0x40
#define RH_RF95_PAYLOAD_CRC_ERROR 0x20
#define RH_RF95_TX_DONE 0x08
#define RH_RF95_CAD_DONE 0x04
#define RH_RF95_CAD_DETECTED 0x01
#define RH_RF95_PA_DAC_DISABLE 0x04
#define RH_RF95_PA_DAC_ENABLE 0x07

class xRL0X {
  public:
    typedef struct {
        uint8_t reg_1d;
        uint8_t reg_1e;
        uint8_t reg_26;
    } ModemConfig;

    typedef enum {
        Bw125Cr45Sf128 = 0,
        Bw500Cr45Sf128,
        Bw31_25Cr48Sf512,
        Bw125Cr48Sf4096,
    } ModemConfigChoice;
    xRL0X();
    bool begin(void);
    uint8_t read(uint8_t reg);
    void write(uint8_t reg, uint8_t value);
    void setModeTx();
    void setModeRx();
    void setModeIdle();
    bool sleep();
    bool setModemConfig(ModemConfigChoice index);
    bool setFrequency(float centre);
    void setPreambleLength(uint16_t bytes);
    void setTxPower(int8_t power, bool useRFO = false);
    void burstWrite(uint8_t reg, const uint8_t *src, uint8_t len);
    void burstRead(uint8_t reg, uint8_t *dest, uint8_t len);
    bool send(const uint8_t *data, uint8_t len);
    bool recv(uint8_t *buf, uint8_t *len);
    void poll();
    bool available();
    bool waitPacketSent(uint16_t timeout);
    bool waitAvailableTimeout(uint16_t timeout);
    void setHeaders(uint8_t address, uint8_t from, uint8_t to);
    void setPromiscuous(bool promiscuous);
    void setThisAddress(uint8_t address);
    void setHeaderTo(uint8_t to);
    void setHeaderFrom(uint8_t from);
    void setHeaderId(uint8_t id);
    uint8_t headerTo();
    uint8_t headerFrom();
    uint8_t headerId();
    uint8_t headerFlags();
    int16_t lastRssi();

  private:
    void setupSC18IS602();
    typedef enum {
        RHModeInitialising = 0,
        RHModeSleep,
        RHModeIdle,
        RHModeTx,
        RHModeRx,
        RHModeCad
    } RHMode;
    void setModemRegisters(const ModemConfig *config);
    void validateRxBuf();
    void clearRxBuf();

  protected:
    volatile RHMode _mode;
    uint8_t _thisAddress;
    bool _promiscuous;
    volatile uint8_t _rxHeaderTo;
    volatile uint8_t _rxHeaderFrom;
    volatile uint8_t _rxHeaderId;
    volatile uint8_t _rxHeaderFlags;
    uint8_t _txHeaderTo;
    uint8_t _txHeaderFrom;
    uint8_t _txHeaderId;
    uint8_t _txHeaderFlags;
    volatile int16_t _lastRssi;
    volatile uint16_t _rxBad;
    volatile uint16_t _rxGood;
    volatile uint16_t _txGood;
    volatile bool _cad;
    unsigned int _cad_timeout;
    bool _usingHFport;
    int8_t _lastSNR;
    bool _ISRedgeFlag;
    volatile uint8_t _bufLen;
    uint8_t _buf[RH_RF95_MAX_PAYLOAD_LEN];
    volatile bool _rxBufValid;
};

extern xRL0X RL0X;

#endif
