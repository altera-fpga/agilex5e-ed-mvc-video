# Project setup with top.qsf 
set prj_top [file join .. .. .. top.qsf]
project_open $prj_top

post_message [pwd]
puts [pwd]

# Project setup with qsys system
set prj_vds [file join src vds vvp_pipe vvp_pipe.vds]

post_message [pwd]
puts [pwd]

# This package contains vsd::* commands
load_package vds

# project_new top
# 
# Info: Current script was exported with Quartus Prime Pro Release 26.1.1 Build 130
# 
regexp {[\.0-9]+} $quartus(version) import_release
if {$import_release != "26.1.1"} {
	post_message -type error "Cannot import script exported from Quartus Prime Pro version \"26.1.1 Build 130\" into current release \"$import_release\".\n\tOpen, upgrade, and validate the original system in VDS to ensure design correctness."
	return
}
proc get_current_hier {} {
set current_hier [vds::get_current_hier]
if {$current_hier == "|"} {
	set current_hier ""
}
return $current_hier
}

vds::create_system vvp_pipe
vds::create_cell -properties { parameters:enableDebugReset {true} parameters:resetSlave {onchip_mem.s1}  } -vlnv altera.com:ip:intel_niosv_m:26.0.0 cpu
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 cpu_reset_bridge
vds::create_cell -properties { parameters:readBufferDepth {1024} parameters:readIRQThreshold {1} parameters:writeBufferDepth {1024} parameters:writeIRQThreshold {1}  } -vlnv altera.com:ip:altera_avalon_jtag_uart:19.3.2 jtag_uart
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 mgmt_clk
vds::create_cell -properties { parameters:initializationFileName {} parameters:memorySize {131072}  } -vlnv altera.com:ip:intel_onchip_memory:2.0.0 onchip_mem
vds::create_cell -properties { parameters:NUM_RESET_INPUTS {2} parameters:RESET_REQUEST_PRESENT {1}  } -vlnv altera.com:ip:altera_reset_controller:19.2.4 rst_controller
vds::create_cell -vlnv altera.com:ip:altera_reset_translator:19.2.1 rst_translator
vds::create_cell -properties { parameters:periodUnits {USEC}  } -vlnv altera.com:ip:altera_avalon_timer:19.3.5 sys_clock_timer
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_00
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_01
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_02
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_03
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_04
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_05
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_06
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_07
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_08
vds::create_cell -properties { parameters:direction {Input} parameters:width {24}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_clk_dbg_09
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_actv_dim_dptx
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_actv_dim_hdmitx
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_actv_dim_sditx
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {300000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 vvp_pipe_axis_clk
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 vvp_pipe_axis_reset_bridge
vds::create_cell -properties { parameters:BPS {10} parameters:C_OMNI_CAP_VERSION {1} parameters:HORIZ_ALGORITHM {FILTERED} parameters:HORIZ_ENABLE_LUMA_ADAPT {1} parameters:MAX_WIDTH {4096} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL_IN {2} parameters:PIXELS_IN_PARALLEL_OUT {2} parameters:SUPPORT_420_TO_444 {1} parameters:SUPPORT_444_PASS {1} parameters:VERT_ALGORITHM {FILTERED} parameters:VERT_ENABLE_LUMA_ADAPT {1}  } -vlnv altera.com:ip:intel_vvp_crs:24.8.0 vvp_pipe_crs_dp_rx
vds::create_cell -properties { parameters:BPS {10} parameters:C_OMNI_CAP_VERSION {1} parameters:HORIZ_ALGORITHM {FILTERED} parameters:HORIZ_ENABLE_LUMA_ADAPT {1} parameters:MAX_WIDTH {4096} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL_IN {2} parameters:PIXELS_IN_PARALLEL_OUT {2} parameters:SUPPORT_420_TO_444 {1} parameters:SUPPORT_444_PASS {1} parameters:VERT_ALGORITHM {FILTERED} parameters:VERT_ENABLE_LUMA_ADAPT {1}  } -vlnv altera.com:ip:intel_vvp_crs:24.8.0 vvp_pipe_crs_hdmi_rx
vds::create_cell -properties { parameters:BPS {10} parameters:C_OMNI_CAP_VERSION {1} parameters:HORIZ_ALGORITHM {FILTERED} parameters:HORIZ_ENABLE_LUMA_ADAPT {1} parameters:MAX_WIDTH {4096} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL_IN {2} parameters:PIXELS_IN_PARALLEL_OUT {2} parameters:SUPPORT_420_TO_444 {1} parameters:SUPPORT_444_PASS {1} parameters:VERT_ALGORITHM {FILTERED} parameters:VERT_ENABLE_LUMA_ADAPT {1}  } -vlnv altera.com:ip:intel_vvp_crs:24.8.0 vvp_pipe_crs_sdi_rx
vds::create_cell -properties { parameters:BPS {10} parameters:C_OMNI_CAP_VERSION {1} parameters:ENABLE_DEBUG {1} parameters:HORIZ_ALGORITHM {FILTERED} parameters:MAX_WIDTH {4096} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL_IN {2} parameters:PIXELS_IN_PARALLEL_OUT {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:SUPPORT_422_TO_444 {0} parameters:SUPPORT_444_PASS {1} parameters:SUPPORT_444_TO_420 {1} parameters:SUPPORT_444_TO_422 {1} parameters:VERT_ALGORITHM {FILTERED}  } -vlnv altera.com:ip:intel_vvp_crs:24.8.0 vvp_pipe_crs_sdi_tx
vds::create_cell -properties { parameters:BPS_IN {10} parameters:BPS_OUT {10} parameters:COEFFICIENT_INT_BITS {2} parameters:ENABLE_DEBUG {1} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:SUMMAND_INT_BITS {11} parameters:SUMMAND_SIGNED {1}  } -vlnv altera.com:ip:intel_vvp_csc:24.8.0 vvp_pipe_csc_dp_rx
vds::create_cell -properties { parameters:BPS_IN {10} parameters:BPS_OUT {10} parameters:COEFFICIENT_INT_BITS {2} parameters:ENABLE_DEBUG {1} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:SUMMAND_INT_BITS {11} parameters:SUMMAND_SIGNED {1}  } -vlnv altera.com:ip:intel_vvp_csc:24.8.0 vvp_pipe_csc_hdmi_rx
vds::create_cell -properties { parameters:BPS_IN {10} parameters:BPS_OUT {10} parameters:COEFFICIENT_INT_BITS {2} parameters:ENABLE_DEBUG {1} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:SUMMAND_INT_BITS {11} parameters:SUMMAND_SIGNED {1}  } -vlnv altera.com:ip:intel_vvp_csc:24.8.0 vvp_pipe_csc_sdi_rx
vds::create_cell -properties { parameters:BPS_IN {10} parameters:BPS_OUT {10} parameters:COEFFICIENT_INT_BITS {2} parameters:ENABLE_DEBUG {1} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:SUMMAND_INT_BITS {11} parameters:SUMMAND_SIGNED {1}  } -vlnv altera.com:ip:intel_vvp_csc:24.8.0 vvp_pipe_csc_sdi_tx
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:MAX_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1}  } -vlnv altera.com:ip:intel_vvp_dil:24.8.0 vvp_pipe_dil_dp_rx
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:MAX_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1}  } -vlnv altera.com:ip:intel_vvp_dil:24.8.0 vvp_pipe_dil_hdmi_rx
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:MAX_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1}  } -vlnv altera.com:ip:intel_vvp_dil:24.8.0 vvp_pipe_dil_sdi_rx
vds::create_cell -properties { parameters:BURSTCOUNT_WIDTH {7} parameters:DATA_WIDTH {256} parameters:ENABLE_SLAVE_PORT {false} parameters:MASTER_ADDRESS_WIDTH {33} parameters:MAX_PENDING_READS {8} parameters:SLAVE_ADDRESS_WIDTH {27}  } -vlnv altera.com:ip:altera_address_span_extender:19.2.1 vvp_pipe_emif_addr
vds::create_cell -properties { parameters:BURSTCOUNT_WIDTH {7} parameters:DATA_WIDTH {256} parameters:ENABLE_SLAVE_PORT {false} parameters:MASTER_ADDRESS_WIDTH {33} parameters:MAX_PENDING_READS {8} parameters:SLAVE_ADDRESS_WIDTH {27}  } -vlnv altera.com:ip:altera_address_span_extender:19.2.1 vvp_pipe_emif_addr_ch2
vds::create_cell -properties { parameters:BURSTCOUNT_WIDTH {7} parameters:DATA_WIDTH {256} parameters:ENABLE_SLAVE_PORT {false} parameters:MASTER_ADDRESS_WIDTH {33} parameters:MAX_PENDING_READS {8} parameters:SLAVE_ADDRESS_WIDTH {27}  } -vlnv altera.com:ip:altera_address_span_extender:19.2.1 vvp_pipe_emif_addr_ch3
vds::create_cell -properties { parameters:CTRL_DM_EN {true} parameters:EX_DESIGN_USER_PLL_OUTPUT_FREQ_MHZ {290.0} parameters:JEDEC_OVERRIDE_TABLE_PARAM_NAME {{MEM_CL_CYC} {MEM_CWL_CYC} {MEM_TRFC_NS}} parameters:MEM_TRFC_NS {350} parameters:MEM_CL_CYC {12.0} parameters:MEM_CWL_CYC {11.0} parameters:MEM_DIE_DENSITY_GBITS {16} parameters:MEM_DIE_DQ_WIDTH {8} parameters:MEM_DQ_VREF {35} parameters:MEM_FINE_GRANULARITY_REFRESH_MODE {1.0} parameters:MEM_ODT_DQ_X_IDLE {off} parameters:MEM_ODT_DQ_X_NON_TGT_RD {off} parameters:MEM_ODT_DQ_X_NON_TGT_WR {off} parameters:MEM_ODT_DQ_X_RON {7} parameters:MEM_ODT_DQ_X_TGT_WR {4} parameters:MEM_ODT_NOM {off} parameters:MEM_ODT_PARK {4} parameters:MEM_ODT_WR {off} parameters:MEM_OPERATING_FREQ_MHZ {800} parameters:MEM_PAGE_SIZE {1024.0} parameters:MEM_RD_PREAMBLE_MODE {1.0} parameters:MEM_SPEEDBIN {3200AA} parameters:MEM_TCCD_L_NS {6.25} parameters:MEM_TCCD_S_NS {5.0} parameters:MEM_TCKESR_CYC {5.0} parameters:MEM_TCKE_NS {5.0} parameters:MEM_TCKSRE_NS {10.0} parameters:MEM_TCKSRX_NS {10.0} parameters:MEM_TCK_CL_CWL_MAX_NS {1.5} parameters:MEM_TCK_CL_CWL_MIN_NS {1.25} parameters:MEM_TCPDED_NS {5.0} parameters:MEM_TDQSCK_MAX_MIN_NS {0.16} parameters:MEM_TFAW_NS {25.0} parameters:MEM_TMOD_NS {30.0} parameters:MEM_TMPRR_NS {1.25} parameters:MEM_TMRD_NS {10.0} parameters:MEM_TRAS_MAX_NS {70200.0} parameters:MEM_TRAS_MIN_NS {32.0} parameters:MEM_TRAS_NS {32.0} parameters:MEM_TRCD_NS {13.75} parameters:MEM_TRC_NS {45.75} parameters:MEM_TREFI_NS {7800.0} parameters:MEM_TRP_NS {13.75} parameters:MEM_TRRD_L_NS {5.0} parameters:MEM_TRRD_S_NS {5.0} parameters:MEM_TRTP_NS {7.5} parameters:MEM_TWR_CRC_DM_NS {6.25} parameters:MEM_TWR_NS {15.0} parameters:MEM_TWTR_L_CRC_DM_NS {6.25} parameters:MEM_TWTR_L_NS {7.5} parameters:MEM_TWTR_S_CRC_DM_NS {6.25} parameters:MEM_TWTR_S_NS {2.5} parameters:MEM_TXP_NS {6.0} parameters:MEM_TXS_DLL_NS {1280.0} parameters:MEM_TXS_NS {360.0} parameters:MEM_TZQCS_NS {160.0} parameters:MEM_TZQINIT_CYC {1024.0} parameters:MEM_TZQOPER_CYC {512.0} parameters:MEM_VREF_DQ_X_VALUE {67.75} parameters:MEM_WR_PREAMBLE_MODE {1.0} parameters:PHY_AC_TX_EQUALIZATION {OFF} parameters:PHY_CK_TX_EQUALIZATION {OFF} parameters:PHY_CS_TX_EQUALIZATION {OFF} parameters:PHY_DQ_TX_EQUALIZATION {OFF} parameters:PHY_MAINBAND_ACCESS_MODE {SYNC} parameters:PHY_MAINBAND_ACCESS_MODE_AUTOSET_EN {false} parameters:PHY_REFCLK_FREQ_MHZ {150.0} parameters:PHY_REFCLK_FREQ_MHZ_AUTOSET_EN {false} parameters:PHY_SIDEBAND_ACCESS_MODE {FABRIC} parameters:PHY_SWIZZLE_MAP {BYTE_SWIZZLE_CH0=0,X,X,X,1,2,3,ECC; PIN_SWIZZLE_CH0_ECC=0,2,6,4,1,3,7,5; PIN_SWIZZLE_CH0_DQS3=24,25,26,27,28,29,30,31; PIN_SWIZZLE_CH0_DQS2=16,17,18,19,20,21,22,23; PIN_SWIZZLE_CH0_DQS1=12,14,13,10,8,11,15,9; PIN_SWIZZLE_CH0_DQS0=2,0,6,4,7,3,5,1;} parameters:PHY_TERM_X_AC_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_AC_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CK_OUTPUT_IO_STD_TYPE {DF_SSTL} parameters:PHY_TERM_X_CK_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CS_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_CS_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQS_IO_STD_TYPE {DF_POD} parameters:PHY_TERM_X_DQ_IO_STD_TYPE {POD} parameters:PHY_TERM_X_DQ_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQ_VREF {68.3} parameters:PHY_TERM_X_GPIO_IO_STD_TYPE {LVCMOS} parameters:PHY_TERM_X_REFCLK_IO_STD_TYPE {TRUE_DIFF} parameters:PHY_TERM_X_R_S_AC_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CK_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CS_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_DQ_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_T_DQ_INPUT_OHM {RT_50_OHM_CAL} parameters:PHY_TERM_X_R_T_GPIO_INPUT_OHM {RT_OFF} parameters:PHY_TERM_X_R_T_REFCLK_INPUT_OHM {RT_DIFF} parameters:MEM_OPERATING_FREQ_MHZ_AUTOSET_EN {false} parameters:MEM_CHANNEL_ECC_DQ_WIDTH {8}  } -vlnv altera.com:ip:emif_io96b_ddr4comp:4.3.0 vvp_pipe_emif_ddr4
vds::create_cell -vlnv :ip:emif_ph2_axil_driver:1.0.0 vvp_pipe_emif_ddr4_cal
vds::create_cell -vlnv :ip:emif_ph2_axil_driver:1.0.0 vvp_pipe_emif_ddr4_cal_ch2
vds::create_cell -vlnv :ip:emif_ph2_axil_driver:1.0.0 vvp_pipe_emif_ddr4_cal_ch3
vds::create_cell -properties { parameters:CTRL_DM_EN {true} parameters:EX_DESIGN_USER_PLL_OUTPUT_FREQ_MHZ {290.0} parameters:JEDEC_OVERRIDE_TABLE_PARAM_NAME {{MEM_CL_CYC} {MEM_CWL_CYC} {MEM_TRFC_NS}} parameters:MEM_TRFC_NS {350} parameters:MEM_CL_CYC {12.0} parameters:MEM_CWL_CYC {11.0} parameters:MEM_DIE_DENSITY_GBITS {16} parameters:MEM_DIE_DQ_WIDTH {8} parameters:MEM_DQ_VREF {35} parameters:MEM_FINE_GRANULARITY_REFRESH_MODE {1.0} parameters:MEM_ODT_DQ_X_IDLE {off} parameters:MEM_ODT_DQ_X_NON_TGT_RD {off} parameters:MEM_ODT_DQ_X_NON_TGT_WR {off} parameters:MEM_ODT_DQ_X_RON {7} parameters:MEM_ODT_DQ_X_TGT_WR {4} parameters:MEM_ODT_NOM {off} parameters:MEM_ODT_PARK {4} parameters:MEM_ODT_WR {off} parameters:MEM_OPERATING_FREQ_MHZ {800} parameters:MEM_PAGE_SIZE {1024.0} parameters:MEM_RD_PREAMBLE_MODE {1.0} parameters:MEM_SPEEDBIN {3200AA} parameters:MEM_TCCD_L_NS {6.25} parameters:MEM_TCCD_S_NS {5.0} parameters:MEM_TCKESR_CYC {5.0} parameters:MEM_TCKE_NS {5.0} parameters:MEM_TCKSRE_NS {10.0} parameters:MEM_TCKSRX_NS {10.0} parameters:MEM_TCK_CL_CWL_MAX_NS {1.5} parameters:MEM_TCK_CL_CWL_MIN_NS {1.25} parameters:MEM_TCPDED_NS {5.0} parameters:MEM_TDQSCK_MAX_MIN_NS {0.16} parameters:MEM_TFAW_NS {25.0} parameters:MEM_TMOD_NS {30.0} parameters:MEM_TMPRR_NS {1.25} parameters:MEM_TMRD_NS {10.0} parameters:MEM_TRAS_MAX_NS {70200.0} parameters:MEM_TRAS_MIN_NS {32.0} parameters:MEM_TRAS_NS {32.0} parameters:MEM_TRCD_NS {13.75} parameters:MEM_TRC_NS {45.75} parameters:MEM_TREFI_NS {7800.0} parameters:MEM_TRP_NS {13.75} parameters:MEM_TRRD_L_NS {5.0} parameters:MEM_TRRD_S_NS {5.0} parameters:MEM_TRTP_NS {7.5} parameters:MEM_TWR_CRC_DM_NS {6.25} parameters:MEM_TWR_NS {15.0} parameters:MEM_TWTR_L_CRC_DM_NS {6.25} parameters:MEM_TWTR_L_NS {7.5} parameters:MEM_TWTR_S_CRC_DM_NS {6.25} parameters:MEM_TWTR_S_NS {2.5} parameters:MEM_TXP_NS {6.0} parameters:MEM_TXS_DLL_NS {1280.0} parameters:MEM_TXS_NS {360.0} parameters:MEM_TZQCS_NS {160.0} parameters:MEM_TZQINIT_CYC {1024.0} parameters:MEM_TZQOPER_CYC {512.0} parameters:MEM_VREF_DQ_X_VALUE {67.75} parameters:MEM_WR_PREAMBLE_MODE {1.0} parameters:PHY_AC_TX_EQUALIZATION {OFF} parameters:PHY_CK_TX_EQUALIZATION {OFF} parameters:PHY_CS_TX_EQUALIZATION {OFF} parameters:PHY_DQ_TX_EQUALIZATION {OFF} parameters:PHY_MAINBAND_ACCESS_MODE {SYNC} parameters:PHY_MAINBAND_ACCESS_MODE_AUTOSET_EN {false} parameters:PHY_REFCLK_FREQ_MHZ {150.0} parameters:PHY_REFCLK_FREQ_MHZ_AUTOSET_EN {false} parameters:PHY_SIDEBAND_ACCESS_MODE {FABRIC} parameters:PHY_SWIZZLE_MAP {BYTE_SWIZZLE_CH0=1,X,X,X,0,2,3,X; PIN_SWIZZLE_CH0_DQS0=7,5,1,3,4,2,0,6; PIN_SWIZZLE_CH0_DQS1=9,15,13,11,14,12,8,10; PIN_SWIZZLE_CH0_DQS2=19,23,21,17,16,18,20,22; PIN_SWIZZLE_CH0_DQS3=31,29,27,25,26,30,24,28;}                                        parameters:PHY_TERM_X_AC_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_AC_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CK_OUTPUT_IO_STD_TYPE {DF_SSTL} parameters:PHY_TERM_X_CK_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CS_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_CS_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQS_IO_STD_TYPE {DF_POD} parameters:PHY_TERM_X_DQ_IO_STD_TYPE {POD} parameters:PHY_TERM_X_DQ_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQ_VREF {68.3} parameters:PHY_TERM_X_GPIO_IO_STD_TYPE {LVCMOS} parameters:PHY_TERM_X_REFCLK_IO_STD_TYPE {TRUE_DIFF} parameters:PHY_TERM_X_R_S_AC_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CK_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CS_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_DQ_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_T_DQ_INPUT_OHM {RT_50_OHM_CAL} parameters:PHY_TERM_X_R_T_GPIO_INPUT_OHM {RT_OFF} parameters:PHY_TERM_X_R_T_REFCLK_INPUT_OHM {RT_DIFF} parameters:MEM_OPERATING_FREQ_MHZ_AUTOSET_EN {false} parameters:CTRL_ECC_AUTOCORRECT_EN {false} parameters:CTRL_ECC_WRITEBACK_EN {false}  } -vlnv altera.com:ip:emif_io96b_ddr4comp:4.3.0 vvp_pipe_emif_ddr4_ch2
vds::create_cell -properties { parameters:CTRL_DM_EN {true} parameters:EX_DESIGN_USER_PLL_OUTPUT_FREQ_MHZ {290.0} parameters:JEDEC_OVERRIDE_TABLE_PARAM_NAME {{MEM_CL_CYC} {MEM_CWL_CYC} {MEM_TRFC_NS}} parameters:MEM_TRFC_NS {350} parameters:MEM_CL_CYC {12.0} parameters:MEM_CWL_CYC {11.0} parameters:MEM_DIE_DENSITY_GBITS {16} parameters:MEM_DIE_DQ_WIDTH {8} parameters:MEM_DQ_VREF {35} parameters:MEM_FINE_GRANULARITY_REFRESH_MODE {1.0} parameters:MEM_ODT_DQ_X_IDLE {off} parameters:MEM_ODT_DQ_X_NON_TGT_RD {off} parameters:MEM_ODT_DQ_X_NON_TGT_WR {off} parameters:MEM_ODT_DQ_X_RON {7} parameters:MEM_ODT_DQ_X_TGT_WR {4} parameters:MEM_ODT_NOM {off} parameters:MEM_ODT_PARK {4} parameters:MEM_ODT_WR {off} parameters:MEM_OPERATING_FREQ_MHZ {800} parameters:MEM_PAGE_SIZE {1024.0} parameters:MEM_RD_PREAMBLE_MODE {1.0} parameters:MEM_SPEEDBIN {3200AA} parameters:MEM_TCCD_L_NS {6.25} parameters:MEM_TCCD_S_NS {5.0} parameters:MEM_TCKESR_CYC {5.0} parameters:MEM_TCKE_NS {5.0} parameters:MEM_TCKSRE_NS {10.0} parameters:MEM_TCKSRX_NS {10.0} parameters:MEM_TCK_CL_CWL_MAX_NS {1.5} parameters:MEM_TCK_CL_CWL_MIN_NS {1.25} parameters:MEM_TCPDED_NS {5.0} parameters:MEM_TDQSCK_MAX_MIN_NS {0.16} parameters:MEM_TFAW_NS {25.0} parameters:MEM_TMOD_NS {30.0} parameters:MEM_TMPRR_NS {1.25} parameters:MEM_TMRD_NS {10.0} parameters:MEM_TRAS_MAX_NS {70200.0} parameters:MEM_TRAS_MIN_NS {32.0} parameters:MEM_TRAS_NS {32.0} parameters:MEM_TRCD_NS {13.75} parameters:MEM_TRC_NS {45.75} parameters:MEM_TREFI_NS {7800.0} parameters:MEM_TRP_NS {13.75} parameters:MEM_TRRD_L_NS {5.0} parameters:MEM_TRRD_S_NS {5.0} parameters:MEM_TRTP_NS {7.5} parameters:MEM_TWR_CRC_DM_NS {6.25} parameters:MEM_TWR_NS {15.0} parameters:MEM_TWTR_L_CRC_DM_NS {6.25} parameters:MEM_TWTR_L_NS {7.5} parameters:MEM_TWTR_S_CRC_DM_NS {6.25} parameters:MEM_TWTR_S_NS {2.5} parameters:MEM_TXP_NS {6.0} parameters:MEM_TXS_DLL_NS {1280.0} parameters:MEM_TXS_NS {360.0} parameters:MEM_TZQCS_NS {160.0} parameters:MEM_TZQINIT_CYC {1024.0} parameters:MEM_TZQOPER_CYC {512.0} parameters:MEM_VREF_DQ_X_VALUE {67.75} parameters:MEM_WR_PREAMBLE_MODE {1.0} parameters:PHY_AC_TX_EQUALIZATION {OFF} parameters:PHY_CK_TX_EQUALIZATION {OFF} parameters:PHY_CS_TX_EQUALIZATION {OFF} parameters:PHY_DQ_TX_EQUALIZATION {OFF} parameters:PHY_MAINBAND_ACCESS_MODE {SYNC} parameters:PHY_MAINBAND_ACCESS_MODE_AUTOSET_EN {false} parameters:PHY_REFCLK_FREQ_MHZ {150.0} parameters:PHY_REFCLK_FREQ_MHZ_AUTOSET_EN {false} parameters:PHY_SIDEBAND_ACCESS_MODE {FABRIC} parameters:PHY_SWIZZLE_MAP {BYTE_SWIZZLE_CH0=0,X,X,X,1,2,3,ECC; PIN_SWIZZLE_CH0_ECC=4,6,2,0,1,7,5,3; PIN_SWIZZLE_CH0_DQS3=26,30,28,24,25,27,29,31; PIN_SWIZZLE_CH0_DQS2=16,20,22,18,23,21,19,17; PIN_SWIZZLE_CH0_DQS1=14,11,12,8,10,9,13,15; PIN_SWIZZLE_CH0_DQS0=2,0,6,4,7,5,3,1;} parameters:PHY_TERM_X_AC_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_AC_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CK_OUTPUT_IO_STD_TYPE {DF_SSTL} parameters:PHY_TERM_X_CK_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_CS_OUTPUT_IO_STD_TYPE {SSTL} parameters:PHY_TERM_X_CS_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQS_IO_STD_TYPE {DF_POD} parameters:PHY_TERM_X_DQ_IO_STD_TYPE {POD} parameters:PHY_TERM_X_DQ_SLEW_RATE {FASTEST} parameters:PHY_TERM_X_DQ_VREF {68.3} parameters:PHY_TERM_X_GPIO_IO_STD_TYPE {LVCMOS} parameters:PHY_TERM_X_REFCLK_IO_STD_TYPE {TRUE_DIFF} parameters:PHY_TERM_X_R_S_AC_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CK_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_CS_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_S_DQ_OUTPUT_OHM {SERIES_34_OHM_CAL} parameters:PHY_TERM_X_R_T_DQ_INPUT_OHM {RT_50_OHM_CAL} parameters:PHY_TERM_X_R_T_GPIO_INPUT_OHM {RT_OFF} parameters:PHY_TERM_X_R_T_REFCLK_INPUT_OHM {RT_DIFF} parameters:MEM_OPERATING_FREQ_MHZ_AUTOSET_EN {false} parameters:MEM_CHANNEL_ECC_DQ_WIDTH {8}  } -vlnv altera.com:ip:emif_io96b_ddr4comp:4.3.0 vvp_pipe_emif_ddr4_ch3
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 vvp_pipe_emif_reset_bridge
vds::create_cell -properties { parameters:BPS {10} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:OUTPUT_GUARD_BAND_LOWER_0 {4} parameters:OUTPUT_GUARD_BAND_LOWER_1 {4} parameters:OUTPUT_GUARD_BAND_LOWER_2 {4} parameters:OUTPUT_GUARD_BAND_UPPER_0 {1019} parameters:OUTPUT_GUARD_BAND_UPPER_1 {1019} parameters:OUTPUT_GUARD_BAND_UPPER_2 {1019} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_guard_bands:24.8.0 vvp_pipe_guard_bands_sdi_tx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_dp_rx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_dp_tx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_hdmi_rx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_hdmi_tx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_sdi_rx
vds::create_cell -properties { parameters:BPS {10} parameters:FIFO_DEPTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2}  } -vlnv altera.com:ip:intel_vvp_fifo:24.5.0 vvp_pipe_intel_fifo_sdi_tx
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_actv_dim_dptx
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_actv_dim_hdmitx
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_actv_dim_sditx
vds::create_cell -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_fps_dpitx
vds::create_cell -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_fps_hdmitx
vds::create_cell -vlnv altera.com:ip:altera_avalon_pio:19.2.4 vvp_pipe_new_fps_sditx
vds::create_cell -properties { parameters:BPS {10} parameters:MAX_HEIGHT {4096} parameters:MEM_BUFF_LINE_STRIDE {49152} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIXELS_IN_PARALLEL {2} parameters:P_AV_MM_DATA_WIDTH {256} parameters:READ_BURST_TARGET {64} parameters:READ_FIFO_DEPTH {512} parameters:WRITE_BURST_TARGET {64} parameters:WRITE_FIFO_DEPTH {512}  } -vlnv altera.com:ip:intel_vvp_vfb:24.8.0 vvp_pipe_vfb_ch3
vds::create_cell -properties { parameters:BPS {10} parameters:EDGE_MIRROR {MIRROR} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:H_TAPS {12} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32} parameters:V_TAPS {12}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_down
vds::create_cell -properties { parameters:BPS {10} parameters:EDGE_MIRROR {MIRROR} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:H_TAPS {12} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32} parameters:V_TAPS {12}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_down_ch2
vds::create_cell -properties { parameters:BPS {10} parameters:EDGE_MIRROR {MIRROR} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:H_TAPS {12} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32} parameters:V_TAPS {12}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_down_ch3
vds::create_cell -properties { parameters:BPS {10} parameters:EDGE_MIRROR {MIRROR} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:MEM_INIT {1} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_LOAD {0} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_up
vds::create_cell -properties { parameters:BPS {10} parameters:EDGE_MIRROR {MIRROR} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:MEM_INIT {1} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_LOAD {0} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_up_ch2
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:H_COEFF_FRAC_BITS {8} parameters:H_PHASES {32} parameters:MAX_IN_WIDTH {4096} parameters:MAX_OUT_WIDTH {4096} parameters:MEM_INIT {1} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_LOAD {0} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:V_COEFF_FRAC_BITS {8} parameters:V_PHASES {32}  } -vlnv altera.com:ip:intel_vvp_scaler:24.8.0 vvp_pipe_vvp_scaler_up_ch3
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:NUM_INPUTS {4} parameters:NUM_OUTPUTS {3} parameters:PIXELS_IN_PARALLEL {2} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:USE_OP_RESP {1}  } -vlnv altera.com:ip:intel_vvp_switch:24.7.0 vvp_pipe_vvp_switch_input
vds::create_cell -properties { parameters:BPS {10} parameters:ENABLE_DEBUG {1} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:NUM_INPUTS {3} parameters:NUM_OUTPUTS {3} parameters:PIXELS_IN_PARALLEL {2} parameters:SEPARATE_SLAVE_CLOCK {1} parameters:USE_OP_RESP {1}  } -vlnv altera.com:ip:intel_vvp_switch:24.7.0 vvp_pipe_vvp_switch_output
vds::create_cell -properties { parameters:BPS {10} parameters:CORE_PATTERN_1 {1} parameters:ENABLE_DEBUG {1} parameters:FIXED_HEIGHT {16384} parameters:FIXED_WIDTH {16384} parameters:NUM_CORES {2} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:RUNTIME_CONTROL {1} parameters:SEPARATE_SLAVE_CLOCK {1}  } -vlnv altera.com:ip:intel_vvp_tpg:24.8.0 vvp_pipe_vvp_tpg
vds::create_cell -properties { parameters:BPS {10} parameters:MAX_HEIGHT {4096} parameters:MEM_BUFF_LINE_STRIDE {49152} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIXELS_IN_PARALLEL {2} parameters:P_AV_MM_DATA_WIDTH {256} parameters:READ_BURST_TARGET {64} parameters:READ_FIFO_DEPTH {512} parameters:WRITE_BURST_TARGET {64} parameters:WRITE_FIFO_DEPTH {512}  } -vlnv altera.com:ip:intel_vvp_vfb:24.8.0 vvp_pipe_vvp_vfb
vds::create_cell -properties { parameters:BPS {10} parameters:MAX_HEIGHT {4096} parameters:MEM_BUFF_LINE_STRIDE {49152} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIXELS_IN_PARALLEL {2} parameters:P_AV_MM_DATA_WIDTH {256} parameters:READ_BURST_TARGET {64} parameters:READ_FIFO_DEPTH {512} parameters:WRITE_BURST_TARGET {64} parameters:WRITE_FIFO_DEPTH {512}  } -vlnv altera.com:ip:intel_vvp_vfb:24.8.0 vvp_pipe_vvp_vfb_ch2
vds::connect_interface_net -dest cpu|dm_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.dm_agent 0x00110000
vds::connect_interface_net -dest cpu|dm_agent -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/cpu.dm_agent 0x00110000
vds::lock_base_address -interface cpu|dm_agent
vds::connect_interface_net -dest vvp_pipe_crs_sdi_tx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_crs_sdi_tx.av_mm_control_agent 0x00022600
vds::connect_interface_net -dest vvp_pipe_csc_dp_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_csc_dp_rx.av_mm_control_agent 0x00022400
vds::connect_interface_net -dest vvp_pipe_csc_hdmi_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_csc_hdmi_rx.av_mm_control_agent 0x00022200
vds::connect_interface_net -dest vvp_pipe_csc_sdi_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_csc_sdi_rx.av_mm_control_agent 0x00022000
vds::connect_interface_net -dest vvp_pipe_csc_sdi_tx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_csc_sdi_tx.av_mm_control_agent 0x00021e00
vds::connect_interface_net -dest vvp_pipe_dil_dp_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_dil_dp_rx.av_mm_control_agent 0x00021c00
vds::connect_interface_net -dest vvp_pipe_dil_hdmi_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_dil_hdmi_rx.av_mm_control_agent 0x00021a00
vds::connect_interface_net -dest vvp_pipe_dil_sdi_rx|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_dil_sdi_rx.av_mm_control_agent 0x00021800
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_down.av_mm_control_agent 0x00020800
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down_ch2|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_down_ch2.av_mm_control_agent 0x00020400
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down_ch3|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_down_ch3.av_mm_control_agent 0x00020000
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_up.av_mm_control_agent 0x00021600
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up_ch2|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_up_ch2.av_mm_control_agent 0x00021400
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up_ch3|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_scaler_up_ch3.av_mm_control_agent 0x00021200
vds::connect_interface_net -dest vvp_pipe_vvp_switch_input|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_switch_input.av_mm_control_agent 0x00021000
vds::connect_interface_net -dest vvp_pipe_vvp_switch_output|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_switch_output.av_mm_control_agent 0x00020e00
vds::connect_interface_net -dest vvp_pipe_vvp_tpg|av_mm_control_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_vvp_tpg.av_mm_control_agent 0x00020c00
vds::connect_interface_net -dest jtag_uart|avalon_jtag_slave -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/jtag_uart.avalon_jtag_slave 0x00022950
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest sys_clock_timer|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sys_clock_timer.s1 0x00022800
vds::connect_interface_net -dest vvp_clk_dbg_00|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_00.s1 0x000228b0
vds::connect_interface_net -dest vvp_clk_dbg_01|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_01.s1 0x000228a0
vds::connect_interface_net -dest vvp_clk_dbg_02|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_02.s1 0x00022890
vds::connect_interface_net -dest vvp_clk_dbg_03|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_03.s1 0x00022880
vds::connect_interface_net -dest vvp_clk_dbg_04|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_04.s1 0x00022870
vds::connect_interface_net -dest vvp_clk_dbg_05|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_05.s1 0x00022860
vds::connect_interface_net -dest vvp_clk_dbg_06|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_06.s1 0x00022850
vds::connect_interface_net -dest vvp_clk_dbg_07|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_07.s1 0x00022840
vds::connect_interface_net -dest vvp_clk_dbg_08|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_08.s1 0x00022830
vds::connect_interface_net -dest vvp_clk_dbg_09|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_clk_dbg_09.s1 0x00022820
vds::connect_interface_net -dest vvp_pipe_actv_dim_dptx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_actv_dim_dptx.s1 0x00022940
vds::connect_interface_net -dest vvp_pipe_actv_dim_hdmitx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_actv_dim_hdmitx.s1 0x00022930
vds::connect_interface_net -dest vvp_pipe_actv_dim_sditx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_actv_dim_sditx.s1 0x00022920
vds::connect_interface_net -dest vvp_pipe_new_actv_dim_dptx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_actv_dim_dptx.s1 0x00022910
vds::connect_interface_net -dest vvp_pipe_new_actv_dim_hdmitx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_actv_dim_hdmitx.s1 0x00022900
vds::connect_interface_net -dest vvp_pipe_new_actv_dim_sditx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_actv_dim_sditx.s1 0x000228f0
vds::connect_interface_net -dest vvp_pipe_new_fps_dpitx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_fps_dpitx.s1 0x000228e0
vds::connect_interface_net -dest vvp_pipe_new_fps_hdmitx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_fps_hdmitx.s1 0x000228d0
vds::connect_interface_net -dest vvp_pipe_new_fps_sditx|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/vvp_pipe_new_fps_sditx.s1 0x000228c0
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest jtag_uart|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/jtag_uart.irq 0
vds::connect_interface_net -dest sys_clock_timer|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/sys_clock_timer.irq 1
vds::lock_base_address -interface onchip_mem|s1
vds::connect_interface_net -dest vvp_pipe_csc_dp_rx|axi4s_vid_in -src vvp_pipe_crs_dp_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_crs_dp_rx|axi4s_vid_in -src vvp_pipe_dil_dp_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_csc_hdmi_rx|axi4s_vid_in -src vvp_pipe_crs_hdmi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_crs_hdmi_rx|axi4s_vid_in -src vvp_pipe_dil_hdmi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_csc_sdi_rx|axi4s_vid_in -src vvp_pipe_crs_sdi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_crs_sdi_rx|axi4s_vid_in -src vvp_pipe_dil_sdi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_guard_bands_sdi_tx|axi4s_vid_in -src vvp_pipe_crs_sdi_tx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_crs_sdi_tx|axi4s_vid_in -src vvp_pipe_csc_sdi_tx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_input|axi4s_vid_in_2 -src vvp_pipe_csc_dp_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_input|axi4s_vid_in_1 -src vvp_pipe_csc_hdmi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_input|axi4s_vid_in_3 -src vvp_pipe_csc_sdi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_csc_sdi_tx|axi4s_vid_in -src vvp_pipe_vvp_switch_output|axi4s_vid_out_2
vds::connect_interface_net -dest vvp_pipe_dil_dp_rx|axi4s_vid_in -src vvp_pipe_intel_fifo_dp_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_dil_hdmi_rx|axi4s_vid_in -src vvp_pipe_intel_fifo_hdmi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_dil_sdi_rx|axi4s_vid_in -src vvp_pipe_intel_fifo_sdi_rx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_emif_ddr4|s0_axi4 -src vvp_pipe_emif_addr|expanded_master
vds::assign_base_address -connection vvp_pipe_emif_addr.expanded_master/vvp_pipe_emif_ddr4.s0_axi4 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr|windowed_slave -src vvp_pipe_vvp_vfb|av_mm_mem_read_host
vds::assign_base_address -connection vvp_pipe_vvp_vfb.av_mm_mem_read_host/vvp_pipe_emif_addr.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr|windowed_slave -src vvp_pipe_vvp_vfb|av_mm_mem_write_host
vds::assign_base_address -connection vvp_pipe_vvp_vfb.av_mm_mem_write_host/vvp_pipe_emif_addr.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_ddr4_ch2|s0_axi4 -src vvp_pipe_emif_addr_ch2|expanded_master
vds::assign_base_address -connection vvp_pipe_emif_addr_ch2.expanded_master/vvp_pipe_emif_ddr4_ch2.s0_axi4 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr_ch2|windowed_slave -src vvp_pipe_vvp_vfb_ch2|av_mm_mem_read_host
vds::assign_base_address -connection vvp_pipe_vvp_vfb_ch2.av_mm_mem_read_host/vvp_pipe_emif_addr_ch2.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr_ch2|windowed_slave -src vvp_pipe_vvp_vfb_ch2|av_mm_mem_write_host
vds::assign_base_address -connection vvp_pipe_vvp_vfb_ch2.av_mm_mem_write_host/vvp_pipe_emif_addr_ch2.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_ddr4_ch3|s0_axi4 -src vvp_pipe_emif_addr_ch3|expanded_master
vds::assign_base_address -connection vvp_pipe_emif_addr_ch3.expanded_master/vvp_pipe_emif_ddr4_ch3.s0_axi4 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr_ch3|windowed_slave -src vvp_pipe_vfb_ch3|av_mm_mem_read_host
vds::assign_base_address -connection vvp_pipe_vfb_ch3.av_mm_mem_read_host/vvp_pipe_emif_addr_ch3.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_addr_ch3|windowed_slave -src vvp_pipe_vfb_ch3|av_mm_mem_write_host
vds::assign_base_address -connection vvp_pipe_vfb_ch3.av_mm_mem_write_host/vvp_pipe_emif_addr_ch3.windowed_slave 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_ddr4|s0_axi4lite -src vvp_pipe_emif_ddr4_cal|axil_driver_axi4_lite
vds::assign_base_address -connection vvp_pipe_emif_ddr4_cal.axil_driver_axi4_lite/vvp_pipe_emif_ddr4.s0_axi4lite 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_ddr4_ch2|s0_axi4lite -src vvp_pipe_emif_ddr4_cal_ch2|axil_driver_axi4_lite
vds::assign_base_address -connection vvp_pipe_emif_ddr4_cal_ch2.axil_driver_axi4_lite/vvp_pipe_emif_ddr4_ch2.s0_axi4lite 0x0000
vds::connect_interface_net -dest vvp_pipe_emif_ddr4_ch3|s0_axi4lite -src vvp_pipe_emif_ddr4_cal_ch3|axil_driver_axi4_lite
vds::assign_base_address -connection vvp_pipe_emif_ddr4_cal_ch3.axil_driver_axi4_lite/vvp_pipe_emif_ddr4_ch3.s0_axi4lite 0x0000
vds::connect_interface_net -dest vvp_pipe_intel_fifo_sdi_tx|axi4s_vid_in -src vvp_pipe_guard_bands_sdi_tx|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_intel_fifo_dp_tx|axi4s_vid_in -src vvp_pipe_vvp_switch_output|axi4s_vid_out_1
vds::connect_interface_net -dest vvp_pipe_intel_fifo_hdmi_tx|axi4s_vid_in -src vvp_pipe_vvp_switch_output|axi4s_vid_out_0
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up_ch3|axi4s_vid_in -src vvp_pipe_vfb_ch3|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vfb_ch3|axi4s_vid_in -src vvp_pipe_vvp_scaler_down_ch3|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_vfb|axi4s_vid_in -src vvp_pipe_vvp_scaler_down|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down|axi4s_vid_in -src vvp_pipe_vvp_switch_input|axi4s_vid_out_0
vds::connect_interface_net -dest vvp_pipe_vvp_vfb_ch2|axi4s_vid_in -src vvp_pipe_vvp_scaler_down_ch2|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down_ch2|axi4s_vid_in -src vvp_pipe_vvp_switch_input|axi4s_vid_out_1
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_down_ch3|axi4s_vid_in -src vvp_pipe_vvp_switch_input|axi4s_vid_out_2
vds::connect_interface_net -dest vvp_pipe_vvp_switch_output|axi4s_vid_in_0 -src vvp_pipe_vvp_scaler_up|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up|axi4s_vid_in -src vvp_pipe_vvp_vfb|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_output|axi4s_vid_in_1 -src vvp_pipe_vvp_scaler_up_ch2|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_scaler_up_ch2|axi4s_vid_in -src vvp_pipe_vvp_vfb_ch2|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_output|axi4s_vid_in_2 -src vvp_pipe_vvp_scaler_up_ch3|axi4s_vid_out
vds::connect_interface_net -dest vvp_pipe_vvp_switch_input|axi4s_vid_in_0 -src vvp_pipe_vvp_tpg|axi4s_vid_out
vds::export_interface_pin vvp_pipe_intel_fifo_sdi_tx|axi4s_vid_out vvp_pipe_intel_fifo_sdi_tx_axi4s_vid_out
vds::export_interface_pin vvp_pipe_intel_fifo_sdi_rx|axi4s_vid_in vvp_pipe_intel_fifo_sdi_rx_axi4s_vid_in
vds::export_interface_pin vvp_pipe_intel_fifo_hdmi_tx|axi4s_vid_out vvp_pipe_intel_fifo_hdmi_tx_axi4s_vid_out
vds::export_interface_pin vvp_pipe_intel_fifo_hdmi_rx|axi4s_vid_in vvp_pipe_intel_fifo_hdmi_rx_axi4s_vid_in
vds::export_interface_pin vvp_pipe_intel_fifo_dp_tx|axi4s_vid_out vvp_pipe_intel_fifo_dp_tx_axi4s_vid_out
vds::export_interface_pin vvp_pipe_intel_fifo_dp_rx|axi4s_vid_in vvp_pipe_intel_fifo_dp_rx_axi4s_vid_in
vds::export_interface_pin vvp_pipe_emif_ddr4_ch3|mem_ck_0 vvp_pipe_emif_ddr4_ch3_mem_ck_0
vds::export_interface_pin vvp_pipe_emif_ddr4_ch3|mem_0 vvp_pipe_emif_ddr4_ch3_mem_0
vds::export_interface_pin vvp_pipe_emif_ddr4_ch2|mem_ck_0 vvp_pipe_emif_ddr4_ch2_mem_ck_0
vds::export_interface_pin vvp_pipe_emif_ddr4_ch2|mem_0 vvp_pipe_emif_ddr4_ch2_mem_0
vds::export_interface_pin vvp_pipe_emif_ddr4|mem_ck_0 vvp_pipe_emif_ddr4_mem_ck_0
vds::export_interface_pin vvp_pipe_emif_ddr4|mem_0 vvp_pipe_emif_ddr4_mem_0
vds::export_interface_pin cpu|timer_sw_agent cpu_timer_sw_agent
vds::connect_net -src cpu|dbg_reset_out -dest cpu|ndm_reset_in
vds::connect_net -src cpu|dbg_reset_out -dest rst_controller|reset_in0
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_crs_sdi_tx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_csc_dp_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_csc_hdmi_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_csc_sdi_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_csc_sdi_tx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_dil_dp_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_dil_hdmi_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_dil_sdi_rx|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down_ch2|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down_ch3|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up_ch2|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up_ch3|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_switch_input|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_switch_output|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_vvp_tpg|agent_reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest cpu|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest sys_clock_timer|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_00|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_01|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_02|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_03|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_04|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_05|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_06|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_07|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_08|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_clk_dbg_09|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_actv_dim_dptx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_actv_dim_hdmitx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_actv_dim_sditx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_actv_dim_dptx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_actv_dim_hdmitx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_actv_dim_sditx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_fps_dpitx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_fps_hdmitx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest vvp_pipe_new_fps_sditx|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest rst_controller|reset_in1
vds::export_pin cpu_reset_bridge|in_reset cpu_reset_bridge_in_reset
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_crs_sdi_tx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_csc_dp_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_csc_hdmi_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_csc_sdi_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_csc_sdi_tx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_dil_dp_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_dil_hdmi_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_dil_sdi_rx|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_down|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_down_ch2|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_down_ch3|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_up|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_up_ch2|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_scaler_up_ch3|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_switch_input|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_switch_output|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_vvp_tpg|agent_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4_cal|axil_driver_clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4_cal_ch2|axil_driver_clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4_cal_ch3|axil_driver_clk
vds::connect_net -src mgmt_clk|out_clk -dest cpu|clk
vds::connect_net -src mgmt_clk|out_clk -dest cpu_reset_bridge|clk
vds::connect_net -src mgmt_clk|out_clk -dest jtag_uart|clk
vds::connect_net -src mgmt_clk|out_clk -dest sys_clock_timer|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_00|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_01|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_02|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_03|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_04|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_05|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_06|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_07|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_08|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_clk_dbg_09|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_actv_dim_dptx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_actv_dim_hdmitx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_actv_dim_sditx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_actv_dim_dptx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_actv_dim_hdmitx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_actv_dim_sditx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_fps_dpitx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_fps_hdmitx|clk
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_new_fps_sditx|clk
vds::connect_net -src mgmt_clk|out_clk -dest rst_controller|clk
vds::connect_net -src mgmt_clk|out_clk -dest rst_translator|clk
vds::connect_net -src mgmt_clk|out_clk -dest onchip_mem|clk1
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4|s0_axi4lite_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4_ch2|s0_axi4lite_clock
vds::connect_net -src mgmt_clk|out_clk -dest vvp_pipe_emif_ddr4_ch3|s0_axi4lite_clock
vds::export_pin mgmt_clk|in_clk mgmt_clk_in_clk
vds::connect_net -src rst_controller|reset_out -dest rst_translator|in_reset
vds::connect_net -src rst_controller|reset_out -dest onchip_mem|reset1
vds::connect_net -src rst_translator|out_reset -dest jtag_uart|reset
vds::export_pin vvp_clk_dbg_00|external_connection vvp_clk_dbg_00_external_connection
vds::export_pin vvp_clk_dbg_01|external_connection vvp_clk_dbg_01_external_connection
vds::export_pin vvp_clk_dbg_02|external_connection vvp_clk_dbg_02_external_connection
vds::export_pin vvp_clk_dbg_03|external_connection vvp_clk_dbg_03_external_connection
vds::export_pin vvp_clk_dbg_04|external_connection vvp_clk_dbg_04_external_connection
vds::export_pin vvp_clk_dbg_05|external_connection vvp_clk_dbg_05_external_connection
vds::export_pin vvp_clk_dbg_06|external_connection vvp_clk_dbg_06_external_connection
vds::export_pin vvp_clk_dbg_07|external_connection vvp_clk_dbg_07_external_connection
vds::export_pin vvp_clk_dbg_08|external_connection vvp_clk_dbg_08_external_connection
vds::export_pin vvp_clk_dbg_09|external_connection vvp_clk_dbg_09_external_connection
vds::export_pin vvp_pipe_actv_dim_dptx|external_connection vvp_pipe_actv_dim_dptx_external_connection
vds::export_pin vvp_pipe_actv_dim_hdmitx|external_connection vvp_pipe_actv_dim_hdmitx_external_connection
vds::export_pin vvp_pipe_actv_dim_sditx|external_connection vvp_pipe_actv_dim_sditx_external_connection
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_crs_dp_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_crs_hdmi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_crs_sdi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_crs_sdi_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_csc_dp_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_csc_hdmi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_csc_sdi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_csc_sdi_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_dil_dp_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_dil_hdmi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_dil_sdi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_guard_bands_sdi_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_dp_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_dp_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_hdmi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_hdmi_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_sdi_rx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_intel_fifo_sdi_tx|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vfb_ch3|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_down|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_down_ch2|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_down_ch3|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_up|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_up_ch2|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_scaler_up_ch3|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_switch_input|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_switch_output|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_tpg|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_vfb|main_clock
vds::connect_net -src vvp_pipe_axis_clk|out_clk -dest vvp_pipe_vvp_vfb_ch2|main_clock
vds::export_pin vvp_pipe_axis_clk|in_clk vvp_pipe_axis_clk_in_clk
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_crs_dp_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_crs_hdmi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_crs_sdi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_crs_sdi_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_csc_dp_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_csc_hdmi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_csc_sdi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_csc_sdi_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_dil_dp_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_dil_hdmi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_dil_sdi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_guard_bands_sdi_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_dp_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_dp_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_hdmi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_hdmi_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_sdi_rx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_intel_fifo_sdi_tx|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vfb_ch3|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down_ch2|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_down_ch3|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up_ch2|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_scaler_up_ch3|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_switch_input|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_switch_output|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_tpg|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_vfb|main_reset
vds::connect_net -src vvp_pipe_axis_reset_bridge|out_reset -dest vvp_pipe_vvp_vfb_ch2|main_reset
vds::export_pin vvp_pipe_axis_reset_bridge|in_reset vvp_pipe_axis_reset_bridge_in_reset
vds::export_pin vvp_pipe_emif_ddr4|mem_reset_n vvp_pipe_emif_ddr4_mem_reset_n
vds::connect_net -src vvp_pipe_emif_ddr4|s0_axi4_clock_out -dest vvp_pipe_emif_addr|clock
vds::connect_net -src vvp_pipe_emif_ddr4|s0_axi4_clock_out -dest vvp_pipe_vvp_vfb|mem_clock
vds::connect_net -src vvp_pipe_emif_ddr4|s0_axi4_ctrl_ready -dest vvp_pipe_vvp_vfb|mem_reset
vds::connect_net -src vvp_pipe_emif_ddr4|s0_axi4_ctrl_ready -dest vvp_pipe_emif_addr|reset
vds::export_pin vvp_pipe_emif_ddr4_cal|cal_done_rst_n vvp_pipe_emif_ddr4_cal_cal_done_rst_n
vds::export_pin vvp_pipe_emif_ddr4_cal_ch2|cal_done_rst_n vvp_pipe_emif_ddr4_cal_ch2_cal_done_rst_n
vds::export_pin vvp_pipe_emif_ddr4_cal_ch3|cal_done_rst_n vvp_pipe_emif_ddr4_cal_ch3_cal_done_rst_n
vds::export_pin vvp_pipe_emif_ddr4_ch2|mem_reset_n vvp_pipe_emif_ddr4_ch2_mem_reset_n
vds::connect_net -src vvp_pipe_emif_ddr4_ch2|s0_axi4_clock_out -dest vvp_pipe_emif_addr_ch2|clock
vds::connect_net -src vvp_pipe_emif_ddr4_ch2|s0_axi4_clock_out -dest vvp_pipe_vvp_vfb_ch2|mem_clock
vds::connect_net -src vvp_pipe_emif_ddr4_ch2|s0_axi4_ctrl_ready -dest vvp_pipe_vvp_vfb_ch2|mem_reset
vds::connect_net -src vvp_pipe_emif_ddr4_ch2|s0_axi4_ctrl_ready -dest vvp_pipe_emif_addr_ch2|reset
vds::export_pin vvp_pipe_emif_ddr4_ch2|core_init_n vvp_pipe_emif_ddr4_ch2_core_init_n
vds::export_pin vvp_pipe_emif_ddr4_ch2|oct_0 vvp_pipe_emif_ddr4_ch2_oct_0
vds::export_pin vvp_pipe_emif_ddr4_ch2|ref_clk vvp_pipe_emif_ddr4_ch2_ref_clk
vds::export_pin vvp_pipe_emif_ddr4_ch3|mem_reset_n vvp_pipe_emif_ddr4_ch3_mem_reset_n
vds::connect_net -src vvp_pipe_emif_ddr4_ch3|s0_axi4_clock_out -dest vvp_pipe_emif_addr_ch3|clock
vds::connect_net -src vvp_pipe_emif_ddr4_ch3|s0_axi4_clock_out -dest vvp_pipe_vfb_ch3|mem_clock
vds::connect_net -src vvp_pipe_emif_ddr4_ch3|s0_axi4_ctrl_ready -dest vvp_pipe_vfb_ch3|mem_reset
vds::connect_net -src vvp_pipe_emif_ddr4_ch3|s0_axi4_ctrl_ready -dest vvp_pipe_emif_addr_ch3|reset
vds::export_pin vvp_pipe_emif_ddr4_ch3|core_init_n vvp_pipe_emif_ddr4_ch3_core_init_n
vds::export_pin vvp_pipe_emif_ddr4_ch3|oct_0 vvp_pipe_emif_ddr4_ch3_oct_0
vds::export_pin vvp_pipe_emif_ddr4_ch3|ref_clk vvp_pipe_emif_ddr4_ch3_ref_clk
vds::export_pin vvp_pipe_emif_ddr4|core_init_n vvp_pipe_emif_ddr4_core_init_n
vds::export_pin vvp_pipe_emif_ddr4|oct_0 vvp_pipe_emif_ddr4_oct_0
vds::export_pin vvp_pipe_emif_ddr4|ref_clk vvp_pipe_emif_ddr4_ref_clk
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4_cal|axil_driver_rst_n
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4_cal_ch2|axil_driver_rst_n
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4_cal_ch3|axil_driver_rst_n
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4|s0_axi4lite_reset_n
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4_ch2|s0_axi4lite_reset_n
vds::connect_net -src vvp_pipe_emif_reset_bridge|out_reset -dest vvp_pipe_emif_ddr4_ch3|s0_axi4lite_reset_n
vds::export_pin vvp_pipe_emif_reset_bridge|in_reset vvp_pipe_emif_reset_bridge_in_reset
vds::export_pin vvp_pipe_new_actv_dim_dptx|external_connection vvp_pipe_new_actv_dim_dptx_external_connection
vds::export_pin vvp_pipe_new_actv_dim_hdmitx|external_connection vvp_pipe_new_actv_dim_hdmitx_external_connection
vds::export_pin vvp_pipe_new_actv_dim_sditx|external_connection vvp_pipe_new_actv_dim_sditx_external_connection
vds::export_pin vvp_pipe_new_fps_dpitx|external_connection vvp_pipe_new_fps_dpitx_external_connection
vds::export_pin vvp_pipe_new_fps_hdmitx|external_connection vvp_pipe_new_fps_hdmitx_external_connection
vds::export_pin vvp_pipe_new_fps_sditx|external_connection vvp_pipe_new_fps_sditx_external_connection
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } cpu.data_manager
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType EMBEDDED_MEMORY_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_addr.expanded_master
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType EMBEDDED_MEMORY_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_addr_ch2.expanded_master
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE      qsys_mm.responseFifoType EMBEDDED_MEMORY_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_addr_ch3.expanded_master
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_ddr4_cal.axil_driver_axi4_lite
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_ddr4_cal_ch2.axil_driver_axi4_lite
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE      qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_emif_ddr4_cal_ch3.axil_driver_axi4_lite
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE      qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_vfb_ch3.av_mm_mem_read_host
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_vvp_vfb.av_mm_mem_read_host
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter HANDSHAKE qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } vvp_pipe_vvp_vfb_ch2.av_mm_mem_read_host
vds::set_properties {parameters:MEM_CL_CYC 12.0} [vds::get_cells vvp_pipe_emif_ddr4]
vds::set_properties {parameters:MEM_TRFC_NS 350.0} [vds::get_cells vvp_pipe_emif_ddr4]
vds::set_properties {parameters:MEM_CL_CYC 12.0} [vds::get_cells vvp_pipe_emif_ddr4_ch2]
vds::set_properties {parameters:MEM_TRFC_NS 350.0} [vds::get_cells vvp_pipe_emif_ddr4_ch2]
vds::set_properties {parameters:MEM_CL_CYC 12.0} [vds::get_cells vvp_pipe_emif_ddr4_ch3]
vds::set_properties {parameters:MEM_TRFC_NS 350.0} [vds::get_cells vvp_pipe_emif_ddr4_ch3]
vds::sync_system_info
vds::validate_system
vds::save_system vvp_pipe
post_message [pwd]
puts [pwd]
vds::generate_system -design_file $prj_vds -search_path $ -synthesis Verilog
vds::close_system vvp_pipe

post_message "Script completed successfully."



