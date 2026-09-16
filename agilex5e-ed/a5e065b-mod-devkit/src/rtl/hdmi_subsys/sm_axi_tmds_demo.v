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



// Uncomment below to enable SignalTap Debug Features
// `define DEBUG_EN 1

`timescale 1 ps / 1 ps
module sm_axi_tmds_demo (

      // Clocks Inputs
      input         clk_100,         // 100 Mhz general purpose clock
      input         syspll_refclk,   // 100 Mhz Fixed SystemPLL Ref-Clock
      input         phy_refclk_rx,   // TMDS Rx Ref-Clock
      input         phy_refclk_tx,   // TMDS Tx Ref-Clock      

      input         sysclk_ready_i,
      output        sysclk_startup_o,

      // Board Level I2C Configuration
      inout         i2c_max10_scl,
      inout         i2c_max10_sda,

      // HDMI I2C Configuration
      inout         i2c_clocks_scl,
      inout         i2c_clocks_sda,
      inout         i2c_hdmi_sda,
      inout         i2c_hdmi_scl,

      // AXI-S Video clock
      output        hdmi_rx_phy_clk_out,
      output        hdmi_tx_phy_clk_out,
      output        hdmi_tx_vid_clk_out,      
      input         vid_clk,
      input         mgmt_reset_hdmi,
      input         cold_reset_hdmi,    
      output [3:0]  hdmi_rss_req,
      input  [3:0]  hdmi_rss_grant,
      input         hdmi_rss_clk,


     // To/From VVP PIPE    
     output [31:0]  hdmi_tx_actv_dim_o, 
     input  [31:0]  hdmi_tx_new_actv_dim_i, 
     input  [ 7:0]  hdmi_tx_fps_i,

      // HDMI Rx Port   
      output [63:0] hdmi_rx_tdata,  
      output [7:0]  hdmi_rx_tuser,  
      output        hdmi_rx_tvalid, 
      input         hdmi_rx_tready, 
      output        hdmi_rx_tlast,  
        
      // HDMI Tx Port   
      input  [63:0] hdmi_tx_tdata,  
      input  [7:0]  hdmi_tx_tuser,  
      input         hdmi_tx_tvalid, 
      output        hdmi_tx_tready, 
      input         hdmi_tx_tlast,  

      // Serial Interface
      input [2:0]   fmc_rx_p,
      input [2:0]   fmc_rx_n,
      output[3:0]   fmc_tx_p,
      output[3:0]   fmc_tx_n,

      // HDMI RX Interface
      output        hdmi_rx_hpd_n,
      input         hdmi_5v_detect_n,
      inout         hdmi_rx_i2c_sda,
      input         hdmi_rx_i2c_scl,

      // HDMI TX Interface
      input         hdmi_tx_hpd_n,
      inout         hdmi_tx_i2c_sda,
      inout         hdmi_tx_i2c_scl
);

wire         device_ready;
wire [31:0]  nios_pio_out0;



//------------------------------------------------------------------------------
// HDMI Tx Phy AVMM Interface
//------------------------------------------------------------------------------
wire         tx_phy_clk;
wire         tx_phy_lock;
wire [159:0] tx_phy_data;
wire [1:0]   tx_phy_os;
wire         tx_phy_m0_waitrequest;
wire [31:0]  tx_phy_m0_readdata;
reg          tx_phy_m0_readdatavalid;
wire [0:0]   tx_phy_m0_burstcount;
wire [31:0]  tx_phy_m0_writedata;
wire [9:0]   tx_phy_m0_address;
wire         tx_phy_m0_write;
wire         tx_phy_m0_read;
wire [3:0]   tx_phy_m0_byteenable;
wire         tx_phy_m0_debugaccess;

assign tx_phy_m0_waitrequest = 1'b0;

always @(posedge clk_100) begin
  tx_phy_m0_readdatavalid <= tx_phy_m0_read;
end

wire         hdmi_tx_5v;
assign hdmi_tx_5v = ~nios_pio_out0[1];


wire rx_phy_5v;

wire   hdmi_rx_hpd;
assign hdmi_rx_hpd_n = ~hdmi_rx_hpd;
wire   hdmi_5v_detect;
assign hdmi_5v_detect = ~hdmi_5v_detect_n;

//------------------------------------------------------------------------------
// HDMI Rx Phy AVMM Interface
//------------------------------------------------------------------------------
wire [3:0]   rx_phy_clk;
wire         rx_phy_lock;
wire         rx_phy_os;
wire         rx_phy_tbcr;
wire [159:0] rx_phy_data;
wire         rx_core_locked;
wire         rx_phy_m0_waitrequest;
wire [31:0]  rx_phy_m0_readdata;
reg          rx_phy_m0_readdatavalid;
wire [0:0]   rx_phy_m0_burstcount;
wire [31:0]  rx_phy_m0_writedata;
wire [9:0]   rx_phy_m0_address;
wire         rx_phy_m0_write;
wire         rx_phy_m0_read;
wire [3:0]   rx_phy_m0_byteenable;
wire         rx_phy_m0_debugaccess;

assign rx_phy_m0_waitrequest = 1'b0;

always @(posedge clk_100) begin
  rx_phy_m0_readdatavalid <= rx_phy_m0_read;
end

//------------------------------------------------------------------------------
// Clock Signals & Status
//------------------------------------------------------------------------------
wire         sysclk_locked;
wire         sysclk;

wire mgmt_reset;
wire cold_reset;

assign mgmt_reset = mgmt_reset_hdmi;
assign cold_reset = cold_reset_hdmi;

//---------------------------------------------------------------------------
// TX Video Clock Generation
//---------------------------------------------------------------------------
assign hdmi_tx_vid_clk_out = tx_phy_clk;
assign hdmi_rx_phy_clk_out = rx_phy_clk[0];
assign hdmi_tx_phy_clk_out = tx_phy_clk;

//------------------------
// HDMI-Tx & Rx Phy in Dual-Simplex mode
//------------------------
DS_GROUP_0 u_DS_GROUP_0(
  .i_rs_grant_ch0                       (hdmi_rss_grant[0]),
  .i_rs_grant_ch1                       (hdmi_rss_grant[1]),
  .i_rs_grant_ch2                       (hdmi_rss_grant[2]),
  .i_rs_grant_ch3                       (hdmi_rss_grant[3]),
  .o_rs_request_ch0                     (hdmi_rss_req[0]),
  .o_rs_request_ch1                     (hdmi_rss_req[1]),
  .o_rs_request_ch2                     (hdmi_rss_req[2]),
  .o_rs_request_ch3                     (hdmi_rss_req[3]),
  .i_pma_cu_clk_bank0                   (hdmi_rss_clk),
  .sysclk                               (sysclk),
  .sysclk_locked                        (sysclk_locked),
  .av_mm_control_address_hdmi_rx_inst0  (rx_phy_m0_address[9:2]),
  .av_mm_control_address_hdmi_tx_inst0  (tx_phy_m0_address[9:2]),
  .av_mm_control_read_hdmi_rx_inst0     (rx_phy_m0_read),
  .av_mm_control_read_hdmi_tx_inst0     (tx_phy_m0_read),
  .av_mm_control_readdata_hdmi_rx_inst0 (rx_phy_m0_readdata),
  .av_mm_control_readdata_hdmi_tx_inst0 (tx_phy_m0_readdata),
  .av_mm_control_write_hdmi_rx_inst0    (rx_phy_m0_write),
  .av_mm_control_write_hdmi_tx_inst0    (tx_phy_m0_write),
  .av_mm_control_writedata_hdmi_rx_inst0(rx_phy_m0_writedata),
  .av_mm_control_writedata_hdmi_tx_inst0(tx_phy_m0_writedata),
  .reset_hdmi_rx_inst0                  (mgmt_reset),
  .mgmt_clk_hdmi_rx_inst0               (clk_100),
  .rx_phy_cdr_refclk_tmds_hdmi_rx_inst0 (phy_refclk_rx),
  .rx_parallel_data_hdmi_rx_inst0       (rx_phy_data),
  .in_lock_hdmi_rx_inst0                (rx_phy_lock),
  // .vid_clk_hdmi_rx_inst0                (),
  .rx_clk_hdmi_rx_inst0                 (rx_phy_clk),
  .locked_hdmi_rx_inst0                 (rx_core_locked),
  .tmds_bit_clock_ratio_hdmi_rx_inst0   (rx_phy_tbcr),
  .os_hdmi_rx_inst0                     (rx_phy_os),
  // .i2c_trans_detected_hdmi_rx_inst0     (rx_phy_i2c_trans_detected),
  // .i2c_trans_detected_ack_hdmi_rx_inst0 (rx_phy_i2c_trans_detected_ack),
  .hdmi_5v_detect_hdmi_rx_inst0         (hdmi_5v_detect),
  .rx_5v_detect_hdmi_rx_inst0           (rx_phy_5v),
  // .color_depth_hdmi_rx_inst0            (rx_phy_color_depth),
  .tmds_freq_hdmi_rx_inst0              (),
  .tmds_freq_valid_hdmi_rx_inst0        (),
  .tmds_clk_hdmi_rx_inst0               (),
  .i_rx_serial_data_p_ch0               (fmc_rx_p[0]),
  .i_rx_serial_data_n_ch0               (fmc_rx_n[0]),
  .i_rx_serial_data_p_ch1               (fmc_rx_p[1]),
  .i_rx_serial_data_n_ch1               (fmc_rx_n[1]),
  .i_rx_serial_data_p_ch2               (fmc_rx_p[2]),
  .i_rx_serial_data_n_ch2               (fmc_rx_n[2]),  
  .reset_hdmi_tx_inst0                  (mgmt_reset),
  .mgmt_clk_hdmi_tx_inst0               (clk_100),
  .tx_phy_refclk_tmds_hdmi_tx_inst0     (phy_refclk_tx),
  .hdmi_tx_5v_hdmi_tx_inst0             (hdmi_tx_5v),
  .tx_parallel_data_hdmi_tx_inst0       (tx_phy_data),
  .tx_clk_hdmi_tx_inst0                 (tx_phy_clk),
  .diag_tx_clk_hdmi_tx_inst0            (tx_phy_clk),
  .in_lock_hdmi_tx_inst0                (tx_phy_lock),
  .os_hdmi_tx_inst0                     (tx_phy_os),
  .o_tx_serial_data_p_ch0               (fmc_tx_p[0]),
  .o_tx_serial_data_n_ch0               (fmc_tx_n[0]),
  .o_tx_serial_data_p_ch1               (fmc_tx_p[1]),
  .o_tx_serial_data_n_ch1               (fmc_tx_n[1]),
  .o_tx_serial_data_p_ch2               (fmc_tx_p[2]),
  .o_tx_serial_data_n_ch2               (fmc_tx_n[2]),
  .o_tx_serial_data_p_ch3               (fmc_tx_p[3]),
  .o_tx_serial_data_n_ch3               (fmc_tx_n[3])
);

//---------------------------------------------------------------------------
// I2C Buffers for control of board infrastructure clock-muxes
//---------------------------------------------------------------------------
wire i2c_max10_sda_in, i2c_max10_scl_in, i2c_max10_sda_oe, i2c_max10_scl_oe;

output_buf_i2c u_i2c_max10_scl_buf  (.datain(1'b0),.padio(i2c_max10_scl), .oe(i2c_max10_scl_oe), .dataout(i2c_max10_scl_in));
output_buf_i2c u_i2c_max10_sda_buf  (.datain(1'b0),.padio(i2c_max10_sda), .oe(i2c_max10_sda_oe), .dataout(i2c_max10_sda_in));

//---------------------------------------------------------------------------
// I2C Buffers for control of board infrastructure clocks
//---------------------------------------------------------------------------
wire i2c_clocks_sda_in,i2c_clocks_scl_in,i2c_clocks_sda_oe,i2c_clocks_scl_oe;

output_buf_i2c u_i2c_clocks_scl_buf (.datain(1'b0),.padio(i2c_clocks_scl),.oe(i2c_clocks_scl_oe),.dataout(i2c_clocks_scl_in));
output_buf_i2c u_i2c_clocks_sda_buf (.datain(1'b0),.padio(i2c_clocks_sda),.oe(i2c_clocks_sda_oe),.dataout(i2c_clocks_sda_in));

//---------------------------------------------------------------------------
// I2C Buffers for HDMI Redriver and Equaliser
//---------------------------------------------------------------------------
wire i2c_hdmi_sda_in, i2c_hdmi_scl_in, i2c_hdmi_sda_oe, i2c_hdmi_scl_oe;

output_buf_i2c u_i2c_hdmi_scl_buf  (.datain(1'b0),.padio(i2c_hdmi_scl), .oe(i2c_hdmi_scl_oe), .dataout(i2c_hdmi_scl_in));
output_buf_i2c u_i2c_hdmi_sda_buf  (.datain(1'b0),.padio(i2c_hdmi_sda), .oe(i2c_hdmi_sda_oe), .dataout(i2c_hdmi_sda_in));

//---------------------------------------------------------------------------
// NIOS Platform Designer System
//---------------------------------------------------------------------------
nios u_nios (
    // CPU clock
    .cpu_dbg_reset_out_reset                                          (),
    .cpu_clk_in_clk_clk                                               (clk_100),
    .cpu_rst_in_reset_reset                                           (mgmt_reset),

    // Video PLL 300MHz
    .vid_rst_in_reset_reset                                           (cold_reset),
    .vid_clk_in_clk_clk                                               (vid_clk),

    .hdmitx_pio_new_actv_dim_external_connection_export               (hdmi_tx_actv_dim_o       ),
    .hdmitx_pio_actv_dim_external_connection_export                   (hdmi_tx_new_actv_dim_i   ),  
    .hdmitx_pio_fps_external_connection_export                        (hdmi_tx_fps_i            ),

    // HDMI Rx Port   
    .nios_vvp_pixel_rx_axi4s_vid_out_tuser                            (hdmi_rx_tuser),                       
    .nios_vvp_pixel_rx_axi4s_vid_out_tdata                            (hdmi_rx_tdata),                       
    .nios_vvp_pixel_rx_axi4s_vid_out_tvalid                           (hdmi_rx_tvalid),                       
    .nios_vvp_pixel_rx_axi4s_vid_out_tready                           (hdmi_rx_tready),                       
    .nios_vvp_pixel_rx_axi4s_vid_out_tlast                            (hdmi_rx_tlast),                      

    // HDMI Tx Port   
    .nios_vvp_pixel_tx_axi4s_vid_in_tuser                             (hdmi_tx_tuser), 
    .nios_vvp_pixel_tx_axi4s_vid_in_tdata                             (hdmi_tx_tdata), 
    .nios_vvp_pixel_tx_axi4s_vid_in_tvalid                            (hdmi_tx_tvalid),
    .nios_vvp_pixel_tx_axi4s_vid_in_tready                            (hdmi_tx_tready),
    .nios_vvp_pixel_tx_axi4s_vid_in_tlast                             (hdmi_tx_tlast), 

    // AVMM interface from CPU To RX Phy
    .nios_hdmi_rx_phy_m0_waitrequest                                  (rx_phy_m0_waitrequest),
    .nios_hdmi_rx_phy_m0_readdata                                     (rx_phy_m0_readdata),
    .nios_hdmi_rx_phy_m0_readdatavalid                                (rx_phy_m0_readdatavalid),
    .nios_hdmi_rx_phy_m0_burstcount                                   (rx_phy_m0_burstcount),
    .nios_hdmi_rx_phy_m0_writedata                                    (rx_phy_m0_writedata),
    .nios_hdmi_rx_phy_m0_address                                      (rx_phy_m0_address),
    .nios_hdmi_rx_phy_m0_write                                        (rx_phy_m0_write),
    .nios_hdmi_rx_phy_m0_read                                         (rx_phy_m0_read),
    .nios_hdmi_rx_phy_m0_byteenable                                   (rx_phy_m0_byteenable),
    .nios_hdmi_rx_phy_m0_debugaccess                                  (rx_phy_m0_debugaccess),

    // Phy interface between phy and core
    .nios_hdmi_rx_phy_interface_rx_parallel_data                      (rx_phy_data),
    .nios_hdmi_rx_phy_interface_in_lock                               (rx_phy_lock),
    .nios_hdmi_rx_phy_interface_vid_clk                               (vid_clk),
    .nios_hdmi_rx_hpd_interface_in_5v_power                           (rx_phy_5v),
    .nios_hdmi_rx_phy_interface_rx_clk                                (rx_phy_clk),
    .nios_hdmi_rx_phy_interface_locked                                (rx_core_locked),
    .nios_hdmi_rx_phy_interface_tmds_bit_clock_ratio                  (rx_phy_tbcr),
    .nios_hdmi_rx_phy_interface_os                                    (rx_phy_os),
    .nios_hdmi_rx_phy_interface_i2c_trans_detected                    (),
    .nios_hdmi_rx_phy_interface_i2c_trans_detected_ack                (1'b0),
    .nios_hdmi_rx_phy_interface_color_depth                           (),    

    // Top Level RX wires
    .nios_hdmi_rx_i2c_clk_clk                                         (clk_100),
    .nios_hdmi_rx_hpd_interface_hpd                                   (hdmi_rx_hpd),
    .nios_hdmi_rx_i2c_interface_scl                                   (hdmi_rx_i2c_scl),
    .nios_hdmi_rx_i2c_interface_sda                                   (hdmi_rx_i2c_sda),

    // RX Auxillary bus
    .nios_hdmi_rx_av_mm_aux_out_aux_pkt_data                          (),
    .nios_hdmi_rx_av_mm_aux_out_aux_pkt_addr                          (),
    .nios_hdmi_rx_av_mm_aux_out_aux_pkt_wr                            (),

    // AVMM interface from CPU To TX Phy
    .nios_hdmi_tx_phy_m0_waitrequest                                  (tx_phy_m0_waitrequest),
    .nios_hdmi_tx_phy_m0_readdata                                     (tx_phy_m0_readdata),
    .nios_hdmi_tx_phy_m0_readdatavalid                                (tx_phy_m0_readdatavalid),
    .nios_hdmi_tx_phy_m0_burstcount                                   (tx_phy_m0_burstcount),
    .nios_hdmi_tx_phy_m0_writedata                                    (tx_phy_m0_writedata),
    .nios_hdmi_tx_phy_m0_address                                      (tx_phy_m0_address),
    .nios_hdmi_tx_phy_m0_write                                        (tx_phy_m0_write),
    .nios_hdmi_tx_phy_m0_read                                         (tx_phy_m0_read),
    .nios_hdmi_tx_phy_m0_byteenable                                   (tx_phy_m0_byteenable),
    .nios_hdmi_tx_phy_m0_debugaccess                                  (tx_phy_m0_debugaccess),

    // Phy interface between phy and core
    .nios_hdmi_tx_phy_interface_tx_parallel_data                      (tx_phy_data),
    .nios_hdmi_tx_phy_interface_tx_clk                                (tx_phy_clk),
    .nios_hdmi_tx_phy_interface_vid_clk                               (tx_phy_clk),
    .nios_hdmi_tx_phy_interface_in_lock                               (tx_phy_lock),
    .nios_hdmi_tx_phy_interface_os                                    (tx_phy_os),

    // Top Level TX wires
    .nios_hdmi_tx_hpd_interface_hpd                                   (hdmi_tx_hpd_n),    
    .nios_hdmi_tx_i2c_interface_sda                                   (hdmi_tx_i2c_sda),
    .nios_hdmi_tx_i2c_interface_scl                                   (hdmi_tx_i2c_scl),

    // System-PLL interface
    .nios_systempll_o_pll_lock_o_pll_lock                             (sysclk_locked),
    .nios_systempll_o_syspll_c0_clk                                   (sysclk),
    .nios_systempll_refclk_xcvr_clk                                   (syspll_refclk),
    .nios_systempll_i_refclk_rdy_data                                 (sysclk_ready_i),

    // Board Level Infrastructure
    .i2c_max10_i2c_serial_sda_in                                      (i2c_max10_sda_in),
    .i2c_max10_i2c_serial_scl_in                                      (i2c_max10_scl_in),
    .i2c_max10_i2c_serial_sda_oe                                      (i2c_max10_sda_oe),
    .i2c_max10_i2c_serial_scl_oe                                      (i2c_max10_scl_oe),

    // I2C Clock Control
    .i2c_clocks_i2c_serial_sda_in                                     (i2c_clocks_sda_in),
    .i2c_clocks_i2c_serial_scl_in                                     (i2c_clocks_scl_in),
    .i2c_clocks_i2c_serial_sda_oe                                     (i2c_clocks_sda_oe),
    .i2c_clocks_i2c_serial_scl_oe                                     (i2c_clocks_scl_oe),

    // Redriver/Retimer I2C interface
    .i2c_hdmi_i2c_serial_sda_in                                       (i2c_hdmi_sda_in),
    .i2c_hdmi_i2c_serial_scl_in                                       (i2c_hdmi_scl_in),
    .i2c_hdmi_i2c_serial_sda_oe                                       (i2c_hdmi_sda_oe),
    .i2c_hdmi_i2c_serial_scl_oe                                       (i2c_hdmi_scl_oe),
    .debug_signal_external_connection_export                          (nios_pio_out0),
    .pio_systempll_external_connection_in_port                        (sysclk_ready_i), // sysclk_locked
    .pio_systempll_external_connection_out_port                       (sysclk_startup_o)
  );

// Signal Tap debug features. search netlist for "stp_*"
`ifdef DEBUG_EN

//---------------------------------------------------------------------------
// Check vid_clk frequency (300MHz)
//---------------------------------------------------------------------------
// (* noprune *) wire [23:0] stp_meas_vid_clk;

// mr_rate_detect vid_clk_freq (
    // .refclock           (vid_clk),
    // .measure_clk        (clk_100),
    // .reset              (mgmt_reset),
    // .enable             (1'b1),
    // .refclock_measure   (stp_meas_vid_clk),
    // .valid              ()
// );

//---------------------------------------------------------------------------
// Check rx_clk from Phy
//---------------------------------------------------------------------------
// (* noprune *) wire [23:0] stp_meas_rx_phy_clk;

// mr_rate_detect u_freq_rx_clk (
    // .refclock           (rx_phy_clk[0]),
    // .measure_clk        (clk_100),
    // .reset              (mgmt_reset),
    // .enable             (1'b1),
    // .refclock_measure   (stp_meas_rx_phy_clk),
    // .valid              ()
// );

//---------------------------------------------------------------------------
// Check tx_clk from Phy
//---------------------------------------------------------------------------
// (* noprune *) wire [23:0] stp_meas_tx_phy_clk;

// mr_rate_detect u_freq_tx_clk (
    // .refclock           (tx_phy_clk),
    // .measure_clk        (clk_100),
    // .reset              (mgmt_reset),
    // .enable             (1'b1),
    // .refclock_measure   (stp_meas_tx_phy_clk),
    // .valid              ()
// );

//---------------------------------------------------------------------------
// Check generated tx video clock
//---------------------------------------------------------------------------
// (* noprune *) wire [23:0] stp_meas_tx_vid_clk;

// mr_rate_detect u_freq_tx_vid_clk (
    // .refclock           (tx_vid_clk),
    // .measure_clk        (clk_100),
    // .reset              (mgmt_reset),
    // .enable             (1'b1),
    // .refclock_measure   (stp_meas_tx_vid_clk),
    // .valid              ()
// );

//---------------------------------------------------------------------------
// Useful in SignalTap to provide 1ms timestamp for "transistional" captures
//---------------------------------------------------------------------------
(* noprune *) reg [15:0] stp_timestamp;
reg [24:0] stp_timestamp_div;
reg        stp_timestamp_evt;

always @(posedge clk_100) begin
  if (stp_timestamp_div == 23'd100000) begin
    stp_timestamp_div <= 24'd1;
	 stp_timestamp_evt <= 1'b1;
  end else begin
    stp_timestamp_div <= stp_timestamp_div + 1'b1;
	 stp_timestamp_evt <= 1'b0;
  end
  if (stp_timestamp_evt)
    stp_timestamp <= stp_timestamp + 1'b1;
end

`endif

endmodule


