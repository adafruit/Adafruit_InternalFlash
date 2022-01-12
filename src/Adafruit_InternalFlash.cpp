/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ha Thach (tinyusb.org) for Adafruit Industries
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

#include "Adafruit_InternalFlash.h"

#define BLOCK_SZ  512
#define FAKE_MBR_PART1_START_BLOCK 1

Adafruit_InternalFlash::Adafruit_InternalFlash(uint32_t addr, uint32_t size) :
  _start_addr(addr), _size(size), _flash((const void *) addr, size)
{
  // default it fake MBR to match CircuitPython
  _fake_mbr = true;
}

bool Adafruit_InternalFlash::begin(void)
{
  return true;
}

void Adafruit_InternalFlash::fakeMBR(bool fake) {
  _fake_mbr = fake;
}

uint32_t Adafruit_InternalFlash::blockCount(void) {
  return _size/BLOCK_SZ + (_fake_mbr ? 1 : 0);
}

uint32_t Adafruit_InternalFlash::blockSize(void) {
  return BLOCK_SZ;
}

uint32_t Adafruit_InternalFlash::block2addr(uint32_t block)
{
  // adjust block address if we are faking MBR
  // flash contents are data only without mbr
  if (_fake_mbr) block--;

  return _start_addr + block*BLOCK_SZ;
}

//--------------------------------------------------------------------+
// SdFat BaseBlockDRiver API
// A block is 512 bytes
//--------------------------------------------------------------------+

static void build_partition(uint8_t *buf, int boot, int type, uint32_t start_block, uint32_t num_blocks);
static void build_fake_mbr(uint8_t* dest, uint32_t num_blocks);

bool Adafruit_InternalFlash::readBlock(uint32_t block, uint8_t *dst) {
  return readBlocks(block, dst, 1);
}

bool Adafruit_InternalFlash::writeBlock(uint32_t block, const uint8_t *src) {
  return writeBlocks(block, src, 1);
}

bool Adafruit_InternalFlash::syncBlocks(){
  // nothing to do
  return true;
}

bool Adafruit_InternalFlash::readBlocks(uint32_t block, uint8_t *dst, size_t nb) {
  if (_fake_mbr && block == 0) {
    build_fake_mbr(dst, _size/BLOCK_SZ);

    // adjust parameters for non-mbr
    block++;
    dst += BLOCK_SZ;
    nb--;

    // nothing more to read
    if (nb == 0) return true;
  }

  uint32_t const addr = block2addr(block);
  memcpy(dst, (void const*) addr, nb*BLOCK_SZ);

  return true;
}

bool Adafruit_InternalFlash::writeBlocks(uint32_t block, const uint8_t *src, size_t nb) {
  return false;
}

// fake the mbr
static void build_fake_mbr(uint8_t* dest, uint32_t num_blocks)
{
  for (int i = 0; i < 446; i++) {
    dest[i] = 0;
  }

  build_partition(dest + 446, 0, 0x01 /* FAT12 */, FAKE_MBR_PART1_START_BLOCK, num_blocks);
  build_partition(dest + 462, 0, 0, 0, 0);
  build_partition(dest + 478, 0, 0, 0, 0);
  build_partition(dest + 494, 0, 0, 0, 0);

  dest[510] = 0x55;
  dest[511] = 0xaa;
}

// Helper to build a partition in a fake MBR
static void build_partition(uint8_t *buf, int boot, int type, uint32_t start_block, uint32_t num_blocks) {
  buf[0] = boot;

  if (num_blocks == 0) {
      buf[1] = 0;
      buf[2] = 0;
      buf[3] = 0;
  } else {
      buf[1] = 0xff;
      buf[2] = 0xff;
      buf[3] = 0xff;
  }

  buf[4] = type;

  if (num_blocks == 0) {
      buf[5] = 0;
      buf[6] = 0;
      buf[7] = 0;
  } else {
      buf[5] = 0xff;
      buf[6] = 0xff;
      buf[7] = 0xff;
  }

  buf[8] = start_block;
  buf[9] = start_block >> 8;
  buf[10] = start_block >> 16;
  buf[11] = start_block >> 24;

  buf[12] = num_blocks;
  buf[13] = num_blocks >> 8;
  buf[14] = num_blocks >> 16;
  buf[15] = num_blocks >> 24;
}
