// ********************************************************************************
//
// Copyright 2001-2021 Intel Corporation
// This software and the related documents are Intel copyrighted materials,
// and your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise,
// you may not use, modify, copy, publish, distribute, disclose or transmit
// this software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express or
// implied warranties, other than those that are expressly stated in the License.
//
// ********************************************************************************
// DisplayPort Core test code debug routines definitions
//
// Description:
//
// ********************************************************************************

#include "config.h"


#if BITEC_STATUS_DEBUG || BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG

//==================================================================
// Debug/UI initialization
//==================================================================
void debug_init();

//==================================================================
// Get Nios jtag input
//==================================================================
char* get_stdin();

//==================================================================
// Printout all menu commands
//==================================================================
void menu_print();

//==================================================================
// Implementation of menu commands
//==================================================================
void menu_cmd();

#endif


#if BITEC_STATUS_DEBUG

//==================================================================
// Output IP parameterization
//==================================================================
void print_hw_config();

//==================================================================
// Output status
//==================================================================
#if DP_SUPPORT_RX
void print_sink_msa(unsigned int base_addr);
void print_sink_config(unsigned int base_addr);
void print_sink_debug(unsigned int base_addr);
#endif
#if DP_SUPPORT_TX
void print_source_msa(unsigned int base_addr);
void print_source_config(unsigned int base_addr);
void print_source_debug(unsigned int base_addr);
#endif


#endif


#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG
//==================================================================
// Output aux traffic
//==================================================================
void dump_aux_traffic();

#endif

