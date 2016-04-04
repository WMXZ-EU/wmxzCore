/*
 * WMXZ Teensy simple data logger
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//iic.h

#ifndef IIC_H
#define IIC_H

typedef	void (*I2C_FUNC)(void *,int) ;

#ifdef __cplusplus
extern "C"{
#endif

void i2c_init(void);

void IIC_setup(int alt);
int IIC_write(const uint8_t *data, int ndat, uint8_t address, int sendStop);
int IIC_read(uint8_t *data, int length, uint8_t address, int sendStop);

uint8_t i2cWriteRegister(uint8_t address, uint8_t reg, uint8_t value);
uint8_t i2cReadRegister(uint8_t address, uint8_t reg);

uint8_t i2cWriteRegister8(uint8_t address, uint8_t reg, uint8_t value);
uint8_t i2cReadRegister8(uint8_t address, uint8_t reg, uint8_t *value);
uint8_t i2cReadRegister8s(uint8_t address, uint8_t reg, uint8_t *value); // with stop after writing reg to address
uint8_t i2cWriteRegister16(uint8_t address, uint16_t reg, uint16_t value);
uint8_t i2cReadRegister16(uint8_t address, uint16_t reg, uint16_t *value);

uint8_t writeRegister(uint8_t address, uint8_t reg, uint8_t value);
uint8_t readRegister(uint8_t address, uint8_t reg);
void printRegister(uint8_t address, uint8_t reg);

void i2c_resetBus(int alt);
void i2c_send(uint8_t address, uint8_t *data, int ndat, int sendStop, I2C_FUNC funct);
void i2c_receive(uint8_t address, uint8_t *data, int ndat, int sendStop, I2C_FUNC funct);

uint8_t i2c_error(void) ;
uint8_t i2c_done(void) ;
uint8_t i2c_txCount(void);
uint8_t i2c_rxCount(void);

#ifdef __cplusplus
}
#endif

#endif