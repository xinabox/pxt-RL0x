#include "SC18IS602.h"

using namespace pxt;

uint8_t i2cwrite(uint16_t address, uint8_t reg, uint8_t *data, int len) {
    int i2c_error_status = 0;
#ifdef CODAL_I2C
    auto sda = LOOKUP_PIN(SDA);
    auto scl = LOOKUP_PIN(SCL);
    codal::I2C *i2c = pxt::getI2C(sda, scl);
#endif
    uint8_t val[len + 1];
    val[0] = reg;
    for (uint8_t i = 0; i < len; i++) {
        val[i + 1] = data[i];
    }
#ifdef CODAL_I2C
    return i2c_error_status = i2c->write((uint16_t)address, (uint8_t *)&val, len + 1, false);
#else
    return i2c_error_status = uBit.i2c.write(address, (const char *)&val, len + 1, false);
#endif
}

uint8_t i2cread(uint16_t address, uint8_t reg, uint8_t *data, int len) {
#ifdef CODAL_I2C
    auto sda = LOOKUP_PIN(SDA);
    auto scl = LOOKUP_PIN(SCL);
    codal::I2C *i2c = pxt::getI2C(sda, scl);
#endif
    int i2c_error_status = 0;

#ifdef CODAL_I2C
    i2c_error_status = i2c->write((uint16_t)address, (uint8_t *)&reg, 1, true);
#else
    i2c_error_status = uBit.i2c.write(address, (const char *)&reg, 1, true);
#endif

#ifdef CODAL_I2C
    if (len == 1)
        return i2c_error_status = i2c->read((uint16_t)address, (uint8_t *)&data, len, false);
    else
        return i2c_error_status = i2c->read((uint16_t)address, (uint8_t *)&data, len, false);
#else
    return i2c_error_status = uBit.i2c.read(address, (char *)data, len, false);
#endif
}

void _sleep(uint16_t time_ms) {
#ifdef CODAL_I2C
    sleep_ms(time_ms);
#else
    uBit.sleep(time_ms);
#endif
}

SC18IS602::SC18IS602(uint8_t addr) {
    i2cAddr = (addr >> 1) << 1;
}

void SC18IS602::ResetDevice(void) {
    reg_f0_config = 0x00;
    reg_f4_gpio_w = 0x00;
    reg_f5_gpio_r = 0x0F;
    reg_f6_gpio_e = 0x00;
    reg_f7_gpio_d = 0xAA;
    WriteRegister(0xf0, reg_f0_config);
    WriteRegister(0xf4, reg_f4_gpio_w);

    WriteRegister(0xf6, reg_f6_gpio_e);
    WriteRegister(0xf7, reg_f7_gpio_d);
    pinMode(0, INPUT);
    pinMode(1, INPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    GPIOEnable(0);
    GPIOEnable(1);
    GPIOEnable(2);
    GPIOEnable(3);
}

void SC18IS602::GPIOEnable(uint8_t pin) {
    reg_f6_gpio_e |= (0x01 << pin);
    WriteRegister(0xf6, reg_f6_gpio_e);
}

void SC18IS602::SSEnable(uint8_t pin) {
    reg_f6_gpio_e &= (uint8_t)(~(0x01 << pin));
    WriteRegister(0xf6, reg_f6_gpio_e);
}

void SC18IS602::begin(void) {
    ResetDevice();
    ss_pin = SC18IS602_SS_0;
    SSEnable(ss_pin);
}

uint8_t SC18IS602::transfer(uint8_t *buffer, uint8_t length) {
    WriteBytes(buffer, length);
    return (ReadBytes(buffer, length));
}

void SC18IS602::pinMode(uint8_t pin, uint8_t i_o) {
    reg_f7_gpio_d &= (((uint8_t) ~(0x03)) << (pin < 1));
    if (i_o == INPUT) {
        reg_f7_gpio_d |= (0x02 << (pin << 1));
    } else {
        reg_f7_gpio_d |= (0x01 << (pin << 1));
    }
    WriteRegister(0xF7, reg_f7_gpio_d);
}

uint8_t SC18IS602::digitalRead(uint8_t pin) {
    uint8_t buffer[1];
    i2cread(i2cAddr, (uint8_t)0xF5, buffer, 1);
    if ((buffer[0] & (1 << pin)) == (1 << pin))
        return 1;
    else
        return 0;
}

void SC18IS602::WriteBytes(uint8_t *buffer, uint8_t length) {
    uint8_t i;
    i2cwrite(i2cAddr, (uint8_t)(0x01 << ss_pin), buffer, length);
    _sleep(10);
    return;
}
uint8_t SC18IS602::ReadBytes(uint8_t *buffer, uint8_t length) {
    uint8_t i;
    uint8_t count;
    int i2c_error_status = 0;
#ifdef CODAL_I2C
    auto sda = LOOKUP_PIN(SDA);
    auto scl = LOOKUP_PIN(SCL);
    codal::I2C *i2c = pxt::getI2C(sda, scl);
#endif
#ifdef CODAL_I2C
    if (length == 1)
        return i2c_error_status = i2c->read((uint16_t)i2cAddr, (uint8_t *)buffer, length, false);
    else
        return i2c_error_status = i2c->read((uint16_t)i2cAddr, (uint8_t *)buffer, length, false);
#else
    return i2c_error_status = uBit.i2c.read(i2cAddr, (char *)buffer, length, false);
#endif
    return count;
}

void SC18IS602::WriteRegister(uint8_t reg_addr, uint8_t val) {
    i2cwrite(i2cAddr, reg_addr, (uint8_t *)val, 1);
    _sleep(10);
    return;
}

void SC18IS602::setClockDivider(uint8_t divider) {
    reg_f0_config &= 0xFC;

    reg_f0_config |= divider;

    WriteRegister(0xF0, reg_f0_config);

    return;
}
