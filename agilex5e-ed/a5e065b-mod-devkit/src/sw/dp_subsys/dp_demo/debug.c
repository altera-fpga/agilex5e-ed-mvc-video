// ********************************************************************************
//
// Copyright 2001-2021 Intel Corporation
// This software and the related documents are Intel copyrighted materials,
// and your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise,
// you may not use, modify, copy, publish, distribute, disclose or transmit
// this software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express or
// implied warranties, other than those that are expressly stated in the License.
//
// ********************************************************************************
// DisplayPort Core test code debug routines
//
// Description:
//
// ********************************************************************************
#include "debug.h"

#if BITEC_STATUS_DEBUG || BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG


#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DBG_PRINTF printf
#else
#define DBG_PRINTF(format, args...) ((void)0)
#endif


#include <system.h>
#include "sys/alt_timestamp.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <system.h>
#include <unistd.h>

#include "aux_decoder.h"


#if DP_SUPPORT_TX_DSC
#include "dsc_utils.h"
#endif


// Get the core capabilities (defined in QSYS and ported to system.h)
#if DP_SUPPORT_RX
#include "btc_dprx_syslib.h"
#if DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_RX_ONLY
#define RX_MAX_LINK_RATE      DP_RX_DP_SINK_RX_MGMT_BITEC_CFG_RX_MAX_LINK_RATE
#define RX_MAX_LANE_COUNT     DP_RX_DP_SINK_RX_MGMT_BITEC_CFG_RX_MAX_LANE_COUNT
#else
#define RX_MAX_LINK_RATE      DP_RX_DP_SINK_BITEC_CFG_RX_MAX_LINK_RATE
#define RX_MAX_LANE_COUNT     DP_RX_DP_SINK_BITEC_CFG_RX_MAX_LANE_COUNT
#endif
#endif

#if DP_SUPPORT_TX
#include "btc_dptx_syslib.h"
#include "tx_utils.h"
#if BITEC_TX_CAPAB_MST_OR_DP20
#include "btc_dptxll_syslib.h"
#endif
#if DP_SUPPORT_HDCP_KEY_MANAGE || DP_DSC_TX_ONLY
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LANE_COUNT
#else
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LANE_COUNT
#endif
#endif

#if DP_DSC_TX_ONLY
//Arrays to hold mode and pps data (from main.c)
extern struct mode_parameters mode_param_array[20];
extern struct pps_parameters  dsc_data_pps_parameters;
extern int mode_command;

#endif


//==================================================================
// A few global variables used to hanlde menu actions
//==================================================================
// Global buffer to read user inputs
char btc_stdbuf[20];
BYTE btc_stdbuf_ptr = 0;

#if BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX
char auxRxDebugEnable;
static AuxDecoderInstance _gRxAuxInstance;
#endif
#if BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX
char auxTxDebugEnable;
static AuxDecoderInstance _gTxAuxInstance;
#endif
#if DP_SUPPORT_TX
unsigned int lt_tx_link_rate;
unsigned int lt_tx_lane_count;
#endif

//==================================================================
// Debug/UI initialization
//==================================================================
void debug_init()
{
#if BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX
    dp_dump_aux_debug_init(&_gRxAuxInstance, DP_RX_AUX_RX_DEBUG_FIFO_IN_CSR_BASE, DP_RX_AUX_RX_DEBUG_FIFO_OUT_BASE, true);
    auxRxDebugEnable=0;
#endif
#if BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX
    dp_dump_aux_debug_init(&_gTxAuxInstance, DP_TX_AUX_TX_DEBUG_FIFO_IN_CSR_BASE, DP_TX_AUX_TX_DEBUG_FIFO_OUT_BASE, false);
    auxTxDebugEnable=0;
#endif
#if DP_SUPPORT_TX
    lt_tx_link_rate = TX_MAX_LINK_RATE;
    lt_tx_lane_count = TX_MAX_LANE_COUNT;
#endif

}

//==================================================================
// Catch user input
//==================================================================
char* get_stdin()
{
    int d, i;
    char* c = (char*)&d;

    i = read(0, (void*)&d, 1);  // 0 = stdin
    if ((i < 1) || (*c == EOF))
        return NULL;

    printf("%c", d);
    if (*c == '\n' || *c == 0xd)
    {
        // input is complete
        if (*c == 0xd)
            printf("\n");
        btc_stdbuf[btc_stdbuf_ptr] = 0x00;
        btc_stdbuf_ptr = 0;
        return btc_stdbuf;
    }

    btc_stdbuf[btc_stdbuf_ptr++] = *c;
    if ((btc_stdbuf_ptr + 1) == sizeof(btc_stdbuf))
    {
        // buffer is full
        btc_stdbuf[btc_stdbuf_ptr] = 0x00;
        btc_stdbuf_ptr = 0;
        return btc_stdbuf;
    }

    return NULL;
}


//==================================================================
// Printout all menu commands
//==================================================================
void menu_print()
{
    printf("h = Help\n");
#if BITEC_STATUS_DEBUG
    printf("q  = Hardware setup\n");
    printf("s  = Status\n");  // 's' will give the full status. 'sr', 'st' for Rx-only or Tx-only, 'srv', 'stv' for Rx-only or Tx-only debug info
#endif
#if DP_SUPPORT_TX
    printf("c   = Read sink CRC (through DPCD)\n");
    printf("t+  = Increase TX max link rate (requires retrain)\n");
    printf("t-  = Decrease TX max link rate (requires retrain)\n");
    printf("x+  = Increase TX max num lanes (requires retrain)\n");
    printf("x-  = Decrease TX max num lanes (requires retrain)\n");
#if DP_TX_SUPPORT_AUTOMATED_TEST
    printf("p?? = Send phy test pattern, at 8b10b:    00 = None,  03 = PRBS7 DP1.4, 01 = D10.2, 02 = Symbol Error Measurement,\n");
    printf("                                          (DP1.2+ Rx only) 04 = 80-bit custom (using sink DPCD registers),\n");
    printf("                                          (DP1.2+ Rx only) 05 = CP2520 1, 06 = CP2520 2. 07 = TPS4\n");
    printf("                             at 128b123b: 03 = PRBS7 DP2.0, 08 = TPS1,  10 = TPS2, 18 = PRBS9,\n");
    printf("                                          20 = PRBS11, 28 = PRBS15, 30 = PRBS23, 38 = PRBS31,\n");
    printf("                                          40 = 264-bit custom (using sink DPCD registers),\n");
    printf("                                          48 = square sequence, use p48 [num-1] where num is number of consecutive 0 (or 1)\n");
#else
    printf("p?? = Send phy test pattern N/A, automation support not enabled\n");
#endif
    printf("y   = TX retrain\n");
#if (DP_SUPPORT_TX_DSC)
    printf("j = Display sink's DPCD capabilities\n");
    printf("dsc? = Check DSC support\n");
#if (DP_DSC_TX_ONLY)
    printf("dscp = Load DSC Tx Only config\n");
#endif
    printf("dsc+ = Enable DSC mode and set DSC DPCD\n");  // dscf for force
    printf("dsc- = Disable DSC mode and unset DSC DPCD\n");
#endif //(DP_SUPPORT_TX_DSC)
    printf("Dr [N] [start_addr]  = Read N sink DPCD register(s) from given start address (use hexadecimal start_addr)\n");
    printf("Dw [addr] [byte_val] = Write sink DPCD register (use hexadecimal addr and byte_val)\n");
#if BITEC_TX_AUX_DEBUG
    printf("a = Enable/Disable Tx AUX Log\n");
#endif
#endif // DP_SUPPORT_TX


#if DP_SUPPORT_RX
#if BITEC_RX_GPUMODE
    printf("l  = RX HPD\n");
    // Beware that this may end up being incompatible with our current EDID
    printf("r+ = Increase RX max link rate (requires HPD)\n");
    printf("r- = Decrease RX max link rate (requires HPD)\n");
    printf("n+ = Increase RX max num lanes (requires HPD)\n");
    printf("n- = Decrease RX max num lanes (requires HPD)\n");
    printf("Dc [N] [start_addr]  = Check N sink DPCD register(s) from given start address (use hexadecimal start_addr)\n");
    printf("Do [addr] [byte_val] = Change sink DPCD register (use hexadecimal addr and byte_val)\n");
#endif // BITEC_RX_GPUMODE

#if BITEC_RX_AUX_DEBUG
    printf("d = Enable/Disable Rx AUX Log\n");
#endif // BITEC_RX_AUX_DEBUG
#endif // DP_SUPPORT_RX

}


//==================================================================
// Implementation of menu commands
//==================================================================
void menu_cmd()
{
    char* cmd;

    cmd = get_stdin();
    if (cmd != NULL)
    {
        // Clear the non-blocking flag while handling and responding to a command
        int flags = 0;
        flags = fcntl(STDOUT_FILENO, F_GETFL);
        fcntl(STDOUT_FILENO, F_SETFL, flags & ~O_NONBLOCK);

        switch (cmd[0])
        {
            //***** Common Tx/Rx functions *****//
            // Status, dump MSA and config
            
#if BITEC_STATUS_DEBUG
            case 'q':
            {
              print_hw_config();
              break;
            }
            case 's':
            {
                if (cmd[1] != 'r')
                {
                    #if DP_SUPPORT_TX
                    if (cmd[1] && (cmd[2] == 'v'))
                    {
                        print_source_debug(btc_dptx_baseaddr(0));
                    }
                    else
                    {
                        print_source_msa(btc_dptx_baseaddr(0));
                        print_source_config(btc_dptx_baseaddr(0));
                    }
                    #endif
                }
                if (cmd[1] != 't')
                {
                    #if DP_SUPPORT_RX
                    if (cmd[1] && (cmd[2] == 'v'))
                    {
                        print_sink_debug(btc_dprx_baseaddr(0));
                    }
                    else
                    {
                        print_sink_msa(btc_dprx_baseaddr(0));
                        print_sink_config(btc_dprx_baseaddr(0));
                    }
                    #endif
                }
                break;
            }
#endif
            // DPCD register read/write
            case 'D':
            {
                // Tx or Rx read
                if ((DP_SUPPORT_TX && (cmd[1] == 'r')) || (DP_SUPPORT_RX && BITEC_RX_GPUMODE && (cmd[1] == 'c')))
                {
                    BYTE data[16];
                    char *str_end = NULL;
                    int err_code = 0;
                    unsigned int n = strtoul(cmd+2, &str_end, 0);
                    if ((n > 0) && (n <= 16) && str_end)
                    {
                        unsigned int start_addr = (unsigned int)strtoul(str_end, NULL, 16);
#if DP_SUPPORT_TX
                        if (cmd[1] == 'r')
                        {
                            err_code = btc_dptx_aux_read(0, start_addr, n, data);
                        }
#endif
#if DP_SUPPORT_RX && BITEC_RX_GPUMODE
                        if (cmd[1] == 'c')
                        {
                            btc_dprx_dpcd_gpu_access(0, 0, start_addr, n, data);
                        }
#endif
                        printf("'h%05X: ", start_addr);
                        switch (err_code)
                        {
                            case 0:
                            {
                                for (int i = 0; i < n; i++)
                                {
                                    if (i > 0) printf(":");
                                        printf("%02X", data[i]);

                                }
                                break;
                            }
                            case 2: printf("NO_REPLY"); break;
                            case 3: printf("AUX_NACK"); break;
                            case 4: printf("AUX_DEFER"); break;
                            case 5: printf("INVALID"); break;
                            default: printf("ERR"); break;
                        }
                        printf("\n");
                    }
                }
                // Tx or Rx write
                if ((DP_SUPPORT_TX && (cmd[1] == 'w')) || (DP_SUPPORT_RX && BITEC_RX_GPUMODE && (cmd[1] == 'o')))
                {
                    BYTE data_byte;
                    char *str_end = NULL;
                    int err_code = 0;
                    unsigned int addr = strtoul(cmd+2, &str_end, 16);
                    if ((addr <= 0xFFFFF) && str_end)
                    {
                        data_byte = strtoul(str_end, NULL, 16);
#if DP_SUPPORT_TX
                        if (cmd[1] == 'w')
                        {
                            err_code = btc_dptx_aux_write(0, addr, 1, &data_byte);
                        }
#endif
#if DP_SUPPORT_RX && BITEC_RX_GPUMODE
                        if (cmd[1] == 'o')
                        {
                            btc_dprx_dpcd_gpu_access(0, 1, addr, 1, &data_byte);
                        }
#endif
                        printf("'h%05X <- ", addr);
                        switch (err_code)
                        {
                            case 0: printf("0x%02X", data_byte); break;
                            case 2: printf("NO_REPLY"); break;
                            case 3: printf("AUX_NACK"); break;
                            case 4: printf("AUX_DEFER"); break;
                            case 5: printf("INVALID"); break;
                            default: printf("ERR"); break;
                        }
                        printf("\n");
                    }
                }
                break;
            }
            // Help
            case 'h':
            {
                menu_print();
                break;
            }
            //***** DSC or Rx aux debug functions *****//
            case 'd':
            {
#if DP_SUPPORT_RX && BITEC_RX_AUX_DEBUG
                if (cmd[1] == '\0')
                {
                    auxRxDebugEnable++;
                    if (auxRxDebugEnable&0x01) printf ("RX_AUX_DEBUG ENABLED\n");
                    else printf ("RX_AUX_DEBUG DISABLED\n");
                }
#endif   // DP_SUPPORT_RX && BITEC_RX_AUX_DEBUG

#if (DP_SUPPORT_TX_DSC)
                if ((cmd[1] == 's') && (cmd[2] == 'c'))
                {
                    if (cmd[3] == '?') {
                        BYTE dpcd_val[6];
                        BYTE sink_supports_Enhanced_Framing;
                        BYTE sink_supports_FEC;
                        BYTE sink_FEC_enabled;
                        BYTE sink_supports_DSC;

                        sink_supports_Enhanced_Framing  = 0;
                        sink_supports_FEC               = 0;
                        sink_FEC_enabled                = 0;
                        sink_supports_DSC               = 0;
                        // Check Sink is capable of enhanced framing, FEC and DSC before enabling

                        btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, dpcd_val);
                        sink_supports_Enhanced_Framing = (dpcd_val[0] & 0x80) >> 7;
                        usleep(50000);

                         // Check Sink FEC capability
                        btc_dptx_aux_read(0,DPCD_ADDR_FEC_CAPABILITY,1,dpcd_val);
                        sink_supports_FEC = (dpcd_val[0] & 0x01);
                        usleep(50000);

                         // Check Sink FEC is enabled
                        btc_dptx_aux_read(0,DPCD_ADDR_FEC_CONFIGURATION,1,dpcd_val);
                        sink_FEC_enabled = (dpcd_val[0] & 0x01);
                        usleep(50000);

                         // Check Sink suports DSC
                        btc_dptx_aux_read(0, DPCD_ADDR_DSC_SUPPORT, 1, dpcd_val);
                        sink_supports_DSC = (dpcd_val[0] & 0x01);
                        usleep(50000);


                        printf("Link rate  : %d Mbps\n",
                        ((IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);
                        printf("Lane count : %d\n", (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 5) & 0x1f);

                        if ( (sink_supports_Enhanced_Framing == 0x01) && (sink_supports_FEC == 0x01) && (sink_supports_DSC == 0x01)) {
                            printf("Sink Supports DSC  \n");
                            if (sink_FEC_enabled == 0x01) {
                                printf("Sink FEC Enabled  \n");
                            } else {
                                printf("Sink FEC not Enabled  \n");
                            }
                        } else {
                            printf("Sink Does Not Support DSC  \n");

                            if (sink_supports_Enhanced_Framing == 0x01) {
                                printf("Sink Supports Enhanced Framing  \n");
                            } else {
                                printf("Sink Does Not Support Enhanced Framing  \n");
                            }
                            if (sink_supports_FEC == 0x01) {
                                printf("Sink Supports FEC  \n");
                            } else {
                                printf("Sink Does Not Support FEC  \n");
                            }
                            if (sink_supports_DSC == 0x01) {
                                printf("Sink Supports DSC  \n");
                            } else {
                                printf("Sink Does Not Support DSC  \n");
                            }
                        }
                    } else {

                        // Enabling dsc
                        if (cmd[3] == '+') {
                            enable_dsc();
                        }
                        // Disabling dsc
                        if (cmd[3] == '-') {
                            disable_dsc();
                        }

                        #if DP_DSC_TX_ONLY
                        if (cmd[3] == 'p') {
                            int modes_found;
                            struct mode_parameters updated_mode_parameters = {
                               .frame_rate = 0,
                               .pixel_clk  = 0,
                               .HTotal     = 0,
                               .HActive    = 0,
                               .HBlank     = 0,
                               .HFront     = 0,
                               .HSync      = 0,
                               .VTotal     = 0,
                               .VActive    = 0,
                               .VBlank     = 0,
                               .VFront     = 0,
                               .VSync      = 0,
                               .mode_valid = 0
                             };

                            modes_found = 0;
                            read_pps_params( &dsc_data_pps_parameters);
                            load_resolutions (1, mode_param_array, &dsc_data_pps_parameters, modes_found);
                        }
                        if (cmd[3] == 's') {
                            select_resolutions(0,mode_param_array, &dsc_data_pps_parameters, &mode_command);
                        }
                        
                        // Catch dscXX where XX is a number in the range 1..19
                        if ((cmd[3] > '0') && (cmd[3] <= '9')) {
                            // Pull the first single digit 
                            mode_command = cmd[3] - '0';
                            // Get the second digit if it exists and is valid
                            if ((mode_command == 1) && (cmd[4] >= '0') && (cmd[4] <= '9'))
                            {
                                mode_command = 10 + cmd[4] - '0';
                            }
                            
                            struct mode_parameters new_mode_parameters = { .frame_rate = 0,
                                                           .pixel_clk  = 0,
                                                           .HTotal     = 0,
                                                           .HActive    = 0,
                                                           .HBlank     = 0,
                                                           .HFront     = 0,
                                                           .HSync      = 0,
                                                           .VTotal     = 0,
                                                           .VActive    = 0,
                                                           .VBlank     = 0,
                                                           .VFront     = 0,
                                                           .VSync      = 0,
                                                           .mode_valid = 0};
                            struct mode_parameters updated_mode_parameters = { .frame_rate = 0,
                                                           .pixel_clk  = 0,
                                                           .HTotal     = 0,
                                                           .HActive    = 0,
                                                           .HBlank     = 0,
                                                           .HFront     = 0,
                                                           .HSync      = 0,
                                                           .VTotal     = 0,
                                                           .VActive    = 0,
                                                           .VBlank     = 0,
                                                           .VFront     = 0,
                                                           .VSync      = 0,
                                                           .mode_valid = 0};
                            updated_mode_parameters = mode_param_array[mode_command];

                            DBG_PRINTF(" Mode is      %d %d \n", mode_command, updated_mode_parameters.mode_valid);
                            program_cvo_tpg (1, &updated_mode_parameters, &dsc_data_pps_parameters);
                            program_dsc_regs( &updated_mode_parameters, &dsc_data_pps_parameters);
                        }
                        #endif  //DP_DSC_TX_ONLY, dscp and dscs commands
                    } // end of handling dsc commands dsc+, dsc- dscp, dscs, dsc
                }
#endif   //(DP_SUPPORT_TX_DSC)
                break;
            }  // end of case 'd' (for debug or DSC)

#if DP_SUPPORT_TX
            //***** Tx only functions *****//
            // Put sink in test mode and read CRC
            case 'c':
            {
                BYTE data_byte;
                BYTE d[6];

                data_byte = 1;
                btc_dptx_aux_write(0, DPCD_ADDR_TEST_SINK, 1, &data_byte);
                usleep(50000);
                btc_dptx_aux_read(0, DPCD_ADDR_TEST_CRC_R_CR_LSB, 6, d);
                data_byte = 0;
                btc_dptx_aux_write(0, DPCD_ADDR_TEST_SINK, 1, &data_byte);
                printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n",
                               d[0] + (d[1] << 8), d[2] + (d[3] << 8), d[4] + (d[5] << 8));
                break;
            }
            // Change TX max num lanes
            case 'x':
            {
                switch (cmd[1])
                {
                    case '+': if (lt_tx_lane_count != TX_MAX_LANE_COUNT) lt_tx_lane_count = lt_tx_lane_count << 1; break;
                    case '-': if (lt_tx_lane_count != 1) lt_tx_lane_count = lt_tx_lane_count >> 1; break;
                    case '1': lt_tx_lane_count = 1; break;
                    case '2': if (TX_MAX_LANE_COUNT >= 2) lt_tx_lane_count = 2; break;
                    case '4': if (TX_MAX_LANE_COUNT >= 4) lt_tx_lane_count = 4; break;
                }
                printf("Set TX max lane count to %d\n", lt_tx_lane_count);
#if BITEC_TX_CAPAB_MST_OR_DP20
                btc_dptxll_syslib_set_max_lane_count(0, lt_tx_lane_count);
#else
                bitec_dptx_change_link_rate(lt_tx_link_rate, lt_tx_lane_count);
#endif
                break;
            }
            // Change TX max link rate
            case 't':
            {
                switch (cmd[1])
                {
                    case '+': {
                        if (lt_tx_link_rate != TX_MAX_LINK_RATE)
                        {
                            switch (lt_tx_link_rate)
                            {
                                case 0x06: lt_tx_link_rate = 0x0A; break;
                                case 0x0A: lt_tx_link_rate = 0x14; break;
                                case 0x14: lt_tx_link_rate = 0x1E; break;
                                case 0x1E: lt_tx_link_rate = 0x01; break;
                                case 0x01: lt_tx_link_rate = 0x04; break;
                                case 0x04: lt_tx_link_rate = 0x02; break;
                            }
                        }
                        break;
                    }
                    case '-': {
                        switch (lt_tx_link_rate)
                        {
                            case 0x0A: lt_tx_link_rate = 0x06; break;
                            case 0x14: lt_tx_link_rate = 0x0A; break;
                            case 0x1E: lt_tx_link_rate = 0x14; break;
                            case 0x01: lt_tx_link_rate = 0x1E; break;
                            case 0x04: lt_tx_link_rate = 0x01; break;
                            case 0x02: lt_tx_link_rate = 0x04; break;
                        }
                        break;
                    }
                    case '0': lt_tx_link_rate = 0x06; break;
                    case '1': if ((TX_MAX_LINK_RATE<=0x04) || (TX_MAX_LINK_RATE>=0x0A)) lt_tx_link_rate = 0x0A; break;
                    case '2': if ((TX_MAX_LINK_RATE<=0x04) || (TX_MAX_LINK_RATE>=0x14)) lt_tx_link_rate = 0x14; break;
                    case '3': if ((TX_MAX_LINK_RATE<=0x04) || (TX_MAX_LINK_RATE>=0x1E)) lt_tx_link_rate = 0x1E; break;
                    case '4': if (TX_MAX_LINK_RATE<=0x04) lt_tx_link_rate = 0x01; break;
                    case '5': if ((TX_MAX_LINK_RATE==0x02) || (TX_MAX_LINK_RATE==0x04)) lt_tx_link_rate = 0x04; break;
                    case '6': if (TX_MAX_LINK_RATE==0x02) lt_tx_link_rate = 0x02; break;
                }
                switch (lt_tx_link_rate)
                {
                    case 0x06: printf("Set TX max link rate to 1.62Gbps\n"); break;
                    case 0x0A: printf("Set TX max link rate to 2.7Gbps\n"); break;
                    case 0x14: printf("Set TX max link rate to 5.4Gbps\n"); break;
                    case 0x1E: printf("Set TX max link rate to 8.1Gbps\n"); break;
                    case 0x01: printf("Set TX max link rate to 10Gbps\n"); break;
                    case 0x04: printf("Set TX max link rate to 13.5Gbps\n"); break;
                    case 0x02: printf("Set TX max link rate to 20Gbps\n"); break;
                }
#if BITEC_TX_CAPAB_MST_OR_DP20
                btc_dptxll_syslib_set_max_link_rate(0, lt_tx_link_rate);
#else
                bitec_dptx_change_link_rate(lt_tx_link_rate, lt_tx_lane_count);
#endif
                break;
            }
            // PHY test patterns
#if DP_TX_SUPPORT_AUTOMATED_TEST
            case 'p':
            {
                BYTE chan_coding = (lt_tx_link_rate >= 0x06) ? 1 : 2;
                if (chan_coding == 1)
                {
                    // p00 is a valid pattern to return to normal video
                    if (cmd[1] == '0') {
                        BYTE pattern = cmd[2] - '0';
                        if ((pattern >= 0) && (pattern <= 7)) {
                            printf("Applying 8b10b pattern %d\n", pattern);
                            btc_dptx_phy_test_pattern(0, 1, pattern);
                        }
                    }
                }
                else
                {
                    // p00 is a valid pattern to return to normal video
                    if ((cmd[1] >= '0') && (cmd[1] <= '4') && ((cmd[2] == '0') || (cmd[2] == '8')))
                    {
                        BYTE pattern = (cmd[1] - '0') * 16 + (cmd[2] - '0');
                        if (pattern == 0x48) {
                            long value = strtol(cmd + 3, NULL, 10);
                            if (value >= 0 && value <= 255)
                            {
                                BYTE sq_value = value;
                                printf("Applying 128b132b square pattern %02d\n", (int)value + 1);
                                btc_dptx_aux_write(0,DPCD_ADDR_PHY_SQUARE_PATTERN_NUM, 1, &sq_value);
                            }
                        }
                        else
                        {
                            printf("Applying 128b132b pattern h%x\n", pattern);
                        }
                        btc_dptx_phy_test_pattern(0, 2, pattern);
                    }
                    else if ((cmd[1] == '0') && (cmd[2] == '3'))
                    {
                        btc_dptx_phy_test_pattern(0, 2, 0x03);
                    }
                }
                break;
            }
#endif
            // Restart TX link training
            case 'y':
            {
                pc_fsm = PC_FSM_HPD_1;
                break;
            }
            
#if DP_SUPPORT_TX_DSC
            case 'j':
            {
                dump_dsc_dpcd();
                break;
            }
#endif
#if BITEC_TX_AUX_DEBUG
            // Enable/disable TX aux debug
            case 'a':
            {
                auxTxDebugEnable++;
                if (auxTxDebugEnable&0x01) printf ("TX_AUX_DEBUG ENABLED\n");
                else printf ("TX_AUX_DEBUG DISABLED\n");
                break;
            }
#endif
#endif  // DP_SUPPORT_TX

#if DP_SUPPORT_RX && BITEC_RX_GPUMODE
            //***** Rx only functions *****//
            // HPD, 500 ms
            case 'l':
            {
                btc_dprx_hpd_set(0, 0);  // HPD = 0
                // Wait for 500 ms to have a long HPD
                {
                    alt_timestamp_start();
                    unsigned int timeout = alt_timestamp_freq() / 2;
                    while (alt_timestamp() < timeout) ; //delay
                }
                btc_dprx_hpd_set(0, 1);  // HPD = 1
                break;
            }
            // Update RX max num lanes in the DPCD registers
            case 'n':
            {
                BYTE d;
                unsigned int lt_rx_lane_count;

                btc_dprx_dpcd_gpu_access(0, 0, DPCD_ADDR_MAX_LANE_COUNT, 1, &d);
                lt_rx_lane_count = d & 7;
                switch (cmd[1])
                {
                    case '+': if (lt_rx_lane_count != RX_MAX_LANE_COUNT) lt_rx_lane_count = lt_rx_lane_count << 1; break;
                    case '-': if (lt_rx_lane_count != 1) lt_rx_lane_count = lt_rx_lane_count >> 1; break;
                    case '1': lt_rx_lane_count = 1; break;
                    case '2': if (RX_MAX_LANE_COUNT >= 2) lt_rx_lane_count = 2; break;
                    case '4': if (RX_MAX_LANE_COUNT >= 4) lt_rx_lane_count = 4; break;
                }
                printf("Set RX max num lanes to %d\n", lt_rx_lane_count);
                d = (d & ~0x1F) | lt_rx_lane_count;
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_MAX_LANE_COUNT, 1, &d);

                // Also update the extended capabilities, whether used or not
                btc_dprx_dpcd_gpu_access(0, 0, DPCD_ADDR_EXT_MAX_LANE_COUNT, 1, &d);
                d = (d & ~0x1F) | lt_rx_lane_count;
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_EXT_MAX_LANE_COUNT, 1, &d);
                break;
            }
            // Change RX max link rate
            case 'r':
            {
                BYTE link_cap[2];
                BYTE channel_coding;
                BYTE uhbr_supported_link_rates;
                unsigned int lt_rx_link_rate;

                btc_dprx_dpcd_gpu_access(0, 0, DPCD_ADDR_MAX_LINK_RATE, 2, link_cap);
                btc_dprx_dpcd_gpu_access(0, 0, DPCD_ADDR_MAIN_LINK_CHANNEL_CODING, 1, &channel_coding);
                // Supported 128b132b rates (extended cap table)
                btc_dprx_dpcd_gpu_access(0, 0, DPCD_ADDR_EXT_LINK_RATE, 1, &uhbr_supported_link_rates);
                if (channel_coding & 0x02)
                {
                    if (uhbr_supported_link_rates & 0x02)
                    {
                        lt_rx_link_rate = 0x02;
                    }
                    else
                    {
                        lt_rx_link_rate = (uhbr_supported_link_rates & 0x04) ? 0x04 : 0x01;
                    }
                }
                else
                {
                    lt_rx_link_rate = link_cap[0] & 0x1F;
                }
                switch (cmd[1])
                {
                    case '+': {
                        if (lt_rx_link_rate != RX_MAX_LINK_RATE)
                        {
                            switch (lt_rx_link_rate)
                            {
                                case 0x06: lt_rx_link_rate = 0x0A; break;
                                case 0x0A: lt_rx_link_rate = 0x14; break;
                                case 0x14: lt_rx_link_rate = 0x1E; break;
                                case 0x1E: lt_rx_link_rate = 0x01; break;
                                case 0x01: lt_rx_link_rate = 0x04; break;
                                case 0x04: lt_rx_link_rate = 0x02; break;
                            }
                        }
                        break;
                    }
                    case '-': {
                        switch (lt_rx_link_rate)
                        {
                            case 0x0A: lt_rx_link_rate = 0x06; break;
                            case 0x14: lt_rx_link_rate = 0x0A; break;
                            case 0x1E: lt_rx_link_rate = 0x14; break;
                            case 0x01: lt_rx_link_rate = 0x1E; break;
                            case 0x04: lt_rx_link_rate = 0x01; break;
                            case 0x02: lt_rx_link_rate = 0x04; break;
                        }
                        break;
                    }
                    case '0': lt_rx_link_rate = 0x06; break;
                    case '1': if ((RX_MAX_LINK_RATE<=0x04) || (RX_MAX_LINK_RATE>=0x0A)) lt_rx_link_rate = 0x0A; break;
                    case '2': if ((RX_MAX_LINK_RATE<=0x04) || (RX_MAX_LINK_RATE>=0x14)) lt_rx_link_rate = 0x14; break;
                    case '3': if ((RX_MAX_LINK_RATE<=0x04) || (RX_MAX_LINK_RATE>=0x1E)) lt_rx_link_rate = 0x1E; break;
                    case '4': if (RX_MAX_LINK_RATE<=0x04) lt_rx_link_rate = 0x01; break;
                    case '5': if ((RX_MAX_LINK_RATE==0x02) || (RX_MAX_LINK_RATE==0x04)) lt_rx_link_rate = 0x04; break;
                    case '6': if (RX_MAX_LINK_RATE==0x02) lt_rx_link_rate = 0x02; break;
                }
                if (lt_rx_link_rate > 0x04)
                {
                    // 8b10b
                    channel_coding = (channel_coding & 0xFC) | 0x01;
                    link_cap[0] = (link_cap[0] & ~0x1F) | lt_rx_link_rate;
                    // Set TPS3_SUPPORTED if link rate >HBR2
                    link_cap[1] = (lt_rx_link_rate >= 0x14) ? link_cap[1] | 0x40 : link_cap[1] & 0xBF;
                }
                else
                {
                    // 128b132, all 8b10 rates supported
                    channel_coding = channel_coding | 0x03;
                    link_cap[0] = (link_cap[0] & ~0x1F) | 0x1E;
                    // TPS3 supported
                    link_cap[1] |= 0x40;
                    uhbr_supported_link_rates = (uhbr_supported_link_rates & 0xF8) | 0x01;
                    if (lt_rx_link_rate == 0x02)
                    {
                        // Support 20Gbps and 13.5Gbps
                        uhbr_supported_link_rates |= 0x06;
                    }
                    else if (lt_rx_link_rate == 0x04)
                    {
                        uhbr_supported_link_rates |= 0x04;
                    }
                    btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_EXT_LINK_RATE, 1, &uhbr_supported_link_rates);
                }
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_MAX_LINK_RATE, 2, link_cap);
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_MAIN_LINK_CHANNEL_CODING, 1, &channel_coding);
                // Also update the equivalent in the extended cap table
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_EXT_MAX_LINK_RATE, 2, link_cap);
                btc_dprx_dpcd_gpu_access(0, 1, DPCD_ADDR_EXT_MAIN_LINK_CHANNEL_CODING, 1, &channel_coding);
                switch (lt_rx_link_rate)
                {
                    case 0x06: printf("Set RX max link rate to 1.62Gbps\n"); break;
                    case 0x0A: printf("Set RX max link rate to 2.7Gbps\n"); break;
                    case 0x14: printf("Set RX max link rate to 5.4Gbps\n"); break;
                    case 0x1E: printf("Set RX max link rate to 8.1Gbps\n"); break;
                    case 0x01: printf("Set RX max link rate to 10Gbps\n"); break;
                    case 0x04: printf("Set RX max link rate to 13.5Gbps\n"); break;
                    case 0x02: printf("Set RX max link rate to 20Gbps\n"); break;
                }
                break;
            }
#endif   // DP_SUPPORT_RX && BITEC_RX_GPUMODE
        } // switch cmd[0]
        // Reset the state.
        fcntl(STDOUT_FILENO, F_SETFL, flags);
    } // cmd != NULL
}



#if BITEC_STATUS_DEBUG

//==================================================================
// print_hw_config()
//==================================================================
int _bps_convert(int bps_code)
{
    switch (bps_code)
    {
        case 1: return 8; 
        case 2: return 10; 
        case 3: return 12; 
        case 4: return 16; 
        default: return 0; 
    }
}
    
const char* _rate_convert(int rate)
{
    switch (rate)
    {
        case 1: return "10 Gbps"; 
        case 2: return "20 Gbps"; 
        case 4: return "13.5 Gbps"; 
        case 0x06: return "1.62 Gbps"; 
        case 0x0A: return "2.7 Gbps"; 
        case 0x14: return "5.4 Gbps"; 
        case 0x1E: return "8.1 Gbps"; 
        default: return "Unk"; 
    }
}
void print_hw_config()
{
    printf("--- DP RX: %s\n", DP_SUPPORT_RX ? "enabled" : "disabled");
    #if DP_SUPPORT_RX
    {
        printf("  DP RX bps: %d\n", _bps_convert(DP_RX_BPS));
        printf("  DP RX max link rate: %s\n", _rate_convert(RX_MAX_LINK_RATE));
        printf("  DP RX max lane count: %d\n", RX_MAX_LANE_COUNT);
        printf("  DP RX interface: %s\n", DP_SUPPORT_AXI ? "AXIS VVP Full" : "Clocked Video");
        printf("  DP RX 8b10b FEC support: %s\n", DP_SUPPORT_RX_FEC ? "enabled" : "disabled");
        if (MST_RX_STREAMS > 1)
            printf("  DP RX multi-stream support: enabled, %d streams\n", MST_RX_STREAMS);
        else
            printf("  DP RX multi-stream support: disabled\n");
        printf("  DP RX DSC support: %s\n", DP_SUPPORT_RX_DSC ? "enabled" : "disabled");
        printf("  DP RX Aux channel monitoring: %s\n", BITEC_RX_AUX_DEBUG ? "enabled" : "disabled");
        printf("  DP RX test automation support: %s\n", DP_RX_SUPPORT_AUTOMATED_TEST ? "enabled" : "disabled");
    }
    #endif
    printf("--- DP TX: %s\n", DP_SUPPORT_TX ? "enabled" : "disabled");
    #if DP_SUPPORT_TX
    {
        printf("  DP TX bps: %d\n", _bps_convert(DP_TX_BPS));
        printf("  DP TX max link rate: %s\n", _rate_convert(TX_MAX_LINK_RATE));
        printf("  DP TX max lane count: %d\n", TX_MAX_LANE_COUNT);
        printf("  DP TX interface: %s\n", DP_SUPPORT_AXI ? "AXIS VVP Full" : (TX_VIDEO_IM_ENABLE ? "video IM" : "Clocked Video"));
        printf("  DP TX 8b10b FEC support: %s\n", DP_SUPPORT_TX_FEC ? "enabled" : "disabled");
        if (MST_TX_STREAMS > 1)
            printf("  DP TX multi-stream support: enabled, %d streams\n", MST_TX_STREAMS);
        else
            printf("  DP TX multi-stream support: disabled\n");
        printf("  DP TX DSC support: %s\n", DP_SUPPORT_TX_DSC ? ( (DP_SUPPORT_RX && DP_SUPPORT_RX_DSC) ? "DSC Passthrough" : "Tx only, Nios control" ) : "disabled");
        printf("  DP TX Aux channel monitoring: %s\n", BITEC_TX_AUX_DEBUG ? "enabled" : "disabled");
        printf("  DP TX test automation support: %s\n", DP_TX_SUPPORT_AUTOMATED_TEST ? "enabled" : "disabled");
    }
    #endif

    printf("Configured for FMC Rev %d\n ", BITEC_DP_CARD_REV);
}



//==================================================================
// Sink status
//==================================================================
#if DP_SUPPORT_RX
void print_sink_msa(unsigned int base_addr)
{
    unsigned int rx_stream_offset=0 , k;
    unsigned int rx_stream_delta;

    rx_stream_delta = DPRX1_REG_MSA_MVID - DPRX0_REG_MSA_MVID;

  
  
    unsigned rx_vbid;
    unsigned int vfreq_dec;
    BYTE ch_coding;
    ch_coding = (IORD(base_addr, DPRX_REG_RX_CONTROL)) >> 5 & 0x03;


    printf("------------------------------------------\n");
    printf("------   RX Main stream attributes  ------\n");
    printf("------------------------------------------\n");
    if (ch_coding == 0x2)
        printf("--- Channel Coding : 128b132b\n");
    else
        printf("--- Channel Coding : 8b10b\n");
    printf("------------------------------------------\n");

    for (k=0;k<MST_RX_STREAMS;k++) 
    {
      rx_vbid = IORD(base_addr, DPRX0_REG_VBID + rx_stream_offset);

      printf("--- Stream %d ---\n" , k);
      printf("VB-ID lock : %1.1X   MSA lock : %1.1X\n", (rx_vbid >> 6) & 1, (rx_vbid >> 7) & 1);
      printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n", ((rx_vbid & 0x100) >> 2) | (rx_vbid & 0x3F), IORD(base_addr, DPRX0_REG_MSA_MISC0 + rx_stream_offset),IORD(base_addr, DPRX0_REG_MSA_MISC1+rx_stream_offset));
      if (ch_coding == 0x2)
      {
          vfreq_dec = IORD(base_addr, DPRX0_REG_MSA_MVID+rx_stream_offset);
          vfreq_dec &= 0x00FFFFFF;
          vfreq_dec |= IORD(base_addr, DPRX0_REG_MSA_NVID+rx_stream_offset) << 24;
          printf("Vfreq  : %X (%d Hz) \n", vfreq_dec, vfreq_dec);
      }
      else
          printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPRX0_REG_MSA_MVID+rx_stream_offset),IORD(base_addr, DPRX0_REG_MSA_NVID+rx_stream_offset));
        
      printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HTOTAL+rx_stream_offset),  IORD(base_addr, DPRX0_REG_MSA_VTOTAL +rx_stream_offset));
      printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HSP   +rx_stream_offset),  IORD(base_addr, DPRX0_REG_MSA_HSW    +rx_stream_offset));
      printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HSTART+rx_stream_offset),  IORD(base_addr, DPRX0_REG_MSA_VSTART +rx_stream_offset));
      printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_VSP   +rx_stream_offset),  IORD(base_addr, DPRX0_REG_MSA_VSW    +rx_stream_offset));
      printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HWIDTH+rx_stream_offset),  IORD(base_addr, DPRX0_REG_MSA_VHEIGHT+rx_stream_offset));
      #if DP_RX_SUPPORT_AUTOMATED_TEST
          printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPRX0_REG_CRC_R+rx_stream_offset),IORD(base_addr, DPRX0_REG_CRC_G+rx_stream_offset), IORD(base_addr, DPRX0_REG_CRC_B+rx_stream_offset));
      #else
          printf("CRC : N/A\n");
      #endif
      rx_stream_offset+=rx_stream_delta;
    }
}

void print_sink_config(unsigned int base_addr)
{
    unsigned int ctrl_reg;
    unsigned int status_reg;
    unsigned int mst_control1_reg;

    unsigned int ch_coding;
    unsigned int link_rate;
    unsigned int lane_count;
    unsigned int preset;
    unsigned int ber_cntrl_reg;
    unsigned int ber_count_reg;

    BYTE stream_id[4];            // Mapping between stream and assigned payload IDs (valid payload ID are in the range 1-7)
    uint64_t mst_alloc[8];        // Allocation for payload IDs (ID 0==unassigned slots)
    BYTE slot_count[8];           // Slot count for payload IDs (ID 0==number of unassigned slots)
    BYTE idx, offset, s, slot;
    unsigned int slice;

    ctrl_reg = IORD(base_addr, DPRX_REG_RX_CONTROL);
    status_reg = IORD(base_addr, DPRX_REG_RX_STATUS);

    ch_coding = (ctrl_reg >> 5) & 0x03;
    lane_count = ctrl_reg & 0x001f;

    printf("------------------------------------------\n");
    printf("--------   RX Link configuration   -------\n");
    printf("------------------------------------------\n");
    printf("CR Done: %1.1X        SYM Done: %1.1X      Interlane Align: %d\n", status_reg & 0x0f, (status_reg & 0xf0) >> 4, (status_reg & 0x100) >> 8);
    printf("Lane count : %d\n", lane_count);
    
    if (ch_coding == 0x2)
    {
        printf("Channel Coding : 128b132b\n");
        link_rate = (ctrl_reg >> 16) & 0xFF;
        printf("Link rate  : %s\n", _rate_convert(link_rate));
        if (lane_count)
        {
            printf("FFE  : ");
            for (idx = 0; idx < lane_count; ++idx)
            {
                preset = IORD(base_addr, DPRX_REG_PRE_VOLT0 + idx)  & 0xF;
                printf("%d%s", preset, idx == (lane_count - 1) ? "\n" : ":");
            }
        }
    }
    else
    {
        printf("Channel Coding : 8b10b\n");
        printf("Link rate  : %d Mbps\n", ((ctrl_reg >> 16) & 0xff) * 270);
        if (lane_count)
        {
            printf("Swing/Preemphasis  : ");
            for (idx = 0; idx < lane_count; ++idx)
            {
                preset = IORD(base_addr, DPRX_REG_PRE_VOLT0 + idx);
                printf("%d/%d%s", preset & 0x3, (preset >> 2) & 0x3, idx == (lane_count - 1) ? "\n" : ":");
            }
        }
    }

    // Multi stream status
    mst_control1_reg = IORD(base_addr, DPRX_REG_MST_CONTROL1);
    if (mst_control1_reg & 0x01)
        printf("MST : on\n");
    else
        printf("MST : off\n");


    // Reading BER counters
    ber_count_reg = IORD(base_addr, DPRX_REG_BER_CNT0);
    printf("BER0   : %4.4X     BER1    : %4.4X\n", ber_count_reg & 0x7FFF, (ber_count_reg >> 16) & 0x7FFF);
    ber_count_reg = IORD(base_addr, DPRX_REG_BER_CNT1);
    printf("BER2   : %4.4X     BER3    : %4.4X\n", ber_count_reg & 0x7FFF, (ber_count_reg >> 16) & 0x7FFF);

    // Reset BER counters
    ber_cntrl_reg = IORD(base_addr, DPRX_REG_BER_CONTROL);
    IOWR(base_addr, DPRX_REG_BER_CONTROL, ber_cntrl_reg | 0xF0000);

    // Check slot allocation when MST framing is enabled
    if ((ch_coding == 0x2) || (mst_control1_reg & 0x01))
    {
        // Zero the allocation for each payload ID
        for (s = 0; s < 8; ++s)
        {
            slot_count[s] = 0;
            mst_alloc[s]  = 0ULL;
        }
        // Pull the mapping stream ID -> payload ID from DPRX_REG_MST_CONTROL1
        for (s = 0; s < 4; ++s)
        {
            stream_id[s] = (mst_control1_reg >> (4 * (s+1))) & 0x0F;
            // A value of 0xF is used to indicate an unassigned stream, map all values that are out of the valid range 1-7 to 0 (unassigned)
            if (stream_id[s] > 7) stream_id[s] = 0;
        }
        slot = 0;
        for (idx = 0; idx < 8; ++idx)
        {
            slice = IORD(base_addr, DPRX_REG_MST_VCPTAB0 + idx);
            offset = 0;
            // Skip the very first slot in HBR rates with MST enabled
            if ((ch_coding != 0x2) && (idx == 0))
            {
                offset = 1;
                slice = slice >> 4;
                ++slot;
            }
            while (offset < 8)
            {
                // map all values that are out of the valid range 1-7 to 0 (unassigned)
                s = slice & 0x0F;
                if (s > 7) s = 0;
                slice = slice >> 4;
                ++slot_count[s];
                mst_alloc[s] = mst_alloc[s] | (1ULL << slot);
                ++offset;
                ++slot;
            }
        }
        printf("Slot allocation:\n");
        for (s = 0; s < 4; ++s)
        {
            // If stream s was assigned a valid VC ID
            if (stream_id[s])
            {
                printf("stream%u, VC payload ID %u, %u slots: 0x%016llx\n", s, stream_id[s], slot_count[stream_id[s]], mst_alloc[stream_id[s]]);
                slot_count[stream_id[s]] = 0; // 0 to mark that this was processed
            }
        }
        if (slot_count[0])
        {
            printf("unallocated, %u slots: 0x%016llx\n", slot_count[0], mst_alloc[0]);
        }
        // Catch all remaining payload IDs in the VCP table that were not linked to a stream
        for (s = 1; s <8; ++s)
        {
            if (slot_count[s])
            {
                printf("unmapped VC payload ID %u, %u slots: 0x%016llx\n", s, slot_count[s], mst_alloc[s]);
            }
        }
    }


#if BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_HDCP1
    if (IORD(base_addr, DPRX_REG_HDCP1_STATUS) & 0x80000)
        printf("HDCP 1.3 decoder authenticated\n");
    else
        printf("HDCP 1.3 decoder not authenticated\n");
#endif
#if BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_HDCP2
    if (IORD(base_addr, DPRX_REG_HDCP2_STATUS) & 0x80000)
        printf("HDCP 2.2 decoder authenticated\n");
    else
        printf("HDCP 2.2 decoder not authenticated\n");
#endif

    // FEC compulsory at 128b/132b, optional support at 8b/10b
    if (DP_SUPPORT_RX_FEC || (ch_coding == 0x2))
    {
        if (status_reg & 0x100000) // if FEC running
        {
            printf("FEC: running, error count ");
            unsigned int fec_cntrl = IORD(base_addr, DPRX_REG_FEC_ERR_CNF);
            if (fec_cntrl & 0x07) // if one error counter is selected
            {
                unsigned int fec_err_cnt = IORD(base_addr, DPRX_REG_FEC_ERR_CNT);
                if (fec_err_cnt & 0x8000)
                {
                    printf("0x%4.4X (", fec_err_cnt & 0x7FFF);
                    switch (fec_cntrl & 0x07)
                    {
                        case 1: printf("UNCORRECTED_BLOCK_ERROR_COUNT"); break;
                        case 2: printf("CORRECTED_BLOCK_ERROR_COUNT"); break;
                        case 3: printf("BIT_ERROR_COUNT"); break;
                        case 4: printf("PARITY_BLOCK_ERROR_COUNT"); break;
                        case 5: printf("PARITY_BIT_ERROR_COUNT"); break;
                        default: printf("UNKNOWN");
                    }
                    if (fec_cntrl & 0x20)
                    {
                        printf("), all lanes\n");
                    }
                    else
                    {
                        printf("), lane %d\n", (fec_cntrl >> 3) & 0x3);
                    }
                }
                else
                {
                    printf("invalid\n");
                }
            }
            else
            {
                printf("disabled\n");
            }
        }
        else
        {
            printf("FEC: not running\n");
        }
    }
    else
    {
        printf("FEC: not supported\n");
    }
}

void print_sink_debug(unsigned int base_addr)
{  
    printf("------------------------------------------\n");
    printf("------  RX Link Debug Information  -------\n");
    printf("------------------------------------------\n");
    #if RX_MAX_LINK_RATE < 6
    printf("128b132b DNU flags (lowest FFE value at LSB):\n");
    struct { int rate; int rate_mapping; } dnu_ffe_rates[] = {
        // Print 20Gbps then 13.5Gbps then 10Gbps (2,4,1), this maps to 1,2,0 for btc_dprx_lt_dnu_ffe_get
        {2, 1}, {4, 2}, {1, 0}
    };
    for (int i = 0; i < 3; ++i){
        printf("%s: 0x%04x\n", _rate_convert(dnu_ffe_rates[i].rate), btc_dprx_lt_dnu_ffe_get(0, dnu_ffe_rates[i].rate_mapping));
    }
    #endif
    printf("8b10b DNU flags {pree3[3:0], pree2[3:0],pree1[3:0],pree0[3:0]} (lowest swing values at LSBs):\n");
    struct { int rate; int rate_mapping; } dnu_swpre_rates[] = {
        // Print 8.1 Gbps then 5.4Gbps then 2.7Gbps then 1.62Gbps(0x1E,0x14,0x0A,0x06), this maps to 3,2,1,0 for btc_dprx_lt_dnu_swpre_get
        {0x1E, 3}, {0x14, 2}, {0x0A, 1}, {0x06, 0}
    };
    for (int i = 0; i < 4; ++i){
        printf("%s: 0x%04x\n", _rate_convert(dnu_swpre_rates[i].rate), btc_dprx_lt_dnu_swpre_get(0, dnu_swpre_rates[i].rate_mapping));
    }
    #if RX_MAX_LINK_RATE < 6
    printf("128b132b link loss count at 1,2 and 4 lanes at 10ms, 500ms+, LQM triggered (high BER):\n");
    int uhbr_rates[] = {2, 4, 1}; // 20Gbps, 13.5Gbps, 10Gbps
    for (int i = 0; i < 3; ++i) {
        printf("%s: 1lane (%d/%d/%d), 2lanes (%d/%d/%d), 4lanes (%d/%d/%d)\n",
               _rate_convert(uhbr_rates[i]),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 1, 0),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 1, 1),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 1, 2),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 2, 0),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 2, 1),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 2, 2),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 4, 0),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 4, 1),
               btc_dprx_get_lt_failure_count(0, uhbr_rates[i], 4, 2));
    }
    #endif
    printf("8b10b link loss count at 1,2 and 4 lanes at 10ms, 500ms+, LQM triggered (high BER):\n");
    int hbr_rates[] = {0x1E, 0x14, 0x0A, 0x06}; //  8.1 Gbps, 5.4Gbps, 2.7Gbps, 1.62Gbps
    for (int i = 0; i < 4; ++i) {
        printf("%s: 1lane (%d/%d/%d), 2lanes (%d/%d/%d), 4lanes (%d/%d/%d)\n",
               _rate_convert(hbr_rates[i]),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 1, 0),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 1, 1),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 1, 2),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 2, 0),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 2, 1),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 2, 2),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 4, 0),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 4, 1),
               btc_dprx_get_lt_failure_count(0, hbr_rates[i], 4, 2));
    }
}

#endif // DP_SUPPORT_RX


//==================================================================
// Source status
//==================================================================
#if DP_SUPPORT_TX
void print_source_msa(unsigned int base_addr)
{
    unsigned int tx_stream_offset=0 , k;
    unsigned int tx_stream_delta;

    tx_stream_delta = DPTX1_REG_MSA_MVID - DPTX0_REG_MSA_MVID;
    unsigned int vfreq_dec;
    BYTE ch_coding;
    ch_coding = (IORD(base_addr, DPTX_REG_TX_CONTROL)) >> 10 & 0x03;

    printf("------------------------------------------\n");
    printf("------   TX Main stream attributes  ------\n");
    printf("------------------------------------------\n");
    if (ch_coding == 0x2)
        printf("--- Channel Coding : 128b132b\n");
    else
        printf("--- Channel Coding : 8b10b\n");
    printf("------------------------------------------\n");
    for (k=0 , tx_stream_offset=0;k<MST_TX_STREAMS;k++,tx_stream_offset+=tx_stream_delta)
    {
      printf("--- Stream %d ---\n" , k);
      printf("MSA lock : %1.1X\n", (IORD(base_addr, DPTX0_REG_VBID+tx_stream_offset) >> 7) & 1);
      printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n", IORD(base_addr, DPTX0_REG_VBID+tx_stream_offset) & 0x7F,IORD(base_addr, DPTX0_REG_MSA_MISC0+tx_stream_offset), IORD(base_addr, DPTX0_REG_MSA_MISC1+tx_stream_offset));
      if (ch_coding == 0x2)
      {
          vfreq_dec = IORD(base_addr, DPTX0_REG_MSA_MVID+tx_stream_offset);
          vfreq_dec &= 0x00FFFFFF;
          vfreq_dec |= IORD(base_addr, DPTX0_REG_MSA_NVID+tx_stream_offset) << 24;
          printf("Vfreq  : %X (%d Hz) \n", vfreq_dec, vfreq_dec);
      }
      else
          printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPTX0_REG_MSA_MVID+tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_NVID+tx_stream_offset));

      printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HTOTAL+tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_VTOTAL +tx_stream_offset));
      printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HSP   +tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_HSW    +tx_stream_offset));
      printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HSTART+tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_VSTART +tx_stream_offset));
      printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_VSP   +tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_VSW    +tx_stream_offset));
      printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HWIDTH+tx_stream_offset),IORD(base_addr, DPTX0_REG_MSA_VHEIGHT+tx_stream_offset));
      #if DP_TX_SUPPORT_AUTOMATED_TEST
          printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPTX0_REG_CRC_R+tx_stream_offset),IORD(base_addr, DPTX0_REG_CRC_G+tx_stream_offset), IORD(base_addr, DPTX0_REG_CRC_B+tx_stream_offset));
      #else
          printf("CRC : N/A\n");
      #endif
  }
}

void print_source_config(unsigned int base_addr)
{
    unsigned int ctrl_reg;
    unsigned int mst_control1_reg;
    unsigned int ch_coding;
    unsigned int link_rate;
    unsigned int lane_count;
    unsigned int preset;
    BYTE stream_id[4];            // Mapping between stream and assigned payload IDs (valid payload IDs are in the range 1-7)
    uint64_t mst_alloc[8];        // Allocation for payload IDs (ID 0==unassigned slots)
    BYTE slot_count[8];           // Slot count for payload IDs (ID 0==number of unassigned slots)
    BYTE idx, offset, s, slot;
    unsigned int slice;
    

    ctrl_reg = IORD(base_addr, DPTX_REG_TX_CONTROL);

    ch_coding = (ctrl_reg >> 10) & 0x03;
    lane_count = (ctrl_reg >> 5) & 0x1f;

    printf("------------------------------------------\n");
    printf("--------   TX Link configuration   -------\n");
    printf("------------------------------------------\n");
    printf("Lane count : %d\n", lane_count);

    if (ch_coding == 0x2)
    {
        printf("Channel Coding : 128b132b\n");
        link_rate = (ctrl_reg >> 21) & 0xFF;
        printf("Link rate  : %s\n", _rate_convert(link_rate));
        if (lane_count)
        {
            printf("FFE  : ");
            for (idx = 0; idx < lane_count; ++idx)
            {
                preset = IORD(base_addr, DPTX_REG_TXFFE0 + idx) & 0xF;
                printf("%d%s", preset, idx == (lane_count - 1) ? "\n" : ":");
            }
        }
    }
    else
    {
        printf("Channel Coding : 8b10b\n");
        printf("Link rate  : %d Mbps\n", ((ctrl_reg >> 21) & 0xff) * 270);
        if (lane_count)
        {
            printf("Swing/Preemphasis  : ");
            for (idx = 0; idx < lane_count; ++idx)
            {
                preset = IORD(base_addr, DPTX_REG_PRE_VOLT0 + idx);
                printf("%d/%d%s", preset & 0x3, (preset >> 2) & 0x3, idx == (lane_count - 1) ? "\n" : ":");
            }
        }
    }

    mst_control1_reg = IORD(base_addr, DPTX_REG_MST_CONTROL1);
    if (mst_control1_reg & 0x01)
        printf("MST : on\n");
    else
        printf("MST : off\n");

    // Check slot allocation
    if ((ch_coding == 0x2) || (mst_control1_reg & 0x01))
    {
        // Zero the allocation for each payload ID
        for (s = 0; s < 8; ++s)
        {
            slot_count[s] = 0;
            mst_alloc[s]  = 0ULL;
        }
        // Pull the mapping stream ID -> payload ID from DPTX_REG_MST_CONTROL1
        for (s = 0; s < 4; ++s)
        {
            stream_id[s] = (mst_control1_reg >> (4 * (s+1))) & 0x0F;
            // A value of 0xF is used to indicate an unassigned stream, map all values that are out of the valid range 1-7 to 0 (unassigned)
            if (stream_id[s] > 7) stream_id[s] = 0;
        }
        slot = 0;
        for (idx = 0; idx < 8; ++idx)
        {
            slice = IORD(base_addr, DPTX_REG_MST_VCPTAB0 + idx);
            offset = 0;
            // Skip the very first slot in HBR rates with MST enabled
            if ((ch_coding != 0x2) && (idx == 0))
            {
                offset = 1;
                slice = slice >> 4;
                ++slot;
            }
            while (offset < 8)
            {
                // map all values that are out of the valid range 1-7 to 0 (unassigned)
                s = slice & 0x0F;
                if (s > 7) s = 0;
                slice = slice >> 4;
                ++slot_count[s];
                mst_alloc[s] = mst_alloc[s] | (1ULL << slot);
                ++offset;
                ++slot;
            }
        }
        printf("Slot allocation:\n");
        for (s = 0; s < 4; ++s)
        {
            // If stream s was assigned a valid VC ID
            if (stream_id[s])
            {
                printf("stream%u, VC payload ID %u, %u slots: 0x%016llx\n", s, stream_id[s], slot_count[stream_id[s]], mst_alloc[stream_id[s]]);
                slot_count[stream_id[s]] = 0; // 0 to mark that this was processed
            }
        }
        if (slot_count[0])
        {
            printf("unallocated, %u slots: 0x%016llx\n", slot_count[0], mst_alloc[0]);
        }
        // Catch all remaining payload IDs in the VCP table that were not linked to a stream
        for (s = 1; s < 8; ++s)
        {
            if (slot_count[s])
            {
                printf("unmapped VC payload ID %u, %u slots: 0x%016llx\n", s, slot_count[s], mst_alloc[s]);
            }
        }
    }

#if BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP1
    if (IORD(base_addr, DPTX_REG_HDCP1_STATUS) & 0x80000)
        printf("HDCP 1.3 encoder authenticated\n");
    else
        printf("HDCP 1.3 encoder not authenticated\n");
#endif
#if BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP2
    if (IORD(base_addr, DPTX_REG_HDCP2_STATUS) & 0x80000)
        printf("HDCP 2.2 encoder authenticated\n");
    else
        printf("HDCP 2.2 encoder not authenticated\n");
#endif
}

void print_source_debug(unsigned int base_addr)
{  
    printf("------------------------------------------\n");
    printf("------  TX Link Debug Information  -------\n");
    printf("------------------------------------------\n");
    printf("LT failure counters at 1/2/4 lanes:\n");
    #if TX_MAX_LINK_RATE < 6
    printf("20 Gbps  : %d %d %d\n", btc_dptx_get_lt_failure_count(0, 2, 1), btc_dptx_get_lt_failure_count(0, 2, 2), btc_dptx_get_lt_failure_count(0, 2, 4));
    printf("13.5 Gbps: %d %d %d\n", btc_dptx_get_lt_failure_count(0, 4, 1), btc_dptx_get_lt_failure_count(0, 4, 2), btc_dptx_get_lt_failure_count(0, 4, 4));
    printf("10 Gbps  : %d %d %d\n", btc_dptx_get_lt_failure_count(0, 1, 1), btc_dptx_get_lt_failure_count(0, 1, 2), btc_dptx_get_lt_failure_count(0, 1, 4));
    #endif
    printf("8.1 Gbps : %d %d %d\n", btc_dptx_get_lt_failure_count(0, 0x1E, 1), btc_dptx_get_lt_failure_count(0, 0x1E, 2), btc_dptx_get_lt_failure_count(0, 0x1E, 4));
    printf("5.4 Gbps : %d %d %d\n", btc_dptx_get_lt_failure_count(0, 0x14, 1), btc_dptx_get_lt_failure_count(0, 0x14, 2), btc_dptx_get_lt_failure_count(0, 0x14, 4));
    printf("2.7 Gbps : %d %d %d\n", btc_dptx_get_lt_failure_count(0, 0x0A, 1), btc_dptx_get_lt_failure_count(0, 0x0A, 2), btc_dptx_get_lt_failure_count(0, 0x0A, 4));
    printf("1.62 Gbps: %d %d %d\n", btc_dptx_get_lt_failure_count(0, 0x06, 1), btc_dptx_get_lt_failure_count(0, 0x06, 2), btc_dptx_get_lt_failure_count(0, 0x06, 4));
    //btc_dptx_reset_lt_failure_counters(0); // Reset the counters
}

#endif // DP_SUPPORT_TX
#endif // BITEC_STATUS_DEBUG


#if BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG
void dump_aux_traffic()
{
#if BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX
    // Dump AUX channel traffic
    if (auxRxDebugEnable&1)
        dp_dump_aux_debug(&_gRxAuxInstance);
#endif
#if BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX
    if (auxTxDebugEnable&1)
        dp_dump_aux_debug(&_gTxAuxInstance);
#endif
}
#endif // BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG 



#endif // BITEC_STATUS_DEBUG || BITEC_RX_AUX_DEBUG || BITEC_TX_AUX_DEBUG 

