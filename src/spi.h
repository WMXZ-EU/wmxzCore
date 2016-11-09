/*
 * WMXZ Teensy core library
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
//spi.h

#ifndef SPI_H
#define SPI_H

typedef enum IRQ_NUMBER_t IRQ_NUMBER;

#ifdef __cplusplus
extern "C" {
#endif

//============================ SPI ===================================
void spi_init(void);
void spi_configPorts(int iconf);
uint32_t spi_setup(int16_t port, int sp);

uint8_t spi_receive(int16_t port) ;
void spi_send(int16_t port, uint8_t b) ;
uint8_t spi_receiveBulk(int16_t port, uint8_t* buf, size_t n) ;
void spi_sendBulk(int16_t port, const uint8_t* buf , size_t n) ;

uint8_t spi_transfer(int16_t port, uint8_t data) ;
uint16_t spi_transfer16(int16_t port, uint16_t data) ;
void spi_transferBlock(int16_t port, void *inpbuf, void *outbuf, size_t count) ;
void spi_transferBlock16(int16_t port, void *inpbuf, void *outbuf, size_t count) ;

int spi_isAsserted();
void spi_assertCS(int16_t port, int cs, uint32_t ctar) ;
void spi_releaseCS(void) ;

void spi_read16(int16_t port, uint8_t *buf, size_t len) ;
void spi_write16(int16_t port, uint8_t *buf, size_t len) ;

//============================ DMA ===================================
int spi_dmaSetup(int16_t port, int dma_tx, int dma_rx, int prio_rx) ;
int spi_dma_done(void) ;
void spi_dma_setCB(Fxn_t f, Ptr s, Ptr d, int nice) ;

void spi_readDMA_noWait(int16_t port, int8_t *buf,  size_t len) ;
void spi_writeDMA_noWait(int16_t port, int8_t *buf,  size_t len) ;

//============================ NVIC ===================================
void spi_usingPinInterrupt(uint8_t n) ;
void spi_usingInterrupt(IRQ_NUMBER n);
void spi_notUsingInterrupt(IRQ_NUMBER n);
void spi_reserve(int16_t port, uint32_t ctar); // from  beginTransactions
void spi_release(void); // from endTransactions

#ifdef __cplusplus
}
#endif

#endif
