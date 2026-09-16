#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "config.h"
#include "utils.h"
#include "board.h"

// Definition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Rx", format, ##__VA_ARGS__))

#if HDMI_RX_SUPPORTED

// Function to convert frl rate to a string. Implementation is in main.c
extern const char* frl_rate_to_string(uint32_t frl_rate);

#include "rx_utils.h"
#include "edid.h"
#include "intel_hdmi21_rx_regs.h"
#include "intel_hdmi_common_return_types.h"

#define Is60(centi_rate) ((centi_rate > 5900) && (centi_rate < 6100)) /**< Is rate 59.94 or 60. */
#define Is50(centi_rate) ((centi_rate > 4900) && (centi_rate < 5100)) /**< Is rate 50. */
#define Is30(centi_rate) ((centi_rate > 2900) && (centi_rate < 3100)) /**< Is rate 29.97 or 30. */
#define Is25(centi_rate) ((centi_rate > 2450) && (centi_rate < 2550)) /**< Is rate 25. */
#define Is24(centi_rate) ((centi_rate > 2300) && (centi_rate < 2450)) /**< Is rate 23.97 or 24. */

//==================================================================

/**
 * @brief Initializes the HDMI RX context.
 *
 * This function initializes the HDMI RX context with the specified base addresses and sets the initial state.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] base Base address for the HDMI RX.
 * @param[in] phy_base Base address for the HDMI RX PHY.
 * @return True if the context is successfully initialized, false otherwise.
 */
bool rxutils_init_context(HdmiRxContext *pContext, intel_hdmi_rx_base_t base, intel_hdmi21_rx_phy_base_t phy_base)
{
    if (pContext == NULL)
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return false;
    }
    memset(pContext, 0, sizeof(HdmiRxContext));
    intel_hdmi21_rx_phy_init(&(pContext->_phy_instance), phy_base);
    intel_hdmi21_rx_init(&(pContext->_instance), base, &(pContext->_phy_instance));

    // De-assert the hpd
    rxutils_assert_hpd(pContext, false);

    // Make sure that the counters are in the correct state
    rxutils_reset_counters(pContext);

    // Let's enable dynamic timing
    pContext->_bUseDynamicTimings = true;

    // Start-up as Idle
    pContext->_state = kHdmiRx_Idle;

#if HDMI_PASS_THROUGH
    pContext->_bTxEdidFollow = true; /**< Follow the TX EDID */
#else
    pContext->_bTxEdidFollow = false; /**< Do not follow the TX EDID, as TX not available */
#endif //HDMI_PASSTHRU

    // Configure for the MAX FRL Rate
    pContext->_frl_rate = RX_MAX_FRL_RATE;
    
    return true;
}

//==================================================================

/**
 * @brief Retrieves the video standard from the HDMI RX context.
 *
 * This function determines the video standard based on the HDMI RX context and updates the provided video standard structure.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[out] pStandard Pointer to the video standard structure to be updated.
 * @param[in] bValidForTx Boolean indicating if the standard is valid for transmission.
 * @return True if the video standard is valid, false otherwise.
 */
bool rxutils_get_video_standard(HdmiRxContext *pContext, video_standard_t *pStandard, bool bValidForTx)
{
    video_standard_t new_standard;

    if ((pContext == NULL) || (pStandard == NULL))
    {
        logPrint(kError, "Invalid context pointers\n");
        return false;
    }
    intel_hdmi21_rx_instance_t *instance = &(pContext->_instance);

    intel_hdmi21_rx_status_t rx_status;
    uint8_t rx_5v;
    bool bCviLocked;
    rxutils_read_status(pContext, &rx_5v, &rx_status, &bCviLocked);
    
    intel_hdmi21_rx_get_color_information(instance, &(new_standard._details._image_config.color_depth), &(new_standard._details._image_config.color_space), &(new_standard._details._image_config.full_range));
    new_standard._details._vic = intel_hdmi21_rx_core_get_vic(instance);
    intel_hdmi21_rx_core_get_standard(instance, &(new_standard._details._image_config));

    if ( (!new_standard._details._image_config.is_valid) ||
        !bCviLocked || !rx_status.rx_locked || !rx_status.video_locked )
    {
        // Invalid standard so mark the standard as invalid
        pStandard->_details._image_config.is_valid = false;
    }
    else
    {
        intel_hdmi21_rx_avi_pckt_data(instance, pStandard->_avi_pckt_data);
        pStandard->_details = new_standard._details;
        // Clear the video image information
        pStandard->_video_timing = NULL;

        if (bValidForTx)
        {
            if (pStandard->_details._image_config.progressive == 0)
            {
                if (pStandard->_details._image_config.active_y == 1080)
                {
                    pStandard->_video_timing = &_gvideo_standards[kCVO_1920x1080I60_MODE];
                }
            }
            else if (new_standard._details._image_config.color_space == 3) // 'b11 = 4:2:0 ; 'b01 = 4:2:2
            {
                switch (pStandard->_details._image_config.active_y)
                {
                case 480:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_480P_420_MODE];
                    break;
                case 720:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_720P_420_MODE];
                    break;
                case 1080:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_1080P_420_MODE];
                    break;
                case 2160:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_2160P_420_MODE];
                    break;
                case 4320:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_4320P_420_MODE];
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch (pStandard->_details._image_config.active_y)
                {
                case 4320:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_4320P60_MODE];
                    break;
                case 2160:
                    if ((pStandard->_details._vic == 97) || Is60(pStandard->_details._image_config.centi_rate) || Is30(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_3840x2160P60_MODE];
                    }
                    else if ((pStandard->_details._vic == 96) || Is50(pStandard->_details._image_config.centi_rate) || Is25(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_3840x2160P50_MODE];
                    }
                    else if (Is24(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_3840x2160P24_MODE];
                    }
                    break;
                case 1080:
                    if ((pStandard->_details._vic == 16) || Is60(pStandard->_details._image_config.centi_rate) || Is30(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_1920x1080P60_MODE];
                    }
                    else if (Is50(pStandard->_details._image_config.centi_rate) || Is25(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_1920x1080P50_MODE];
                    } // vic = 31 (assumed)
                    else if (Is24(pStandard->_details._image_config.centi_rate))
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_1920x1080P24_MODE];
                    }
                    break;
                case 768:
                    pStandard->_video_timing = &_gvideo_standards[kCVO_1024x768P60_MODE];
                    break;
                case 720:
                    if (pStandard->_details._vic == 4)
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_1280x720P60_MODE];
                    }
                    else
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_1280x720P50_MODE];
                    } // vic = 19 (assumed)
                    break;
                case 600:
                    break;
                case 480:
                    if (pStandard->_details._vic == 2)
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_720x480P60_MODE];
                    }
                    else
                    {
                        pStandard->_video_timing = &_gvideo_standards[kCVO_640x480P60_MODE];
                    } // vic = 1 (assumed)
                    break;
                default:
                    break;
                }
            }
            if (pContext->_bUseDynamicTimings)
            {
                logPrint(kInfo, "Building CVI timings\n");
                // We'll make you the cv timing from the input data
                intel_hdmi_timings_t timings;
                if (intel_hdmi21_rx_get_timings(&(pContext->_instance), &timings) != intel_hdmi_success)
                {
                    logPrint(kError, "Failed to read rx timings.\n");
                    return pStandard->_details._image_config.is_valid;
                }

                pContext->_video_timing._frame_rate = (float)pStandard->_details._image_config.centi_rate / 100.0;
                pContext->_video_timing._cvo_params.sequential = false;
                pContext->_video_timing._cvo_params.interlaced = timings.interlaced ? true : false;
                pContext->_video_timing._cvo_params.sequential = false;

                // Horizontal Timing
                if (pStandard->_details._image_config.color_space == 3)
                {
                  pContext->_video_timing._cvo_params.sample_count = ((timings.h_active) / 2);
                  pContext->_video_timing._cvo_params.h_front_porch = ((timings.h_front) / 2);
                  pContext->_video_timing._cvo_params.h_sync_length = ((timings.h_sync) / 2);
                  pContext->_video_timing._cvo_params.h_blanking = ((timings.h_front + timings.h_sync + timings.h_back) / 2);
                } else {
                  pContext->_video_timing._cvo_params.sample_count = timings.h_active;
                  pContext->_video_timing._cvo_params.h_front_porch = timings.h_front;
                  pContext->_video_timing._cvo_params.h_sync_length = timings.h_sync;
                  pContext->_video_timing._cvo_params.h_blanking = timings.h_front + timings.h_sync + timings.h_back;
                }

                // Vertical Timing

                if (timings.interlaced)
                {
                    pContext->_video_timing._cvo_params.f0_line_count = timings.v_active;
                    pContext->_video_timing._cvo_params.f0_v_front_porch = timings.v_front_a;
                    pContext->_video_timing._cvo_params.f0_v_sync_length = timings.v_sync_a;
                    pContext->_video_timing._cvo_params.f0_v_blanking = timings.v_front_a + timings.v_sync_a + timings.v_back_a;
                    pContext->_video_timing._cvo_params.f0_ancillary_line = pContext->_video_timing._cvo_params.active_picture_line +
                                                                            pContext->_video_timing._cvo_params.f0_v_blanking -
                                                                            pContext->_video_timing._cvo_params.f0_v_front_porch + 1;

                    pContext->_video_timing._cvo_params.active_picture_line = pContext->_video_timing._cvo_params.f0_v_blanking -
                                                                              pContext->_video_timing._cvo_params.f0_v_front_porch + 1;

                    pContext->_video_timing._cvo_params.f1_line_count = pContext->_video_timing._cvo_params.f0_line_count;
                    pContext->_video_timing._cvo_params.v_front_porch = timings.v_front_b;
                    pContext->_video_timing._cvo_params.v_sync_length = timings.v_sync_b;
                    pContext->_video_timing._cvo_params.v_blanking = timings.v_front_b + timings.v_sync_b + timings.v_back_b;

                    pContext->_video_timing._cvo_params.f0_v_rising = pContext->_video_timing._cvo_params.active_picture_line +
                                                                      pContext->_video_timing._cvo_params.f0_line_count;
                    pContext->_video_timing._cvo_params.field_rising = pContext->_video_timing._cvo_params.f0_v_rising + timings.v_front_b;
                    pContext->_video_timing._cvo_params.field_falling = 1;
                    pContext->_video_timing._cvo_params.ancillary_line = 10;
                    pContext->_video_timing._cvo_params.f0_ancillary_line = 562;

                    pContext->_video_timing._cvo_params.active_picture_line += 0;
                }
                else
                {
                    pContext->_video_timing._cvo_params.f0_line_count = timings.v_active;
                    pContext->_video_timing._cvo_params.v_front_porch = timings.v_front_a;
                    pContext->_video_timing._cvo_params.v_sync_length = timings.v_sync_a;
                    pContext->_video_timing._cvo_params.v_blanking = timings.v_front_a + timings.v_sync_a + timings.v_back_a;
                    pContext->_video_timing._cvo_params.active_picture_line = timings.v_sync_a + timings.v_back_a + 1;
                    pContext->_video_timing._cvo_params.ancillary_line = 0; // TODO: What should this be?

                    pContext->_video_timing._cvo_params.f1_line_count = 0;
                    pContext->_video_timing._cvo_params.f0_v_front_porch = 0;
                    pContext->_video_timing._cvo_params.f0_v_sync_length = 0;
                    pContext->_video_timing._cvo_params.f0_v_blanking = 0;
                    pContext->_video_timing._cvo_params.f0_ancillary_line = 0;
                    pContext->_video_timing._cvo_params.f0_v_rising = 0;
                    pContext->_video_timing._cvo_params.field_rising = 0;
                    pContext->_video_timing._cvo_params.field_falling = 0;
                }

                pContext->_video_timing._cvo_params.h_sync_polarity = timings.h_polarity;
                pContext->_video_timing._cvo_params.v_sync_polarity = timings.v_polarity;
                pStandard->_video_timing = &pContext->_video_timing;
            }
            if (pStandard->_video_timing == 0)
            {
                logPrint(kInfo, "(new image config not supported in this application)\n");
            }
        }
    }
    return pStandard->_details._image_config.is_valid;
}

//==================================================================

/**
 * @brief Reconfigures the HDMI RX context with a specified FRL rate.
 *
 * This function updates the EDID data based on the specified FRL rate and configures the HDMI RX context accordingly.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] frl_rate The FRL rate to configure.
 * @param[in] bForce Boolean flag to force the update.
 */
void rxutils_reconfig(HdmiRxContext *pContext, uint8_t frl_rate, bool bForce)
{
    if (pContext == NULL)
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }
    uint8_t edid_data[NUM_EDID_PAGES * EDID_PAGE_SIZE];

    switch (frl_rate)
    {
    case 0:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[1], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 1:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[2], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 2:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[3], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 3:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[4], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 4:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[5], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 5:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[6], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    case 6:
        logPrint(kInfo, "Configure RX EDID Max FRL capability to %s\n", frl_rate_to_string(frl_rate));
        memcpy(edid_data, fixed_edid[0], EDID_PAGE_SIZE);
        memcpy(edid_data + 128, fixed_edid[7], EDID_PAGE_SIZE);
        rxutils_set_edid(pContext, edid_data, sizeof(edid_data), bForce);
        break;

    default:
        logPrint(kWarning, "Invalid FRL type - %d\n", frl_rate);
        break;
    }
}

//==================================================================

/**
 * @brief Reads the status of the HDMI RX context.
 *
 * This function retrieves various status indicators from the HDMI RX context, including 5V status, RX status, and CVI lock status.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[out] pRx_5v Pointer to store the 5V status.
 * @param[out] pRxStatus Pointer to store the RX status.
 * @param[out] pCviLocked Pointer to store the CVI lock status.
 */
void rxutils_read_status(HdmiRxContext *pContext, uint8_t *pRx_5v, intel_hdmi21_rx_status_t *pRxStatus, bool *pCviLocked)
{
    if (pContext == NULL)
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    if (pRx_5v)
    {
        *pRx_5v = intel_hdmi21_rx_phy_get_5v_status(&(pContext->_phy_instance));
    }
    if (pRxStatus)
    {
        intel_hdmi21_rx_get_status(&(pContext->_instance), pRxStatus);
    }
    if (pCviLocked)
    {
        *pCviLocked = intel_hdmi21_rx_get_cvi_locked(&(pContext->_instance));
    }
}

#define RX_LOCKED_COUNT (1000)

/**
 * @brief Gets the current status of the HDMI RX context.
 *
 * This function checks the current status of the HDMI RX context and updates its state accordingly.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 */
void rxutils_get_status(HdmiRxContext *pContext)
{
    intel_hdmi21_rx_status_t rx_status;
    uint8_t rx_5v;
    bool bCviLocked;

    if (pContext == NULL)
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    // read the current rx status
    rxutils_read_status(pContext, &rx_5v, &rx_status, &bCviLocked);

#if (BOARD_CONNECTOR == BOARD_CONNECTOR_ONBOARD && BOARD_NAME == BOARD_A5E_MDK)
    tmds1204_monitor(rx_5v, rx_status.tmds_ratio);
#endif
    pContext->_sticky_5v &= rx_5v;
    pContext->_sticky_video_locked &= rx_status.video_locked;
    pContext->_sticky_rx_locked &= rx_status.rx_locked;
    pContext->_sticky_bCviLocked &= bCviLocked;

    if (rx_5v && ((rx_status.video_locked == 1) && (rx_status.rx_locked == 1)) && bCviLocked)
    {
        // We've must have a new standard
        if (pContext->_state == kHdmiRx_Idle)
        {
// #ifdef LOOPBACK_MODE == 1
            // pContext->_frl_rate = rx_status.frl_rate;
// #endif
            pContext->_state = kHdmiRx_NewStandard;
            pContext->_change_count++;
            logPrint(kInfo, "Restarting the CVI\n");
            intel_hdmi21_rx_stop_cvi(&(pContext->_instance));
            usleep(100 * 1000);
            intel_hdmi21_rx_restart_cvi(&(pContext->_instance));        
            logPrint(kInfo, "Detected New Standard [change %d]\n", pContext->_change_count);            
        }
    }
    else
    {
        if (pContext->_state != kHdmiRx_Idle)
        {
            pContext->_change_count++;
            logPrint(kWarning, "Lost Lock : 5v = %d, rx = %d, vid = %d, cvi = %d [change %d]\n",
                     rx_5v, rx_status.rx_locked, rx_status.video_locked, bCviLocked, pContext->_change_count);
            rxutils_enable(pContext, false);
            // Allow system to settle
            usleep(500*1000);
        }
        pContext->_state = kHdmiRx_Idle;
    }
}

//==================================================================

/**
 * @brief Enables or disables the HDMI RX CVI.
 *
 * This function controls the HDMI RX CVI based on the specified enable flag.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] bEnable Boolean flag to enable or disable the RX CVI.
 */
void rxutils_enable(HdmiRxContext *pContext, const bool bEnable)
{
    if (pContext == NULL)
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    if (bEnable)
    {
        logPrint(kInfo, "Enable RX CVI\n");
        intel_hdmi21_rx_start_cvi(&(pContext->_instance));
    }
    else
    {
        logPrint(kInfo, "Disable RX CVI\n");
        intel_hdmi21_rx_stop_cvi(&(pContext->_instance));
    }
}

//==================================================================

/**
 * @brief Sets the EDID data for the HDMI RX context.
 *
 * This function updates the EDID data in the HDMI RX context, optionally forcing the update.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] edid_data Pointer to the EDID data.
 * @param[in] edid_data_size Size of the EDID data.
 * @param[in] bForce Boolean flag to force the update.
 */
void rxutils_set_edid(HdmiRxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size, bool bForce)
{
    if ((pContext == NULL))
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    // Check that the edid is smaller that expected
    if (edid_data_size > sizeof(pContext->_current_edid))
    {
        logPrint(kError, "Unexpected edid size %d [expected %d]\n", edid_data_size, sizeof(pContext->_current_edid));
        return;
    }

    // Only send if this is a new edid as will thrash the rx otherwise
    if ((memcmp(edid_data, pContext->_current_edid, edid_data_size) != 0) || bForce)
    {
        logPrint(kInfo, "Set New Rx EDID [bForce %d]\n", bForce);
        intel_hdmi21_rx_set_edid_ram_data(&(pContext->_instance), edid_data, edid_data_size);
        memcpy(pContext->_current_edid, edid_data, edid_data_size);
    }
    else
    {
        logPrint(kDebug, "Not sending EDID as the same.\n");
    }
}

//==================================================================
/**
 * @brief Sets up the Edid depending on the request mode of edid passthru or specific FRL rate.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] edid_data Pointer to the EDID data.
 * @param[in] edid_data_size Size of the EDID data.
 * @param[in] bForce Boolean flag to force the update.
 */
void rxutils_update_edid(HdmiRxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size, bool bForce)
{
    if ((pContext == NULL))
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    if (pContext->_bTxEdidFollow )
    {
        logPrint(kInfo, "Update Rx EDID from Tx EDID [bForce %d]\n", bForce);
        rxutils_set_edid(pContext, edid_data, edid_data_size, bForce);
    }
    else
    {
        // Update the EDID based on the FRL rate
        rxutils_reconfig(pContext, pContext->_frl_rate, bForce);
    }
}

//==================================================================

/**
 * @brief Asserts or de-asserts the HDMI RX HPD.
 *
 * This function controls the HDMI RX HPD based on the specified enable flag.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] bEnable Boolean flag to assert or de-assert the HPD.
 */
void rxutils_assert_hpd(HdmiRxContext *pContext, const bool bEnable)
{
    if ((pContext == NULL))
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }
    if (bEnable)
    {
        logPrint(kInfo, "Assert HPD\n");
        intel_hdmi21_rx_hotplug(&(pContext->_instance), INTEL_HDMI_RX_HOTPLUG_DETECT_BIT);
    }
    else
    {
        logPrint(kInfo, "De-assert HPD\n");
        intel_hdmi21_rx_hotplug(&(pContext->_instance), 0);
    }
}

//==================================================================

/**
 * @brief Converts HDMI RX state to a string representation.
 *
 * This function returns a string representation of the given HDMI RX state.
 *
 * @param[in] state The HDMI RX state to convert.
 * @return A string representation of the state.
 */
const char *rxutils_state_str(eHdmiRxState state)
{
    switch (state)
    {
    case kHdmiRx_Idle:
        return "Idle";
        break;
    case kHdmiRx_Enabled:
        return "Enabled";
        break;
    case kHdmiRx_NewStandard:
        return "New Standard";
        break;

    default:
        return "Unknown";
        break;
    }
}

//==================================================================

/**
 * @brief Display HDMI Rx video timings.
 *
 * @param[in] state The HDMI RX state to convert.
 */
void rxutils_display_timings(HdmiRxContext *pContext)
{
    if ((pContext == NULL))
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }

    intel_hdmi_timings_t timings;
    if (intel_hdmi21_rx_get_timings(&(pContext->_instance), &timings) != intel_hdmi_success)
    {
        logPrint(kError, "Failed to read rx timings.\n");
        return;
    }

    intel_hdmi_standard_t image_config;
    if (intel_hdmi21_rx_core_get_standard(&(pContext->_instance), &(image_config)) != intel_hdmi_success)
    {
        logPrint(kError, "Failed to read rx core timings.\n");
        return;
    }

    logPrint(kInfo, "\t   Events    =");
    bool bLost = false;
    if (pContext->_sticky_5v == 0)
    {
        if( bLost )
        {
            logPrint(kInfo|kSuppress, ",");
        }
        logPrint(kInfo|kSuppress, " 5v");
        bLost = true;
    }
    if( pContext->_sticky_video_locked == 0 )
    {
        if( bLost )
        {
            logPrint(kInfo|kSuppress, ",");
        }
        logPrint(kInfo|kSuppress, " vid_lock lost");
        bLost = true;
    }
    if( pContext->_sticky_rx_locked == 0 )
    {
        if( bLost )
        {
            logPrint(kInfo|kSuppress, ",");
        }
        logPrint(kInfo|kSuppress, " rx_lock lost");
        bLost = true;
    }
    if( pContext->_sticky_bCviLocked == 0 )
    {
        if( bLost )
        {
            logPrint(kInfo|kSuppress, ",");
        }
        logPrint(kInfo|kSuppress, " cvi_lock lost");
        bLost = true;
    }
    if( bLost )
    {
        logPrint(kInfo|kSuppress, "\n");
    }
    else
    {
        logPrint(kInfo|kSuppress, " none\n");
    }
    
    if( image_config.is_valid == 0 )
    {
        logPrint(kInfo, "\tInvalid Video Timing\n");
    }
    else
    {
        uint32_t h_blank = timings.h_front + timings.h_sync + timings.h_back;
        uint32_t v_blank_a = timings.v_front_a + timings.v_sync_a + timings.v_back_a;
        uint32_t v_blank_b = timings.v_front_b + timings.v_sync_b + timings.v_back_b;
        uint32_t h_total = timings.h_active + h_blank;
        uint32_t v_total = timings.v_active + v_blank_a;
        uint32_t v_active = timings.v_active;
        if (timings.interlaced)
        {
            v_total += timings.v_active + v_blank_b;
            v_active *= 2;
        }

        logPrint(kInfo, "\t   Hactive   = %4" PRIu16 "  Vactive   = %4" PRIu16 "  Rate %2.2f %s\n", timings.h_active, v_active, (float)image_config.centi_rate / 100.0, timings.interlaced ? "Interlaced" : "Progressive");
        logPrint(kInfo, "\t   Htotal    = %4" PRIu32 "  Vtotal    = %4" PRIu32 "\n", h_total, v_total);
        logPrint(kInfo, "\t   Hblank    = %4" PRIu32 "  Hwidth    = %4" PRIu32 "\n", h_blank, timings.h_sync );
        logPrint(kInfo, "\t   HFporch   = %4" PRIu32 "  HBporch   = %4" PRIu32 "\n", timings.h_front, timings.h_back);
        if( timings.interlaced )
        {
            logPrint(kInfo, "\t   Vblank_a  = %4" PRIu32 "  Vwidth_a  = %4" PRIu32 "\n", v_blank_a, timings.v_sync_a);
            logPrint(kInfo, "\t   VFPorch_a = %4" PRIu32 "  VBPorch_a = %4" PRIu32 "\n", timings.v_front_a, timings.v_back_a);
            logPrint(kInfo, "\t   Vblank_b  = %4" PRIu32 "  Vwidth_b  = %4" PRIu32 "\n", v_blank_b, timings.v_sync_b);
            logPrint(kInfo, "\t   VFporch_b = %4" PRIu32 "  VFporch_b = %4" PRIu32 "\n", timings.v_front_b, timings.v_back_b);
        }
        else
        {
            logPrint(kInfo, "\t   Vblank    = %4" PRIu32 "  Vwidth    = %4" PRIu32 "\n", v_blank_a, timings.v_sync_a);
            logPrint(kInfo, "\t   VFPorch   = %4" PRIu32 "  VBPorch   = %4" PRIu32 "\n", timings.v_front_a, timings.v_back_a);
        }
        logPrint(kInfo, "\t   HPol      = %4d  VPol      = %4d\n", timings.h_polarity, timings.v_polarity);
        logPrint(kInfo, "\t   CVI Drop Count = %4d   CVI Overflow = %4d\n",
                intel_hdmi21_rx_cvi_get_picture_drop_count(&(pContext->_instance.cvi)),
                intel_hdmi21_rx_cvi_is_overflowed(&(pContext->_instance.cvi)));
    }
}

/**
 * @brief Reset status counters.
 *
 * @param[in] state The HDMI RX state to convert.
 */
void rxutils_reset_counters(HdmiRxContext *pContext)
{
    if ((pContext == NULL))
    {
        // Must have a valid instance pointer
        logPrint(kError, "Invalid context pointer\n");
        return;
    }
    pContext->_change_count = 0;
    // Mark the sticky bits as true, and will clear to indicate the bit was lost
    pContext->_sticky_5v = 1;
    pContext->_sticky_video_locked = 1;
    pContext->_sticky_rx_locked = 1;
    pContext->_sticky_bCviLocked = true;
}

#endif // HDMI_RX_SUPPORTED
