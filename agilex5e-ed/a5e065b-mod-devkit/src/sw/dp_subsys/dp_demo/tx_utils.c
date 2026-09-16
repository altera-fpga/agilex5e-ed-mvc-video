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
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "btc_dptx_syslib.h"
#include "config.h"
#include "tx_utils.h"
#include "sys/alt_irq.h"
#include "sys/alt_timestamp.h"
#include "sys/msw_interrupt.h"
#if BITEC_TX_CAPAB_MST_OR_DP20
#include "btc_dptxll_syslib.h"
#include "btc_dptxll_globals.h"
#endif
#if DP_SUPPORT_TX_HDCP
#include "hdcp.h"
#endif

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DGB_PRINTF printf
#else
#define DGB_PRINTF(format, args...) ((void)0)
#endif

BTC_PC_STATE pc_fsm = PC_FSM_IDLE;       // PC fsm state
unsigned int active_stream_state;        // Just a 0..3 stream counter to complement the pc_fsm state machine and avoid code duplication

#if DEBUG_PRINT_ENABLED
BTC_PC_STATE pc_fsm_prev = PC_FSM_IDLE;  // Previous PC fsm state for debug
#endif

// If new Sink detected
int new_rx = 0;

// Just a flag for the FSM to check current status in case of external event(s)
unsigned int irq_triggered = 0;

// Dashboard bitfields
#define DASH_BPP18_MASK 0x0
#define DASH_BPP24_MASK 0x1
#define DASH_BPP30_MASK 0x2
#define DASH_BPP36_MASK 0x3
#define DASH_BPP48_MASK 0x4

BYTE tx_edid_data[128 * 4];  // TX copy of Sink EDID

void bitec_dptx_hpd_isr(void* context);
int bitec_dptx_hpd_irq();
void bitec_csn_callback(BTC_MST_CONN_STAT_NOTIFY* csn_data);
unsigned int bitec_dptx_test_autom();

#if BITEC_TX_CAPAB_MST_OR_DP20
// MST peer device types
#define BTC_PEER_DEV_NONE 0x00
#define BTC_PEER_DEV_SOURCE 0x01
#define BTC_PEER_DEV_BRANCH 0x02
#define BTC_PEER_DEV_SST_SINK 0x03
#define BTC_PEER_DEV_DP_TO_LEGACY 0x04
#define BTC_PEER_DEV_DP_TO_WIRELESS 0x05
#define BTC_PEER_DEV_WIRELESS_TO_DP 0x06
// MST status variables
BTC_MST_DEVPORT* dev_ports;              // Device ports of connected sink
BTC_MST_DEVICE_PORT* aPort;
int port_idx[MST_TX_STREAMS];
BYTE num_of_ports;
BYTE tavg_ts;
#else
// When the Txll library is not used, we need to keep track of the link rate and lane count requested for the link
unsigned int tx_link_rate;
unsigned int tx_lane_count;
#endif


// Get the core capabilities (defined in QSYS and ported to system.h)
#if DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_TX_ONLY
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LANE_COUNT
#else
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LANE_COUNT
#endif

//******************************************************
// Initialize the TX
//******************************************************
void bitec_dptx_init()
{
#if BITEC_TX_CAPAB_MST_OR_DP20
    #if (DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_TX_ONLY)
    btc_dptxll_syslib_add_tx(0, TX_MAX_LINK_RATE, TX_MAX_LANE_COUNT,
                             DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_NUM_OF_STREAMS, tx_edid_data);
    #else
    btc_dptxll_syslib_add_tx(0, TX_MAX_LINK_RATE, TX_MAX_LANE_COUNT,
                             DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_NUM_OF_STREAMS, tx_edid_data);
    #endif

    btc_dptxll_syslib_init();
    btc_dptxll_mst_set_csn_callback(0, bitec_csn_callback);

    for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
    {
        btc_dptxll_stream_set_color_space(0, k, 0, 1, 0, 0, 0);  // Set Stream k video color space
    }
    // Link training tweak for DP2.0 IP, retry link training at same rate once
    btc_dptxll_syslib_set_max_num_lt_retries(0, 1);

#else
    tx_link_rate = TX_MAX_LINK_RATE;
    tx_lane_count = TX_MAX_LANE_COUNT;
#endif

    // If Tx supports FEC at 8b10b, the FEC_READY bit (FEC_CONFIGURATION[0]) of a FEC capable Rx should be set to 1 prior to link training.
    btc_dptx_allow_8b10b_fec(0, DP_SUPPORT_TX_FEC);

    //Link training + aux tweaks
    //btc_dptx_set_128b132b_lt_400ms_timeout(900000);
    //btc_dptx_aux_set_max_num_defers(0, 8);

    // Register the interrupt handler
#if (DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_TX_ONLY)
    alt_ic_isr_register(DP_TX_DP_SOURCE_TX_MGMT_IRQ_INTERRUPT_CONTROLLER_ID,
                        DP_TX_DP_SOURCE_TX_MGMT_IRQ, bitec_dptx_hpd_isr, NULL, 0x0);
#else
    alt_ic_isr_register(DP_TX_DP_SOURCE_IRQ_INTERRUPT_CONTROLLER_ID, DP_TX_DP_SOURCE_IRQ,
                        bitec_dptx_hpd_isr, NULL, 0x0);
#endif

    irq_triggered = 0;
    new_rx = 0;
}

//******************************************************
// Perform Link Training in DP1.4 non-MST case
//******************************************************
#if BITEC_TX_CAPAB_MST_OR_DP20 == 0
void bitec_dptx_change_link_rate(unsigned int link_rate, unsigned int lane_count)
{
    // Store link_rate/lane_count so they are re-used for next link training after rx HPD
    tx_link_rate = link_rate;
    tx_lane_count = lane_count;
}

void bitec_dptx_linktrain()
{
    // Link train with default link_rate/lane_count values
    btc_dptx_edid_read(0, tx_edid_data);                     // Read the sink EDID
    btc_dptx_set_color_space(0, 0, DP_TX_BPS, 0, 0, 0);      // Set TX video color space
    btc_dptx_link_training(0, tx_link_rate, tx_lane_count);        // Do link training without FEC
}
#endif

//******************************************************
// HPD activity service routine
//******************************************************
void bitec_dptx_hpd_isr(void* context)
{
    unsigned int status_reg;

    // Disable TX Core HPD interrupts
    BTC_DPTX_DISABLE_HPD_IRQ(0);

    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    status_reg = IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS);
    IOWR(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS, 0);  // Writing DPTX_REG_TX_STATUS clears IRQ
    if (status_reg & 0x1)
    {
        // Long HPD
        if (status_reg & 0x4)
        {
            DGB_PRINTF("bitec_dptx_hpd_isr(): long HPD\n");
            // HPD is at '1'
            pc_fsm = PC_FSM_HPD_1;
        }
        else
        {
            // HPD is at '0'
            DGB_PRINTF("bitec_dptx_hpd_isr(): HPD 0\n");
            pc_fsm = PC_FSM_HPD_0;
        }
    }
    else if (status_reg & 0x2)
    {
        // HPD short pulse (IRQ)
        #if DEBUG_PRINT_ENABLED
            BYTE irq_reg;
            BYTE stat[2];
            btc_dptx_aux_read(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, &irq_reg);  // Read irq
            btc_dptx_aux_read(0,DPCD_ADDR_LANE_ALIGN_STATUS_UPDATED, 2, stat);
            DGB_PRINTF("bitec_dptx_hpd_isr(): HPD interrupt 0x%x, lane_align_status 0x%x, sink_status 0x%x\n", irq_reg, stat[0], stat[1]);
        #endif

        // Route the call to the btc_dptxll_hpd_irq handler or the bespoke bitec_dptx_hpd_irq below
        // In either cases, non-zero return values indicate that actions from outside the interrupt handlers are required (eg, reestablishing the link, test_autom,...)
#if BITEC_TX_CAPAB_MST_OR_DP20
        int irq_ret = btc_dptxll_hpd_irq(0); // Use handler from the dptxll lib
#else
        int irq_ret = bitec_dptx_hpd_irq(); // Use bespoke handler (below)
#endif

        // Whichever handler was used, state machines may have to handle a status change
        // Check for Test Automation requests
        if (irq_ret == 4) {
        #if DP_SUPPORT_TX_HDCP
            // HDCP IRQ
            hdcp_cp_irq(0);
        #endif
        }
        if (irq_ret == 3) {
            pc_fsm = PC_FSM_TEST_AUTOM;
        }
        // Check for port status change (in the MST case) and redo the allocation
        if (irq_ret == 2) {
            #if BITEC_TX_CAPAB_MST_OR_DP20
                pc_fsm = PC_FSM_HPD_1;
            #endif
        }
        // Check for link status change
        if (irq_ret == 1) {
            irq_triggered = 1;
        }
    }

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable TX Core HPD interrupts
    BTC_DPTX_ENABLE_HPD_IRQ(0);
}

//******************************************************
// HPD IRQ (short pulse) bespoke handler, non MST case
// Returns:   0 = handled, 1 = lane status change, 2 = downstream port status change, 3 = test request, 4 = hdcp IRQ
//******************************************************
int bitec_dptx_hpd_irq()
{
    BYTE data[5];

    BYTE data_byte;
    unsigned int status_ok;

    btc_dptx_aux_read(0, DPCD_ADDR_SINK_COUNT, 5, data);  // Read link status

    // Check CP_IRQ
    if (data[1] & 0x04)
    {
        // HDCP CP_IRQ
        data_byte = 0x04;
        btc_dptx_aux_write(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, &data_byte);  // Reset CP_IRQ
        return 4;
    }

    // Non-zero return values require action from outside the interrupt handlers (eg, reestablishing the link, test_autom,...)
    if (bitec_dptx_test_autom()) {
        return 3;
    }

    // Check and return downstream port change
    if ((data[4] & (1<<6)) != 0)
    {
        // Get sink edid
        btc_dptx_edid_read(0, tx_edid_data);
        return 2;
    }

    // Check link status
    status_ok = data[4] & 0x01; // Get inter-lane align
    btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, &data_byte);
    BYTE lane_count = data_byte & 0x1F;
    switch(lane_count)
    {
      case 1 : status_ok = status_ok && ((data[2] & 0x07) == 0x07); break;
      case 2 : status_ok = status_ok && ((data[2] & 0x77) == 0x77); break;
      case 4 : status_ok = status_ok && ((data[2] & 0x77) == 0x77) && ((data[3] & 0x77) == 0x77); break;
      default : break;
    }
    if (!status_ok)
    {
        return 1;
    }

    return 0;
}


//******************************************************
// Test Automation handler
//
// Return: 1 = Test Automation requested, 0 = no action
//******************************************************
unsigned int bitec_dptx_test_autom()
{
    BYTE data[2];

    // Check Automated test request
    btc_dptx_aux_read(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, data);
    btc_dptx_aux_read(0, DPCD_ADDR_TEST_REQUEST, 1, data + 1);

    if (data[0] & 0x02)
    {
        BYTE test_autom_clear = 0x02;
        btc_dptx_aux_write(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, &test_autom_clear);  // Reset test automation IRQ
        // Flag that test automation handling is required
        if (data[1] != 0x00)
            return 1;
    }

    return 0;
}

#if BITEC_TX_CAPAB_MST_OR_DP20
//******************************************************
// Simulates a PC using the TX MST link
// Pick up one of the connected sink
// devices and try to output video to it
//
// Must be invoked periodically
//
// Stream management (MST framing and 128b/132b modes):
// ensure the output stream configuration is properly set
// when the allocation starts (PC_FSM_START)
//******************************************************
void bitec_dptx_pc()
{
    unsigned int i;
    int ret_val;
    BYTE vcp_size;
    BYTE chan_coding;
    BYTE data_byte;
    BYTE *edid;

    // Debug code
#if DEBUG_PRINT_ENABLED
    const char* pc_fsm_states[] = {"PC_FSM_IDLE",
                                   "PC_FSM_HPD_0",
                                   "PC_FSM_HPD_1",
                                   "PC_FSM_TEST_AUTOM",
                                   //"PC_FSM_LT",
                                   "PC_FSM_CLEAR_ALLOCATION",
                                   "PC_FSM_WAIT_CLEAR_ALLOCATION",
                                   "PC_FSM_START",
                                   "PC_FSM_GET_PORTS",
                                   "PC_FSM_FIND_STREAM_X",
                                   "PC_FSM_RDEDID_X",
                                   "PC_FSM_ALLOCATE_STREAM_X",
                                   "PC_FSM_WAIT_ALLOCATED_X",
                                   "PC_FSM_NOOUT",
                                   "PC_FSM_MST_ON",
                                   };

    if (pc_fsm != pc_fsm_prev)
    {
        DGB_PRINTF("pc_fsm(): %s \n", pc_fsm_states[pc_fsm]);
    }
    pc_fsm_prev = pc_fsm;
#endif

    switch (pc_fsm)
    {
    case PC_FSM_IDLE:  // No sink detected
        break;

    case PC_FSM_HPD_0:  // HPD set to 0
        btc_dptxll_hpd_change(0, 0);
        pc_fsm = PC_FSM_IDLE;
        break;

    case PC_FSM_HPD_1:  // HPD set to 1
#if DP_SUPPORT_TX_HDCP
        hdcp_unauth();
#endif
        DGB_PRINTF("PC_FSM_HPD_1\n");
        // This triggers link discovery, test automation check, link training and topology discovery
        ret_val = btc_dptxll_hpd_change(0, 1);
        if (ret_val == 0)
        {
            new_rx = 1;
            // Test Automation requests are handled within btc_dptxll_hpd_change
            // Link training is handled within btc_dptxll_hpd_change
            // btc_dptxll_hpd_change will also call btc_dptxll_mst_new_sink:
            //      * if the sink is Multi-stream capable then it clears the stream allocation and started a topology discovery
            //      * if the sink is not MST-capable but we link trained at 128b132b rates then it simply clears the allocation table

            //if (bitec_dptx_test_autom()) {
            //    pc_fsm = PC_FSM_TEST_AUTOM;
            //} else {
            //    pc_fsm = PC_FSM_LT;
            //}
            pc_fsm = PC_FSM_START; // Jump straight to PC_FSM_START to go through a new allocation
        }
        else
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;
    case PC_FSM_TEST_AUTOM:
        btc_dptx_test_autom(0);
        pc_fsm = PC_FSM_NOOUT;   //TODO(?): sink hotplug needed or other internal action to restart normal video
        break;
    case PC_FSM_CLEAR_ALLOCATION:        // Clear allocation tables (Tx HW, direct sink and MST topology)
        pc_fsm = PC_FSM_NOOUT;  // jump to SST output by default if MST framing is not used, otherwise we clear the payload table
        btc_dptx_aux_read(0, DPCD_ADDR_TRAINING_AUX_RD_INTERVAL, 1, &data_byte);
        if (data_byte & 0x80)
        {
            btc_dptx_aux_read(0, DPCD_ADDR_EXTENDED_CAPAB_FIELD, 1, &data_byte);  // Use DP 1.3 capabilities
        }
        else
        {
            btc_dptx_aux_read(0, DPCD_ADDR_DPCD_REV, 1, &data_byte);
        }
        if (data_byte >= 0x12)
        {
            // DPCD 1.2 or higher, go through the vcp allocation if MST framing is on at 128b/132b rates OR both the sink and this source support MST at 8b/10b rates)
            // BITEC_TX_CAPAB_MST_OR_DP20 doesn't guarantee the Tx can do MST framing at 8b10b rates
            btc_dptx_aux_read(0, DPCD_ADDR_MST_CAP, 1, &data_byte);
            chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
            if (((data_byte & 1) && BITEC_TX_CAPAB_MST) || (chan_coding & 0x2))
            {
                btc_dptxll_stream_clear_payload_id_table_req(0);
                pc_fsm = PC_FSM_WAIT_CLEAR_ALLOCATION;
            }
        }
        break;
    case PC_FSM_WAIT_CLEAR_ALLOCATION:
        i = btc_dptxll_stream_clear_payload_id_table_rep(0);
        if (i != 2)
        {
            // Move on even in case of error, delay until clear allocation is complete
            for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
            {
                port_idx[k] = 255;
            }
            pc_fsm = PC_FSM_GET_PORTS;
        }
        break;
    case PC_FSM_START:  // A new sink got connected, redo the allocation
        btc_dptx_aux_read(0, DPCD_ADDR_TRAINING_AUX_RD_INTERVAL, 1, &data_byte);
        if (data_byte & 0x80)
        {
            btc_dptx_aux_read(0, DPCD_ADDR_EXTENDED_CAPAB_FIELD, 1, &data_byte);  // Use DP 1.3 capabilities
        }
        else
        {
            btc_dptx_aux_read(0, DPCD_ADDR_DPCD_REV, 1, &data_byte);
        }
        if (data_byte >= 0x12)
        {
            // DPCD 1.2 or higher, go through the vcp allocation if MST framing is on at 128b/132b rates OR both the sink and this source support MST at 8b/10b rates)
            // BITEC_TX_CAPAB_MST_OR_DP20 doesn't guarantee the Tx can do MST framing at 8b10b rates
            btc_dptx_aux_read(0, DPCD_ADDR_MST_CAP, 1, &data_byte);
            chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
            if (((data_byte & 1) && BITEC_TX_CAPAB_MST) || (chan_coding & 0x2))
            {
                for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
                {
                    port_idx[k] = 255;
                }
                pc_fsm = PC_FSM_GET_PORTS;  // MST_CAP = 1
            }
            else
            {
                pc_fsm = PC_FSM_NOOUT;  // MST_CAP = 0: use SST output
            }
        }
        else
        {
            pc_fsm = PC_FSM_NOOUT;  // MST_CAP = 0: use SST output
        }
        break;
    case PC_FSM_GET_PORTS:  // Determine DP device attached
        i = btc_dptxll_mst_get_device_ports(0, &dev_ports, &num_of_ports);
        // i==2 -> RETRY
        if (i == 1) {
            DGB_PRINTF("btc_dptxll_mst_get_device_ports returned FAIL\n");
            pc_fsm = PC_FSM_NOOUT;
        }
        else if (i == 0)
        {
            if (num_of_ports == 0)
            {
                // Bypass, no valid reply from sink (or SST mode)
                num_of_ports = 1;
                dev_ports[0].port.displayport_device_plug_status = 1;
                dev_ports[0].port.peer_device_type = BTC_PEER_DEV_SST_SINK;
                port_idx[0] = 0;
                // Allocate stream 0
                active_stream_state = 0;
                pc_fsm = PC_FSM_ALLOCATE_STREAM_X;
            }
            else
            {
                DGB_PRINTF("btc_dptxll_mst_get_device_ports returned SUCCESS. Number of ports: %x \n", num_of_ports);
                // Topology discovery ready: search for sink devices for streams
                active_stream_state = 0;
                pc_fsm = PC_FSM_FIND_STREAM_X;
            }
        }
        break;
    case PC_FSM_FIND_STREAM_X:  // Find a port for stream "active_stream_state"
        int initial_port_idx;
        initial_port_idx = 0;
        if (active_stream_state != 0)
        {
            initial_port_idx = port_idx[active_stream_state-1] + 1;
        }
        // Loop through left-over ports and stop when one is found
        for (i = initial_port_idx; i < num_of_ports; i++)
        {
            aPort = &dev_ports[i].port;
            DGB_PRINTF("Input Port: %x \n", aPort->input_port);
            DGB_PRINTF("Port Connected: %x \n", aPort->displayport_device_plug_status);
            DGB_PRINTF("Peer Device Type: %x \n", aPort->peer_device_type);
            DGB_PRINTF("Messaging Capability: %x \n", aPort->messaging_capability_status);
            DGB_PRINTF("available_PBN is : %d \n", dev_ports[i].available_PBN);
            if (aPort->input_port == 0 && aPort->displayport_device_plug_status == 1 &&
                (aPort->peer_device_type == BTC_PEER_DEV_SST_SINK &&
                 aPort->messaging_capability_status == 0) &&
                 dev_ports[i].available_PBN > 0)
                break;
        }

        // i < num_of_ports, a candidate port was found
        if (i < num_of_ports)
        {
            // A suitable device port was found
            port_idx[active_stream_state] = i;
            // Read EDID for port tied to active_stream_state
            btc_dptxll_mst_edid_read_req(0, &dev_ports[i].RAD, dev_ports[i].port.port_number);
            pc_fsm = PC_FSM_RDEDID_X;
        }
        else
        {
            port_idx[active_stream_state] = 255;
            // Get stream 0 going, even if we just found port_idx[0] == 255
            active_stream_state = 0;
            pc_fsm = PC_FSM_ALLOCATE_STREAM_X;
        }
        break;
    case PC_FSM_RDEDID_X:  // Wait for EDID Stream X read complete
        i = btc_dptxll_mst_edid_read_rep(0, &edid);
        if (i == 0)
        {
            // Edid read done, look for a port for the next stream or move on to allocation
            if (MST_TX_STREAMS > (active_stream_state + 1))
            {
                ++active_stream_state;
                pc_fsm = PC_FSM_FIND_STREAM_X;
            }
            else
            {
                active_stream_state = 0;
                pc_fsm = PC_FSM_ALLOCATE_STREAM_X;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;
    case PC_FSM_ALLOCATE_STREAM_X:  // Allocate Stream "active_stream_state" to port_idx[active_stream_state]
        if (!btc_dptx_is_link_up(0))
        {
            DGB_PRINTF("link down, allocation for stream %d skipped\n", active_stream_state);
            pc_fsm = PC_FSM_NOOUT;
            break;
        }

        chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
        vcp_size = btc_dptxll_stream_calc_VCP_size(0, active_stream_state);
        if ((vcp_size == 0) || (vcp_size > ((chan_coding == 2) ? 64 : 63)))
        {
            DGB_PRINTF("Could not allocate Stream %d, invalid vcp_size=%d\n", active_stream_state, vcp_size);
            pc_fsm = PC_FSM_NOOUT;
        }
        else
        {
            // This calls cover the VCP allocation for the sink but also the ALLOCATE_PAYLOAD message
            // when in multi-stream mode
            btc_dptxll_stream_allocate_req(0, active_stream_state, &dev_ports[port_idx[active_stream_state]]);
            pc_fsm = PC_FSM_WAIT_ALLOCATED_X;
        }
        break;

    case PC_FSM_WAIT_ALLOCATED_X:  // Wait for Stream "active_stream_state" allocation
        i = btc_dptxll_stream_allocate_rep(0);
        if (i == 0)
        {
            // Allocate next stream or move back to waiting state
            if ((MST_TX_STREAMS > (active_stream_state + 1)) && (port_idx[active_stream_state+1] != 255))
            {
                ++active_stream_state;
                pc_fsm = PC_FSM_ALLOCATE_STREAM_X;
            }
            else
            {
                pc_fsm = PC_FSM_MST_ON;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_X;  // Retry
        }
        break;
    case PC_FSM_NOOUT:  // No suitable sink device available
        // Fallthrough intentional so we deal with Rx IRQ the same way and the state machine may be reset
    case PC_FSM_MST_ON:  // MST ON!
        if (irq_triggered)
        {
            BYTE lstat[5];

            // Get current lane count
            btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, &data_byte);
            BYTE lane_count = data_byte & 0x1F;

            btc_dptx_aux_read(0, DPCD_ADDR_SINK_COUNT, 5, lstat);

            // Check link status
            BYTE status_ok = lstat[4] & 0x01;  // Get inter-lane align
            switch (lane_count)
            {
            case 1:
                status_ok = status_ok && ((lstat[2] & 0x07) == 0x07);
                break;
            case 2:
                status_ok = status_ok && ((lstat[2] & 0x77) == 0x77);
                break;
            case 4:
                status_ok = status_ok && ((lstat[2] & 0x77) == 0x77) && ((lstat[3] & 0x77) == 0x77);
                break;
            default:
                break;
            }
            if (!status_ok)
            {
        #if DP_SUPPORT_TX_HDCP
                hdcp_cp_irq(0);
        #endif
                pc_fsm = PC_FSM_HPD_1;
            }
            irq_triggered = 0;
        }
    }
}

#else
//******************************************************
// Simulates a PC maintaining the TX link (no MST framing)
//
// Must be invoked periodically
//******************************************************
void bitec_dptx_pc()
{
    // Debug code
#if DEBUG_PRINT_ENABLED
    const char* pc_fsm_states[] = {"PC_FSM_IDLE",
                                   "PC_FSM_HPD_0",
                                   "PC_FSM_HPD_1",
                                   "PC_FSM_TEST_AUTOM",
                                   "PC_FSM_LT",
                                   "PC_FSM_ON",
                                   "PC_FSM_LINK_STAT_CHANGE"};

    if (pc_fsm != pc_fsm_prev)
    {
        DGB_PRINTF("pc_fsm(): %s->%s \n", pc_fsm_states[pc_fsm_prev], pc_fsm_states[pc_fsm]);
    }
    pc_fsm_prev = pc_fsm;
#endif

    // int link_rate;
    // int lane_count;
    // int bpc;
    BYTE data_byte;

    switch (pc_fsm)
    {
    case PC_FSM_IDLE:  // No sink detected
        break;

    case PC_FSM_HPD_0:  // HPD set to 0
        // Send the idle pattern
        btc_dptx_hpd_change(0, 0);
        btc_dptx_video_enable(0, 0);
        pc_fsm = PC_FSM_IDLE;
        break;

    case PC_FSM_HPD_1:  // HPD set to 1
#if DP_SUPPORT_TX_HDCP
        hdcp_unauth();
#endif
        // This triggers link discovery
        btc_dptx_hpd_change(0, 1);

        // Get a copy of the sink EDID
        btc_dptx_edid_read(0, tx_edid_data);

        // If new Sink is detected.
        new_rx = 1;

        // Check for automated test request, trigger link training
        if (bitec_dptx_test_autom()) {
            pc_fsm = PC_FSM_TEST_AUTOM;
        } else {
            pc_fsm = PC_FSM_LT;
        }
        break;
    case PC_FSM_TEST_AUTOM:
        btc_dptx_test_autom(0);
        pc_fsm = PC_FSM_LT;
        break;
    case PC_FSM_LT:
        // Link train at current tx_link_rate, tx_lane_count
        bitec_dptx_linktrain();
        btc_dptx_video_enable(0, 1);
        pc_fsm = PC_FSM_ON;
        break;
    case PC_FSM_NOOUT:  // error state
        // fallthrough intentional
    case PC_FSM_ON:  // ON!
        if (irq_triggered)
        {
            BYTE lstat[5];

            // Get current lane count
            btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, &data_byte);
            BYTE lane_count = data_byte & 0x1F;

            btc_dptx_aux_read(0, DPCD_ADDR_SINK_COUNT, 5, lstat);

            // Check link status
            BYTE status_ok = lstat[4] & 0x01;  // Get inter-lane align
            switch (lane_count)
            {
            case 1:
                status_ok &= ((lstat[2] & 0x07) == 0x07);
                break;
            case 2:
                status_ok &= ((lstat[2] & 0x77) == 0x77);
                break;
            case 4:
                status_ok &= ((lstat[2] & 0x77) == 0x77) & ((lstat[3] & 0x77) == 0x77);
                break;
            default:
                break;
            }
            if (!status_ok)
            {
        #if DP_SUPPORT_TX_HDCP
                hdcp_unauth();
        #endif
                pc_fsm = PC_FSM_LT;
            }
            irq_triggered = 0;
        }
    }
}

#endif


//******************************************************
// CONNECTION_STATUS_NOTIFY callback
//******************************************************
void bitec_csn_callback(BTC_MST_CONN_STAT_NOTIFY* csn_data)
{
}

