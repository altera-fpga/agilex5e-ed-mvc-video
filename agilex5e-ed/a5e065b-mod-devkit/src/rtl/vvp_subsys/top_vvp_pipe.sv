// (C) 2001-2025 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// *********************************************************************
// Description
//
// DisplayPort Design Example Top-level module
//
// *********************************************************************

module top_vvp_pipe
(
    //Clocks Inputs
    input  logic                        core_refclk_100,            // 100Mhz
    input  logic                        clk_300,
    input  logic                        sys_resetn,
    input  logic                        cold_resetn,
    output logic                        cal_done_rst_n,
    output logic                        bank_2b_cal_done_rst_n,
    output logic                        bank_3a_cal_done_rst_n,
    input  logic                        emif_core_init_n_i,

    input  logic [23:0]                 clk_dbg_00,
    input  logic [23:0]                 clk_dbg_01,
    input  logic [23:0]                 clk_dbg_02,
    input  logic [23:0]                 clk_dbg_03,
    input  logic [23:0]                 clk_dbg_04,
    input  logic [23:0]                 clk_dbg_05,
    input  logic [23:0]                 clk_dbg_06,
    input  logic [23:0]                 clk_dbg_07,
    input  logic [23:0]                 clk_dbg_08,
    input  logic [23:0]                 clk_dbg_09,

    input  logic [31:0]                 dp_tx_actv_dim_i,
    output logic [31:0]                 dp_tx_new_actv_dim_o,
    output logic [ 7:0]                 dp_tx_fps_o,    

    input  logic [31:0]                 hdmi_tx_actv_dim_i,
    output logic [31:0]                 hdmi_tx_new_actv_dim_o,
    output logic [ 7:0]                 hdmi_tx_fps_o, 
    
    input  logic [31:0]                 sdi_tx_actv_dim_i,
    output logic [31:0]                 sdi_tx_new_actv_dim_o,
    output logic [ 7:0]                 sdi_tx_fps_o,  

    // output logic [31:0]                 new_actv_vid_hdmi_tx_pio,
    // output logic [31:0]                 new_actv_vid_sdi_tx_pio,    
    // output logic [31:0]                 dp_tx_new_actv_dim_o, 
    // input  logic [31:0]                 dp_tx_actv_dim_i,           

    // HDMI Rx Port   
    input  logic [63:0]                 hdmi_rx_tdata,    //        hdmi_rx_dp_sink_rx_axi4s_vid_out.tdata
    input  logic [7:0]                  hdmi_rx_tuser,    //                                      .tuser
    input  logic                        hdmi_rx_tvalid,   //                                      .tvalid
    output logic                        hdmi_rx_tready,   //                                      .tready
    input  logic                        hdmi_rx_tlast,    //                                      .tlast
    
    // HDMI Tx Port   
    output logic [63:0]                 hdmi_tx_tdata,   //       hdmi_tx_dp_source_tx_axi4s_vid_in.tdata
    output logic [7:0]                  hdmi_tx_tuser,   //                                      .tuser
    output logic                        hdmi_tx_tvalid,  //                                      .tvalid
    input  logic                        hdmi_tx_tready,  //                                      .tready
    output logic                        hdmi_tx_tlast,   //                                      .tlast

    // DP Rx Port   
    input  logic [63:0]                 dp_rx_tdata,    //        dp_rx_dp_sink_rx_axi4s_vid_out.tdata
    input  logic [7:0]                  dp_rx_tuser,    //                                      .tuser
    input  logic                        dp_rx_tvalid,   //                                      .tvalid
    output logic                        dp_rx_tready,   //                                      .tready
    input  logic                        dp_rx_tlast,    //                                      .tlast
    
    // DP Tx Port   
    output logic [63:0]                 dp_tx_tdata,   //       dp_tx_dp_source_tx_axi4s_vid_in.tdata
    output logic [7:0]                  dp_tx_tuser,   //                                      .tuser
    output logic                        dp_tx_tvalid,  //                                      .tvalid
    input  logic                        dp_tx_tready,  //                                      .tready
    output logic                        dp_tx_tlast,   //                                      .tlast

    // SDI Rx Port   
    input  logic [63:0]                 sdi_rx_tdata,    //        dp_rx_dp_sink_rx_axi4s_vid_out.tdata
    input  logic [7:0]                  sdi_rx_tuser,    //                                      .tuser
    input  logic                        sdi_rx_tvalid,   //                                      .tvalid
    output logic                        sdi_rx_tready,   //                                      .tready
    input  logic                        sdi_rx_tlast,    //                                      .tlast
    
    // SDI Tx Port   
    output logic [63:0]                 sdi_tx_tdata,   //       dp_tx_dp_source_tx_axi4s_vid_in.tdata
    output logic [7:0]                  sdi_tx_tuser,   //                                      .tuser
    output logic                        sdi_tx_tvalid,  //                                      .tvalid
    input  logic                        sdi_tx_tready,  //                                      .tready
    output logic                        sdi_tx_tlast,   //                                      .tlast

    //DDR4 Bank 3B: VVP Channel # 1
    input  logic                        mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                        mem_oct_rzqin,
    output logic [0:0]                  mem_ck,
    output logic [0:0]                  mem_ck_n,
    output logic [16:0]                 mem_a,
    output logic [0:0]                  mem_act_n,
    output logic [1:0]                  mem_ba,
    output logic [1:0]                  mem_bg,
    output logic [0:0]                  mem_cke,
    output logic [0:0]                  mem_cs_n,
    output logic [0:0]                  mem_odt,
    output logic [0:0]                  mem_reset_n,
    output logic [0:0]                  mem_par,
    input  logic [0:0]                  mem_alert_n,
    inout  logic [4:0]                  mem_dqs,
    inout  logic [4:0]                  mem_dqs_n,
    inout  logic [4:0]                  mem_dbi_n,
    inout  logic [39:0]                 mem_dq, 
    
    //DDR4 Bank 2B: VVP Channel # 2
    input  logic                        bank_2b_mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                        bank_2b_mem_oct_rzqin,
    output logic [0:0]                  bank_2b_mem_ck,
    output logic [0:0]                  bank_2b_mem_ck_n,
    output logic [16:0]                 bank_2b_mem_a,
    output logic [0:0]                  bank_2b_mem_act_n,
    output logic [1:0]                  bank_2b_mem_ba,
    output logic [1:0]                  bank_2b_mem_bg,
    output logic [0:0]                  bank_2b_mem_cke,
    output logic [0:0]                  bank_2b_mem_cs_n,
    output logic [0:0]                  bank_2b_mem_odt,
    output logic [0:0]                  bank_2b_mem_reset_n,
    output logic [0:0]                  bank_2b_mem_par,
    input  logic [0:0]                  bank_2b_mem_alert_n,
    inout  logic [3:0]                  bank_2b_mem_dqs,
    inout  logic [3:0]                  bank_2b_mem_dqs_n,
    inout  logic [3:0]                  bank_2b_mem_dbi_n,
    inout  logic [31:0]                 bank_2b_mem_dq,
 
    //DDR4 Bank 3A: VVP Channel # 3
    input  logic                        bank_3a_mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                        bank_3a_mem_oct_rzqin,
    output logic [0:0]                  bank_3a_mem_ck,
    output logic [0:0]                  bank_3a_mem_ck_n,
    output logic [16:0]                 bank_3a_mem_a,
    output logic [0:0]                  bank_3a_mem_act_n,
    output logic [1:0]                  bank_3a_mem_ba,
    output logic [1:0]                  bank_3a_mem_bg,
    output logic [0:0]                  bank_3a_mem_cke,
    output logic [0:0]                  bank_3a_mem_cs_n,
    output logic [0:0]                  bank_3a_mem_odt,
    output logic [0:0]                  bank_3a_mem_reset_n,
    output logic [0:0]                  bank_3a_mem_par,
    input  logic [0:0]                  bank_3a_mem_alert_n,
    inout  logic [4:0]                  bank_3a_mem_dqs,
    inout  logic [4:0]                  bank_3a_mem_dqs_n,
    inout  logic [4:0]                  bank_3a_mem_dbi_n,
    inout  logic [39:0]                 bank_3a_mem_dq
);

// -------------------------------------------------------------------------
// DisplayPort Core Qsys system
// -------------------------------------------------------------------------
  vvp_pipe i_vvp_pipe (
    // -------------------------------------------------------------------
    // Clocks and Resets
    // -------------------------------------------------------------------
    .mgmt_clk_in_clk_clk                                        (core_refclk_100),
    .cpu_reset_bridge_in_reset_reset_n                          (sys_resetn),
    .vvp_pipe_axis_clk_in_clk_clk                               (clk_300),
    .vvp_pipe_axis_reset_bridge_in_reset_reset_n                (sys_resetn),

    // PIO
    .vvp_pipe_actv_dim_hdmitx_external_connection_export        (hdmi_tx_actv_dim_i),
    .vvp_pipe_new_actv_dim_hdmitx_external_connection_export    (hdmi_tx_new_actv_dim_o),
    .vvp_pipe_new_fps_hdmitx_external_connection_export         (hdmi_tx_fps_o),

    .vvp_pipe_actv_dim_dptx_external_connection_export          (dp_tx_actv_dim_i),
    .vvp_pipe_new_actv_dim_dptx_external_connection_export      (dp_tx_new_actv_dim_o),
    .vvp_pipe_new_fps_dpitx_external_connection_export          (dp_tx_fps_o),

    .vvp_pipe_actv_dim_sditx_external_connection_export         (sdi_tx_actv_dim_i),
    .vvp_pipe_new_actv_dim_sditx_external_connection_export     (sdi_tx_new_actv_dim_o),
    .vvp_pipe_new_fps_sditx_external_connection_export          (sdi_tx_fps_o),
    
    // DBG CLKs
    .vvp_clk_dbg_00_external_connection_export                   (clk_dbg_00),
    .vvp_clk_dbg_01_external_connection_export                   (clk_dbg_01),
    .vvp_clk_dbg_02_external_connection_export                   (clk_dbg_02),
    .vvp_clk_dbg_03_external_connection_export                   (clk_dbg_03),
    .vvp_clk_dbg_04_external_connection_export                   (clk_dbg_04),
    .vvp_clk_dbg_05_external_connection_export                   (clk_dbg_05),
    .vvp_clk_dbg_06_external_connection_export                   (clk_dbg_06),
    .vvp_clk_dbg_07_external_connection_export                   (clk_dbg_07),
    .vvp_clk_dbg_08_external_connection_export                   (clk_dbg_08),
    .vvp_clk_dbg_09_external_connection_export                   (clk_dbg_09),

    // -------------------------------------------------------------------
    // HDMI Rx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in_tuser             (hdmi_rx_tuser),   //   input,  width = 8,
    .vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in_tdata             (hdmi_rx_tdata),   //   input,  width = 64,
    .vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in_tvalid            (hdmi_rx_tvalid),  //   input,   width = 1,
    .vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in_tready            (hdmi_rx_tready),  //  output,   width = 1,
    .vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in_tlast             (hdmi_rx_tlast),   //   input,   width = 1,
    
    // -------------------------------------------------------------------
    // DisplayPort Rx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in_tuser               (dp_rx_tuser),   //   input,  width = 8,
    .vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in_tdata               (dp_rx_tdata),   //   input,  width = 64,
    .vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in_tvalid              (dp_rx_tvalid),  //   input,   width = 1,
    .vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in_tready              (dp_rx_tready),  //  output,   width = 1,
    .vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in_tlast               (dp_rx_tlast),   //   input,   width = 1,

    // -------------------------------------------------------------------
    // SDI Rx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in_tuser              (sdi_rx_tuser),   //   input,  width = 8,
    .vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in_tdata              (sdi_rx_tdata),   //   input,  width = 64,
    .vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in_tvalid             (sdi_rx_tvalid),  //   input,   width = 1,
    .vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in_tready             (sdi_rx_tready),  //  output,   width = 1,
    .vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in_tlast              (sdi_rx_tlast),   //   input,   width = 1,
		
    // -------------------------------------------------------------------
    // VVP Frame Buffer and EMIF Bank 3B: VVP Channel # 1
    // -------------------------------------------------------------------
    .vvp_pipe_emif_ddr4_core_init_n_reset_n                     (emif_core_init_n_i),
    .vvp_pipe_emif_ddr4_mem_ck_0_mem_ck_t                       (mem_ck),
    .vvp_pipe_emif_ddr4_mem_ck_0_mem_ck_c                       (mem_ck_n),
    .vvp_pipe_emif_ddr4_mem_0_mem_cke                           (mem_cke),
    .vvp_pipe_emif_ddr4_mem_0_mem_odt                           (mem_odt),
    .vvp_pipe_emif_ddr4_mem_0_mem_cs_n                          (mem_cs_n),
    .vvp_pipe_emif_ddr4_mem_0_mem_a                             (mem_a),
    .vvp_pipe_emif_ddr4_mem_0_mem_ba                            (mem_ba),
    .vvp_pipe_emif_ddr4_mem_0_mem_bg                            (mem_bg),
    .vvp_pipe_emif_ddr4_mem_0_mem_act_n                         (mem_act_n),
    .vvp_pipe_emif_ddr4_mem_0_mem_par                           (mem_par),
    .vvp_pipe_emif_ddr4_mem_0_mem_alert_n                       (mem_alert_n),
    .vvp_pipe_emif_ddr4_mem_reset_n_mem_reset_n                 (mem_reset_n),
    .vvp_pipe_emif_ddr4_mem_0_mem_dq                            (mem_dq),
    .vvp_pipe_emif_ddr4_mem_0_mem_dqs_t                         (mem_dqs),
    .vvp_pipe_emif_ddr4_mem_0_mem_dqs_c                         (mem_dqs_n),
    .vvp_pipe_emif_ddr4_mem_0_mem_dbi_n                         (mem_dbi_n),
    .vvp_pipe_emif_ddr4_oct_0_oct_rzqin                         (mem_oct_rzqin),
    .vvp_pipe_emif_ddr4_ref_clk_clk                             (mem_pll_ref_clk),
    .vvp_pipe_emif_ddr4_cal_cal_done_rst_n_reset_n              (cal_done_rst_n),
    .vvp_pipe_emif_reset_bridge_in_reset_reset_n                (cold_resetn),

    // -------------------------------------------------------------------
    // VVP Frame Buffer and EMIF Bank 2B: VVP Channel # 2
    // -------------------------------------------------------------------
    .vvp_pipe_emif_ddr4_ch2_core_init_n_reset_n                 (emif_core_init_n_i),
    .vvp_pipe_emif_ddr4_ch2_mem_ck_0_mem_ck_t                   (bank_2b_mem_ck),
    .vvp_pipe_emif_ddr4_ch2_mem_ck_0_mem_ck_c                   (bank_2b_mem_ck_n),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_cke                       (bank_2b_mem_cke),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_odt                       (bank_2b_mem_odt),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_cs_n                      (bank_2b_mem_cs_n),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_a                         (bank_2b_mem_a),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_ba                        (bank_2b_mem_ba),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_bg                        (bank_2b_mem_bg),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_act_n                     (bank_2b_mem_act_n),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_par                       (bank_2b_mem_par),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_alert_n                   (bank_2b_mem_alert_n),
    .vvp_pipe_emif_ddr4_ch2_mem_reset_n_mem_reset_n             (bank_2b_mem_reset_n),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_dq                        (bank_2b_mem_dq),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_dqs_t                     (bank_2b_mem_dqs),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_dqs_c                     (bank_2b_mem_dqs_n),
    .vvp_pipe_emif_ddr4_ch2_mem_0_mem_dbi_n                     (bank_2b_mem_dbi_n),
    .vvp_pipe_emif_ddr4_ch2_oct_0_oct_rzqin                     (bank_2b_mem_oct_rzqin),
    .vvp_pipe_emif_ddr4_ch2_ref_clk_clk                         (bank_2b_mem_pll_ref_clk),
    .vvp_pipe_emif_ddr4_cal_ch2_cal_done_rst_n_reset_n          (bank_2b_cal_done_rst_n),

    // -------------------------------------------------------------------
    // VVP Frame Buffer and EMIF Bank 3A: VVP Channel # 3
    // -------------------------------------------------------------------
    .vvp_pipe_emif_ddr4_ch3_core_init_n_reset_n                 (emif_core_init_n_i),
    .vvp_pipe_emif_ddr4_ch3_mem_ck_0_mem_ck_t                   (bank_3a_mem_ck),
    .vvp_pipe_emif_ddr4_ch3_mem_ck_0_mem_ck_c                   (bank_3a_mem_ck_n),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_cke                       (bank_3a_mem_cke),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_odt                       (bank_3a_mem_odt),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_cs_n                      (bank_3a_mem_cs_n),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_a                         (bank_3a_mem_a),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_ba                        (bank_3a_mem_ba),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_bg                        (bank_3a_mem_bg),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_act_n                     (bank_3a_mem_act_n),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_par                       (bank_3a_mem_par),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_alert_n                   (bank_3a_mem_alert_n),
    .vvp_pipe_emif_ddr4_ch3_mem_reset_n_mem_reset_n             (bank_3a_mem_reset_n),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_dq                        (bank_3a_mem_dq),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_dqs_t                     (bank_3a_mem_dqs),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_dqs_c                     (bank_3a_mem_dqs_n),
    .vvp_pipe_emif_ddr4_ch3_mem_0_mem_dbi_n                     (bank_3a_mem_dbi_n),
    .vvp_pipe_emif_ddr4_ch3_oct_0_oct_rzqin                     (bank_3a_mem_oct_rzqin),
    .vvp_pipe_emif_ddr4_ch3_ref_clk_clk                         (bank_3a_mem_pll_ref_clk),
    .vvp_pipe_emif_ddr4_cal_ch3_cal_done_rst_n_reset_n          (bank_3a_cal_done_rst_n),

    // -------------------------------------------------------------------
    // HDMI Tx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out_tuser            (hdmi_tx_tuser),  //  output,  width = 8,        
    .vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out_tdata            (hdmi_tx_tdata),  //  output,  width = 64,       
    .vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out_tvalid           (hdmi_tx_tvalid), //  output,   width = 1,       
    .vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out_tready           (hdmi_tx_tready), //  input,   width = 1,       
    .vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out_tlast            (hdmi_tx_tlast),  //  output,   width = 1,       
    
    // -------------------------------------------------------------------
    // DisplayPort Tx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out_tuser              (dp_tx_tuser),  //  output,  width = 8,       
    .vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out_tdata              (dp_tx_tdata),  //  output,  width = 64,      
    .vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out_tvalid             (dp_tx_tvalid), //  output,   width = 1,      
    .vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out_tready             (dp_tx_tready), //  input,   width = 1,      
    .vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out_tlast              (dp_tx_tlast),  //  output,   width = 1,      

    // -------------------------------------------------------------------
    // SDI Tx Interface
    // -------------------------------------------------------------------
    .vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out_tuser             (sdi_tx_tuser),  //  output,  width = 8,      
    .vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out_tdata             (sdi_tx_tdata),  //  output,  width = 64,     
    .vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out_tvalid            (sdi_tx_tvalid), //  output,   width = 1,     
    .vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out_tready            (sdi_tx_tready), //  input,   width = 1,     
    .vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out_tlast             (sdi_tx_tlast)   //  output,   width = 1,      
  );

endmodule








