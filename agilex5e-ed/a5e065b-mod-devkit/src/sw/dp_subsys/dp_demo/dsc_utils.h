
//
// ********************************************************************************
// DisplayPort Core DSC utility functions
//
// Description:
//
// ********************************************************************************


#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "btc_dptx_syslib.h"


#define VVP_CVO_VID_PID_REG                   0x000
#define VVP_CVO_VERSION_REG                   0x001

#define VVP_CVO_FALLBACK_REG                  0x055

#define VVP_CVO_VID_FREQ_REG                  0x0A6

#define VVP_CVO_STATUS_REG                    0x090
#define VVP_CVO_COMMIT_REG                    0x091
#define VVP_CVO_MODE_REG                      0x092
#define VVP_CVO_RESET_POS_REG                 0x093
#define VVP_CVO_TOTALS_REG                    0x094
#define VVP_CVO_HB_END_REG                    0x095
#define VVP_CVO_V1B_POS_REG                   0x096
#define VVP_CVO_V2B_POS_REG                   0x097
#define VVP_CVO_FIELDS_START_REG              0x098
#define VVP_CVO_HS_POS_REG                    0x099
#define VVP_CVO_V1S_START_REG                 0x09A
#define VVP_CVO_V1S_END_REG                   0x09B
#define VVP_CVO_V2S_START_REG                 0x09C
#define VVP_CVO_V2S_END_REG                   0x09D
#define VVP_CVO_JITTER_CONT_REG               0x09E

#define VVP_CVO_VID_FREQ_REG                  0x0A6

#define PPS_CTRL_REG                           0x16
#define PPS_STATUS_REG                         0x17
#define PPS_REG                                0x18
#define HTOTAL_REG                             0x19
#define HSTART_REG                             0x1A
#define MSA_MVID_REG                           0x1B
#define PACKET_CTRL1_REG                       0x1B
#define PACKET_CTRL2_REG                       0x1C
#define LINE_MARKERS_REG                       0x1D
#define CHUNK_CTRL_REG                         0x1E
#define CHUNK_CTRL_REG1                        0x1F


#define DPTX_MSA_MVID                          0x00
#define DPTX_MSA_NVID                          0x01
#define DPTX_MSA_HTOTAL                        0x02
#define DPTX_MSA_VTOTAL                        0x03
#define DPTX_MSA_HSP                           0x04
#define DPTX_MSA_HSW                           0x05
#define DPTX_MSA_HSTART                        0x06
#define DPTX_MSA_VSTART                        0x07
#define DPTX_MSA_VSP                           0x08
#define DPTX_MSA_VSW                           0x09
#define DPTX_MSA_HWIDTH                        0x0a
#define DPTX_MSA_VHEIGHT                       0x0b
#define DPTX_MSA_MISC0                         0x0c
#define DPTX_MSA_MISC1                         0x0d
#define DPTX_MSA_COLOUR                        0x0e
#define DPTX_VBID                              0x0f
#define DPTX_CRC_R                             0x10
#define DPTX_CRC_G                             0x11
#define DPTX_CRC_B                             0x12
#define DPTX_AUD_CONTROL                       0x13
#define DPTX_DSC_CAPAB                         0x15
#define DPTX_DSC_CONTROL                       0x16
#define DPTX_DSC_STATUS                        0x17
#define DPTX_DSC_PPS                           0x18
#define DPTX_DSC_HTOTAL                        0x19
#define DPTX_DSC_HSTART                        0x1A
#define DPTX_DSC_PACKET_CTRL1                  0x1B
#define DPTX_DSC_PACKET_CTRL2                  0x1C
#define DPTX_DSC_LINE_MARKERS                  0x1D
#define DPTX_DSC_CHUNK_CTRL                    0x1E
#define DPTX_DSC_TU_CALC_CTRL                  0x1F


struct pps_parameters 
{
    unsigned int pic_height;
    unsigned int pic_width;
    unsigned int bpp;
    unsigned int slice_height;
    unsigned int slice_width;
    unsigned int slices_per_line;
    unsigned int dsc_packet_start;
    unsigned int dsc_packet_length;
    unsigned int dsc_packet_gap;
    unsigned int dsc_start_line;
    unsigned int dsc_end_line;
    unsigned int dsc_line_length;
    unsigned int dsc_chunk_length;
    unsigned int chunk_size;
    unsigned int dsc_line_bytes;
    unsigned int line_pixels_per_dsc_byte;
};


struct mode_parameters 
{
    unsigned int frame_rate;
    unsigned int adjusted_frame_rate;
    unsigned int pixel_clk;
    unsigned int HTotal;
    unsigned int HActive;
    unsigned int HBlank;
    unsigned int HFront;
    unsigned int HSync;
    unsigned int VTotal;
    unsigned int VActive;
    unsigned int VBlank;
    unsigned int VFront;
    unsigned int VSync;
    unsigned int mode_valid;
    unsigned int dsc_packet_start;
    unsigned int dsc_packet_length;
    unsigned int dsc_packet_gap;

 
};


void         program_cvo_tpg(int is_dsc, struct mode_parameters* selected_mode_parameters, struct pps_parameters* dsc_data_pps_parameters);
void         program_dsc_regs(struct mode_parameters* selected_mode_parameters, struct pps_parameters* dsc_data_pps_parameters);
void         read_pps_params(struct pps_parameters* dsc_data_pps_parameters);
void         load_standard_timing( unsigned int EDID_standard_timing, int* mode_id, struct pps_parameters* dsc_data_pps_parameters, struct mode_parameters* selected_mode_parameters);
void         load_resolutions   (int is_dsc, struct mode_parameters* mode_param_array, struct pps_parameters* dsc_data_pps_parameters, int* modes_found);
void         select_resolutions (int is_dsc, struct mode_parameters* mode_param_array, struct pps_parameters* dsc_data_pps_parameters, int* selected_mode);
void         enable_dsc();
void         disable_dsc();
void         dump_dsc_dpcd();