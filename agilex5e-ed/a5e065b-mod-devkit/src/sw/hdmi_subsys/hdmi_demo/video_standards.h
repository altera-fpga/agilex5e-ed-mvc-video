#ifndef _VIDEO_STANDARDS_H
#define _VIDEO_STANDARDS_H
#include <unistd.h>
#include "intel_hdmi21_tx_cvo.h"
#include "intel_hdmi21_rx_cvi.h"
#include "intel_hdmi_common_video_types.h"

/**
 * @brief Enumeration representing various video standards.
 */
typedef enum _eVideoStandards
{
    kCVO_NULL_MODE,         /**< No Mode */
    kCVO_4320P30_MODE,      /**< 4320p mode. */
    kCVO_4320P60_MODE,      /**< 4320p mode. */
    kCVO_4320P_420_MODE,    /**< 4320p 420 mode. */
    kCVO_3840x2160P60_MODE, /**< 3840x2160p60 mode. */
    kCVO_3840x2160P50_MODE, /**< 3840x2160p50 mode. */
    kCVO_3840x2160P30_MODE, /**< 3840x2160p30 mode. */
    kCVO_3840x2160P24_MODE, /**< 3840x2160p24 mode. */
    kCVO_2160P_420_MODE,    /**< 2160p 420 mode. */
    kCVO_1920x1080P60_MODE, /**< 1920x1080p60 mode. */
    kCVO_1920x1080P50_MODE, /**< 1920x1080p50 mode. */
    kCVO_1920x1080P30_MODE, /**< 1920x1080p30 mode. */
    kCVO_1920x1080P24_MODE, /**< 1920x1080p24 mode. */
    kCVO_1920x1080I60_MODE, /**< 1920x1080i60 mode. */
    kCVO_1080P_420_MODE,    /**< 1080p 420 mode. */
    kCVO_1280x720P60_MODE,  /**< 1280x720p60 mode. */
    kCVO_1280x720P50_MODE,  /**< 1280x720p50 mode. */
    kCVO_720P_420_MODE,     /**< 720p 420 mode. */
    kCVO_1024x768P60_MODE,  /**< 1024x768p60 mode. */
    kCVO_800x600P75_MODE,   /**< 800x600p75 mode. */
    kCVO_800x600P60_MODE,   /**< 800x600p60 mode. */
    kCVO_720x480P60_MODE,   /**< 720x480p60 mode. */
    kCVO_480P_420_MODE,     /**< 480p 420 mode. */
    kCVO_640x480P60_MODE,   /**< 640x480p60 mode. */
    kCVO_1080I_SDI_MODE,    /**< 1080i SDI mode. */
    kCVO_MAX_MODE           /**< Maximum mode. */
} eVideoStandards;

#define AVI_PACKET_SIZE (15)

#pragma pack(push, 4)

/**
 * @brief Structure representing image details.
 */
typedef struct _image_details_t
{
    intel_hdmi_standard_t _image_config; /**< General video standard information. */
    uint8_t _vic;                        /**< Video Identification Code. */
} image_details_t;

/**
 * @brief Structure representing HDMI TX video standard.
 */
typedef struct _hdmi_tx_standard_t
{
    eVideoStandards _video_enum;
    uint8_t _vic;                        /**< Video Identification Code. */
    float _frame_rate; /**< Frame rate of the video standard. */
    cvo_params_t _cvo_params; /**< CVO parameters for the video standard. */
} hdmi_tx_standard_t;

/**
 * @brief Array of predefined HDMI TX video standards.
 */
extern hdmi_tx_standard_t _gvideo_standards[kCVO_MAX_MODE];

/**
 * @brief Structure representing a video standard.
 */
typedef struct _video_standard_t
{
    uint32_t _avi_pckt_data[AVI_PACKET_SIZE]; /**< AVI packet data. */
    const hdmi_tx_standard_t *_video_timing; /**< TX video standard, NULL if not available for the TX. */
    image_details_t _details; /**< Basic image information. */
} video_standard_t;

/**
 * @brief Compares two image details structures.
 *
 * @param[in] a Pointer to the first image details structure.
 * @param[in] b Pointer to the second image details structure.
 * @return True if the structures are equal, false otherwise.
 */
bool compare_image_details(const image_details_t *a, const image_details_t *b);

/**
 * @brief Retrieves the video standard based on the VIC code.
 *
 * @param[in] vic_code The VIC code for the video standard.
 * @param[out] pStandard Pointer to the video standard structure to be updated.
 * @param[in] color_space The color space of the video standard.
 * @param[in] color_depth The color depth of the video standard.
 * @param[in] full_range The full range flag for the video standard.
 * @return True if the video standard is successfully retrieved, false otherwise.
 */
bool get_video_standard(eVideoStandards video_enum, video_standard_t *pStandard,
                        const intel_hdmi_color_space_t color_space, const intel_hdmi_color_depth_t color_depth, const uint8_t full_range);

/**
 * @brief Calculates TMDS settings for a given video standard.
 *
 * @param[in] pStandard Pointer to the video standard.
 * @param[in] color_depth The color depth of the video standard.
 * @param[in] color_space The color space of the video standard.
 * @param[out] pTmdsFreq Pointer to store the calculated TMDS frequency.
 * @param[out] pTmdsRatio Pointer to store the calculated TMDS ratio.
 */
void get_tmds_settings(const video_standard_t *pStandard, const intel_hdmi_color_depth_t color_depth, intel_hdmi_color_space_t color_space,
                       uint32_t *pTmdsFreq, intel_hdmi_tmds_ratio_t *pTmdsRatio);
					   
/**
 * @brief Calculates FRL settings for a given video standard.
 *
 * @param[in] pStandard Pointer to the video standard.
 * @param[in] color_depth The color depth of the video standard.
 * @param[in] color_space The color space of the video standard.
 * @param[out] pPixelFreq Pointer to store the calculated TMDS frequency.
 * @param[out] pTmdsRatio Pointer to store the calculated TMDS ratio.
 */
void get_frl_settings(const video_standard_t *pStandard, const intel_hdmi_color_depth_t color_depth, intel_hdmi_color_space_t color_space,
                       uint32_t *pPixelFreq, intel_hdmi_tmds_ratio_t *pTmdsRatio);

/**
 * @brief Prints the details of a video standard.
 *
 * @param[in] pStandard Pointer to the video standard to print.
 * @param[in] bDetailed Bool indicating that detail information should be displayed
 */
extern void print_video_standard(video_standard_t *pStandard, bool bDetailed);

#pragma pack(pop)

#endif // _VIDEO_STANDARDS_H