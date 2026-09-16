//  ###############################################################################
//   
//  INTEL CONFIDENTIAL
//  
//  Copyright Intel Corporation.
//  
//  This software and the related documents are Intel copyrighted materials, and
//  your use of them is governed by the express license under which they were
//  provided to you ("License"). Unless the License provides otherwise, you may
//  not use, modify, copy, publish, distribute, disclose or transmit this software
//  or the related documents without Intel's prior written permission.
//  
//  This software and the related documents are provided as is, with no express or
//  implied warranties, other than those that are expressly stated in the License.
//  
//  ###############################################################################

#ifndef __SCALER_COEFFS_H
#define __SCALER_COEFFS_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "intel_vvp_scaler.h"

int get_required_lobes(int in_size, int out_size, int taps);

int get_active_taps(int in_size, int out_size, int lobes, int taps);

float lanczos(float x, float lobes);

void generate_lanczos_coefficients(float *coeffs, int phase, int taps, int phases, int lobes, int frac_bits, int active_taps);

void scaler_create_and_load_horizontal_coefficents(intel_vvp_scaler_instance* scaler, int in_width, int out_width, int bank);

void scaler_create_and_load_vertical_coefficents(intel_vvp_scaler_instance* scaler, int in_width, int out_width, int bank);

#ifdef __cplusplus
}
#endif /* __cplusplus */

  
#endif /* __SCALER_COEFFS_H */