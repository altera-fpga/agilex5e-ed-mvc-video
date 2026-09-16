#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/alt_timestamp.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "board.h"

#if HDMI_TX_SUPPORTED
#include "tx_utils.h"
#include "intel_hdmi_common_return_types.h"
#include "utils.h"

// Definition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Tx", format, ##__VA_ARGS__))

// Function to convert frl rate to a string. Implementation is in main.c
extern const char* frl_rate_to_string(uint32_t frl_rate);

#define TX_POLL_RATE 1000 // Poll every 1000ms (1 second)

//==================================================================

/**
 * @brief Poll timeout function for HDMI TX.
 *
 * This function is called when the poll timeout occurs. It sets a flag to indicate that the poll timeout has occurred.
 *
 * @param[in] context Pointer to the HDMI TX context.
 * @return 0 to indicate no restart of the alarm.
 */
alt_u32 poll_timeout_fn(void* context)
{
    HdmiTxContext *pContext = (HdmiTxContext *)context;
    if (pContext == NULL)
    {
        return 0; // No restart of the alarm
    }
    pContext->_poll_timeout = true;
    return pContext->_poll_rate; // Restart the timer
}

//==================================================================

/**
 * @brief Initializes the HDMI TX context.
 *
 * This function initializes the HDMI TX context with the specified base addresses and sets the initial state.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] base Base address for the HDMI TX.
 * @param[in] phy_base Base address for the HDMI TX PHY.
 * @return True if the context is successfully initialized, false otherwise.
 */
bool txutils_init_context(HdmiTxContext *pContext, hdmi_tx_base_t base, intel_hdmi21_tx_phy_base_t phy_base)
{
    if (pContext == NULL)
    {
        // Invalid context pointer
        logPrint(kInfo, "ERROR: Invalid context pointer.");
        return false;
    }
    memset(pContext, 0, sizeof(HdmiTxContext));
    pContext->_current_lts = -1;
    pContext->_state = kHdmiTx_Idle;
    pContext->_current_video_standard._details._image_config.is_valid = false;
    pContext->_tx_frl_rate = TX_MAX_FRL_RATE;

    if (intel_hdmi21_tx_phy_init(&(pContext->_phy_instance), phy_base) != intel_hdmi_success)
    {
        logPrint(kInfo, "ERROR: Failed to initialize the hdmi tx phy.");
        return false;
    }

    // Setup a convenient pointer to the instance
    if (intel_hdmi21_tx_init(&(pContext->_instance), base, &(pContext->_phy_instance)) != intel_hdmi_success)
    {
        logPrint(kInfo, "ERROR: Failed to initialize the hdmi pContext->");
        return false;
    }

    // Disable DVI mode
    if (intel_hdmi21_tx_set_dvi_mode((&(pContext->_instance)), 0) != intel_hdmi_success)
    {
        logPrint(kInfo, "ERROR: Failed to disable DVI mode.");
        return false;
    }

    // Turn off the output
    intel_hdmi21_tx_enable(&(pContext->_instance), 0);

    // Setup the polling timeout
    pContext->_poll_rate = TX_POLL_RATE;
    pContext->_poll_timeout = false;
    alt_alarm_start(&(pContext->_poll_alarm), pContext->_poll_rate, poll_timeout_fn, pContext);

    return true;
}

//==================================================================

/**
 * @brief Prints the current link training state.
 *
 * This function logs the current link training state of the HDMI TX.
 *
 * @param[in] tx_lts The current link training state.
 */
void print_lts(intel_hdmi21_tx_link_training_state_t tx_lts)
{
    switch (tx_lts)
    {
    case link_training_init:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_init\n");
        break;
    case link_training_LTS1:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTS1\n");
        break;
    case link_training_LTS2:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTS2\n");
        break;
    case link_training_LTS3:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTS3\n");
        break;
    case link_training_LTS4:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTS4\n");
        break;
    case link_training_LTSL:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTSL\n");
        break;
    case link_training_LTSP:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_LTSP\n");
        break;
    default:
        logPrint(kInfo, "\tHDMI TX LTS= link_training_invalid\n");
        break;
    }
}

//==================================================================

/**
 * @brief Delay for user defined time
 *
 * This function pauses the nios for milliseconds
 *
 * @param[in] msec Milliseconds to wait
 */
void txutils_pause_msec (int32_t msec)
{
    long    delay_duration;
    clock_t current_time;
    clock_t start_time;

    delay_duration = msec*(CLOCKS_PER_SEC/100000);
    current_time = clock();
    start_time = clock();
    while( (current_time-start_time) < delay_duration )
        current_time = clock();
}

//==================================================================

/**
 * @brief Sets the video standard for the HDMI TX context.
 *
 * This function configures the HDMI TX context with the specified video standard.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] pStandard Pointer to the video standard structure.
 * @return True if the standard is successfully set, false otherwise.
 */
bool txutils_set_standard(HdmiTxContext *pContext, video_standard_t *pStandard)
{
    if ((pContext == NULL) || (pStandard == NULL))
    {
        logPrint(kInfo, "tx_utils_enable_tx: Invalid pointers.\n");
        return false;
    }

    if (pStandard->_video_timing == NULL)
    {
        logPrint(kInfo, "Invalid tx standard, no timing info.\n");
        txutils_enable_output(pContext, false);
        pContext->_current_video_standard._details._image_config.is_valid = false; 
        return false;
    }

    logPrint(kInfo, "Enabling HDMI TX\n");
    print_video_standard(pStandard, false);
    // Take a copy of the video standard, used when displaying the status
    pContext->_current_video_standard = *pStandard;

    txutils_enable_output(pContext, false);
    uint32_t tmds_freq;
    uint32_t frl_divide;
    intel_hdmi_tmds_ratio_t tmds_ratio;
    if (pContext->_tx_frl_rate)
    {
        get_frl_settings(pStandard, pStandard->_details._image_config.color_depth, pStandard->_details._image_config.color_space, &tmds_freq, &tmds_ratio);
    } else {
        get_tmds_settings(pStandard, pStandard->_details._image_config.color_depth, pStandard->_details._image_config.color_space, &tmds_freq, &tmds_ratio);		
    }
    logPrint(kInfo, "\ttmds_freq %lu, tmds_ratio %d\n", tmds_freq, tmds_ratio);
    // FRL TMDS Clock needs to be scaled based upon datapath width
    frl_divide = pContext->_tx_frl_rate ? PIXEL_IN_PARALLEL : 1;
    board_set_txclk(tmds_freq/frl_divide,tmds_ratio);
    usleep(100*1000);
    intel_hdmi21_tx_set_tmds_config(&(pContext->_instance), tmds_ratio, tmds_ratio);
    intel_hdmi21_tx_phy_set_tmds_freq(&(pContext->_phy_instance), tmds_freq / 100);

    logPrint(kInfo, "\tSetting Tx FRL Rate= %s\n", frl_rate_to_string(pContext->_tx_frl_rate));
    
    uint8_t tx_lts_prev = intel_hdmi21_tx_poll(&(pContext->_instance));
    intel_hdmi21_tx_reconfig(&(pContext->_instance), pContext->_tx_frl_rate);
    uint8_t tx_lts;
    do
    {
        tx_lts = intel_hdmi21_tx_poll(&(pContext->_instance));
        if (tx_lts != tx_lts_prev)
        {
            print_lts(tx_lts);

        }
        tx_lts_prev = tx_lts;
    } while ((tx_lts != link_training_LTSP) && (tx_lts != link_training_LTSL));

    // Needs to be moved into driver .
    if (tx_lts == link_training_LTSP) {
        uint8_t frl_start_timeout = 125; // 250mS wait for FRL_Start during LTSP
        do {
        	txutils_pause_msec(2);
        	tx_lts = intel_hdmi21_tx_poll(&(pContext->_instance));
        	if (pContext->_instance.frl_start)
        		frl_start_timeout = 0;
        	else
        		frl_start_timeout--;
        } while (frl_start_timeout);

    }
    uint32_t frl_rate = intel_hdmi21_tx_get_frl_rate(&(pContext->_instance));
    logPrint(kInfo, "\tCompleted link training - FRL Rate = %d\n", frl_rate);

    tx_lts_prev = tx_lts;

    intel_hdmi21_tx_set_dvi_mode(&(pContext->_instance), 0);
    intel_hdmi21_tx_enable_avi_infoframe(&(pContext->_instance), 1);
    intel_hdmi21_tx_avi_packet_data(&(pContext->_instance), pStandard->_avi_pckt_data);
    intel_hdmi21_tx_set_color_space(&(pContext->_instance), pStandard->_details._image_config.color_space);
    intel_hdmi21_tx_set_color_depth(&(pContext->_instance), pStandard->_details._image_config.color_depth);

    intel_hdmi21_tx_set_output_mode(&(pContext->_instance), 0, &(pStandard->_video_timing->_cvo_params), 0, 0, 0, 0);
    txutils_enable_output(pContext, true);

    // Mark the context as running
    pContext->_state = kHdmiTx_Enabled;

    return true;
}

//==================================================================

/**
 * @brief Enables or disables the HDMI TX output.
 *
 * This function controls the HDMI TX output based on the specified enable flag.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] bEnable Boolean flag to enable or disable the TX output.
 */
void txutils_enable_output(HdmiTxContext *pContext, const bool bEnable)
{
    if (pContext == NULL)
    {
        logPrint(kError, "Invalid Tx Context\n");
    }
    logPrint(kInfo, "\t%s Tx Output\n", bEnable ? "Enable" : "Disable");
    intel_hdmi21_tx_enable(&(pContext->_instance), bEnable ? 1 : 0);
    // Give time for the output to spin up
    usleep(100*1000);
}

//==================================================================

/**
 * @brief Checks the state of the TX HPD and updates RX EDID if changed.
 *
 * This function checks the TX HPD state and updates the RX EDID if there is a change.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] tx_hpd_prev Previous state of the TX HPD.
 * @param[out] edid_data Pointer to the EDID data buffer.
 * @param[in] edid_data_size Size of the EDID data buffer.
 * @return The current state of the TX HPD.
 */
uint8_t txutils_check_tx_hotplug(HdmiTxContext *pContext, uint8_t tx_hpd_prev, uint8_t *edid_data, uint32_t edid_data_size)
{
    if (pContext == NULL)
    {
        logPrint(kError, "Invalid Tx Context\n");
    }
    uint8_t tx_hpd = intel_hdmi21_tx_get_hotplug_detect(&(pContext->_instance));

    // check for inactive -> active transition
    if (tx_hpd && (tx_hpd != tx_hpd_prev))
    {
        uint8_t valid;
        uint32_t edid_size;
        uint32_t retries = 5;

        do
        {
            if (intel_hdmi21_tx_edid_get_sink_data(&(pContext->_instance), edid_data, edid_data_size, &valid, &edid_size) == intel_hdmi_success)
            {
                if (valid)
                {
                    intel_hdmi_sink_format_support_t tx_sink_format_support;
                    if (valid && intel_hdmi21_tx_edid_get_sink_support(&(pContext->_instance), edid_data, &tx_sink_format_support) == intel_hdmi_success)
                    {
                        logPrint(kInfo, "EDID Sink format support updated:\n");

                        logPrint(kInfo, "\tSink Name          = ");
                        for (int z = 0; z < 13; z++)
                        {
                            logPrint(kInfo | kSuppress, "%c", tx_sink_format_support.sinkName[z]);
                        }
                        logPrint(kInfo | kSuppress, "\n");
                        logPrint(kInfo, "\tY444 Support       = %d\n", tx_sink_format_support.y444_supported);
                        logPrint(kInfo, "\tY422 Support       = %d\n", tx_sink_format_support.y422_supported);
                        logPrint(kInfo, "\tMax TMDS char rate = %lu\n", tx_sink_format_support.max_tmds_char_rate);
                        logPrint(kInfo, "\tSCDC present       = %d\n", tx_sink_format_support.scdc_present);
                        logPrint(kInfo, "\tMax FRL rate       = %lu\n", tx_sink_format_support.max_frl_rate);
                    }
                }
                else
                {
                    logPrint(kInfo, "ERROR: EDID read from sink failed\n");
                }
            }
        } while (valid == 0 && --retries);
    }
    return tx_hpd;
}

/**
 * @brief Checks the state of the TX HPD and updates RX EDID if changed.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[out] edid_data Pointer to the EDID data buffer.
 * @param[in] edid_data_size Size of the EDID data buffer.
 * @return The size of the read EDID.
 */
uint32_t txutils_read_edid(HdmiTxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size)
{
    uint32_t edid_size = 0;
    if (pContext == NULL)
    {
        logPrint(kError, "Invalid Tx Context\n");
    }
    uint8_t tx_hpd = intel_hdmi21_tx_get_hotplug_detect(&(pContext->_instance));

    // check for inactive -> active transition
    if (tx_hpd )
    {
        uint8_t valid;
        if (intel_hdmi21_tx_edid_get_sink_data(&(pContext->_instance), edid_data, edid_data_size, &valid, &edid_size) == intel_hdmi_success)
        {
            if (!valid)
            {
                logPrint(kError, "Failed to read Tx EDID.\n");
                return 0;
            }
        }
    }
    return edid_size;
}

//==================================================================

/**
 * @brief Sets the TX clock frequency.
 *
 * This function configures the TX clock frequency based on the specified TMDS frequency and ratio.
 *
 * @param[in] phy Pointer to the HDMI TX PHY instance.
 * @param[in] tmds_freq The TMDS frequency.
 * @param[in] tmds_ratio The TMDS ratio.
 */
void set_txclk_freq(intel_hdmi21_tx_phy_instance_t *phy, uint32_t tmds_freq, intel_hdmi_tmds_ratio_t tmds_ratio)
{
    logPrint(kInfo, "TX TMDS Clock Freq = %luHz, Ratio %d \n", tmds_freq, tmds_ratio);
    intel_hdmi21_tx_phy_xcvr_pll_reset(phy, 0);

#if CLOCKS_ARE_SAME == 0
    board_set_txclk((double)tmds_freq,(tmds_ratio!=intel_hdmi_tmds_ratio_one_over_10));
#else
    logPrint(kInfo, "        Reseting PLL Only \n");
#endif

    intel_hdmi21_tx_phy_xcvr_pll_reset(phy, 1);
}

//==================================================================

/**
 * @brief Converts HDMI TX state to a string representation.
 *
 * This function returns a string representation of the given HDMI TX state.
 *
 * @param[in] state The HDMI TX state to convert.
 * @return A string representation of the state.
 */
const char *txutils_state_str(eHdmiTxState state)
{
    switch (state)
    {
        case kHdmiTx_Idle:
            return "Idle";
            break;
        case kHdmiTx_Available:
            return "Available";
            break;
        case kHdmiTx_Enabled:
            return "Enabled";
            break;
        case kHdmiTx_InvalidFormat:
            return "Invalid";
            break;
        default:
            return "Unknown";
            break;
    }
}

//==================================================================
#define INTEL_HDMI_COMMON_SCDC_I2C_ADDR (0xA8)
static void hdmi_tx_scdc_read(intel_hdmi21_tx_instance_t* instance, uint8_t reg, uint8_t len, uint8_t *data)
{
    if( (instance == NULL) || (data == NULL) )
    {
        return;
    }
    intel_hdmi_common_i2c_burst_read_extended(&(instance->i2c_instance), INTEL_HDMI_COMMON_SCDC_I2C_ADDR >> 1, reg, len, data);
}

/**
 * @brief Display the current SCDC information if available.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 */
void txutils_display_scdc(HdmiTxContext *pContext)
{
    if (pContext == NULL)
    {
        logPrint(kError, "Invalid Tx Context\n");
        return;
    }

    uint8_t version[2];
    uint8_t update_flags[2];
    uint8_t tmds_config[2];
    uint8_t config[2];
    uint8_t source_test_cfg;
    uint8_t status_flags[3];
    uint8_t err[11];
    uint8_t ieee_oui[3];
    uint8_t read_req;


    hdmi_tx_scdc_read(&(pContext->_instance), 0x01, 2, version);
    hdmi_tx_scdc_read(&(pContext->_instance), 0x10, 2, update_flags);
    hdmi_tx_scdc_read(&(pContext->_instance), 0x20, 2, tmds_config);
    hdmi_tx_scdc_read(&(pContext->_instance), 0x30, 2, config);
    hdmi_tx_scdc_read(&(pContext->_instance), 0x35, 1, &source_test_cfg);
    hdmi_tx_scdc_read(&(pContext->_instance), 0x40, 3, status_flags);


    hdmi_tx_scdc_read(&(pContext->_instance), 0x50, 11, err);
    hdmi_tx_scdc_read(&(pContext->_instance), 0xC0, 1, &read_req);
    hdmi_tx_scdc_read(&(pContext->_instance), 0xD0, 3, ieee_oui);    

    logPrint(kInfo, "SCDC :\n");
    logPrint(kInfo, "\tSCDC_SINK_VERSION       = %X\n",version[0]); // R Sink Version See Section 10.4.1.2
    logPrint(kInfo, "\tSCDC_SOURCE_VERSION     = %X\n",version[1]); // R/W Source Version See Section 10.4.1.2
    logPrint(kInfo, "\tSCDC_UPDATE_0           = %X\n",update_flags[0]); // R/W Update_0 See Section 10.4.1.3
    logPrint(kInfo, "\tSCDC_UPDATE_1           = %X\n",update_flags[1]); // R/W Update_1 See Section 10.4.1.3
    
    logPrint(kInfo, "\tSCDC_TMDS_CONFIG        = %X\n",tmds_config[0]); // R/W TMDS_Config See Section 10.4.1.4
    logPrint(kInfo, "\tSCDC_SCRAMBLER_state   = %X\n",tmds_config[1]); // R Scrambler_state See Section 10.4.1.5
    logPrint(kInfo, "\tSCDC_CONFIG_0           = %X\n",config[0]); // R/W Config_0 See Section 10.4.1.6
    logPrint(kInfo, "\tSCDC_CONFIG_1           = %X\n",config[1]); // R/W Config_0 See Section 10.4.1.6
    logPrint(kInfo, "\tSCDC_SOURCE_TEST_CONFIG = %X\n",source_test_cfg); // R/W Config_0 See Section 10.4.1.6
    
    logPrint(kInfo, "\tSCDC_state_FLAG_0      = %X\n",status_flags[0]); //R Status_Flags_0 See Section 10.4.1.7
    logPrint(kInfo, "\tSCDC_state_FLAG_1      = %X\n",status_flags[1]); //R Status_Flags_1 See Section 10.4.1.7
    logPrint(kInfo, "\tSCDC_state_FLAG_2      = %X\n",status_flags[2]); //R Status_Flags_1 See Section 10.4.1.7

#if (ALT_TIMESTAMP_CLK!=none) 
    uint64_t error_time_ns = 0;
    error_time_ns = alt_timestamp() * (uint64_t)(1000000000) / alt_timestamp_freq();
    logPrint(kInfo, "\tSCDC_ERR_DE_TIME_NS     = %llu\n",error_time_ns);
#endif
    logPrint(kInfo, "\tSCDC_ERR_DE_0           = %X\n",(err[1] << 8) | err[0]);
    logPrint(kInfo, "\tSCDC_ERR_DE_1           = %X\n",(err[3] << 8) | err[2]);
    logPrint(kInfo, "\tSCDC_ERR_DE_2           = %X\n",(err[5] << 8) | err[4]);
    logPrint(kInfo, "\tSCDC_ERR_DE_3           = %X\n",(err[8] << 8) | err[7]);
    logPrint(kInfo, "\tSCDC_ERR_DE_CHECKSUM    = %x\n",err[6]);
    logPrint(kInfo, "\tSCDC_RS_COUNTER         = %X\n",(err[10] << 8) | err[9]);

    logPrint(kInfo, "\tSCDC_TEST_CONFIG_0      = %X\n",read_req); // R/W Test_Config_0 See Section 10.4.1.9

    logPrint(kInfo, "\tSCDC_IEEE_OUI_2         = %X\n",ieee_oui[0]); // R Manufacturer IEEE OUI, Third Octet See Section 10.4.1.10
    logPrint(kInfo, "\tSCDC_IEEE_OUI_1         = %X\n",ieee_oui[1]); // R Manufacturer IEEE OUI, Second Octet See Section 10.4.1.10
    logPrint(kInfo, "\tSCDC_IEEE_OUI_0         = %X\n",ieee_oui[2]); // R Manufacturer IEEE OUI, First Octet See Section 10.4.1.10"
}

//==================================================================
/**
 * @brief Gets the current FRL rate of the HDMI TX.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @return The current FRL rate.
 */
uint8_t txutils_current_frl_rate(HdmiTxContext *pContext)
{
    return intel_hdmi21_tx_get_frl_rate(&(pContext->_instance));
}

//==================================================================
/**
 * @brief Polls the HDMI TX context for updates.
 *
 * This function is called periodically to check the state of the HDMI TX and perform necessary updates.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 */
void txutils_poll(HdmiTxContext *pContext)
{
    if (pContext == NULL)
    {
        logPrint(kError, "Invalid Tx Context\n");
        return;
    }

    // Check if the poll timeout has occurred
    if (pContext->_poll_timeout)
    {
        pContext->_poll_timeout = false;

    }
}
#endif // HDMI_TX_SUPPORTED