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
// DisplayPort Core test code RX utilities
//
// Description:
//
// ********************************************************************************


#include <stdio.h>
#include <string.h>
#include <io.h>
#include "btc_dprx_syslib.h"
#include "sys/alt_irq.h"
#include "sys/alt_timestamp.h"
#include "config.h"

#include "intel_fpga_i2c.h"

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DGB_PRINTF printf
#else
#define DGB_PRINTF(format, args...) ((void)0)
#endif

#if BITEC_RX_GPUMODE

void bitec_dprx_isr(void* context);
void bitec_edid_set_stream(BYTE port, BYTE offset, BYTE *edid);

void bitec_mc_monitor(void);
void bitec_mc_lt_handler(BYTE cmd, unsigned int address, BYTE length, BYTE *data);
void bitec_mc_rx_reset(void);
int bitec_mc_cr_check(int lane_cnt);

//******************************************************
// Initialize the RX
//******************************************************
void bitec_dprx_init()
{
  BYTE data[256];
  unsigned int rx_link_rate, rx_lane_count;
  unsigned int rx_8b10b_link_rate;

  // Get the core capabilities (defined in QSYS and ported to system.h)
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
  rx_link_rate = DP_RX_DP_SINK_RX_MGMT_BITEC_CFG_RX_MAX_LINK_RATE;
  rx_lane_count = DP_RX_DP_SINK_RX_MGMT_BITEC_CFG_RX_MAX_LANE_COUNT;
#else
  rx_link_rate = DP_RX_DP_SINK_BITEC_CFG_RX_MAX_LINK_RATE;
  rx_lane_count = DP_RX_DP_SINK_BITEC_CFG_RX_MAX_LANE_COUNT;
#endif

    rx_8b10b_link_rate      = rx_link_rate;

  // Init the DPCD

  // Set the capabilities
  data[0]  = 0x14; // DPCD_ADDR_DPCD_REV
  data[1]  = rx_8b10b_link_rate; // DPCD_ADDR_MAX_LINK_RATE
  data[2]  = 0x80 | (BITEC_RX_LQA_SUPPORT << 5) | rx_lane_count; // DPCD_ADDR_MAX_LANE_COUNT
  if(rx_8b10b_link_rate >= 0x14)
  {
    data[2]  |= 0x40; // Set TPS3_SUPPORTED if link rate >HBR2
  }
  data[3]  = 0x81 | (BITEC_RX_FAST_LT_SUPPORT << 6); // DPCD_ADDR_MAX_DOWNSPREAD
  data[4]  = 0x01; // DPCD_ADDR_NORP
  data[5]  = 0x01; // DPCD_ADDR_DOWNSTREAMPORT_PRESENT
  data[6]  = 0x01; // DPCD_ADDR_MAIN_LINK_CHANNEL_CODING
  data[7]  = 0x80; // DPCD_ADDR_DOWN_STREAM_PORT_COUNT or 0x81?
#if BITEC_RX_CAPAB_MST && BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_MST
  data[8]  = 0x00; // DPCD_ADDR_RECEIVE_PORT0_CAP_0
#else
  data[8]  = 0x02; // DPCD_ADDR_RECEIVE_PORT0_CAP_0
#endif
  data[9]  = 0x00; // DPCD_ADDR_RECEIVE_PORT0_CAP_1
  data[10] = 0x06; // DPCD_ADDR_RECEIVE_PORT1_CAP_0 (set to 0 if Audio not supported)
  data[11] = 0x00; // DPCD_ADDR_RECEIVE_PORT1_CAP_1
  data[12] = 0x00; // DPCD_ADDR_I2C_SPEED
  data[13] = 0x00; // DPCD_ADDR_EDP_CONFIGURATION_CAP
  data[14] = 0x84; // DPCD_ADDR_TRAINING_AUX_RD_INTERVAL, enable EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT
  data[15] = 0x00; // DPCD_ADDR_ADAPTER_CAP
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_DPCD_REV,16,data);

  // Set the Extended capabilities (for DP 1.3)
  data[0]  = 0x14; // DPCD_ADDR_DPCD_REV for DP 1.3
  data[1]  = rx_8b10b_link_rate; // DPCD_ADDR_MAX_LINK_RATE for DP 1.3
  data[5]  = 0x01; // DPCD_ADDR_DOWNSTREAMPORT_PRESENT for DP 1.3
#if BITEC_RX_SDP_SUPPORT
  data[16] = 0x08; // DPCD_ADDR_DPRX_FEATURE_ENUMERATION_LIST
#else
  data[16] = 0x00; // DPCD_ADDR_DPRX_FEATURE_ENUMERATION_LIST
#endif
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_EXTENDED_CAPAB_FIELD,17,data);


  data[0]  = 0x07; // DPCD_ADDR_FAUX_CAP
#if BITEC_RX_CAPAB_MST && BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_MST
  data[1]  = 0x01; // DPCD_ADDR_MST_CAP
#else
  data[1]  = 0x00; // DPCD_ADDR_MST_CAP
#endif
  data[2]  = 0x00; // DPCD_ADDR_N_OF_AUDIO_ENDPOINTS or 0x01?
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_FAUX_CAP,3,data);

  // Set GUID (all zeros)
  memset(data,0x00,16);
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_GUID,16,data);


  // Set Link Configuration
  data[0]  = 0x06; // DPCD_ADDR_LINK_BW_SET
  data[1]  = 0x01; // DPCD_ADDR_LANE_COUNT_SET
  data[2]  = 0x00; // DPCD_ADDR_TRAINING_PATTERN_SET
  data[3]  = 0x00; // DPCD_ADDR_TRAINING_LANE0_SET
  data[4]  = 0x00; // DPCD_ADDR_TRAINING_LANE1_SET
  data[5]  = 0x00; // DPCD_ADDR_TRAINING_LANE2_SET
  data[6]  = 0x00; // DPCD_ADDR_TRAINING_LANE3_SET
  data[7]  = 0x00; // DPCD_ADDR_DOWNSPREAD_CTRL
  data[8]  = 0x01; // DPCD_ADDR_MAIN_LINK_CHANNEL_CODING_SET
  data[9]  = 0x00; // DPCD_ADDR_I2C_SPEED1
  data[10] = 0x00; // DPCD_ADDR_EDP_CONFIGURATION_SET
  data[11] = 0x00; // DPCD_ADDR_LINK_QUAL_LANE0_SET
  data[12] = 0x00; // DPCD_ADDR_LINK_QUAL_LANE1_SET
  data[13] = 0x00; // DPCD_ADDR_LINK_QUAL_LANE2_SET
  data[14] = 0x00; // DPCD_ADDR_LINK_QUAL_LANE3_SET
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_LINK_BW_SET,15,data);
  data[0]  = 0x00; // DPCD_ADDR_LINK_SQUARE_PATTERN_NUM
  data[1]  = 0x00; // DPCD_ADDR_CABLE_ATTRIBUTES_BY_DPTX
  data[2]  = 0x00; // DPCD_ADDR_MSTM_CTRL
  data[3]  = 0x00; // DPCD_ADDR_AUDIO_DELAY0
  data[4]  = 0x00; // DPCD_ADDR_AUDIO_DELAY1
  data[5]  = 0x00; // DPCD_ADDR_AUDIO_DELAY2
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_LINK_SQUARE_PATTERN_NUM,6,data);

  // Set payload allocate
  data[0]  = 0x00; // DPCD_ADDR_PAYLOAD_ALLOCATE_SET
  data[1]  = 0x00; // DPCD_ADDR_PAYLOAD_ALLOCATE_START_TSLOT
  data[2]  = 0x00; // DPCD_ADDR_PAYLOAD_ALLOCATE_TSLOT_COUNT
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_PAYLOAD_ALLOCATE_SET,3,data);

  // Set Link Status
  data[0]  = 0x01; // DPCD_ADDR_SINK_COUNT
  data[1]  = 0x00; // DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR
  data[2]  = 0x00; // DPCD_ADDR_LANE0_1_STATUS
  data[3]  = 0x00; // DPCD_ADDR_LANE2_3_STATUS
  data[4]  = 0x00; // DPCD_ADDR_LANE_ALIGN_STATUS_UPDATED
  data[5]  = 0x00; // DPCD_ADDR_SINK_STATUS
  data[6]  = 0x00; // DPCD_ADDR_ADJUST_REQUEST_LANE0_1
  data[7]  = 0x00; // DPCD_ADDR_ADJUST_REQUEST_LANE2_3
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_SINK_COUNT,8,data);

  // Set VC payload table
  data[0]  = 0x00;
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_PAYLOAD_TABLE_UPDATE_STATUS,1,data);
  memset(data,0x00,63);
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_VC_PAYLOAD_ID_SLOT_1,63,data);

  // Set Source-specific locations
  memset(data,0x00,16);
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_SOURCE_IEEE_OUI_0,16,data);

  // Set Sink-specific locations
  data[0]  = 0x00; // DPCD_ADDR_SINK_IEEE_OUI_0
  data[1]  = 0x1B; // DPCD_ADDR_SINK_IEEE_OUI_1
  data[2]  = 0xC5; // DPCD_ADDR_SINK_IEEE_OUI_2
  data[3]  = 'I'; // Device identification string
  data[4]  = 'N';
  data[5]  = 'T';
  data[6]  = 'E';
  data[7]  = 'L';
  data[8]  = 0x00;
  data[9]  = 0x12; // HW revision
  data[10] = 0x01; // SW major revision
  data[11] = 0x02; // SW minor revision
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_SINK_IEEE_OUI_0,12,data);

  // Set Branch-specific locations
  data[0]  = 0x00; // DPCD_ADDR_BRANCH_IEEE_OUI_0
  data[1]  = 0x00; // DPCD_ADDR_BRANCH_IEEE_OUI_0
  data[2]  = 0x00; // DPCD_ADDR_BRANCH_IEEE_OUI_0
  data[3]  = 0x00; // Device identification string
  data[4]  = 0x00;
  data[5]  = 0x00;
  data[6]  = 0x00;
  data[7]  = 0x00;
  data[8]  = 0x00;
  data[9]  = 0x00; // HW revision
  data[10] = 0x00; // SW major revision
  data[11] = 0x00; // SW minor revision
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_BRANCH_IEEE_OUI_0,12,data);

  // Set ESI
  data[0]  = 0x01; // DPCD_ADDR_SINK_COUNT_ESI
  data[1]  = 0x00; // DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR_ESI0
  data[2]  = 0x00; // DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR_ESI1
  data[3]  = 0x00; // DPCD_ADDR_LINK_SERVICE_IRQ_VECTOR_ESI0
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_SINK_COUNT_ESI,4,data);

  // Set various locations
  data[0]  = 0x01; // DPCD_ADDR_SET_POWER_STATE
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_SET_POWER_STATE,1,data);

#if (DP_SUPPORT_RX_DSC && ( DP_SUPPORT_TX_DSC == 0))
  // Set DSC Capability for Rx only

  data[0]  = 0x01; // DPCD_ADDR_DSC_SUPPORT
  data[1]  = 0x21; // DPCD_ADDR_DSC_ALGORITHM_REVISION
  data[2]  = 0x01; // DPCD_ADDR_DSC_RC_BUFFER_BLOCK_SIZE
  data[3]  = 0x7F; // DPCD_ADDR_DSC_RC_BUFFER_SIZE
  data[4]  = 0x3B; // DPCD_ADDR_DSC_SLICE_CAPABILITIES_1
  data[5]  = 0x07; // DPCD_ADDR_DSC_LINE_BUFFER_DEPTH
  data[6]  = 0x01; // DPCD_ADDR_DSC_BLOCK_PREDICTION_SUPPORT
  data[7]  = 0x00; 
  data[8]  = 0x00; 
  data[9]  = 0x1F; // DPCD_ADDR_DSC_DECODER_COLOR_FORMAT_CAPAB
  data[10] = 0x07; // DPCD_ADDR_DSC_DECODER_COLOR_DEPTH_CAPAB
  data[11] = 0xAA; // DPCD_ADDR_PEAK_DSC_THROUGHPUT
  data[12] = 0x08; // DPCD_ADDR_DSC_MAXIMUM_SLICE_WIDTH
  data[13] = 0x00; // DPCD_ADDR_DSC_SLICE_CAPABILITIES_2
  data[14] = 0x00; 
  data[15] = 0x00; // DPCD_ADDR_BITS_PER_PIXEL_INCREMENT
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_DSC_SUPPORT,16,data);
#endif

#if DP_SUPPORT_RX_FEC
  // Set FEC Capability
  data[0]  = 0xbf; // DPCD_ADDR_FEC_CAPABILITY
  btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_FEC_CAPABILITY,16,data);
#endif

  // Register the interrupt handler
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
  alt_ic_isr_register(DP_RX_DP_SINK_RX_MGMT_IRQ_INTERRUPT_CONTROLLER_ID,
                      DP_RX_DP_SINK_RX_MGMT_IRQ,
                      bitec_dprx_isr,
                      NULL,
                      0x0);
#else
  alt_ic_isr_register(DP_RX_DP_SINK_IRQ_INTERRUPT_CONTROLLER_ID,
                      DP_RX_DP_SINK_IRQ,
                      bitec_dprx_isr,
                      NULL,
                      0x0);
#endif
}

//******************************************************
// RX IRQ service routine
//******************************************************
void bitec_dprx_isr(void* context)
{
  BYTE cmd,length,data[16];
  unsigned int address;

  // Use RX port 0
  if(btc_dprx_aux_get_request(0,&cmd,&address,&length,data))
    return; // No Source AUX Request?!

#if (BITEC_DP_CARD_REV == 11 || BITEC_DP_CARD_REV == 12)
    bitec_mc_lt_handler(cmd,address,length,data);
#endif

  btc_dprx_aux_handler(0,cmd,address,length,data);
}


//******************************************************
// Modify the EDID to get a different product name
// for each port number
//
// Input:   port    port number (0-7)
//          offset  offset of ASCII port number
//          edid    pntr. to edid to modify
//******************************************************
void bitec_edid_set_stream(BYTE port, BYTE offset, BYTE *edid)
{
  BYTE chksum = 0;
  int i;

  *(edid + offset) = 0x31 + port; // Set the product name

  // Recalculate block 0 checksum
  for(i = 0; i < 127; i++)
    chksum += edid[i];
  edid[127] = ~chksum + 1;
}

//*******************************
// MCDP6000 monitoring function
//*******************************
void bitec_mc_monitor(void)
{
  unsigned int aux_stat,data;
  static BYTE source_detected = 0;

  aux_stat = IORD(btc_dprx_baseaddr(0), DPRX_REG_AUX_STATUS);
  if((aux_stat & 0x03) == 0x03)
  {
    // Source cable connected and powered
    source_detected = 1;
    return;
  }
  else
  {
    // Source disconnected or powered off
    if(source_detected == 1)
    {
      // It just happened
      source_detected = 0;
      bitec_mc_lt_handler(0xFF,0,0,NULL); // Status reset
      data = 0x55801E14; intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0A00, (unsigned char *)&data, 4); // MC solution #2
    }
  }
}

//*******************
// MCDP6000 RX reset
//*******************
void bitec_mc_rx_reset(void)
{
  unsigned int mc_wr_dat;

  mc_wr_dat = 0x81; intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0150, (unsigned char *)&mc_wr_dat, 1);
  mc_wr_dat = 0x80; intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0150, (unsigned char *)&mc_wr_dat, 1);

}

//******************************************************
// MCDP6000 link status check in LT CR phase
//
// Input:   lane_cnt    lane count
//******************************************************
int bitec_mc_cr_check(int lane_cnt)
{
  unsigned int mc_rd_dat;

  intel_fpga_i2c_mc_read(I2C_MASTER_BASE, 0x28 >> 1, 0x0B20, (unsigned char *)&mc_rd_dat, 4);
  if (lane_cnt == 4)
  {
    if((mc_rd_dat & 0x1FFFF)!=0x1111 && (mc_rd_dat & 0x1FFFF)!=0x17777)
      return 1;
  }
  else if (lane_cnt == 2)
  {
    if((mc_rd_dat & 0x100FF)!=0x11 && (mc_rd_dat & 0x100FF)!=0x10077)
      return 1;
  }

  return 0;
}

//******************************************************
// MC solution #1
// AUX Request Handler (LT snooper)
//
// Input:   cmd     command (uses DP standards)
//          address address
//          length  length (0 - 16)
//          data    pntr. to data (1 - 16 bytes)
//******************************************************
void bitec_mc_lt_handler(BYTE cmd, unsigned int address, BYTE length, BYTE *data)
{
  static BYTE lane_cnt = 0;
  static BYTE lttpr = 0;
  static BYTE tps = 0;
  static BYTE rst_req = 0;

  if(cmd == 0xFF)
  {
    // Status reset
    lane_cnt = 0;
    lttpr = 0;
    tps = 0;
    rst_req = 0;
    return;
  }
  if ((cmd == 0x80) && (address == DPCD_ADDR_LINK_BW_SET) && (length > 1))
  {
    // Save lane count
    lane_cnt = data[1] & 0x1F;
    return;
  }
  else if (cmd == 0x80 && address == DPCD_ADDR_LANE_COUNT_SET)
  {
    // Save lane count
    lane_cnt = data[0] & 0x1F;
    return;
  }
  else if ((cmd == 0x80) && (address == 0xF0003))
  {
    // Save LTTPR mode
    if ((data[0] & 0xFF) == 0x55)
      lttpr = 0;
    else if ((data[0] & 0xFF) == 0xAA)
      lttpr = 1;
    return;
  }

  if (lttpr == 1)
  {
    if ((cmd == 0x80) && (address == 0xF0010))
    {
      // Save Test Pattern Sequence used
      tps = data[0] & 0xF;
      return;
    }
  }
  else
  {
    if ((cmd == 0x80) && (address == DPCD_ADDR_TRAINING_PATTERN_SET))
    {
      // Save Test Pattern Sequence used
      tps = data[0] & 0xF;
      return;
    }
  }

  if (tps == 1)
  {
    if (cmd == 0x80 && ((address == 0xF0011) || (address == DPCD_ADDR_TRAINING_LANE0_SET)))
    {
      rst_req = bitec_mc_cr_check(lane_cnt);
      tps = tps & rst_req;
      return;
    }
  }
  else if (tps != 0)
  {
    if (cmd == 0x80 && ((address == 0xF0011) || (address == DPCD_ADDR_TRAINING_LANE0_SET)))
    {
      rst_req = bitec_mc_cr_check(lane_cnt);
      if (rst_req == 0)
        tps = 0;
      return;
    }
  }

  if (rst_req == 1)
  {
    if (cmd == 0x90 && ((address == 0xF0030) || (address == DPCD_ADDR_LANE0_1_STATUS)))
    {
      bitec_mc_rx_reset();
      rst_req = 0;
      return;
    }
  }
}

#endif // BITEC_RX_GPUMODE


