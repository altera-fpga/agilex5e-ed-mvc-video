# (C) 2001-2025 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files from any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License Subscription 
# Agreement, Altera IP License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


# *********************************************************************
# Description
#
# Timing constraints for the DisplayPort Example Design
#
# *********************************************************************
# Time Information
# *********************************************************************
set_time_format -unit ns -decimal_places 3

#**************************************************************
# Tcl Procedure to create Rx clk group
#**************************************************************
proc set_rxclk_grp { sdi_rx_name rxphy_top_name } {

    # List out all the transceiver recovered clocks that are clocking rx_sdi instance registers (register Vsync and Hsync are used as reference)
    # Hsync reg could be recognized as separate clock, due to its function in pfd block in a parallel loopback design.
    # Vsync reg is clocked by rx_clkout2 from Direct PHY IP
    # Rx_ready_sync signal in PHY adapter block is clocked by rx_clkout from Direct PHY IP
    set rx_clkout_H_list [query_collection -all -list_format [get_clocks -nowarn -of_objects [get_keepers "${sdi_rx_name}|u_rx_protocol|sdi_receive_gen[0].u_receive|u_trs|H"]]]
    set rx_clkout_list   [query_collection -all -list_format [get_clocks -nowarn -of_objects [get_keepers "${rxphy_top_name}sdi_phy_adapter_inst|rx_comp_gen.rxdata_3ghd_gen.rx_3ghd_fifo_rdreq"]]]
    set rx_sysclk_list   [query_collection -all -list_format [get_clocks -nowarn -of_objects [get_keepers "${rxphy_top_name}sdi_phy_adapter_inst|rx_comp_gen.gxb_rx_ready_sync_inst|din_s1"]]]

    set i 0
    foreach rx_clkout_H $rx_clkout_H_list {
        set rx${i}_clkout_H $rx_clkout_H
        incr i
    }

    set i 0
    foreach rx_clkout $rx_clkout_list {
        set rx${i}_clkout $rx_clkout
        incr i
    }

    set i 0
    foreach rx_sysclk $rx_sysclk_list {
        set rx${i}_sysclk $rx_sysclk
        incr i
    }

    # Multi rate SDI will have 4 profiles while Triple rate SDI will have 2
    # Compare clkout and Hsync regs' clocks, include Hsync clock into the clock group function if they are different.
    if { [get_collection_size [get_clocks -nowarn -of_objects [get_keepers "${rxphy_top_name}sdi_phy_adapter_inst|rx_comp_gen.rxdata_3ghd_gen.rx_3ghd_fifo_rdreq"]]] == 4 } {
        if { [get_collection_size [get_clocks -nowarn -of_objects [get_keepers "${sdi_rx_name}|u_rx_protocol|sdi_receive_gen[0].u_receive|u_trs|H"]]] == 0 ||
             [string equal $rx0_clkout $rx0_clkout_H] } {
            set_clock_groups -physically_exclusive  -group [get_clocks "$rx0_clkout $rx0_sysclk"] \
                                                    -group [get_clocks "$rx1_clkout $rx1_sysclk"] \
                                                    -group [get_clocks "$rx2_clkout $rx2_sysclk"] \
                                                    -group [get_clocks "$rx3_clkout $rx3_sysclk"] \
        } else {
            set_clock_groups -physically_exclusive  -group [get_clocks "$rx0_clkout $rx0_clkout_H $rx0_sysclk"] \
                                                    -group [get_clocks "$rx1_clkout $rx1_clkout_H $rx1_sysclk"] \
                                                    -group [get_clocks "$rx2_clkout $rx2_clkout_H $rx2_sysclk"] \
                                                    -group [get_clocks "$rx3_clkout $rx3_clkout_H $rx3_sysclk"] \
        }
    } elseif { [get_collection_size [get_clocks -nowarn -of_objects [get_keepers "${rxphy_top_name}sdi_phy_adapter_inst|rx_comp_gen.rxdata_3ghd_gen.rx_3ghd_fifo_rdreq"]]] == 2 } {
        if { [get_collection_size [get_clocks -nowarn -of_objects [get_keepers "${sdi_rx_name}|u_rx_protocol|sdi_receive_gen[0].u_receive|u_trs|H"]]] == 0 ||
             [string equal $rx0_clkout $rx0_clkout_H] } {
            set_clock_groups -physically_exclusive  -group [get_clocks "$rx0_clkout $rx0_sysclk"] \
                                                    -group [get_clocks "$rx1_clkout $rx1_sysclk"]
        } else {
            set_clock_groups -physically_exclusive  -group [get_clocks "$rx0_clkout $rx0_clkout_H $rx0_sysclk"] \
                                                    -group [get_clocks "$rx1_clkout $rx1_clkout_H $rx1_sysclk"]
        }
    }
}

# *********************************************************************
# Create Clocks
# *********************************************************************
# derive_clock_uncertainty

create_clock -period "150 MHz" -name {clk150} [get_ports {fgt_refclk_150}]

create_clock -period "400Khz"  -name {board_i2c_clk}   [get_ports {board_i2c_scl}]
create_clock -period "400Khz"  -name {clk_i2c_clocks}  [get_ports i2c_clocks_scl]
create_clock -period "400Khz"  -name {clk_i2c_hdmi}    [get_ports i2c_hdmi_scl]
create_clock -period "400Khz"  -name {clk_i2c_max10}   [get_ports i2c_max10_scl]

create_clock -period "312.5 MHz"  -name {sdi_syspll_refclk} [get_ports {sdi_syspll_refclk}]
create_clock -period "100 MHz"    -name {syspll_refclk}     [get_ports {syspll_refclk}]
create_clock -period "148.5 MHz"  -name {xcvr_refclk_1485}  [get_ports {xcvr_refclk_1485}]
create_clock -period "297 MHz"    -name {txpll_refclk}      [get_ports {txpll_refclk}]

#**************************************************************
# Create clock groups by calling proc on top
#**************************************************************
set_rxclk_grp sdi_ss|nios_inst|sdi_rx_mr|sdi_rx_mr|sdi_rx_inst|rx_sdi     sdi_ss|rx_inst|

# *********************************************************************
# Constraint DisplayPort Aux port, HPD
# - AUX interface is transferring at 1Mbps
# *********************************************************************
set_false_path -from [get_ports hdmi_tx_hpd_n]
set_false_path -from [get_ports fmc_dp_rx_cable_detect] -to *
set_false_path -from [get_ports fmc_dp_rx_pwr_detect] -to *
set_false_path -to [get_ports fmc_dp_rx_hpd]
set_false_path -to [get_ports fmc_dp_rx_aux_oe]
set_false_path -to [get_ports fmc_dp_rx_aux_out]

set_false_path -to [get_ports fmc_dp_tx_aux_oe]
set_false_path -to [get_ports fmc_dp_tx_aux_out]

## *********************************************************************************
## HDMI connector 
## *********************************************************************************
set_false_path -from [get_ports hdmi_5v_detect_n]
set_false_path -to   [get_ports hdmi_rx_hpd_n]

# *********************************************************************
# Constraint LEDs, Push Button, Reset
# *********************************************************************
set_false_path -from [get_ports cpu_resetn]
set_false_path -from [get_ports user_pb[*]]

set_false_path -to   [get_ports fmc_lmh1983_init]
set_false_path -to   [get_ports fmc_fpga_fldn]
set_false_path -to   [get_ports fmc_fpga_vsyncn]
set_false_path -to   [get_ports fmc_fpga_hsyncn]

set_false_path -to   [get_ports board_i2c_scl]
set_false_path -from [get_ports board_i2c_scl]
set_false_path -to   [get_ports board_i2c_sda]
set_false_path -from [get_ports board_i2c_sda]

set_false_path -to   [get_ports i2c_max10_scl]
set_false_path -from [get_ports i2c_max10_scl]
set_false_path -to   [get_ports i2c_max10_sda]
set_false_path -from [get_ports i2c_max10_sda]

set_false_path -to   [get_ports i2c_clocks_scl]
set_false_path -from [get_ports i2c_clocks_scl]
set_false_path -to   [get_ports i2c_clocks_sda]
set_false_path -from [get_ports i2c_clocks_sda]    

set_false_path -to   [get_ports i2c_hdmi_scl]
set_false_path -from [get_ports i2c_hdmi_scl]
set_false_path -to   [get_ports i2c_hdmi_sda]
set_false_path -from [get_ports i2c_hdmi_sda]

set_false_path -to   [get_ports hdmi_rx_i2c_scl]
set_false_path -from [get_ports hdmi_rx_i2c_scl]
set_false_path -to   [get_ports hdmi_rx_i2c_sda]
set_false_path -from [get_ports hdmi_rx_i2c_sda]

set_false_path -to   [get_ports hdmi_tx_i2c_sda]
set_false_path -from [get_ports hdmi_tx_i2c_sda]
set_false_path -to   [get_ports hdmi_tx_i2c_scl]
set_false_path -from [get_ports hdmi_tx_i2c_scl]

set_false_path -to   [get_ports i2c_sdi_scl]
set_false_path -from [get_ports i2c_sdi_scl]
set_false_path -to   [get_ports i2c_sdi_sda]
set_false_path -from [get_ports i2c_sdi_sda]

set_false_path -from [get_keepers dp_ss|i_reset_ctrl|cold_reset] -to [get_keepers vvp_ss|i_vvp_pipe|vvp_pipe_emif_ddr4|vvp_pipe_emif_ddr4|emif_arch_top|arch_emif_0.arch0_1ch_per_io.arch_0|lock_sync_inst|dreg[*]]
set_false_path -from [get_keepers dp_ss|i_reset_ctrl|cold_reset] -to [get_keepers vvp_ss|i_vvp_pipe|vvp_pipe_emif_ddr4_ch2|vvp_pipe_emif_ddr4_ch2|emif_arch_top|arch_emif_0.arch0_1ch_per_io.arch_0|lock_sync_inst|dreg[*]]
set_false_path -from [get_keepers dp_ss|i_reset_ctrl|cold_reset] -to [get_keepers vvp_ss|i_vvp_pipe|vvp_pipe_emif_ddr4_ch3|vvp_pipe_emif_ddr4_ch3|emif_arch_top|arch_emif_0.arch0_1ch_per_io.arch_0|lock_sync_inst|dreg[*]]

set_false_path -from [get_keepers u_clk_dbg_01|tgl_send] -to [get_keepers u_clk_dbg_01|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_02|tgl_send] -to [get_keepers u_clk_dbg_02|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_03|tgl_send] -to [get_keepers u_clk_dbg_03|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_04|tgl_send] -to [get_keepers u_clk_dbg_04|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_05|tgl_send] -to [get_keepers u_clk_dbg_05|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_06|tgl_send] -to [get_keepers u_clk_dbg_06|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_07|tgl_send] -to [get_keepers u_clk_dbg_07|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_08|tgl_send] -to [get_keepers u_clk_dbg_08|tgl_echo_meta]
set_false_path -from [get_keepers u_clk_dbg_09|tgl_send] -to [get_keepers u_clk_dbg_09|tgl_echo_meta]

set_false_path -from [get_keepers u_clk_dbg_01|tgl_echo] -to [get_keepers u_clk_dbg_01|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_02|tgl_echo] -to [get_keepers u_clk_dbg_02|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_03|tgl_echo] -to [get_keepers u_clk_dbg_03|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_04|tgl_echo] -to [get_keepers u_clk_dbg_04|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_05|tgl_echo] -to [get_keepers u_clk_dbg_05|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_06|tgl_echo] -to [get_keepers u_clk_dbg_06|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_07|tgl_echo] -to [get_keepers u_clk_dbg_07|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_08|tgl_echo] -to [get_keepers u_clk_dbg_08|tgl_rcvd_meta]
set_false_path -from [get_keepers u_clk_dbg_09|tgl_echo] -to [get_keepers u_clk_dbg_09|tgl_rcvd_meta]

set_false_path -from [get_keepers u_clk_dbg_01|refclock_count_lat*] -to [get_keepers u_clk_dbg_01|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_02|refclock_count_lat*] -to [get_keepers u_clk_dbg_02|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_03|refclock_count_lat*] -to [get_keepers u_clk_dbg_03|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_04|refclock_count_lat*] -to [get_keepers u_clk_dbg_04|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_05|refclock_count_lat*] -to [get_keepers u_clk_dbg_05|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_06|refclock_count_lat*] -to [get_keepers u_clk_dbg_06|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_07|refclock_count_lat*] -to [get_keepers u_clk_dbg_07|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_08|refclock_count_lat*] -to [get_keepers u_clk_dbg_08|refclock_measure_meta*]
set_false_path -from [get_keepers u_clk_dbg_09|refclock_count_lat*] -to [get_keepers u_clk_dbg_09|refclock_measure_meta*]

###########################################################
if {[get_collection_size [get_pins -nocase -compatibility_mode {*hdmi_tx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*hdmi_tx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*hdmi_rx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*hdmi_rx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*dp_tx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*dp_tx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*dp_rx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*dp_rx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*sdi_tx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*sdi_tx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*sdi_rx*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*sdi_rx*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*rx_comp_gen.rxdata*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*rx_comp_gen.rxdata*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*tx_comp_gen.txdata*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*tx_comp_gen.txdata*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*cv2axi_core*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*cv2axi_core*|*|clrn}]
}

if {[get_collection_size [get_pins -nocase -compatibility_mode {*axi2cv_core*|*|clrn} -nowarn]] > 0} {
    set_false_path -to [get_pins -nocase -compatibility_mode {*axi2cv_core*|*|clrn}]
}

if {[get_collection_size [get_pins {*|auto_generated|*wraclr|dffe*a[0]} -nowarn]] > 0} {
    set_false_path -to [get_pins {*|auto_generated|*wraclr|dffe*a[0]}]
}

if {[get_collection_size [get_pins {*|auto_generated|*rdaclr|dffe*a[0]} -nowarn]] > 0} {
    set_false_path -to [get_pins {*|auto_generated|*rdaclr|dffe*a[0]}]
}

## *********************************************************************************
## HDMI connector 
## *********************************************************************************
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_rx_i2c_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_rx_i2c_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_tx_i2c_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_tx_i2c_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports board_i2c_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports board_i2c_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_max10_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_max10_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_clocks_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_clocks_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_hdmi_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_hdmi_sda]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_sdi_scl]
set_input_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_sdi_sda]

set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_rx_i2c_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_tx_i2c_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports hdmi_tx_i2c_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports board_i2c_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports board_i2c_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_max10_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_max10_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_clocks_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_clocks_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_hdmi_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_hdmi_sda]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_sdi_scl]
set_output_delay -clock [get_clocks dp_ss|i_dp_iopll|dp_iopll_refclk] 100 [get_ports i2c_sdi_sda]














