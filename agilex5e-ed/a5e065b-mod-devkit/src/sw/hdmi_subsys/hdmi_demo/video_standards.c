#include <inttypes.h>
#include <string.h>
#include "config.h"
#include "video_standards.h"
#include "utils.h"

// Definition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Vid", format, ##__VA_ARGS__))

/**
 * @brief Array of predefined HDMI TX video standards.
 *
 * This array contains the configuration parameters for various HDMI TX video standards.
 */
hdmi_tx_standard_t _gvideo_standards[kCVO_MAX_MODE] =
{
    //Standard Enum            vic, rate { interl,  seq, width, height, f1_h, h blanking, v_blanking, f0_blanking, active_line,field_toggles, ancillary, sync_pol},
    {kCVO_640x480P60_MODE,     1,   60.0, {false, false,  640,  480,   0,   16,  96,  160, 10,  2, 45, 0, 0,  0, 43,   0,   0, 0, 10,   0, false, false}},
    {kCVO_720x480P60_MODE,     2,   60.0, {false, false,  720,  480,   0,   16,  62,  138,  9,  6, 45, 0, 0,  0, 43,   0,   0, 0, 10,   0, false, false}},
    {kCVO_800x600P60_MODE,     0,   60.0, {false, false,  800,  600,   0,   40, 128,  256,  1,  4, 28, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_800x600P75_MODE,     0,   75.0, {false, false,  800,  600,   0,   16,  80,  256,  1,  3, 25, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1024x768P60_MODE,    0,   60.0, {false, false, 1024,  768,   0,   24, 136,  320,  3,  6, 38, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1280x720P50_MODE,    19,  50.0, {false, false, 1280,  720,   0,  440,  40,  700,  5,  5, 30, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1280x720P60_MODE,    4,   60.0, {false, false, 1280,  720,   0,  110,  40,  370,  5,  5, 30, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1920x1080P24_MODE,   32,  24.0, {false, false, 1920, 1080,   0,   88,  44,  830,  4,  5, 45, 0, 0,  0, 42,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1920x1080P30_MODE,   34,  30.0, {false, false, 1920, 1080,   0,   88,  44,  280,  4,  5, 45, 0, 0,  0, 42,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1920x1080P50_MODE,   31,  50.0, {false, false, 1920, 1080,   0,  528,  44,  720,  4,  5, 45, 0, 0,  0, 42,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_1920x1080P60_MODE,   16,  60.0, {false, false, 1920, 1080,   0,   88,  44,  280,  4,  5, 45, 0, 0,  0, 42,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_3840x2160P24_MODE,   93,  24.0, {false, false, 3840, 2160,   0,  176,  88, 1660,  8, 10, 90, 0, 0,  0, 84,   0,   0, 0, 20,   0, true,  true}},
    {kCVO_3840x2160P30_MODE,   95,  30.0, {false, false, 3840, 2160,   0,  176,  88,  560,  8, 10, 90, 0, 0,  0, 84,   0,   0, 0, 20,   0, true,  true}},
    {kCVO_3840x2160P50_MODE,   96,  50.0, {false, false, 3840, 2160,   0, 1056,  88, 1440,  8, 10, 90, 0, 0,  0, 84,   0,   0, 0, 20,   0, true,  true}},
    {kCVO_3840x2160P60_MODE,   97,  60.0, {false, false, 3840, 2160,   0,  176,  88,  560,  8, 10, 90, 0, 0,  0, 84,   0,   0, 0, 20,   0, true,  true}},
    {kCVO_4320P30_MODE,        196, 30.0, {false, false, 7680, 4320,   0,  552, 176, 1320, 16, 20, 80, 0, 0,  0, 65,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_4320P60_MODE,        0,   60.0, {false, false, 7680, 4320,   0,  552, 176, 1320, 16, 20, 80, 0, 0,  0, 65,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_4320P_420_MODE,      0,   60.0, {false, false, 3840, 4320,   0,  276,  88,  660, 16, 20, 80, 0, 0,  0, 65,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_2160P_420_MODE,      0,   60.0, {false, false, 1920, 2160,   0,   88,  44,  280,  8, 10, 90, 0, 0,  0, 84,   0,   0, 0, 20,   0, true,  true}},
    {kCVO_1080P_420_MODE,      0,   60.0, {false, false,  960, 1080,   0,   44,  22,  140,  4,  5, 45, 0, 0,  0, 42,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_720P_420_MODE,       0,   60.0, {false, false,  640,  720,   0,   55,  20,  185,  5,  5, 30, 0, 0,  0, 26,   0,   0, 0, 10,   0, true,  true}},
    {kCVO_480P_420_MODE,       0,   60.0, {false, false,  360,  480,   0,    8,  31,   69,  9,  6, 45, 0, 0,  0, 43,   0,   0, 0, 10,   0, false, false}},
    {kCVO_1920x1080I60_MODE,   0,   30.0, { true, false, 1920,  540, 540,   88,  44,  280,  2,  5, 22, 2, 5, 23, 22, 562, 564, 1, 10, 562, true,  true}},
    {kCVO_1080I_SDI_MODE,      0,   30.0 / 1.001, {true, false, 1920, 540, 540, 0, 0, 280, 0, 0, 22, 0, 0, 23, 21, 561, 564, 1, 10, 562, true, true}},
};                                       

/**
 * @brief Calculates the pixel clock for a given video standard.
 *
 * This function calculates the pixel clock based on the video standard and the specified centi-rate.
 *
 * @param[in] pStandard Pointer to the HDMI TX video standard.
 * @param[in] centi_rate The centi-rate for the video standard.
 * @return The calculated pixel clock.
 */
uint32_t get_pixel_clock(const hdmi_tx_standard_t *pStandard, uint32_t centi_rate)
{
    if ((pStandard == NULL))
    {
        return 0;
    }

    float rate = 0;

    // But if we have a centi-rate from a received standard then use that
    switch (centi_rate)
    {
    case 0: // Not set so use the default
        rate = pStandard->_frame_rate;
        break;
    case 5994:
        rate = 60 / 1.001;
        break;
    case 2997:
        rate = 30 / 1.001;
        break;
    case 2397:
        rate = 24 / 1.001;
        break;
    default:
        rate = ((float)centi_rate / 100.0);
        break;
    }
    uint32_t full_x = pStandard->_cvo_params.sample_count + pStandard->_cvo_params.h_blanking;
    uint32_t full_y = (pStandard->_cvo_params.f0_line_count + pStandard->_cvo_params.f1_line_count) + (pStandard->_cvo_params.v_blanking + pStandard->_cvo_params.f0_v_blanking);
    return (uint32_t)((float)full_x * (float)full_y * rate);
}

#define HDMI14_MAX_TMDS_FREQ (340 * 1000 * 1000)

/**
 * @brief Calculates TMDS settings for a given video standard.
 *
 * This function calculates the TMDS frequency and ratio based on the video standard, color depth, and color space.
 *
 * @param[in] pStandard Pointer to the video standard.
 * @param[in] color_depth The color depth of the video standard.
 * @param[in] color_space The color space of the video standard.
 * @param[out] pTmdsFreq Pointer to store the calculated TMDS frequency.
 * @param[out] pTmdsRatio Pointer to store the calculated TMDS ratio.
 */
void get_tmds_settings(const video_standard_t *pStandard, const intel_hdmi_color_depth_t color_depth, intel_hdmi_color_space_t color_space,
                       uint32_t *pTmdsFreq, intel_hdmi_tmds_ratio_t *pTmdsRatio)
{
    if ((pStandard == NULL) || (pTmdsFreq == NULL) || (pTmdsRatio == NULL))
    {
        return;
    }

    // Calculate the pixel clock
    uint32_t pixel_clock = get_pixel_clock(pStandard->_video_timing, pStandard->_details._image_config.centi_rate);

    uint8_t color_space_divider = 1; //no need to differentiate between 420 or 444 since pixel_clock calculation already consider that.
    uint8_t color_depth_multiplexer = 0;

    if (color_depth == intel_hdmi_color_depth_bpc_8)
    {
        color_depth_multiplexer = 8;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_10)
    {
        color_depth_multiplexer = 10;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_12)
    {
        color_depth_multiplexer = 12;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_16)
    {
        color_depth_multiplexer = 16;
    }

    *pTmdsFreq = (uint32_t)(((float)pixel_clock * (float)color_depth_multiplexer) / ((float)color_space_divider * 8.0));
    if (*pTmdsFreq <= HDMI14_MAX_TMDS_FREQ)
    {
        *pTmdsRatio = intel_hdmi_tmds_ratio_one_over_10;
    }
    else
    {
        *pTmdsRatio = intel_hdmi_tmds_ratio_one_over_40;
    }
}

void get_frl_settings(const video_standard_t *pStandard, const intel_hdmi_color_depth_t color_depth, intel_hdmi_color_space_t color_space,
                       uint32_t *pPixelFreq, intel_hdmi_tmds_ratio_t *pTmdsRatio)
{
    if ((pStandard == NULL) || (pPixelFreq == NULL) || (pTmdsRatio == NULL))
    {
        return;
    }
    
	uint32_t centi_rate = 0;
	uint8_t  vic = pStandard->_details._vic;
	
	if ((vic == 1) || (vic == 2) || (vic == 4) || (vic == 16) || (vic == 97))
	{
		centi_rate = 6000;
	}
	else if (vic == 93)
	{
		centi_rate = 2400;
	}
	else 
	{
		centi_rate = 3000;
	}

    // Calculate the pixel clock
    uint32_t pixel_clock = get_pixel_clock(pStandard->_video_timing, centi_rate);

    uint8_t color_space_divider = 1;
    uint8_t color_depth_multiplexer = 0;

    if (color_depth == intel_hdmi_color_depth_bpc_8)
    {
        color_depth_multiplexer = 8;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_10)
    {
        color_depth_multiplexer = 10;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_12)
    {
        color_depth_multiplexer = 12;
    }
    else if (color_depth == intel_hdmi_color_depth_bpc_16)
    {
        color_depth_multiplexer = 16;
    }

    *pPixelFreq = (uint32_t)(((float)pixel_clock * (float)color_depth_multiplexer) / ((float)color_space_divider * 8.0));
    if (*pPixelFreq <= HDMI14_MAX_TMDS_FREQ)
    {
        *pTmdsRatio = intel_hdmi_tmds_ratio_one_over_10;
    }
    else
    {
        *pTmdsRatio = intel_hdmi_tmds_ratio_one_over_40;
    }
}

/**
 * @brief Retrieves the video standard based on the VIC code.
 *
 * This function retrieves the video standard configuration based on the specified VIC code, color space, color depth, and full range.
 *
 * @param[in] vic_code The VIC code for the video standard.
 * @param[out] pStandard Pointer to the video standard structure to be updated.
 * @param[in] color_space The color space of the video standard.
 * @param[in] color_depth The color depth of the video standard.
 * @param[in] full_range The full range flag for the video standard.
 * @return True if the video standard is successfully retrieved, false otherwise.
 */
bool get_video_standard(eVideoStandards video_enum, video_standard_t *pStandard,
                        const intel_hdmi_color_space_t color_space, const intel_hdmi_color_depth_t color_depth, const uint8_t full_range)
{
    bool bResult = false;
    if (pStandard == NULL)
    {
        return false;
    }

    // Clear the video standard
    memset(pStandard, 0, sizeof(video_standard_t));

    for (uint8_t s=0; s<kCVO_MAX_MODE; s++) {
      if (video_enum == _gvideo_standards[s]._video_enum) {
        bResult = true;
        pStandard->_video_timing = &_gvideo_standards[s];
      }
    }

    if (bResult == true)
    {
        pStandard->_details._vic = pStandard->_video_timing->_vic;
        pStandard->_details._image_config.active_x = pStandard->_video_timing->_cvo_params.sample_count;
        pStandard->_details._image_config.active_y1 = pStandard->_video_timing->_cvo_params.f0_line_count;
        pStandard->_details._image_config.active_y2 = pStandard->_video_timing->_cvo_params.f1_line_count;
        pStandard->_details._image_config.active_y = pStandard->_details._image_config.active_y1 + pStandard->_details._image_config.active_y2;
        pStandard->_details._image_config.full_x = pStandard->_video_timing->_cvo_params.sample_count + pStandard->_video_timing->_cvo_params.h_blanking;
        pStandard->_details._image_config.full_y = (pStandard->_video_timing->_cvo_params.f0_line_count + pStandard->_video_timing->_cvo_params.f1_line_count) +
                                                   (pStandard->_video_timing->_cvo_params.v_blanking +
                                                    pStandard->_video_timing->_cvo_params.f0_v_blanking);
                                                    
        pStandard->_details._image_config.progressive = pStandard->_video_timing->_cvo_params.interlaced ? 0 : 1;
        pStandard->_details._image_config.color_depth = color_depth;
        pStandard->_details._image_config.color_space = color_space;
        pStandard->_details._image_config.full_range = full_range;
        pStandard->_details._image_config.centi_rate = (uint32_t)(pStandard->_video_timing->_frame_rate * 100);
        pStandard->_details._image_config.is_valid = true;

        // Create the avi packet
        uint8_t S01, B01, Q01, R0123, M01, Y;
        S01 = 0x2;   // Scan Type (Over/Underscan)
        B01 = 0x3;   // Both LR and TB bars are defined.
        Q01 = 0x1;   // Quantization (Full,Limited)
        R0123 = 0x8; // Active Aspect Ratio
        M01 = 0x2;   // Picture Aspect Ratio 4:3, 16:9 etc

        if( color_space == intel_hdmi_color_space_yuv444 )
        {
            Y = 0x2;
        }
        else if ( color_space == intel_hdmi_color_space_yuv422 )
        {
            Y = 0x1;
        }
        else
        {
            Y = 0;
        }

        pStandard->_avi_pckt_data[1] = (B01 << 2) | (S01 << 0);
        pStandard->_avi_pckt_data[1] |= (Y << 5);
        pStandard->_avi_pckt_data[2] = (M01 << 4) | (R0123 << 0);
        pStandard->_avi_pckt_data[3] = (Q01 << 2);
        pStandard->_avi_pckt_data[4] = pStandard->_video_timing->_vic;
        pStandard->_avi_pckt_data[5] = 0x00;
        pStandard->_avi_pckt_data[6] = 0x00; // Bar Top LSB
        pStandard->_avi_pckt_data[7] = 0x00; // Bar Top MSB
        pStandard->_avi_pckt_data[8] = ((pStandard->_details._image_config.active_y + 1) >> 0) & 0xFF;
        pStandard->_avi_pckt_data[9] = ((pStandard->_details._image_config.active_y + 1) >> 8) & 0xFF;
        pStandard->_avi_pckt_data[10] = 0x00; // Bar Left LSB
        pStandard->_avi_pckt_data[11] = 0x00; // Bar Left LSB
        pStandard->_avi_pckt_data[12] = ((pStandard->_details._image_config.active_x + 1) >> 0) & 0xFF;
        pStandard->_avi_pckt_data[13] = ((pStandard->_details._image_config.active_x + 1) >> 8) & 0xFF;
        pStandard->_avi_pckt_data[14] = 0x00; // RSVS

        uint8_t chksum = 0x82 + 0x02 + 0x0D; // AVI Infoframe Fixed Header.
        for (uint32_t x = 1; x < 15; x++)
        {
            chksum += pStandard->_avi_pckt_data[x];
        }
        pStandard->_avi_pckt_data[0] = 256 - (chksum & 0xFF);
    }
    return bResult;
}

/**
 * @brief Return the string representation of the colorspace.
 *
 * This function converts from intel_hdmi_color_space_t to a string.
 *
 * @param[in] color_space The colorspace.
 * @return String representation of the colorspace.
 */
const char* colorspaceStr(intel_hdmi_color_space_t color_space)
{
    switch (color_space)
    {
    case intel_hdmi_color_space_yuv420:
        return "YUV420";
        break;
    case intel_hdmi_color_space_yuv422:
        return "YUV422";
        break;
    case intel_hdmi_color_space_yuv444:
        return "YUV444";
        break;
    default:
        return "RGB";
        break;
    }
}

/**
 * @brief Return the string representation of the color depth.
 *
 * This function converts from intel_hdmi_color_depth_t to a string.
 *
 * @param[in] color_depth The color depth.
 * @return String representation of the color depth.
 */
const char* colordepthStr(intel_hdmi_color_depth_t color_depth)
{
    switch (color_depth)
    {
    case intel_hdmi_color_depth_bpc_16:
        return "16-Bit";
        break;
    case intel_hdmi_color_depth_bpc_12:
        return "12-Bit";
        break;
    case intel_hdmi_color_depth_bpc_10:
        return "10-Bit";
        break;
    default:
        return "8-Bit";
        break;
    }
}

/**
 * @brief Prints the details of a video standard.
 *
 * This function logs the details of the specified video standard.
 *
 * @param[in] pStandard Pointer to the video standard to print.
 * @param[in] bDetailed Bool indicating that detail information should be displayed
 */
void print_video_standard(video_standard_t *pStandard, bool bDetailed)
{
    if (pStandard == NULL)
    {
        logPrint(kDebug, "\tInvalid Video Standard - Pointer NULL\n");
        return;
    }

    if (pStandard->_details._image_config.is_valid == false)
    {
        logPrint(kInfo, "\tInvalid Video Standard\n");
    }
    else if ( bDetailed )
    {
        const intel_hdmi_standard_t *pHdmiStandard = &(pStandard->_details._image_config);
        logPrint(kInfo, "\t Hactive = %4" PRIu32 "  Vactive = %4" PRIu32 "\n", pHdmiStandard->active_x, pHdmiStandard->active_y);
        logPrint(kInfo, "\t Htotal  = %4" PRIu32 "  Vtotal  = %4" PRIu32 "\n", pHdmiStandard->full_x, pHdmiStandard->full_y);
        logPrint(kInfo, "\t Rate    = %3.2f %s %s %s\n", (float)(pHdmiStandard->centi_rate) / 100.0, pHdmiStandard->progressive ? "Progressive" : "Interlaced",
                            colorspaceStr(pHdmiStandard->color_space), colordepthStr(pHdmiStandard->color_depth));
        if( pStandard->_video_timing )
        {
            logPrint(kInfo, "\t PixClk  = %d\n", get_pixel_clock(pStandard->_video_timing, pStandard->_details._image_config.centi_rate));
        }
        logPrint(kInfo, "\t VIC     = %d\n", pStandard->_details._vic);

        if( pStandard->_video_timing )
        {
            const hdmi_tx_standard_t *pTiming = pStandard->_video_timing;
            const cvo_params_t *pCvo_params = &(pTiming->_cvo_params);

            logPrint(kInfo, "\tCV Timing Details:\n");
            logPrint(kInfo, "\t   Hactive  = %4" PRIu32 "  Vactive     = %4" PRIu32 "\n", pCvo_params->sample_count, 
                pCvo_params->f0_line_count + pCvo_params->f1_line_count);
            logPrint(kInfo, "\t   Htotal   = %4" PRIu32 "  Vtotal      = %4" PRIu32 "\n", pCvo_params->sample_count + pCvo_params->h_blanking,
                (pCvo_params->f0_line_count + pCvo_params->f1_line_count) + (pCvo_params->v_blanking + pCvo_params->f0_v_blanking));
            logPrint(kInfo, "\t   Rate     = %3.2f %s\n", pTiming->_frame_rate, pCvo_params->interlaced ? "Interlaced" : "Progressive");
            logPrint(kInfo, "\t   Htotal   = %4" PRIu32 "  Hblank      = %4" PRIu32 "  Hwidth     = %4" PRIu32"\n", 
                pCvo_params->sample_count + pCvo_params->h_blanking, pCvo_params->h_blanking, pCvo_params->h_sync_length);
            logPrint(kInfo, "\t   HFporch  = %4" PRIu32 "  HBporch     = %4" PRIu32"\n", pCvo_params->h_front_porch, pCvo_params->h_blanking - pCvo_params->h_front_porch - pCvo_params->h_sync_length);

            if( pCvo_params->interlaced )
            {
                logPrint(kInfo, "\t   F0lines  = %4" PRIu32 "  F0blank     = %4" PRIu32 "  F0width    = %4" PRIu32"\n", 
                    pCvo_params->f0_line_count, pCvo_params->f0_v_blanking, pCvo_params->f0_v_sync_length);
                logPrint(kInfo, "\t   F0Fporch = %4" PRIu32 "  F0Bporch    = %4" PRIu32"\n", 
                        pCvo_params->f0_v_front_porch, pCvo_params->f0_v_blanking - pCvo_params->f0_v_front_porch - pCvo_params->f0_v_sync_length);
                
                logPrint(kInfo, "\t   F1lines  = %4" PRIu32 "  Vblank      = %4" PRIu32 "  Vwidth     = %4" PRIu32"\n", 
                    pCvo_params->f1_line_count,  pCvo_params->v_blanking, pCvo_params->v_sync_length);
                logPrint(kInfo, "\t   VFporch  = %4" PRIu32 "  VBporch     = %4" PRIu32"\n", 
                    pCvo_params->v_front_porch, pCvo_params->v_blanking - pCvo_params->v_front_porch - pCvo_params->v_sync_length);
            }
            else
            {
                logPrint(kInfo, "\t   Vlines   = %4" PRIu32 "  Vblank      = %4" PRIu32 "  Vwidth     = %4" PRIu32"\n", 
                    pCvo_params->f0_line_count,  pCvo_params->v_blanking, pCvo_params->v_sync_length);
                logPrint(kInfo, "\t   VFporch  = %4" PRIu32 "  VBporch     = %4" PRIu32"\n", 
                    pCvo_params->v_front_porch, pCvo_params->v_blanking - pCvo_params->v_front_porch - pCvo_params->v_sync_length);
            }    
            logPrint(kInfo, "\t   HPol     = %4d  VPol        = %4d\n", pCvo_params->h_sync_polarity, pCvo_params->v_sync_polarity);
            logPrint(kInfo, "\t   F0rising = %4" PRIu32 "  fieldRising = %4" PRIu32 "  fieldFall = %4" PRIu32"\n", 
                pCvo_params->f0_v_rising, pCvo_params->field_rising, pCvo_params->field_falling);
            logPrint(kInfo, "\t   Active Picture Line    = %4" PRIu32 "\n", pCvo_params->active_picture_line);
        }
        logPrint(kInfo, "\t AVI Infoframe:\n");
        logPrint(kInfo, "\t   ");
        for ( uint32_t i=0; i<AVI_PACKET_SIZE; i++ )
        {
            logPrint(kInfo|kSuppress, "0x%02x", pStandard->_avi_pckt_data[i]);
            if( (i!=0) && ((i%8)==0) )
            {
                logPrint(kInfo|kSuppress, "\n");
                logPrint(kInfo, "\t   ");
            }
            else if( (i == (AVI_PACKET_SIZE-1))  )
            {
                logPrint(kInfo|kSuppress, "\n");
            }
            else
            {
                logPrint(kInfo|kSuppress, ", ");
            }
        }
    }
    else {
        const intel_hdmi_standard_t *pHdmiStandard = &(pStandard->_details._image_config);
        logPrint(kInfo, "\t%" PRIu32 "x%" PRIu32 " (%" PRIu32 "x%" PRIu32 "%s%" PRIu32 ".%02" PRIu32 "",
                 pHdmiStandard->active_x, pHdmiStandard->active_y, pHdmiStandard->full_x, pHdmiStandard->full_y, pHdmiStandard->progressive ? "p" : "i", pHdmiStandard->centi_rate / 100, pHdmiStandard->centi_rate % 100);
        logPrint(kInfo | kSuppress, " %s", colorspaceStr(pHdmiStandard->color_space));
        logPrint(kInfo | kSuppress, " %s)\n", colordepthStr(pHdmiStandard->color_depth));
        if (pStandard->_video_timing)
        {
            const hdmi_tx_standard_t *pTiming = pStandard->_video_timing;
            logPrint(kInfo, "\tTiming Details:\n");
            logPrint(kInfo, "\t  %" PRIu32 "x%" PRIu32 " (%" PRIu32 "x%" PRIu32 "%s%2.2f)\n",
                    pTiming->_cvo_params.sample_count,
                    pTiming->_cvo_params.f0_line_count + pTiming->_cvo_params.f1_line_count,
                    pTiming->_cvo_params.sample_count + pTiming->_cvo_params.h_blanking,
                    (pTiming->_cvo_params.f0_line_count + pTiming->_cvo_params.f1_line_count) +
                        (pTiming->_cvo_params.v_blanking + pTiming->_cvo_params.f0_v_blanking),
                    pTiming->_cvo_params.interlaced ? "i" : "p", pTiming->_frame_rate);
        }
    }
}

#define EQUAL(var) (a->var == b->var)

/**
 * @brief Compares two image details structures.
 *
 * This function compares two image details structures for equality, ignoring the centi-rate.
 *
 * @param[in] a Pointer to the first image details structure.
 * @param[in] b Pointer to the second image details structure.
 * @return True if the structures are equal, false otherwise.
 */
bool compare_image_details(const image_details_t *a, const image_details_t *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return false;
    }
    // Compare for a change, but ignore centirate for now
    return EQUAL(_image_config.is_valid) && EQUAL(_image_config.progressive) &&
           EQUAL(_image_config.active_x) && EQUAL(_image_config.active_y) && EQUAL(_image_config.active_y1) && EQUAL(_image_config.active_y2) &&
           EQUAL(_image_config.full_x) && EQUAL(_image_config.full_y) && EQUAL(_image_config.color_depth) && EQUAL(_image_config.color_space) && EQUAL(_image_config.full_range);
}