#include "xRL0x.h"

SC18IS602 i2cspi = SC18IS602(SC18IS602_ADDRESS_000);

xRL0X::xRL0X(void) {}

static const xRL0X::ModemConfig MODEM_CONFIG_TABLE[] = {
    {0x72, 0x74, 0x00},
    {0x92, 0x74, 0x00},
    {0x48, 0x94, 0x00},
    {0x78, 0xc4, 0x00},
};

bool xRL0X::begin(void) {
    uint8_t buf[2] = {0x42, 0x00};

    i2cspi.transfer(buf, 2);

    setupSC18IS602();
    _mode = RHModeInitialising;
    _thisAddress = RH_BROADCAST_ADDRESS;
    _txHeaderTo = RH_BROADCAST_ADDRESS;
    _txHeaderFrom = RH_BROADCAST_ADDRESS;
    _txHeaderId = 0;
    _txHeaderFlags = 0;
    _rxBad = 0;
    _rxGood = 0;
    _txGood = 0;
    _cad_timeout = 0;
    _ISRedgeFlag = 0;

    write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);

    write(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    write(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    setModeIdle();

    setModemConfig(Bw125Cr45Sf128);

    setPreambleLength(8);

    setFrequency(434.0);

    setTxPower(13);

    return buf[1];
}

void xRL0X::setupSC18IS602(void) {

    i2cspi.begin();
    i2cspi.setClockDivider(0);

    i2cspi.pinMode(1, INPUT);
    i2cspi.pinMode(2, INPUT);
}

uint8_t xRL0X::read(uint8_t reg) {
    uint8_t sendArray[5];
    sendArray[0] = (reg & ~SPI_WRITE_MASK);
    sendArray[1] = 0x00;

    i2cspi.transfer(sendArray, 2);

    return sendArray[1];
}

void xRL0X::burstRead(uint8_t reg, uint8_t *dest, uint8_t len) {
    uint8_t sendArray[200];
    uint8_t ptr = 1;

    sendArray[0] = (reg & ~SPI_WRITE_MASK);
    for (int i = 1; i < 200; i++) {
        sendArray[i] = 0x00;
    }

    i2cspi.transfer(sendArray, len + 1);

    while (len--)
        *dest++ = sendArray[ptr++];
}

void xRL0X::write(uint8_t reg, uint8_t value) {
    uint8_t sendArray[5];

    sendArray[0] = (reg | SPI_WRITE_MASK);
    sendArray[1] = value;

    i2cspi.transfer(sendArray, 2);
}

void xRL0X::burstWrite(uint8_t reg, const uint8_t *src, uint8_t len) {
    uint8_t status = 0;
    uint8_t sendArray[200];
    uint8_t ptr = 0;
    uint8_t numbytes = len + 1;

    sendArray[ptr] = reg | SPI_WRITE_MASK;

    while (len--) {
        ptr++;
        sendArray[ptr] = *src++;
    }

    i2cspi.transfer(sendArray, numbytes);
}

bool xRL0X::send(const uint8_t *data, uint8_t len) {
    if (len > RH_RF95_MAX_MESSAGE_LEN)
        return false;

    waitPacketSent(radioSendTimeout);
    setModeIdle();

    write(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);

    write(RH_RF95_REG_00_FIFO, _txHeaderTo);
    write(RH_RF95_REG_00_FIFO, _txHeaderFrom);
    write(RH_RF95_REG_00_FIFO, _txHeaderId);
    write(RH_RF95_REG_00_FIFO, _txHeaderFlags);

    burstWrite(RH_RF95_REG_00_FIFO, data, len);
    write(RH_RF95_REG_22_PAYLOAD_LENGTH, len + RH_RF95_HEADER_LEN);

    setModeTx();
    waitPacketSent(radioSendTimeout);
    return true;
}

bool xRL0X::recv(uint8_t *buf, uint8_t *len) {
    if (!available()) {
        return false;
    }

    if (buf && len) {

        if (*len > _bufLen - RH_RF95_HEADER_LEN)
            *len = _bufLen - RH_RF95_HEADER_LEN;
        memcpy(buf, _buf + RH_RF95_HEADER_LEN, *len);
    }
    clearRxBuf();
    return true;
}

void xRL0X::poll() {
    if ((i2cspi.digitalRead(1) == 1)) {
        uint8_t irq_flags = read(RH_RF95_REG_12_IRQ_FLAGS);
        if (_mode == RHModeRx && irq_flags & (RH_RF95_RX_TIMEOUT | RH_RF95_PAYLOAD_CRC_ERROR)) {

            _rxBad++;
        } else if (_mode == RHModeRx && irq_flags & RH_RF95_RX_DONE) {

            uint8_t len = read(RH_RF95_REG_13_RX_NB_BYTES);

            write(RH_RF95_REG_0D_FIFO_ADDR_PTR, read(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR));
            burstRead(RH_RF95_REG_00_FIFO, _buf, len);
            _bufLen = len;
            write(RH_RF95_REG_12_IRQ_FLAGS, 0xff);

            _lastSNR = (int8_t)read(RH_RF95_REG_19_PKT_SNR_VALUE) / 4;

            _lastRssi = read(RH_RF95_REG_1A_PKT_RSSI_VALUE);

            if (_lastSNR < 0)
                _lastRssi = _lastRssi + _lastSNR;
            else
                _lastRssi = (int)_lastRssi * 16 / 15;
            if (_usingHFport)
                _lastRssi -= 157;
            else
                _lastRssi -= 164;

            validateRxBuf();
            if (_rxBufValid)
                setModeIdle();
        } else if (_mode == RHModeTx && irq_flags & RH_RF95_TX_DONE) {

            _txGood++;
            setModeIdle();
        } else if (_mode == RHModeCad && irq_flags & RH_RF95_CAD_DONE) {

            _cad = irq_flags & RH_RF95_CAD_DETECTED;
            setModeIdle();
        }

        write(RH_RF95_REG_12_IRQ_FLAGS, 0xff);
        write(RH_RF95_REG_12_IRQ_FLAGS, 0xff);
    }
}

void xRL0X::validateRxBuf() {
    if (_bufLen < 4)
        return;

    _rxHeaderTo = _buf[0];
    _rxHeaderFrom = _buf[1];
    _rxHeaderId = _buf[2];
    _rxHeaderFlags = _buf[3];
    if (_promiscuous || _rxHeaderTo == _thisAddress || _rxHeaderTo == RH_BROADCAST_ADDRESS) {
        _rxGood++;
        _rxBufValid = true;
    }
}

bool xRL0X::available() {
    poll();
    if (_mode == RHModeTx) {
        return false;
    }
    setModeRx();
    return _rxBufValid;
}

void xRL0X::clearRxBuf() {
    _rxBufValid = false;
    _bufLen = 0;
}

void xRL0X::setModeTx() {
    if (_mode != RHModeTx) {

        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_TX);
        write(RH_RF95_REG_40_DIO_MAPPING1, 0x40);
        _mode = RHModeTx;
    }
}

void xRL0X::setModeRx() {
    if (_mode != RHModeRx) {

        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_RXCONTINUOUS);
        write(RH_RF95_REG_40_DIO_MAPPING1, 0x00);
        _mode = RHModeRx;
    }
}

void xRL0X::setModeIdle() {
    if (_mode != RHModeIdle) {

        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);
        _mode = RHModeIdle;
    }
}

bool xRL0X::sleep() {
    if (_mode != RHModeSleep) {
        write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP);
        _mode = RHModeSleep;
    }
    return true;
}

bool xRL0X::setModemConfig(ModemConfigChoice index) {
    if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    ModemConfig cfg;
    memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(xRL0X::ModemConfig));
    setModemRegisters(&cfg);

    return true;
}

void xRL0X::setModemRegisters(const ModemConfig *config) {
    write(RH_RF95_REG_1D_MODEM_CONFIG1, config->reg_1d);
    write(RH_RF95_REG_1E_MODEM_CONFIG2, config->reg_1e);
    write(RH_RF95_REG_26_MODEM_CONFIG3, config->reg_26);
}

void xRL0X::setPreambleLength(uint16_t bytes) {
    write(RH_RF95_REG_20_PREAMBLE_MSB, bytes >> 8);
    write(RH_RF95_REG_21_PREAMBLE_LSB, bytes & 0xff);
}

bool xRL0X::setFrequency(float centre) {

    uint32_t frf = (centre * 1000000.0) / RH_RF95_FSTEP;
    write(RH_RF95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
    write(RH_RF95_REG_07_FRF_MID, (frf >> 8) & 0xff);
    write(RH_RF95_REG_08_FRF_LSB, frf & 0xff);
    _usingHFport = (centre >= 779.0);

    return true;
}

void xRL0X::setTxPower(int8_t power, bool useRFO) {

    if (useRFO) {
        if (power > 14)
            power = 14;
        if (power < -1)
            power = -1;
        write(RH_RF95_REG_09_PA_CONFIG, RH_RF95_MAX_POWER | (power + 1));
    } else {
        if (power > 23)
            power = 23;
        if (power < 5)
            power = 5;

        if (power > 20) {
            write(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_ENABLE);
            power -= 3;
        } else {
            write(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_DISABLE);
        }

        write(RH_RF95_REG_09_PA_CONFIG, RH_RF95_PA_SELECT | (power - 5));
    }
}

bool xRL0X::waitPacketSent(uint16_t timeout) {
    unsigned long starttime = system_timer_current_time();
    while ((system_timer_current_time() - starttime) < timeout) {
        poll();
        if (_mode != RHModeTx)
            return true;
        schedule();
        // yield();
    }
    return false;
}

bool xRL0X::waitAvailableTimeout(uint16_t timeout) {
    unsigned long starttime = system_timer_current_time();
    while ((system_timer_current_time() - starttime) < timeout) {
        if (available()) {
            return true;
        }
        schedule();
        // yield();
    }
    return false;
}

void xRL0X::setThisAddress(uint8_t address) {
    _thisAddress = address;
}

void xRL0X::setHeaderTo(uint8_t to) {
    _txHeaderTo = to;
}

void xRL0X::setHeaderFrom(uint8_t from) {
    _txHeaderFrom = from;
}

uint8_t xRL0X::headerTo() {
    return _rxHeaderTo;
}

uint8_t xRL0X::headerFrom() {
    return _rxHeaderFrom;
}

uint8_t xRL0X::headerId() {
    return _rxHeaderId;
}

uint8_t xRL0X::headerFlags() {
    return _rxHeaderFlags;
}

int16_t xRL0X::lastRssi() {
    return _lastRssi;
}

namespace rl0x {
static xRL0X *ptr = new xRL0X;

//%
void begin() {
    ptr->begin();
}

//%
void setFrequency(float freq) {
    ptr->setFrequency(freq);
}

//%
void setTxPower(uint8_t val) {
    ptr->setTxPower(23, false);
}

//%
String recv() {
    uint8_t buf[195] = {NULL};
    uint8_t len = sizeof(buf);
    if (ptr->waitAvailableTimeout(10000)) {
        if (ptr->recv(buf, &len)) {
            return mkString((const char *)buf, strlen((const char *)buf));
        }
    }
    return mkString("");
}

//%
uint16_t lastRssi() {
    return ptr->lastRssi();
}

//%
void configId(uint16_t freq, uint8_t id) {
    ptr->setFrequency(freq);
    ptr->setThisAddress(id);
    ptr->setHeaderFrom(id);
    ptr->setHeaderTo(id);
}

//%
void sendString(String s) {
    ptr->send((const uint8_t *)s->getUTF8Data(), s->getUTF8Size());
}

//%
void sendNumber(String s) {
    ptr->send((const uint8_t *)s->getUTF8Data(), s->getUTF8Size());
}
} // namespace rl0x
