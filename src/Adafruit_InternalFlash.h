/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ha Thach for Adafruit Industries LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ADAFRUIT_INTERNALFLASH_H_
#define ADAFRUIT_INTERNALFLASH_H_

// implement SdFat Block Driver
#include "SdFat.h"
#include "SdFatConfig.h"

// FlashStorage for flashing for SAMD MCUs
#ifdef ARDUINO_ARCH_SAMD
#include "FlashStorage.h"
#endif

#if ENABLE_EXTENDED_TRANSFER_CLASS == 0
#error ENABLE_EXTENDED_TRANSFER_CLASS must be set to 1 in SdFat SdFatConfig.h
#endif

#if FAT12_SUPPORT == 0
#error FAT12_SUPPORT must be set to 1 in SdFat SdFatConfig.h
#endif

// This class adds support for the BaseBlockDriver interface.
// This allows it to be used with SdFat's FatFileSystem class.
class Adafruit_InternalFlash : public BaseBlockDriver {
public:
  Adafruit_InternalFlash(uint32_t start_addr, uint32_t size);
  ~Adafruit_InternalFlash() {}

  bool begin(void);
  bool end(void);

  uint32_t size(void);

  //------------- SdFat BaseBlockDRiver API -------------//
  virtual bool readBlock(uint32_t block, uint8_t *dst);
  virtual bool writeBlock(uint32_t block, const uint8_t *src);
  virtual bool syncBlocks();
  virtual bool readBlocks(uint32_t block, uint8_t *dst, size_t nb);
  virtual bool writeBlocks(uint32_t block, const uint8_t *src, size_t nb);

private:
  uint32_t block2addr(uint32_t block);

  uint32_t _start_addr;
  uint32_t _size;
  FlashClass _flash;
};

#endif /* ADAFRUIT_INTERNALFLASH_H_ */
