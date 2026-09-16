//  ###############################################################################
//   
//  INTEL CONFIDENTIAL
//  
//  Copyright Intel Corporation.
//  
//  This software and the related documents are Intel copyrighted materials, and
//  your use of them is governed by the express license under which they were
//  provided to you ("License"). Unless the License provides otherwise, you may
//  not use, modify, copy, publish, distribute, disclose or transmit this software
//  or the related documents without Intel's prior written permission.
//  
//  This software and the related documents are provided as is, with no express or
//  implied warranties, other than those that are expressly stated in the License.
//  
//  ###############################################################################

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/times.h>
#include "sys/alt_stdio.h"
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "priv/alt_busy_sleep.h"
#include "intel_vvp_tpg.h"
#include "intel_vvp_switch.h"
#include "intel_vvp_dil.h"
#include "intel_vvp_csc.h"
#include "intel_vvp_crs.h"
#include "intel_vvp_scaler.h"
#include "scaler_coeffs.h"

#define IMG_INFO_COLORSPACE_RGB   0
#define IMG_INFO_COLORSPACE_YUV   1

#define IMG_INFO_SUBSAMPLING_420  0
#define IMG_INFO_SUBSAMPLING_422  2
#define IMG_INFO_SUBSAMPLING_444  3

#define INIT_HSIZE  1920
#define INIT_VSIZE  1080

#define TPG_IN   0
#define HDMI_IN  1
#define DP_IN    2
#define SDI_IN   3

#define VVP_CH1 0
#define VVP_CH2 1
#define VVP_CH3 2

#define VVP_SCALER_CH1_OUT   1
#define VVP_SCALER_CH2_OUT   2
#define VVP_SCALER_CH3_OUT   3

#define HDMI_OUT    0
#define DP_OUT      1
#define SDI_OUT     2

#define HDMI_IN_DP_IN_SDI_IN    0                                       
#define TPG_IN_ALL              1
#define HDMI_IN_ALL             2
#define DP_IN_ALL               3
#define SDI_IN_ALL              4
#define SDI_IN_HDMI_IN_DP_IN    5                                       
#define HDMI_IN_SDI_IN_DP_IN    6                                      
#define SDI_IN_DP_IN_HDMI_IN    7                                      
#define DP_IN_HDMI_IN_SDI_IN    8 
#define DP_IN_HDMI_IN_TPG_IN    9                                      

#define DELAY_US_long 1000000
#define DELAY_US_SW 100000

intel_vvp_tpg_instance input_tpg;
intel_vvp_switch_instance input_switch;
intel_vvp_dil_instance dil_hdmi_rx;
intel_vvp_dil_instance dil_dp_rx;
intel_vvp_dil_instance dil_sdi_rx;
intel_vvp_csc_instance csc_hdmi_rx;
intel_vvp_csc_instance csc_dp_rx;
intel_vvp_csc_instance csc_sdi_rx;
intel_vvp_scaler_instance scale_down_ch1;
intel_vvp_scaler_instance scale_down_ch2;
intel_vvp_scaler_instance scale_down_ch3;
intel_vvp_scaler_instance scale_up_ch1; 
intel_vvp_scaler_instance scale_up_ch2;
intel_vvp_scaler_instance scale_up_ch3;
intel_vvp_switch_instance output_switch;
intel_vvp_csc_instance csc_sdi_tx;
intel_vvp_crs_instance crs_sdi_tx;

unsigned char *get_stdin();
unsigned char int_stdbuf[20];
unsigned char int_stdbuf_ptr = 0;

void cfg_tpg_in(bool bars_ena, eIntelVvpTpgBarsType bar_type, unsigned int solid_color_val, unsigned int h_dim, unsigned int v_dim) {
    intel_vvp_tpg_stop(&input_tpg);
    intel_vvp_core_set_img_info_width(&input_tpg.core_instance, h_dim);
    intel_vvp_core_set_img_info_height(&input_tpg.core_instance, v_dim);
    if (bars_ena) {
        intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
        intel_vvp_tpg_set_bars_type(&input_tpg, bar_type);
    }
    else {
        intel_vvp_tpg_set_pattern(&input_tpg, 1); // Colour bars = 0, Solid colours = 1
        if (solid_color_val == 2) {
            intel_vvp_tpg_set_colors(&input_tpg, 1023, 0, 0); // BGR        
        }
        else if (solid_color_val == 1) {
            intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR        
        }
        else {
            intel_vvp_tpg_set_colors(&input_tpg, 0, 0, 1023); // BGR        
        }
    }        
    intel_vvp_tpg_commit_writes(&input_tpg);
    intel_vvp_tpg_start(&input_tpg);      
}

void sel_input_source(unsigned int in_xpoint_type) {
    usleep(DELAY_US_SW); 
    if (in_xpoint_type == HDMI_IN_DP_IN_SDI_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, SDI_IN);     
        intel_vvp_switch_commit_writes(&input_switch);
    }    
    else if (in_xpoint_type == TPG_IN_ALL) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true,  TPG_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true,  TPG_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true,  TPG_IN);    
        intel_vvp_switch_commit_writes(&input_switch);      
    }
    else if (in_xpoint_type == HDMI_IN_ALL) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true,  HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true,  HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true,  HDMI_IN);    
        intel_vvp_switch_commit_writes(&input_switch);      
    }
    else if (in_xpoint_type == DP_IN_ALL) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true,  DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true,  DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true,  DP_IN);    
        intel_vvp_switch_commit_writes(&input_switch);      
    }
    else if (in_xpoint_type == SDI_IN_ALL) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true,  SDI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true,  SDI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true,  SDI_IN);    
        intel_vvp_switch_commit_writes(&input_switch);      
    }    
    else if (in_xpoint_type == SDI_IN_HDMI_IN_DP_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, SDI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, DP_IN);    
        intel_vvp_switch_commit_writes(&input_switch);
    }
    else if (in_xpoint_type == SDI_IN_DP_IN_HDMI_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, SDI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, HDMI_IN);    
        intel_vvp_switch_commit_writes(&input_switch);
    }    
    else if (in_xpoint_type == HDMI_IN_SDI_IN_DP_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, SDI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, DP_IN);    
        intel_vvp_switch_commit_writes(&input_switch);
    }
    else if (in_xpoint_type == DP_IN_HDMI_IN_SDI_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputConsumed);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, SDI_IN);     
        intel_vvp_switch_commit_writes(&input_switch);
    }
    else if (in_xpoint_type == DP_IN_HDMI_IN_TPG_IN) {      
        intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputEnabled);
        intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputEnabled );
        intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputConsumed );
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true, DP_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true, HDMI_IN);    
        intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true, TPG_IN);     
        intel_vvp_switch_commit_writes(&input_switch);
    }    
}

void cfg_vidpipe_channel(unsigned int channel_num, unsigned int h_dim_in, unsigned int v_dim_in, unsigned int h_dim_out, unsigned int v_dim_out) {
    usleep(DELAY_US_SW); 
    if (channel_num == VVP_CH1) {        
        // Scaler UP
        intel_vvp_scaler_set_output_width(&scale_up_ch1, h_dim_out);
        intel_vvp_scaler_set_output_height(&scale_up_ch1, v_dim_out);
        intel_vvp_scaler_commit_writes(&scale_up_ch1);    

        scaler_create_and_load_horizontal_coefficents(&scale_down_ch1, h_dim_in, h_dim_out, 0);
        scaler_create_and_load_vertical_coefficents(&scale_down_ch1, v_dim_in, v_dim_out, 0);

        // Up Scaling
        if (h_dim_in <= h_dim_out) {
            intel_vvp_scaler_set_output_width(&scale_down_ch1, h_dim_in);
            intel_vvp_scaler_set_output_height(&scale_down_ch1, v_dim_in);    
            intel_vvp_scaler_commit_writes(&scale_down_ch1);
        }
        // Down Scaling
        else {
            intel_vvp_scaler_set_output_width(&scale_down_ch1, h_dim_out);
            intel_vvp_scaler_set_output_height(&scale_down_ch1, v_dim_out);    
            intel_vvp_scaler_commit_writes(&scale_down_ch1);
        }
    }
    else if (channel_num == VVP_CH2) {              
        // Scaler UP              
        intel_vvp_scaler_set_output_width(&scale_up_ch2, h_dim_out);
        intel_vvp_scaler_set_output_height(&scale_up_ch2, v_dim_out);
        intel_vvp_scaler_commit_writes(&scale_up_ch2);    

        scaler_create_and_load_horizontal_coefficents(&scale_down_ch2, h_dim_in, h_dim_out, 0);
        scaler_create_and_load_vertical_coefficents(&scale_down_ch2, v_dim_in, v_dim_out, 0);

        // Up Scaling
        if (h_dim_in <= h_dim_out) {
            intel_vvp_scaler_set_output_width(&scale_down_ch2, h_dim_in);
            intel_vvp_scaler_set_output_height(&scale_down_ch2, v_dim_in);    
            intel_vvp_scaler_commit_writes(&scale_down_ch2);
        }
        // Down Scaling
        else {
            intel_vvp_scaler_set_output_width(&scale_down_ch2, h_dim_out);
            intel_vvp_scaler_set_output_height(&scale_down_ch2, v_dim_out);    
            intel_vvp_scaler_commit_writes(&scale_down_ch2);
        }
    }
    else if (channel_num == VVP_CH3) {              
        // Scaler UP              
        intel_vvp_scaler_set_output_width(&scale_up_ch3, h_dim_out);
        intel_vvp_scaler_set_output_height(&scale_up_ch3, v_dim_out);
        intel_vvp_scaler_commit_writes(&scale_up_ch3);    

        scaler_create_and_load_horizontal_coefficents(&scale_down_ch3, h_dim_in, h_dim_out, 0);
        scaler_create_and_load_vertical_coefficents(&scale_down_ch3, v_dim_in, v_dim_out, 0);

        // Up Scaling
        if (h_dim_in <= h_dim_out) {
            intel_vvp_scaler_set_output_width(&scale_down_ch3, h_dim_in);
            intel_vvp_scaler_set_output_height(&scale_down_ch3, v_dim_in);    
            intel_vvp_scaler_commit_writes(&scale_down_ch3);
        }
        // Down Scaling
        else {
            intel_vvp_scaler_set_output_width(&scale_down_ch3, h_dim_out);
            intel_vvp_scaler_set_output_height(&scale_down_ch3, v_dim_out);    
            intel_vvp_scaler_commit_writes(&scale_down_ch3);
        }
    }     
}

int main()
{
    printf("\n=============================================================");
    printf("\nDemo   : 4Kp60  Multi-Video Connectivity (MVC) Example Design");
    printf("\nDevkit : Agilex 5E Group B MDK Rev C");
    printf("\nHDMI   : Onboard Connectors");
    printf("\nDP     : Onboard Connectors");
    printf("\nSDI    : Nextera SDI 12G-FMC");
    printf("\nOptions from the Keyboard:");
    printf("\n=============================================================\n");
            
    printf("board_init has started...\n");
    unsigned int vvp_pipe_cfg_done      = 0;

    // HDMI Tx CVO Free-running mode @ 1920x1080p60
    unsigned int hdmi_tx_case_vid_change    = 1;   
    IOWR(VVP_PIPE_NEW_FPS_HDMITX_BASE, 0, 60);
    IOWR(VVP_PIPE_NEW_ACTV_DIM_HDMITX_BASE, 0, 0x07800438);

    // DP Tx CVO Free-running mode @ 1920x1080p60
    unsigned int dp_tx_case_vid_change    = 1;  
    IOWR(VVP_PIPE_NEW_FPS_DPITX_BASE, 0, 60);
    IOWR(VVP_PIPE_NEW_ACTV_DIM_DPTX_BASE, 0, 0x07800438);   

    // SDI Tx CVO Free-running mode @ 1920x1080p60
    unsigned int sdi_tx_case_vid_change    = 1;
    IOWR(VVP_PIPE_NEW_FPS_SDITX_BASE, 0, 60);
    IOWR(VVP_PIPE_NEW_ACTV_DIM_SDITX_BASE, 0, 0x07800438);       

    // Force non-blocking jtag uart
    unsigned char* cmd;
    int res_in = 0;
    int res_out = 0;
    
    res_out = fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    res_in = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    if ( (res_in == -1) || (res_out == -1) )
    {
        printf("FCNTL Failed\n");
    } 

//////////////////////////////////////
    printf("\nStarting VVP IP Cores Configuration\n");
    
    const intel_vvp_coefficients csc_passthrough            = CSC_PASSTHROUGH_COEFFS;
    const intel_vvp_coefficients csc_ycchd_to_rgb_10bits    = CSC_YCCHD_TO_RGB_COEFFS; // 10-bit data   
    const intel_vvp_coefficients csc_rgb_to_ycchd_10bits    = CSC_RGB_TO_YCCHD_COEFFS; // 8-bit data

    unsigned int prev_cvi_dp_csc        = 0;
    unsigned int prev_cvi_hdmi_csc      = 0;
    unsigned int prev_cvi_sdi_csc       = 0;

    unsigned int cvi_tpg_x_dim          = INIT_HSIZE; 
    unsigned int cvi_tpg_y_dim          = INIT_VSIZE;

    unsigned int cvi_hdmi_x_dim         = 0; 
    unsigned int cvi_hdmi_y_dim         = 0;
    unsigned int cvi_hdmi_x_dim_old     = 0; 
    unsigned int cvi_hdmi_y_dim_old     = 0; 

    unsigned int cvi_dp_x_dim           = 0; 
    unsigned int cvi_dp_y_dim           = 0;
    unsigned int cvi_dp_x_dim_old       = 0; 
    unsigned int cvi_dp_y_dim_old       = 0; 

    unsigned int cvi_sdi_x_dim          = 0; 
    unsigned int cvi_sdi_y_dim          = 0;
    unsigned int cvi_sdi_x_dim_old      = 0; 
    unsigned int cvi_sdi_y_dim_old      = 0; 

    unsigned int cvo_hdmi_fps           = 0; 
    unsigned int cvo_hdmi_x_dim         = 0; 
    unsigned int cvo_hdmi_y_dim         = 0;

    unsigned int cvo_dp_fps             = 0; 
    unsigned int cvo_dp_x_dim           = 0; 
    unsigned int cvo_dp_y_dim           = 0;

    unsigned int cvo_sdi_fps            = 0; 
    unsigned int cvo_sdi_x_dim          = 0; 
    unsigned int cvo_sdi_y_dim          = 0;

    unsigned int mvc_init_seq           = 1;
    unsigned int hdmi_tx_res_changed    = 0;
    unsigned int hdmi_rx_res_changed    = 0;
    unsigned int dp_tx_res_changed      = 0;
    unsigned int dp_rx_res_changed      = 0;
    unsigned int sdi_tx_res_changed     = 0;
    unsigned int sdi_rx_res_changed     = 0;
    unsigned int init_cnt               = 0;
    unsigned int case_val_reg           = 1;
    unsigned int hdmi_startup_cond      = 0;   
      
    // Output CRS
    intel_vvp_crs_init(&crs_sdi_tx, (intel_vvp_core_base)VVP_PIPE_CRS_SDI_TX_BASE);
    //kIntelVvpCrsSubsampling422, kIntelVvpCrsSubsampling444
    intel_vvp_crs_set_output_subsampling(&crs_sdi_tx, kIntelVvpCrsSubsampling422);
    intel_vvp_crs_commit_writes(&crs_sdi_tx);     

    // Output CSC
    intel_vvp_csc_init(&csc_sdi_tx, (intel_vvp_core_base)VVP_PIPE_CSC_SDI_TX_BASE);
    //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
    intel_vvp_csc_set_output_color_space(&csc_sdi_tx, kIntelVvpCsYcc);
    // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
    intel_vvp_csc_set_coeff_data(&csc_sdi_tx, &csc_rgb_to_ycchd_10bits, 2);
    intel_vvp_csc_commit_writes(&csc_sdi_tx);     

    // Output Switch (VVP-Full) 3x3
    intel_vvp_switch_init(&output_switch, (intel_vvp_core_base)VVP_PIPE_VVP_SWITCH_OUTPUT_BASE);
    intel_vvp_switch_set_input_config(&output_switch, VVP_CH1, kIntelVvpSwitchInputEnabled); // VVP CH1
    intel_vvp_switch_set_input_config(&output_switch, VVP_CH2, kIntelVvpSwitchInputEnabled); // VVP CH2
    intel_vvp_switch_set_input_config(&output_switch, VVP_CH3, kIntelVvpSwitchInputEnabled); // VVP CH3
    intel_vvp_switch_set_output_config(&output_switch, HDMI_OUT, true, VVP_CH1); // HDMI Tx connected to out_0    
    intel_vvp_switch_set_output_config(&output_switch, DP_OUT,   true, VVP_CH2); // DP Tx connected to out_0    
    intel_vvp_switch_set_output_config(&output_switch, SDI_OUT,  true, VVP_CH3); // DP Tx connected to out_0    
    intel_vvp_switch_commit_writes(&output_switch);
    
    // Scaler UP
    intel_vvp_scaler_init(&scale_up_ch1, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_UP_BASE);
    intel_vvp_scaler_set_v_bank(&scale_up_ch1, 0);
    intel_vvp_scaler_set_h_bank(&scale_up_ch1, 0);
    intel_vvp_scaler_set_output_width(&scale_up_ch1, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_up_ch1, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_up_ch1);    

    intel_vvp_scaler_init(&scale_up_ch2, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_UP_CH2_BASE);
    intel_vvp_scaler_set_v_bank(&scale_up_ch2, 0);
    intel_vvp_scaler_set_h_bank(&scale_up_ch2, 0);
    intel_vvp_scaler_set_output_width(&scale_up_ch2, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_up_ch2, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_up_ch2);    

    intel_vvp_scaler_init(&scale_up_ch3, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_UP_CH3_BASE);
    intel_vvp_scaler_set_v_bank(&scale_up_ch3, 0);
    intel_vvp_scaler_set_h_bank(&scale_up_ch3, 0);
    intel_vvp_scaler_set_output_width(&scale_up_ch3, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_up_ch3, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_up_ch3);    

    // Scaler DOWN
    intel_vvp_scaler_init(&scale_down_ch1, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_DOWN_BASE);
    intel_vvp_scaler_set_v_bank(&scale_down_ch1, 0);
    intel_vvp_scaler_set_h_bank(&scale_down_ch1, 0);
    scaler_create_and_load_horizontal_coefficents(&scale_down_ch1, INIT_HSIZE, INIT_HSIZE, 0);
    scaler_create_and_load_vertical_coefficents(&scale_down_ch1, INIT_VSIZE, INIT_VSIZE, 0);
    intel_vvp_scaler_set_output_width(&scale_down_ch1, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_down_ch1, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_down_ch1);    

    intel_vvp_scaler_init(&scale_down_ch2, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_DOWN_CH2_BASE);
    intel_vvp_scaler_set_v_bank(&scale_down_ch2, 0);
    intel_vvp_scaler_set_h_bank(&scale_down_ch2, 0);
    scaler_create_and_load_horizontal_coefficents(&scale_down_ch2, INIT_HSIZE, INIT_HSIZE, 0);
    scaler_create_and_load_vertical_coefficents(&scale_down_ch2, INIT_VSIZE, INIT_VSIZE, 0);
    intel_vvp_scaler_set_output_width(&scale_down_ch2, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_down_ch2, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_down_ch2);    

    intel_vvp_scaler_init(&scale_down_ch3, (intel_vvp_core_base)VVP_PIPE_VVP_SCALER_DOWN_CH3_BASE);
    intel_vvp_scaler_set_v_bank(&scale_down_ch3, 0);
    intel_vvp_scaler_set_h_bank(&scale_down_ch3, 0);
    scaler_create_and_load_horizontal_coefficents(&scale_down_ch3, INIT_HSIZE, INIT_HSIZE, 0);
    scaler_create_and_load_vertical_coefficents(&scale_down_ch3, INIT_VSIZE, INIT_VSIZE, 0);
    intel_vvp_scaler_set_output_width(&scale_down_ch3, INIT_HSIZE);
    intel_vvp_scaler_set_output_height(&scale_down_ch3, INIT_VSIZE);
    intel_vvp_scaler_commit_writes(&scale_down_ch3); 

    // Input CSC
    intel_vvp_csc_init(&csc_hdmi_rx, (intel_vvp_core_base)VVP_PIPE_CSC_HDMI_RX_BASE);
    //  kIntelVvpCsYcc (YCbCr) == 1, RGB == 0
    intel_vvp_csc_set_output_color_space(&csc_hdmi_rx, kIntelVvpCsRgb);
    // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
    intel_vvp_csc_set_coeff_data(&csc_hdmi_rx, &csc_passthrough, 0);
    intel_vvp_csc_commit_writes(&csc_hdmi_rx);                

    intel_vvp_csc_init(&csc_dp_rx, (intel_vvp_core_base)VVP_PIPE_CSC_DP_RX_BASE);
    //  kIntelVvpCsYcc (YCbCr) == 1, RGB == 0
    intel_vvp_csc_set_output_color_space(&csc_dp_rx, kIntelVvpCsRgb);
    // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
    intel_vvp_csc_set_coeff_data(&csc_dp_rx, &csc_passthrough, 0);
    intel_vvp_csc_commit_writes(&csc_dp_rx);                

    intel_vvp_csc_init(&csc_sdi_rx, (intel_vvp_core_base)VVP_PIPE_CSC_SDI_RX_BASE);
    //  kIntelVvpCsYcc (YCbCr) == 1, RGB == 0
    intel_vvp_csc_set_output_color_space(&csc_sdi_rx, kIntelVvpCsRgb);
    // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
    intel_vvp_csc_set_coeff_data(&csc_sdi_rx, &csc_passthrough, 0);
    intel_vvp_csc_commit_writes(&csc_sdi_rx);                

    // DIL
    intel_vvp_dil_init(&dil_hdmi_rx, (intel_vvp_core_base)VVP_PIPE_DIL_HDMI_RX_BASE); 
    // Control Register
    // 0x0 == bypass
    // 0x1 == wave 
    // 0x3 == bob 
    IOWR(VVP_PIPE_DIL_HDMI_RX_BASE, 0x53, 0x0); 
    intel_vvp_dil_commit_writes(&dil_hdmi_rx);

    intel_vvp_dil_init(&dil_dp_rx, (intel_vvp_core_base)VVP_PIPE_DIL_DP_RX_BASE); 
    // Control Register
    // 0x0 == bypass
    // 0x1 == wave 
    // 0x3 == bob 
    IOWR(VVP_PIPE_DIL_DP_RX_BASE, 0x53, 0x0); 
    intel_vvp_dil_commit_writes(&dil_dp_rx);

    intel_vvp_dil_init(&dil_sdi_rx, (intel_vvp_core_base)VVP_PIPE_DIL_SDI_RX_BASE); 
    // Control Register
    // 0x0 == bypass
    // 0x1 == wave 
    // 0x3 == bob 
    IOWR(VVP_PIPE_DIL_SDI_RX_BASE, 0x53, 0x0); 
    intel_vvp_dil_commit_writes(&dil_sdi_rx);

    // Input Switch (VVP-Full) 4x3
    intel_vvp_switch_init(&input_switch, (intel_vvp_core_base)VVP_PIPE_VVP_SWITCH_INPUT_BASE);
    intel_vvp_switch_set_input_config(&input_switch, TPG_IN, kIntelVvpSwitchInputEnabled); // TPG
    intel_vvp_switch_set_input_config(&input_switch, HDMI_IN, kIntelVvpSwitchInputConsumed ); // HDMI Rx
    intel_vvp_switch_set_input_config(&input_switch, DP_IN, kIntelVvpSwitchInputConsumed ); // DP Rx
    intel_vvp_switch_set_input_config(&input_switch, SDI_IN, kIntelVvpSwitchInputConsumed ); // SDI Rx
    intel_vvp_switch_set_output_config(&input_switch, VVP_CH1, true,  TPG_IN);    
    intel_vvp_switch_set_output_config(&input_switch, VVP_CH2, true,  TPG_IN);    
    intel_vvp_switch_set_output_config(&input_switch, VVP_CH3, true,  TPG_IN);    
    intel_vvp_switch_commit_writes(&input_switch);  

    // Input TPG (VVP-Full) with colour bars
    unsigned int int_tpg_cnt = 0;
    intel_vvp_tpg_init(&input_tpg, (intel_vvp_core_base)VVP_PIPE_VVP_TPG_BASE);
    intel_vvp_tpg_stop(&input_tpg);
    intel_vvp_core_set_img_info_width(&input_tpg.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&input_tpg.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&input_tpg.core_instance, 0);
    intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
    intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
    intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
    intel_vvp_tpg_commit_writes(&input_tpg);
    intel_vvp_tpg_start(&input_tpg);

    printf("\nWaiting for clocks to settle...\n");
    usleep(DELAY_US_long);
    usleep(DELAY_US_long);     

    printf("\n==========================================================");
    printf("\nClock Debug Information");
    printf("\n==========================================================");
    printf("\nCPU            : %.2f MHz" ,   (IORD(VVP_CLK_DBG_00_BASE, 0)*(1.0))/(10000.0));
    printf("\nAXIS Clk       : %.2f MHz" ,   (IORD(VVP_CLK_DBG_01_BASE, 0)*(1.0))/(10000.0));
    
    printf("\nHDMI PHY Rx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_06_BASE, 0)*(1.0))/(10000.0));
    printf("\nHDMI PHY Tx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_07_BASE, 0)*(1.0))/(10000.0));
    printf("\nHDMI Vid Tx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_03_BASE, 0)*(1.0))/(10000.0));
    
    printf("\nDP PHY Rx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_04_BASE, 0)*(1.0))/(10000.0));
    printf("\nDP PHY Tx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_05_BASE, 0)*(1.0))/(10000.0));
    printf("\nDP Vid Tx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_02_BASE, 0)*(1.0))/(10000.0));
    
    printf("\nSDI Rx         : %.2f MHz" ,   (IORD(VVP_CLK_DBG_08_BASE, 0)*(1.0))/(10000.0));
    printf("\nSDI Tx         : %.2f MHz" ,   (IORD(VVP_CLK_DBG_09_BASE, 0)*(1.0))/(10000.0));
    printf("\n==========================================================\n");  

    while(1) {
        // HDMI Rx CVI lock check
        cvi_hdmi_x_dim_old = cvi_hdmi_x_dim;
        cvi_hdmi_y_dim_old = cvi_hdmi_y_dim;
        cvi_hdmi_x_dim  = intel_vvp_core_get_img_info_width(&csc_hdmi_rx.core_instance);
        cvi_hdmi_y_dim  = intel_vvp_core_get_img_info_height(&csc_hdmi_rx.core_instance);

        if ( (cvi_hdmi_x_dim != cvi_hdmi_x_dim_old) ||
             (cvi_hdmi_y_dim != cvi_hdmi_y_dim_old) ) {
            hdmi_rx_res_changed = 1;
            printf("\nHDMI Rx CVI resolution has changed");
        }

        // DP Rx CVI lock check
        cvi_dp_x_dim_old = cvi_dp_x_dim;
        cvi_dp_y_dim_old = cvi_dp_y_dim;
        cvi_dp_x_dim    = intel_vvp_core_get_img_info_width(&csc_dp_rx.core_instance);
        cvi_dp_y_dim    = intel_vvp_core_get_img_info_height(&csc_dp_rx.core_instance);

        if ( (cvi_dp_x_dim != cvi_dp_x_dim_old) ||
             (cvi_dp_y_dim != cvi_dp_y_dim_old) ) {
            dp_rx_res_changed = 1;
            printf("\nDP Rx CVI resolution has changed");
        }

        // SDI Rx CVI lock check
        cvi_sdi_x_dim_old = cvi_sdi_x_dim;
        cvi_sdi_y_dim_old = cvi_sdi_y_dim;
        cvi_sdi_x_dim   = intel_vvp_core_get_img_info_width(&csc_sdi_rx.core_instance);
        cvi_sdi_y_dim   = intel_vvp_core_get_img_info_height(&csc_sdi_rx.core_instance);
 
        if ( (cvi_sdi_x_dim != cvi_sdi_x_dim_old) ||
             (cvi_sdi_y_dim != cvi_sdi_y_dim_old) ) {
            sdi_rx_res_changed = 1;
            printf("\nSDI Rx CVI resolution has changed");
        }

        // HDMI Tx CVO lock check
        cvo_hdmi_fps       = (IORD(VVP_PIPE_ACTV_DIM_HDMITX_BASE, 0x0) >> 24) & (0x00FF);  
        cvo_hdmi_x_dim     = (IORD(VVP_PIPE_ACTV_DIM_HDMITX_BASE, 0x0) >> 12) & (0x0FFF);        
        cvo_hdmi_y_dim     = (IORD(VVP_PIPE_ACTV_DIM_HDMITX_BASE, 0x0)) & (0x0FFF);  

        if (hdmi_tx_case_vid_change == 0) {
            if ( (cvo_hdmi_x_dim == 0) ||
                 (cvo_hdmi_y_dim == 0) ) {
                hdmi_startup_cond = 1;
                // printf("\nHDMI Tx might not be connected");
            }
            else if ( (cvo_hdmi_fps   != 60)   || 
                      (cvo_hdmi_x_dim != 1280) ||
                      (cvo_hdmi_y_dim != 720) ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed");
             }
            else if ( (cvo_hdmi_fps   == 60)   && 
                      (cvo_hdmi_x_dim == 1280) &&
                      (cvo_hdmi_y_dim == 720)  && 
                      (hdmi_startup_cond)  ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed after power up");
             }
        }                
        else if (hdmi_tx_case_vid_change == 1) {
            if ( (cvo_hdmi_x_dim == 0) ||
                 (cvo_hdmi_y_dim == 0) ) {
                hdmi_startup_cond = 1;
                // printf("\nHDMI Tx might not be connected");
            }
            else if ( (cvo_hdmi_fps   != 60)   || 
                      (cvo_hdmi_x_dim != 1920) ||
                      (cvo_hdmi_y_dim != 1080) ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed");
             }
            else if ( (cvo_hdmi_fps   == 60)   && 
                      (cvo_hdmi_x_dim == 1920) &&
                      (cvo_hdmi_y_dim == 1080)  && 
                      (hdmi_startup_cond)  ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed after power up");
             }
        }  
        else if (hdmi_tx_case_vid_change == 2) {
            if ( (cvo_hdmi_x_dim == 0) ||
                 (cvo_hdmi_y_dim == 0) ) {
                hdmi_startup_cond = 1;
                // printf("\nHDMI Tx might not be connected");
            }
            else if ( (cvo_hdmi_fps   != 30)   || 
                      (cvo_hdmi_x_dim != 3840) ||
                      (cvo_hdmi_y_dim != 2160) ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed");
             }
            else if ( (cvo_hdmi_fps   == 30)   && 
                      (cvo_hdmi_x_dim == 3840) &&
                      (cvo_hdmi_y_dim == 2160)  && 
                      (hdmi_startup_cond)  ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed after power up");
             }
        }  
        else if (hdmi_tx_case_vid_change == 3) {
            if ( (cvo_hdmi_x_dim == 0) ||
                 (cvo_hdmi_y_dim == 0) ) {
                hdmi_startup_cond = 1;
                // printf("\nHDMI Tx might not be connected");
            }
            else if ( (cvo_hdmi_fps   != 60)   || 
                      (cvo_hdmi_x_dim != 3840) ||
                      (cvo_hdmi_y_dim != 2160) ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed");
             }
            else if ( (cvo_hdmi_fps   == 60)   && 
                      (cvo_hdmi_x_dim == 3840) &&
                      (cvo_hdmi_y_dim == 2160)  && 
                      (hdmi_startup_cond)  ) {
                hdmi_tx_res_changed = 1;     
                printf("\nHDMI Tx CVO Resolution has changed after power up");
             }
        } 
        
        // DP Tx CVO lock check
        cvo_dp_fps      = (IORD(VVP_PIPE_ACTV_DIM_DPTX_BASE, 0x0) >> 24) & (0x00FF);  
        cvo_dp_x_dim    = (IORD(VVP_PIPE_ACTV_DIM_DPTX_BASE, 0x0) >> 12) & (0x0FFF);        
        cvo_dp_y_dim    = (IORD(VVP_PIPE_ACTV_DIM_DPTX_BASE, 0x0)) & (0x0FFF);

        if (dp_tx_case_vid_change == 0) {
            if ( (cvo_dp_fps   != 60)   || 
                 (cvo_dp_x_dim != 1280) ||
                 (cvo_dp_y_dim != 720) ) {
                dp_tx_res_changed = 1;     
                printf("\nDP Tx CVO Resolution has changed");
             }
        }                
        else if (dp_tx_case_vid_change == 1) {
            if ( (cvo_dp_fps   != 60)   || 
                 (cvo_dp_x_dim != 1920) ||
                 (cvo_dp_y_dim != 1080) ) {
                dp_tx_res_changed = 1;  
                printf("\nDP Tx CVO Resolution has changed");
             }
        }  
        else if (dp_tx_case_vid_change == 2) {
            if ( (cvo_dp_fps   != 30)   || 
                 (cvo_dp_x_dim != 3840) ||
                 (cvo_dp_y_dim != 2160) ) {
                dp_tx_res_changed = 1;
                printf("\nDP Tx CVO Resolution has changed");
             }
        }  
        else if (dp_tx_case_vid_change == 3) {
            if ( (cvo_dp_fps   != 60)   || 
                 (cvo_dp_x_dim != 3840) ||
                 (cvo_dp_y_dim != 2160) ) {
                dp_tx_res_changed = 1;
                printf("\nDP Tx CVO Resolution has changed");
             }
        }  
        
        // SDI Tx CVO lock check
        cvo_sdi_fps     = (IORD(VVP_PIPE_ACTV_DIM_SDITX_BASE, 0x0) >> 24) & (0x00FF);  
        cvo_sdi_x_dim   = (IORD(VVP_PIPE_ACTV_DIM_SDITX_BASE, 0x0) >> 12) & (0x0FFF);        
        cvo_sdi_y_dim   = (IORD(VVP_PIPE_ACTV_DIM_SDITX_BASE, 0x0)) & (0x0FFF);

        if (sdi_tx_case_vid_change == 0) {
            if ( (cvo_sdi_fps   != 60)   || 
                 (cvo_sdi_x_dim != 1280) ||
                 (cvo_sdi_y_dim != 720) ) {
                sdi_tx_res_changed = 1;     
                printf("\nSDI Tx CVO Resolution has changed");
             }
        }                
        else if (sdi_tx_case_vid_change == 1) {
            if ( (cvo_sdi_fps   != 60)   || 
                 (cvo_sdi_x_dim != 1920) ||
                 (cvo_sdi_y_dim != 1080) ) {
                sdi_tx_res_changed = 1;  
                printf("\nSDI Tx CVO Resolution has changed");
             }
        }  
        else if (sdi_tx_case_vid_change == 2) {
            if ( (cvo_sdi_fps   != 30)   || 
                 (cvo_sdi_x_dim != 3840) ||
                 (cvo_sdi_y_dim != 2160) ) {
                sdi_tx_res_changed = 1;
                printf("\nSDI Tx CVO Resolution has changed");
             }
        }  
        else if (sdi_tx_case_vid_change == 3) {
            if ( (cvo_sdi_fps   != 60)   || 
                 (cvo_sdi_x_dim != 3840) ||
                 (cvo_sdi_y_dim != 2160) ) {
                sdi_tx_res_changed = 1;
                printf("\nSDI Tx CVO Resolution has changed");
             }
        }  
      
        // CSC Status
        prev_cvi_hdmi_csc   = intel_vvp_core_get_img_info_colorspace(&csc_hdmi_rx.core_instance);
        prev_cvi_dp_csc     = intel_vvp_core_get_img_info_colorspace(&csc_dp_rx.core_instance);
        prev_cvi_sdi_csc    = intel_vvp_core_get_img_info_colorspace(&dil_sdi_rx.core_instance);

        if (!prev_cvi_hdmi_csc) {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_hdmi_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_hdmi_rx, &csc_passthrough, 0);
            intel_vvp_csc_commit_writes(&csc_hdmi_rx);                 
        }
        else {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_hdmi_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_hdmi_rx, &csc_ycchd_to_rgb_10bits, 0);
            intel_vvp_csc_commit_writes(&csc_hdmi_rx); 
        }

        if (!prev_cvi_dp_csc) {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_dp_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_dp_rx, &csc_passthrough, 0);
            intel_vvp_csc_commit_writes(&csc_dp_rx);                 
        }
        else {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_dp_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_dp_rx, &csc_ycchd_to_rgb_10bits, 0);
            intel_vvp_csc_commit_writes(&csc_dp_rx); 
        }
               
        if (!prev_cvi_sdi_csc) {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_sdi_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_sdi_rx, &csc_passthrough, 0);
            intel_vvp_csc_commit_writes(&csc_sdi_rx);                 
        }
        else {
            //  kIntelVvpCsYcc (YCbCr) == 1, kIntelVvpCsRgb == 0
            intel_vvp_csc_set_output_color_space(&csc_sdi_rx, kIntelVvpCsRgb);
            // csc_rgb_to_ycchd_10bits, csc_ycchd_to_rgb_10bits, csc_passthrough 
            intel_vvp_csc_set_coeff_data(&csc_sdi_rx, &csc_ycchd_to_rgb_10bits, 0);
            intel_vvp_csc_commit_writes(&csc_sdi_rx); 
        }

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// MVC Video loop
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
        if ( (hdmi_rx_res_changed) || (dp_rx_res_changed) || (sdi_rx_res_changed) || 
             (hdmi_tx_res_changed) || (dp_tx_res_changed) || (sdi_tx_res_changed) || 
             (mvc_init_seq) ) {

            if (hdmi_rx_res_changed)
                printf("\nHDMI Rx video resolution has changed\n");
            else if (hdmi_tx_res_changed)
                printf("\nHDMI Tx video resolution has changed\n");
            else if (dp_rx_res_changed)
                printf("\nDP Rx video resolution has changed\n");
            else if (dp_tx_res_changed)
                printf("\nDP Tx video resolution has changed\n");
            else if (sdi_rx_res_changed)
                printf("\nSDI Rx video resolution has changed\n");
            else if (sdi_tx_res_changed)
                printf("\nSDI Tx video resolution has changed\n");
            else if (mvc_init_seq)
                printf("\nMVC Initialization Sequence\n");            
            else                 
                printf("\nSomething went wrong if we get here\n");

            // Wait a few frames for resolution to settle
            usleep(DELAY_US_long);
            init_cnt = init_cnt + 1;

            // HDMI CVI lock check
            if ( (cvi_hdmi_x_dim == cvi_hdmi_x_dim_old) ||
                 (cvi_hdmi_y_dim == cvi_hdmi_y_dim_old) ) {
                printf("\nHDMI Rx CVI is locked");
            }
            else {
                printf("\nWaiting for HDMI Rx CVI resolution to change");
            }
               
            // DP CVI lock check
            if ( (cvi_dp_x_dim == cvi_dp_x_dim_old) ||
                 (cvi_dp_y_dim == cvi_dp_y_dim_old) ) {
                printf("\nDP Rx CVI is locked");
            }
            else {
                printf("\nWaiting for DP Rx CVI resolution to change");
            }
            
            // SDI CVI lock check
            if ( (cvi_sdi_x_dim == cvi_sdi_x_dim_old) ||
                 (cvi_sdi_y_dim == cvi_sdi_y_dim_old) ) {
                printf("\nSDI Rx CVI is locked");
            }
            else {
                printf("\nWaiting for SDI Rx CVI resolution to change");
            }            

            // HDMI Tx CVO lock check
            if (hdmi_tx_case_vid_change == 0) {
                if ( (cvo_hdmi_fps   == 60)   || 
                     (cvo_hdmi_x_dim == 1280) ||
                     (cvo_hdmi_y_dim == 720) ) {
                    printf("\nHDMI Tx CVO is locked to 1280x720p60");
                 }
                else {
                    printf("\nWaiting for HDMI Tx CVO resolution to change");
                }                 
            }                
            else if (hdmi_tx_case_vid_change == 1) {
                if ( (cvo_hdmi_fps   == 60)   || 
                     (cvo_hdmi_x_dim == 1920) ||
                     (cvo_hdmi_y_dim == 1080) ) {
                    printf("\nHDMI Tx is locked to 1920x1080p60");
                 }
                else {
                    printf("\nWaiting for HDMI Tx CVO resolution to change");
                } 
            }  
            else if (hdmi_tx_case_vid_change == 2) {
                if ( (cvo_hdmi_fps   == 30)   || 
                     (cvo_hdmi_x_dim == 3840) ||
                     (cvo_hdmi_y_dim == 2160) ) {
                    printf("\nHDMI Tx is locked to 3840x2160p30");
                 }
                else {
                    printf("\nWaiting for HDMI Tx CVO resolution to change");
                } 
            }  
            else if (hdmi_tx_case_vid_change == 3) {
                if ( (cvo_hdmi_fps   == 60)   || 
                     (cvo_hdmi_x_dim == 3840) ||
                     (cvo_hdmi_y_dim == 2160) ) {
                    printf("\nHDMI Tx is locked to 3840x2160p60");
                 }
                else {
                    printf("\nWaiting for HDMI Tx CVO resolution to change");
                } 
            } 

            // DP Tx CVO lock check
            if (dp_tx_case_vid_change == 0) {
                if ( (cvo_dp_fps   == 60)   || 
                     (cvo_dp_x_dim == 1280) ||
                     (cvo_dp_y_dim == 720) ) {
                    printf("\nDP Tx CVO is locked to 1280x720p60");
                 }
                else {
                    printf("\nWaiting for DP Tx CVO resolution to change");
                }                 
            }                
            else if (dp_tx_case_vid_change == 1) {
                if ( (cvo_dp_fps   == 60)   || 
                     (cvo_dp_x_dim == 1920) ||
                     (cvo_dp_y_dim == 1080) ) {
                    printf("\nDP Tx is locked to 1920x1080p60");
                 }
                else {
                    printf("\nWaiting for DP Tx CVO resolution to change");
                } 
            }  
            else if (dp_tx_case_vid_change == 2) {
                if ( (cvo_dp_fps   == 30)   || 
                     (cvo_dp_x_dim == 3840) ||
                     (cvo_dp_y_dim == 2160) ) {
                    printf("\nDP Tx is locked to 3840x2160p30");
                 }
                else {
                    printf("\nWaiting for DP Tx CVO resolution to change");
                } 
            }  
            else if (dp_tx_case_vid_change == 3) {
                if ( (cvo_dp_fps   == 60)   || 
                     (cvo_dp_x_dim == 3840) ||
                     (cvo_dp_y_dim == 2160) ) {
                    printf("\nDP Tx is locked to 3840x2160p60");
                 }
                else {
                    printf("\nWaiting for DP Tx CVO resolution to change");
                } 
            } 

            // SDI Tx CVO lock check
            if (sdi_tx_case_vid_change == 0) {
                if ( (cvo_sdi_fps   == 60)   || 
                     (cvo_sdi_x_dim == 1280) ||
                     (cvo_sdi_y_dim == 720) ) {
                    printf("\nSDI Tx CVO is locked to 1280x720p60");
                 }
                else {
                    printf("\nWaiting for SDI Tx CVO resolution to change");
                }                 
            }                
            else if (sdi_tx_case_vid_change == 1) {
                if ( (cvo_sdi_fps   == 60)   || 
                     (cvo_sdi_x_dim == 1920) ||
                     (cvo_sdi_y_dim == 1080) ) {
                    printf("\nSDI Tx is locked to 1920x1080p60");
                 }
                else {
                    printf("\nWaiting for SDI Tx CVO resolution to change");
                } 
            }  
            else if (sdi_tx_case_vid_change == 2) {
                if ( (cvo_sdi_fps   == 30)   || 
                     (cvo_sdi_x_dim == 3840) ||
                     (cvo_sdi_y_dim == 2160) ) {
                    printf("\nSDI Tx is locked to 3840x2160p30");
                 }
                else {
                    printf("\nWaiting for SDI Tx CVO resolution to change");
                } 
            }  
            else if (sdi_tx_case_vid_change == 3) {
                if ( (cvo_sdi_fps   == 60)   || 
                     (cvo_sdi_x_dim == 3840) ||
                     (cvo_sdi_y_dim == 2160) ) {
                    printf("\nSDI Tx is locked to 3840x2160p60");
                 }
                else {
                    printf("\nWaiting for SDI Tx CVO resolution to change");
                } 
            } 

            if (case_val_reg == 0) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for One-to-one (same interface type)"); 
                printf("\nCH1: HDMI Rx  -> HDMI Tx");
                printf("\nCH2: DP Rx    -> DP Tx");
                printf("\nCH3: SDI Rx   -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 0;
                cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(HDMI_IN_DP_IN_SDI_IN);                                        
                vvp_pipe_cfg_done = 1; 
            }             
            else if (case_val_reg == 1) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for One-to-any (different interface types)");                 
                printf("\nCH1: TPG Rx -> HDMI Tx");
                printf("\nCH2: TPG Rx -> DP Tx");
                printf("\nCH3: TPG Rx -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 1;
                cfg_vidpipe_channel(VVP_CH1, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(TPG_IN_ALL);                            
                vvp_pipe_cfg_done = 1; 
            }
            else if (case_val_reg == 2) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for One-to-any (different interface types)");                 
                printf("\nCH1: HDMI Rx -> HDMI Tx");
                printf("\nCH2: HDMI Rx -> DP Tx");
                printf("\nCH3: HDMI Rx -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 2;
                cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim);  
                cfg_vidpipe_channel(VVP_CH3, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim);  
                sel_input_source(HDMI_IN_ALL);
                vvp_pipe_cfg_done = 1;
            }
            else if (case_val_reg == 3) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for One-to-any (different interface types)");                 
                printf("\nCH1: DP Rx -> HDMI Tx");
                printf("\nCH2: DP Rx -> DP Tx");
                printf("\nCH3: DP Rx -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 3;
                cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(DP_IN_ALL);
                vvp_pipe_cfg_done = 1; 
            }
            else if (case_val_reg == 4) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for One-to-any (different interface types)");                 
                printf("\nCH1: SDI Rx -> HDMI Tx");
                printf("\nCH2: SDI Rx -> DP Tx");
                printf("\nCH3: SDI Rx -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 4;
                cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(SDI_IN_ALL);
                vvp_pipe_cfg_done = 1; 
            }
            else if (case_val_reg == 5) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for Any-to-any (different interface types)");                   
                printf("\nCH1: SDI Rx   -> HDMI Tx");
                printf("\nCH2: HDMI Rx  -> DP Tx");
                printf("\nCH3: DP Rx    -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 5;
                cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(SDI_IN_HDMI_IN_DP_IN);                                        
                vvp_pipe_cfg_done = 1; 
            }
            else if (case_val_reg == 6) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for Any-to-any (different interface types)");                   
                printf("\nCH1: HDMI Rx  -> HDMI Tx");
                printf("\nCH2: SDI Rx   -> DP Tx");
                printf("\nCH3: DP Rx    -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 6;
                cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(HDMI_IN_SDI_IN_DP_IN);                                       
                vvp_pipe_cfg_done = 1;
            }               
            else if (case_val_reg == 7) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for Any-to-any (different interface types)");                   
                printf("\nCH1: SDI Rx   -> HDMI Tx");
                printf("\nCH2: DP Rx    -> DP Tx");
                printf("\nCH3: HDMI Rx  -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 7;
                cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(SDI_IN_DP_IN_HDMI_IN);
                vvp_pipe_cfg_done = 1;                
            }                
            else if (case_val_reg == 8) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for Any-to-any (different interface types)");                   
                printf("\nCH1: DP Rx    -> HDMI Tx");
                printf("\nCH2: HDMI Rx  -> DP Tx");
                printf("\nCH3: SDI Rx   -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 8;
                cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(DP_IN_HDMI_IN_SDI_IN);                                        
                vvp_pipe_cfg_done = 1;
            }
            else if (case_val_reg == 9) {
                printf("\n==========================================================");
                printf("\nStarting MVC Pipes Configuration for Any-to-any (different interface types)");                   
                printf("\nCH1: DP Rx    -> HDMI Tx");
                printf("\nCH2: HDMI Rx  -> DP Tx");
                printf("\nCH3: TPG Rx   -> SDI Tx");
                printf("\n==========================================================");
                case_val_reg = 9;
                cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                cfg_vidpipe_channel(VVP_CH3, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                sel_input_source(DP_IN_HDMI_IN_TPG_IN);                                        
                vvp_pipe_cfg_done = 1;
            } 

            if (hdmi_startup_cond && hdmi_tx_res_changed) {
                if (init_cnt >= 4) {
                    //-- Usually the PHY takes longer to configured the first time
                    hdmi_startup_cond    = 0;
                }
                printf("\nStartup sequence, it will take a few seconds\n");                
            }
            else if ( (init_cnt >= 2) && (vvp_pipe_cfg_done) ) {
                printf("\nMVC Configuration Sequence Done");
                printf("\nMVC Pipes Done");

                if (cvi_hdmi_x_dim <= 100) {
                    printf("\nHDMI Rx was not detected correctly\n");
                    printf("\nCheck that is properly connected\n");
                }

                if (cvi_dp_x_dim <= 100) {
                    printf("\nDP Rx was not detected correctly\n");
                    printf("\nCheck that is properly connected\n");
                }
                
                if (cvi_sdi_x_dim <= 100) {
                    printf("\nSDI Rx was not detected correctly\n");
                    printf("\nCheck that is properly connected\n");
                }                
                
                printf("\nHDMI Rx Hactive   : %d pixels",   cvi_hdmi_x_dim);
                printf("\nHDMI Rx Vactive   : %d lines" ,   cvi_hdmi_y_dim);
                printf("\nHDMI Tx Hactive   : %d pixels",   cvo_hdmi_x_dim);
                printf("\nHDMI Tx Vactive   : %d lines" ,   cvo_hdmi_y_dim);
                printf("\nHDMI Tx FPS       : %d Hz" ,      cvo_hdmi_fps);

                printf("\nDP Rx Hactive     : %d pixels",   cvi_dp_x_dim);
                printf("\nDP Rx Vactive     : %d lines" ,   cvi_dp_y_dim);
                printf("\nDP Tx Hactive     : %d pixels",   cvo_dp_x_dim);
                printf("\nDP Tx Vactive     : %d lines" ,   cvo_dp_y_dim);
                printf("\nDP Tx FPS         : %d Hz" ,      cvo_dp_fps);

                printf("\nSDI Rx Hactive    : %d pixels",   cvi_sdi_x_dim);
                printf("\nSDI Rx Vactive    : %d lines" ,   cvi_sdi_y_dim);
                printf("\nSDI Tx Hactive    : %d pixels",   cvo_sdi_x_dim);
                printf("\nSDI Tx Vactive    : %d lines" ,   cvo_sdi_y_dim);
                printf("\nSDI Tx FPS        : %d Hz" ,      cvo_sdi_fps);

                mvc_init_seq            = 0;
                init_cnt                = 0;
                
                hdmi_rx_res_changed     = 0;
                hdmi_tx_res_changed     = 0;

                dp_rx_res_changed       = 0;
                dp_tx_res_changed       = 0;

                sdi_rx_res_changed      = 0;
                sdi_tx_res_changed      = 0;

                vvp_pipe_cfg_done       = 0;

                printf("\nPress 'h' to see the menu options\n");
            }
            else {
                //-- Keep checking that the CVI and CVO are locked...                
                printf("\nMVC configuration sequence, it will take a few seconds...\n");
            }
        }

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// MVC JTAG-UART Menu
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
        cmd = get_stdin();

        if (cmd != NULL) {
            switch (cmd[0]) {
                case 't':
                    printf("Toggle Test Pattern \n");
                    if (int_tpg_cnt < 6) {
                        int_tpg_cnt = int_tpg_cnt + 1;
                    }
                    else {
                        int_tpg_cnt = 0;
                    }

                    if (int_tpg_cnt == 1) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Color Bars \n");
                    }
                    else if (int_tpg_cnt == 2) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgGreyBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Grey Bars \n");
                    }                    
                    else if (int_tpg_cnt == 3) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgBlackWhiteBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Alternating Black and White Bars \n");
                    }                    
                    else if (int_tpg_cnt == 4) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgMixedBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Mixed Bars \n");
                    }                    
                    else if (int_tpg_cnt == 5) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 1); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 1023, 0, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Solid-Blue \n");
                    }
                    else if (int_tpg_cnt == 6) {
                        intel_vvp_tpg_set_pattern(&input_tpg, 1); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Solid-Green \n");
                    }
                    else {
                        intel_vvp_tpg_set_pattern(&input_tpg, 1); // Colour bars = 0, Solid colours = 1
                        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
                        intel_vvp_tpg_set_colors(&input_tpg, 0, 0, 1023); // BGR 
                        intel_vvp_tpg_commit_writes(&input_tpg);
                        printf("Input TPG Solid-Red \n");
                    }
                break;
                
                case 'u':
                    printf("\n==========================================================");
                    printf("\nEnable all the available output interfaces (default)");      
                    printf("\n==========================================================");
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH1, kIntelVvpSwitchInputEnabled); // VVP CH1
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH2, kIntelVvpSwitchInputEnabled); // VVP CH2
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH3, kIntelVvpSwitchInputEnabled); // VVP CH3
                    intel_vvp_switch_set_output_config(&output_switch, HDMI_OUT, true, VVP_CH1); // HDMI Tx connected to out_0    
                    intel_vvp_switch_set_output_config(&output_switch, DP_OUT,   true, VVP_CH2); // DP Tx connected to out_0    
                    intel_vvp_switch_set_output_config(&output_switch, SDI_OUT,  true, VVP_CH3); // DP Tx connected to out_0    
                    intel_vvp_switch_commit_writes(&output_switch);
                    printf("\n==========================================================\n");                                         
                break;                 
                case 'i':
                    printf("\n==========================================================");
                    printf("\nDisable HDMI Tx interfaces");      
                    printf("\n==========================================================");
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH1, kIntelVvpSwitchInputConsumed); // VVP CH1
                    intel_vvp_switch_set_output_config(&output_switch, HDMI_OUT, true, VVP_CH1); // HDMI Tx off      
                    intel_vvp_switch_commit_writes(&output_switch);
                    printf("\n==========================================================\n");                                         
                break; 
                case 'o':
                    printf("\n==========================================================");
                    printf("\nDisable DP Tx interfaces");      
                    printf("\n==========================================================");
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH2, kIntelVvpSwitchInputConsumed); // VVP CH1
                    intel_vvp_switch_set_output_config(&output_switch, DP_OUT, true, VVP_CH2); // DP Tx off      
                    intel_vvp_switch_commit_writes(&output_switch);
                    printf("\n==========================================================\n");                                         
                break; 
                case 'p':
                    printf("\n==========================================================");
                    printf("\nDisable SDI Tx interfaces");      
                    printf("\n==========================================================");
                    intel_vvp_switch_set_input_config(&output_switch, VVP_CH3, kIntelVvpSwitchInputConsumed); // VVP CH1
                    intel_vvp_switch_set_output_config(&output_switch, SDI_OUT, true, VVP_CH3); // SDI Tx off      
                    intel_vvp_switch_commit_writes(&output_switch);
                    printf("\n==========================================================\n");                                         
                break; 
                
                case '0':
                    printf("\n==========================================================");
                    printf("\nOne-to-one (same interface type)");      
                    printf("\nCH1: HDMI Rx  -> HDMI Tx");
                    printf("\nCH2: DP Rx    -> DP Tx");
                    printf("\nCH3: SDI Rx   -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 0;
                    cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(HDMI_IN_DP_IN_SDI_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;                 
                case '1':
                    printf("\n==========================================================");
                    printf("\nOne-to-any (different interface types)");                 
                    printf("\nCH1: TPG Rx -> HDMI Tx");
                    printf("\nCH2: TPG Rx -> DP Tx");
                    printf("\nCH3: TPG Rx -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 1;
                    cfg_vidpipe_channel(VVP_CH1, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(TPG_IN_ALL);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break; 
                case '2':
                    printf("\n==========================================================");
                    printf("\nOne-to-any (different interface types)");                 
                    printf("\nCH1: HDMI Rx -> HDMI Tx");
                    printf("\nCH2: HDMI Rx -> DP Tx");
                    printf("\nCH3: HDMI Rx -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 2;
                    cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim);  
                    cfg_vidpipe_channel(VVP_CH3, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim);  
                    sel_input_source(HDMI_IN_ALL);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;                                
                case '3':
                    printf("\n==========================================================");
                    printf("\nOne-to-any (different interface types)");                 
                    printf("\nCH1: DP Rx -> HDMI Tx");
                    printf("\nCH2: DP Rx -> DP Tx");
                    printf("\nCH3: DP Rx -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 3;
                    cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(DP_IN_ALL);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break; 
                case '4':
                    printf("\n==========================================================");
                    printf("\nOne-to-any (different interface types)");                 
                    printf("\nCH1: SDI Rx -> HDMI Tx");
                    printf("\nCH2: SDI Rx -> DP Tx");
                    printf("\nCH3: SDI Rx -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 4;
                    cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(SDI_IN_ALL);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;
                case '5':
                    printf("\n==========================================================");
                    printf("\nAny-to-any (different interface types)");                     
                    printf("\nCH1: SDI Rx   -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: DP Rx    -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 5;
                    cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(SDI_IN_HDMI_IN_DP_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;
                case '6':
                    printf("\n==========================================================");
                    printf("\nAny-to-any (different interface types)");                     
                    printf("\nCH1: HDMI Rx  -> HDMI Tx");
                    printf("\nCH2: SDI Rx   -> DP Tx");
                    printf("\nCH3: DP Rx    -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 6;
                    cfg_vidpipe_channel(VVP_CH1, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_dp_x_dim, cvi_dp_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(HDMI_IN_SDI_IN_DP_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;                
                case '7':
                    printf("\n==========================================================");
                    printf("\nAny-to-any (different interface types)");                     
                    printf("\nCH1: SDI Rx   -> HDMI Tx");
                    printf("\nCH2: DP Rx    -> DP Tx");
                    printf("\nCH3: HDMI Rx  -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 7;
                    cfg_vidpipe_channel(VVP_CH1, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_dp_x_dim, cvi_dp_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(SDI_IN_DP_IN_HDMI_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break;                 
                case '8':
                    printf("\n==========================================================");
                    printf("\nAny-to-any (different interface types)");                     
                    printf("\nCH1: DP Rx    -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: SDI Rx   -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 8;
                    cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_sdi_x_dim, cvi_sdi_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(DP_IN_HDMI_IN_SDI_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break; 
                case '9':
                    printf("\n==========================================================");
                    printf("\nAny-to-any (different interface types)");                     
                    printf("\nCH1: DP Rx    -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: TPG Rx   -> SDI Tx");
                    printf("\n==========================================================");
                    case_val_reg = 9;
                    cfg_vidpipe_channel(VVP_CH1, cvi_dp_x_dim, cvi_dp_y_dim, cvo_hdmi_x_dim, cvo_hdmi_y_dim); 
                    cfg_vidpipe_channel(VVP_CH2, cvi_hdmi_x_dim, cvi_hdmi_y_dim, cvo_dp_x_dim, cvo_dp_y_dim); 
                    cfg_vidpipe_channel(VVP_CH3, cvi_tpg_x_dim, cvi_tpg_y_dim, cvo_sdi_x_dim, cvo_sdi_y_dim); 
                    sel_input_source(DP_IN_HDMI_IN_TPG_IN);
                    usleep(DELAY_US_SW); 
                    printf("\n==========================================================\n");                                         
                break; 
                             
                case 'c':          
                   printf("\n==========================================================");
                   printf("\nClock Debug Information");
                   printf("\n==========================================================");
                   printf("\nCPU            : %.2f MHz" ,   (IORD(VVP_CLK_DBG_00_BASE, 0)*(1.0))/(10000.0));
                   printf("\nAXIS Clk       : %.2f MHz" ,   (IORD(VVP_CLK_DBG_01_BASE, 0)*(1.0))/(10000.0));
                    
                   printf("\nHDMI PHY Rx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_06_BASE, 0)*(1.0))/(10000.0));
                   printf("\nHDMI PHY Tx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_07_BASE, 0)*(1.0))/(10000.0));
                   printf("\nHDMI Vid Tx    : %.2f MHz" ,   (IORD(VVP_CLK_DBG_03_BASE, 0)*(1.0))/(10000.0));
                    
                   printf("\nDP PHY Rx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_04_BASE, 0)*(1.0))/(10000.0));
                   printf("\nDP PHY Tx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_05_BASE, 0)*(1.0))/(10000.0));
                   printf("\nDP Vid Tx      : %.2f MHz" ,   (IORD(VVP_CLK_DBG_02_BASE, 0)*(1.0))/(10000.0));

                   printf("\nSDI Rx         : %.2f MHz" ,   (IORD(VVP_CLK_DBG_08_BASE, 0)*(1.0))/(10000.0));
                   printf("\nSDI Tx         : %.2f MHz" ,   (IORD(VVP_CLK_DBG_09_BASE, 0)*(1.0))/(10000.0));
                   printf("\n==========================================================\n");                                         
                break;
                
                case 'd':
                    printf("\n==========================================================");
                    printf("\nVideo Dimensions");
                    printf("\n==========================================================");
                    printf("\nTPG Rx Width      : %0d pixels", cvi_tpg_x_dim);
                    printf("\nTPG Rx Height     : %0d lines",  cvi_tpg_y_dim);
    
                    printf("\nHDMI Rx Width     : %0d pixels", cvi_hdmi_x_dim);
                    printf("\nHDMI Rx Height    : %0d lines",  cvi_hdmi_y_dim);
                    printf("\nHDMI Tx Width     : %0d pixels", cvo_hdmi_x_dim);
                    printf("\nHDMI Tx Height    : %0d lines",  cvo_hdmi_y_dim);
                    printf("\nHDMI Tx FPS       : %0d Hz",     cvo_hdmi_fps);  
    
                    printf("\nDP Rx Width       : %0d pixels", cvi_dp_x_dim);
                    printf("\nDP Rx Height      : %0d lines",  cvi_dp_y_dim);
                    printf("\nDP Tx Width       : %0d pixels", cvo_dp_x_dim);
                    printf("\nDP Tx Height      : %0d lines",  cvo_dp_y_dim);
                    printf("\nDP Tx FPS         : %0d Hz",     cvo_dp_fps);
    
                    printf("\nSDI Rx Width      : %0d pixels", cvi_sdi_x_dim);
                    printf("\nSDI Rx Height     : %0d lines",  cvi_sdi_y_dim);    
                    printf("\nSDI Tx Width      : %0d pixels", cvo_sdi_x_dim);
                    printf("\nSDI Tx Height     : %0d lines",  cvo_sdi_y_dim);
                    printf("\nSDI Tx FPS        : %0d Hz",     cvo_sdi_fps); 
                    printf("\n==========================================================\n");                                         
                break;

                case 'q':          
                    if (cmd[1] == '1') {
                        printf("\n==========================================================");
                        printf("\nHDMI Tx @ 1280x720p60");
                        printf("\n==========================================================");
                        hdmi_tx_case_vid_change    = 0;
                        IOWR(VVP_PIPE_NEW_FPS_HDMITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_HDMITX_BASE, 0, 0x050002D0);
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '2') {
                        printf("\n==========================================================");
                        printf("\nHDMI Tx @ 1920x1080p60");
                        printf("\n==========================================================");
                        hdmi_tx_case_vid_change    = 1;
                        IOWR(VVP_PIPE_NEW_FPS_HDMITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_HDMITX_BASE, 0, 0x07800438);
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '3') {
                        printf("\n==========================================================");
                        printf("\nHDMI Tx @ 3840x2160p30");
                        printf("\n==========================================================");
                        hdmi_tx_case_vid_change    = 2;
                        IOWR(VVP_PIPE_NEW_FPS_HDMITX_BASE, 0, 30);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_HDMITX_BASE, 0, 0x0F000870);                      
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '4') {
                        printf("\n==========================================================");
                        printf("\nHDMI Tx @ 3840x2160p60");
                        printf("\n==========================================================");
                        hdmi_tx_case_vid_change    = 3;
                        IOWR(VVP_PIPE_NEW_FPS_HDMITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_HDMITX_BASE, 0, 0x0F000870);                      
                        printf("\n==========================================================\n");
                    }
                    hdmi_tx_res_changed = 1;
                break; 

                case 'w':          
                    if (cmd[1] == '1') {
                        printf("\n==========================================================");
                        printf("\nDisplyPort Tx @ 1280x720p60");
                        printf("\n==========================================================");
                        dp_tx_case_vid_change    = 0;
                        IOWR(VVP_PIPE_NEW_FPS_DPITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_DPTX_BASE, 0, 0x050002D0);                       
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '2') {
                        printf("\n==========================================================");
                        printf("\nDisplyPort Tx @ 1920x1080p60");
                        printf("\n==========================================================");
                        dp_tx_case_vid_change    = 1;
                        IOWR(VVP_PIPE_NEW_FPS_DPITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_DPTX_BASE, 0, 0x07800438);                   
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '3') {
                        printf("\n==========================================================");
                        printf("\nDisplyPort Tx @ 3840x2160p30");
                        printf("\n==========================================================");
                        dp_tx_case_vid_change    = 2;
                        IOWR(VVP_PIPE_NEW_FPS_DPITX_BASE, 0, 30);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_DPTX_BASE, 0, 0x0F000870);                                            
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '4') {
                        printf("\n==========================================================");
                        printf("\nDisplyPort Tx @ 3840x2160p60");
                        printf("\n==========================================================");
                        dp_tx_case_vid_change    = 3;
                        IOWR(VVP_PIPE_NEW_FPS_DPITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_DPTX_BASE, 0, 0x0F000870);                                                                     
                        printf("\n==========================================================\n");
                    }
                    dp_tx_res_changed = 1;
                break; 

                case 'e':          
                    if (cmd[1] == '1') {
                        printf("\n==========================================================");
                        printf("\nSDI Tx @ 1280x720p60 HD-YUV422");
                        printf("\n==========================================================");
                        sdi_tx_case_vid_change    = 0;
                        IOWR(VVP_PIPE_NEW_FPS_SDITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_SDITX_BASE, 0, 0x050002D0);                        
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '2') {
                        printf("\n==========================================================");
                        printf("\nSDI Tx @ 1920x1080p60 3G-YUV422");
                        printf("\n==========================================================");
                        sdi_tx_case_vid_change    = 1;
                        IOWR(VVP_PIPE_NEW_FPS_SDITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_SDITX_BASE, 0, 0x07800438);                       
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '3') {
                        printf("\n==========================================================");
                        printf("\nSDI Tx @ 3840x2160p30 6G-YUV422");
                        printf("\n==========================================================");
                        sdi_tx_case_vid_change    = 2;
                        IOWR(VVP_PIPE_NEW_FPS_SDITX_BASE, 0, 30);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_SDITX_BASE, 0, 0x0F000870);                                            
                        printf("\n==========================================================\n");
                    }
                    else if (cmd[1] == '4') {
                        printf("\n==========================================================");
                        printf("\nSDI Tx @ 3840x2160p60 12G-YUV422");
                        printf("\n==========================================================");
                        sdi_tx_case_vid_change    = 3;
                        IOWR(VVP_PIPE_NEW_FPS_SDITX_BASE, 0, 60);
                        IOWR(VVP_PIPE_NEW_ACTV_DIM_SDITX_BASE, 0, 0x0F000870);                                            
                        printf("\n==========================================================\n");
                    }
                    sdi_tx_res_changed = 1;     
                break; 

                case 'j':          
                    printf("\n==========================================================");
                    printf("\nOptions for One-to-any (different interface types): ");
                    printf("\n=========================================================="); 
                    printf("\n'1':");
                    printf("\nCH1: TPG Rx -> HDMI Tx");
                    printf("\nCH2: TPG Rx -> DP Tx");
                    printf("\nCH3: TPG Rx -> SDI Tx");
                    
                    printf("\n'2':");
                    printf("\nCH1: HDMI Rx  -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: HDMI Rx  -> SDI Tx");

                    printf("\n'3':");
                    printf("\nCH1: DP Rx    -> HDMI Tx");
                    printf("\nCH2: DP Rx    -> DP Tx");
                    printf("\nCH3: DP Rx    -> SDI Tx");

                    printf("\n'4':");
                    printf("\nCH1: SDI Rx   -> HDMI Tx");
                    printf("\nCH2: SDI Rx   -> DP Tx");
                    printf("\nCH3: SDI Rx   -> SDI Tx");
                    printf("\n========================================================\n");                      
                break; 

                case 'k':                              
                    printf("\n==========================================================");
                    printf("\nOptions for Any-to-any (different interface types) ");
                    printf("\n=========================================================="); 
                    printf("\n'5':");
                    printf("\nCH1: SDI Rx   -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: DP Rx    -> SDI Tx");

                    printf("\n'6':");
                    printf("\nCH1: HDMI Rx  -> HDMI Tx");
                    printf("\nCH2: SDI Rx   -> DP Tx");
                    printf("\nCH3: DP Rx    -> SDI Tx");

                    printf("\n'7':");
                    printf("\nCH1: SDI Rx   -> HDMI Tx");
                    printf("\nCH2: DP Rx    -> DP Tx");
                    printf("\nCH3: HDMI Rx  -> SDI Tx");

                    printf("\n'8':");
                    printf("\nCH1: DP Rx    -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: SDI Rx   -> SDI Tx");                    

                    printf("\n'9':");
                    printf("\nCH1: DP Rx    -> HDMI Tx");
                    printf("\nCH2: HDMI Rx  -> DP Tx");
                    printf("\nCH3: TPG Rx   -> SDI Tx");  
                    printf("\n========================================================\n");                      
                break; 

                case 'l':          
                    printf("\n==========================================================");
                    printf("\nTx Standard Options per Interface Types");
                    printf("\n=========================================================="); 
                    printf("\n'q1'  : HDMI Tx - 1280x720p60  (RGB)");                
                    printf("\n'q2'  : HDMI Tx - 1920x1080p60 (RGB)");                
                    printf("\n'q3'  : HDMI Tx - 3840x2160p30 (RGB)"); 
                    printf("\n'q4'  : HDMI Tx - 3840x2160p60 (RGB)"); 

                    printf("\n'w1'  : DP Tx   - 1280x720p60  (RGB)");                
                    printf("\n'w2'  : DP Tx   - 1920x1080p60 (RGB)");                
                    printf("\n'w3'  : DP Tx   - 3840x2160p30 (RGB)"); 
                    printf("\n'w4'  : DP Tx   - 3840x2160p60 (RGB)"); 

                    printf("\n'e1'  : SDI Tx  - 1280x720p60  ( HD-YUV422)");                
                    printf("\n'e2'  : SDI Tx  - 1920x1080p60 ( 3G-YUV422)");                
                    printf("\n'e3'  : SDI Tx  - 3840x2160p30 ( 6G-YUV422)"); 
                    printf("\n'e4'  : SDI Tx  - 3840x2160p60 (12G-YUV422)"); 
                    printf("\n========================================================\n");                      
                break; 
                case 'h':
                    printf("\n=============================================================");
                    printf("\n=============================================================");
                    printf("\nDemo   : 4Kp60  Multi-Video Connectivity (MVC) Example Design");
                    printf("\nDevkit : Agilex 5E Group B MDK Rev C");
                    printf("\nHDMI   : Onboard Connectors");
                    printf("\nDP     : Onboard Connectors");
                    printf("\nSDI    : Nextera SDI 12G-FMC");
                    printf("\nOptions from the Keyboard:");
                    printf("\n=============================================================");
                    printf("\n=============================================================");
                    printf("\n'h'   : Help Menu");
                    printf("\n'c'   : Debug Information  - Clocks Status");                
                    printf("\n'd'   : Debug Information  - Video Dimensions");                

                    printf("\n't'   : Toggle TPG Patterns");                
                    printf("\n'0'   : One-to-one (same interface type)");                    
                    printf("\n'j'   : Options for One-to-any use cases (different interface types)");                
                    printf("\n'k'   : Options for Any-to-any use cases (different interface types)");                
                    printf("\n'l'   : Select Tx Video Standard Options per Interface Types");  
                    printf("\n'u'   : Enable all the available output interfaces (default)");  
                    printf("\n'i'   : Disable HDMI Tx interfaces");  
                    printf("\n'o'   : Disable DP Tx interfaces");  
                    printf("\n'p'   : Disable SDI Tx interfaces");  
                    printf("\n========================================================\n");  
                break;
                
                default:
                break;
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

unsigned char *get_stdin()
{
  int d,i;
  char *c = (char *)&d;

  i = read(0,(void*)&d,1); // 0 = stdin
  if((i < 1) || (*c == EOF))
    return NULL;

  printf("%c",d);
  if(*c == '\n' || *c == 0xd)
  {
    // input is complete
    if(*c == 0xd)
      printf("\n");
    int_stdbuf[int_stdbuf_ptr] = 0x00;
    int_stdbuf_ptr = 0;
    return int_stdbuf;
  }

  int_stdbuf[int_stdbuf_ptr++] = *c;
  if((int_stdbuf_ptr + 1) == sizeof(int_stdbuf))
  {
    // buffer is full
    int_stdbuf[int_stdbuf_ptr] = 0x00;
    int_stdbuf_ptr = 0;
    return int_stdbuf;
  }

  return NULL;
}

