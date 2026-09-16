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


// Tx state machines, handles events outside hpd/interrupt requests

#if BITEC_TX_CAPAB_MST_OR_DP20

typedef enum                       // PC fsm states
{
    PC_FSM_IDLE = 0,               // no operation
    PC_FSM_HPD_0,                  // HPD set to 0
    PC_FSM_HPD_1,                  // HPD set to 1
    PC_FSM_TEST_AUTOM,             // test automation
    //PC_FSM_LT,                   // link training (done through PC_FSM_HPD_!)
    PC_FSM_CLEAR_ALLOCATION,       // step to clear and redo the stream allocation
    PC_FSM_WAIT_CLEAR_ALLOCATION,  // step to clear and redo the stream allocation
    PC_FSM_START,                  // start checking a new connected sink
    PC_FSM_GET_PORTS,              // find connected MST ports
    PC_FSM_FIND_STREAM_X,          // find a port for Stream x
    PC_FSM_RDEDID_X,               // EDID Stream x read
    PC_FSM_ALLOCATE_STREAM_X,      // Allocate Stream x to port_idx[x]
    PC_FSM_WAIT_ALLOCATED_X,       // Wait for Stream x allocation
    PC_FSM_NOOUT,                  // Error state
    PC_FSM_MST_ON,                 // MST ON
} BTC_PC_STATE;
#else
// Link training at DP1.4 rates when MST and DP2.0 are not supported
void bitec_dptx_change_link_rate(unsigned int link_rate, unsigned int lane_count);
void bitec_dptx_linktrain();
// Simplified state machine for DP1.4 is MST is not supported
typedef enum                   // PC fsm states
{
    PC_FSM_IDLE = 0,           // no operation
    PC_FSM_HPD_0,              // HPD set to 0
    PC_FSM_HPD_1,              // HPD set to 1
    PC_FSM_TEST_AUTOM,         // test automation
    PC_FSM_LT,                 // link training
    PC_FSM_NOOUT,              // Error state
    PC_FSM_ON,                 // Running
} BTC_PC_STATE;
#endif

extern BTC_PC_STATE pc_fsm;  // PC fsm state

void bitec_dptx_init();
void bitec_dptx_pc();

