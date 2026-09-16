#include <inttypes.h>
#include <io.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "alt_types.h"

#include "config.h"
#include "system.h"
#include "intel_hdmi_common_return_types.h"
#include "intel_hdmi_common_i2c.h"
#include "priv/alt_busy_sleep.h"
#include "board.h"

#include "edid.h"
#include "video_standards.h"
#include "tpg_utils.h"
#include "tx_utils.h"
#include "rx_utils.h"

#include "utils.h"

#include "types.h"

// Definition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "App", format, ##__VA_ARGS__))

#if defined(HDMI_SUPPORT_HDCP1X) || defined(HDMI_SUPPORT_HDCP2X)
#if HDMI_SUPPORT_HDCP1X == 1 || HDMI_SUPPORT_HDCP2X == 1
#include "hdcp.h"
#define SUPPORT_HDCP
#endif
#endif

//==================================================================

/**
 * @brief Converts FRL rate value to string representation.
 *
 * This function converts an FRL rate value to its string representation.
 * 
 * @param[in] frl_rate The FRL rate value to convert.
 * @return String representation of the FRL rate.
 *         - 0 returns "tmds"
 *         - 1-6 returns the FRL
 *         - Any other value returns "Unknown (number)"
 */
const char* frl_rate_to_string(uint32_t frl_rate)
{
    static char unknown_buffer[32];
    
    switch (frl_rate)
    {
        case 0:
            return "TMDS";
        case 1:
            return "FRL1 3x3Gbps";
        case 2:
            return "FRL2 3x6Gbps";
        case 3:
            return "FRL3 4x6Gbps";
        case 4:
            return "FRL4 4x8Gbps";
        case 5:
            return "FRL5 4x10Gbps";
        case 6:
            return "FRL6 4x12Gbps";
        default:
            snprintf(unknown_buffer, sizeof(unknown_buffer), "Unknown [%ld]", frl_rate);
            return unknown_buffer;
    }
}
//==================================================================

/**
 * @brief Displays the application banner.
 *
 * This function logs the application banner with version and configuration information.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool displayBanner(utils_context_t *pContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    UNUSED(pCode);

    if (pContext == NULL)
    {
        return false;
    }

    logPrint(kInfo, "******************************************************\n");
    logPrint(kInfo, "***         Intel Agilex AXI Design (HDMI 2.1)     ***\n");
    logPrint(kInfo, "*** %11s = %-8s %-8s             ***\n", "Build Date", __DATE__, __TIME__);
    logPrint(kInfo, "***                                                ***\n");
    logPrint(kInfo, "*** %11s = %-8s  %11s = %-8s ***\n", "Board", get_board_name(BOARD_NAME), "Conn", get_connector_name(BOARD_CONNECTOR));
    logPrint(kInfo, "*** %11s = %-8d  %11s = %-8d ***\n", "HDMI RX", HDMI_RX_SUPPORTED, "HDMI Tx", HDMI_TX_SUPPORTED);
    logPrint(kInfo, "*** %11s = %-8d                         ***\n", "FRL", HDMI_SUPPORT_FRL);
    logPrint(kInfo, "*** %11s = %-8d  %11s = %-8d ***\n", "FRL RX", RX_MAX_FRL_RATE, "FRL TX", TX_MAX_FRL_RATE);
    logPrint(kInfo, "*** %11s = %-8d  %11s = %-8d ***\n", "PIP", PIXEL_IN_PARALLEL, "Clks Same", CLOCKS_ARE_SAME);
    logPrint(kInfo, "*** %11s = %-8d  %11s = %-8d ***\n", "Loopback", LOOPBACK_MODE, "EDID Pages", NUM_EDID_PAGES);
    logPrint(kInfo, "*** %11s = %-8d  %11s = %-8d ***\n", "HDCP1X", HDMI_SUPPORT_HDCP1X, "HCDP2X", HDMI_SUPPORT_HDCP2X);
    logPrint(kInfo, "******************************************************\n");
    return true;
}

/**
 * @brief Displays help information for the menu.
 *
 * This function logs help information for the available menu items.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool DisplayHelp(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata);

#if HDMI_RX_SUPPORTED
//==================================================================

/**
 * @brief Select which Edid is presented to the Rx.  
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxSelectEdid(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;

    if( strcmp(pCode, "ef") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = Follow Tx\n");
        pContext->rx._bTxEdidFollow = true;
    }
    else if( strcmp(pCode, "et") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = TMDS\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 0; 
    }
    else if( strcmp(pCode, "e1") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL1 3x3Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 1;
    }
    else if( strcmp(pCode, "e2") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL2 3x6Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 2;
    }
    else if( strcmp(pCode, "e3") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL3 4x6Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 3;
    }
    else if( strcmp(pCode, "e4") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL4 4x8Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 4;
    }
    else if( strcmp(pCode, "e5") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL5 4x10Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 5;
    }
    else if( strcmp(pCode, "e6") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = FRL6 4x12Gbps\n");
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = 6;
    }
    else if( strcmp(pCode, "em") == 0 )
    {
        logPrint(kInfo, "Setting Rx Edid Mode = %s\n", frl_rate_to_string(RX_MAX_FRL_RATE));
        pContext->rx._bTxEdidFollow = false;
        pContext->rx._frl_rate = RX_MAX_FRL_RATE;
    }
    else
    {
        logPrint(kWarning, "Invalid FRL Rate %s\n", pCode);
        return false;
    }

    // Set the FRL rate in the Rx context
    rxutils_update_edid(&(pContext->rx), pContext->edid_data, sizeof(pContext->edid_data), true);

    return true;
}

struct MenuItem _rxEdidModeMenuItems[] = 
{
    {"", "Rx Edid Mode", NULL},
#if HDMI_PASS_THROUGH
    {"ef", "Edid Follow Tx", RxSelectEdid, NULL},
#endif // HDMI_PASS_THROUGH
    {"et", "TMDS", RxSelectEdid, NULL},
#if RX_MAX_FRL_RATE >= 1
    {"e1", "FRL1 3x3Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 1
#if RX_MAX_FRL_RATE >= 2
    {"e2", "FRL2 3x6Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 2
#if RX_MAX_FRL_RATE >= 3
    {"e3", "FRL3 4x6Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 3
#if RX_MAX_FRL_RATE >= 4
    {"e4", "FRL4 4x8Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 4
#if RX_MAX_FRL_RATE >= 5
    {"e5", "FRL5 4x10Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 5
#if RX_MAX_FRL_RATE >= 6
    {"e6", "FRL6 4x12Gbps", RxSelectEdid, NULL},
#endif // RX_MAX_FRL_RATE >= 6
    {"em", "Max FRL Rate", RxSelectEdid, NULL},
    {"", NULL, NULL}, // Terminate the list
};

//==================================================================

/**
 * @brief Controls the RX HPD based on the provided code.
 *
 * This function asserts or de-asserts the RX HPD based on the provided code.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxHpdControl(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }
    app_context_t *pContext = pUtlContext->pApp;
    if (strcmp(pCode, "p1") == 0)
    {
        rxutils_assert_hpd(&(pContext->rx), true);
    }
    else if (strcmp(pCode, "p0") == 0)
    {
        rxutils_assert_hpd(&(pContext->rx), false);
    }
    else if (strcmp(pCode, "pt") == 0)
    {
        rxutils_assert_hpd(&(pContext->rx), false);
        usleep(100*1000);
        rxutils_assert_hpd(&(pContext->rx), true);
    }
    return true;
}

//==================================================================

/**
 * @brief Displays the RX status.
 *
 * This function logs the current status and video standard of the RX context.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxDisplayStatus(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;

    logPrint(kInfo, "Rx Status   = %s  Change Count = %" PRIu32 "\n", rxutils_state_str(pContext->rx._state), pContext->rx._change_count);
    logPrint(kInfo, "\tEdid Mode = %s\n", pContext->rx._bTxEdidFollow ? "Follow Tx" : frl_rate_to_string(pContext->rx._frl_rate));
    intel_hdmi21_rx_status_t rx_status;
    uint8_t rx_5v;
    bool bCviLocked;
    rxutils_read_status(&(pContext->rx), &rx_5v, &rx_status, &bCviLocked);
    logPrint(kInfo, "\tRx 5v     = %d  CVI Locked   = %d\n", rx_5v, bCviLocked);
    logPrint(kInfo, "\tRx Locked = %d  Video Locked = %d\n", rx_status.rx_locked, rx_status.video_locked);
    video_standard_t rx_standard;
    rxutils_get_video_standard(&(pContext->rx), &rx_standard, true);
    print_video_standard(&rx_standard, true);

    return true;
}

//==================================================================

/**
 * @brief Displays the RX Timings.
 *
 * This function logs the current video timings of the RX context.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxDisplayTimings(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }
    app_context_t *pContext = pUtlContext->pApp;

    logPrint(kInfo, "CVI Video Timings : Change Count = %" PRIu32 "\n", pContext->rx._change_count);
    rxutils_display_timings(&(pContext->rx));
    return true;
}

//==================================================================

/**
 * @brief Debug: Restart CVI.
 *
 * This function is a debug function to restart the CVI.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
#include "intel_hdmi21_rx.h"
bool RxDebugCvi(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    logPrint(kInfo, "Restarting the CVI\n");
    intel_hdmi21_rx_stop_cvi(&(pContext->rx._instance));
    usleep(100 * 1000);
    intel_hdmi21_rx_restart_cvi(&(pContext->rx._instance));
    return true;
}

//==================================================================

/**
 * @brief Reset the RX Context change count.
 *
 * This function reset the input change count on the RX context.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxResetChangeCount(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    logPrint(kInfo, "Reseting RX Input change counter.\n");
    rxutils_reset_counters(&(pContext->rx));

    return true;
}

//==================================================================

/**
 * @brief Enable/Disable the dynamic timing.
 *
 * This function enables/disables the dynamic timing setting of the Rx.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool RxDynamicTiming(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    if( strcmp(pCode, "qd") == 0 )
    {
        pContext->rx._bUseDynamicTimings = false;
    }
    else
    {
        pContext->rx._bUseDynamicTimings = true;
    }

    // Toggle the hotplug
    rxutils_assert_hpd(&(pContext->rx), false);
    usleep(50 * 1000);
    rxutils_assert_hpd(&(pContext->rx), true);
    return true;
}

#endif // HDMI_RX_SUPPORTED

#if HDMI_TX_SUPPORTED
//==================================================================

/**
 * @brief Displays the TX status.
 *
 * This function logs the current status and video standard of the TX context.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool TxDisplayStatus(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    logPrint(kInfo, "Tx Status = %s   Rx Follow = %d FRL Rate = %s\n", txutils_state_str(pContext->tx._state), pContext->bFollow, frl_rate_to_string(txutils_current_frl_rate(&(pContext->tx))));
    print_video_standard(&(pContext->tx._current_video_standard), true);

    return true;
}

/**
 * @brief Displays the TX SCDC.
 *
 * This function displays the SCDC on the TX device.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool TxDisplaySCDC(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    txutils_display_scdc(&(pContext->tx));
    return true;
}


//==================================================================

/**
 * @brief Dump EDID.
 *
 * This function displays the current TX EDID.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool TxDumpEdid(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;
    uint8_t edid_data[NUM_EDID_PAGES * EDID_PAGE_SIZE];

    uint32_t edid_size = txutils_read_edid(&(pContext->tx), edid_data, sizeof(edid_data));
    if( edid_size )
    {
        logPrint(kInfo, "Edid Size = %" PRIu32 "\n", edid_size);
        // Limit the size to the size of the edid_data structure
        if( edid_size > sizeof(edid_data) )
        {
            edid_size = sizeof(edid_data);
        }
        for(int i=0; i<edid_size; i++)
        {
            if( i!=0 )
            {
                if( (i%8)==0 )
                {
                    logPrint(kInfo|kSuppress, "\n");
                }
                else
                {
                    logPrint(kInfo|kSuppress, ", ");
                }
            }
            logPrint(kInfo|kSuppress, "0x%02x", edid_data[i]);
        }
        logPrint(kInfo|kSuppress, "\n");
    }
    return true;
}

//==================================================================

/**
 * @brief Checks to see if 2 standards are compatible for pass thru.
 *
 * This function checks to see if 2 standards are compatible for pass thru.
 *
 * @param[in] pA Pointer to a video standard.
 * @param[in] pB Pointer to a video standard.
 * @return True if compatible.
 */
bool passthru_compatible(video_standard_t *pA, video_standard_t *pB)
{
    if( pA->_details._image_config.is_valid && pB->_details._image_config.is_valid &&
        (pA->_details._image_config.active_x == pB->_details._image_config.active_x) &&
        (pA->_details._image_config.active_y == pB->_details._image_config.active_y)
    )
    {
        return true;
    }
    return false;
}

//==================================================================
#if HDMI_TX_SUPPORTED
/**
 * @brief Select the Max FRL Rate for the TX.  
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool SelectTxFrlRate(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    app_context_t *pContext = pUtlContext->pApp;

    if( strcmp(pCode, "ft") == 0 )
    {
        pContext->tx._tx_frl_rate = 0;
    }
    else if( strcmp(pCode, "f1") == 0 )
    {
        pContext->tx._tx_frl_rate = 1;
    }
    else if( strcmp(pCode, "f2") == 0 )
    {
        pContext->tx._tx_frl_rate = 2;
    }
    else if( strcmp(pCode, "f3") == 0 )
    {
        pContext->tx._tx_frl_rate = 3;
    }
    else if( strcmp(pCode, "f4") == 0 )
    {
        pContext->tx._tx_frl_rate = 4;
    }
    else if( strcmp(pCode, "f5") == 0 )
    {
        pContext->tx._tx_frl_rate = 5;
    }
    else if( strcmp(pCode, "f6") == 0 )
    {
        pContext->tx._tx_frl_rate = 6;
    }
    else if( strcmp(pCode, "fm") == 0 )
    {
        pContext->tx._tx_frl_rate = TX_MAX_FRL_RATE;
    }
    else
    {
        logPrint(kWarning, "Invalid FRL Rate %s\n", pCode);
        return false;
    }

    logPrint(kInfo, "Setting Tx FRL Rate = %s\n", frl_rate_to_string(pContext->tx._tx_frl_rate));
    
    /* Force a new standard */
    if( pContext->tx._state != kHdmiTx_Idle )
    {
        pContext->tx._state = kHdmiTx_Available; // Mark as available to set the new output standard
    }    
    
    return true;
}

struct MenuItem _txFrlRateMenuItems[] = 
{
    {"", "Tx FRL Rate", NULL},
    {"ft", "TMDS", SelectTxFrlRate},
#if TX_MAX_FRL_RATE >= 1
    {"f1", "FRL1 3x3Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 1
#if TX_MAX_FRL_RATE >= 2
    {"f2", "FRL2 3x6Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 2
#if TX_MAX_FRL_RATE >= 3
    {"f3", "FRL3 4x6Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 3
#if TX_MAX_FRL_RATE >= 4
    {"f4", "FRL4 4x8Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 4
#if TX_MAX_FRL_RATE >= 5
    {"f5", "FRL5 4x10Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 5
#if TX_MAX_FRL_RATE >= 6
    {"f6", "FRL6 4x12Gbps", SelectTxFrlRate},
#endif  // TX_MAX_FRL_RATE >= 6
    {"fm", "Max FRL Rate", SelectTxFrlRate},
    {"", NULL, NULL}, // Terminate the list
};

#endif // HDMI_TX_SUPPORTED

#if TPG_0_BASE || (HDMI_PASS_THROUGH && (CLOCKS_ARE_SAME==0))
//==================================================================

/**
 * @brief Select the standard for the TX.
 *
 * This function logs the current status and video standard of the TX context.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool SelectTxStandard(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    logPrint(kInfo, "Select Standard = %s \n", pCode);
    app_context_t *pContext = pUtlContext->pApp;

    if( metadata )
    {
        video_standard_t new_videostandard;
        eVideoStandards video_enum = (eVideoStandards)(metadata);

        intel_hdmi_color_depth_t depth = intel_hdmi_color_depth_bpc_8;
        intel_hdmi_color_space_t colorspace = intel_hdmi_color_space_rgb444;
        bool full_range = false;

#if 1 //HDMI_PASS_THROUGH
        // If we are doing pass-thru and we have a valid input standard then use it's colorspace
        // if( pContext->src_standard._details._image_config.is_valid )
        // {
            // depth = pContext->src_standard._details._image_config.color_depth;
            // colorspace = pContext->src_standard._details._image_config.color_space;
            // full_range = pContext->src_standard._details._image_config.full_range;
        // }
#endif // HDMI_PASS_THROUGH

        // Now try and select the standard
        if( (video_enum != kCVO_MAX_MODE) && get_video_standard(video_enum, &(new_videostandard), colorspace, depth, full_range) )
        {
#if (HDMI_PASS_THROUGH && (CLOCKS_ARE_SAME==0))
            // We have a valid standard
            print_video_standard(&new_videostandard, false);

            // Do we currently have an RX input?
            // if( pContext->src_standard._details._image_config.is_valid )
            // {
                // Is this requested standard compatible with the RX standard?
                // if( passthru_compatible(&(pContext->src_standard), &(new_videostandard)) )
                // {
                    logPrint(kDebug, "Compatible Standard\n");
                    pContext->tx._new_video_standard = new_videostandard;
                    if( pContext->tx._state != kHdmiTx_Idle )
                    {
                        pContext->tx._state = kHdmiTx_Available; // Mark as available to set the new output standard
                    }
                    pContext->rx._bTxEdidFollow = false;
                // }
                // else
                // {
                    // logPrint(kDebug, "Incompatible Standard\n");
                // }
            // }
#elif TPG_0_BASE
            // Select the new standard
            pContext->tpg._state = kTpg_NewStandard;
            pContext->tx._new_video_standard = new_videostandard;
            // If the previous standard was invalid then make the TX available again
            if (pContext->tx._state != kHdmiTx_Idle )
            {
                pContext->tx._state = kHdmiTx_Available;
            }
#endif // TPG_0_BASE || (HDMI_PASS_THROUGH && (CLOCKS_ARE_SAME==0))
        }
        else
        {
            logPrint(kWarning, "Invalid standard.");
        }
    }

    return true;
}

//==================================================================

/**
 * @brief Put Tx in Rx follow mode.
 *
 * This function sets the Tx to follow the standard on the Rx.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool SelectRxTxFollow(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL) || (pUtlContext->pApp == NULL))
    {
        return false;
    }

    logPrint(kInfo, "Select Rx/Tx Follow\n");
    app_context_t *pContext = pUtlContext->pApp;
    pContext->bFollow = true;
    pContext->tx._state = kHdmiTx_Available;
    return true;
}

struct MenuItem _txMenuItems[] = 
{
    {"",    "Tx Standards",  NULL,             NULL, kCVO_NULL_MODE        },
    {"v01", "640x480p60",    SelectTxStandard, NULL, kCVO_640x480P60_MODE  },
    {"v02", "720x480p60",    SelectTxStandard, NULL, kCVO_720x480P60_MODE  },
    {"v03", "800x600p60",    SelectTxStandard, NULL, kCVO_800x600P60_MODE  },
    {"v04", "800x600p75",    SelectTxStandard, NULL, kCVO_800x600P75_MODE  },
    {"v05", "1024x768p60",   SelectTxStandard, NULL, kCVO_1024x768P60_MODE },
    {"v06", "1280x720p50",   SelectTxStandard, NULL, kCVO_1280x720P50_MODE },
    {"v07", "1280x720p60",   SelectTxStandard, NULL, kCVO_1280x720P60_MODE },  
    {"v08", "1920x1080p24",  SelectTxStandard, NULL, kCVO_1920x1080P24_MODE},  
    {"v09", "1920x1080p30",  SelectTxStandard, NULL, kCVO_1920x1080P30_MODE},  
    {"v10", "1920x1080p50",  SelectTxStandard, NULL, kCVO_1920x1080P50_MODE},  
    {"v11", "1920x1080p60",  SelectTxStandard, NULL, kCVO_1920x1080P60_MODE},  
    {"v12", "3840x2160p24",  SelectTxStandard, NULL, kCVO_3840x2160P24_MODE},  
    {"v13", "3840x2160p30",  SelectTxStandard, NULL, kCVO_3840x2160P30_MODE},  
    {"v14", "3840x2160p50",  SelectTxStandard, NULL, kCVO_3840x2160P50_MODE},  
    {"v15", "3840x2160p60",  SelectTxStandard, NULL, kCVO_3840x2160P60_MODE},  
#ifdef HDMI_SUPPORT_FRL
    {"v16", "7680x4320p30",  SelectTxStandard, NULL, kCVO_4320P30_MODE},
    {"v17", "7680x4320p60",  SelectTxStandard, NULL, kCVO_4320P60_MODE},
#endif
#ifndef TPG_0_BASE
    {"vf",  "Follow Mode",   SelectRxTxFollow, NULL, kCVO_NULL_MODE},
#endif
    {"",    NULL,            NULL,             NULL, kCVO_NULL_MODE} // Terminate the list
};
#endif // TPG_0_BASE || (CLOCKS_ARE_SAME==0)

#endif // HDMI_TX_SUPPORTED

//==================================================================

/**
 * @brief Array of menu items for the application.
 *
 * This array contains the available menu items and their corresponding actions.
 */
struct MenuItem _gMenuItems[] =
    {
        {"", "General", NULL},
        {"h", "Help", DisplayHelp, NULL},
        {"b", "Banner", displayBanner, NULL},
#if HDMI_RX_SUPPORTED
        {"", "Rx Status", NULL, NULL},
        {"eh", "Rx Edid Mode Help", DisplayHelp, _rxEdidModeMenuItems},
        {"r", "Display Rx Status", RxDisplayStatus, NULL},
        {"i", "Display Cvi Timings", RxDisplayTimings, NULL},
        {"w", "Reset Change Count.", RxResetChangeCount, NULL},
        {"z", "Restart Cvi", RxDebugCvi, NULL},
        {"qe", "Use Cvi Timing", RxDynamicTiming, NULL},
        {"qd", "Use Fixed Timing", RxDynamicTiming, NULL},
        {"", "Rx Hotplug", NULL, NULL},
        {"p0", "De-assert HPD", RxHpdControl, NULL},
        {"p1", "Assert HPD", RxHpdControl, NULL},
        {"pt", "Toggle HPD", RxHpdControl, NULL},
#endif // HDMI_RX_SUPPORTED
#if HDMI_TX_SUPPORTED
        {"", "Tx Status", NULL, NULL},
        {"fh", "Tx FRL Rate Help", DisplayHelp, _txFrlRateMenuItems},
        {"t", "Display Tx Status", TxDisplayStatus, NULL},
        {"y", "DIsplay Tx SCDC", TxDisplaySCDC, NULL},
        {"d", "Dump Tx Edid", TxDumpEdid, NULL},
#if TPG_0_BASE || (HDMI_PASS_THROUGH && (CLOCKS_ARE_SAME==0))
        {"vh", "Tx Standards Help", DisplayHelp, _txMenuItems},
#endif // TPG_0_BASE || (HDMI_PASS_THROUGH && (CLOCKS_ARE_SAME==0))
#endif // HDMI_TX_SUPPORTED
        {"", "Set Log Level", NULL, NULL},
        {"lh", "Log Help", DisplayHelp, _LogMenuItems},
        {"lm", "Max", UtilsMenuFunction, NULL},
        {"ls", "Silent", UtilsMenuFunction, NULL},
        {"c",  "Clearscreen", UtilsMenuFunction, NULL},
        {"", NULL, NULL, NULL} // Terminate the list
};

//==================================================================

/**
 * @brief Main function of the application.
 *
 * This function initializes the application context, displays the banner, and enters the main loop to process user input and manage HDMI contexts.
 *
 * @return 0 on successful execution, 1 on error.
 */
int main()
{

    // MVC
    unsigned int hdmitx_new_std           = 0;
    unsigned int hdmi_tx_fps              = 0;
    unsigned int hdmi_tx_actv_dim_x       = 0;
    unsigned int hdmi_tx_actv_dim_y       = 0;
    unsigned int hdmi_tx_actv_dim         = 0;
    unsigned int hdmi_tx_new_fps          = 0;
    unsigned int hdmi_tx_new_dim_x        = 0;
    unsigned int hdmi_tx_new_dim_y        = 0;     
    
    app_context_t appContext;
    memset(&(appContext), 0, sizeof(appContext));
    
    // Default to video standard pass-thru
    appContext.bFollow = true;
    
    utils_context_t utilsContext; 
    memset(&utilsContext, 0, sizeof(utilsContext));
    utilsContext.pApp = &appContext;
    
#if DEBUG_MODE
    // Set to max logging
    set_log_level(kMaxLogLevel);
#else
    // Silence the logging
    set_log_level(kSilent);
#endif
    // make std-out non-blocking to allow UART output to flush when JTAG is disconnected
    int result;
    result = fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    if (result != 0)
    {
        logPrint(kError, "Failed to switch to non-blocking stdout\n");
        return 1;
    }
    result = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    if (result != 0)
    {
        logPrint(kError, "Failed to switch to non-blocking stdout\n");
        return 1;
    }

    // Set print color to white
    printf("\e[0m");

    // Display the start-up banner
    displayBanner(&(utilsContext), "", NULL, 0);
    logPrint(kInfo, "System startup....\n");

    // Show the available user commands
    DisplayHelp(&utilsContext, "", _gMenuItems, 0);

    // Initialize the board
    board_init();

#if HDMI_TX_SUPPORTED
    // Initialize the Tx Context
    if (!txutils_init_context(&(appContext.tx), (hdmi_tx_base_t)HDMI_TX_0_BASE, (intel_hdmi21_tx_phy_base_t)HDMI_TX_0_PHY_BASE))
    {
        return 1;
    }
    logPrint(kInfo, "HDMI TX initialization done\n");
#endif

#if HDMI_RX_SUPPORTED
    // Initialize the Rx context
    if (!rxutils_init_context(&(appContext.rx), (intel_hdmi_rx_base_t)HDMI_RX_0_BASE, (intel_hdmi21_rx_phy_base_t)HDMI_RX_0_PHY_BASE))
    {
        return 1;
    }

    #if !HDMI_PASS_THROUGH
        // If we're not doing passthrough then setup an EDID to the Rx
        rxutils_update_edid(&(appContext.rx), appContext.edid_data, sizeof(appContext.edid_data), true);
    #endif // HDMI_PASS_THROUGH
    logPrint(kInfo, "HDMI RX initialization done\n");
#endif // HDMI_RX_SUPPORTED

#if HDMI_RX_SUPPORTED
    // Ready to go so assert the RX HPD
    rxutils_assert_hpd(&(appContext.rx), true);
#endif // HDMI_RX_SUPPORTED

    // Loop forever servicing the hardware
    while (1)
    {
        // Do we have any user requests
        CheckMenuInput(&(utilsContext), _gMenuItems);

        // HDMI Tx MVC
        hdmi_tx_actv_dim_x = IORD(NIOS_HDMI_TX_BASE, 0x65) & (0xFFFF);
        hdmi_tx_actv_dim_y = IORD(NIOS_HDMI_TX_BASE, 0x66) & (0xFFFF);

        hdmi_tx_actv_dim  = ( (hdmi_tx_fps << 24) | (hdmi_tx_actv_dim_x << 12) | (hdmi_tx_actv_dim_y));

        // PIO Output to VVP_SS
        IOWR(HDMITX_PIO_NEW_ACTV_DIM_BASE, 0x0, hdmi_tx_actv_dim);

        // PIO Input from VVP_SS
        hdmi_tx_new_fps   = (IORD(HDMITX_PIO_FPS_BASE, 0x0)) & (0xFF);  
        hdmi_tx_new_dim_x = (IORD(HDMITX_PIO_ACTV_DIM_BASE, 0x0) >> 16) & (0xFFFF);        
        hdmi_tx_new_dim_y = (IORD(HDMITX_PIO_ACTV_DIM_BASE, 0x0)) & (0xFFFF);

        // DP Tx CVO loop MVC 
        // if ( (hdmi_tx_new_fps   != hdmi_tx_fps) )        
        if ( (hdmi_tx_new_fps   != hdmi_tx_fps)        ||
             (hdmi_tx_new_dim_x != hdmi_tx_actv_dim_x) ||
             (hdmi_tx_new_dim_y != hdmi_tx_actv_dim_y)    ) 
        {
            switch (hdmi_tx_new_dim_y)
            {
                case 720:
                    SelectTxStandard(&(utilsContext), "v07", NULL, kCVO_1280x720P60_MODE);
                    logPrint(kInfo, "TX_VIC = kCVO_1280x720P60_MODE\n");
                break;
                case 1080:
                    SelectTxStandard(&(utilsContext), "v11", NULL, kCVO_1920x1080P60_MODE);
                    logPrint(kInfo, "TX_VIC = kCVO_1920x1080P60_MODE\n");
                break;
                case 2160:
                    if (hdmi_tx_new_fps == 30) {
                        SelectTxStandard(&(utilsContext), "v13", NULL, kCVO_3840x2160P30_MODE);
                        logPrint(kInfo, "TX_VIC = kCVO_3840x2160P30_MODE\n");
                    }
                    else {
                        SelectTxStandard(&(utilsContext), "v15", NULL, kCVO_3840x2160P60_MODE);
                        logPrint(kInfo, "TX_VIC = kCVO_3840x2160P60_MODE\n");
                    }
                break;           
                default:
                    SelectTxStandard(&(utilsContext), "v11", NULL, kCVO_1920x1080P60_MODE);
                    logPrint(kInfo, "default = kCVO_1920x1080P60_MODE\n");
                break;
            }
            hdmitx_new_std = 1;
            hdmi_tx_fps = hdmi_tx_new_fps;
            printf("New Tx Standard Detected\n");
            printf("Now we need to enabled the HDMI Tx\n");
            printf("hdmi_tx_new_fps = %d \n", hdmi_tx_new_fps);
            printf("hdmi_tx_new_dim_x = %d \n", hdmi_tx_new_dim_x);
            printf("hdmi_tx_new_dim_y = %d \n", hdmi_tx_new_dim_y);
            printf("hdmi_tx_fps = %d \n", hdmi_tx_fps);
            printf("hdmi_tx_actv_dim_x = %d \n", hdmi_tx_actv_dim_x);
            printf("hdmi_tx_actv_dim_y = %d \n", hdmi_tx_actv_dim_y);
        }

#if HDMI_TX_SUPPORTED
        static uint8_t prev_hpd = 0;

        // Poll the TX state machine
        txutils_poll(&(appContext.tx));
        
        // Has the HPD changed state?
        uint8_t tx_hpd = txutils_check_tx_hotplug(&(appContext.tx), prev_hpd, appContext.edid_data, sizeof(appContext.edid_data));
        if ((prev_hpd != tx_hpd))
        {
            if (tx_hpd)
            {
                // Transition to Available
                appContext.tx._state = kHdmiTx_Available;
                logPrint(kDebug, "TX hotplug detected.\n");
    #if HDMI_RX_SUPPORTED
                // Update the EDID when we have a new source connected
                rxutils_update_edid(&(appContext.rx), appContext.edid_data, sizeof(appContext.edid_data), false);
    #endif // HDMI_RX_SUPPORTED
            }
            else
            {
                logPrint(kWarning, "TX hotplug lost.\n");
                txutils_enable_output(&(appContext.tx), false);
            }
        }
        // Remember the TX HPD for next time round the loop
        prev_hpd = tx_hpd;
#endif // HDMI_TX_SUPPORTED

#if HDMI_RX_SUPPORTED
        // Get the current state of the connection
        rxutils_get_status(&(appContext.rx));

        switch (appContext.rx._state)
        {
        case kHdmiRx_NewStandard:
            if (!rxutils_get_video_standard(&(appContext.rx), &(appContext.src_standard), HDMI_TX_SUPPORTED ? true : false))
            {
                appContext.rx._state = kHdmiRx_Idle;
            }
    #if HDMI_TX_SUPPORTED
            // Always enter follow mode when standard changes
            appContext.bFollow = true;

            // If the previous standard was invalid then make the TX available again
            if (appContext.tx._state == kHdmiTx_InvalidFormat)
            {
                appContext.tx._state = kHdmiTx_Available;
            }
    #endif // HDMI_RX_SUPPORTED
            print_video_standard(&(appContext.src_standard), false);
            break;
        case kHdmiRx_Enabled:
            break;
        default:
            memset(&(appContext.src_standard), 0, sizeof(video_standard_t));
            break;
        }
#endif // HDMI_RX_SUPPORTED

        // MVC Rx Loop
        if (appContext.rx._state == kHdmiRx_NewStandard)
        {
            // If RX Only design we simple acknowledge the standard change
            logPrint(kDebug, "ENABLE Decoupled Rx and Tx: Rx config side\n");            
            appContext.rx._state = kHdmiRx_Enabled;
        }        
        
        // MVC Tx Loop
        if ( (hdmitx_new_std) && ((appContext.tx._state == kHdmiTx_Available) || (appContext.tx._state == kHdmiTx_Enabled)) )
        {
            logPrint(kDebug, "ENABLE Decoupled Rx and Tx: Tx config side\n");
            hdmitx_new_std = 0;
            video_standard_t *pTxStandard;


            pTxStandard = &(appContext.tx._new_video_standard);
            logPrint(kInfo, "Using MVC Tx Standard\n");
            printf("The HDMI Tx is now enabled\n");
            print_video_standard(pTxStandard, false);

            bool bEnabled = false;
            bEnabled = txutils_set_standard(&(appContext.tx), pTxStandard);
            if (!bEnabled)
            {
                // Mark the tx as enabled to so that we don't get thrashing for unsupported formats
                appContext.tx._state = kHdmiTx_InvalidFormat;
            }
        }

    }

    return 0;
}
