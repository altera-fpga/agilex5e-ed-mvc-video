# Project setup with top.qsf 
set prj_top [file join .. .. .. top.qsf]
project_open $prj_top

post_message [pwd]
puts [pwd]

# Project setup with qsys system
set prj_vds [file join src vds nios_sdi_ss nios_sdi_ss.vds]

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

vds::create_system nios_sdi_ss
vds::create_cell -vlnv altera.com:ip:altera_clock_bridge:19.2.0 axi4s_clock_bridge
vds::create_cell -vlnv altera.com:ip:altera_reset_bridge:19.2.0 axi4s_reset_bridge
vds::create_cell -properties { parameters:enableDebugReset {true} parameters:resetSlave {onchip_mem.s1}  } -vlnv altera.com:ip:intel_niosv_m:26.0.0 cpu
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 cpu_clock_bridge
vds::create_cell -properties { parameters:readBufferDepth {1024} parameters:readIRQThreshold {1} parameters:writeBufferDepth {1024} parameters:writeIRQThreshold {1}  } -vlnv altera.com:ip:altera_avalon_jtag_uart:19.3.2 cpu_jtag_debug
vds::create_cell -vlnv altera.com:ip:altera_reset_bridge:19.2.0 cpu_reset_bridge
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 i2c_0
vds::create_cell -vlnv altera.com:ip:altera_avalon_timer:19.3.5 nios_sdi_ss_timer
vds::create_cell -properties { parameters:memorySize {131072}  } -vlnv altera.com:ip:intel_onchip_memory:2.0.0 onchip_mem
vds::create_cell -properties { parameters:AXIS_VIDEOIF_EN {1} parameters:BPS {10} parameters:DIRECTION {rx} parameters:ED_TXPLL_TYPE {fPLL} parameters:RX_EN_VPID_EXTRACT {1} parameters:RX_CORECLK_FREQ {100.0} parameters:TEST_RECONFIG_SEQ {half} parameters:TEST_SYNC_OUTPUT {0} parameters:VIDEO_STANDARD {mr} parameters:WRAPPER_OPT {0}  } -vlnv altera.com:ip:altera_sdi_ii_gts:2.5.0 sdi_rx_mr
vds::create_cell -properties { parameters:AXIS_VIDEOIF_EN {1} parameters:BPS {10} parameters:ED_TXPLL_TYPE {fPLL} parameters:TEST_RECONFIG_SEQ {half} parameters:TEST_SYNC_OUTPUT {0} parameters:TX_EN_VPID_INSERT {1} parameters:VIDEO_STANDARD {mr} parameters:WRAPPER_OPT {0}  } -vlnv altera.com:ip:altera_sdi_ii_gts:2.5.0 sdi_tx_mr
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 sditx_pio_actv_dim
vds::create_cell -properties { parameters:direction {Input}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 sditx_pio_fps
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 sditx_pio_new_actv_dim
vds::connect_interface_net -dest cpu|dm_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.dm_agent 0x00020000
vds::connect_interface_net -dest cpu|dm_agent -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/cpu.dm_agent 0x00020000
vds::connect_interface_net -dest cpu|timer_sw_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.timer_sw_agent 0x00031000
vds::connect_interface_net -dest cpu_jtag_debug|avalon_jtag_slave -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu_jtag_debug.avalon_jtag_slave 0x000310d0
vds::connect_interface_net -dest i2c_0|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/i2c_0.csr 0x00031040
vds::connect_interface_net -dest sdi_rx_mr|rx_av_mm_control -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sdi_rx_mr.rx_av_mm_control 0x00030800
vds::connect_interface_net -dest nios_sdi_ss_timer|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/nios_sdi_ss_timer.s1 0x00031080
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest sditx_pio_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sditx_pio_actv_dim.s1 0x000310c0
vds::connect_interface_net -dest sditx_pio_fps|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sditx_pio_fps.s1 0x000310a0
vds::connect_interface_net -dest sditx_pio_new_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sditx_pio_new_actv_dim.s1 0x000310b0
vds::connect_interface_net -dest sdi_tx_mr|tx_av_mm_control -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sdi_tx_mr.tx_av_mm_control 0x00030000
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest i2c_0|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/i2c_0.interrupt_sender 0
vds::connect_interface_net -dest cpu_jtag_debug|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/cpu_jtag_debug.irq 1
vds::connect_interface_net -dest nios_sdi_ss_timer|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/nios_sdi_ss_timer.irq 2
vds::export_interface_pin sdi_tx_mr|tx_axi4s_vid_in sdi_tx_mr_tx_axi4s_vid_in
vds::export_interface_pin sdi_rx_mr|rx_axi4s_vid_out sdi_rx_mr_rx_axi4s_vid_out
vds::export_interface_pin i2c_0|i2c_serial i2c_0_i2c_serial
vds::connect_net -src axi4s_clock_bridge|out_clk -dest axi4s_reset_bridge|clk
vds::connect_net -src axi4s_clock_bridge|out_clk -dest sdi_rx_mr|rx_axi4s_clk
vds::connect_net -src axi4s_clock_bridge|out_clk -dest sdi_tx_mr|tx_axi4s_clk
vds::export_pin axi4s_clock_bridge|in_clk axi4s_clock_bridge_in_clk
vds::connect_net -src axi4s_reset_bridge|out_reset -dest sdi_rx_mr|rx_axi4s_reset
vds::connect_net -src axi4s_reset_bridge|out_reset -dest sdi_tx_mr|tx_axi4s_reset
vds::export_pin axi4s_reset_bridge|in_reset axi4s_reset_bridge_in_reset
vds::export_pin cpu|dbg_reset_out cpu_dbg_reset_out
vds::connect_net -src cpu_clock_bridge|out_clk -dest cpu|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest cpu_jtag_debug|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest cpu_reset_bridge|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest nios_sdi_ss_timer|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest sditx_pio_actv_dim|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest sditx_pio_fps|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest sditx_pio_new_actv_dim|clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest onchip_mem|clk1
vds::connect_net -src cpu_clock_bridge|out_clk -dest i2c_0|clock
vds::connect_net -src cpu_clock_bridge|out_clk -dest sdi_rx_mr|rx_mgmt_clk
vds::connect_net -src cpu_clock_bridge|out_clk -dest sdi_tx_mr|tx_mgmt_clk
vds::export_pin cpu_clock_bridge|in_clk cpu_clock_bridge_in_clk
vds::connect_net -src cpu_reset_bridge|out_reset -dest cpu|ndm_reset_in
vds::connect_net -src cpu_reset_bridge|out_reset -dest cpu|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest cpu_jtag_debug|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest nios_sdi_ss_timer|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest sditx_pio_actv_dim|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest sditx_pio_fps|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest sditx_pio_new_actv_dim|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest onchip_mem|reset1
vds::connect_net -src cpu_reset_bridge|out_reset -dest i2c_0|reset_sink
vds::connect_net -src cpu_reset_bridge|out_reset -dest sdi_rx_mr|rx_rst
vds::connect_net -src cpu_reset_bridge|out_reset -dest sdi_tx_mr|tx_rst
vds::export_pin cpu_reset_bridge|in_reset cpu_reset_bridge_in_reset
vds::export_pin sdi_rx_mr|gxb_ltd sdi_rx_mr_gxb_ltd
vds::export_pin sdi_rx_mr|rst_trig_rst sdi_rx_mr_rst_trig_rst
vds::export_pin sdi_rx_mr|rx_align_locked sdi_rx_mr_rx_align_locked
vds::export_pin sdi_rx_mr|rx_clkout_is_ntsc_paln sdi_rx_mr_rx_clkout_is_ntsc_paln
vds::export_pin sdi_rx_mr|rx_f sdi_rx_mr_rx_f
vds::export_pin sdi_rx_mr|rx_format sdi_rx_mr_rx_format
vds::export_pin sdi_rx_mr|rx_frame_locked sdi_rx_mr_rx_frame_locked
vds::export_pin sdi_rx_mr|rx_h sdi_rx_mr_rx_h
vds::export_pin sdi_rx_mr|rx_rst_proto_out sdi_rx_mr_rx_rst_proto_out
vds::export_pin sdi_rx_mr|rx_sdi_start_reconfig sdi_rx_mr_rx_sdi_start_reconfig
vds::export_pin sdi_rx_mr|rx_std sdi_rx_mr_rx_std
vds::export_pin sdi_rx_mr|rx_trs_locked sdi_rx_mr_rx_trs_locked
vds::export_pin sdi_rx_mr|rx_v sdi_rx_mr_rx_v
vds::export_pin sdi_rx_mr|rx_vpid_checksum_error sdi_rx_mr_rx_vpid_checksum_error
vds::export_pin sdi_rx_mr|rx_vpid_checksum_error_b sdi_rx_mr_rx_vpid_checksum_error_b
vds::export_pin sdi_rx_mr|trig_rst_ctrl sdi_rx_mr_trig_rst_ctrl
vds::export_pin sdi_rx_mr|rx_core_refclk sdi_rx_mr_rx_core_refclk
vds::export_pin sdi_rx_mr|rx_datain sdi_rx_mr_rx_datain
vds::export_pin sdi_rx_mr|rx_ready sdi_rx_mr_rx_ready
vds::export_pin sdi_rx_mr|rx_sdi_reconfig_done sdi_rx_mr_rx_sdi_reconfig_done
vds::export_pin sdi_rx_mr|rx_xcvr_reset_ack sdi_rx_mr_rx_xcvr_reset_ack
vds::export_pin sdi_rx_mr|xcvr_rxclk sdi_rx_mr_xcvr_rxclk
vds::export_pin sdi_tx_mr|tx_dataout sdi_tx_mr_tx_dataout
vds::export_pin sdi_tx_mr|tx_dataout_valid sdi_tx_mr_tx_dataout_valid
vds::export_pin sdi_tx_mr|tx_pclk sdi_tx_mr_tx_pclk
vds::export_pin sditx_pio_actv_dim|external_connection sditx_pio_actv_dim_external_connection
vds::export_pin sditx_pio_fps|external_connection sditx_pio_fps_external_connection
vds::export_pin sditx_pio_new_actv_dim|external_connection sditx_pio_new_actv_dim_external_connection
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } cpu.data_manager
vds::sync_system_info
vds::validate_system
vds::save_system nios_sdi_ss
post_message [pwd]
puts [pwd]
vds::generate_system -design_file $prj_vds -search_path $ -synthesis Verilog
vds::close_system nios_sdi_ss

post_message "Script completed successfully."
