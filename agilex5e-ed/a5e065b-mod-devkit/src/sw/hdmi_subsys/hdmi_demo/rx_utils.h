#ifndef _RX_UTILS_H_
#define _RX_UTILS_H_

#include "config.h"

#if HDMI_RX_SUPPORTED
#include "intel_hdmi21_rx.h"
#include "video_standards.h"
#include "edid.h"

/**
 * @brief Enumeration representing the state of the HDMI RX.
 */
typedef enum
{
    kHdmiRx_Idle,        /**< Indicates that the Rx device is setup but has not detected anything. */
    kHdmiRx_NewStandard, /**< A new standard has been detected. */
    kHdmiRx_Enabled,     /**< Rx is in the enabled state. */
} eHdmiRxState;

/**
 * @brief Structure representing the HDMI RX context.
 */
typedef struct _HdmiRxContext
{
    intel_hdmi21_rx_instance_t _instance;                   /**< Holds the initialized RX instance. */
    intel_hdmi21_rx_phy_instance_t _phy_instance;           /**< Holds the initialized PHY instance. */
    eHdmiRxState _state;                                    /**< Current state of the RX instance. */
    uint8_t _current_edid[NUM_EDID_PAGES * EDID_PAGE_SIZE]; /**< Holds the latest EDID written to the RX. */
    uint32_t _change_count;                                 /**< Increments each time a new standard is detected. */
    hdmi_tx_standard_t _video_timing;                       /**< Context video standard used for pass thru */
    uint8_t _sticky_5v;                                     /**< Indicates whether the 5v detect has dropped since last query */
    uint8_t _sticky_video_locked;                           /**< Indicates whether the video_locked was lost since last query */
    uint8_t _sticky_rx_locked;                              /**< Indicates whether the rx_lockec was lost since last query */
    bool _sticky_bCviLocked;                                /**< Indicates whether the CVi lost lock since last query */
    bool _bUseDynamicTimings;                               /**< Indicates that we should calculate the TX timing parameters */
    bool _bTxEdidFollow;                                    /**< Indicates that the RX should follow the TX EDID */
    uint32_t _frl_rate;                                     /**< The current FRL rate for the RX. */
} HdmiRxContext;

/**
 * @brief Initializes the HDMI RX context.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] base Base address for the HDMI RX.
 * @param[in] phy_base Base address for the HDMI RX PHY.
 * @return True if the context is successfully initialized, false otherwise.
 */
extern bool rxutils_init_context(HdmiRxContext *pContext, intel_hdmi_rx_base_t base, intel_hdmi21_rx_phy_base_t phy_base);

/**
 * @brief Retrieves the video standard from the HDMI RX context.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[out] pStandard Pointer to the video standard structure to be updated.
 * @param[in] bValidForTx Boolean indicating if the standard is valid for transmission.
 * @return True if the video standard is valid, false otherwise.
 */
extern bool rxutils_get_video_standard(HdmiRxContext *pContext, video_standard_t *pStandard, bool bValidForTx);

/**
 * @brief Enables or disables the HDMI RX CVI.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] bEnable Boolean flag to enable or disable the RX CVI.
 */
extern void rxutils_enable(HdmiRxContext *pContext, const bool bEnable);

/**
 * @brief Reads the status of the HDMI RX context.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[out] pRx_5v Pointer to store the 5V status.
 * @param[out] pRxStatus Pointer to store the RX status.
 * @param[out] pCviLocked Pointer to store the CVI lock status.
 */
extern void rxutils_read_status(HdmiRxContext *pContext, uint8_t *pRx_5v, intel_hdmi21_rx_status_t *pRxStatus, bool *pCviLocked);

/**
 * @brief Gets the current status of the HDMI RX context.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 */
extern void rxutils_get_status(HdmiRxContext *pContext);

/**
 * @brief Reconfigures the HDMI RX context with a specified FRL rate.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] frl_rate The FRL rate to configure.
 * @param[in] bForce Boolean flag to force the reconfiguration.
 */
extern void rxutils_reconfig(HdmiRxContext *pContext, uint8_t frl_rate, bool bForce);

/**
 * @brief Sets the EDID data for the HDMI RX context.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] edid_data Pointer to the EDID data.
 * @param[in] edid_data_size Size of the EDID data.
 * @param[in] bForce Boolean flag to force the update.
 */
extern void rxutils_set_edid(HdmiRxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size, bool bForce);

/**
 * @brief Sets up the Edid depending on the request mode of edid passthru or specific FRL rate.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] edid_data Pointer to the EDID data.
 * @param[in] edid_data_size Size of the EDID data.
 * @param[in] bForce Boolean flag to force the update.
 */
extern void rxutils_update_edid(HdmiRxContext *pContext, uint8_t *edid_data, uint32_t edid_data_size, bool bForce);

/**
 * @brief Asserts or de-asserts the HDMI RX HPD.
 *
 * @param[in] pContext Pointer to the HDMI RX context.
 * @param[in] bEnable Boolean flag to assert or de-assert the HPD.
 */
extern void rxutils_assert_hpd(HdmiRxContext *pContext, const bool bEnable);

/**
 * @brief Converts HDMI RX state to a string representation.
 *
 * @param[in] state The HDMI RX state to convert.
 * @return A string representation of the state.
 */
extern const char *rxutils_state_str(eHdmiRxState state);

/**
 * @brief Display HDMI Rx video timings.
 *
 * @param[in] state The HDMI RX state to convert.
 */
extern void rxutils_display_timings(HdmiRxContext *pContext);

/**
 * @brief Reset status counters.
 *
 * @param[in] state The HDMI RX state to convert.
 */
extern void rxutils_reset_counters(HdmiRxContext *pContext);
#endif // HDMI_RX_SUPPORTED

#endif // _RX_UTILS_H_