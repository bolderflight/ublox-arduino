/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2022 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef SRC_UBX_MGA_H_
#define SRC_UBX_MGA_H_

#include <cstdint>
#include <cstddef>
#include "ubx_defs.h"  // NOLINT

namespace bfs {

/* UBX-MGA IDs */
static constexpr uint8_t UBX_MGA_ACK_ID_ = 0x60;
static constexpr uint8_t UBX_MGA_BDS_ID_ = 0x03;
static constexpr uint8_t UBX_MGA_DBD_ID_ = 0x80;
static constexpr uint8_t UBX_MGA_GAL_ID_ = 0x02;
static constexpr uint8_t UBX_MGA_GLO_ID_ = 0x06;
static constexpr uint8_t UBX_MGA_GPS_ID_ = 0x00;
static constexpr uint8_t UBX_MGA_INI_ID_ = 0x40;
static constexpr uint8_t UBX_MGA_QZSS_ID_ = 0x05;

}  // namespace bfs

#endif  // SRC_UBX_MGA_H_