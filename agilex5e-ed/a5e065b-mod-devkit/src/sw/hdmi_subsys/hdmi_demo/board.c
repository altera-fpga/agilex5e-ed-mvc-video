#include <stdio.h>
#include <io.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "alt_types.h"
#include "intel_hdmi_common_i2c.h"
#include "config.h"
#include "system.h"
#include "utils.h"

// Deinition for the local logging message
#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Brd", format, ##__VA_ARGS__))

intel_hdmi_common_i2c_instance_t hdmi_clocks_i2c_instance;
intel_hdmi_common_i2c_instance_t hdmi_max10_i2c_instance;
intel_hdmi_common_i2c_instance_t hdmi_redriver_i2c_instance;
int tmds1204_state = 0;

//==================================================================
// get the board name
//==================================================================
char *get_board_name(uint32_t board)
{
  switch (board)
  {
  case BOARD_AGI_FM87:
    return "FM87";
    break;
  case BOARD_A5E_MDK:
    return "AGX5 MDK";
    break;
  case BOARD_A5E_PDK:
    return "AGX5 PDK";
    break;
  default:
    return "Unknown";
  }
}

char *get_connector_name(uint32_t connector)
{
  switch (connector)
  {
  case BOARD_CONNECTOR_FMC9:
    return "FMC 9";
    break;
  case BOARD_CONNECTOR_ONBOARD:
    return "ONBOARD";
    break;
  default:
    return "Unknown";
    break;
  }
}

#if (BOARD_NAME == BOARD_NO_DEVKIT)
//==================================================================
// Stubs for no devkit option
//==================================================================
void start_systempll(void) {}
void board_init(void) {}

void board_set_txclk(double freq, int tbcr) {}
void tmds1204_monitor(int rx_5v, int rx_tbcr) {}
#else

//==================================================================
//
//==================================================================
void tmds1204_wr(intel_hdmi_common_i2c_instance_t *i2c, unsigned char reg, unsigned char val)
{

  intel_hdmi_common_i2c_write_extended(i2c, 0x5b, reg, val);
}

//==================================================================
//
//==================================================================
void tmds1204_monitor(int rx_5v, int rx_tbcr)
{
  if (!tmds1204_state && rx_5v) {
    logPrint(kDebug, "TMDS1204 Powering up\n");

    // Reset TMDS1204
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x09, 0x80);

    // Fanout Enable + Snoop Enabled
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x0a, 0x40);

    // Slew for upto 3G and 6G
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x0b, 0x22);

    // Slew for upto 12g + clock lane
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x0c, 0x22);

    // Limited mode + AC coupling + 0dB Global CTLE Gain + No Terminations between TMDS and FPGA
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x0d, 0x60);

    // Auto CTLE in each mode based on Snoop
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x0e, 0x2a);

    // EQ
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x13, 0x00);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x15, 0x03);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x17, 0x03);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x19, 0x03);

    // VOD and TXFFE
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x12, 0x03);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x14, 0x03);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x16, 0x03);
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x18, 0x03);

    // Power up device, never power down on HPD
    tmds1204_wr(&hdmi_redriver_i2c_instance, 0x09, 0x04);
    tmds1204_state = 1;
  }

}

//==================================================================
//
//==================================================================
int set_si54x(intel_hdmi_common_i2c_instance_t *i2c, const int i2c_addr, const double Output_Freq, const int is546, const char SpeedGrade)
{

  double Fvco_min;  // Fvco Min per Table 5.3
  double Xtal_freq; // Xtal_Freq per Table 5.3
  double Fout_min;  // Minimum output frequency
  double Fout_max;  // Maximum output frequency

  // Device divider limits (see Tables 5.1 & 5.2)
  int HSDIV_UpperLimit = 2046;
  int HSDIV_LowerLimit_Odd = 5;  // min count for odd HSDIV divisor
  int HSDIV_UpperLimit_Odd = 33; // max count for odd HSDIV divisor

  // Working variables
  double Min_HSLS_Div;
  double LSDIV_Div; // actual LSDIV divide ratio
  int LSDIV_Reg;    // LSDIV as encoded in power of 2 for device register use
  double HSDIV;
  uint32_t HSDIV_Int;
  double FBDIV;
  double Fvco;
  uint32_t FBDIV_Int;
  uint32_t FBDIV_Frac;
  uint32_t Reg23 = 0; // HSDIV[7:0]
  uint32_t Reg24 = 0; // OD_LSDIV[2:0],HSDIV[10:8] (*2^4,/2^8)
  uint32_t Reg26 = 0; // FBDIV[7:0]
  uint32_t Reg27 = 0; // FBDIV[15:8] (/2^8)
  uint32_t Reg28 = 0; // FBDIV[23:16] (/2^16)
  uint32_t Reg29 = 0; // FBDIV[31:24] (/2^24)
  uint32_t Reg30 = 0; // FBDIV[39:32] (/2^32)
  uint32_t Reg31 = 0; // FBDIV[42:40] (/2^40)

  int ReturnCode = 0;
  Xtal_freq = 152600000.0;
  if (SpeedGrade == 'A')
  {
    Fvco_min = 10800000000.0;
    Fout_min = 200000.0;
    Fout_max = 1500000000.0;
    if ((Output_Freq < Fout_min) || (Output_Freq > Fout_max))
    {
      ReturnCode = -1;
    }
  }
  else if (SpeedGrade == 'B')
  {
    Fvco_min = 10800000000.0;
    Fout_min = 200000.0;
    Fout_max = 800000000.0;
    if ((Output_Freq < Fout_min) || (Output_Freq > Fout_max))
    {
      ReturnCode = -1;
    }
  }
  else if (SpeedGrade == 'C')
  {
    Fvco_min = 10800000000.0;
    Fout_min = 200000.0;
    Fout_max = 325000000.0;
    if ((Output_Freq < Fout_min) || (Output_Freq > Fout_max))
    {
      ReturnCode = -1;
    }
  }
  else
  {
    ReturnCode = -1;
  }

  // Set device limits based on device type and speed grade.
  // (Checks if desired output frequency is valid based on device and speed grade)
  if (ReturnCode == 0)
  {
    // If limits are set and output frequency is valid, calculate frequency plan...
    //***********************************************************************************************
    // Step 1: Find theoretical HSDIV *LSDIV value based on lowest valid VCO frequency...
    // (Assumes "Output_Freq" has been tested and is in valid range for the device grade according to Table 5.3)
    Min_HSLS_Div = Fvco_min / Output_Freq; // Floating point HS*LS div value. Remember to first bounds check Output_Freq!
    // Step 2: Find LSDIV divisor value given Min_HSLS_Div value
    LSDIV_Div = ceil(Min_HSLS_Div / HSDIV_UpperLimit); // Divisor value of LSDIV, NOT yet encoded as power of 2
    if (LSDIV_Div > 32)
      LSDIV_Div = 32; // clip at 32 (max LSDIV divisor)

    // Encode LSDIV divisor value into next nearest 'power of 2' value if not already. This will be LSDIV_Reg
    LSDIV_Reg = ceil(log(LSDIV_Div) / log(2)); // LSDIV_Reg now encoded as proper power of 2. Will range from 0 to 5.
    // Adjust LSDIV_Div (holder of divisor) based on rounded power of 2 value in LSDIV_Reg
    LSDIV_Div = pow(2, LSDIV_Reg); // LSDIV_Div divisor now synchronized to actual LSDIV_Reg.

    // Step 3: Find HSDIV divisor value using known LSDIV divisor
    HSDIV = ceil(Min_HSLS_Div / LSDIV_Div);
    if ((HSDIV >= HSDIV_LowerLimit_Odd) && (HSDIV <= HSDIV_UpperLimit_Odd))
    {
      HSDIV = HSDIV; // Leaves HSDIV as even or odd only if HSDIV is from 5 to 33.
    }
    else
    {
      // Round to nearest even.
      HSDIV = round(HSDIV / 2) * 2;
    }

    // Step 4: Now calculate Fvco and FBDIV
    Fvco = (HSDIV * LSDIV_Div * Output_Freq); // Calculate Fvco based on valid HSDIV, LSDIV, and Fout
    FBDIV = Fvco / Xtal_freq;                 // Finally, calculate FBDIV based on xtal freq
    // Calculate 11.32 fixed point FBDIV value (MCTL_M)
    // Extract Integer part
    FBDIV_Int = (uint32_t)FBDIV;
    HSDIV_Int = (uint32_t)HSDIV;
    // Extract fractional part
    FBDIV = (FBDIV - FBDIV_Int);
    FBDIV = FBDIV * pow(2, 32);
    FBDIV_Frac = (uint32_t)FBDIV;

    // Generate Register values based on LSDIV, HSDIV, and FBDIV (MCTL_M)
    Reg23 = (HSDIV_Int & 0xFF);
    Reg24 = ((HSDIV_Int >> 8) & 0x7) | ((LSDIV_Reg & 0x7) << 4);
    Reg26 = (FBDIV_Frac & 0xFF);
    Reg27 = (FBDIV_Frac >> 8) & 0xFF;
    Reg28 = (FBDIV_Frac >> 16) & 0xFF;
    Reg29 = (FBDIV_Frac >> 24) & 0xFF;
    Reg30 = (FBDIV_Int) & 0xFF;
    Reg31 = (FBDIV_Int >> 8) & 0x7;
    //logPrint(kDebug, "Programing SI548 : %02X %02X %02X %02X %02X %02X %02X %02X\n",Reg23,Reg24,Reg26,Reg27,Reg28,Reg29,Reg30,Reg31);
    //logPrint(kDebug, "SI : Addr=%02X Ver=%d\n", i2c_addr, intel_hdmi_common_i2c_read_extended(i2c, i2c_addr, 0));
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 255, 0);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 69, 0);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 17, 0);

    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 23, Reg23);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 24, Reg24);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 26, Reg26);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 27, Reg27);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 28, Reg28);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 29, Reg29);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 30, Reg30);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 31, Reg31);

    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 7, 8);
    // Wait 50mS for SI548 to reboot...
    usleep(50000);
    intel_hdmi_common_i2c_write_extended(i2c, i2c_addr, 17, 1);

    return 0;
  }

  logPrint(kDebug, "*** Device invalid or Device limits exceeded. Frequency plan not calculated.\n");
  return -1;
}

//==================================================================
// Make Sure System PLL is running at 100MHz before starting it.
//==================================================================
void start_systempll(void) {

  unsigned int syspll_status;
  
  syspll_status = IORD(PIO_SYSTEMPLL_BASE,0);
  if (!syspll_status) {
    logPrint(kDebug, "System PLL Starting\n");   
#if (BOARD_NAME == BOARD_A5E_PDK)
    // SI569 SDI Bank 4B Global - Reconfigure to 100MHz.
    set_si54x(&hdmi_clocks_i2c_instance, 0x55, 100000000, 0, 'B');
#endif
    // 50 ms To ensure Clocks are stable.
    usleep(50000);
    // Start System-PLL.
    IOWR(PIO_SYSTEMPLL_BASE,0,1);
    
    usleep(50000);    
  } else {
    logPrint(kDebug, "System PLL Reference clock already initialised\n");   
  }
}

//==================================================================
// Generic Routine to set TX Phy clock frequency (TMDS Freq)
//==================================================================
void board_set_txclk(double freq, int tbcr)
{
  // PDK is fitted with a -B part.
  char SpeedGrade = 'B'; // Can only be 'A' or 'B' or 'C'

#if (BOARD_NAME == BOARD_A5E_MDK)
#if (BOARD_CONNECTOR == BOARD_CONNECTOR_ONBOARD)
  // HDMI VCXO SI569
  set_si54x(&hdmi_clocks_i2c_instance, 0x52, tbcr ? freq/4.0 : freq, 1, SpeedGrade);
#else
  // SDI VCXO SI569
  set_si54x(&hdmi_clocks_i2c_instance, 0x55, tbcr ? freq/4.0 : freq, 0, SpeedGrade);
#endif
#else
  // SI548 - DP Bank 4C
  set_si54x(&hdmi_clocks_i2c_instance, 0x5a, tbcr ? freq/4.0 : freq, 0, SpeedGrade);
#endif
  if (TX_MAX_FRL_RATE && BOARD_CONNECTOR != BOARD_CONNECTOR_ONBOARD) {
	  // FRL Pixel Clock Generator on PDK & MDK board comes from an video-pll as
      // There is no way to route TX TMDS RefClk into the fabric
      
	  // Calculate the synthetic pixel clock and update the clock-divider.
      // 1.0001 is slightly faster than needed to ensure that in TMDS mode, the data being
      // consumed by the HDMI TX can keep up with the phy. The TMDS core will throttle 
      // incoming video to match TMDS CLK, but in FRL the link will always consume with
      // no pushback.
	  float div_float = (((freq*1.00001)) / 300000000.0)*65536.0;
	  uint32_t div = (uint32_t)(ceil(div_float));
	  // Read-Mod-Write the debug PIO that drives the Clock-Generator = Bits [23:8]
	  uint32_t reg = IORD(DEBUG_SIGNAL_BASE,0);
	  reg &= 0xFF0000FF;
	  reg |= (div << 8);
	  IOWR(DEBUG_SIGNAL_BASE,0,reg);
  }
}

#if (BOARD_CONNECTOR == BOARD_CONNECTOR_ONBOARD)
//==================================================================
// Initialise Board specific features for an Onboard Connector
//==================================================================
void board_init(void)
{
  // Board I2C is used to control clock-muxes
  intel_hdmi_common_i2c_init(&hdmi_max10_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_MAX10_BASE), 100000000);

  // Board I2C is used to control clocks
  intel_hdmi_common_i2c_init(&hdmi_clocks_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_CLOCKS_BASE), 100000000);

  // Board I2C is used to control clocks
  intel_hdmi_common_i2c_init(&hdmi_redriver_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_HDMI_BASE), 100000000);
  tmds1204_state = 0;

  // Instruct the MAX10 to route the SI569 to the TX Refclock pin
  unsigned char bmc_reg[2];
  unsigned char bmc_data[4];

  bmc_reg[1] = 0x10 << 2;
  bmc_reg[0] = 0;
  intel_hdmi_common_i2c_burst_read16_extended(&hdmi_max10_i2c_instance, 0x55, bmc_reg, 4, bmc_data);
  //logPrint(kDebug, "Max 10 : %02x\n", bmc_data[0]);
  bmc_data[0] |= (1 << 0); // Route SDI SI569 into Quad (Tx Refclk)
  bmc_data[0] |= (1 << 4); // Route HDMI SI569 into Quad (Tx Refclk)
  bmc_data[0] |= (1 << 5); // Specific for MVC: Route FMC RX into Quad  (Rx Refclk)
  bmc_data[0] |= (1 << 6); // Route HDMI SI569 into Fabric (clk_vid_tx)    
  intel_hdmi_common_i2c_burst_write16_extended(&hdmi_max10_i2c_instance, 0x55, bmc_reg, 4, bmc_data);

  // Specific for MVC
  // Here SDI VCXO SI569 is reconfigured to 148.5MHz.
  set_si54x(&hdmi_clocks_i2c_instance, 0x55, 148500000, 0, 'B');
  // 50 ms To ensure Clocks are stable.
  usleep(50000);
  
  start_systempll();
}
#endif

#if (BOARD_CONNECTOR == BOARD_CONNECTOR_FMC9)
static void redriver_init(intel_hdmi_common_i2c_instance_t *instance)
{
  unsigned char RDEXT = 0x08; // LEVEL 1
  unsigned char EQx = 0x04;   // LEVEL 1
  unsigned char VOD = 0xa8 + 7;
  unsigned char VOD_DB = 0x00;
  unsigned char SD_TH = 0x00;

  uint32_t addr = 0xCE; // TX

  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x06, 0x18); // - SLAVE REGISTER CONTROL
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x08, 0x08); // - SET Override RDEXT

  // Read Device ID
  logPrint(kDebug, "DS125BR820 TX = %02X\n", intel_hdmi_common_i2c_read_extended(instance, addr >> 1, 0x51));

  // Channel B
  int32_t i = 14;
  while (i < 43)
  {
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, RDEXT);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, EQx);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, VOD);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, VOD_DB);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, SD_TH);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, 0); // RESERVED = 00
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, 0); // RESERVED = 00
  }

  // Channel A
  i = 0x2b;
  while (i < 0x45)
  {
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, RDEXT);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, EQx);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, VOD);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, VOD_DB);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, SD_TH);
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, 0); // RESERVED = 00
    intel_hdmi_common_i2c_write_extended(instance, addr >> 1, i++, 0); // RESERVED = 00
  }

  RDEXT = 0x0C; // LEVEL 1
                //    EQx    = 0x07; // LEVEL 1
                //    VOD    = 0xa8+7;
  EQx = 0x00;
  VOD = 0xa8 + 5;
  VOD_DB = 0x01;
  SD_TH = 0x0C;
  addr = 0xCC;

  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x06, 0x18); // - SLAVE REGISTER CONTROL
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x08, 0x08); // - SET PWDN

  // Read Device ID
  logPrint(kDebug, "DS125BR820 RX = %02X\n", intel_hdmi_common_i2c_read_extended(instance, addr >> 1, 0x51));

  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x0e, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x0f, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x15, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x16, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x17, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x18, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x19, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x1C, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x1D, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x23, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x24, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x25, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x26, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x27, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x2B, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x2C, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x2D, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x2E, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x2F, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x34, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x35, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x36, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x39, RDEXT);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x3A, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x3B, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x3C, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x3D, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x41, EQx);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x42, VOD);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x43, VOD_DB);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x44, SD_TH);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x5E, 0x07);
  intel_hdmi_common_i2c_write_extended(instance, addr >> 1, 0x5f, 0xfc);
}

//==================================================================
// Initialise Board specific features for a Rev 9 FMC
//==================================================================
void board_init(void)
{

  // Board I2C is used to control clocks
  intel_hdmi_common_i2c_init(&hdmi_clocks_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_CLOCKS_BASE), 100000000);

  // Board I2C is used to control clocks
  intel_hdmi_common_i2c_init(&hdmi_redriver_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_HDMI_BASE), 100000000);

  redriver_init(&hdmi_redriver_i2c_instance);

#if (BOARD_NAME == BOARD_A5E_MDK)
  // Board I2C is used to control clock-muxes
  intel_hdmi_common_i2c_init(&hdmi_max10_i2c_instance, (intel_hdmi_common_i2c_base_t)(I2C_MAX10_BASE), 100000000);

  // Instruct the MAX10 on the MDK Carrier to route the FMC-Refclock for TMDS RX to the FPGA.
  unsigned char bmc_reg[2];
  unsigned char bmc_data[4];

  bmc_reg[1] = 0x10 << 2;
  bmc_reg[0] = 0;
  intel_hdmi_common_i2c_burst_read16_extended(&hdmi_max10_i2c_instance, 0x55, bmc_reg, 4, bmc_data);
  logPrint(kDebug, "Max 10 : %02x\n", bmc_data[0]);
  bmc_data[0] |= (1 << 0); // Route SDI SI569 into Quad (Tx Refclk)
  bmc_data[0] |= (1 << 4); // Route HDMI SI569 into Quad (Tx Refclk)
  bmc_data[0] |= (1 << 5); // Route FMC RX into Quad  (Rx Refclk)
  bmc_data[0] |= (1 << 6); // Route HDMI SI569 into Fabric (clk_vid_tx)
  intel_hdmi_common_i2c_burst_write16_extended(&hdmi_max10_i2c_instance, 0x55, bmc_reg, 4, bmc_data);
#endif

  start_systempll();  
}
#endif
#endif
