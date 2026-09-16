#include <stdio.h>
#include "config.h"
#include "tpg_utils.h"
#include "utils.h"

// Definition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Tpg", format, ##__VA_ARGS__))

// Enable when in TX Only mode
#if !HDMI_RX_SUPPORTED & HDMI_TX_SUPPORTED

/**
 * @brief Initializes the TPG context.
 *
 * This function initializes the TPG context with the specified base address and sets the initial state.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] base Base address for the TPG.
 * @return True if the context is successfully initialized, false otherwise.
 */
bool tpgutils_init_context(TpgContext *pContext, intel_vvp_core_base base)
{
    if (pContext == NULL)
    {
        logPrint(kDebug, "init_tpg_context: Invalid pointer");
        return false;
    }
    intel_vvp_tpg_init(&(pContext->_instance), base);
    pContext->_state = kTpg_Idle;
    tpgutils_enable_output(pContext, false);
    return true;
}

/**
 * @brief Sets the video standard for the TPG context.
 *
 * This function configures the TPG context with the specified video standard.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] pStandard Pointer to the video standard structure.
 */
void tpgutils_set_standard(TpgContext *pContext, const video_standard_t *pStandard)
{
    if ((pContext == NULL) && (pStandard == NULL))
    {
        logPrint(kError, "tpg_set_standard: Invalid pointers.");
        return;
    }
    intel_vvp_core_set_img_info_width(&(pContext->_instance), pStandard->_details._image_config.active_x);
    intel_vvp_core_set_img_info_height(&(pContext->_instance), pStandard->_details._image_config.active_y);
    intel_vvp_core_set_img_info_interlace(&(pContext->_instance), 3); // set to progressive
    intel_vvp_tpg_set_pattern(&(pContext->_instance), 0);
    intel_vvp_tpg_set_colors(&(pContext->_instance), 128, 0, 0); // set black background
    intel_vvp_tpg_commit_writes(&(pContext->_instance));
}

/**
 * @brief Enables or disables the TPG output.
 *
 * This function controls the TPG output based on the specified enable flag.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] bEnable Boolean flag to enable or disable the TPG output.
 */
void tpgutils_enable_output(TpgContext *pContext, const bool bEnable)
{
    if (pContext == NULL)
    {
        logPrint(kDebug, "tpg_enable_output: Invalid pointer");
        return;
    }
    logPrint(kInfo, "\t%s Tpg Output\n", bEnable ? "Enable" : "Disable");
    if (bEnable)
    {
        intel_vvp_tpg_start(&(pContext->_instance));
    }
    else
    {
        intel_vvp_tpg_stop(&(pContext->_instance));
    }
}

#endif // !HDMI_RX_SUPPORTED & HDMI_TX_SUPPORTED