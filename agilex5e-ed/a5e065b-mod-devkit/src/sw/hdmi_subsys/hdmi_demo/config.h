
// Added by Richard Davies
#include <stdio.h>
#define UNUSED(x) (void)(x)

#define DEBUG_MODE 1
#define VERBOSE_PRINTF if (DEBUG_MODE) printf

#define HDMI_SUPPORT_HDCP1X          0 // Set to 1 to enable Intel HDCP1X
#define HDMI_SUPPORT_HDCP2X          0 // Set to 1 to enable Intel HDCP2X
#define HDMI_SUPPORT_FRL             1
#define HDMI_SUPPORT_HDCP_KEY_MANAGE 0

//Define HDCP I2C Base Addresses
#define HDCP_I2C_MASTER_BASE_ADDR HDMI_I2C_MASTER_BASE
#define HDCP_I2C_MASTER_TI_BASE_ADDR REDRIVER_I2C_MASTER_BASE

// Supported Boards
#define BOARD_NO_DEVKIT 0
#define BOARD_AGI_FM87  1
#define BOARD_A5E_MDK   2
#define BOARD_A5E_PDK   3

// Supported FMCs
#define BOARD_CONNECTOR_NONE    0
#define BOARD_CONNECTOR_FMC9    1
#define BOARD_CONNECTOR_ONBOARD 2

// Target Configuration
#define BOARD_NAME       BOARD_A5E_MDK
#define BOARD_CONNECTOR  BOARD_CONNECTOR_ONBOARD

#define CLOCKS_ARE_SAME 0

#define HDMI_TX_SUPPORTED 1
#define HDMI_RX_SUPPORTED 1
#define HDMI_PASS_THROUGH (HDMI_TX_SUPPORTED & HDMI_RX_SUPPORTED)
#define HDMI_TX_ONLY      (HDMI_TX_SUPPORTED & !HDMI_RX_SUPPORTED)

#if NIOS_HDMI_TX_BASE
#define HDMI_TX_0_BASE      NIOS_HDMI_TX_BASE
#endif
#if NIOS_HDMI_TX_PHY_BASE
#define HDMI_TX_0_PHY_BASE  NIOS_HDMI_TX_PHY_BASE
#endif

#if NIOS_HDMI_RX_BASE
#define HDMI_RX_0_BASE      NIOS_HDMI_RX_BASE
#endif
#if NIOS_HDMI_RX_PHY_BASE
#define HDMI_RX_0_PHY_BASE  NIOS_HDMI_RX_PHY_BASE
#endif

#define PIXEL_IN_PARALLEL 2
#define LOOPBACK_MODE 0
#define RX_MAX_FRL_RATE 0
#define TX_MAX_FRL_RATE 0

#define DEBUG_MODE 1
#define NUM_EDID_PAGES 2


