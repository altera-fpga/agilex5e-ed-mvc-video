// (C) 2001-2024 Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License Subscription 
// Agreement, Intel FPGA IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Intel and sold by 
// Intel or its authorized distributors.  Please refer to the applicable 
// agreement for further details.

// Disable FMC I/O Tie Off Warnings.
(* altera_attribute = "-name MESSAGE_DISABLE 13010; -name MESSAGE_DISABLE 13410; -name MESSAGE_DISABLE 13008" *)
module top
#(
    parameter RX_MAX_LANE_COUNT = 4,
    parameter TX_MAX_LANE_COUNT = 4
)
(
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DP IOs
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Clocks Inputs
    input  logic                            cpu_resetn,                 // CPU Reset Pushbutton (TR=0)
    input  logic [0:0]                      user_pb,                    //User Pushbuttons
    input  logic                            fgt_refclk_150,             // 150Mhz System PLL Reference Clock
    input  logic                            core_refclk_100,            // 100Mhz

    // board I2C
    inout  logic                            board_i2c_scl,
    inout  logic                            board_i2c_sda,
 
   //FPGA Mezzanine Card (FMC)
    // DP Rx Port
    input  logic [(RX_MAX_LANE_COUNT-1):0]  fmc_rx_p,                  //Transceiver Data FPGA RX
    input  logic [(RX_MAX_LANE_COUNT-1):0]  fmc_rx_n,                  //Transceiver Data FPGA RX
    output logic                            fmc_dp_rx_hpd,             // RX_HPD
    input  logic                            fmc_dp_rx_cable_detect,    // RX Cable Detect
    input  logic                            fmc_dp_rx_pwr_detect,      // RX Power Detect (invert)
    input  logic                            fmc_dp_rx_aux_in,          // RX Aux In
    output logic                            fmc_dp_rx_aux_out,         // RX Aux Out
    output logic                            fmc_dp_rx_aux_oe,          // RX Aux OE

    // DP Tx Port
    output logic [(TX_MAX_LANE_COUNT-1):0]  fmc_tx_p,                  //Transceiver Data FPGA TX
    output logic [(TX_MAX_LANE_COUNT-1):0]  fmc_tx_n,                  //Transceiver Data FPGA TX
    output logic                            fmc_dp_tx_cad,             // TX CAD
    input  logic                            fmc_dp_tx_hpd,             // TX HPD
    input  logic                            fmc_dp_tx_aux_in,          // TX Aux In
    output logic                            fmc_dp_tx_aux_out,         // TX Aux Out
    output logic                            fmc_dp_tx_aux_oe,          // TX Aux OE

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EMIF
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //DDR4 Bank 3B: VVP Channel # 1
    input  logic                            mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                            mem_oct_rzqin,
    output logic [0:0]                      mem_ck,
    output logic [0:0]                      mem_ck_n,
    output logic [16:0]                     mem_a,
    output logic [0:0]                      mem_act_n,
    output logic [1:0]                      mem_ba,
    output logic [1:0]                      mem_bg,
    output logic [0:0]                      mem_cke,
    output logic [0:0]                      mem_cs_n,
    output logic [0:0]                      mem_odt,
    output logic [0:0]                      mem_reset_n,
    output logic [0:0]                      mem_par,
    input  logic [0:0]                      mem_alert_n,
    inout  logic [4:0]                      mem_dqs,
    inout  logic [4:0]                      mem_dqs_n,
    inout  logic [4:0]                      mem_dbi_n,
    inout  logic [39:0]                     mem_dq,
    
    //DDR4 Bank 2B: VVP Channel # 2
    input  logic                            bank_2b_mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                            bank_2b_mem_oct_rzqin,  
    output logic [0:0]                      bank_2b_mem_ck,         
    output logic [0:0]                      bank_2b_mem_ck_n,       
    output logic [16:0]                     bank_2b_mem_a,          
    output logic [0:0]                      bank_2b_mem_act_n,      
    output logic [1:0]                      bank_2b_mem_ba,         
    output logic [1:0]                      bank_2b_mem_bg,         
    output logic [0:0]                      bank_2b_mem_cke,        
    output logic [0:0]                      bank_2b_mem_cs_n,       
    output logic [0:0]                      bank_2b_mem_odt,        
    output logic [0:0]                      bank_2b_mem_reset_n,    
    output logic [0:0]                      bank_2b_mem_par,        
    input  logic [0:0]                      bank_2b_mem_alert_n,    
    inout  logic [3:0]                      bank_2b_mem_dqs,        
    inout  logic [3:0]                      bank_2b_mem_dqs_n,      
    inout  logic [3:0]                      bank_2b_mem_dbi_n,      
    inout  logic [31:0]                     bank_2b_mem_dq,

    //DDR4 Bank 3A: VVP Channel # 3
    input  logic                            bank_3a_mem_pll_ref_clk,            //166.67Mhz - Reference clock for DDR4
    input  logic                            bank_3a_mem_oct_rzqin,  
    output logic [0:0]                      bank_3a_mem_ck,         
    output logic [0:0]                      bank_3a_mem_ck_n,       
    output logic [16:0]                     bank_3a_mem_a,          
    output logic [0:0]                      bank_3a_mem_act_n,      
    output logic [1:0]                      bank_3a_mem_ba,         
    output logic [1:0]                      bank_3a_mem_bg,         
    output logic [0:0]                      bank_3a_mem_cke,        
    output logic [0:0]                      bank_3a_mem_cs_n,       
    output logic [0:0]                      bank_3a_mem_odt,        
    output logic [0:0]                      bank_3a_mem_reset_n,    
    output logic [0:0]                      bank_3a_mem_par,        
    input  logic [0:0]                      bank_3a_mem_alert_n,    
    inout  logic [4:0]                      bank_3a_mem_dqs,        
    inout  logic [4:0]                      bank_3a_mem_dqs_n,      
    inout  logic [4:0]                      bank_3a_mem_dbi_n,      
    inout  logic [39:0]                     bank_3a_mem_dq,

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HDMI IOs
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Clocks Inputs
    input                                   syspll_refclk,   // 156.25 mhz
    input                                   phy_refclk_rx,   // GTS Reference Clock
    input                                   phy_refclk_tx,   // GTS Reference Clock TX

    // Board Level I2C Configuration
    inout                                   i2c_max10_scl,
    inout                                   i2c_max10_sda,

    // HDMI I2C Configuration
    inout                                   i2c_clocks_scl,
    inout                                   i2c_clocks_sda,
    inout                                   i2c_hdmi_sda,
    inout                                   i2c_hdmi_scl,

    // Serial Interface
    input [2:0]                             hdmi_fmc_rx_p,
    input [2:0]                             hdmi_fmc_rx_n,
    output[3:0]                             hdmi_fmc_tx_p,
    output[3:0]                             hdmi_fmc_tx_n,

    // HDMI RX Interface
    output                                  hdmi_rx_hpd_n,
    input                                   hdmi_5v_detect_n,
    inout                                   hdmi_rx_i2c_sda,
    input                                   hdmi_rx_i2c_scl,

    // HDMI TX Interface
    input                                   hdmi_tx_hpd_n,
    inout                                   hdmi_tx_i2c_sda,
    inout                                   hdmi_tx_i2c_scl,

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDI IOs
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Clocks Inputs
    input  logic                            sdi_syspll_refclk,  // 312.5 MHz default from Nextera FMC.
    input  logic                            xcvr_refclk_1485,   // 148.35 MHz default from Si569 U33 tab. Program to 148.5 MHz in Clock GUI.
    input  logic                            txpll_refclk,       // Transceiver Ref Clocks - 297 / 296.7Mhz default via clk_mux_6 EU21 sel to high (via MAX10 GUI)

    inout  logic                            i2c_sdi_sda,
    inout  logic                            i2c_sdi_scl, 

//SDI daughter card-----------------------------------------------
    input  logic                            fmc_rx2_p,          // Transceiver Data FPGA RX
    input  logic                            fmc_rx2_n,

    output logic                            fmc_tx0_p,          // Transceiver Data FPGA TX
    output logic                            fmc_tx0_n,

    output logic                            lmk03328_pdn,       // LMK03328 Device Power Down (active low)
    output logic                            fmc_lmh1983_init,   // LMH1983 Init
    output logic                            fmc_fpga_fldn,      // Field sync signal to LMH1983
    output logic                            fmc_fpga_vsyncn,    // V sync signal to LMH1983
    output logic                            fmc_fpga_hsyncn     // H sync signal to LMH1983
);

logic                                       clk_300;
logic                                       sys_resetn;
logic                                       cold_resetn;
logic                                       cal_done_rst_n;
logic                                       bank_2b_cal_done_rst_n;
logic                                       bank_3a_cal_done_rst_n;

// AXI-S Video clock
logic                                       mgmt_reset_hdmi;
logic                                       cold_reset_hdmi;

logic                                       mgmt_reset_sdi;
logic                                       cold_reset_sdi;     
    
// HDMI Rx Port     
logic [63:0]                                hdmi_rx_tdata;  
logic [7:0]                                 hdmi_rx_tuser;  
logic                                       hdmi_rx_tvalid; 
logic                                       hdmi_rx_tready; 
logic                                       hdmi_rx_tlast;  
    
// HDMI Tx Port     
logic [63:0]                                hdmi_tx_tdata;  
logic [7:0]                                 hdmi_tx_tuser;  
logic                                       hdmi_tx_tvalid; 
logic                                       hdmi_tx_tready; 
logic                                       hdmi_tx_tlast; 

// DP Rx Port                
logic [63:0]                                dp_rx_tdata;    //        dp_rx_dp_sink_rx_axi4s_vid_out.tdata
logic [7:0]                                 dp_rx_tuser;    //                                      .tuser
logic                                       dp_rx_tvalid;   //                                      .tvalid
logic                                       dp_rx_tready;   //                                      .tready
logic                                       dp_rx_tlast;    //                                      .tlast

// DP Tx Port                
logic [63:0]                                dp_tx_tdata;   //       dp_tx_dp_source_tx_axi4s_vid_in.tdata
logic [7:0]                                 dp_tx_tuser;   //                                      .tuser
logic                                       dp_tx_tvalid;  //                                      .tvalid
logic                                       dp_tx_tready;  //                                      .tready
logic                                       dp_tx_tlast;   //             

// SDI Rx Port                
logic [63:0]                                sdi_rx_tdata;    //        dp_rx_dp_sink_rx_axi4s_vid_out.tdata
logic [7:0]                                 sdi_rx_tuser;    //                                      .tuser
logic                                       sdi_rx_tvalid;   //                                      .tvalid
logic                                       sdi_rx_tready;   //                                      .tready
logic                                       sdi_rx_tlast;    //                                      .tlast

// SDI Tx Port                
logic [63:0]                                sdi_tx_tdata;   //       dp_tx_dp_source_tx_axi4s_vid_in.tdata
logic [7:0]                                 sdi_tx_tuser;   //                                      .tuser
logic                                       sdi_tx_tvalid;  //                                      .tvalid
logic                                       sdi_tx_tready;  //                                      .tready
logic                                       sdi_tx_tlast;   // 

logic [23:0]                                clk_dbg_00;
logic [23:0]                                clk_dbg_01;
logic [23:0]                                clk_dbg_02;
logic [23:0]                                clk_dbg_03;
logic [23:0]                                clk_dbg_04;
logic [23:0]                                clk_dbg_05;
logic [23:0]                                clk_dbg_06;
logic [23:0]                                clk_dbg_07;
logic [23:0]                                clk_dbg_08;
logic [23:0]                                clk_dbg_09;

logic [31:0]                                hdmi_tx_actv_dim;
logic [31:0]                                hdmi_tx_new_actv_dim; 
logic [ 7:0]                                hdmi_tx_fps; 

logic [31:0]                                dp_tx_actv_dim;
logic [31:0]                                dp_tx_new_actv_dim; 
logic [ 7:0]                                dp_tx_fps; 

logic [31:0]                                sdi_tx_actv_dim;
logic [31:0]                                sdi_tx_new_actv_dim; 
logic [ 7:0]                                sdi_tx_fps; 


logic                                       dp_rx_phy_clk_out; 
logic                                       dp_tx_phy_clk_out; 
logic                                       dp_tx_vid_clk_out; 
logic                                       hdmi_tx_vid_clk_out;
logic                                       hdmi_rx_phy_clk_out; 
logic                                       hdmi_tx_phy_clk_out;

logic [31:0]                                new_actv_vid_hdmi_tx_pio;
logic [31:0]                                new_actv_vid_sdi_tx_pio;

logic [3:0]                                 hdmi_rss_req;
logic [3:0]                                 hdmi_rss_grant;
logic                                       hdmi_rss_clk; 

logic                                       sdi_rx_clkout_dbg;
logic                                       sdi_tx_clkout_dbg;

logic                                       emif_core_init_n_tmp;

logic                                       sysclk_ready_tmp;
logic                                       sysclk_startup_tmp;
logic                                       hdmi_rx_hpd_n_tmp;

// Contrary to what the schematics says
// this signal has a positive polarity
assign hdmi_rx_hpd_n = ~hdmi_rx_hpd_n_tmp;

mr_rate_detect u_clk_dbg_00 (
    .refclock           (core_refclk_100),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_00),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_01 (
    .refclock           (clk_300),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_01),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_02 (
    .refclock           (dp_tx_vid_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_02),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_03 (
    .refclock           (hdmi_tx_vid_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_03),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_04 (
    .refclock           (dp_rx_phy_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_04),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_05 (
    .refclock           (dp_tx_phy_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_05),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_06 (
    .refclock           (hdmi_rx_phy_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_06),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_07 (
    .refclock           (hdmi_tx_phy_clk_out),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_07),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_08 (
    .refclock           (sdi_rx_clkout_dbg),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_08),
    .too_slow           (),
    .valid              ()
);

mr_rate_detect u_clk_dbg_09 (
    .refclock           (sdi_tx_clkout_dbg),
    .measure_clk        (core_refclk_100),
    .reset              (1'b0),
    .refclock_measure   (clk_dbg_09),
    .too_slow           (),
    .valid              ()
);


// -------------------------------------------------------------------------
// DP Subsystem
// -------------------------------------------------------------------------
agi_dp_demo dp_ss (                  
    .cpu_resetn                             (cpu_resetn             ),
    .user_pb                                (user_pb                ),
    .fgt_refclk_150                         (fgt_refclk_150         ),
    .core_refclk_100                        (core_refclk_100        ),

    .emif_core_init_n_o                     (emif_core_init_n_tmp   ),
    .sysclk_ready_o                         (sysclk_ready_tmp       ),
    .sysclk_startup_i                       (sysclk_startup_tmp     ),

    .dp_rx_phy_clk_out                      (dp_rx_phy_clk_out      ),
    .dp_tx_phy_clk_out                      (dp_tx_phy_clk_out      ),
    .dp_tx_vid_clk_out                      (dp_tx_vid_clk_out      ),                                            
    .clk_300_out                            (clk_300                ),
    .sys_resetn_out                         (sys_resetn             ),
    .cold_resetn                            (cold_resetn            ),
    .cal_done_rst_n                         (cal_done_rst_n         ),
    .bank_2b_cal_done_rst_n                 (bank_2b_cal_done_rst_n ),
    .bank_3a_cal_done_rst_n                 (bank_3a_cal_done_rst_n ),

    .mgmt_reset_hdmi                        (mgmt_reset_hdmi        ),
    .cold_reset_hdmi                        (cold_reset_hdmi        ),

    .mgmt_reset_sdi                         (mgmt_reset_sdi         ),
    .cold_reset_sdi                         (cold_reset_sdi         ),

    .hdmi_rss_req                           (hdmi_rss_req           ),
    .hdmi_rss_grant                         (hdmi_rss_grant         ),
    .hdmi_rss_clk                           (hdmi_rss_clk           ),
          
    .dp_tx_actv_dim_o                       (dp_tx_actv_dim         ),         
    .dp_tx_new_actv_dim_i                   (dp_tx_new_actv_dim     ),
    .dp_tx_fps_i                            (dp_tx_fps              ),
                    
    .dp_rx_tdata                            (dp_rx_tdata            ),
    .dp_rx_tuser                            (dp_rx_tuser            ),
    .dp_rx_tvalid                           (dp_rx_tvalid           ),
    .dp_rx_tready                           (dp_rx_tready           ),
    .dp_rx_tlast                            (dp_rx_tlast            ),
    
    .dp_tx_tdata                            (dp_tx_tdata            ),
    .dp_tx_tuser                            (dp_tx_tuser            ),
    .dp_tx_tvalid                           (dp_tx_tvalid           ),
    .dp_tx_tready                           (dp_tx_tready           ),
    .dp_tx_tlast                            (dp_tx_tlast            ),

    .board_i2c_scl                          (board_i2c_scl          ),
    .board_i2c_sda                          (board_i2c_sda          ),

    .fmc_rx_p                               (fmc_rx_p               ),
    .fmc_rx_n                               (fmc_rx_n               ),
    .fmc_dp_rx_hpd                          (fmc_dp_rx_hpd          ),
    .fmc_dp_rx_cable_detect                 (fmc_dp_rx_cable_detect ),
    .fmc_dp_rx_pwr_detect                   (fmc_dp_rx_pwr_detect   ),
    .fmc_dp_rx_aux_in                       (fmc_dp_rx_aux_in       ),
    .fmc_dp_rx_aux_out                      (fmc_dp_rx_aux_out      ),
    .fmc_dp_rx_aux_oe                       (fmc_dp_rx_aux_oe       ),
                                            
    .fmc_tx_p                               (fmc_tx_p               ),
    .fmc_tx_n                               (fmc_tx_n               ),
    .fmc_dp_tx_cad                          (fmc_dp_tx_cad          ),
    .fmc_dp_tx_hpd                          (fmc_dp_tx_hpd          ),
    .fmc_dp_tx_aux_in                       (fmc_dp_tx_aux_in       ),
    .fmc_dp_tx_aux_out                      (fmc_dp_tx_aux_out      ),
    .fmc_dp_tx_aux_oe                       (fmc_dp_tx_aux_oe       )
);

// -------------------------------------------------------------------------
// VVP Pipe Subsystem
// -------------------------------------------------------------------------
top_vvp_pipe vvp_ss (                  
    .core_refclk_100                        (core_refclk_100         ),
    .clk_300                                (clk_300                 ),
    .sys_resetn                             (sys_resetn              ),
    .cold_resetn                            (cold_resetn             ),
    .cal_done_rst_n                         (cal_done_rst_n          ),
    .bank_2b_cal_done_rst_n                 (bank_2b_cal_done_rst_n  ),
    .bank_3a_cal_done_rst_n                 (bank_3a_cal_done_rst_n  ),

    .emif_core_init_n_i                     (emif_core_init_n_tmp    ),

    .clk_dbg_00                             (clk_dbg_00              ),
    .clk_dbg_01                             (clk_dbg_01              ),
    .clk_dbg_02                             (clk_dbg_02              ),
    .clk_dbg_03                             (clk_dbg_03              ),
    .clk_dbg_04                             (clk_dbg_04              ),
    .clk_dbg_05                             (clk_dbg_05              ),
    .clk_dbg_06                             (clk_dbg_06              ),
    .clk_dbg_07                             (clk_dbg_07              ),
    .clk_dbg_08                             (clk_dbg_08              ),
    .clk_dbg_09                             (clk_dbg_09              ),

    .hdmi_tx_actv_dim_i                     (hdmi_tx_actv_dim        ),
    .hdmi_tx_new_actv_dim_o                 (hdmi_tx_new_actv_dim    ),
    .hdmi_tx_fps_o                          (hdmi_tx_fps             ),    

    .dp_tx_actv_dim_i                       (dp_tx_actv_dim          ),
    .dp_tx_new_actv_dim_o                   (dp_tx_new_actv_dim      ),
    .dp_tx_fps_o                            (dp_tx_fps               ),    

    .sdi_tx_actv_dim_i                      (sdi_tx_actv_dim         ),
    .sdi_tx_new_actv_dim_o                  (sdi_tx_new_actv_dim     ),
    .sdi_tx_fps_o                           (sdi_tx_fps              ),    

    .hdmi_rx_tdata                          (hdmi_rx_tdata           ),
    .hdmi_rx_tuser                          (hdmi_rx_tuser           ),
    .hdmi_rx_tvalid                         (hdmi_rx_tvalid          ),
    .hdmi_rx_tready                         (hdmi_rx_tready          ),
    .hdmi_rx_tlast                          (hdmi_rx_tlast           ),
                                            
    .hdmi_tx_tdata                          (hdmi_tx_tdata           ),
    .hdmi_tx_tuser                          (hdmi_tx_tuser           ),
    .hdmi_tx_tvalid                         (hdmi_tx_tvalid          ),
    .hdmi_tx_tready                         (hdmi_tx_tready          ),
    .hdmi_tx_tlast                          (hdmi_tx_tlast           ),
    
    .dp_rx_tdata                            (dp_rx_tdata             ),
    .dp_rx_tuser                            (dp_rx_tuser             ),
    .dp_rx_tvalid                           (dp_rx_tvalid            ),
    .dp_rx_tready                           (dp_rx_tready            ),
    .dp_rx_tlast                            (dp_rx_tlast             ),
    
    .dp_tx_tdata                            (dp_tx_tdata             ),
    .dp_tx_tuser                            (dp_tx_tuser             ),
    .dp_tx_tvalid                           (dp_tx_tvalid            ),
    .dp_tx_tready                           (dp_tx_tready            ),
    .dp_tx_tlast                            (dp_tx_tlast             ),

    .sdi_rx_tdata                           (sdi_rx_tdata            ),
    .sdi_rx_tuser                           (sdi_rx_tuser            ),
    .sdi_rx_tvalid                          (sdi_rx_tvalid           ),
    .sdi_rx_tready                          (sdi_rx_tready           ),
    .sdi_rx_tlast                           (sdi_rx_tlast            ),
    
    .sdi_tx_tdata                           (sdi_tx_tdata            ),
    .sdi_tx_tuser                           (sdi_tx_tuser            ),
    .sdi_tx_tvalid                          (sdi_tx_tvalid           ),
    .sdi_tx_tready                          (sdi_tx_tready           ),
    .sdi_tx_tlast                           (sdi_tx_tlast            ),
                                                                                        
    .mem_pll_ref_clk                        (mem_pll_ref_clk         ),
    .mem_oct_rzqin                          (mem_oct_rzqin           ),
    .mem_ck                                 (mem_ck                  ),
    .mem_ck_n                               (mem_ck_n                ),
    .mem_a                                  (mem_a                   ),
    .mem_act_n                              (mem_act_n               ),
    .mem_ba                                 (mem_ba                  ),
    .mem_bg                                 (mem_bg                  ),
    .mem_cke                                (mem_cke                 ),
    .mem_cs_n                               (mem_cs_n                ),
    .mem_odt                                (mem_odt                 ),
    .mem_reset_n                            (mem_reset_n             ),
    .mem_par                                (mem_par                 ),
    .mem_alert_n                            (mem_alert_n             ),
    .mem_dqs                                (mem_dqs                 ),
    .mem_dqs_n                              (mem_dqs_n               ),
    .mem_dbi_n                              (mem_dbi_n               ),
    .mem_dq                                 (mem_dq                  ),

    .bank_2b_mem_pll_ref_clk                (bank_2b_mem_pll_ref_clk ),
    .bank_2b_mem_oct_rzqin                  (bank_2b_mem_oct_rzqin   ),
    .bank_2b_mem_ck                         (bank_2b_mem_ck          ),
    .bank_2b_mem_ck_n                       (bank_2b_mem_ck_n        ),
    .bank_2b_mem_a                          (bank_2b_mem_a           ),
    .bank_2b_mem_act_n                      (bank_2b_mem_act_n       ),
    .bank_2b_mem_ba                         (bank_2b_mem_ba          ),
    .bank_2b_mem_bg                         (bank_2b_mem_bg          ),
    .bank_2b_mem_cke                        (bank_2b_mem_cke         ),
    .bank_2b_mem_cs_n                       (bank_2b_mem_cs_n        ),
    .bank_2b_mem_odt                        (bank_2b_mem_odt         ),
    .bank_2b_mem_reset_n                    (bank_2b_mem_reset_n     ),
    .bank_2b_mem_par                        (bank_2b_mem_par         ),
    .bank_2b_mem_alert_n                    (bank_2b_mem_alert_n     ),
    .bank_2b_mem_dqs                        (bank_2b_mem_dqs         ),
    .bank_2b_mem_dqs_n                      (bank_2b_mem_dqs_n       ),
    .bank_2b_mem_dbi_n                      (bank_2b_mem_dbi_n       ),
    .bank_2b_mem_dq                         (bank_2b_mem_dq          ),
    
    .bank_3a_mem_pll_ref_clk                (bank_3a_mem_pll_ref_clk ),
    .bank_3a_mem_oct_rzqin                  (bank_3a_mem_oct_rzqin   ),
    .bank_3a_mem_ck                         (bank_3a_mem_ck          ),
    .bank_3a_mem_ck_n                       (bank_3a_mem_ck_n        ),
    .bank_3a_mem_a                          (bank_3a_mem_a           ),
    .bank_3a_mem_act_n                      (bank_3a_mem_act_n       ),
    .bank_3a_mem_ba                         (bank_3a_mem_ba          ),
    .bank_3a_mem_bg                         (bank_3a_mem_bg          ),
    .bank_3a_mem_cke                        (bank_3a_mem_cke         ),
    .bank_3a_mem_cs_n                       (bank_3a_mem_cs_n        ),
    .bank_3a_mem_odt                        (bank_3a_mem_odt         ),
    .bank_3a_mem_reset_n                    (bank_3a_mem_reset_n     ),
    .bank_3a_mem_par                        (bank_3a_mem_par         ),
    .bank_3a_mem_alert_n                    (bank_3a_mem_alert_n     ),
    .bank_3a_mem_dqs                        (bank_3a_mem_dqs         ),
    .bank_3a_mem_dqs_n                      (bank_3a_mem_dqs_n       ),
    .bank_3a_mem_dbi_n                      (bank_3a_mem_dbi_n       ),
    .bank_3a_mem_dq                         (bank_3a_mem_dq          )
);

sm_axi_tmds_demo hdmi_ss (
    .syspll_refclk                          (syspll_refclk           ),
    .phy_refclk_rx                          (phy_refclk_rx           ),
    .clk_100                                (core_refclk_100         ),
    .phy_refclk_tx                          (phy_refclk_tx           ),

    .sysclk_ready_i                         (sysclk_ready_tmp        ),
    .sysclk_startup_o                       (sysclk_startup_tmp      ),
    
    .i2c_max10_scl                          (i2c_max10_scl           ),
    .i2c_max10_sda                          (i2c_max10_sda           ),
    .i2c_clocks_scl                         (i2c_clocks_scl          ),
    .i2c_clocks_sda                         (i2c_clocks_sda          ),
    .i2c_hdmi_sda                           (i2c_hdmi_sda            ),
    .i2c_hdmi_scl                           (i2c_hdmi_scl            ),

    .hdmi_tx_actv_dim_o                     (hdmi_tx_actv_dim        ),
    .hdmi_tx_new_actv_dim_i                 (hdmi_tx_new_actv_dim    ),
    .hdmi_tx_fps_i                          (hdmi_tx_fps             ),       

    .hdmi_rx_phy_clk_out                    (hdmi_rx_phy_clk_out     ),
    .hdmi_tx_phy_clk_out                    (hdmi_tx_phy_clk_out     ),
    .hdmi_tx_vid_clk_out                    (hdmi_tx_vid_clk_out     ),
    .vid_clk                                (clk_300                 ),
    .mgmt_reset_hdmi                        (mgmt_reset_hdmi         ),
    .cold_reset_hdmi                        (cold_reset_hdmi         ), 

    .hdmi_rss_req                           (hdmi_rss_req            ),
    .hdmi_rss_grant                         (hdmi_rss_grant          ),
    .hdmi_rss_clk                           (hdmi_rss_clk            ),

    .hdmi_rx_tdata                          (hdmi_rx_tdata           ),
    .hdmi_rx_tuser                          (hdmi_rx_tuser           ),
    .hdmi_rx_tvalid                         (hdmi_rx_tvalid          ),
    .hdmi_rx_tready                         (hdmi_rx_tready          ),
    .hdmi_rx_tlast                          (hdmi_rx_tlast           ),

    .hdmi_tx_tdata                          (hdmi_tx_tdata           ),
    .hdmi_tx_tuser                          (hdmi_tx_tuser           ),
    .hdmi_tx_tvalid                         (hdmi_tx_tvalid          ),
    .hdmi_tx_tready                         (hdmi_tx_tready          ),
    .hdmi_tx_tlast                          (hdmi_tx_tlast           ),

    .fmc_rx_p                               (hdmi_fmc_rx_p           ),
    .fmc_rx_n                               (hdmi_fmc_rx_n           ),
    .fmc_tx_p                               (hdmi_fmc_tx_p           ),
    .fmc_tx_n                               (hdmi_fmc_tx_n           ),

    .hdmi_rx_hpd_n                          (hdmi_rx_hpd_n_tmp       ),
    .hdmi_5v_detect_n                       (hdmi_5v_detect_n        ),
    .hdmi_rx_i2c_sda                        (hdmi_rx_i2c_sda         ),
    .hdmi_rx_i2c_scl                        (hdmi_rx_i2c_scl         ),

    .hdmi_tx_hpd_n                          (hdmi_tx_hpd_n           ),
    .hdmi_tx_i2c_sda                        (hdmi_tx_i2c_sda         ),
    .hdmi_tx_i2c_scl                        (hdmi_tx_i2c_scl         )
);


sdi_ii_agi_demo sdi_ss(
    .clk_100mhz                             (core_refclk_100            ),
    .syspll_refclk                          (sdi_syspll_refclk          ),
    .xcvr_refclk_1485                       (xcvr_refclk_1485           ),
    .txpll_refclk                           (txpll_refclk               ),

    .sysclk_ready_i                         (sysclk_ready_tmp           ),

    .i2c_sdi_sda                            (i2c_sdi_sda                ),
    .i2c_sdi_scl                            (i2c_sdi_scl                ),

    .axi4s_clk                              (clk_300                    ),
    .mgmt_reset_sdi                         (mgmt_reset_sdi             ),
    .cold_reset_sdi                         (cold_reset_sdi             ),
    .sdi_rx_clkout_dbg                      (sdi_rx_clkout_dbg          ),
    .sdi_tx_clkout_dbg                      (sdi_tx_clkout_dbg          ),

    .sdi_rx_tdata                           (sdi_rx_tdata               ),
    .sdi_rx_tuser                           (sdi_rx_tuser               ),
    .sdi_rx_tvalid                          (sdi_rx_tvalid              ),
    .sdi_rx_tready                          (sdi_rx_tready              ),
    .sdi_rx_tlast                           (sdi_rx_tlast               ),

    .sdi_tx_tdata                           (sdi_tx_tdata               ),
    .sdi_tx_tuser                           (sdi_tx_tuser               ),
    .sdi_tx_tvalid                          (sdi_tx_tvalid              ),
    .sdi_tx_tready                          (sdi_tx_tready              ),
    .sdi_tx_tlast                           (sdi_tx_tlast               ),

    .sdi_tx_actv_dim_o                      (sdi_tx_actv_dim            ),
    .sdi_tx_new_actv_dim_i                  (sdi_tx_new_actv_dim        ),
    .sdi_tx_fps_i                           (sdi_tx_fps                 ), 

    .fmc_rx2_p                              (fmc_rx2_p                  ),
    .fmc_rx2_n                              (fmc_rx2_n                  ),

    .fmc_tx0_p                              (fmc_tx0_p                  ),
    .fmc_tx0_n                              (fmc_tx0_n                  ),

    .lmk03328_pdn                           (lmk03328_pdn               ),
    .fmc_lmh1983_init                       (fmc_lmh1983_init           ),
    .fmc_fpga_fldn                          (fmc_fpga_fldn              ),
    .fmc_fpga_vsyncn                        (fmc_fpga_vsyncn            ),
    .fmc_fpga_hsyncn                        (fmc_fpga_hsyncn            )
);

endmodule




