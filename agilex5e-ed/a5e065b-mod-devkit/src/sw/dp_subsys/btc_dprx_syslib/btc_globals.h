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
// DisplayPort Sink System Library global (library private) definitions
//
// Description:
// These definitions are global to all library routines but private to this
// library.
// Updated for DP2.0
// ********************************************************************************

#include "btc_cpu.h"
#include "btc_dprx_syslib.h"

#define BTC_RX_DISABLE_DP13  0  // Set to 1 to disable RX DP 1.3 capabilities

#define AUX_NATIVE_WR     0x80
#define AUX_NATIVE_RD     0x90
#define AUX_I2C_WR        0x00
#define AUX_I2C_RD        0x10
#define AUX_I2C_UPDATE    0x20
#define AUX_I2C_MOT       0x40

#define AUX_ACK           0x00
#define AUX_NACK          0x10
#define AUX_DEFER         0x20
#define AUX_I2C_ACK       0x00
#define AUX_I2C_NACK      0x40
#define AUX_I2C_DEFER     0x80

// RX instance type
typedef struct
{
  unsigned int base_addr;     // RX base address
  unsigned int irq_id;        // RX IRQ ID
  unsigned int irq_num;       // RX IRQ number
  unsigned int options;
  BYTE num_of_sinks : 3;      // Number of Stream Sinks for this RX
  BYTE source_detected : 1;   // 1 = source detected, 0 = source unplugged or powered off
  BYTE dpcd : 2;              // 1 = DPCD 1.1, 2 = DPCD 1.2
  BYTE rxcaps_msg : 1;        // 1 = HDCP2.2 RxCaps read as part of AKE_Send_Cert msg, 0 = HDCP2.2 RxCaps read by itself
} BRX_RXINSTANCE_TYPE;

// Sideband DOWN_REQ msg type
typedef struct
{
  BYTE relative_address[14];
  BYTE link_count_total : 4;
  BYTE msg_seq_no : 1;
  BYTE broadcast_msg : 1;
  BYTE path_msg : 1;
  BYTE valid : 1;   // 1 = msg complete and valid
  BYTE msg[256];
  BYTE ptr;         // index of next msg byte free to fill (= total message length)
}BRX_SBMSG_DWNREQ;

// Sideband DOWN_REP msg type
typedef struct
{
  BYTE relative_address[14];
  BYTE link_count_total : 4;
  BYTE msg_seq_no : 1;
  BYTE broadcast_msg : 1;
  BYTE path_msg : 1;
  BYTE valid : 1;      // 1 = msg valid (= must be transmitted)
  BYTE msg[256];
  BYTE length;        // message length (1 - 255)
  BYTE ptr;           // index of next msg byte to be sent
}BRX_SBMSG_DWNREP;

// Configuration data
#ifdef BITEC_DPRX_MST_SUPPORT
#define BTC_MAX_NUM_OF_STRM_SINKS    4 // Max number of MST Stream Sinks supported
#else
#define BTC_MAX_NUM_OF_STRM_SINKS    1 // Max number of MST Stream Sinks supported
#endif
#define BTC_MAX_NUM_OF_SINKS         4 // Max number of Sink cores supported
#define BTC_RX_SMARTLT_OPT1          1 // Smart Link Training Option 1 (remember unsuccessful swing and pre-emphasis pairs)
#define BTC_RX_SMARTLT_OPT3          1 // Set to 1 to enable Link Quality Analysis (LQA)
#define BTC_RX_EYEQ                  1 // Set to 1 to enable EyeQ equalizer management
#define BTC_RX_LT_TESTER             0 // Set to 1 to enable Link Training tester functionality

//********** btc_dprx_aux_ch.c *********//
int btc_dprx_aux_init(void);
void btc_dprx_aux_monitor(BYTE rx_idx);

//********** btc_dprx_dpcd.c *********//
int btc_dprx_dpcd_aux_access(BYTE rx_idx,BYTE wrcmd,unsigned int address,BYTE length,BYTE *data);
BYTE *btc_dprx_dpcd_MAX_LINK_RATE(BYTE rx_idx);
BYTE *btc_dprx_dpcd_MAX_LANE_COUNT(BYTE rx_idx);
BYTE *btc_dprx_dpcd_CHANNEL_CODING_CAP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LINK_BW_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LANE_COUNT_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_PATTERN_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_LANE0_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_LANE1_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_LANE2_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_LANE3_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_MAIN_LINK_CHANNEL_CODING_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_DEVICE_SERVICE_IRQ_VECTOR(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LANE0_1_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LANE2_3_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LANE_ALIGN_STATUS_UPDATED(BYTE rx_idx);
BYTE *btc_dprx_dpcd_SINK_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_ADJUST_REQUEST_LANE0_1(BYTE rx_idx);
BYTE *btc_dprx_dpcd_ADJUST_REQUEST_LANE2_3(BYTE rx_idx);
BYTE *btc_dprx_dpcd_DOWN_REQ(BYTE rx_idx);
BYTE *btc_dprx_dpcd_DOWN_REP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_UP_REQ(BYTE rx_idx);
BYTE *btc_dprx_dpcd_UP_REP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_PANEL_REPLAY_ENABLE_AND_CONFIG(BYTE rx_idx);
BYTE *btc_dprx_dpcd_PAYLOAD_ALLOCATE_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT(BYTE rx_idx);
BYTE *btc_dprx_dpcd_PAYLOAD_TABLE_UPDATE_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_VC_PAYLOAD_ID_SLOT_1(BYTE rx_idx);
BYTE *btc_dprx_dpcd_SET_POWER_STATE(BYTE rx_idx);
BYTE *btc_dprx_dpcd_MSTM_CAP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_MSTM_CTRL(BYTE rx_idx);
BYTE *btc_dprx_dpcd_DEVICE_SERVICE_IRQ_VECTOR_ESI1(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LINK_SERVICE_IRQ_VECTOR_ESI0(BYTE rx_idx);
BYTE *btc_dprx_dpcd_PANEL_REPLAY_ERROR_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_SUPR_EVENT_STATUS_INDICATOR(BYTE rx_idx);
BYTE *btc_dprx_dpcd_SINK_DEVICE_PANEL_REPLAY_STATUS(BYTE rx_idx);
BYTE *btc_dprx_dpcd_DEBUG_1_LAST_RECEIVED_VSC_SDP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_LINK_QUAL_LANE0_SET(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TEST_SINK(BYTE rx_idx);
BYTE *btc_dprx_dpcd_EXT_MAX_LINK_RATE(BYTE rx_idx);
BYTE *btc_dprx_dpcd_EXT_CHANNEL_CODING_CAP(BYTE rx_idx);
BYTE *btc_dprx_dpcd_FEC_CONFIG(BYTE rx_idx);
BYTE *btc_dprx_dpcd_SOURCE_IEEE_OUI(BYTE rx_idx);
BYTE *btc_dprx_dpcd_UD_DP_PWR(BYTE rx_idx);
BYTE *btc_dprx_dpcd_TRAINING_PATTERN_SET_LTTPR(BYTE rx_idx);

//********** btc_dprx_edid_ch.c *********//
void btc_dprx_edid_init(void);
int btc_dprx_edid_aux_access(BYTE rx_idx,BYTE port,BYTE cmd,unsigned int address,BYTE length,BYTE *data);
int btc_dprx_edid_sbmsg_rd_access(BYTE rx_idx,BYTE port,BYTE *wr_addr,BYTE *wr_len,BYTE wr_num,BYTE *wr_data,BYTE rd_addr,BYTE *rd_len,BYTE *rd_data);
void btc_dprx_edid_monitor(BYTE rx_idx);

//********** btc_dprx_lt.c *********//
int btc_dprx_lt_init(void);
void btc_dprx_lt_reset(BYTE rx_idx);
int btc_dprx_lt_is_link_up(BYTE rx_idx);
void btc_dprx_lt_monitor(BYTE rx_idx);
void btc_dprx_lt_linkrate_set(BYTE rx_idx);
void btc_dprx_lt_lanecount_set(BYTE rx_idx);
void btc_dprx_lt_channel_coding_set(BYTE rx_idx);
int btc_dprx_lt_tp_set(BYTE rx_idx);
int btc_dprx_lt_iteration(BYTE rx_idx);
int btc_dprx_lt_upd_status(BYTE rx_idx);
void btc_dprx_lt_upd_adjreq(BYTE rx_idx);
int btc_dprx_lt_is_LQA_in_progress(BYTE rx_idx);
void btc_dprx_inc_loop_count(BYTE rx_idx);
void btc_dprx_max_loop_count(BYTE rx_idx);

//********** btc_dprx_sbmsg.c *********//
void btc_dprx_sbmsg_init(void);
void btc_dprx_sbmsg_reset(BYTE rx_idx);
void btc_dprx_sbmsg_monitor(BYTE rx_idx);
void btc_dprx_sbmsg_rx_frag(BYTE rx_idx,BYTE offset,BYTE *data,BYTE length);
int btc_dprx_sbmsg_tx_reply(BYTE rx_idx,BRX_SBMSG_DWNREP *rep);
int btc_dprx_sbmsg_is_downreq_buf_avail(BYTE rx_idx);
int btc_dprx_sbmsg_is_uprep_buf_avail(BYTE rx_idx);
void btc_dprx_sbmsg_tx_rep_frag(BYTE rx_idx,BYTE offset,BYTE *data,BYTE length);

//********** btc_dprx_mst.c *********//
void btc_dprx_mst_init(void);
void btc_dprx_mst_reset(BYTE rx_idx);
void btc_dprx_mst_monitor(BYTE rx_idx);
void btc_dprx_mst_exec(BYTE rx_idx,BRX_SBMSG_DWNREQ *req);
void btc_dprx_mst_payload_allocate_set(BYTE rx_idx);
void btc_dprx_mst_force_vcp_tab_update(BYTE rx_idx);
void btc_dprx_mst_up_rep_decode(BYTE rx_idx,BYTE *msg);
int btc_dprx_mst_vcpid_set(BYTE rx_idx,BYTE strm_idx, BYTE vcpid);

//********** btc_dprx_common.c *********//
extern BRX_RXINSTANCE_TYPE _brx_inst[BTC_MAX_NUM_OF_SINKS]; // RX instances info
void btc_dprx_syslib_irq_enable(BYTE rx_idx,int enable);
int btc_dprx_syslib_irq_disable(BYTE rx_idx);

//********** btc_dprx_hdcp.c *********//
void btc_dprx_pr_monitor(BYTE rx_idx);

// RX instance I/O
#define BRX_IORD(idx,offset)      IORD(_brx_inst[idx].base_addr,offset)
#define BRX_IOWR(idx,offset,data) IOWR(_brx_inst[idx].base_addr,offset,data)

// HW Timers
#define BTC_RX_WAIT(rx_idx,delay_us) {  unsigned to = (delay_us/100)*BTC_100US_RXTICKS; \
                                        unsigned tstart = BRX_IORD(rx_idx,DPRX_REG_TIMESTAMP); \
                                        while(((BRX_IORD(rx_idx,DPRX_REG_TIMESTAMP) - tstart) & 0xFFFFFF) < to);  }

#define BTC_RX_TOUT(rx_idx,start,tout_us) (((BRX_IORD(rx_idx,DPRX_REG_TIMESTAMP) - start) & 0xFFFFFF) >= \
                                           ((tout_us/100)*BTC_100US_RXTICKS))

