#ifndef _RX_TX_UTILS_H_
#define _RX_TX_UTILS_H_

#include "config.h"

#if HDMI_TX_SUPPORTED
#include <sys/alt_alarm.h>
#include "intel_hdmi21_tx.h"
#include "intel_hdmi21_tx_edid.h"
#include "video_standards.h"

/**
 * @brief Enumeration representing the state of the HDMI TX.
 */
typedef enum
{
    kHdmiTx_Idle,         /**< Indicates that the TX is not doing anything. */
    kHdmiTx_Available,    /**< Indicates a monitor is attached and awaiting setup. */
    kHdmiTx_Enabled,      /**< Indicates that the TX is transmitting. */
    kHdmiTx_InvalidFormat /**< Indicates that the TX standard attempt was invalid. */
} eHdmiTxState;

/**
 * @brief Structure representing the HDMI TX context.
 */
typedef struct _HdmiTxContext
{
    intel_hdmi21_tx_instance_t _instance;               /**< Holds the initialized TX instance. */
    intel_hdmi21_tx_phy_instance_t _phy_instance;       /**< Holds the initialized PHY instance. */
    intel_hdmi21_tx_link_training_state_t _current_lts; /**< Current status of the link training. */
    uint8_t _hpd;                                       /**< Current state of the HPD. */
    eHdmiTxState _state;                                /**< Current state of the TX instance. */
    video_standard_t _current_video_standard;           /**< Last video standard applied to the output. */
    video_standard_t _new_video_standard;               /**< Try and establish this standard when requested. */
    uint32_t    _tx_frl_rate;                              /**< Current TX FRL rate. */
    
    uint32_t    _poll_rate;                             /**< Poll rate for the FRL Poll Rate register. */
    bool        _poll_timeout;                          /**< Flag to indicate if the poll timeout has occurred. */
    alt_alarm   _poll_alarm;                           /**< Polling alarm for the TX. */
} HdmiTxContext;

/**
 * @brief Initializes the HDMI TX context.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] base Base address for the HDMI TX.
 * @param[in] phy_base Base address for the HDMI TX PHY.
 * @return True if the context is successfully initialized, false otherwise.
 */
extern bool txutils_init_context(HdmiTxContext *pContext, hdmi_tx_base_t base, intel_hdmi21_tx_phy_base_t phy_base);

/**
 * @brief Sets the video standard for the HDMI TX context.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] pStandard Pointer to the video standard structure.
 * @return True if the standard is successfully set, false otherwise.
 */
extern bool txutils_set_standard(HdmiTxContext *pContext, video_standard_t *pStandard);

/**
 * @brief Enables or disables the HDMI TX output.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] bEnable Boolean flag to enable or disable the TX output.
 */
extern void txutils_enable_output(HdmiTxContext *pContext, const bool bEnable);

/**
 * @brief Checks the state of the TX HPD and returns the current Tx EDID.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[in] tx_hpd_prev Previous state of the TX HPD.
 * @param[out] edid_data Pointer to the EDID data buffer.
 * @param[in] edid_data_size Size of the EDID data buffer.
 * @return The size of the read EDID.
 */
extern uint8_t txutils_check_tx_hotplug(HdmiTxContext *pContext, uint8_t tx_hpd_prev, uint8_t *edid_data, uint32_t edid_data_size);

/**
 * @brief Reads the Tx EDID.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @param[out] edid_data Pointer to the EDID data buffer.
 * @param[in] edid_data_size Size of the EDID data buffer.
 * @return The current state of the TX HPD.
 */
extern uint32_t txutils_read_edid(HdmiTxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size);

/**
 * @brief Prints the current link training state.
 *
 * @param[in] tx_lts The current link training state.
 */
extern void print_lts(intel_hdmi21_tx_link_training_state_t tx_lts);

/**
 * @brief Sets the TX clock frequency.
 *
 * @param[in] phy Pointer to the HDMI TX PHY instance.
 * @param[in] tmds_freq The TMDS frequency.
 * @param[in] tmds_ratio The TMDS ratio.
 */
extern void set_txclk_freq(intel_hdmi21_tx_phy_instance_t *phy, uint32_t tmds_freq, intel_hdmi_tmds_ratio_t tmds_ratio);

/**
 * @brief Converts HDMI TX state to a string representation.
 *
 * @param[in] state The HDMI TX state to convert.
 * @return A string representation of the state.
 */
extern const char *txutils_state_str(eHdmiTxState state);

/**
 * @brief Display the current SCDC information if available.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 */
extern void txutils_display_scdc(HdmiTxContext *pContext);

/**
 * @brief Gets the current FRL rate of the HDMI TX.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 * @return The current FRL rate.
 */
extern uint8_t txutils_current_frl_rate(HdmiTxContext *pContext);

/**
 * @brief Polls the HDMI TX context for updates.
 *
 * This function is called periodically to check the state of the HDMI TX and perform necessary updates.
 *
 * @param[in] pContext Pointer to the HDMI TX context.
 */
extern void txutils_poll(HdmiTxContext *pContext);

#endif // HDMI_TX_SUPPORTED

#endif // _RX_TX_UTILS_H_
