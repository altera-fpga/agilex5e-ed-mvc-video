// Contains application specific types
#ifndef _TYPES_H_
#define _TYPES_H_

#include "edid.h"
#include "video_standards.h"
#include "tpg_utils.h"
#include "tx_utils.h"
#include "rx_utils.h"
#include "utils.h"

#define MAX_MENU_CODE (3)

//==================================================================

/**
 * @brief Structure representing the application context.
 */
struct app_context_t 
{
#if TPG_0_BASE
    TpgContext tpg; /**< TPG context. */
#endif              // TPG_0_BASE
#if HDMI_RX_0_BASE && HDMI_RX_0_PHY_BASE
    HdmiRxContext rx; /**< HDMI RX context. */
#endif                // HDMI_RX_0_BASE && HDMI_RX_0_PHY_BASE
#if HDMI_TX_0_BASE && HDMI_TX_0_PHY_BASE
    HdmiTxContext tx; /**< HDMI TX context. */
#endif                // HDMI_TX_0_BASE && HDMI_TX_0_PHY_BASE

    bool bFollow; /**< Set true to indicate input format is passed to output */
    video_standard_t src_standard;                      /**< The current source (RX or TPG) standard. */
    uint8_t edid_data[NUM_EDID_PAGES * EDID_PAGE_SIZE]; /**< Current EDID to be used for RX device. */
};

#endif // _TYPES_H_