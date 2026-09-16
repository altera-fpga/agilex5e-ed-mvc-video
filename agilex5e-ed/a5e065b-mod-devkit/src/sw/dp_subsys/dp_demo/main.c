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
// DisplayPort Core test code main
//
// Description:
//
// ********************************************************************************

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "btc_dprx_syslib.h"
#include "btc_dptx_syslib.h"
#if BITEC_TX_CAPAB_MST_OR_DP20
#include "btc_dptxll_syslib.h"
#endif

#include "debug.h"
#include "intel_fpga_i2c.h"
#include "tx_utils.h"
#include "board.h"
#if DP_SUPPORT_TX_HDCP
#include "hdcp.h"
#endif
#include "tdp2004.h"

#if DP_SUPPORT_AXI
#define AXI_OFFSET 0x300
#if DP_SUPPORT_TX
#include "intel_axi2cv.h"
#endif
#if DP_SUPPORT_RX
#include "intel_cv2axi.h"
#endif
#endif


#if (DP_SUPPORT_TX && DP_SUPPORT_TX_DSC)
#include "dsc_utils.h"
#endif

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DBG_PRINTF printf
#else
#define DBG_PRINTF(format, args...) ((void)0)
#endif


#if PSG_8K_EDID
// Intel PSG 8K30 EDID
BYTE intel_psg_edid[256] = {
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x42, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2E, 0x1C, 0x01, 0x04, 0xA5, 0x46, 0x27, 0x78, 0x3A, 0x77, 0x45, 0xAE, 0x51, 0x33, 0xBA, 0x26,
    0x0D, 0x50, 0x54, 0xA5, 0x4B, 0x00, 0x81, 0x00, 0xB3, 0x00, 0xD1, 0x00, 0xA9, 0x40, 0x81, 0x80,
    0xD1, 0xC0, 0x01, 0x00, 0x01, 0x00, 0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20,
    0x35, 0x00, 0xBA, 0x89, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0x00, 0x50, 0x53, 0x47, 0x2D, 0x38, 0x4B, 0x33, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x01, 0xD7,
    0x70, 0x12, 0x1E, 0x00, 0x00, 0x81, 0x00, 0x04, 0x23, 0x08, 0x1F, 0x03, 0x03, 0x00, 0x14, 0x65,
    0x8E, 0x01, 0x84, 0xFF, 0x1D, 0x4F, 0x00, 0x07, 0x80, 0x1F, 0x00, 0xDF, 0x10, 0x3C, 0x00, 0x2E,
    0x00, 0x07, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90};
#else
#if BITEC_RX_CAPAB_MST && BITEC_TX_CAPAB_MST
// MST is enabled
BYTE intel_psg_edid[128] = {
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x10, 0xAC, 0x8B, 0x40, 0x4C, 0x35, 0x33, 0x30,
    0x2A, 0x17, 0x01, 0x04, 0xA5, 0x43, 0x1C, 0x78, 0x3E, 0xEE, 0x95, 0xA3, 0x54, 0x4C, 0x99, 0x26,
    0x0F, 0x50, 0x54, 0xA5, 0x4B, 0x00, 0x81, 0x00, 0xB3, 0x00, 0x71, 0x4F, 0x81, 0x80, 0xD1, 0xC0,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7E, 0x48, 0x80, 0xE0, 0x70, 0x38, 0x1F, 0x40, 0x40, 0x40,
    0x3A, 0x00, 0xA1, 0x1C, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x35, 0x59, 0x44,
    0x38, 0x43, 0x33, 0x41, 0x48, 0x30, 0x33, 0x35, 0x4C, 0x0A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x44,
    0x45, 0x4C, 0x4C, 0x20, 0x55, 0x32, 0x39, 0x31, 0x33, 0x57, 0x4D, 0x0A, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x31, 0x56, 0x1D, 0x5E, 0x13, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x26};
#else
// Intel PSG 4K60 EDID
BYTE intel_psg_edid[128] = {
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x42, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2E, 0x1C, 0x01, 0x04, 0xA5, 0x46, 0x27, 0x78, 0x3A, 0x77, 0x45, 0xAE, 0x51, 0x33, 0xBA, 0x26,
    0x0D, 0x50, 0x54, 0xA5, 0x4B, 0x00, 0x81, 0x00, 0xB3, 0x00, 0xD1, 0x00, 0xA9, 0x40, 0x81, 0x80,
    0xD1, 0xC0, 0x01, 0x00, 0x01, 0x00, 0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20,
    0x35, 0x00, 0xBA, 0x89, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0x00, 0x50, 0x53, 0x47, 0x2D, 0x34, 0x4B, 0x36, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0xD9};
#endif
#endif

#if (DP_DSC_TX_ONLY)
BYTE edid_2560x1600[128] = {
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x6F, 0x28, 0x16, 0x00, 0x60, 0x00, 0x00,
    0x00, 0x20, 0x01, 0x04, 0xB5, 0x22, 0x16, 0x78, 0x03, 0x29, 0xF5, 0xA5, 0x54, 0x4B, 0x9D, 0x24,
    0x0D, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xC0, 0x72, 0x00, 0xA0, 0xA0, 0x40, 0xC8, 0x60, 0x30, 0x20,
    0x36, 0x00, 0x58, 0xD7, 0x10, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFD, 0x0C, 0x3C, 0xF0, 0xB1,
    0xB1, 0x76, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x43,
    0x53, 0x4F, 0x54, 0x20, 0x54, 0x33, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFE,
    0x00, 0x4D, 0x4E, 0x47, 0x30, 0x30, 0x37, 0x44, 0x41, 0x32, 0x2D, 0x33, 0x0A, 0x20, 0x02, 0xA9};
#endif

void bitec_dprx_init();
void bitec_mc_monitor(void);

extern int new_rx;
extern BYTE tx_edid_data[512];      // TX copy of Sink EDID

#if (DP_DSC_TX_ONLY)
//Arrays to hold mode and pps data
struct mode_parameters mode_param_array[20];
struct pps_parameters  dsc_data_pps_parameters;
int mode_command;
#endif

#if DP_SUPPORT_TX && DP_SUPPORT_RX
int sink_status[MST_RX_STREAMS];    // Keep a record of sink status to handle passthrough changes
int source_status[MST_TX_STREAMS];  // Keep a record of source status to handle passthrough changes
unsigned int get_vfreq_from_mvid_nvid(unsigned int mvid, unsigned int nvid, int link_rate); // For DP1.4->DP2.0 passthrough, deduce vfreq (within 1%) from mvid/nvid
#endif


#if DP_SUPPORT_AXI
struct color_info
{
    int space;
    int depth;
    int wcg; /* SDR color (bt.709/sRGB) or HDR gamut (bt2020) */
};
struct image_config
{
    int x;
    int y;
    bool interlace;
    struct color_info image_color;
};

typedef enum
{
    CLK_3_375,
    CLK_6_75,
    CLK_9_281,
    CLK_18_5625,
    CLK_37_125,
    CLK_74_25,
    CLK_148_5,
    CLK_297
} TX_CLK_FREQ;
void tx_clk_freq(TX_CLK_FREQ clk_freq);

#if DP_SUPPORT_TX
intel_axi2cv_instance axi2cv;
void cvo_program_mode_bank(intel_axi2cv_instance* cvo, unsigned int sel);
#endif

#if DP_SUPPORT_RX
intel_cv2axi_instance cv2axi;

// MVC                    
void cvi_res_switch(intel_cv2axi_instance* cvi,
                    struct image_config* current_image_config);

void cvo_res_switch(intel_axi2cv_instance* cvo, unsigned int dim_x, unsigned int dim_y, unsigned int fps_val);                    

struct image_config cvi_image_config = {.x = 0,
                                        .y = 0,
                                        .interlace = false,
                                        .image_color.space = 0,
                                        .image_color.depth = 0,
                                        .image_color.wcg = 0};
#endif

// MVC
int res_switch = 1;
int res_switch_tx = 1;

#endif // //DP_SUPPORT_AXI



// Rx caps can only be changed with GPU_MODE on and are kept in DPCD
// registers so don't need to be tracked here


#if DP_SUPPORT_TX && DP_SUPPORT_RX

//==================================================================
// Helper functions for stream management in loopback mode
//==================================================================
void stream_passthrough(unsigned int rx_stream, unsigned int tx_stream)
{
    unsigned prx;
    unsigned ptx;
    unsigned int rx_stream_offset;
    unsigned int tx_stream_offset;
    BYTE rx_ch_coding;
    BYTE tx_ch_coding;

    prx = btc_dprx_baseaddr(0);
    ptx = btc_dptx_baseaddr(0);

    rx_ch_coding = (IORD(prx, DPRX_REG_RX_CONTROL) >> 5) & 0x03;
    tx_ch_coding = (IORD(ptx, DPTX_REG_TX_CONTROL) >> 10) & 0x03;

    rx_stream_offset = DPRX1_REG_MSA_MVID - DPRX0_REG_MSA_MVID;
    tx_stream_offset = DPTX1_REG_MSA_MVID - DPTX0_REG_MSA_MVID;

    // MVC
    // Detect RX Colormetry and update TX MSA accordingly, the call to btc_dptxll_stream_set_color_space should happen
    // before the call to btc_dptxll_stream_set_pixel_rate for the VCP slot allocation to be computed correctly
    // unsigned int rx_color_enc;
    // unsigned int rx_color_range;
    // unsigned int rx_color_colorimetry;
    // unsigned int rx_color_bpc;
    // unsigned int rx_color_sdp;
    unsigned int rx_color;
    // unsigned int tx_color;
    rx_color = IORD(btc_dprx_baseaddr(0), DPRX0_REG_MSA_COLOUR + rx_stream_offset*rx_stream) & 0xFFFF;
    // rx_color_enc = (rx_color >> 4) & 0x0f;
    // rx_color_range = (rx_color >> 12) & 0x01;
    // rx_color_colorimetry = (rx_color >> 8) & 0x0f;
    // rx_color_bpc = (rx_color >> 0) & 0x07;
    // rx_color_sdp = (rx_color >> 13) & 0x01;  
    
/////////////////////////
    // MVC Specific
    // Tx-only design set colorimetry
    // RX is decoupled from Tx, hence set colorimetry for Tx-side
    // unsigned int ptx;
    // ptx = btc_dptx_baseaddr(0);
    unsigned int current_color = IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_COLOUR) & 0xFFFF;
    unsigned int tx_color_format = 0;
    unsigned int tx_color_bpc = 1; //8bit ==1, 10bbits == 2, DP_TX_BPS;    
    unsigned int tx_color_range = 1;
    unsigned int tx_color_colorimetry = 3;
    unsigned int tx_color_use_vsc_sdp = 0; // use misc0
    unsigned int tx_color = (tx_color_use_vsc_sdp << 13) | (tx_color_range << 12) | (tx_color_colorimetry << 8)  | (tx_color_format << 4) | tx_color_bpc;

    if (current_color != tx_color)
    {
        #if BITEC_TX_CAPAB_MST_OR_DP20
        btc_dptxll_stream_set_color_space(0, 0, tx_color_format, tx_color_bpc, tx_color_range,
                                         tx_color_colorimetry, tx_color_use_vsc_sdp);
        #else
        btc_dptx_set_color_space(0, tx_color_format, tx_color_bpc, tx_color_range,
                                 tx_color_colorimetry, tx_color_use_vsc_sdp);
        #endif
    }
  
////////////////////////    
    
    // tx_color = IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_COLOUR + tx_stream_offset*tx_stream) & 0xFFFF;
    // if((rx_color & 0xDFFF) != (tx_color & 0xDFFF))   //ignore bit 13 as RX does not always set this when the TX does
    // {
    //     unsigned int tx_color_sdp = rx_color_sdp;
    //     if (rx_color_enc == 3) {    //for 420 we need to send colorimetry via VSC packet, but DP1.4 core does not assert rx_color_sdp in reg map
    //         tx_color_sdp = 1;
    //     }
    //     #if BITEC_TX_CAPAB_MST_OR_DP20
    //         btc_dptxll_stream_set_color_space(0, tx_stream, rx_color_enc, rx_color_bpc, rx_color_range,
    //                                          rx_color_colorimetry, tx_color_sdp);
    //     #else
    //         btc_dptx_video_enable(0, 0);
    //         btc_dptx_set_color_space(0, rx_color_enc, rx_color_bpc, rx_color_range,
    //                                  rx_color_colorimetry, tx_color_sdp);
    //         btc_dptx_video_enable(0, 1);
    //     #endif
    // }

    if (rx_ch_coding == 1)
    {
        // DP1.4 Rx, get mvid/nvid
        // unsigned int rx_mvid;
        // unsigned int rx_nvid;

        // rx_mvid = IORD(prx, DPRX0_REG_MSA_MVID + rx_stream_offset*rx_stream) & 0xFFFFFF;
        // rx_nvid = IORD(prx, DPRX0_REG_MSA_NVID + rx_stream_offset*rx_stream) & 0xFFFFFF;

        #if BITEC_TX_CAPAB_MST_OR_DP20   // Compulsory for DP2.0 Tx and if tx_ch_coding == 2
        {
            int rx_link_rate;
            unsigned vfreq_k;

            rx_link_rate = (IORD(prx, DPRX_REG_RX_CONTROL) >> 16) & 0xFF;
            vfreq_k = get_vfreq_from_mvid_nvid(rx_mvid, rx_nvid, rx_link_rate);

            DBG_PRINTF("DEBUG: 8b10b->? passthrough %d->%d, rx_mvid=0x%x,rx_nvid=0x%x -> vfreq_k=%u\n", rx_stream, tx_stream, rx_mvid, rx_nvid, vfreq_k);

            btc_dptxll_stream_set_pixel_rate(0, tx_stream, vfreq_k);
            // Set vfreq on DPTX at UHBR rates
            if (tx_ch_coding == 2)
            {
                dp2p0_dptxll_stream_set_vfreq(0, tx_stream, vfreq_k);
            }
        }
        #endif
        //if (tx_ch_coding == 1)
        //{
            // copy Rx mvid/nvid values directly to Tx (vfreq written above if tx_ch_coding == 2
            //IOWR(ptx, DPTX0_REG_MSA_MVID + tx_stream_offset*tx_stream, rx_mvid);
            //IOWR(ptx, DPTX0_REG_MSA_NVID + tx_stream_offset*tx_stream, rx_nvid);
        //}
    }
    else
    {
        // DP2.0 Rx, get vfreq
        // unsigned int vfreq_upper;
        // unsigned int vfreq_lower;
        // vfreq_lower = IORD(prx, DPRX0_REG_MSA_MVID + rx_stream_offset*rx_stream) & 0xFFFFFF;
        // vfreq_upper = IORD(prx, DPRX0_REG_MSA_NVID + rx_stream_offset*rx_stream) & 0xFFFFFF;

        #if BITEC_TX_CAPAB_MST_OR_DP20
            unsigned long long vfreq = (unsigned long long)vfreq_upper << 24 | (unsigned long long)vfreq_lower;
            unsigned int vfreq_k = vfreq / 1000;
            btc_dptxll_stream_set_pixel_rate(0, tx_stream, vfreq_k);
            // Set vfreq on DPTX at UHBR rates
            if (tx_ch_coding == 2)
            {
                dp2p0_dptxll_stream_set_vfreq(0, tx_stream, vfreq_k);
            }
            DBG_PRINTF("DEBUG: 128b132b->? passthrough %d->%d, vfreq_k prop=%u\n", rx_stream, tx_stream, vfreq_k);
        #endif
        //if (tx_ch_coding == 1)
        //{
            // DP 1.4 Tx, NVID = 0x8000, calculate MVID
            //unsigned int mvid = get_mvid_from_vfreq(vfreq_upper, vfreq_lower, tx_link_rate);
            //IOWR(ptx, DPTX0_REG_MSA_MVID + tx_stream_offset*tx_stream, mvid);
            //IOWR(ptx, DPTX0_REG_MSA_NVID + tx_stream_offset*tx_stream, 0x8000);
        //}
    }

    #if TX_VIDEO_IM_ENABLE
    {
        // Video Image interface Tx input in use:
        // copy RX MSA parameters to the TX
        IOWR(ptx, DPTX0_REG_MSA_HTOTAL + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_HTOTAL + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_VTOTAL + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_VTOTAL + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_HSP + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_HSP + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_HSW + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_HSW + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_HSTART + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_HSTART + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_VSTART + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_VSTART + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_VSP + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_VSP + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_VSW + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_VSW + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_HWIDTH + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_HWIDTH + rx_stream_offset*rx_stream));
        IOWR(ptx, DPTX0_REG_MSA_VHEIGHT + tx_stream_offset*tx_stream, IORD(prx, DPRX0_REG_MSA_VHEIGHT + rx_stream_offset*rx_stream));
        DBG_PRINTF("DEBUG: Video IM Tx, passthrough %d->%d, pull MSA params from Rx, htotal=%u, vtotal=%u\n", rx_stream, tx_stream,
                   IORD(ptx, DPTX0_REG_MSA_HTOTAL + tx_stream_offset*tx_stream), IORD(ptx, DPTX0_REG_MSA_VTOTAL + tx_stream_offset*tx_stream));
    }
    #endif
}
#endif  //DP_SUPPORT_TX && DP_SUPPORT_RX (passthrough setup)


//==================================================================
// Main Program
//==================================================================
int main()
{
    // MVC
    unsigned int dp_tx_fps              = 0;
    unsigned int dp_tx_actv_dim_x       = 0;
    unsigned int dp_tx_actv_dim_y       = 0;
    unsigned int dp_tx_actv_dim         = 0;
    // unsigned int prev_dp_tx_new_dim_x   = 0;
    // unsigned int prev_dp_tx_new_dim_y   = 0;
    // unsigned int prev_dp_tx_new_fps     = 0;
    unsigned int dp_tx_new_fps          = 0;
    unsigned int dp_tx_new_dim_x        = 0;
    unsigned int dp_tx_new_dim_y        = 0;     
    
#if DP_SUPPORT_TX_HDCP
    unsigned int previous_hdcpenabled = 0;
#endif

    // Start with non-blocking jtag uart to avoid lock up when the host is not connected to the JTAG UART terminal
    fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    // Initialise user interface and display HW parameterization
#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG || BITEC_STATUS_DEBUG
    debug_init();
    #if BITEC_STATUS_DEBUG
    // print_hw_config();
    #endif
#endif

    // Perform Devkit Specific initialisations
    board_configure();

#ifdef ALT_VIP_MIX_0_BASE
    // Enable TPG Background : Disable DP image
    IOWR(ALT_VIP_MIX_0_BASE, 0, 0);          // Stop
    IOWR(ALT_VIP_MIX_0_BASE, 2, 0);          // Stream 0 offset X
    IOWR(ALT_VIP_MIX_0_BASE, 3, 0);          // Stream 0 offset Y
    IOWR(ALT_VIP_MIX_0_BASE, 4, 0);          // Stream 0 off
    IOWR(ALT_VIP_MIX_0_BASE, 5, 1920 / 2);   // Stream 1 offset X
    IOWR(ALT_VIP_MIX_0_BASE, 6, 0);          // Stream 1 offset Y
    IOWR(ALT_VIP_MIX_0_BASE, 7, 0);          // Stream 1 off
    IOWR(ALT_VIP_MIX_0_BASE, 8, 0);          // Stream 2 offset X
    IOWR(ALT_VIP_MIX_0_BASE, 9, 600);        // Stream 2 offset Y
    IOWR(ALT_VIP_MIX_0_BASE, 10, 0);         // Stream 2 off
    IOWR(ALT_VIP_MIX_0_BASE, 11, 1920 / 2);  // Stream 3 offset X
    IOWR(ALT_VIP_MIX_0_BASE, 12, 600);       // Stream 3 offset Y
    IOWR(ALT_VIP_MIX_0_BASE, 13, 0);         // Stream 3 off
    IOWR(ALT_VIP_MIX_0_BASE, 0, 1);          // Go
#endif

#if (DP_SUPPORT_TX_HDCP || DP_SUPPORT_RX_HDCP)
    hdcp_init();
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
    intel_fpga_i2c_init(I2C_MASTER_EEPROM_BASE, 100000000);
    hdcp_read_keys_eeprom();
#endif
#endif

#if (DP_SUPPORT_AXI && DP_SUPPORT_TX && DP_SUPPORT_RX)
    tx_clk_freq(CLK_37_125);
#endif

    // Init Bitec DP system library
#if DP_SUPPORT_TX
#if (DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_TX_ONLY)
    btc_dptx_syslib_add_tx(0, DP_TX_DP_SOURCE_TX_MGMT_BASE,
                           DP_TX_DP_SOURCE_TX_MGMT_IRQ_INTERRUPT_CONTROLLER_ID,
                           DP_TX_DP_SOURCE_TX_MGMT_IRQ);
    btc_dptx_syslib_init();
#else
    btc_dptx_syslib_add_tx(0, DP_TX_DP_SOURCE_BASE, DP_TX_DP_SOURCE_IRQ_INTERRUPT_CONTROLLER_ID,
                           DP_TX_DP_SOURCE_IRQ);
    btc_dptx_syslib_init();
#endif
#endif

#if DP_SUPPORT_RX
#if BITEC_RX_GPUMODE
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
    btc_dprx_syslib_add_rx(
        0, DP_RX_DP_SINK_RX_MGMT_BASE, DP_RX_DP_SINK_RX_MGMT_IRQ_INTERRUPT_CONTROLLER_ID,
        DP_RX_DP_SINK_RX_MGMT_IRQ, DP_RX_DP_SINK_RX_MGMT_BITEC_CFG_RX_MAX_NUM_OF_STREAMS, 0);
    btc_dprx_syslib_init();
#else
    btc_dprx_syslib_add_rx(0, DP_RX_DP_SINK_BASE, DP_RX_DP_SINK_IRQ_INTERRUPT_CONTROLLER_ID,
                           DP_RX_DP_SINK_IRQ, DP_RX_DP_SINK_BITEC_CFG_RX_MAX_NUM_OF_STREAMS, 0);
    btc_dprx_syslib_init();
#endif
#endif
#endif

#if (BITEC_DP_CARD_REV == 9 || BITEC_DP_CARD_REV == 10)
    // Bitec Daughter Card Rev 10
    // Init the PS8460 I2C interface
    intel_fpga_i2c_init(I2C_MASTER_BASE, 100000000);

    // Set the PS8460 P4 registers
    // Set the PS8460 equaliser as required by your design
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x09, 0x02);
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x0B,
                                  0xC4);  // Enable EQ from I2C register,squelch enabled
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x0C, 0x55);  // HBR RBR EQ
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x0D, 0x85);  // HBR2 EQ
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x0E, 0x05);  // HBR3 EQ
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x9A,
                                  0x88);  // L1_VOD L1_PRE L0_VOD L0_PRE
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0x9B,
                                  0x88);  // L3_VOD L3_PRE L2_VOD L2_PRE
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, 0x18 >> 1, 0xA4,
                                  0x08);  // Full Jitter cleaning mode
#endif

#if (BITEC_DP_CARD_REV == 11 || BITEC_DP_CARD_REV == 12)
    // Bitec Daughter Card Rev 11 & 12
    unsigned int data;

    // Init the MCDP6000 on the Bitec Sink main link input
    // (on the Bitec daughter board)
    // Set the MCDP6000 as required by your design

    intel_fpga_i2c_init(I2C_MASTER_BASE, 100000000);

    data = 0x0001704E;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0504, (unsigned char*)&data, 4);
    data = 0x00000601;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x01D8, (unsigned char*)&data, 4);
    data = 0x00005011;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0660, (unsigned char*)&data, 4);
    data = 0x00000001;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x067C, (unsigned char*)&data, 4);
    data = 0x55801E14;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0A00, (unsigned char*)&data,
                            4);  // MC solution #2
    data = 0x0000001F;
    intel_fpga_i2c_mc_write(I2C_MASTER_BASE, 0x28 >> 1, 0x0350, (unsigned char*)&data,
                            4);  // MC solution #3

#endif

#if (BITEC_DP_CARD_REV == 128 && DP_SUPPORT_TX)

printf("\nConfigure Parretto Tx redriver\n");
intel_fpga_i2c_init(I2C_MASTER_BASE, 100000000);
unsigned int tdp2004_data;

tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_ID1_ADDR);
if (tdp2004_data == 0x29) {
    printf("TDP2004 OK\n");
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_GENERAL_REG_ADDR, (0x1 << 6));

    // take TDP2004 out of test mode
    tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_TSTMODE_ALL_ADDR);
    tdp2004_data |= (1<<2);
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_TSTMODE_ALL_ADDR, tdp2004_data);

    // set equalisation to index 7, level 9 = 11dB - leave flat gain at the default -0.6dB
    tdp2004_set_eq(0,7);
    tdp2004_set_eq(1,7);
    tdp2004_set_eq(2,7);
    tdp2004_set_eq(3,7);

} else {
    printf("TDP2004 not found\n");
}

#endif

#if DP_SUPPORT_AXI
	int result;

	#if DP_SUPPORT_TX
    intel_vab_core_base axi2cv_addr_base =
        (intel_vab_core_base)(DP_TX_DP_SOURCE_BASE + (AXI_OFFSET << 2));
    result = intel_axi2cv_init(&axi2cv, axi2cv_addr_base);
    #endif

	#if DP_SUPPORT_RX
		intel_vab_core_base cv2axi_addr_base =
			(intel_vab_core_base)(DP_RX_DP_SINK_BASE + (AXI_OFFSET << 2));
		result = intel_cv2axi_init(&cv2axi, cv2axi_addr_base);
	#endif

#endif

    // Init sink and source

#if  ((DP_SUPPORT_RX && DP_SUPPORT_RX_DSC)  &&  (DP_SUPPORT_TX && DP_SUPPORT_TX_DSC))
   // DSC passthrough
   // Transfer DPCD values from sink to source

   BYTE sink_dpcd_val[6];

   // Read DSC DPCD values from sink
    usleep(50000);
    btc_dptx_aux_read(0, DPCD_ADDR_DSC_SUPPORT,                    16, sink_dpcd_val);
    usleep(50000);

     // Set DSC DPCD values to be read by source
     btc_dprx_dpcd_gpu_access(0,1,DPCD_ADDR_DSC_SUPPORT,16,sink_dpcd_val);

#endif


#if DP_SUPPORT_EDID_PASSTHRU
    bitec_dptx_init();

    // Added 100ms delay //
    {
        unsigned int tout;
        alt_timestamp_start();
        tout = alt_timestamp_freq() / 10;
        while (alt_timestamp() < tout)
            ;
    }
    {
        unsigned int sr;
        sr = IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS);  // Reading SR clears IRQ

        if (sr & 0x04)
        {
            btc_dptx_edid_read(0, tx_edid_data);       // Read the sink EDID
            btc_dprx_edid_set(0, 0, tx_edid_data,
                              tx_edid_data[126] + 1);  // EDID Passthru from Sink to Source
        }
        else
        {
            btc_dprx_edid_set(0, 0, intel_psg_edid, sizeof(intel_psg_edid) / 128);
        }
    }
#else
#if DP_SUPPORT_TX
    bitec_dptx_init();
#endif
    if (BITEC_RX_CAPAB_MST && BITEC_TX_CAPAB_MST)
    {
        for (unsigned int k = 0; k < MST_RX_STREAMS; ++k)
        {
            btc_dprx_edid_set(0, k, intel_psg_edid, sizeof(intel_psg_edid) / 128);  // strmk
        }
    }
    else
    {
        btc_dprx_edid_set(0, 0, intel_psg_edid, sizeof(intel_psg_edid) / 128);
    }
#endif

#if DP_SUPPORT_RX && BITEC_RX_GPUMODE
    btc_dprx_hpd_set(0, 0);  // HPD = 0

    bitec_dprx_init();

    BTC_DPRX_ENABLE_IRQ(0);  // Enable IRQ on AUX Requests from the source

    // Wait for 500 ms to have a long HPD
    {
        unsigned int tout;
        alt_timestamp_start();
        tout = alt_timestamp_freq() / 2;
        while (alt_timestamp() < tout)
            ;
    }
    btc_dprx_hpd_set(0, 1);  // HPD = 1
#endif

#if DP_SUPPORT_TX && DP_SUPPORT_RX
    for (unsigned int k = 0; k < MST_RX_STREAMS; ++k)
    {
        // Mark all Rx sinks as off
        sink_status[k] = 0;
    }
    for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
    {
        // Mark all Tx sources as off
        source_status[k] = 0;
    }
#endif


#if DP_SUPPORT_EDID_PASSTHRU
    BYTE d;
    // Propagate fallback formats
    btc_dptx_aux_read(0, DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS, 1, &d);
    btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS, 1, &d);
    //printf("Read 0x%0x from DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS\n" , d);
#endif

#if DP_SUPPORT_TX
    // Check if a Sink is readily connected
    {
        unsigned int sr;
        sr = IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS);  // Reading SR clears IRQ

        if (sr & 0x04)
        {
#if BITEC_TX_CAPAB_MST_OR_DP20
            pc_fsm = PC_FSM_HPD_1;
#else
            btc_dptx_hpd_change(0, 1);
#endif
        }
    }

    BTC_DPTX_ENABLE_HPD_IRQ(0);  // Enable IRQ on HPD changes from the sink
#endif

#if (DP_DSC_TX_ONLY)
    // Set a default non-DSC mode before the DSC data is loaded
    select_resolutions(0,mode_param_array, &dsc_data_pps_parameters, &mode_command);
#endif


#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG || BITEC_STATUS_DEBUG
    printf("Welcome to Altera DisplayPort Nios console...\nPress 'h' for help and UI menu, 's' for status\n");
#endif

    // Main loop
    while (1)
    {
#ifdef ALT_VIP_MIX_0_BASE
        if (IORD(btc_dprx_baseaddr(0), DPRX0_REG_VBID) & 0x80)
            IOWR(ALT_VIP_MIX_0_BASE, 4, 1);  // MSA lock -> Enable DP image
        else
            IOWR(ALT_VIP_MIX_0_BASE, 4, 0);  // MSA not locked -> Disable DP image

        if (IORD(btc_dprx_baseaddr(0), DPRX1_REG_VBID) & 0x80)
            IOWR(ALT_VIP_MIX_0_BASE, 7, 1);  // MSA lock -> Enable DP image
        else
            IOWR(ALT_VIP_MIX_0_BASE, 7, 0);  // MSA not locked -> Disable DP image

        if (IORD(btc_dprx_baseaddr(0), DPRX2_REG_VBID) & 0x80)
            IOWR(ALT_VIP_MIX_0_BASE, 10, 1);  // MSA lock -> Enable DP image
        else
            IOWR(ALT_VIP_MIX_0_BASE, 10, 0);  // MSA not locked -> Disable DP image

        if (IORD(btc_dprx_baseaddr(0), DPRX3_REG_VBID) & 0x80)
            IOWR(ALT_VIP_MIX_0_BASE, 13, 1);  // MSA lock -> Enable DP image
        else
            IOWR(ALT_VIP_MIX_0_BASE, 13, 0);  // MSA not locked -> Disable DP image
#endif

// "Stream management" for loopback mode
#if DP_SUPPORT_TX && DP_SUPPORT_RX
        {
            unsigned int rx_stream_offset = DPRX1_REG_VBID - DPRX0_REG_VBID;
            unsigned int prx = btc_dprx_baseaddr(0);
            for (unsigned int k = 0; k < MST_RX_STREAMS; ++k)
            {
                BYTE msa_lock = (IORD(prx, DPRX0_REG_VBID + k * rx_stream_offset) >> 7) & 1;
                #if BITEC_TX_CAPAB_MST_OR_DP20
                // When using the MST/DP2.0 state machine, delay the handling of a change of msa_lock status until we can safely update stream status
                if ((pc_fsm == PC_FSM_IDLE) || (pc_fsm == PC_FSM_HPD_0) || (pc_fsm == PC_FSM_HPD_1) || (pc_fsm == PC_FSM_MST_ON) || (pc_fsm == PC_FSM_NOOUT))
                {
                #endif
                if (sink_status[k] != msa_lock)
                {
                    sink_status[k] = msa_lock;
                    #if DEBUG_PRINT_ENABLED
                        unsigned int rx_mvid = IORD(prx, DPRX0_REG_MSA_MVID + rx_stream_offset*k) & 0xFFFFFF;
                        unsigned int rx_nvid = IORD(prx, DPRX0_REG_MSA_NVID + rx_stream_offset*k) & 0xFFFFFF;
                        DBG_PRINTF("Sink_status[%d] state change -> %d, rx_mvid=0x%x, rx_nvid=0x%x\n", k, msa_lock, rx_mvid, rx_nvid);
                    #endif
                    // Currently the expectation for this example design is MST_RX_STREAMS == MST_TX_STREAMS and there is a 1:1 mapping input->output
                    if (msa_lock)
                    {
                        if (k < MST_TX_STREAMS)
                        {
                            stream_passthrough(k, k);
                            #if BITEC_TX_CAPAB_MST_OR_DP20
                                // Redo the allocation
                                if ((pc_fsm == PC_FSM_MST_ON) || (pc_fsm == PC_FSM_NOOUT))
                                {
                                    DBG_PRINTF("DEBUG: restarting allocation\n");
                                    pc_fsm = PC_FSM_CLEAR_ALLOCATION;
                                }
                                else
                                {
                                    DBG_PRINTF("DEBUG: source busy, no allocation\n");
                                }
                            #endif
                        }
                    }
                }
                #if BITEC_TX_CAPAB_MST_OR_DP20
                }
                #endif
            }
        }
        {
            unsigned int tx_stream_offset = DPTX1_REG_VBID - DPTX0_REG_VBID;
            unsigned int ptx = btc_dptx_baseaddr(0);
            for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
            {
                BYTE msa_lock = (IORD(ptx, DPTX0_REG_VBID + k * tx_stream_offset) >> 7) & 1;
                if (source_status[k] != msa_lock)
                {
                    source_status[k] = msa_lock;
                    #if DEBUG_PRINT_ENABLED
                        unsigned int tx_mvid = IORD(ptx, DPTX0_REG_MSA_MVID + tx_stream_offset*k) & 0xFFFFFF;
                        unsigned int tx_nvid = IORD(ptx, DPTX0_REG_MSA_NVID + tx_stream_offset*k) & 0xFFFFFF;
                        DBG_PRINTF("Source_status[%d] state change -> %d, tx_mvid=0x%x, tx_nvid=0x%x\n", k, msa_lock, tx_mvid, tx_nvid);
                    #endif
                    #if BITEC_TX_CAPAB_MST_OR_DP20
                        // Redo the allocation
                        if (msa_lock && (pc_fsm == PC_FSM_NOOUT))
                        {
                            DBG_PRINTF("DEBUG: restarting allocation\n");
                            pc_fsm = PC_FSM_CLEAR_ALLOCATION;
                        }
                    #endif
                }
            }
        }
#endif
// "Stream management" for Tx design
#if DP_SUPPORT_TX && (DP_SUPPORT_RX == 0)
    // Tx-only design set colorimetry, pixel rate, vfreq for TX-only design (critical for proper allocation in DP2.0)
    {
        unsigned int ptx;
        BYTE tx_ch_coding;
        unsigned int vfreq_k;

        ptx = btc_dptx_baseaddr(0);

        // No RX. Set colorimetry for Tx-only example design
        {
            unsigned int current_color = IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_COLOUR) & 0xFFFF;
            unsigned int tx_color_format = 0; // RGB
            unsigned int tx_color_bpc = DP_TX_BPS;
            unsigned int tx_color_range = 0; //VESA
            unsigned int tx_color_colorimetry = 0; // RGB
            unsigned int tx_color_use_vsc_sdp = 0; // use misc0
            unsigned int tx_color = (tx_color_use_vsc_sdp << 13) | (tx_color_range << 12) | (tx_color_colorimetry << 8)  | (tx_color_format << 4) | tx_color_bpc;

            if (current_color != tx_color)
            {
                #if BITEC_TX_CAPAB_MST_OR_DP20
                btc_dptxll_stream_set_color_space(0, 0, tx_color_format, tx_color_bpc, tx_color_range,
                                             tx_color_colorimetry, tx_color_use_vsc_sdp);
                #else
                btc_dptx_set_color_space(0, tx_color_format, tx_color_bpc, tx_color_range,
                                         tx_color_colorimetry, tx_color_use_vsc_sdp);
                #endif
            }
        }

        tx_ch_coding = (IORD(ptx, DPTX_REG_TX_CONTROL) >> 10) & 0x03;
        vfreq_k = 37125*DP_TX_PIP;  // 1080p for the only stream of the current Tx-only design)
        for (unsigned int tx_stream = 0; tx_stream < MST_TX_STREAMS; ++tx_stream)  // MST_TX_STREAMS==1 for now
        {
            #if BITEC_TX_CAPAB_MST_OR_DP20
            btc_dptxll_stream_set_pixel_rate(0, tx_stream, vfreq_k);
            if (tx_ch_coding == 2)
            {
                // Set vfreq on DPTX
                dp2p0_dptxll_stream_set_vfreq(0, tx_stream, vfreq_k);
            }
            #endif
        }
    }
#endif

        // Serve Syslib periodic tasks
#if DP_SUPPORT_TX
        btc_dptx_syslib_monitor();
#if BITEC_TX_CAPAB_MST_OR_DP20
        btc_dptxll_syslib_monitor();
#endif
#endif

#if DP_SUPPORT_RX && BITEC_RX_GPUMODE
        btc_dprx_syslib_monitor();
#if (BITEC_DP_CARD_REV == 11 || BITEC_DP_CARD_REV == 12 )
        bitec_mc_monitor();
#endif
#endif

#if DP_SUPPORT_TX
        // Simulate the user TX application
        bitec_dptx_pc();
#endif

       // User interface if implemented
#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG || BITEC_STATUS_DEBUG
#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG
        // Dump AUX channel traffic
        dump_aux_traffic();
#endif        
        // Serve menu commands
        menu_cmd();
#endif

#if DP_SUPPORT_TX_HDCP
        hdcp_main();
        unsigned int enabled = get_hdcp_enabled();
        if (previous_hdcpenabled != enabled)
        {
            btc_dptx_mst_hdcp_encryption_enable(0, 0, enabled);
            btc_dptx_mst_hdcp_encryption_enable(0, 1, enabled);
            btc_dptx_mst_hdcp_encryption_enable(0, 2, enabled);
            btc_dptx_mst_hdcp_encryption_enable(0, 3, enabled);
        }
        previous_hdcpenabled = enabled;

#endif

#if DP_SUPPORT_EDID_PASSTHRU
        // If new Sink detected, pass-thru the EDID from Sink to Source
        if (new_rx)
        {
            new_rx = 0;

            btc_dprx_hpd_set(0, 0);  // HPD = 0

            // Init the EDID(s)
            btc_dptx_edid_read(0, tx_edid_data);  // Read the sink EDID
            btc_dprx_edid_set(0, 0, tx_edid_data, tx_edid_data[126] + 1);

            BYTE d;
            // Propagate fallback formats
            btc_dptx_aux_read(0, DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS, 1, &d);
            btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS, 1, &d);
            //printf("Read 0x%0x from DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS\n" , d);

            // Wait for 500 ms to have a long HPD
            {
                unsigned int tout;
                alt_timestamp_start();
                tout = alt_timestamp_freq() / 2;
                while (alt_timestamp() < tout)
                    ;
            }
            btc_dprx_hpd_set(0, 1);  // HPD = 1
        }

#endif


#if DP_SUPPORT_RX
        if (IORD(DP_RX_PIO_0_BASE, 0))
        {
            // User pushbutton pressed
#if 0
#if BITEC_RX_GPUMODE
      btc_dprx_hpd_set(0,0); // HPD = 0

      // Wait for 500 ms to have a long HPD
      {
        unsigned int tout;
        alt_timestamp_start();
        tout = alt_timestamp_freq()/2;
        while(alt_timestamp() < tout);
      }
      btc_dprx_hpd_set(0,1); // HPD = 1
#endif
#else

            // Wait for 500 ms to avoid bouncing
            {
                unsigned int tout;
                alt_timestamp_start();
                tout = alt_timestamp_freq() / 2;
                while (alt_timestamp() < tout)
                    ;
            }

#endif
        }
#endif

#if DP_SUPPORT_AXI
	#if (DP_SUPPORT_RX && DP_SUPPORT_TX)
        
        // DP Tx MVC
        dp_tx_actv_dim_x = IORD(DP_TX_DP_SOURCE_BASE, 0x355) & 0xFFFF;
        dp_tx_actv_dim_y = IORD(DP_TX_DP_SOURCE_BASE, 0x356) & 0xFFFF;

        dp_tx_actv_dim  = ( (dp_tx_fps << 24) | (dp_tx_actv_dim_x << 12) | (dp_tx_actv_dim_y));
        
        // PIO Output to VVP_SS
        IOWR(DPTX_PIO_ACTV_DIM_BASE, 0x0, dp_tx_actv_dim);

        // PIO Input from VVP_SS
        dp_tx_new_fps   = (IORD(DPTX_PIO_FPS_BASE, 0x0)) & (0xFF);  
        dp_tx_new_dim_x = (IORD(DPTX_PIO_NEW_ACTV_DIM_BASE, 0x0) >> 16) & (0xFFFF);        
        dp_tx_new_dim_y = (IORD(DPTX_PIO_NEW_ACTV_DIM_BASE, 0x0)) & (0xFFFF);
        
        // DP Rx CVI loop MVC
        if (res_switch == 0 && (intel_cv2axi_is_locked(&cv2axi) == 0 ||
                                intel_cv2axi_is_valid_resolution(&cv2axi) == 0 ||
                                intel_cv2axi_is_stream_stable(&cv2axi) == 0))
        {
            res_switch = 1;
            intel_cv2axi_stop(&cv2axi);
        }
        if (res_switch == 1 && intel_cv2axi_is_stream_stable(&cv2axi) == 1 &&
            intel_cv2axi_is_valid_resolution(&cv2axi) == 1 && intel_cv2axi_is_locked(&cv2axi) == 1)
        {
            res_switch = 0;
            cvi_res_switch(&cv2axi, &cvi_image_config);
        }
        
        // DP Tx CVO loop MVC  
        if (res_switch_tx == 0 && ( (dp_tx_new_fps   != dp_tx_fps) ||
                                    (dp_tx_new_dim_x != dp_tx_actv_dim_x) ||
                                    (dp_tx_new_dim_y != dp_tx_actv_dim_y) ))
        {
            res_switch_tx = 1;
        }
        if ( (res_switch_tx == 1) )
        {
            res_switch_tx = 0;
            cvo_res_switch(&axi2cv, dp_tx_new_dim_x, dp_tx_new_dim_y, dp_tx_new_fps);
            dp_tx_fps = dp_tx_new_fps;
        }
        
        // prev_dp_tx_new_fps    = dp_tx_new_fps;
        // prev_dp_tx_new_dim_x  = dp_tx_new_dim_x;
        // prev_dp_tx_new_dim_y  = dp_tx_new_dim_y;        
        
	#else
    #if (DP_SUPPORT_TX && (DP_SUPPORT_TX_DSC == 0))
		if (res_switch == 1)
		{
			cvo_program_mode_bank(&axi2cv, 3);
			res_switch = 0;
			//TPG initial set up
		    IOWR(VVP_TPG_BASE, 72, 1920);
		    IOWR(VVP_TPG_BASE, 73, 1080);
		    IOWR(VVP_TPG_BASE, 74, 3);   //set to progressive
		    IOWR(VVP_TPG_BASE, 84, 0);   //core select
		    IOWR(VVP_TPG_BASE, 85, 0);   //pre user
		    IOWR(VVP_TPG_BASE, 86, 0);   //post user
		    IOWR(VVP_TPG_BASE, 87, 0);   //output colors
		    IOWR(VVP_TPG_BASE, 88, 0);
		    IOWR(VVP_TPG_BASE, 89, 0);
		    IOWR(VVP_TPG_BASE, 90, 0);   //set bars to zero
		    IOWR(VVP_TPG_BASE, 83, 1);   //commit
		    IOWR(VVP_TPG_BASE, 82, 1);   //start
		    printf("Programmed TPG\n");
		}
#endif
	#endif
#endif
    }

    return 0;  // Should never get here
}



#if DP_SUPPORT_AXI
#if DP_SUPPORT_TX
//==================================================================
//--Program the supported resolution to the cvo mode bank
//==================================================================
void cvo_program_mode_bank(intel_axi2cv_instance* cvo, unsigned int sel)
{
    switch (sel)
    {
    case 1:
        intel_axi2cv_set_output_mode(cvo, 0, CVO_720P_MODE);
        break;
    case 3:
        intel_axi2cv_set_output_mode(cvo, 0, CVO_1080P_MODE);
        break;
    case 4:
        intel_axi2cv_set_output_mode(cvo, 0, CVO_2160P_MODE);
        break;
    default:
        intel_axi2cv_set_output_mode(cvo, 0, CVO_1080P_MODE);
        break;

    // case 0:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_480P_MODE);
        // break;
    // case 1:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_720P_MODE);
        // break;
    // case 2:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_1080I_MODE);
        // break;
    // case 3:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_1080P_MODE);
        // break;
    // case 4:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_2160P_MODE);
        // break;
    // case 5:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_2160P_420_MODE);
        // break;
    // case 6:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_4320P_MODE);
        // break;
    // case 7:
        // intel_axi2cv_set_output_mode(cvo, 0, CVO_4320P_420_MODE);
        // break;
    // default:
        // break;
    }
}
#endif

#if DP_SUPPORT_RX
//==================================================================
// Resolution Switch
//==================================================================
// MVC
void cvi_res_switch(intel_cv2axi_instance* cvi,
                    struct image_config* current_image_config)
{
    struct image_config new_image_config;
    int res_changed = 0;

    new_image_config.x = intel_cv2axi_get_active_sample_count(cvi);   //-- Width
    new_image_config.y = intel_cv2axi_get_active_line_count_f0(cvi);  //-- Using f0 height only

    new_image_config.interlace = intel_cv2axi_is_interlaced(cvi);
    new_image_config.image_color.space = intel_cv2axi_get_color_pattern(cvi) >> 7;
    new_image_config.image_color.depth = intel_cv2axi_get_bit_width(cvi);

    if (new_image_config.interlace)
    {
        new_image_config.y += intel_cv2axi_get_active_line_count_f1(cvi);  // height is f0 + f1
    }

    // Compare current_image_config and new_image_config
    if (new_image_config.x != current_image_config->x ||
        new_image_config.y != current_image_config->y ||
        new_image_config.interlace != current_image_config->interlace)
    {
        res_changed = 1;
    }

    if (res_changed)
    {
        current_image_config->x = new_image_config.x;
        current_image_config->y = new_image_config.y;
        current_image_config->interlace = new_image_config.interlace;
        current_image_config->image_color = new_image_config.image_color;
    }

    intel_cv2axi_start(cvi);
}

// MVC
void cvo_res_switch(intel_axi2cv_instance* cvo, unsigned int dim_x, unsigned int dim_y, unsigned int fps_val)
{

    // Clk           Out2                    Out3
    // Resolution    Pix clk  Pix_Clk/Pip   CVO clk
    // 480p60 -       27.00    27.00 / 2    13.5000 CLK_13_5
    // 720p60 -       74.25    74.25 / 2    37.1250 CLK_37_125
    // 1080i60 -      74.25    74.25 / 2    37.1250 CLK_37_125
    // 1080p60 -      148.5   148.50 / 2    74.2500 CLK_74_25
    // 2160p30 -      297.0   297.00 / 2    148.500 CLK_148_5
    // 2160p60 -      148.5   594.00 / 2    297.000 CLK_297

    switch (dim_y)
    {
        case 720:
            cvo_program_mode_bank(cvo, 1);
            // tx_clk_freq(CLK_18_5625); // 4PIP
            tx_clk_freq(CLK_37_125); // 2PIP
        break;
        case 1080:
            cvo_program_mode_bank(cvo, 3);
            // tx_clk_freq(CLK_37_125); // 4PIP
            tx_clk_freq(CLK_74_25); // 2PIP
        break;
        case 2160:
            cvo_program_mode_bank(cvo, 4);
            // tx_clk_freq(CLK_148_5); // 4PIP
            if (fps_val == 30) {            
                tx_clk_freq(CLK_148_5); // 2PIP for 30fps
            }
            else {            
                tx_clk_freq(CLK_297); // 2PIP for 60fps
            }
        break;           
        default:
            cvo_program_mode_bank(cvo, 3);
            // tx_clk_freq(CLK_37_125); // 4PIP
            tx_clk_freq(CLK_74_25); // 2PIP        
        break;
    }

}
#endif

#if (DP_SUPPORT_RX && DP_SUPPORT_TX)
//==================================================================
// Tx Video Clock Programming
//==================================================================
void tx_clk_freq(TX_CLK_FREQ clk_freq)
{
    // Clk           Out2                    Out3
    // Resolution    Pix clk  Pix_Clk/Pip   CVO clk
    // 480p60 -       27       27.00 / 4    6.75
    // 720p60 -       74.25    74.25 / 4    9.28125
    // 1080i60 -      74.25    74.25 / 4    9.28125
    // 1080p60 -      148.5   148.50 / 4   18.5625
    // 2160p60 -      148.5   297.00 / 4   74.25
    // 2160p30 -      297.0   594.00 / 4   37.125

    // MVC
    // Clk           Out2                    Out3
    // Resolution    Pix clk  Pix_Clk/Pip   CVO clk
    // 480p60 -       27.00    27.00 / 2    13.5000 CLK_13_5
    // 720p60 -       74.25    74.25 / 2    37.1250 CLK_37_125
    // 1080i60 -      74.25    74.25 / 2    37.1250 CLK_37_125
    // 1080p60 -      148.5   148.50 / 2    74.2500 CLK_74_25
    // 2160p30 -      297.0   297.00 / 2    148.500 CLK_148_5
    // 2160p60 -      148.5   594.00 / 2    297.000 CLK_297

    switch (clk_freq)
    {
    case CLK_6_75: {
        board_tx_freq(6750000);
        break;
    }
    case CLK_9_281: {
        board_tx_freq(9281250);
        break;
    }
    case CLK_18_5625: {
        board_tx_freq(18562500);
        break;
    }
    case CLK_37_125: {
        board_tx_freq(37125000);
        break;
    }
    case CLK_74_25: {
        board_tx_freq(74250000);
        break;
    }
    case CLK_148_5: {
        board_tx_freq(148500000);
        break;
    }
    case CLK_3_375: {
        board_tx_freq(3375000);
        break;
    }
    case CLK_297: {
        board_tx_freq(297000000);
        break;
    }
    default: {
        break;
    }
    }
}
#endif

#endif // DP_SUPPORT_AXI


#if (DP_SUPPORT_RX && DP_SUPPORT_TX)

//=================================================================================================
// Returns VFREQ in kHz unit given values for mvid, nvid and link_rate (at 8b10b rates)
//=================================================================================================
unsigned int get_vfreq_from_mvid_nvid(unsigned int mvid, unsigned int nvid, int link_rate)
{
    unsigned int tx_vfreq_num;
    unsigned int tx_vfreq_k;

    // mvid/nvid is the ratio between pixel clock and link symbol clock
    // The link_rate (0x06 RBR, 0x1E HBR3) is given as a multiple of 270Mbps (per lane).
    //            link_rate * 270 * 10^6 is the number of bits transmitted per seconds per lane
    // A link symbol is 8bits, taking the 8b10 encoding into account, a symbol is transmitted over 10b
    //            link_rate * 27 * 10^6 is the number of link symbol transmitted per seconds per lane
    // We can derive the number of pixels per second: (mvid/nvid) * link_rate * 27 * 10^6
    // vfreq is given in kpps: vfreq = (mvid/nvid) * link_rate * 27 * 10^3

    // Compute vfreq within the limits of 32 bits integer mathematics (mvid is 16 bits at most but 27*link_rate*1000 could be 17 bits)
    tx_vfreq_num = mvid * 27 * link_rate * 100;
    tx_vfreq_k = (tx_vfreq_num / nvid) * 10;

    return tx_vfreq_k;
}
#endif

