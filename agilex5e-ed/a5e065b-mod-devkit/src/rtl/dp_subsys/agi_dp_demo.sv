// (C) 2001-2026 Altera Corporation. All rights reserved.
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

// Uncomment to enable debug features.
//`define DEBUG

// Disable FMC I/O Tie Off Warnings.
(* altera_attribute = "-name MESSAGE_DISABLE 13010; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 13008" *)
module agi_dp_demo
#(
    parameter RX_MAX_LANE_COUNT = 4,
    parameter TX_MAX_LANE_COUNT = 4

)
(
    //Clocks Inputs
    input  logic                            cpu_resetn,                 // CPU Reset Pushbutton (TR=0)
    input  logic                            fgt_refclk_150,             // 150Mhz System PLL Reference Clock
    input  logic                            core_refclk_100,            // 100Mhz
    input  logic [0:0]                      user_pb,                     //User Pushbuttons

    output logic                            emif_core_init_n_o,
    output logic                            sysclk_ready_o,
    input  logic                            sysclk_startup_i,

    // To/From HDMI Subsys 
    output logic                            mgmt_reset_hdmi,
    output logic                            cold_reset_hdmi,    
    input  logic [3:0]                      hdmi_rss_req,
    output logic [3:0]                      hdmi_rss_grant,
    output logic                            hdmi_rss_clk,

    output logic                            mgmt_reset_sdi,
    output logic                            cold_reset_sdi,

    // To/From VVP PIPE    
    output logic [31:0]                     dp_tx_actv_dim_o, 
    input  logic [31:0]                     dp_tx_new_actv_dim_i, 
    input  logic [ 7:0]                     dp_tx_fps_i, 

    output logic                            dp_rx_phy_clk_out,
    output logic                            dp_tx_phy_clk_out,
    output logic                            dp_tx_vid_clk_out,
    output logic                            clk_300_out,
    output logic                            sys_resetn_out,
    output logic                            cold_resetn,
    input  logic                            cal_done_rst_n,
    input  logic                            bank_2b_cal_done_rst_n,
    input  logic                            bank_3a_cal_done_rst_n,
    
    output logic [63:0]                     dp_rx_tdata,    //        dp_rx_dp_sink_rx_axi4s_vid_out.tdata
    output logic [7:0]                      dp_rx_tuser,    //                                      .tuser
    output logic                            dp_rx_tvalid,   //                                      .tvalid
    input  logic                            dp_rx_tready,   //                                      .tready
    output logic                            dp_rx_tlast,    //                                      .tlast
    
    input  logic [63:0]                     dp_tx_tdata,   //       dp_tx_dp_source_tx_axi4s_vid_in.tdata
    input  logic [7:0]                      dp_tx_tuser,   //                                      .tuser
    input  logic                            dp_tx_tvalid,  //                                      .tvalid
    output logic                            dp_tx_tready,  //                                      .tready
    input  logic                            dp_tx_tlast,   //                                      .tlast
    
     // board I2C
    inout  logic                             board_i2c_scl,
    inout  logic                             board_i2c_sda,
    
    //FPGA Mezzanine Card (FMC)
    // DP Rx Port
    input  logic [(RX_MAX_LANE_COUNT-1):0]   fmc_rx_p,                  //Transceiver Data FPGA RX
    input  logic [(RX_MAX_LANE_COUNT-1):0]   fmc_rx_n,                  //Transceiver Data FPGA RX
    output logic                             fmc_dp_rx_hpd,             // RX_HPD
    input  logic                             fmc_dp_rx_cable_detect,    // RX Cable Detect
    input  logic                             fmc_dp_rx_pwr_detect,      // RX Power Detect (invert)
    input  logic                             fmc_dp_rx_aux_in,          // RX Aux In
    output logic                             fmc_dp_rx_aux_out,         // RX Aux Out
    output logic                             fmc_dp_rx_aux_oe,          // RX Aux OE
    
    // DP Tx Port
    output logic [(TX_MAX_LANE_COUNT-1):0]   fmc_tx_p,                  //Transceiver Data FPGA TX
    output logic [(TX_MAX_LANE_COUNT-1):0]   fmc_tx_n,                  //Transceiver Data FPGA TX
    output logic                             fmc_dp_tx_cad,             // TX CAD
    input  logic                             fmc_dp_tx_hpd,             // TX HPD
    input  logic                             fmc_dp_tx_aux_in,          // TX Aux In
    output logic                             fmc_dp_tx_aux_out,         // TX Aux Out
    output logic                             fmc_dp_tx_aux_oe           // TX Aux OE
);

//-------------------------------------------------------------------------------
// Local Parameter
//-------------------------------------------------------------------------------
localparam RX_MAX_LINK_RATE     = 30;
localparam RX_SYMBOLS_PER_CLOCK = 4;
localparam RX_PIXELS_PER_CLOCK  = 2;
localparam RX_PMA_WIDTH         = 40;
localparam RX_VIDEO_BPC         = 10;
localparam TX_MAX_LINK_RATE     = 30;
localparam TX_SYMBOLS_PER_CLOCK = 4;
localparam TX_PIXELS_PER_CLOCK  = 2;
localparam TX_PMA_WIDTH         = 40;
localparam TX_VIDEO_BPC         = 10;

//-------------------------------------------------------------------------------
// Signal Declaration
//-------------------------------------------------------------------------------
logic                                               device_ready;
logic                                               sys_reset, sys_resetn;
logic                                               cold_reset;
logic                                               board_i2c_sda_in;
logic                                               board_i2c_scl_in;
logic                                               board_i2c_sda_oe;
logic                                               board_i2c_scl_oe;

//-------------------------------------------------------------------------------
// Rx Signals
//-------------------------------------------------------------------------------
logic                                               dp_rx_hpd;
logic                                               dp_rx_pwr_detect, dp_rx_cable_detect;
logic                                               dp_rx_aux_in, dp_rx_aux_out, dp_rx_aux_oe;
logic                                               dp_rx_vid_clk;
logic [((RX_PIXELS_PER_CLOCK*RX_VIDEO_BPC*3)-1):0]  dp_rx_vid_data;
logic [(RX_PIXELS_PER_CLOCK-1):0]                   dp_rx_vid_valid;
logic                                               dp_rx_vid_locked;
logic                                               dp_rx_vid_sof;
logic                                               dp_rx_vid_eof;
logic                                               dp_rx_vid_sol;
logic                                               dp_rx_vid_eol;
logic [216:0]                                       dp_rx_msa;
logic [7:0]                                         dp_rx_link_rate;
logic                                               dp_rx_restart;
logic                                               dp_rx_rate_rcfg_req;
logic                                               dp_rx_rate_rcfg_ack;
logic                                               dp_rx_rate_rcfg_busy;
logic                                               dp_rx_analog_rcfg_req;
logic                                               gxb_rx_clkout;
logic [(RX_MAX_LANE_COUNT*2)-1:0]                   dp_rx_analog_rcfg_vod;
logic [(RX_MAX_LANE_COUNT*2)-1:0]                   dp_rx_analog_rcfg_emp;
logic [(RX_MAX_LANE_COUNT-1):0]                     dp_rx_is_lockedtoref;
logic [(RX_MAX_LANE_COUNT-1):0]                     dp_rx_is_lockedtodata;
logic [(RX_MAX_LANE_COUNT-1):0]                     dp_rx_cal_busy;
logic [(RX_MAX_LANE_COUNT-1):0]                     dp_rx_set_locktoref;
logic [(RX_MAX_LANE_COUNT-1):0]                     dp_rx_set_locktodata;
logic [4:0]                                         dp_rx_lane_count;
logic [((RX_MAX_LANE_COUNT*RX_PMA_WIDTH)-1):0]      dp_rx_parallel_data;

assign dp_rx_cal_busy = '0;

//-------------------------------------------------------------------------------
// Tx Signals
//-------------------------------------------------------------------------------
logic                                               dp_tx_hpd;
logic                                               dp_tx_aux_in, dp_tx_aux_out, dp_tx_aux_oe;
logic [7:0]                                         dp_tx_link_rate;
logic                                               dp_tx_vid_clk;
logic [((TX_PIXELS_PER_CLOCK*TX_VIDEO_BPC*3)-1):0]  dp_tx_vid_data;
logic [(TX_PIXELS_PER_CLOCK-1):0]                   dp_tx_vid_hsync;
logic [(TX_PIXELS_PER_CLOCK-1):0]                   dp_tx_vid_vsync;
logic [(TX_PIXELS_PER_CLOCK-1):0]                   dp_tx_vid_de;
logic                                               dp_tx_analog_rcfg_req;
logic                                               dp_tx_analog_rcfg_ack;
logic [(TX_MAX_LANE_COUNT*2)-1:0]                   dp_tx_analog_rcfg_vod;
logic [(TX_MAX_LANE_COUNT*2)-1:0]                   dp_tx_analog_rcfg_emp;
logic                                               dp_tx_rate_rcfg_req;
logic                                               dp_tx_rate_rcfg_ack;
logic                                               dp_tx_rate_rcfg_busy;
logic [(TX_MAX_LANE_COUNT-1):0]                     dp_tx_cal_busy;
logic                                               gxb_tx_clkout;
logic                                               dp_txpll_locked;
logic [((TX_MAX_LANE_COUNT*TX_PMA_WIDTH)-1):0]      dp_tx_parallel_data;

assign dp_tx_cal_busy = '0;
assign fmc_dp_tx_cad = 1'b0;
logic [31:0]                                         pio_board;

logic   clk_16;
logic   clk_300;
logic   dp_iopll_locked;
logic   px_iopll_locked;

logic   cal_done_duo_rst_n;

assign clk_300_out = clk_300;
assign cal_done_duo_rst_n = (cal_done_rst_n | bank_2b_cal_done_rst_n | bank_3a_cal_done_rst_n);

// To/From HDMI Subsys 
assign mgmt_reset_hdmi = sys_reset;
assign cold_reset_hdmi = cold_reset;

// To/From SDI Subsys 
assign mgmt_reset_sdi = sys_reset; 
assign cold_reset_sdi = cold_reset; 

//------------------------------------------------------------------------------
// IOPLL
//------------------------------------------------------------------------------
  dp_iopll i_dp_iopll (
      .rst        (cold_reset),
      .refclk     (core_refclk_100),
      .outclk_0   (clk_300),
      .outclk_1   (clk_16),
      .locked     (dp_iopll_locked)
  );

//------------------------------------------------------------------------------
// Tx Pixel IOPLL
//------------------------------------------------------------------------------
logic [7:0] clk_297_cnt,clk_297_set;
logic       clk_297_en;
logic       clk_297;

  dp_pxl_iopll i_dp_pxl_iopll (
      .rst        (cold_reset),
      .refclk     (core_refclk_100),
      .outclk_0   (clk_297),
      .locked     (px_iopll_locked)
  );

  altera_std_synchronizer_bundle #(.depth(3),.width(8)) i_clk_sync (
      .clk        (clk_297),
      .reset_n    (1'b1),
      .din        (pio_board[15:8]),
      .dout       (clk_297_set)
  );

  always @(posedge clk_297) begin
    if (!clk_297_cnt) begin
      clk_297_cnt <= clk_297_set;
      clk_297_en <= 1'b1;
    end else begin
      clk_297_en <= 1'b0;
      clk_297_cnt <= clk_297_cnt - 1'b1;
    end

  end

  //------------------------------------------------------------------------------
  // Clock Gate control
  //------------------------------------------------------------------------------
  dp_clk_ctrl i_dp_clk_ctrl (
        .ena            (clk_297_en),
        .inclk          (clk_297),
        .outclk         (dp_tx_vid_clk)
    );

assign dp_tx_vid_clk_out = dp_tx_vid_clk;
assign dp_rx_phy_clk_out = gxb_rx_clkout;
assign dp_tx_phy_clk_out = gxb_tx_clkout;

//------------------------------------------------------------------------------
// System Reset Controller
//------------------------------------------------------------------------------
    reset_ctrl #(
      .SOURCES           (1),
      .LOCKS             (3)
    ) i_reset_ctrl (
        .mgmt_clk           (core_refclk_100),
        .async_resetn       ({cpu_resetn}),
        .async_locked       ({px_iopll_locked, dp_iopll_locked,cal_done_duo_rst_n}),
        
        .emif_core_init_n   (emif_core_init_n_o),
        .sysclk_startup     (sysclk_startup_i), // Input from SW app to indicate the system_pll Ref clocks clocks are ready
        .sysclk_ready       (sysclk_ready_o), // Output port to the systemPLL to indicate it can start the locking process
        .cold_reset         (cold_reset),
        .cold_resetn        (cold_resetn),
        .mgmt_reset         (sys_reset),
        .mgmt_resetn        (sys_resetn)
    );

assign sys_resetn_out = sys_resetn;

logic [1:0]  rss_clk;
logic dp_rss_clk;

logic [7:0]  rss_req;
logic [7:0]  rss_grant;
logic [7:0]  rss_priority = '0;

logic [3:0]  dp_rss_req;
logic [3:0]  dp_rss_grant;

  //------------------------------------------------------------------------------
  // Agilex Global Shoreline Reset Sequencer (One Per Side)
  //------------------------------------------------------------------------------
  dp_gts_rss i_dp_gts_rss (
    .i_src_rs_priority            (rss_priority),
    .i_src_rs_req                 (rss_req),
    .o_src_rs_grant               (rss_grant),
    .o_pma_cu_clk                 (rss_clk)
  );

assign rss_req[3:0] = dp_rss_req;
assign dp_rss_grant = rss_grant[3:0];

assign rss_req[7:4]     = hdmi_rss_req;
assign hdmi_rss_grant   = rss_grant[7:4];

assign dp_rss_clk       = rss_clk[0];
assign hdmi_rss_clk     = rss_clk[1];		

  //------------------------------------------------------------------------------
  // Duplex Phy
  //------------------------------------------------------------------------------
  dp_gts_rxtx i_dp_gts_rxtx (
        .refclk_150                            (fgt_refclk_150),
        .mgmt_clock                            (core_refclk_100),
        .mgmt_resetn                           (sys_resetn & pio_board[1]),
        .rss_req                               (dp_rss_req),
        .rss_grant                             (dp_rss_grant),
        .rss_clk                               (dp_rss_clk),
        .rx_clk                                (gxb_rx_clkout),
        .rx_p                                  (fmc_rx_p),
        .rx_n                                  (fmc_rx_n),
        .rx_reconfig_en                        (1'b1),
        .rx_rate_rcfg_busy                     (dp_rx_rate_rcfg_busy),
        .rx_link_rate                          (dp_rx_link_rate),
        .rx_lane_count                         (dp_rx_lane_count),
        .rx_rate_rcfg_req                      (dp_rx_rate_rcfg_req),
        .rx_rate_rcfg_ack                      (dp_rx_rate_rcfg_ack),
        .rx_is_lockedtoref                     (dp_rx_is_lockedtoref),
        .rx_parallel_data                      (dp_rx_parallel_data),
        .rx_is_lockedtodata                    (dp_rx_is_lockedtodata),
        .tx_clk                                (gxb_tx_clkout),
        .tx_p                                  (fmc_tx_p),
        .tx_n                                  (fmc_tx_n),
        .tx_rate_rcfg_busy                     (dp_tx_rate_rcfg_busy),
        .tx_link_rate                          (dp_tx_link_rate),
        .tx_rate_rcfg_req                      (dp_tx_rate_rcfg_req),
        .tx_rate_rcfg_ack                      (dp_tx_rate_rcfg_ack),
        .tx_reconfig_en                        (1'b1),
        .tx_analog_rcfg_req                    (dp_tx_analog_rcfg_req),
        .tx_analog_rcfg_ack                    (dp_tx_analog_rcfg_ack),
        .tx_analog_rcfg_vod                    (dp_tx_analog_rcfg_vod),
        .tx_analog_rcfg_emp                    (dp_tx_analog_rcfg_emp),
        .tx_analog_rcfg_ffe                    ({TX_MAX_LANE_COUNT{4'b0}}),
        .tx_parallel_data                      (dp_tx_parallel_data),
        .tx_pll_locked                         (dp_txpll_locked)
  );


// -------------------------------------------------------------------------
// DisplayPort Core Qsys system
// -------------------------------------------------------------------------
  dp_core i_dp_core (
    .cpu_reset_bridge_in_reset_reset_n                  (sys_resetn),
    .mgmt_clk_in_clk_clk                                (core_refclk_100),
    .dp_core_pio_board_external_connection_export       (pio_board),
    // -------------------------------------------------------------------
    // I2C for board to reconfigure Board Clocks
    // -------------------------------------------------------------------
    .dp_core_i2c_board_i2c_serial_sda_in                (board_i2c_sda_in),
    .dp_core_i2c_board_i2c_serial_scl_in                (board_i2c_scl_in),
    .dp_core_i2c_board_i2c_serial_sda_oe                (board_i2c_sda_oe),
    .dp_core_i2c_board_i2c_serial_scl_oe                (board_i2c_scl_oe),

    .dp_core_axis_clk_in_clk_clk                        (clk_300),
    .dp_core_axis_reset_bridge_in_reset_reset_n         (sys_resetn),

    .dptx_pio_actv_dim_external_connection_export       (dp_tx_actv_dim_o),  
    .dptx_pio_new_actv_dim_external_connection_export   (dp_tx_new_actv_dim_i),
    .dptx_pio_fps_external_connection_export            (dp_tx_fps_i),

    // -------------------------------------------------------------------
    // DisplayPort Sink Sub-System
    // -------------------------------------------------------------------
    .dp_rx_clk_16_in_clk_clk                            (clk_16),
    .dp_rx_reset_bridge_in_reset_reset_n                (sys_resetn),

    // Hot Plug Detect Interface
    .dp_rx_dp_sink_rx_aux_rx_hpd                        (dp_rx_hpd),
    .dp_rx_dp_sink_rx_aux_rx_cable_detect               (dp_rx_cable_detect),
    .dp_rx_dp_sink_rx_aux_rx_pwr_detect                 (dp_rx_pwr_detect),
    // DisplayPort Auxiliarty Interface
    .dp_rx_dp_sink_rx_aux_rx_aux_in                     (dp_rx_aux_in),
    .dp_rx_dp_sink_rx_aux_rx_aux_out                    (dp_rx_aux_out),
    .dp_rx_dp_sink_rx_aux_rx_aux_oe                     (dp_rx_aux_oe),
    // DisplayPort RX Video Protocol Interface (Interface with PCR)
    .dp_rx_dp_sink_rx_vid_clk_clk                       (dp_rx_vid_clk),
    .dp_rx_dp_sink_rx_msa_conduit_rx_msa                (dp_rx_msa),
    .dp_rx_dp_sink_rx_params_rx_lane_count              (dp_rx_lane_count),
    
    // RX AXI4-S Interface
    .dp_rx_dp_sink_rx_axi4s_vid_out_tuser               (dp_rx_tuser),
    .dp_rx_dp_sink_rx_axi4s_vid_out_tdata               (dp_rx_tdata),
    .dp_rx_dp_sink_rx_axi4s_vid_out_tvalid              (dp_rx_tvalid),
    .dp_rx_dp_sink_rx_axi4s_vid_out_tready              (dp_rx_tready),
    .dp_rx_dp_sink_rx_axi4s_vid_out_tlast               (dp_rx_tlast),    

    // DisplayPort Analog Reconfiguration Interface
    .dp_rx_dp_sink_rx_reconfig_rx_link_rate                     (),
    .dp_rx_dp_sink_rx_reconfig_rx_link_rate_8bits               (dp_rx_link_rate),
    .dp_rx_dp_sink_rx_reconfig_rx_reconfig_req                  (dp_rx_rate_rcfg_req),
    .dp_rx_dp_sink_rx_reconfig_rx_reconfig_ack                  (dp_rx_rate_rcfg_ack),
    .dp_rx_dp_sink_rx_reconfig_rx_reconfig_busy                 (dp_rx_rate_rcfg_busy),
    .dp_rx_dp_sink_rx_analog_reconfig_rx_analog_reconfig_req    (dp_rx_analog_rcfg_req),
    .dp_rx_dp_sink_rx_analog_reconfig_rx_vod                    (dp_rx_analog_rcfg_vod),
    .dp_rx_dp_sink_rx_analog_reconfig_rx_emp                    (dp_rx_analog_rcfg_emp),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_parallel_data           (dp_rx_parallel_data),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_std_clkout              (gxb_rx_clkout),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_is_lockedtoref          (dp_rx_is_lockedtoref),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_is_lockedtodata         (dp_rx_is_lockedtodata),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_restart                 (dp_rx_restart),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_cal_busy                (dp_rx_cal_busy),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_set_locktoref           (dp_rx_set_locktoref),
    .dp_rx_dp_sink_rx_xcvr_interface_rx_set_locktodata          (dp_rx_set_locktodata),
    .dp_rx_pio_0_external_connection_export                     (~user_pb[0]),
    // -------------------------------------------------------------------
    // DisplayPort Source Sub-System
    // -------------------------------------------------------------------
    .dp_tx_clk_16_in_clk_clk                            (clk_16),
    .dp_tx_reset_bridge_in_reset_reset_n                (sys_resetn),
     // Hot Plug Detect Interface
    .dp_tx_dp_source_tx_aux_tx_hpd                      (dp_tx_hpd),
    // DisplayPort Auxiliarty Interface
    .dp_tx_dp_source_tx_aux_tx_aux_in                   (dp_tx_aux_in),
    .dp_tx_dp_source_tx_aux_tx_aux_out                  (dp_tx_aux_out),
    .dp_tx_dp_source_tx_aux_tx_aux_oe                   (dp_tx_aux_oe),

    // TX Video Signal Interface
    .dp_tx_dp_source_tx_vid_clk_clk                     (dp_tx_vid_clk),

    // TX AXI4-S Interface  
    .dp_tx_dp_source_tx_axi4s_vid_in_tuser              (dp_tx_tuser),
    .dp_tx_dp_source_tx_axi4s_vid_in_tdata              (dp_tx_tdata),
    .dp_tx_dp_source_tx_axi4s_vid_in_tvalid             (dp_tx_tvalid),
    .dp_tx_dp_source_tx_axi4s_vid_in_tready             (dp_tx_tready),
    .dp_tx_dp_source_tx_axi4s_vid_in_tlast              (dp_tx_tlast),

    // DisplayPort Analog Reconfiguration Interface
    .dp_tx_dp_source_tx_analog_reconfig_tx_analog_reconfig_req  (dp_tx_analog_rcfg_req),
    .dp_tx_dp_source_tx_analog_reconfig_tx_analog_reconfig_ack  (dp_tx_analog_rcfg_ack),
    .dp_tx_dp_source_tx_analog_reconfig_tx_vod                  (dp_tx_analog_rcfg_vod),
    .dp_tx_dp_source_tx_analog_reconfig_tx_emp                  (dp_tx_analog_rcfg_emp),
    .dp_tx_dp_source_tx_reconfig_tx_link_rate                   (),
    .dp_tx_dp_source_tx_reconfig_tx_link_rate_8bits             (dp_tx_link_rate),
    .dp_tx_dp_source_tx_reconfig_tx_reconfig_req                (dp_tx_rate_rcfg_req),
    .dp_tx_dp_source_tx_reconfig_tx_reconfig_ack                (dp_tx_rate_rcfg_ack),
    .dp_tx_dp_source_tx_reconfig_tx_reconfig_busy               (dp_tx_rate_rcfg_busy),
    .dp_tx_dp_source_tx_xcvr_interface_tx_parallel_data         (dp_tx_parallel_data),
    .dp_tx_dp_source_tx_xcvr_interface_tx_pll_powerdown         (),
    .dp_tx_dp_source_tx_xcvr_interface_tx_analogreset           (),
    .dp_tx_dp_source_tx_xcvr_interface_tx_digitalreset          (),
    .dp_tx_dp_source_tx_xcvr_interface_tx_cal_busy              (dp_tx_cal_busy),
    .dp_tx_dp_source_tx_xcvr_interface_tx_std_clkout            (gxb_tx_clkout),
    .dp_tx_dp_source_tx_xcvr_interface_tx_pll_locked            (dp_txpll_locked)
  );

  // -------------------------------------------------------------------------
  // Rx assignment
  // -------------------------------------------------------------------------
  assign dp_rx_vid_clk = clk_300;
  assign fmc_dp_rx_hpd = dp_rx_hpd;
  assign dp_rx_cable_detect = ~fmc_dp_rx_cable_detect;
  assign dp_rx_pwr_detect = fmc_dp_rx_pwr_detect;
  assign dp_rx_aux_in = fmc_dp_rx_aux_in;
  assign fmc_dp_rx_aux_out = dp_rx_aux_out;
  assign fmc_dp_rx_aux_oe = dp_rx_aux_oe;

  // -------------------------------------------------------------------------
  // Tx assignment
  // -------------------------------------------------------------------------
  assign dp_tx_hpd = ~fmc_dp_tx_hpd;          // Inverted on FMC card
  assign dp_tx_aux_in = fmc_dp_tx_aux_in;
  assign fmc_dp_tx_aux_out = dp_tx_aux_out;
  assign fmc_dp_tx_aux_oe = dp_tx_aux_oe;

  // -------------------------------------------------------------------------
  // board Programmable Oscillator I2C Master
  // -------------------------------------------------------------------------
  assign board_i2c_scl_in = board_i2c_scl;
  assign board_i2c_sda_in = board_i2c_sda;
  assign board_i2c_scl = board_i2c_scl_oe ? 1'b0 : 1'bz;
  assign board_i2c_sda = board_i2c_sda_oe ? 1'b0 : 1'bz;
  





`ifdef DEBUG
(* noprune *) logic [23:0] debug_pxlclk_freq;

  // Measure the TX Pixel Clock
  mr_rate_detect i_mr_rate_detect_pxlclk (
      .refclock           (dp_tx_vid_clk),
      .measure_clk        (core_refclk_100),
      .reset              (1'b0),
      .refclock_measure   (debug_pxlclk_freq),
      .valid              ()
  );
 
(* noprune *) logic [23:0] debug_rxclk_freq;

  // Measure the RX Transciever Clock
  mr_rate_detect i_mr_rate_detect_rxclk (
      .refclock           (gxb_rx_clkout),
      .measure_clk        (core_refclk_100),
      .reset              (1'b0),
      .refclock_measure   (debug_rxclk_freq),
      .valid              ()
  );

(* noprune *) logic [23:0] debug_txclk_freq;

  // Measure the TX Transciever Clock
  mr_rate_detect i_mr_rate_detect_txclk (
      .refclock           (gxb_tx_clkout),
      .measure_clk        (core_refclk_100),
      .reset              (1'b0),
      .refclock_measure   (debug_txclk_freq),
      .valid              ()
  );
`endif

endmodule

