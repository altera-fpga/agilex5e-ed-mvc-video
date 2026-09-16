#  ###############################################################################
#
#  INTEL CONFIDENTIAL
#
#  Copyright 2020-2021 Intel Corporation.
#
#  This software and the related documents are Intel copyrighted materials, and
#  your use of them is governed by the express license under which they were
#  provided to you ("License"). Unless the License provides otherwise, you may
#  not use, modify, copy, publish, distribute, disclose or transmit this software
#  or the related documents without Intel's prior written permission.
#
#  This software and the related documents are provided as is, with no express or
#  implied warranties, other than those that are expressly stated in the License.
#
#  ###############################################################################
package require ::quartus::project
package require ::quartus::vds

# Script directory
set script_dir [file dirname [info script]]

puts [pwd]

# Go to .qsf project
set prj_path [file normalize [file join $script_dir ..]]
cd $prj_path
puts "########################################################"
puts "# INFO : generation_step_vds -> .QSF location          #"
puts "########################################################"
post_message [pwd]

# Project handling
set need_to_close_project 0

if {[is_project_open]} {
    if {[string compare $quartus(project) "top"]} {
        error "Project top is not open"
    }
} else {
    if {[project_exists top]} {
        project_open -revision top top
    } else {
        error "Project top does not exist"
    }
    set need_to_close_project 1
}

puts "########################################################"
puts "# INFO : VDS Generation                                #"
puts "########################################################"
puts [pwd]
set dp_ss_path    [file normalize [file join src vds dp_subsys]]
set hdmi_ss_path  [file normalize [file join src vds hdmi_subsys]]
set sdi_ss_path   [file normalize [file join src vds sdi_subsys]]
set vvp_ss_path   [file normalize [file join src vds vvp_subsys]]
set prj_path_dp   [file join .. .. ..]
set prj_path_hdmi [file join .. .. ..]
set prj_path_sdi  [file join .. .. ..]
set prj_path_vvp  [file join .. .. ..]

set quartus_sh [auto_execok quartus_sh]
if {$quartus_sh eq ""} {
    error "quartus_sh not found"
}

# Check if the VDS project has been built before.
# If so, do not rebuild it again.
set dirname_vds [file normalize [file join gen vds]]
if {[file isdirectory $dirname_vds]} {
    puts "Directory already exists, hence just compile the project in its current form"
} else {
    puts "Directory does not exist, hence we need to create the VDS project"

    puts "########################################################"
    puts "# INFO : DP_CORE generation                            #"
    puts "########################################################"
    cd $dp_ss_path
    puts [pwd]
    exec -ignorestderr -- quartus_sh -t dp_subsys.tcl
    cd $prj_path_dp
    post_message [pwd]

    puts "########################################################"
    puts "# INFO : HDMI_CORE generation                          #"
    puts "########################################################"
    cd $hdmi_ss_path
    puts [pwd]
    exec -ignorestderr -- quartus_sh -t hdmi_subsys.tcl
    cd $prj_path_hdmi
    post_message [pwd]

    puts "########################################################"
    puts "# INFO : SDI_CORE generation                           #"
    puts "########################################################"
    cd $sdi_ss_path
    puts [pwd]
    exec -ignorestderr -- quartus_sh -t sdi_subsys.tcl
    cd $prj_path_sdi
    post_message [pwd]

    puts "########################################################"
    puts "# INFO : VVP PIPE generation                           #"
    puts "########################################################"
    cd $vvp_ss_path
    puts [pwd]
    exec -ignorestderr -- quartus_sh -t vpp_subsys.tcl
    cd $prj_path_vvp
    post_message [pwd]

    # Return to script dir
    set scripts_path [file normalize [file join script]]
    cd $scripts_path
    post_message [pwd]

    # Close project
    if {$need_to_close_project} {
        project_close
    }

    puts "########################################################"
    puts "# INFO : DP SW APP rebuild                             #"
    puts "########################################################"
    post_message [pwd]
    source [file join swapp_dp_gen.tcl]

    puts "########################################################"
    puts "# INFO : HDMI SW APP rebuild                           #"
    puts "########################################################"
    post_message [pwd]
    source [file join swapp_hdmi_gen.tcl]

    puts "########################################################"
    puts "# INFO : SDI SW APP rebuild                            #"
    puts "########################################################"
    post_message [pwd]
    source [file join swapp_sdi_gen.tcl]

    puts "########################################################"
    puts "# INFO : MVC SW APP rebuild                            #"
    puts "########################################################"
    post_message [pwd]
    source [file join swapp_mvc_gen.tcl]

    puts "##############################################"
    puts "# INFO : Generation is fully done            #"
    puts "##############################################"
    post_message [pwd]  

    set prj_path [file normalize [file join ..]]
    cd $prj_path
    post_message [pwd]

    set temp1 "set_global_assignment -name SDC_FILE src/rtl/jtag.sdc"
    set temp2 "set_global_assignment -name SDC_FILE src/rtl/top.sdc"
    set temp3 "set_global_assignment -name DESIGN_ASSISTANT_WAIVER_FILE da_drc.dawf"

    set filename "top.qsf"
    set fileId [open $filename "a"]
    puts $fileId $temp1
    puts $fileId $temp2
    puts $fileId $temp3
    flush $fileId 
    close $fileId

    set script_dir [file normalize [file join script]]
    cd $script_dir
    post_message [pwd]     
}


