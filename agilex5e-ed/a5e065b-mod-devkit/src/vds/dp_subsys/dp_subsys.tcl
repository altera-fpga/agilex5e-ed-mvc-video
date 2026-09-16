# Project setup with top.qsf 
set prj_top [file join .. .. .. top.qsf]
project_open $prj_top

post_message [pwd]
puts [pwd]

# Project setup with qsys system
set prj_vds [file join src vds dp_core dp_core.vds]

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

proc create_dp_rx {} {
vds::create_system dp_rx
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {16000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 clk_16
vds::create_cell -properties { parameters:USE_AUTO_ADDRESS_WIDTH {1}  } -vlnv altera.com:ip:altera_avalon_mm_bridge:20.1.0 dp_rx_mgmt_bridge
vds::create_cell -properties { parameters:RX_AXIS_VIDEOIF_EN {1} parameters:RX_PIXELS_PER_CLOCK {2} parameters:RX_VIDEO_BPS {10} parameters:TX_SUPPORT_DP {0}  } -vlnv altera.com:ip:altera_dp:20.4.0 dp_sink
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 mgmt_clk
vds::create_cell -properties { parameters:direction {Input} parameters:width {1}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 pio_0
vds::create_cell -properties { parameters:direction {Input} parameters:width {1}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 pio_1
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 reset_bridge
vds::connect_interface_net -dest dp_sink|rx_mgmt -src dp_rx_mgmt_bridge|m0
vds::assign_base_address -connection dp_rx_mgmt_bridge.m0/dp_sink.rx_mgmt 0x0000
vds::connect_interface_net -dest pio_0|s1 -src dp_rx_mgmt_bridge|m0
vds::assign_base_address -connection dp_rx_mgmt_bridge.m0/pio_0.s1 0x2000
vds::connect_interface_net -dest pio_1|s1 -src dp_rx_mgmt_bridge|m0
vds::assign_base_address -connection dp_rx_mgmt_bridge.m0/pio_1.s1 0x2010
vds::export_interface_pin dp_sink|rx_axi4s_vid_out dp_sink_rx_axi4s_vid_out
vds::export_interface_pin dp_sink|dsc_tx_tpg dp_sink_dsc_tx_tpg
vds::export_interface_pin dp_sink|dsc_axi4s dp_sink_dsc_axi4s
vds::export_interface_pin dp_sink|dsc_sync dp_sink_dsc_sync
vds::export_interface_pin dp_sink|dsc_ctrl dp_sink_dsc_ctrl
vds::export_interface_pin dp_sink|tx_im_dsc dp_sink_tx_im_dsc
vds::export_interface_pin dp_sink|dsc_passthrough dp_sink_dsc_passthrough
vds::export_interface_pin dp_sink|rx_mgmt_interrupt dp_sink_rx_mgmt_interrupt
vds::export_interface_pin dp_sink|dsc_mgmt dp_sink_dsc_mgmt
vds::export_interface_pin dp_sink|rx_analog_reconfig dp_sink_rx_analog_reconfig
vds::export_interface_pin dp_sink|rx_reconfig dp_sink_rx_reconfig
vds::export_interface_pin dp_sink|rx_stream dp_sink_rx_stream
vds::export_interface_pin dp_sink|rx_aux dp_sink_rx_aux
vds::export_interface_pin dp_sink|rx_xcvr_interface dp_sink_rx_xcvr_interface
vds::export_interface_pin dp_rx_mgmt_bridge|s0 dp_rx_mgmt_bridge_s0
vds::connect_net -src clk_16|out_clk -dest dp_sink|aux_clk
vds::export_pin clk_16|in_clk clk_16_in_clk
vds::export_pin dp_sink|rx_msa_conduit dp_sink_rx_msa_conduit
vds::export_pin dp_sink|rx_params dp_sink_rx_params
vds::export_pin dp_sink|dsc_sync_clk dp_sink_dsc_sync_clk
vds::export_pin dp_sink|rx_axi4s_clk dp_sink_rx_axi4s_clk
vds::export_pin dp_sink|rx_axi4s_reset dp_sink_rx_axi4s_reset
vds::export_pin dp_sink|rx_vid_clk dp_sink_rx_vid_clk
vds::export_pin dp_sink|tx_im_dsc_clk dp_sink_tx_im_dsc_clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_rx_mgmt_bridge|clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_sink|clk
vds::connect_net -src mgmt_clk|out_clk -dest pio_0|clk
vds::connect_net -src mgmt_clk|out_clk -dest pio_1|clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_sink|xcvr_mgmt_clk
vds::export_pin mgmt_clk|in_clk mgmt_clk_in_clk
vds::export_pin pio_0|external_connection pio_0_external_connection
vds::export_pin pio_1|external_connection pio_1_external_connection
vds::connect_net -src reset_bridge|out_reset -dest dp_sink|aux_reset
vds::connect_net -src reset_bridge|out_reset -dest dp_rx_mgmt_bridge|reset
vds::connect_net -src reset_bridge|out_reset -dest dp_sink|reset
vds::connect_net -src reset_bridge|out_reset -dest pio_0|reset
vds::connect_net -src reset_bridge|out_reset -dest pio_1|reset
vds::export_pin reset_bridge|in_reset reset_bridge_in_reset
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } dp_rx_mgmt_bridge.m0
vds::validate_system
vds::save_system dp_rx
vds::close_system dp_rx
}
proc create_dp_tx {} {
vds::create_system dp_tx
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {16000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 clk_16
vds::create_cell -properties { parameters:RX_SUPPORT_DP {0} parameters:TX_AXIS_VIDEOIF_EN {1} parameters:TX_PIXELS_PER_CLOCK {2} parameters:TX_VIDEO_BPS {10}  } -vlnv altera.com:ip:altera_dp:20.4.0 dp_source
vds::create_cell -properties { parameters:USE_AUTO_ADDRESS_WIDTH {1}  } -vlnv altera.com:ip:altera_avalon_mm_bridge:20.1.0 dp_tx_mgmt_bridge
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 mgmt_clk
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 reset_bridge
vds::connect_interface_net -dest dp_source|tx_mgmt -src dp_tx_mgmt_bridge|m0
vds::assign_base_address -connection dp_tx_mgmt_bridge.m0/dp_source.tx_mgmt 0x0000
vds::export_interface_pin dp_tx_mgmt_bridge|s0 dp_tx_mgmt_bridge_s0
vds::export_interface_pin dp_source|tx_axi4s_vid_in dp_source_tx_axi4s_vid_in
vds::export_interface_pin dp_source|dsc_tx_tpg dp_source_dsc_tx_tpg
vds::export_interface_pin dp_source|dsc_axi4s dp_source_dsc_axi4s
vds::export_interface_pin dp_source|dsc_sync dp_source_dsc_sync
vds::export_interface_pin dp_source|dsc_ctrl dp_source_dsc_ctrl
vds::export_interface_pin dp_source|tx_im_dsc dp_source_tx_im_dsc
vds::export_interface_pin dp_source|dsc_passthrough dp_source_dsc_passthrough
vds::export_interface_pin dp_source|dsc_mgmt dp_source_dsc_mgmt
vds::export_interface_pin dp_source|tx_mgmt_interrupt dp_source_tx_mgmt_interrupt
vds::export_interface_pin dp_source|tx_reconfig dp_source_tx_reconfig
vds::export_interface_pin dp_source|tx_analog_reconfig dp_source_tx_analog_reconfig
vds::export_interface_pin dp_source|tx_aux dp_source_tx_aux
vds::export_interface_pin dp_source|tx_xcvr_interface dp_source_tx_xcvr_interface
vds::connect_net -src clk_16|out_clk -dest dp_source|aux_clk
vds::export_pin clk_16|in_clk clk_16_in_clk
vds::export_pin dp_source|dsc_sync_clk dp_source_dsc_sync_clk
vds::export_pin dp_source|tx_axi4s_clk dp_source_tx_axi4s_clk
vds::export_pin dp_source|tx_axi4s_reset dp_source_tx_axi4s_reset
vds::export_pin dp_source|tx_im_dsc_clk dp_source_tx_im_dsc_clk
vds::export_pin dp_source|tx_vid_clk dp_source_tx_vid_clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_source|clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_tx_mgmt_bridge|clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_source|xcvr_mgmt_clk
vds::export_pin mgmt_clk|in_clk mgmt_clk_in_clk
vds::connect_net -src reset_bridge|out_reset -dest dp_source|aux_reset
vds::connect_net -src reset_bridge|out_reset -dest dp_source|reset
vds::connect_net -src reset_bridge|out_reset -dest dp_tx_mgmt_bridge|reset
vds::export_pin reset_bridge|in_reset reset_bridge_in_reset
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } dp_tx_mgmt_bridge.m0
vds::validate_system
vds::save_system dp_tx
vds::close_system dp_tx
}
create_dp_tx
create_dp_rx
vds::create_system dp_core
vds::create_cell -properties { parameters:enableDebugReset {true} parameters:resetSlave {onchip_mem.s1}  } -vlnv altera.com:ip:intel_niosv_m:26.0.0 cpu
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 cpu_reset_bridge
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {300000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 dp_core_axis_clk
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 dp_core_axis_reset_bridge
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 dp_core_i2c_board
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 dp_core_pio_board
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {16000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 dp_rx_clk_16
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 dp_rx_reset_bridge
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {16000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 dp_tx_clk_16
vds::create_cell -properties { parameters:ACTIVE_LOW_RESET {1} parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 dp_tx_reset_bridge
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 dptx_pio_actv_dim
vds::create_cell -properties { parameters:direction {Input}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 dptx_pio_fps
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 dptx_pio_new_actv_dim
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 i2c_master
vds::create_cell -properties { parameters:readBufferDepth {1024} parameters:readIRQThreshold {1} parameters:writeBufferDepth {1024} parameters:writeIRQThreshold {1}  } -vlnv altera.com:ip:altera_avalon_jtag_uart:19.3.2 jtag_uart
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 mgmt_clk
vds::create_cell -properties { parameters:initializationFileName {} parameters:memorySize {262144}  } -vlnv altera.com:ip:intel_onchip_memory:2.0.0 onchip_mem
vds::create_cell -properties { parameters:NUM_RESET_INPUTS {2} parameters:RESET_REQUEST_PRESENT {1}  } -vlnv altera.com:ip:altera_reset_controller:19.2.4 rst_controller
vds::create_cell -vlnv altera.com:ip:altera_reset_translator:19.2.1 rst_translator
vds::create_cell -properties { parameters:periodUnits {USEC}  } -vlnv altera.com:ip:altera_avalon_timer:19.3.5 sys_clock_timer
vds::create_cell dp_rx -type system -ref dp_rx
vds::create_cell dp_tx -type system -ref dp_tx
vds::connect_interface_net -dest cpu|dm_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.dm_agent 0x00040000
vds::connect_interface_net -dest cpu|dm_agent -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/cpu.dm_agent 0x00040000
vds::connect_interface_net -dest jtag_uart|avalon_jtag_slave -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/jtag_uart.avalon_jtag_slave 0x000560e0
vds::connect_interface_net -dest dp_core_i2c_board|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dp_core_i2c_board.csr 0x00056040
vds::connect_interface_net -dest i2c_master|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/i2c_master.csr 0x00056000
vds::connect_interface_net -dest dp_rx|dp_rx_mgmt_bridge_s0 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dp_rx.dp_rx_mgmt_bridge_s0 0x00050000
vds::connect_interface_net -dest dp_tx|dp_tx_mgmt_bridge_s0 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dp_tx.dp_tx_mgmt_bridge_s0 0x00054000
vds::connect_interface_net -dest dp_core_pio_board|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dp_core_pio_board.s1 0x000560d0
vds::connect_interface_net -dest dptx_pio_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dptx_pio_actv_dim.s1 0x000560c0
vds::connect_interface_net -dest dptx_pio_fps|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dptx_pio_fps.s1 0x000560a0
vds::connect_interface_net -dest dptx_pio_new_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/dptx_pio_new_actv_dim.s1 0x000560b0
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest sys_clock_timer|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/sys_clock_timer.s1 0x00056080
vds::connect_interface_net -dest onchip_mem|s1 -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/onchip_mem.s1 0x0000
vds::connect_interface_net -dest dp_rx|dp_sink_rx_mgmt_interrupt -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/dp_rx.dp_sink_rx_mgmt_interrupt 4
vds::connect_interface_net -dest dp_tx|dp_source_tx_mgmt_interrupt -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/dp_tx.dp_source_tx_mgmt_interrupt 2
vds::connect_interface_net -dest dp_core_i2c_board|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/dp_core_i2c_board.interrupt_sender 5
vds::connect_interface_net -dest i2c_master|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/i2c_master.interrupt_sender 3
vds::connect_interface_net -dest jtag_uart|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/jtag_uart.irq 0
vds::connect_interface_net -dest sys_clock_timer|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/sys_clock_timer.irq 1
vds::export_interface_pin dp_tx|dp_source_tx_axi4s_vid_in dp_tx_dp_source_tx_axi4s_vid_in
vds::export_interface_pin dp_tx|dp_source_dsc_tx_tpg dp_tx_dp_source_dsc_tx_tpg
vds::export_interface_pin dp_tx|dp_source_dsc_axi4s dp_tx_dp_source_dsc_axi4s
vds::export_interface_pin dp_tx|dp_source_dsc_sync dp_tx_dp_source_dsc_sync
vds::export_interface_pin dp_tx|dp_source_dsc_ctrl dp_tx_dp_source_dsc_ctrl
vds::export_interface_pin dp_tx|dp_source_tx_im_dsc dp_tx_dp_source_tx_im_dsc
vds::export_interface_pin dp_tx|dp_source_dsc_passthrough dp_tx_dp_source_dsc_passthrough
vds::export_interface_pin dp_tx|dp_source_dsc_mgmt dp_tx_dp_source_dsc_mgmt
vds::export_interface_pin dp_tx|dp_source_tx_reconfig dp_tx_dp_source_tx_reconfig
vds::export_interface_pin dp_tx|dp_source_tx_analog_reconfig dp_tx_dp_source_tx_analog_reconfig
vds::export_interface_pin dp_tx|dp_source_tx_aux dp_tx_dp_source_tx_aux
vds::export_interface_pin dp_tx|dp_source_tx_xcvr_interface dp_tx_dp_source_tx_xcvr_interface
vds::export_interface_pin dp_rx|dp_sink_rx_axi4s_vid_out dp_rx_dp_sink_rx_axi4s_vid_out
vds::export_interface_pin dp_rx|dp_sink_dsc_tx_tpg dp_rx_dp_sink_dsc_tx_tpg
vds::export_interface_pin dp_rx|dp_sink_dsc_axi4s dp_rx_dp_sink_dsc_axi4s
vds::export_interface_pin dp_rx|dp_sink_dsc_sync dp_rx_dp_sink_dsc_sync
vds::export_interface_pin dp_rx|dp_sink_dsc_ctrl dp_rx_dp_sink_dsc_ctrl
vds::export_interface_pin dp_rx|dp_sink_tx_im_dsc dp_rx_dp_sink_tx_im_dsc
vds::export_interface_pin dp_rx|dp_sink_dsc_passthrough dp_rx_dp_sink_dsc_passthrough
vds::export_interface_pin dp_rx|dp_sink_dsc_mgmt dp_rx_dp_sink_dsc_mgmt
vds::export_interface_pin dp_rx|dp_sink_rx_analog_reconfig dp_rx_dp_sink_rx_analog_reconfig
vds::export_interface_pin dp_rx|dp_sink_rx_reconfig dp_rx_dp_sink_rx_reconfig
vds::export_interface_pin dp_rx|dp_sink_rx_stream dp_rx_dp_sink_rx_stream
vds::export_interface_pin dp_rx|dp_sink_rx_aux dp_rx_dp_sink_rx_aux
vds::export_interface_pin dp_rx|dp_sink_rx_xcvr_interface dp_rx_dp_sink_rx_xcvr_interface
vds::export_interface_pin i2c_master|i2c_serial i2c_master_i2c_serial
vds::export_interface_pin dp_core_i2c_board|i2c_serial dp_core_i2c_board_i2c_serial
vds::export_interface_pin cpu|timer_sw_agent cpu_timer_sw_agent
vds::connect_net -src cpu|dbg_reset_out -dest cpu|ndm_reset_in
vds::connect_net -src cpu|dbg_reset_out -dest rst_controller|reset_in0
vds::connect_net -src cpu_reset_bridge|out_reset -dest cpu|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest dptx_pio_actv_dim|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest dptx_pio_fps|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest dptx_pio_new_actv_dim|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest sys_clock_timer|reset
vds::connect_net -src cpu_reset_bridge|out_reset -dest rst_controller|reset_in1
vds::export_pin cpu_reset_bridge|in_reset cpu_reset_bridge_in_reset
vds::connect_net -src dp_core_axis_clk|out_clk -dest dp_rx|dp_sink_rx_axi4s_clk
vds::connect_net -src dp_core_axis_clk|out_clk -dest dp_tx|dp_source_tx_axi4s_clk
vds::export_pin dp_core_axis_clk|in_clk dp_core_axis_clk_in_clk
vds::connect_net -src dp_core_axis_reset_bridge|out_reset -dest dp_rx|dp_sink_rx_axi4s_reset
vds::connect_net -src dp_core_axis_reset_bridge|out_reset -dest dp_tx|dp_source_tx_axi4s_reset
vds::export_pin dp_core_axis_reset_bridge|in_reset dp_core_axis_reset_bridge_in_reset
vds::export_pin dp_core_pio_board|external_connection dp_core_pio_board_external_connection
vds::export_pin dp_rx|dp_sink_rx_msa_conduit dp_rx_dp_sink_rx_msa_conduit
vds::export_pin dp_rx|dp_sink_rx_params dp_rx_dp_sink_rx_params
vds::connect_net -src dp_rx_clk_16|out_clk -dest dp_rx|clk_16_in_clk
vds::export_pin dp_rx_clk_16|in_clk dp_rx_clk_16_in_clk
vds::export_pin dp_rx|dp_sink_dsc_sync_clk dp_rx_dp_sink_dsc_sync_clk
vds::export_pin dp_rx|dp_sink_rx_vid_clk dp_rx_dp_sink_rx_vid_clk
vds::export_pin dp_rx|dp_sink_tx_im_dsc_clk dp_rx_dp_sink_tx_im_dsc_clk
vds::export_pin dp_rx|pio_0_external_connection dp_rx_pio_0_external_connection
vds::export_pin dp_rx|pio_1_external_connection dp_rx_pio_1_external_connection
vds::connect_net -src dp_rx_reset_bridge|out_reset -dest dp_rx|reset_bridge_in_reset
vds::export_pin dp_rx_reset_bridge|in_reset dp_rx_reset_bridge_in_reset
vds::connect_net -src dp_tx_clk_16|out_clk -dest dp_tx|clk_16_in_clk
vds::export_pin dp_tx_clk_16|in_clk dp_tx_clk_16_in_clk
vds::export_pin dp_tx|dp_source_dsc_sync_clk dp_tx_dp_source_dsc_sync_clk
vds::export_pin dp_tx|dp_source_tx_im_dsc_clk dp_tx_dp_source_tx_im_dsc_clk
vds::export_pin dp_tx|dp_source_tx_vid_clk dp_tx_dp_source_tx_vid_clk
vds::connect_net -src dp_tx_reset_bridge|out_reset -dest dp_tx|reset_bridge_in_reset
vds::export_pin dp_tx_reset_bridge|in_reset dp_tx_reset_bridge_in_reset
vds::export_pin dptx_pio_actv_dim|external_connection dptx_pio_actv_dim_external_connection
vds::export_pin dptx_pio_fps|external_connection dptx_pio_fps_external_connection
vds::export_pin dptx_pio_new_actv_dim|external_connection dptx_pio_new_actv_dim_external_connection
vds::connect_net -src mgmt_clk|out_clk -dest cpu|clk
vds::connect_net -src mgmt_clk|out_clk -dest cpu_reset_bridge|clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_core_pio_board|clk
vds::connect_net -src mgmt_clk|out_clk -dest dptx_pio_actv_dim|clk
vds::connect_net -src mgmt_clk|out_clk -dest dptx_pio_fps|clk
vds::connect_net -src mgmt_clk|out_clk -dest dptx_pio_new_actv_dim|clk
vds::connect_net -src mgmt_clk|out_clk -dest jtag_uart|clk
vds::connect_net -src mgmt_clk|out_clk -dest sys_clock_timer|clk
vds::connect_net -src mgmt_clk|out_clk -dest rst_controller|clk
vds::connect_net -src mgmt_clk|out_clk -dest rst_translator|clk
vds::connect_net -src mgmt_clk|out_clk -dest onchip_mem|clk1
vds::connect_net -src mgmt_clk|out_clk -dest dp_core_i2c_board|clock
vds::connect_net -src mgmt_clk|out_clk -dest i2c_master|clock
vds::connect_net -src mgmt_clk|out_clk -dest dp_rx|mgmt_clk_in_clk
vds::connect_net -src mgmt_clk|out_clk -dest dp_tx|mgmt_clk_in_clk
vds::export_pin mgmt_clk|in_clk mgmt_clk_in_clk
vds::connect_net -src rst_controller|reset_out -dest rst_translator|in_reset
vds::connect_net -src rst_controller|reset_out -dest onchip_mem|reset1
vds::connect_net -src rst_translator|out_reset -dest dp_core_pio_board|reset
vds::connect_net -src rst_translator|out_reset -dest jtag_uart|reset
vds::connect_net -src rst_translator|out_reset -dest dp_core_i2c_board|reset_sink
vds::connect_net -src rst_translator|out_reset -dest i2c_master|reset_sink
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } cpu.data_manager
vds::sync_system_info
vds::validate_system
vds::save_system dp_core
post_message [pwd]
puts [pwd]
vds::generate_system -design_file $prj_vds -search_path $ -synthesis Verilog
vds::close_system dp_core

post_message "Script completed successfully."

