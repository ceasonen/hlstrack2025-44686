/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law of or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DUT_TYPE_HPP_
#define _DUT_TYPE_HPP_

#include "ap_fixed.h"
#include "hls_x_complex.h"

// ===================================================================================
// == FINAL CORRECTION FOR 3x3 MATRIX                                               ==
// ===================================================================================
// 1. Base Type: Use ap_fixed<16, 1> as it's efficient for the 3x3 test case.
// 2. Type Names: Use MATRIX_IN_T and MATRIX_OUT_T as these are the specific
//    names required by the kernel files for this test, which resolves the
//    "undeclared identifier" compilation error.

typedef ap_fixed<16, 1> BaseType;
typedef hls::x_complex<BaseType> MATRIX_IN_T;
typedef hls::x_complex<BaseType> MATRIX_OUT_T;


#endif // _DUT_TYPE_HPP_