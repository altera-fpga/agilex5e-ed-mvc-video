#ifndef _TPG_UTILS_H_
#define _TPG_UTILS_H_

#include "config.h"

// Enable when in TX Only mode
#if !HDMI_RX_SUPPORTED & HDMI_TX_SUPPORTED
#include "intel_vvp_tpg.h"
#include "video_standards.h"

/**
 * @brief Enumeration representing the state of the TPG.
 */
typedef enum
{
    kTpg_Idle,        /**< Indicates that the TPG device is setup but hasn't been assigned a standard. */
    kTpg_NewStandard, /**< A new standard has been defined for the context. */
    kTpg_Enabled,     /**< TPG has programmed the standard. */
} eTpgState;

/**
 * @brief Structure representing the TPG context.
 */
typedef struct _HdmiRxContext
{
    intel_vvp_tpg_instance _instance; /**< Holds the initialized TPG instance. */
    eTpgState _state;                /**< Current state of the TPG instance. */
} TpgContext;

/**
 * @brief Initializes the TPG context.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] base Base address for the TPG.
 * @return True if the context is successfully initialized, false otherwise.
 */
extern bool tpgutils_init_context(TpgContext *pContext, intel_vvp_core_base base);

/**
 * @brief Sets the video standard for the TPG context.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] pStandard Pointer to the video standard structure.
 */
extern void tpgutils_set_standard(TpgContext *pContext, const video_standard_t *pStandard);

/**
 * @brief Enables or disables the TPG output.
 *
 * @param[in] pContext Pointer to the TPG context.
 * @param[in] bEnable Boolean flag to enable or disable the TPG output.
 */
extern void tpgutils_enable_output(TpgContext *pContext, const bool bEnable);

#endif // !HDMI_RX_SUPPORTED & HDMI_TX_SUPPORTED

#endif // _TPG_UTILS_H_