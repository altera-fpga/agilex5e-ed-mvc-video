

#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "btc_dprx_syslib.h"
#include "btc_dptx_syslib.h"
#include "btc_dptxll_syslib.h"
#include "debug.h"
#include "intel_fpga_i2c.h"
#include "config.h"
#include "tx_utils.h"
#include "board.h"


#include "dsc_utils.h"

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DBG_PRINTF printf
#else
#define DBG_PRINTF(format, args...) ((void)0)
#endif

extern BYTE tx_edid_data[512];  // TX copy of Sink EDID



#if (DP_DSC_TX_ONLY)

extern BYTE edid_2560x1600[128];

void select_resolutions (int is_dsc, struct mode_parameters* mode_param_array, struct pps_parameters* dsc_data_pps_parameters, int* selected_mode)
{

int modes_found;
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

         if (is_dsc) {
          read_pps_params( dsc_data_pps_parameters);
          }

          load_resolutions (is_dsc, mode_param_array, dsc_data_pps_parameters, &modes_found);
          printf(" Modes found     %d \n", modes_found);

          *selected_mode = 1;
          updated_mode_parameters = mode_param_array[*selected_mode];
//
//            DBG_PRINTF(" Mode is      %d %d \n", mode_command, updated_mode_parameters.mode_valid);
//            if        (updated_mode_parameters.mode_valid == 1)
//            {
            DBG_PRINTF("HActive      %d \n", updated_mode_parameters.HActive);
            DBG_PRINTF("HBlank       %d \n", updated_mode_parameters.HBlank);
            DBG_PRINTF("VActive      %d \n", updated_mode_parameters.VActive);
            DBG_PRINTF("VBlank       %d \n", updated_mode_parameters.VBlank);
            DBG_PRINTF("HFront       %d \n", updated_mode_parameters.HFront);
            DBG_PRINTF("HSync        %d \n", updated_mode_parameters.HSync);
            DBG_PRINTF("VFront       %d \n", updated_mode_parameters.VFront);
            DBG_PRINTF("VSync        %d \n", updated_mode_parameters.VSync);
            DBG_PRINTF("HTotal       %d \n", updated_mode_parameters.HTotal);
            DBG_PRINTF("VTotal       %d \n", updated_mode_parameters.VTotal);
            DBG_PRINTF("frame_rate   %d \n", updated_mode_parameters.frame_rate);
//
    program_cvo_tpg (0, &updated_mode_parameters, dsc_data_pps_parameters);
//    program_dsc_regs( &updated_mode_parameters, &dsc_data_pps_parameters);
//    enable_dsc();
//        }

}


void load_resolutions (int is_dsc,struct mode_parameters* mode_param_array, struct pps_parameters* dsc_data_pps_parameters, int* modes_found)
{

    int          mode_index;
    int          prev_mode_index;
    unsigned int base_index;
    int          edid_index;

    // Values from EDID
    unsigned int EDID_freq;
    unsigned int EDID_HActive;
    unsigned int EDID_HBlank;
    unsigned int EDID_VActive;
    unsigned int EDID_VBlank;
    unsigned int EDID_HFront;
    unsigned int EDID_HSync;
    unsigned int EDID_VFront;
    unsigned int EDID_VSync;
    unsigned int EDID_VTotal;
    unsigned int EDID_HTotal;
    //unsigned int EDID_Total; //unused, but for info
    long     int EDID_frame_rate;
    unsigned int EDID_standard_timing;
    unsigned int EDID_extended;
 
    struct mode_parameters new_mode_parameters      = { .frame_rate = 0,
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
                                                        .VSync      = 0};
    struct mode_parameters updated_mode_parameters  = { .frame_rate = 0,
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
                                                        .VSync      = 0};


    //Initialise array index values
    mode_index      = 1;
    prev_mode_index = 1;

    btc_dptx_edid_read(0, tx_edid_data);       // Read the sink EDID
    for (edid_index = 0; edid_index < 0x70; ++edid_index)
    {
        // Debug: dump EDID data
        DBG_PRINTF("EDID [%x]  : %x\n",edid_index, tx_edid_data[edid_index]);
    }
     //Load standard timings from EDID
     DBG_PRINTF("Standard Timings\n");

    for (base_index = 0x26; base_index < 0x35; base_index +=2)
        {
        EDID_standard_timing     = (  tx_edid_data[base_index+1] * 256)                  + tx_edid_data[base_index];
        if (EDID_standard_timing > 0) 
           {
            DBG_PRINTF("Mode :%x %x\n", mode_index, EDID_standard_timing);
            load_standard_timing( EDID_standard_timing, &mode_index, dsc_data_pps_parameters, &updated_mode_parameters);
 
            DBG_PRINTF("updated Index        %d \n", mode_index);
            DBG_PRINTF("updated HActive      %d \n", updated_mode_parameters.HActive);
            DBG_PRINTF("updated HBlank       %d \n", updated_mode_parameters.HBlank);
            DBG_PRINTF("updated VActive      %d \n", updated_mode_parameters.VActive);
            DBG_PRINTF("updated VBlank       %d \n", updated_mode_parameters.VBlank);
            DBG_PRINTF("updated HFront       %d \n", updated_mode_parameters.HFront);
            DBG_PRINTF("updated HSync        %d \n", updated_mode_parameters.HSync);
            DBG_PRINTF("updated VFront       %d \n", updated_mode_parameters.VFront);
            DBG_PRINTF("updated VSync        %d \n", updated_mode_parameters.VSync);
            DBG_PRINTF("updated HTotal       %d \n", updated_mode_parameters.HTotal);
            DBG_PRINTF("updated VTotal       %d \n", updated_mode_parameters.VTotal);
            DBG_PRINTF("updated frame_rate   %d \n", updated_mode_parameters.frame_rate);

            if (mode_index > prev_mode_index) 
                {
                DBG_PRINTF("Mode :%d  %dx%d %dfps \n", prev_mode_index, updated_mode_parameters.HActive , updated_mode_parameters.VActive, updated_mode_parameters.frame_rate);
                mode_param_array[prev_mode_index] = updated_mode_parameters; 
                prev_mode_index = mode_index;
                }
            }
        }
    for (base_index = 0x36; base_index < 0x70; base_index +=18)
        {
        DBG_PRINTF("EDID [%x]  : %x\n",base_index, tx_edid_data[base_index]);
        EDID_freq     = (  tx_edid_data[base_index+1] * 256)                  + tx_edid_data[base_index];
        // Check for a valid EDID frequency being set, which implies the EDID data for this mode exists
        if (EDID_freq > 0) 
            {
            EDID_HActive     = (((tx_edid_data[base_index+ 4] &0xF0) >> 4 ) * 256)   + tx_edid_data[base_index+2];
            EDID_HBlank      = (  tx_edid_data[base_index+ 4] &0x0F         * 256)   + tx_edid_data[base_index+3];
            EDID_VActive     = (((tx_edid_data[base_index+ 7] &0xF0) >> 4 ) * 256)   + tx_edid_data[base_index+5];
            EDID_VBlank      = (  tx_edid_data[base_index+ 7] &0x0F         * 256)   + tx_edid_data[base_index+6];
            EDID_HFront      = (((tx_edid_data[base_index+11] &0xC0) >> 6 ) * 256)   + tx_edid_data[base_index+8];
            EDID_HSync       = (((tx_edid_data[base_index+11] &0x30) >> 4 ) * 256)   + tx_edid_data[base_index+9];
            EDID_VFront      = (((tx_edid_data[base_index+11] &0x0C) >> 2 ) *  32)   + ((tx_edid_data[base_index+10]  &0xF0) >> 4 );
            EDID_VSync       = (((tx_edid_data[base_index+11] &0x03)      ) *  32)   +  (tx_edid_data[base_index+10]  &0x0F);
            EDID_HTotal      = (EDID_HActive + EDID_HBlank);
            EDID_VTotal      = (EDID_VActive + EDID_VBlank);
            //EDID_Total       = (EDID_HTotal  * EDID_VTotal); //unused, but for info
            EDID_frame_rate  = (EDID_freq *1000) ;
            EDID_frame_rate  = (EDID_frame_rate / EDID_HTotal) *1000 ;
            EDID_frame_rate  = (EDID_frame_rate / EDID_VTotal) ;
            EDID_frame_rate  = (EDID_frame_rate +99)/ 100 ;

            DBG_PRINTF("EDID Freq         %d \n", EDID_freq);
            DBG_PRINTF("EDID_HActive      %d \n", EDID_HActive);
            DBG_PRINTF("EDID_HBlank       %d \n", EDID_HBlank);
            DBG_PRINTF("EDID_VActive      %d \n", EDID_VActive);
            DBG_PRINTF("EDID_VBlank       %d \n", EDID_VBlank);
            DBG_PRINTF("EDID_HFront       %d \n", EDID_HFront);
            DBG_PRINTF("EDID_HSync        %d \n", EDID_HSync);
            DBG_PRINTF("EDID_VFront       %d \n", EDID_VFront);
            DBG_PRINTF("EDID_VSync        %d \n", EDID_VSync);
            DBG_PRINTF("EDID_frame_rate   %ld \n", EDID_frame_rate);
            DBG_PRINTF("Pic Height        %x \n",  dsc_data_pps_parameters->pic_height);
            DBG_PRINTF("Pic Width         %x \n",  dsc_data_pps_parameters->pic_width);

            new_mode_parameters.frame_rate = EDID_frame_rate;
            new_mode_parameters.HTotal     = EDID_HTotal;
            new_mode_parameters.pixel_clk  = EDID_freq;
            new_mode_parameters.HActive    = EDID_HActive;
            new_mode_parameters.HBlank     = EDID_HBlank;
            new_mode_parameters.HFront     = EDID_HFront;
            new_mode_parameters.HSync      = EDID_HSync;
            new_mode_parameters.VTotal     = EDID_VTotal;
            new_mode_parameters.VActive    = EDID_VActive;
            new_mode_parameters.VBlank     = EDID_VBlank;
            new_mode_parameters.VFront     = EDID_VFront;
            new_mode_parameters.VSync      = EDID_VSync;

            DBG_PRINTF("Mode :%d  %dx%d %ldfps \n", mode_index, EDID_HActive , EDID_VActive, EDID_frame_rate);
            // Check if this mode matches the resolution of the dsc data in DDR.
            if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
               {
                DBG_PRINTF("Mode Matches PPS \n");
                printf("Mode :%d  %dx%d %ldfps \n", mode_index, EDID_HActive , EDID_VActive, EDID_frame_rate);
                // This mode matches the resolution of the dsc data in DDR, 
                // so mark it as valid and add the parameters to the mode array
                new_mode_parameters.mode_valid      = 1;
                mode_param_array[mode_index]        = new_mode_parameters; 
                mode_index++;
               }
            DBG_PRINTF("===================================\n");
            }
        }
        // Check if the EDID contains extended data
        DBG_PRINTF("EDID Extend[0x7E]  : %X\n", tx_edid_data[0x7E]);
        EDID_extended = 0;
        if (EDID_extended == 1) {
            //Load standard timings from EDID
            DBG_PRINTF("Standard Timings\n");
            for (base_index = 0x99; base_index < 256; base_index +=18)
                {
                DBG_PRINTF("EDID [%x]  : %x\n",base_index, tx_edid_data[base_index]);
                EDID_freq     = (  tx_edid_data[base_index+1] * 256)                  + tx_edid_data[base_index];
                // Check for a valid EDID frequency being set, whihc implies the EDID data for this mode exists
                if (EDID_freq > 0) {
                    EDID_HActive     = (((tx_edid_data[base_index +4] &0xF0) >> 4 ) * 256)  +   tx_edid_data[base_index+ 2];
                    EDID_HBlank      = (  tx_edid_data[base_index +4] &0x0F         * 256)  +   tx_edid_data[base_index+ 3];
                    EDID_VActive     = (((tx_edid_data[base_index +7] &0xF0) >> 4 ) * 256)  +   tx_edid_data[base_index+ 5];
                    EDID_VBlank      = (  tx_edid_data[base_index +7] &0x0F         * 256)  +   tx_edid_data[base_index+ 6];
                    EDID_HFront      = (((tx_edid_data[base_index+11] &0xC0) >> 6 ) * 256)  +   tx_edid_data[base_index+ 8];
                    EDID_HSync       = (((tx_edid_data[base_index+11] &0x30) >> 4 ) * 256)  +   tx_edid_data[base_index+ 9];
                    EDID_VFront      = (((tx_edid_data[base_index+11] &0x0C) >> 2 ) *  32)  + ((tx_edid_data[base_index+10]  &0xF0) >> 4 );
                    EDID_VSync       = (((tx_edid_data[base_index+11] &0x03)      ) *  32)  +  (tx_edid_data[base_index+10]  &0x0F);
                    EDID_HTotal      = (EDID_HActive + EDID_HBlank);
                    EDID_VTotal      = (EDID_VActive + EDID_VBlank);
                    //EDID_Total       = (EDID_HTotal  * EDID_VTotal); //unused, but for info
                    EDID_frame_rate  = (EDID_freq *1000) ;
                    EDID_frame_rate  = (EDID_frame_rate / EDID_HTotal) *1000 ;
                    EDID_frame_rate  = (EDID_frame_rate / EDID_VTotal) ;
                    EDID_frame_rate  = (EDID_frame_rate +99)/ 100 ;

                    DBG_PRINTF("EDID Freq         %d \n", EDID_freq);
                    DBG_PRINTF("EDID_HActive      %d \n", EDID_HActive);
                    DBG_PRINTF("EDID_HBlank       %d \n", EDID_HBlank);
                    DBG_PRINTF("EDID_VActive      %d \n", EDID_VActive);
                    DBG_PRINTF("EDID_VBlank       %d \n", EDID_VBlank);
                    DBG_PRINTF("EDID_HFront       %d \n", EDID_HFront);
                    DBG_PRINTF("EDID_HSync        %d \n", EDID_HSync);
                    DBG_PRINTF("EDID_VFront       %d \n", EDID_VFront);
                    DBG_PRINTF("EDID_VSync        %d \n", EDID_VSync);
                    DBG_PRINTF("EDID_HTotal       %d \n", EDID_HTotal);
                    DBG_PRINTF("EDID_VTotal       %d \n", EDID_VTotal);
                    DBG_PRINTF("EDID_frame_rate   %ld \n", EDID_frame_rate);

                    new_mode_parameters.frame_rate           = EDID_frame_rate;
                    new_mode_parameters.pixel_clk            = EDID_freq;
                    new_mode_parameters.HTotal               = EDID_HTotal;
                    new_mode_parameters.HActive              = EDID_HActive;
                    new_mode_parameters.HBlank               = EDID_HBlank;
                    new_mode_parameters.HFront               = EDID_HFront;
                    new_mode_parameters.HSync                = EDID_HSync;
                    new_mode_parameters.VTotal               = EDID_VTotal;
                    new_mode_parameters.VActive              = EDID_VActive;
                    new_mode_parameters.VBlank               = EDID_VBlank;
                    new_mode_parameters.VFront               = EDID_VFront;
                    new_mode_parameters.VSync                = EDID_VSync;

                    // Check if this mode matches the resolution of the dsc data in DDR.
                    if (( is_dsc == 0) | ( (dsc_data_pps_parameters->pic_height == EDID_VActive) & (dsc_data_pps_parameters->pic_width == EDID_HActive)))
                        {
                        new_mode_parameters.mode_valid = 1;
                        DBG_PRINTF("Mode :%d  %dx%d %ldfps \n", mode_index, EDID_HActive , EDID_VActive, EDID_frame_rate);
                        mode_param_array[mode_index+1] = new_mode_parameters; 
                        updated_mode_parameters = mode_param_array[mode_index+1];
                        mode_index++;
                        }
                    DBG_PRINTF("===================================\n");
                    } // If Edid freq > 0
                } // for (base_index = 0x99; base_index < 256; base_index +=18)
        }//(EDID_extended == 1) 

        //   printf("Parse edid_2560x1600  : \n");
   
   for (base_index = 0x36; base_index < 0x70; base_index +=18)
        {
           // printf("EDID [%x]  : %x\n",base_index, edid_2560x1600[base_index]);
            EDID_freq     = (  edid_2560x1600[base_index+1] * 256)                  + edid_2560x1600[base_index];

            if (EDID_freq > 0) {
            EDID_HActive  = (((edid_2560x1600[base_index+4] &0xF0) >> 4 ) * 256)   + edid_2560x1600[base_index+2];
            EDID_HBlank   = (  edid_2560x1600[base_index+4] &0x0F * 256)           + edid_2560x1600[base_index+3];
            EDID_VActive  = (((edid_2560x1600[base_index+7] &0xF0) >> 4 ) * 256)   + edid_2560x1600[base_index+5];
            EDID_VBlank   = (  edid_2560x1600[base_index+7] &0x0F * 256)           + edid_2560x1600[base_index+6];
            EDID_HFront   = (((edid_2560x1600[base_index+11] &0xC0) >> 6 ) * 256)  + edid_2560x1600[base_index+8];
            EDID_HSync    = (((edid_2560x1600[base_index+11] &0x30) >> 4 ) * 256)  + edid_2560x1600[base_index+9];
            EDID_VFront   = (((edid_2560x1600[base_index+11] &0x0C) >> 2 ) * 32)   + ((edid_2560x1600[base_index+10]  &0xF0) >> 4 );
            EDID_VSync    = (((edid_2560x1600[base_index+11] &0x03)      ) * 32)   +  (edid_2560x1600[base_index+10]  &0x0F);
            EDID_HTotal   = (EDID_HActive + EDID_HBlank);
            EDID_VTotal   = (EDID_VActive + EDID_VBlank);
            //EDID_Total    = (EDID_HTotal  * EDID_VTotal); //unused, but for info
            EDID_frame_rate  = (EDID_freq *1000) ;
            EDID_frame_rate  = (EDID_frame_rate / EDID_HTotal) *1000 ;
            EDID_frame_rate  = (EDID_frame_rate / EDID_VTotal) ;
            EDID_frame_rate  = (EDID_frame_rate +99)/ 100 ;

            new_mode_parameters.frame_rate           = EDID_frame_rate;
            new_mode_parameters.pixel_clk            = EDID_freq;
            new_mode_parameters.HTotal               = EDID_HTotal;
            new_mode_parameters.HActive              = EDID_HActive;
            new_mode_parameters.HBlank               = EDID_HBlank;
            new_mode_parameters.HFront               = EDID_HFront;
            new_mode_parameters.HSync                = EDID_HSync;
            new_mode_parameters.VTotal               = EDID_VTotal;
            new_mode_parameters.VActive              = EDID_VActive;
            new_mode_parameters.VBlank               = EDID_VBlank;
            new_mode_parameters.VFront               = EDID_VFront;
            new_mode_parameters.VSync                = EDID_VSync;
            new_mode_parameters.mode_valid           = 1;

			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %ldfps \n", mode_index, EDID_HActive , EDID_VActive, EDID_frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
			}
        }
		}

           //printf("Add extra format   1920x1200 60fps \n");

             new_mode_parameters.frame_rate = 60;
             new_mode_parameters.pixel_clk  = 14902; // Htotal * Vtotal *frame rate (/10 000)
             new_mode_parameters.HTotal     = 0x7E0; // HActive + HBlank
             new_mode_parameters.HActive    = 0x780;
             new_mode_parameters.HBlank     = 0x60;
             new_mode_parameters.HFront     = 0x30;
             new_mode_parameters.HSync      = 0x2C;
             new_mode_parameters.VTotal     = 0x4D0;
             new_mode_parameters.VActive    = 0x4B0;
             new_mode_parameters.VBlank     = 0x20;
             new_mode_parameters.VFront     = 0x00;
             new_mode_parameters.VSync      = 0x06;
             new_mode_parameters.mode_valid = 1;

			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;

		}
           //printf("Add extra format   1920x1200 120fps \n");

             new_mode_parameters.frame_rate = 120;
             new_mode_parameters.pixel_clk  = 14902; // Htotal * Vtotal *frame rate (/10 000)
             new_mode_parameters.HTotal     = 0x7E0; // HActive + HBlank
             new_mode_parameters.HActive    = 0x780;
             new_mode_parameters.HBlank     = 0x60;
             new_mode_parameters.HFront     = 0x30;
             new_mode_parameters.HSync      = 0x2C;
             new_mode_parameters.VTotal     = 0x4D0;
             new_mode_parameters.VActive    = 0x4B0;
             new_mode_parameters.VBlank     = 0x20;
             new_mode_parameters.VFront     = 0x00;
             new_mode_parameters.VSync      = 0x06;
             new_mode_parameters.mode_valid = 1;

			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
	}

            //printf("Add extra format   3840x2160 60fps \n");

             new_mode_parameters.frame_rate = 60;
             new_mode_parameters.pixel_clk  = 55536;
             new_mode_parameters.HTotal     = 0xFA0;
             new_mode_parameters.HActive    = 0xF00;
             new_mode_parameters.HBlank     = 0xA0;
             new_mode_parameters.HFront     = 0x57;
             new_mode_parameters.HSync      = 0x2C;
             new_mode_parameters.VTotal     = 0x90A;
             new_mode_parameters.VActive    = 2160;
             new_mode_parameters.VBlank     = 0x9A;
             new_mode_parameters.VFront     = 0x00;
             new_mode_parameters.VSync      = 0x05;
             new_mode_parameters.mode_valid = 1;
			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
}

//            printf("Add extra format   3840x2160 120 fps \n");

             new_mode_parameters.frame_rate           =  120;
             new_mode_parameters.adjusted_frame_rate  =  120;
             new_mode_parameters.pixel_clk            = 106656; //0000;
             new_mode_parameters.HTotal               = 4000;
             new_mode_parameters.HActive              = 3840;
             new_mode_parameters.HBlank               =  160;
             new_mode_parameters.HFront               =   48;
             new_mode_parameters.HSync                =   32;
             new_mode_parameters.VTotal               = 2222;
             new_mode_parameters.VActive              = 2160;
             new_mode_parameters.VBlank               =   62;
             new_mode_parameters.VFront               =    3;
             new_mode_parameters.VSync                =    5;
             new_mode_parameters.mode_valid = 1;


			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.adjusted_frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
}
//            printf("Add extra format   3840x2160 144 fps  \n");

             new_mode_parameters.frame_rate           =  144;
             new_mode_parameters.adjusted_frame_rate  =  144;
             new_mode_parameters.pixel_clk            = 128102; //4000;
             new_mode_parameters.HTotal               = 4000;
             new_mode_parameters.HActive              = 3840;
             new_mode_parameters.HBlank               =  160;
             new_mode_parameters.HFront               =   48;
             new_mode_parameters.HSync                =   32;
             new_mode_parameters.VTotal               = 2224;
             new_mode_parameters.VActive              = 2160;
             new_mode_parameters.VBlank               =   64;
             new_mode_parameters.VFront               =    3;
             new_mode_parameters.VSync                =    5;
             new_mode_parameters.mode_valid = 1;


			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.adjusted_frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
}

//            printf("Add extra format   3840x2160 180 fps \n");

             new_mode_parameters.frame_rate           =  180;
             new_mode_parameters.adjusted_frame_rate  =  180;
             new_mode_parameters.pixel_clk            = 169560; //4000;
             new_mode_parameters.HTotal               = 4000;
             new_mode_parameters.HActive              = 3840;
             new_mode_parameters.HBlank               =  160;
             new_mode_parameters.HFront               =   48;
             new_mode_parameters.HSync                =   32;
             new_mode_parameters.VTotal               = 2355;
             new_mode_parameters.VActive              = 2160;
             new_mode_parameters.VBlank               =  195;
             new_mode_parameters.VFront               =    3;
             new_mode_parameters.VSync                =    5;
             new_mode_parameters.mode_valid = 1;


			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.adjusted_frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
}

//            printf("Add extra format    2560x1600 60 fps \n");

             new_mode_parameters.frame_rate           = 60;
             new_mode_parameters.adjusted_frame_rate  = 60;
             new_mode_parameters.pixel_clk            = 29376;
             new_mode_parameters.HTotal               = 2720;
             new_mode_parameters.HActive              = 2560;
             new_mode_parameters.HBlank               = 160;
             new_mode_parameters.HFront               = 48;
             new_mode_parameters.HSync                = 32;
             new_mode_parameters.VTotal               = 1800;
             new_mode_parameters.VActive              = 1600;
             new_mode_parameters.VBlank               = 200;
             new_mode_parameters.VFront               = 0x03;
             new_mode_parameters.VSync                = 0x06;
             new_mode_parameters.mode_valid           = 1;

			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 

            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
}
      //      printf("Add extra format    2560x1600 240 fps \n");

             new_mode_parameters.frame_rate           =   60;
             new_mode_parameters.adjusted_frame_rate  =  240;
             new_mode_parameters.pixel_clk            = 29376;
             new_mode_parameters.HTotal               = 2720;
             new_mode_parameters.HActive              = 2560;
             new_mode_parameters.HBlank               =  160;
             new_mode_parameters.HFront               =   48;
             new_mode_parameters.HSync                =   32;
             new_mode_parameters.VTotal               = 1800;
             new_mode_parameters.VActive              = 1600;
             new_mode_parameters.VBlank               =  200;
             new_mode_parameters.VFront               = 0x03;
             new_mode_parameters.VSync                = 0x06;
             new_mode_parameters.mode_valid           = 1;
			if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
			{
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;
			}
      //      printf("Add extra format    6296x1740 90 fps \n");
             new_mode_parameters.frame_rate           = 90;
             new_mode_parameters.adjusted_frame_rate  = 90;
             new_mode_parameters.pixel_clk            = 103481;
             new_mode_parameters.HTotal               = 6496;
             new_mode_parameters.HActive              = 6296;
             new_mode_parameters.HBlank               = 200;
             new_mode_parameters.HFront               = 80;
             new_mode_parameters.HSync                = 40;
             new_mode_parameters.VTotal               = 1770;
             new_mode_parameters.VActive              = 1740;
             new_mode_parameters.VBlank               = 30;
             new_mode_parameters.VFront               = 20;
             new_mode_parameters.VSync                = 2;
             new_mode_parameters.mode_valid = 1;

         if ( (dsc_data_pps_parameters->pic_height == new_mode_parameters.VActive) & (dsc_data_pps_parameters->pic_width == new_mode_parameters.HActive))
            {
            printf("Mode :%d  %dx%d %dfps \n", mode_index, new_mode_parameters.HActive , new_mode_parameters.VActive, new_mode_parameters.frame_rate);
            mode_param_array[mode_index] = new_mode_parameters; 
            updated_mode_parameters = mode_param_array[mode_index];
            mode_index++;

}  
   if (mode_index > 2) {
    printf("Multiple modes available. Pease select dsc1 to dsc%d \n", mode_index -1);
   }
        }




void load_standard_timing( unsigned int EDID_standard_timing,  int* mode_id, struct pps_parameters* dsc_data_pps_parameters, struct mode_parameters* selected_mode_parameters)
{
//struct mode_parameters standard_mode_parameters;
//int standard_mode_id;

   switch (EDID_standard_timing)
    {
    case 0xC0D1: {
   			
			//standard_mode_id = (int) mode_id;
           //printf("Mode ID  %d \n",  standard_mode_id);
           //printf("Mode ID  %d \n",  mode_id);
			//if ( (dsc_data_pps_parameters->pic_height == 1080) & (dsc_data_pps_parameters->pic_width == 1920))
			//{
            selected_mode_parameters->HActive      = 1920;
            selected_mode_parameters->HBlank       =  280;
            selected_mode_parameters->VActive      = 1080;
            selected_mode_parameters->VBlank       =   45;
            selected_mode_parameters->HFront       =   88;
            selected_mode_parameters->HSync        =   44;
            selected_mode_parameters->VFront       =    4;
            selected_mode_parameters->VSync        =    5;
            selected_mode_parameters->HTotal       = 2200;
            selected_mode_parameters->VTotal       = 1125;
            selected_mode_parameters->pixel_clk    =    0;
            selected_mode_parameters->frame_rate   =   60;
            selected_mode_parameters->mode_valid   =    1;


			    *mode_id += 1;
	       // }
        break;
    }
    case 0xC0E1: {
			if ( (dsc_data_pps_parameters->pic_height == 1152) & (dsc_data_pps_parameters->pic_width == 2048))
			{
            selected_mode_parameters->HActive      = 2048;
            selected_mode_parameters->HBlank       =  202;
            selected_mode_parameters->HTotal       = 2250;
            selected_mode_parameters->HSync        =   80;
            selected_mode_parameters->HFront       =   26;
            selected_mode_parameters->VActive      = 1152;
            selected_mode_parameters->VBlank       =   48;
            selected_mode_parameters->VTotal       = 1200;
            selected_mode_parameters->VFront       =    1;
            selected_mode_parameters->VSync        =    3;
            selected_mode_parameters->pixel_clk    =    0;
            selected_mode_parameters->frame_rate   =   60;
            selected_mode_parameters->mode_valid   =    1;


			    *mode_id += 1;
          printf("Mode increment \n");
	        }
        break;
    }
    case 0xb3: {

			if ( (dsc_data_pps_parameters->pic_height == 1050) & (dsc_data_pps_parameters->pic_width == 1680))
			{
            selected_mode_parameters->HActive      = 1680;
            selected_mode_parameters->HBlank       =  560;
            selected_mode_parameters->HTotal       = 2240;
            selected_mode_parameters->HSync        =  176;
            selected_mode_parameters->HFront       =  104;
            selected_mode_parameters->VActive      = 1050;
            selected_mode_parameters->VBlank       =   39;
            selected_mode_parameters->VTotal       = 1089;
            selected_mode_parameters->VFront       =    3;
            selected_mode_parameters->VSync        =    6;
            selected_mode_parameters->pixel_clk    =    0;
            selected_mode_parameters->frame_rate   =   60;
            selected_mode_parameters->mode_valid   =    1;

		    *mode_id += 1;
	        }
        break;
    }
 
    case 0xd1: {

			if ( (dsc_data_pps_parameters->pic_height == 1200) & (dsc_data_pps_parameters->pic_width == 1920))
			{
            selected_mode_parameters->HActive              = 1920;
            selected_mode_parameters->HBlank               =  672;
            selected_mode_parameters->HTotal               = 2592;
            selected_mode_parameters->HSync                =  200;
            selected_mode_parameters->HFront               =  136;
            selected_mode_parameters->VActive              = 1200;
            selected_mode_parameters->VBlank               =   48;
            selected_mode_parameters->VTotal               = 1200;
            selected_mode_parameters->VFront               =    1;
            selected_mode_parameters->VSync                =    3;
            selected_mode_parameters->pixel_clk            =    0;
            selected_mode_parameters->frame_rate           =   60;
            selected_mode_parameters->mode_valid             =    1;

            *mode_id += 1;
	        }
        break;
    }
    default: {
		    selected_mode_parameters->HActive      =    0;
            selected_mode_parameters->HBlank       =    0;
            selected_mode_parameters->VActive      =    0;
            selected_mode_parameters->VBlank       =    0;
            selected_mode_parameters->HFront       =    0;
            selected_mode_parameters->HSync        =    0;
            selected_mode_parameters->VFront       =    0;
            selected_mode_parameters->VSync        =    0;
            selected_mode_parameters->HTotal       =    0;
            selected_mode_parameters->VTotal       =    0;
            selected_mode_parameters->frame_rate   =    0;
            selected_mode_parameters->pixel_clk    =    0;
            selected_mode_parameters->mode_valid   =    0;
        break;
    }
    }

}

void read_pps_params(struct pps_parameters* dsc_data_pps_parameters)
{
    unsigned pps_data;
    unsigned pps_dsc_version;
    unsigned dsc_line_bytes;
    unsigned dsc_chunk_bytes;
 
    printf("Read PPS       \n");
    IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_CONTROL, 0);
    pps_data  = IORD(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_PPS);

    pps_dsc_version = ((pps_data & 0xFF000000) >> 24);
    printf("pps_dsc_version        %x \n",  pps_dsc_version);
    if ((pps_dsc_version == 0x11) | (pps_dsc_version == 0x12)) {

    IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_CONTROL, 1);
    pps_data  = IORD(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_PPS);

    dsc_data_pps_parameters->bpp        = ((pps_data & 0x03FF0000) >>  16);
    dsc_data_pps_parameters->pic_height =  (pps_data & 0x0000FFFF);

    IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_CONTROL, 2);
    pps_data  = IORD(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_PPS);

    dsc_data_pps_parameters->pic_width    = ((pps_data & 0xFFFF0000) >>  16);
    dsc_data_pps_parameters->slice_height =  (pps_data & 0x0000FFFF);

    IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_CONTROL, 3);
    pps_data  = IORD(DP_TX_DP_SOURCE_DSC_MGMT_BASE, DPTX_DSC_PPS);

    dsc_data_pps_parameters->slice_width  = ((pps_data & 0xFFFF0000) >> 16);
    dsc_data_pps_parameters->chunk_size  =  (pps_data & 0x0000FFFF) ;

    dsc_data_pps_parameters->slices_per_line      = dsc_data_pps_parameters->pic_width/dsc_data_pps_parameters->slice_width;
    dsc_data_pps_parameters->dsc_end_line         = dsc_data_pps_parameters->pic_height -1;
    dsc_data_pps_parameters->line_pixels_per_dsc_byte = 4;

    dsc_line_bytes = (dsc_data_pps_parameters->pic_width * dsc_data_pps_parameters->bpp);
    DBG_PRINTF("pic_width             %x \n",  dsc_data_pps_parameters->pic_width);
    DBG_PRINTF("bpp                   %x \n",  dsc_data_pps_parameters->bpp);
    DBG_PRINTF("dsc_line_bytes        %x \n",  dsc_line_bytes);
	
    dsc_line_bytes = dsc_line_bytes / 8;
    //bpp is actually 1/16 of bpp, so divide by 16
    dsc_line_bytes = dsc_line_bytes / 16;

    dsc_chunk_bytes = (dsc_data_pps_parameters->slice_width * dsc_data_pps_parameters->bpp);
    dsc_chunk_bytes = dsc_chunk_bytes / 8;
    //bpp is actually 1/16 of bpp, so divide by 16
    dsc_chunk_bytes = dsc_chunk_bytes / 16;
    
    dsc_data_pps_parameters->dsc_line_bytes       =  dsc_line_bytes;
 //   dsc_data_pps_parameters->dsc_line_length      = (dsc_line_bytes/16)  ;
    dsc_data_pps_parameters->dsc_line_length      = (dsc_line_bytes/16) -1 ;
   // dsc_data_pps_parameters->dsc_chunk_length     = (dsc_chunk_bytes/16) -1;
    dsc_data_pps_parameters->dsc_chunk_length     = dsc_chunk_bytes ; 

    DBG_PRINTF("Pic Height        %x \n",  dsc_data_pps_parameters->pic_height);
    DBG_PRINTF("Pic Width         %x \n",  dsc_data_pps_parameters->pic_width);
    DBG_PRINTF("Slice Height      %x \n",  dsc_data_pps_parameters->slice_height);
    DBG_PRINTF("Slice Width       %x \n",  dsc_data_pps_parameters->slice_width);
    DBG_PRINTF("Slices per line   %x \n",  dsc_data_pps_parameters->slices_per_line);
    DBG_PRINTF("dsc_end_line      %x \n",  dsc_data_pps_parameters->dsc_end_line);
    DBG_PRINTF("dsc_line_length   %x \n",  dsc_data_pps_parameters->dsc_line_length);
    DBG_PRINTF("dsc_chunk_length  %x \n",  dsc_data_pps_parameters->dsc_chunk_length);
    printf("DSC image %dx%d \n",  dsc_data_pps_parameters->pic_width,  dsc_data_pps_parameters->pic_height);

    } // Check for DSC version 1.1 or 1.2
else {
  printf("PPS data not found\n");
  printf("Please download a valid .dsc file containing the PPS data\n");
}
 
}

void         program_dsc_regs(struct mode_parameters* selected_mode_parameters, struct pps_parameters* dsc_data_pps_parameters)
{
             unsigned int  regval;
             unsigned int  HBlank;
             unsigned int  HTotal;
             unsigned int  valid_start_line;
             unsigned long MVid_calc;
             //unsigned int  NVid; // Unused, but for info
             unsigned int  NVid_div_0x1000;
             //unsigned int  link_rate; // Unused, but for info
             unsigned int  link_rate_div270;

            DBG_PRINTF("Pic Height        %x \n",  dsc_data_pps_parameters->pic_height);
            DBG_PRINTF("Pic Width         %x \n",  dsc_data_pps_parameters->pic_width);
            DBG_PRINTF("Slice Height      %x \n",  dsc_data_pps_parameters->slice_height);
            DBG_PRINTF("Slice Width       %x \n",  dsc_data_pps_parameters->slice_width);
            DBG_PRINTF("Slices per line   %x \n",  dsc_data_pps_parameters->slices_per_line);
            DBG_PRINTF("dsc_packet_start  %x \n",  dsc_data_pps_parameters->dsc_packet_start);
            DBG_PRINTF("dsc_packet_length %x \n",  dsc_data_pps_parameters->dsc_packet_length);
            DBG_PRINTF("dsc_packet_gap    %x \n",  dsc_data_pps_parameters->dsc_packet_gap);
            DBG_PRINTF("dsc_start_line    %x \n",  dsc_data_pps_parameters->dsc_start_line);
            DBG_PRINTF("dsc_end_line      %x \n",  dsc_data_pps_parameters->dsc_end_line);
            DBG_PRINTF("dsc_line_length   %x \n",  dsc_data_pps_parameters->dsc_line_length);
            DBG_PRINTF("dsc_chunk_length  %x \n",  dsc_data_pps_parameters->dsc_chunk_length);
            DBG_PRINTF("pixel clock       %x \n",  selected_mode_parameters->pixel_clk);

            HBlank           = selected_mode_parameters->HBlank;
            HTotal           = selected_mode_parameters->HTotal;

            valid_start_line = (selected_mode_parameters->VBlank - selected_mode_parameters->VFront);

            // NVid     = 0x8000; // Unused, but for info
            NVid_div_0x1000   = 8;
            link_rate_div270 = ((IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 21) & 0xff);
            //link_rate        = link_rate_div270 * 270; // Unused, but for info

            //#MVID 
            //pixel clk = (MVid/Nvid) * LS clk 
            // NVid = 0x8000
            //Calculate in steps to avoid overrun/underrun

            MVid_calc = selected_mode_parameters->pixel_clk * NVid_div_0x1000;// pix clk *8
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc * 0x10;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc / link_rate_div270;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc * 0x10;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc / 270;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc * 0x10;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);
            MVid_calc = MVid_calc / 10;
            DBG_PRINTF("MVid %lx \n",  MVid_calc);


            regval = HTotal;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, HTOTAL_REG,                    regval);

 //           regval =((HBlank) << 16) | (dsc_data_pps_parameters->pic_width + 8);
            regval =((HBlank) << 16) | (dsc_data_pps_parameters->pic_width + 0);
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, HSTART_REG,                    regval);

 
            regval = MVid_calc;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, MSA_MVID_REG,                  regval);
            DBG_PRINTF("Set MSA_MVID_REG        %x \n", regval);


            //#  dsc_packet_gap_reg - unused for NIOS control builds
            regval = selected_mode_parameters->dsc_packet_gap;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, PACKET_CTRL2_REG,              regval);

            //  dsc_start_line_reg ,  dsc_end_line_reg
            regval =(valid_start_line << 16) | dsc_data_pps_parameters->dsc_end_line;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, LINE_MARKERS_REG,              regval);

            regval =(dsc_data_pps_parameters->dsc_line_length << 16) | dsc_data_pps_parameters->dsc_chunk_length;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, CHUNK_CTRL_REG,                regval);

            regval =(dsc_data_pps_parameters->slices_per_line << 16) | 0x0AB0;
            DBG_PRINTF("regval %x \n",  regval);
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, CHUNK_CTRL_REG1,               regval);

            regval =0x80000000;
            IOWR(DP_TX_DP_SOURCE_DSC_MGMT_BASE, PPS_CTRL_REG,                  regval);


}

void program_cvo_tpg(int is_dsc, struct mode_parameters* selected_mode_parameters, struct pps_parameters* dsc_data_pps_parameters)
{
            unsigned int link_rate;
            unsigned int cvo_rate;
            unsigned int lane_count;
            unsigned long int clock_ratio;
            unsigned int frame_rate;
            unsigned int line_period;
            unsigned int line_clocks;
            unsigned int DSC_clocks;
            unsigned int DSC_data;

            unsigned int HActive;
            unsigned int HTotal;
            unsigned int HBlank;
            unsigned int DSC_data_line;

            unsigned int HB_START;
            unsigned int HB_END;
            unsigned int V1B_START;
            unsigned int V1B_END;
            unsigned int HS_START;
            //unsigned int HS_END;  //unused, but for info
            unsigned int V1S_VSTART;
            unsigned int V1S_HSTART;
            unsigned int V1S_VEND;
            unsigned int V1S_HEND;

            cvo_rate = 37125; //Should really read this from CVO

            DBG_PRINTF("Set format  %dx%d %dfps \n", selected_mode_parameters->HActive, selected_mode_parameters->VActive,selected_mode_parameters->adjusted_frame_rate  );
            link_rate = (((IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);

            DBG_PRINTF("HActive      %d \n",  selected_mode_parameters->HActive);
            DBG_PRINTF("HBlank       %d \n",  selected_mode_parameters->HBlank);
            DBG_PRINTF("VActive      %d \n",  selected_mode_parameters->VActive);
            DBG_PRINTF("VBlank       %d \n",  selected_mode_parameters->VBlank);
            DBG_PRINTF("HFront       %d \n",  selected_mode_parameters->HFront);
            DBG_PRINTF("HSync        %d \n",  selected_mode_parameters->HSync);
            DBG_PRINTF("VFront       %d \n",  selected_mode_parameters->VFront);
            DBG_PRINTF("VSync        %d \n",  selected_mode_parameters->VSync);
            DBG_PRINTF("HTotal       %d \n",  selected_mode_parameters->HTotal);
            DBG_PRINTF("VTotal       %d \n",  selected_mode_parameters->VTotal);
            DBG_PRINTF("frame_rate   %d \n",  selected_mode_parameters->frame_rate);

           if  (selected_mode_parameters->frame_rate == selected_mode_parameters->adjusted_frame_rate) {
           frame_rate =  selected_mode_parameters->frame_rate;
           }
           else { 
             if (selected_mode_parameters->adjusted_frame_rate == 0 )
            {
               frame_rate =  selected_mode_parameters->frame_rate;
            } else {
               frame_rate =  selected_mode_parameters->adjusted_frame_rate;  
            }
           }
           printf("Set format  %dx%d %dfps \n", selected_mode_parameters->HActive, selected_mode_parameters->VActive, frame_rate  );

            clock_ratio = (link_rate *10000) / cvo_rate; 
            lane_count = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 5) & 0x1f;
            line_period = 100000000 /(frame_rate *selected_mode_parameters->VTotal);
            line_clocks = line_period * cvo_rate;

            //Get this from PPS
            DSC_data_line = dsc_data_pps_parameters->dsc_line_bytes; 

            DSC_data   = DSC_data_line/lane_count; 
            DSC_clocks = (DSC_data *1000 ) / clock_ratio;

            DBG_PRINTF("Lane count  : %d\n", lane_count);
            DBG_PRINTF("clock_ratio = (%d *10000) / cvo_rate, %d\n", link_rate, cvo_rate );
            DBG_PRINTF("clock ratio : %ld\n", clock_ratio);
            DBG_PRINTF("line_period : %d\n", line_period);
            DBG_PRINTF("line_clocks : %d\n", line_clocks);
            DBG_PRINTF("DSC_clocks  : %d\n", DSC_clocks);

            // Do we need to adjust the values because we're sending
            // DSC data?
            if (is_dsc == 1) {
            HActive = DSC_clocks/10;
            HActive = HActive *4;

            //Calculate HTotal in steps to avoid overrun/underrun
            HTotal = (line_clocks / 100000) *4;
            DBG_PRINTF("HTotal       %d \n",  HTotal);
            HTotal = (line_clocks / 100) ;
            DBG_PRINTF("HTotal       %d \n",  HTotal);
            HTotal = HTotal *4;
            DBG_PRINTF("HTotal       %d \n",  HTotal);
            HTotal = HTotal / 1000;
            DBG_PRINTF("HTotal       %d \n",  HTotal);

           
            HBlank = HTotal - HActive;

            DBG_PRINTF("DSC HActive  : %d\n", HActive);
            DBG_PRINTF("DSC HBlank   : %d\n", HBlank);
            DBG_PRINTF("DSC HTotal   : %d\n", HTotal);

            } else {
            HActive = selected_mode_parameters->HActive;
            HBlank  = selected_mode_parameters->HBlank;
            HTotal  = selected_mode_parameters->HTotal;
           }

            DBG_PRINTF("VActive  : %d\n", selected_mode_parameters->VActive);
            DBG_PRINTF("VBlank   : %d\n", selected_mode_parameters->VBlank);
            DBG_PRINTF("VTotal   : %d\n", selected_mode_parameters->VTotal);

            IOWR(VVP_TPG_BASE, 72, HActive);
            IOWR(VVP_TPG_BASE, 73, selected_mode_parameters->VActive);
            IOWR(VVP_TPG_BASE, 74, 3);   //set to progressive
            IOWR(VVP_TPG_BASE, 84, 0);   //core select
            IOWR(VVP_TPG_BASE, 85, 0);   //pre user
            IOWR(VVP_TPG_BASE, 86, 0);   //post user
            IOWR(VVP_TPG_BASE, 87, 0);   //output colors
            IOWR(VVP_TPG_BASE, 88, 0);   
            IOWR(VVP_TPG_BASE, 89, 0);   
            IOWR(VVP_TPG_BASE, 90, 0);   //set bars to zero
            IOWR(VVP_TPG_BASE, 83, 1);   //commit
            IOWR(VVP_TPG_BASE, 82, 1);   //start
            DBG_PRINTF("Programmed TPG\n");


            HB_START = 0;
            HB_END     = HBlank -HB_START; 
            HS_START   = HB_START + selected_mode_parameters->HFront;
            // For info, but unused
            // HS_END     = HS_START + selected_mode_parameters->HSync;

            V1B_START  = selected_mode_parameters->VBlank -1;
            V1B_END    = selected_mode_parameters->VActive + V1B_START;
            V1S_HSTART = HS_START;
            V1S_HEND   = HS_START;
            V1S_VSTART = selected_mode_parameters->VFront;
            V1S_VEND   = V1S_VSTART + selected_mode_parameters->VSync;

            DBG_PRINTF("HB_START             %x \n", HB_START);
            DBG_PRINTF("HBlank               %x \n", HBlank);
            DBG_PRINTF("HB_END               %x \n", HB_END);
            DBG_PRINTF("HTotal               %x \n", HTotal);

            DBG_PRINTF("V1B_START            %x \n", V1B_START);
            DBG_PRINTF("V1B_END              %x \n", V1B_END);
            DBG_PRINTF("V1S_HSTART           %x \n", V1S_HSTART);
            DBG_PRINTF("V1S_HEND             %x \n", V1S_HEND);
            DBG_PRINTF("V1S_VSTART           %x \n", V1S_VSTART);
            DBG_PRINTF("V1S_VEND             %x \n", V1S_VEND);
            DBG_PRINTF("VTotal               %x \n", selected_mode_parameters->VTotal);


            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_TOTALS_REG,    (((selected_mode_parameters->VTotal) << 16) + (HTotal)));
            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_HB_END_REG,                             HB_END        );
            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_V1B_POS_REG,   ((V1B_START     << 16) + V1B_END      ));

            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_HS_POS_REG,    ((132   << 16)         + HS_START     ));
            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_V1S_START_REG, ((V1S_VSTART    << 16) + V1S_HSTART   ));
            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_V1S_END_REG,   ((V1S_VEND      << 16) + V1S_HEND     ));
 
            IOWR(INTEL_VVP_CVO_0_BASE, VVP_CVO_COMMIT_REG, 0x01);


}

#endif

  char    check_4K_dsc( unsigned int base_addr)
       {
            // Check that we can go into DSC mode without actually switching over
            unsigned int link_rate;
            unsigned int lane_count;
   
            BYTE dpcd_val[6];
            BYTE sink_supports_Enhanced_Framing;
            BYTE sink_supports_FEC;
            BYTE sink_supports_DSC_decode;
            BYTE sink_link_has_4lanes;
            BYTE sink_link_is_8G;
            BYTE sink_supports_DSC;
            char sink_supports_4K_DSC;
            
            
            sink_supports_Enhanced_Framing  = 0;
            sink_supports_FEC               = 0;
            sink_supports_DSC_decode        = 0;
            sink_link_has_4lanes            = 0;
            sink_link_is_8G                 = 0;
            sink_supports_DSC               = 0;
            sink_supports_4K_DSC            = 0;

            // Check Sink is capable of enhanced framing, FEC and DSC
            
            printf("=================================================\n");
            btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, dpcd_val);
            sink_supports_Enhanced_Framing = (dpcd_val[0] & 0x80) >> 7;
            usleep(50000); 
             
             // Check Sink FEC capability
            btc_dptx_aux_read(0,DPCD_ADDR_FEC_CAPABILITY,1,dpcd_val);
            sink_supports_FEC = (dpcd_val[0] & 0x01); 
            usleep(50000); 
            
             // Check Sink suports DSC
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_SUPPORT, 1, dpcd_val);
            sink_supports_DSC_decode = (dpcd_val[0] & 0x01);
            usleep(50000); 
            
 
            // Check link rate supports high rates for 4K144 dsc
            link_rate = (((IORD(base_addr, DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);
            printf("Link rate  : %d\n", link_rate);
            if (link_rate == 8100) {
                sink_link_is_8G = 1;
            }
               
            lane_count = (IORD(base_addr, DPTX_REG_TX_CONTROL) >> 5) & 0x1f;
            printf("Lane count : %d\n", lane_count);
            if (lane_count == 4) {
                sink_link_has_4lanes = 1;
            }
             
             
            // If the sink supports DSC correctly then:
            // Enable DSC in DPCD reg
            // Enable FEC in DP Tx
            // Enable Compressed stream in DP Tx
            // Enable PPS messages in DP Tx
            
             if ( (sink_supports_Enhanced_Framing == 0x01) && (sink_supports_FEC == 0x01) && (sink_supports_DSC_decode == 0x01)) {
                printf("Sink Supports DSC  \n");
                sink_supports_DSC = 1;
                  if ( (sink_link_has_4lanes == 0x01) && (sink_link_is_8G == 0x01) ) {
                     printf("Link established Supports 4K DSC  \n");
                     sink_supports_4K_DSC = 1;
                  } else {
                     printf("..but Link established does not Support 4K DSC  \n");
                     sink_supports_4K_DSC = 0;
                  }
                } else {
                printf("Sink Does Not Support DSC  \n");
                sink_supports_DSC = 1;
                
                }
                
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
                printf("Sink Supports DSC decode\n");
            } else {
                printf("Sink Does Not Support DSC decode\n");
            }
            
            if (sink_link_has_4lanes == 0x01) {
                printf("Link has 4 lanes\n");
            } else {
                printf("Link does not have 4 lanes\n");
            }
            if (sink_link_is_8G == 0x01) {
                printf("Link is 8.1G\n");
            } else {
                printf("Link not 8.1G -not fast enough for 4K DSC\n");
            }
            printf("=================================================\n");
 
           return sink_supports_4K_DSC;
         }


void enable_dsc ()
{
           unsigned d;
            BYTE dpcd_val[6];
            BYTE sink_supports_Enhanced_Framing;
            BYTE sink_supports_FEC;
            BYTE sink_supports_DSC;
            
            
            sink_supports_Enhanced_Framing  = 0;
            sink_supports_FEC               = 0;
            sink_supports_DSC               = 0;
            // Check Sink is capable of enhanced framing, FEC and DSC before enabling
            
            btc_dptx_aux_read(0, DPCD_ADDR_LANE_COUNT_SET, 1, dpcd_val);
            sink_supports_Enhanced_Framing = (dpcd_val[0] & 0x80) >> 7;
            usleep(50000); 
             
             // Check Sink FEC capability
            btc_dptx_aux_read(0,DPCD_ADDR_FEC_CAPABILITY,1,dpcd_val);
            sink_supports_FEC = (dpcd_val[0] & 0x01); 
            usleep(50000); 
            
             // Check Sink suports DSC
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_SUPPORT, 1, dpcd_val);
            sink_supports_DSC = (dpcd_val[0] & 0x01);
            usleep(50000); 

            printf("Link rate  : %d Mbps\n",
            ((IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);
           
            // If the sink supports DSC correctly then:
            // Enable DSC in DPCD reg
            // Enable FEC in DP Tx
            // Enable Compressed stream in DP Tx
            // Enable PPS messages in DP Tx
            
             if ( (sink_supports_FEC == 0x01) && (sink_supports_DSC == 0x01)) {
                printf("Enabling DSC  \n");
   
            
            dpcd_val[0] = 1;
            btc_dptx_aux_write(0, DPCD_ADDR_DSC_ENABLE, 1, dpcd_val);
            usleep(50000); 
            
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_ENABLE, 1, dpcd_val);
            printf("DPCD Sink DSC Enable : %8.8x  \n", dpcd_val[0]);
            usleep(50000); 

#if (DP_SUPPORT_RX_DSC == 0)
            // If Tx only set DSC control reg for DSC and enable PPS
            // We don't need to do this if it's a passthrough design
            // 7 - Fec Enable
            // 6 - DSC Enable
            d = 0x800000C0;

            IOWR(btc_dptx_baseaddr(0), DPTX0_REG_DSC_CONTROL, d); 
            usleep(50000);
             d  = IORD(btc_dptx_baseaddr(0), DPTX0_REG_DSC_CONTROL);
            printf("PPS, FEC and DSC enabled in DP Tx\n "  );
            
            usleep(50000);
            
            // When DSC TX is now output, set PPS update in control register again
            // This triggers the payload TU re-calculation in the packetizer
            d = 0x800000C0;
            IOWR(btc_dptx_baseaddr(0), DPTX0_REG_DSC_CONTROL, d); 
            usleep(50000);
          
            
            printf("PPS enabled \n "  );
#endif // (DP_SUPPORT_RX_DSC == 0)

            printf("DSC enabled \n "  );
            printf("Select dsc- to disable DSC  \n "  );

            
                } else {
                printf("Cannot enable DSC  \n");
                
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
}

void disable_dsc (){
            //Disable DSC
            unsigned d;
            BYTE dpcd_val[6];

 #if (DP_SUPPORT_RX_DSC == 0)
            // If Tx only clear DSC control reg
            // We don't need to do this if it's a passthrough design

            d = 0x00;
            IOWR(btc_dptx_baseaddr(0), DPTX0_REG_DSC_CONTROL, d);
            usleep(50000);
             d  = IORD(btc_dptx_baseaddr(0), DPTX0_REG_DSC_CONTROL);

 #endif
            dpcd_val[0] = 0;
            btc_dptx_aux_write(0, DPCD_ADDR_DSC_ENABLE, 1, dpcd_val);
            usleep(50000);

            btc_dptx_aux_read(0, DPCD_ADDR_DSC_ENABLE, 1, dpcd_val);

            if ((dpcd_val[0] && 0x01) == 0x00) {
                   printf("DSC disabled \n");
            }
            usleep(50000);
         }


void dump_dsc_dpcd () {
            BYTE dpcd_val[6];


            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DPCD_REV,                        1, dpcd_val);
            printf("DPCD_ADDR_DPCD_REV                        : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_MAX_LINK_RATE,                   1, dpcd_val);
            printf("DPCD_ADDR_MAX_LINK_RATE                   : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_MAX_LANE_COUNT,                  1, dpcd_val);
            printf("DPCD_ADDR_MAX_LANE_COUNT                  : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_MAX_DOWNSPREAD,                  1, dpcd_val);
            printf("DPCD_ADDR_MAX_DOWNSPREAD                  : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_NORP,                            1, dpcd_val);
            printf("DPCD_ADDR_NORP                            : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DOWNSTREAMPORT_PRESENT,          1, dpcd_val);
            printf("DPCD_ADDR_DOWNSTREAMPORT_PRESENT          : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_MAIN_LINK_CHANNEL_CODING,        1, dpcd_val);
            printf("DPCD_ADDR_MAIN_LINK_CHANNEL_CODING        : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DOWN_STREAM_PORT_COUNT,          1, dpcd_val);
            printf("DPCD_ADDR_DOWN_STREAM_PORT_COUNT          : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_RECEIVE_PORT0_CAP_0,             1, dpcd_val);
            printf("DPCD_ADDR_RECEIVE_PORT0_CAP_0             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_RECEIVE_PORT0_CAP_1,             1, dpcd_val);
            printf("DPCD_ADDR_RECEIVE_PORT0_CAP_1             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_RECEIVE_PORT1_CAP_0,             1, dpcd_val);
            printf("DPCD_ADDR_RECEIVE_PORT1_CAP_0             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_RECEIVE_PORT1_CAP_1,             1, dpcd_val);
            printf("DPCD_ADDR_RECEIVE_PORT1_CAP_1             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_I2C_SPEED,                       1, dpcd_val);
            printf("DPCD_ADDR_I2C_SPEED                       : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EDP_CONFIGURATION_CAP,           1, dpcd_val);
            printf("DPCD_ADDR_EDP_CONFIGURATION_CAP           : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_TRAINING_AUX_RD_INTERVAL,        1, dpcd_val);
            printf("DPCD_ADDR_TRAINING_AUX_RD_INTERVAL        : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_ADAPTER_CAP,                     1, dpcd_val);
            printf("DPCD_ADDR_ADAPTER_CAP                     : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_SUPPORTED_LINK_RATES,            1, dpcd_val);
            printf("DPCD_ADDR_SUPPORTED_LINK_RATES            : %8.8x  \n", dpcd_val[0]);

            printf("====Extended Capabilities==========================\n");
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXTENDED_CAPAB_FIELD,            1, dpcd_val);
            printf("DPCD_ADDR_EXTENDED_CAPAB_FIELD            : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXT_DPCD_REV,                    1, dpcd_val);
            printf("DPCD_ADDR_EXT_DPCD_REV                    : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXT_MAX_LINK_RATE,               1, dpcd_val);
            printf("DPCD_ADDR_EXT_MAX_LINK_RATE               : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXT_MAX_LANE_COUNT,              1, dpcd_val);
            printf("DPCD_ADDR_EXT_MAX_LANE_COUNT              : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXT_MAX_DOWNSPREAD,              1, dpcd_val);
            printf("DPCD_ADDR_EXT_MAX_DOWNSPREAD              : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_EXT_NORP,                        1, dpcd_val);
            printf("DPCD_ADDR_EXT_NORP                        : %8.8x  \n", dpcd_val[0]);

            printf("===================================================\n");

            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_SUPPORT,                    1, dpcd_val);
            printf("DPCD_ADDR_DSC_SUPPORT                    : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_ALGORITHM_REVISION,         1, dpcd_val);
            printf("DPCD_ADDR_DSC_ALGORITHM_REVISION         : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_RC_BUFFER_BLOCK_SIZE,       1, dpcd_val);
            printf("DPCD_ADDR_DSC_RC_BUFFER_BLOCK_SIZE       : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_RC_BUFFER_SIZE,             1, dpcd_val);
            printf("DPCD_ADDR_DSC_RC_BUFFER_SIZE             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_SLICE_CAPABILITIES_1,       1, dpcd_val);
            printf("DPCD_ADDR_DSC_SLICE_CAPABILITIES_1       : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_LINE_BUFFER_DEPTH,          1, dpcd_val);
            printf("DPCD_ADDR_DSC_LINE_BUFFER_DEPTH          : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_BLOCK_PREDICTION_SUPPORT,   1, dpcd_val);
            printf("DPCD_ADDR_DSC_BLOCK_PREDICTION_SUPPORT   : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_DECODER_COLOR_FORMAT_CAPAB, 1, dpcd_val);
            printf("DPCD_ADDR_DSC_DECODER_COLOR_FORMAT_CAPAB : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_DECODER_COLOR_DEPTH_CAPAB,  1, dpcd_val);
            printf("DPCD_ADDR_DSC_DECODER_COLOR_DEPTH_CAPAB  : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_PEAK_DSC_THROUGHPUT,            1, dpcd_val);
            printf("DPCD_ADDR_PEAK_DSC_THROUGHPUT            : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_MAXIMUM_SLICE_WIDTH,        1, dpcd_val);
            printf("DPCD_ADDR_DSC_MAXIMUM_SLICE_WIDTH        : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_DSC_SLICE_CAPABILITIES_2,       1, dpcd_val);
            printf("DPCD_ADDR_DSC_SLICE_CAPABILITIES_2       : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_BITS_PER_PIXEL_INCREMENT,       1, dpcd_val);
            printf("DPCD_ADDR_BITS_PER_PIXEL_INCREMENT       : %8.8x  \n", dpcd_val[0]);
            usleep(50000);


            printf("===================================================\n");
            btc_dptx_aux_read(0, DPCD_ADDR_FEC_CAPABILITY,                 1, dpcd_val);
            printf("DPCD_ADDR_FEC_CAPABILITY                 : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_FEC_CONFIGURATION,              1, dpcd_val);
            printf("DPCD_ADDR_FEC_CONFIGURATION              : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_FEC_STATUS,                     1, dpcd_val);
            printf("DPCD_ADDR_FEC_STATUS                     : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_FEC_ERROR_COUNT_LS,             1, dpcd_val);
            printf("DPCD_ADDR_FEC_ERROR_COUNT_LS             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_FEC_ERROR_COUNT_MS,             1, dpcd_val);
            printf("DPCD_ADDR_FEC_ERROR_COUNT_MS             : %8.8x  \n", dpcd_val[0]);
            usleep(50000);
}
