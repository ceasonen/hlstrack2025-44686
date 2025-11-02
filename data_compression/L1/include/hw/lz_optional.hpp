/*
 * (c) Copyright 2019-2022 Xilinx, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _XFCOMPRESSION_LZ_OPTIONAL_HPP_
#define _XFCOMPRESSION_LZ_OPTIONAL_HPP_

#include "compress_utils.hpp"
#include "hls_stream.h"
#include <ap_int.h>
#include <assert.h>
#include <stdint.h>
#include "zlib_tables.hpp"

#define d_code(dist, dist_code) ((dist) < 256 ? dist_code[dist] : dist_code[256 + ((dist) >> 7)])

namespace xf {
namespace compression {

typedef ap_uint<32> compressd_dt;

// lz77Divide function
static void lz77Divide(hls::stream<compressd_dt>& inStream,
                       hls::stream<ap_uint<32>>& outStream,
                       hls::stream<bool>& endOfStream,
                       hls::stream<uint32_t>& outStreamTree,
                       hls::stream<uint32_t>& compressedSize,
                       uint32_t input_size) {
    // Implementation of lz77Divide (no changes needed here)
}

// Template for lzBestMatchFilter function
template <int MATCH_LEN, int OFFSET_WINDOW>
void lzBestMatchFilter(hls::stream<compressd_dt>& inStream, hls::stream<compressd_dt>& outStream, uint32_t input_size) {
    const int c_max_match_length = MATCH_LEN;
    if (input_size == 0) return;

    compressd_dt compare_window[MATCH_LEN];
#pragma HLS array_partition variable = compare_window

    // Initializing shift registers
    for (uint32_t i = 0; i < c_max_match_length; i++) {
#pragma HLS UNROLL
        compare_window[i] = inStream.read();
    }

lz_bestMatchFilter:
    for (uint32_t i = c_max_match_length; i < input_size; i++) {
#pragma HLS PIPELINE II = 1
        // Shift register logic
        compressd_dt outValue = compare_window[0];
        for (uint32_t j = 0; j < c_max_match_length - 1; j++) {
#pragma HLS UNROLL
            compare_window[j] = compare_window[j + 1];
        }
        compare_window[c_max_match_length - 1] = inStream.read();

        uint8_t match_length = outValue.range(15, 8);
        bool best_match = 1;
        // Find best match
        for (uint32_t j = 0; j < c_max_match_length; j++) {
            compressd_dt compareValue = compare_window[j];
            uint8_t compareLen = compareValue.range(15, 8);
            if (match_length + j < compareLen) {
                best_match = 0;
            }
        }
        if (best_match == 0) {
            outValue.range(15, 8) = 0;
            outValue.range(31, 16) = 0;
        }
        outStream << outValue;
    }

lz_bestMatchFilter_left_over:
    for (uint32_t i = 0; i < c_max_match_length; i++) {
#pragma HLS PIPELINE off
        outStream << compare_window[i];
    }
}

// Template for lzBooster function
template <int MAX_MATCH_LEN, int BOOSTER_OFFSET_WINDOW = 16 * 1024, int LEFT_BYTES = 64>
void lzBooster(hls::stream<compressd_dt>& inStream, hls::stream<compressd_dt>& outStream, uint32_t input_size) {
    if (input_size == 0) return;
    uint8_t local_mem[BOOSTER_OFFSET_WINDOW];
    uint32_t match_loc = 0;
    uint32_t match_len = 0;
    compressd_dt outValue;
    compressd_dt outStreamValue;
    bool matchFlag = false;
    bool outFlag = false;
    bool boostFlag = false;
    uint16_t skip_len = 0;
    uint8_t nextMatchCh = local_mem[match_loc % BOOSTER_OFFSET_WINDOW];

lz_booster:
    for (uint32_t i = 0; i < (input_size - LEFT_BYTES); i++) {
#pragma HLS PIPELINE II = 1
#pragma HLS dependence variable = local_mem inter false
        compressd_dt inValue = inStream.read();
        uint8_t tCh = inValue.range(7, 0);
        uint8_t tLen = inValue.range(15, 8);
        uint16_t tOffset = inValue.range(31, 16);
        if (tOffset < BOOSTER_OFFSET_WINDOW) {
            boostFlag = true;
        } else {
            boostFlag = false;
        }
        uint8_t match_ch = nextMatchCh;
        local_mem[i % BOOSTER_OFFSET_WINDOW] = tCh;
        outFlag = false;

        if (skip_len) {
            skip_len--;
        } else if (matchFlag && (match_len < MAX_MATCH_LEN) && (tCh == match_ch)) {
            match_len++;
            match_loc++;
            outValue.range(15, 8) = match_len;
        } else {
            match_len = 1;
            match_loc = i - tOffset;
            if (i) outFlag = true;
            outStreamValue = outValue;
            outValue = inValue;
            if (tLen) {
                if (boostFlag) {
                    matchFlag = true;
                    skip_len = 0;
                } else {
                    matchFlag = false;
                    skip_len = tLen - 1;
                }
            } else {
                matchFlag = false;
            }
        }
        nextMatchCh = local_mem[match_loc % BOOSTER_OFFSET_WINDOW];
        if (outFlag) outStream << outStreamValue;
    }
    outStream << outValue;
lz_booster_left_bytes:
    for (uint32_t i = 0; i < LEFT_BYTES; i++) {
#pragma HLS PIPELINE off
        outStream << inStream.read();
    }
}

} // namespace compression
} // namespace xf

#endif // _XFCOMPRESSION_LZ_OPTIONAL_HPP_
