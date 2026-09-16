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
// DisplayPort Sink System Library public definitions
//
// Description:
//
// ********************************************************************************

#ifndef _BTC_DPRX_SYSLIB_H_
#define _BTC_DPRX_SYSLIB_H_

#include <stdio.h>
#include "btc_dp_dpcd.h"
#include "btc_dp_types.h"
#include "btc_dp_rxregs.h"

#ifndef BTC_DPRX_SYSLIB_VER
#define BTC_DPRX_SYSLIB_VER "6.3.3795"
#endif

#define BTC_100US_RXTICKS     1 //number of timer time ticks equ to 100 us

#ifndef BTC_RAD_DEFINE
typedef struct  // MST Relative Address
{
  BYTE length; // Address (addr[]) length
  BYTE addr[15];
}BTC_RAD;
#define BTC_RAD_DEFINE
#endif

// HDCP Decoder status
typedef struct
{
  BYTE new : 1;               // 1 = this StreamID_Type list is new
  BYTE stream_num;            // Number of streams contained in streamid_type (max 63 streams)
  BYTE streamid_type[126];    // StreamID_Type list last received
}BTC_HDCPRX_STAT;

// RX instance options
#define BTC_DPRX_OPT_DISABLE_ERRMON		0x0001

// Enable / Disable IRQ on AUX Requests from source
#define BTC_DPRX_ENABLE_IRQ(rx_idx)     IOWR(btc_dprx_baseaddr(rx_idx),DPRX_REG_AUX_CONTROL,IORD(btc_dprx_baseaddr(rx_idx),DPRX_REG_AUX_CONTROL) | (1U << 8))
#define BTC_DPRX_DISABLE_IRQ(rx_idx)    IOWR(btc_dprx_baseaddr(rx_idx),DPRX_REG_AUX_CONTROL,IORD(btc_dprx_baseaddr(rx_idx),DPRX_REG_AUX_CONTROL) & ~(1U << 8))
#define BTC_DPRX_ISENABLED_IRQ(rx_idx)  ((IORD(btc_dprx_baseaddr(rx_idx),DPRX_REG_AUX_CONTROL) >> 8) & 0x01)

//********** btc_dprx_common.c *********//
int btc_dprx_syslib_add_rx(BYTE rx_idx,
                           unsigned int rx_base_addr,
                           unsigned int rx_irq_id,
                           unsigned int rx_irq_num,
                           unsigned int rx_num_of_sinks,
                           unsigned int options);
int btc_dprx_syslib_init(void);
int btc_dprx_syslib_monitor(void);
void btc_dprx_syslib_info(BYTE *max_sink_num, BYTE *mst_support);
unsigned int btc_dprx_baseaddr(BYTE rx_idx);
int btc_dprx_set_dpcd_ver(BYTE rx_idx, BYTE ver);
void btc_dprx_sw_ver(BYTE *major,BYTE *minor, unsigned int *rev);
void btc_dprx_rtl_ver(BYTE *major,BYTE *minor, unsigned int *rev);

//********** btc_dprx_aux_ch.c *********//
int btc_dprx_aux_get_request(BYTE rx_idx,BYTE *cmd,unsigned int *address,BYTE *length,BYTE *data);
int btc_dprx_aux_post_reply(BYTE rx_idx,BYTE cmd,BYTE size,BYTE *data);
int btc_dprx_aux_handler(BYTE rx_idx,BYTE cmd,unsigned int address,BYTE length,BYTE *data);
void btc_dprx_aux_set_i2c(BYTE rx_idx,BYTE i2c_idx,BYTE start_addr,BYTE end_addr);

//********** btc_dprx_dpcd.c *********//
int btc_dprx_dpcd_gpu_access(BYTE rx_idx,BYTE wrcmd,unsigned int address,BYTE length,BYTE *data);
unsigned int get_dpcd_hdcp_stream_manage_rdy ();
void set_dpcd_hdcp_stream_manage_rdy (unsigned int data);
unsigned char * get_dpcd_hdcp_streamid_type();
unsigned char * get_dpcd_hdcp_stream_k();
void set_dpcd_hdcp_dev_cnt (unsigned int cnt);

//********** btc_dprx_edid.c *********//
int btc_dprx_edid_set(BYTE rx_idx,BYTE port,BYTE *edid_data,BYTE num_blocks);

//********** btc_dprx_hpd.c *********//
void btc_dprx_hpd_set(BYTE rx_idx,int level);
int btc_dprx_hpd_get(BYTE rx_idx);
void btc_dprx_hpd_pulse(BYTE rx_idx,BYTE dev_irq_vect0,BYTE dev_irq_vect1,BYTE link_irq_vect0);

//********** btc_dprx_lt.c *********//
void btc_dprx_lt_force(BYTE rx_idx);
int btc_dprx_lt_eyeq_init(BYTE rx_idx, BYTE enabled, BYTE log_chan_from, BYTE log_chan_to, unsigned int rcnf_base_addr);
int btc_dprx_lt_ffe_retries_set(unsigned int retries);
int btc_dprx_lt_ffe_default_set(BYTE tx_ffe);
int btc_dprx_lt_dnu_swpre_get(BYTE rx_idx, BYTE lrate); // Debug access to 8b10b DNU status. lrate from 0 (RBR) to 3 (HBR3)
                                                        // bit[15:12]= pree 3, swing 3 downto 0, bit[3:0]= pree 0, swing 3 downto 0
int btc_dprx_lt_dnu_ffe_get(BYTE rx_idx, BYTE lrate);   // Debug access to 128b132b DNU status. lrate 0 (10Gbps), 1 (20Gbps), 2 (13.5Gbps)
                                                        // bit[15:0]= respective FFE value


// Debug counters to keep track of link failure *after* succesful link training.
// timepoint 0 = already failed when training pattern DPCD reg is set back to 0, timepoint 1 = failure after 10 ms, timepoint 2 = failure after 500 ms
BYTE btc_dprx_get_lt_failure_count(BYTE rx_idx, unsigned int link_rate, unsigned int lane_count, unsigned int timepoint);
void btc_dprx_reset_lt_failure_counters(BYTE rx_idx); // Debug access to reset failure counters

//********** btc_dprx_dsc.c *********//
int btc_dprx_dsc_set(BYTE rx_idx, BYTE strm_idx, unsigned int clk_dsc_khz, unsigned int rx_im_clk_khz, BYTE pix_per_clk);

//********** btc_dprx_mst.c *********//
int btc_dprx_mst_conn_stat_notify_rep(BYTE rx_idx,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dprx_mst_conn_stat_notify_req(BYTE rx_idx,BYTE dfp_num,BYTE legacy_device_plug_status,BYTE displayport_device_plug_status,BYTE messaging_capability_status,BYTE input_port,BYTE peer_device_type);
int btc_dprx_mst_link_addr_rep_set(BYTE rx_idx,BYTE dfp_num,BYTE input_port,BYTE peer_device_type,BYTE messaging_capability_status,BYTE displayport_device_plug_status,BYTE legacy_device_plug_status,BYTE dpcd_revision);

//********** btc_dprx_sbmsg.c *********//
int btc_dprx_sbmsg_rx_request(BYTE rx_idx,BTC_RAD *RAD,BYTE *msg,BYTE length,BYTE broadcast,BYTE path);

#endif /* _BTC_DPRX_SYSLIB_H_ */
