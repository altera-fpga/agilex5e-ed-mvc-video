# Project setup with top.qsf 
set prj_top [file join .. .. .. top.qsf]
project_open $prj_top

post_message [pwd]
puts [pwd]

# Project setup with qsys system
set prj_vds [file join src vds nios nios.vds]

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

vds::create_system nios
vds::create_cell -properties { parameters:enableDebugReset {true} parameters:resetSlave {cpu_ram.s1}  } -vlnv altera.com:ip:intel_niosv_m:26.0.0 cpu
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {100000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 cpu_clk
vds::create_cell -properties { parameters:readBufferDepth {1024} parameters:readIRQThreshold {1} parameters:writeBufferDepth {1024} parameters:writeIRQThreshold {1}  } -vlnv altera.com:ip:altera_avalon_jtag_uart:19.3.2 cpu_jtag
vds::create_cell -properties { parameters:memorySize {262144}  } -vlnv altera.com:ip:intel_onchip_memory:2.0.0 cpu_ram
vds::create_cell -properties { parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 cpu_rst
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 debug_signal
vds::create_cell -properties { parameters:direction {Input} parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 hdmitx_pio_actv_dim
vds::create_cell -properties { parameters:direction {Input}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 hdmitx_pio_fps
vds::create_cell -properties { parameters:width {32}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 hdmitx_pio_new_actv_dim
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 i2c_clocks
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 i2c_hdmi
vds::create_cell -properties { parameters:FIFO_DEPTH {32}  } -vlnv altera.com:ip:altera_avalon_i2c:19.2.6 i2c_max10
vds::create_cell -properties { parameters:AXIS_VIDEOIF_EN {1} parameters:CLOCKS_ARE_SAME {1} parameters:DIRECTION {rx} parameters:INCLUDE_EDID_RAM {1} parameters:LANE1_ORDER {2} parameters:LANE2_ORDER {1} parameters:RAM_FILEPATH {../../../../../../../src/rtl/hdmi_subsys/EDID.hex} parameters:REFCLK_MONITOR {255} parameters:SUPPORT_FRL {1} parameters:VIDEO_PROTOCOL {vvp_full}  } -vlnv altera.com:ip:altera_hdmi_gts:1.0.5 nios_hdmi_rx
vds::create_cell -properties { parameters:MAX_PENDING_RESPONSES {1}  } -vlnv altera.com:ip:altera_avalon_mm_bridge:20.1.0 nios_hdmi_rx_phy
vds::create_cell -properties { parameters:AXIS_VIDEOIF_EN {1} parameters:INCLUDE_EDID_RAM {1} parameters:LANE0_ORDER {2} parameters:LANE2_ORDER {0} parameters:REFCLK_MONITOR {255} parameters:SUPPORT_FRL {1} parameters:VIDEO_PROTOCOL {vvp_full}  } -vlnv altera.com:ip:altera_hdmi_gts:1.0.5 nios_hdmi_tx
vds::create_cell -properties { parameters:MAX_PENDING_RESPONSES {1}  } -vlnv altera.com:ip:altera_avalon_mm_bridge:20.1.0 nios_hdmi_tx_phy
vds::create_cell -properties { parameters:refclk_xcvr_freq_mhz_0 {100.000000} parameters:syspll_freq_mhz_0 {700} parameters:syspll_mod_0 {User Configuration}  } -vlnv altera.com:ip:intel_systemclk_gts:5.2.1 nios_systempll
vds::create_cell -properties { parameters:A_DITHER_BITS {0} parameters:BPS_IN {16} parameters:B_DITHER_BITS {0} parameters:G_DITHER_BITS {0} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:R_DITHER_BITS {0}  } -vlnv altera.com:ip:intel_vvp_pixel_adapter:24.9.0 nios_vvp_pixel_rx
vds::create_cell -properties { parameters:A_DITHER_BITS {0} parameters:BPS_IN {10} parameters:BPS_OUT {16} parameters:B_DITHER_BITS {0} parameters:G_DITHER_BITS {0} parameters:NUMBER_OF_COLOR_PLANES {3} parameters:PIPELINE_READY {1} parameters:PIXELS_IN_PARALLEL {2} parameters:R_DITHER_BITS {0}  } -vlnv altera.com:ip:intel_vvp_pixel_adapter:24.9.0 nios_vvp_pixel_tx
vds::create_cell -properties { parameters:direction {InOut} parameters:width {1}  } -vlnv altera.com:ip:altera_avalon_pio:19.2.4 pio_systempll
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {300000000}  } -vlnv altera.com:ip:altera_clock_bridge:19.2.0 vid_clk
vds::create_cell -properties { parameters:SYNCHRONOUS_EDGES {none}  } -vlnv altera.com:ip:altera_reset_bridge:19.2.0 vid_rst
vds::create_cell -properties { parameters:alwaysRun {true} parameters:fixedPeriod {true} parameters:period {1.8} parameters:snapshot {false}  } -vlnv altera.com:ip:altera_avalon_timer:19.3.5 wd_timer
vds::connect_interface_net -dest cpu|dm_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.dm_agent 0x00080000
vds::connect_interface_net -dest cpu|dm_agent -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/cpu.dm_agent 0x00080000
vds::connect_interface_net -dest cpu|timer_sw_agent -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu.timer_sw_agent 0x00090800
vds::connect_interface_net -dest nios_hdmi_rx|av_mm_control -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/nios_hdmi_rx.av_mm_control 0x00060000
vds::connect_interface_net -dest nios_hdmi_tx|av_mm_control -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/nios_hdmi_tx.av_mm_control 0x00040000
vds::connect_interface_net -dest cpu_jtag|avalon_jtag_slave -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu_jtag.avalon_jtag_slave 0x00090970
vds::connect_interface_net -dest i2c_clocks|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/i2c_clocks.csr 0x000908c0
vds::connect_interface_net -dest i2c_hdmi|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/i2c_hdmi.csr 0x00090880
vds::connect_interface_net -dest i2c_max10|csr -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/i2c_max10.csr 0x00090840
vds::connect_interface_net -dest nios_hdmi_rx_phy|s0 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/nios_hdmi_rx_phy.s0 0x00090400
vds::connect_interface_net -dest nios_hdmi_tx_phy|s0 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/nios_hdmi_tx_phy.s0 0x00090000
vds::connect_interface_net -dest cpu_ram|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/cpu_ram.s1 0x0000
vds::connect_interface_net -dest debug_signal|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/debug_signal.s1 0x00090960
vds::connect_interface_net -dest hdmitx_pio_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/hdmitx_pio_actv_dim.s1 0x00090940
vds::connect_interface_net -dest hdmitx_pio_fps|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/hdmitx_pio_fps.s1 0x00090930
vds::connect_interface_net -dest hdmitx_pio_new_actv_dim|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/hdmitx_pio_new_actv_dim.s1 0x00090920
vds::connect_interface_net -dest pio_systempll|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/pio_systempll.s1 0x00090950
vds::connect_interface_net -dest wd_timer|s1 -src cpu|data_manager
vds::assign_base_address -connection cpu.data_manager/wd_timer.s1 0x00090900
vds::connect_interface_net -dest cpu_ram|s1 -src cpu|instruction_manager
vds::assign_base_address -connection cpu.instruction_manager/cpu_ram.s1 0x0000
vds::connect_interface_net -dest i2c_clocks|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/i2c_clocks.interrupt_sender 10
vds::connect_interface_net -dest i2c_hdmi|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/i2c_hdmi.interrupt_sender 2
vds::connect_interface_net -dest i2c_max10|interrupt_sender -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/i2c_max10.interrupt_sender 7
vds::connect_interface_net -dest cpu_jtag|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/cpu_jtag.irq 0
vds::connect_interface_net -dest nios_hdmi_rx|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/nios_hdmi_rx.irq 9
vds::connect_interface_net -dest nios_hdmi_tx|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/nios_hdmi_tx.irq 3
vds::connect_interface_net -dest wd_timer|irq -src cpu|platform_irq_rx
vds::set_interrupt_irq -connection cpu.platform_irq_rx/wd_timer.irq 1
vds::connect_interface_net -dest nios_vvp_pixel_rx|axi4s_vid_in -src nios_hdmi_rx|axi4s_vid_out
vds::connect_interface_net -dest nios_hdmi_tx|audio_interface -src nios_hdmi_rx|audio_interface
vds::connect_interface_net -dest nios_hdmi_tx|axi4s_vid_in -src nios_vvp_pixel_tx|axi4s_vid_out
vds::export_interface_pin pio_systempll|external_connection pio_systempll_external_connection
vds::export_interface_pin nios_vvp_pixel_tx|axi4s_vid_in nios_vvp_pixel_tx_axi4s_vid_in
vds::export_interface_pin nios_vvp_pixel_rx|axi4s_vid_out nios_vvp_pixel_rx_axi4s_vid_out
vds::export_interface_pin nios_hdmi_tx_phy|m0 nios_hdmi_tx_phy_m0
vds::export_interface_pin nios_hdmi_tx|phy_interface nios_hdmi_tx_phy_interface
vds::export_interface_pin nios_hdmi_tx|i2c_interface nios_hdmi_tx_i2c_interface
vds::export_interface_pin nios_hdmi_rx_phy|m0 nios_hdmi_rx_phy_m0
vds::export_interface_pin nios_hdmi_rx|av_mm_aux_out nios_hdmi_rx_av_mm_aux_out
vds::export_interface_pin nios_hdmi_rx|phy_interface nios_hdmi_rx_phy_interface
vds::export_interface_pin nios_hdmi_rx|i2c_interface nios_hdmi_rx_i2c_interface
vds::export_interface_pin nios_hdmi_rx|hpd_interface nios_hdmi_rx_hpd_interface
vds::export_interface_pin i2c_max10|i2c_serial i2c_max10_i2c_serial
vds::export_interface_pin i2c_hdmi|i2c_serial i2c_hdmi_i2c_serial
vds::export_interface_pin i2c_clocks|i2c_serial i2c_clocks_i2c_serial
vds::export_pin cpu|dbg_reset_out cpu_dbg_reset_out
vds::connect_net -src cpu_clk|out_clk -dest cpu|clk
vds::connect_net -src cpu_clk|out_clk -dest cpu_jtag|clk
vds::connect_net -src cpu_clk|out_clk -dest debug_signal|clk
vds::connect_net -src cpu_clk|out_clk -dest hdmitx_pio_actv_dim|clk
vds::connect_net -src cpu_clk|out_clk -dest hdmitx_pio_fps|clk
vds::connect_net -src cpu_clk|out_clk -dest hdmitx_pio_new_actv_dim|clk
vds::connect_net -src cpu_clk|out_clk -dest nios_hdmi_rx_phy|clk
vds::connect_net -src cpu_clk|out_clk -dest nios_hdmi_tx_phy|clk
vds::connect_net -src cpu_clk|out_clk -dest pio_systempll|clk
vds::connect_net -src cpu_clk|out_clk -dest wd_timer|clk
vds::connect_net -src cpu_clk|out_clk -dest cpu_ram|clk1
vds::connect_net -src cpu_clk|out_clk -dest i2c_clocks|clock
vds::connect_net -src cpu_clk|out_clk -dest i2c_hdmi|clock
vds::connect_net -src cpu_clk|out_clk -dest i2c_max10|clock
vds::connect_net -src cpu_clk|out_clk -dest nios_hdmi_rx|mgmt_clk
vds::connect_net -src cpu_clk|out_clk -dest nios_hdmi_tx|mgmt_clk
vds::export_pin cpu_clk|in_clk cpu_clk_in_clk
vds::connect_net -src cpu_rst|out_reset -dest cpu|ndm_reset_in
vds::connect_net -src cpu_rst|out_reset -dest cpu|reset
vds::connect_net -src cpu_rst|out_reset -dest cpu_jtag|reset
vds::connect_net -src cpu_rst|out_reset -dest debug_signal|reset
vds::connect_net -src cpu_rst|out_reset -dest hdmitx_pio_actv_dim|reset
vds::connect_net -src cpu_rst|out_reset -dest hdmitx_pio_fps|reset
vds::connect_net -src cpu_rst|out_reset -dest hdmitx_pio_new_actv_dim|reset
vds::connect_net -src cpu_rst|out_reset -dest nios_hdmi_rx|reset
vds::connect_net -src cpu_rst|out_reset -dest nios_hdmi_rx_phy|reset
vds::connect_net -src cpu_rst|out_reset -dest nios_hdmi_tx|reset
vds::connect_net -src cpu_rst|out_reset -dest nios_hdmi_tx_phy|reset
vds::connect_net -src cpu_rst|out_reset -dest pio_systempll|reset
vds::connect_net -src cpu_rst|out_reset -dest wd_timer|reset
vds::connect_net -src cpu_rst|out_reset -dest cpu_ram|reset1
vds::connect_net -src cpu_rst|out_reset -dest i2c_clocks|reset_sink
vds::connect_net -src cpu_rst|out_reset -dest i2c_hdmi|reset_sink
vds::connect_net -src cpu_rst|out_reset -dest i2c_max10|reset_sink
vds::export_pin cpu_rst|in_reset cpu_rst_in_reset
vds::export_pin debug_signal|external_connection debug_signal_external_connection
vds::export_pin hdmitx_pio_actv_dim|external_connection hdmitx_pio_actv_dim_external_connection
vds::export_pin hdmitx_pio_fps|external_connection hdmitx_pio_fps_external_connection
vds::export_pin hdmitx_pio_new_actv_dim|external_connection hdmitx_pio_new_actv_dim_external_connection
vds::export_pin nios_hdmi_rx|i2c_clk nios_hdmi_rx_i2c_clk
vds::export_pin nios_hdmi_tx|audio_mute nios_hdmi_tx_audio_mute
vds::export_pin nios_hdmi_tx|hpd_interface nios_hdmi_tx_hpd_interface
vds::export_pin nios_systempll|o_pll_lock nios_systempll_o_pll_lock
vds::export_pin nios_systempll|o_syspll_c0 nios_systempll_o_syspll_c0
vds::export_pin nios_systempll|i_refclk_rdy nios_systempll_i_refclk_rdy
vds::export_pin nios_systempll|refclk_xcvr nios_systempll_refclk_xcvr
vds::connect_net -src vid_clk|out_clk -dest nios_hdmi_rx|axi4s_clk
vds::connect_net -src vid_clk|out_clk -dest nios_hdmi_tx|axi4s_clk
vds::connect_net -src vid_clk|out_clk -dest nios_vvp_pixel_rx|main_clock
vds::connect_net -src vid_clk|out_clk -dest nios_vvp_pixel_tx|main_clock
vds::export_pin vid_clk|in_clk vid_clk_in_clk
vds::connect_net -src vid_rst|out_reset -dest nios_hdmi_rx|axi4s_reset
vds::connect_net -src vid_rst|out_reset -dest nios_hdmi_tx|axi4s_reset
vds::connect_net -src vid_rst|out_reset -dest nios_vvp_pixel_rx|main_reset
vds::connect_net -src vid_rst|out_reset -dest nios_vvp_pixel_tx|main_reset
vds::export_pin vid_rst|in_reset vid_rst_in_reset
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } cpu.data_manager
vds::sync_system_info
vds::validate_system
vds::save_system nios
post_message [pwd]
puts [pwd]
vds::generate_system -design_file $prj_vds -search_path $ -synthesis Verilog
vds::close_system nios

post_message "Script completed successfully."
