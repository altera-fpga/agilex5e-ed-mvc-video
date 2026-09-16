#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "intel_axi2cv.h"
#include "intel_cv2axi.h"
#include "system.h"
#include "intel_fpga_i2c.h"
#include <stdlib.h>  // For exit()

#define TIMEOUT_LIMIT 100000  // Adjust this value as needed
#define RAW_I2C_SLAVE_ADDR  0x17   // <-- Replace with your actual slave address

void menu_cmd();
unsigned char *get_stdin();
unsigned char int_stdbuf[20];
unsigned char int_stdbuf_ptr = 0;

struct color_info {
    int space;
    int depth;
    int wcg;   /* SDR color (bt.709/sRGB) or HDR gamut (bt2020) */
};
struct image_config {
    int x;
    int y;
    bool interlace;
    struct color_info image_color;
    unsigned int std;
    unsigned int vpid_byte1;
    unsigned int vpid_byte2;
    unsigned int vpid_byte3;
    unsigned int vpid_byte4;
};

intel_cv2axi_instance cv2axi; 
intel_axi2cv_instance axi2cv;

//MVC
// void cvi_res_switch(intel_cv2axi_instance* cvi, intel_axi2cv_instance* cvo, struct image_config *current_image_config);

void cvi_res_switch(intel_cv2axi_instance* cvi,
                    struct image_config* current_image_config);

void cvo_res_switch(intel_axi2cv_instance* cvo, unsigned int dim_x, unsigned int dim_y, unsigned int fps_val); 


void cvo_program_mode_bank(intel_axi2cv_instance* cvo, unsigned int sel, unsigned int vid_std, unsigned int vpid_byte1, unsigned int vpid_byte2, unsigned int vpid_byte3, unsigned int vpid_byte4);
void send_raw_cmd_masked(long i2c_base, unsigned char slave_addr, unsigned char reg, unsigned char data, unsigned char mask);

// MVC
int res_switch = 1;
int res_switch_tx = 1;

int prev_vid_standard = -1;
struct image_config cvi_image_config = {.x = 0, .y = 0, .interlace = false, .image_color.space = 0, .image_color.depth = 0, .image_color.wcg = 0, .std=0, .vpid_byte1=0, .vpid_byte2=0, .vpid_byte3=0, .vpid_byte4=0};

int main()
{
    
    printf("Wait 50mS for SI569 ...\n");
    usleep(50000);    
    printf("It's all Ready :) \n");
    
    // MVC
    unsigned int sdi_tx_fps              = 0;
    unsigned int sdi_tx_actv_dim_x       = 0;
    unsigned int sdi_tx_actv_dim_y       = 0;
    unsigned int sdi_tx_actv_dim         = 0;
    // unsigned int prev_sdi_tx_new_dim_x   = 0;
    // unsigned int prev_sdi_tx_new_dim_y   = 0;
    // unsigned int prev_sdi_tx_new_fps     = 0;
    unsigned int sdi_tx_new_fps          = 0;
    unsigned int sdi_tx_new_dim_x        = 0;
    unsigned int sdi_tx_new_dim_y        = 0;  
    
    printf("Started...\n");
    intel_fpga_i2c_init(I2C_0_BASE, I2C_0_FREQ);
    // Force non-blocking jtag uart
    int res = 0;
    res = fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    res = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    if (res == -1) {
        printf("FCNTL Failed\n");
    }

  intel_vab_core_base cv2axi_addr_base = (intel_vab_core_base)(SDI_RX_MR_BASE);
  intel_cv2axi_init (&cv2axi, cv2axi_addr_base);
  intel_vab_core_base axi2cv_addr_base = (intel_vab_core_base)(SDI_TX_MR_BASE);
  intel_axi2cv_init (&axi2cv, axi2cv_addr_base);
  
  while (1) {
      int cur_vid_standard = intel_cv2axi_get_vid_standard(&cv2axi);

      if (cur_vid_standard == 8 && prev_vid_standard != 8) {
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0xFF, 0x04, 0x07); // RAW FF 04 07
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0xA0, 0x0F, 0x1F); // RAW A0 0F 1F
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0x0A, 0x0C, 0x0C); // RAW 0A 0C 0C
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0x0A, 0x00, 0x0C); // RAW 0A 00 0C
          printf("\n");
      }
      else if (cur_vid_standard != 8 && prev_vid_standard == 8) {
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0xA0, 0x1F, 0x1F); // RAW A0 1F 1F
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0x0A, 0x0C, 0x0C); // RAW 0A 0C 0C
          send_raw_cmd_masked(I2C_0_BASE, RAW_I2C_SLAVE_ADDR, 0x0A, 0x00, 0x0C); // RAW 0A 00 0C
          printf("\n");
      }
      prev_vid_standard = cur_vid_standard;
     // Serve menu commands, if any
     menu_cmd();
     
    // SDI Tx MVC
    sdi_tx_actv_dim_x = (IORD(SDI_TX_MR_BASE, 0x55) & (0xFFFF));
    sdi_tx_actv_dim_y = (IORD(SDI_TX_MR_BASE, 0x56) & (0xFFFF));

    sdi_tx_actv_dim  = ( (sdi_tx_fps << 24) | (sdi_tx_actv_dim_x << 12) | (sdi_tx_actv_dim_y));

    // PIO Output to VVP_SS
    IOWR(SDITX_PIO_ACTV_DIM_BASE, 0x0, sdi_tx_actv_dim);

    // PIO Input from VVP_SS
    sdi_tx_new_fps   = (IORD(SDITX_PIO_FPS_BASE, 0x0)) & (0x7F);  
    sdi_tx_new_dim_x = (IORD(SDITX_PIO_NEW_ACTV_DIM_BASE, 0x0) >> 16) & (0xFFFF);        
    sdi_tx_new_dim_y = (IORD(SDITX_PIO_NEW_ACTV_DIM_BASE, 0x0)) & (0xFFFF);     

    // DP Rx CVI loop MVC
    if (res_switch == 0 && (intel_cv2axi_is_locked(&cv2axi) == 0 ||
                            intel_cv2axi_is_valid_resolution(&cv2axi) == 0 ||
                            intel_cv2axi_is_stream_stable(&cv2axi) == 0))
    {
        res_switch = 1;
        intel_cv2axi_stop(&cv2axi);
    }     
    if (res_switch == 1 && intel_cv2axi_is_stream_stable(&cv2axi) == 1 &&
        intel_cv2axi_is_valid_resolution(&cv2axi) == 1 && intel_cv2axi_is_locked(&cv2axi) == 1)
    {
        res_switch = 0;
        cvi_res_switch(&cv2axi, &cvi_image_config);
    }
     
    // SDI Tx CVO loop MVC  
    if (res_switch_tx == 0 && ( (sdi_tx_new_fps   != sdi_tx_fps) ||
                                (sdi_tx_new_dim_x != sdi_tx_actv_dim_x) ||
                                (sdi_tx_new_dim_y != sdi_tx_actv_dim_y) ))
    {
        res_switch_tx = 1;
    }
    if ( (res_switch_tx == 1) )
    {
        res_switch_tx = 0;
        cvo_res_switch(&axi2cv, sdi_tx_new_dim_x, sdi_tx_new_dim_y, sdi_tx_new_fps);
        sdi_tx_fps = sdi_tx_new_fps;
    }

  }
  return 0; // Should never get here
}

unsigned char *get_stdin()
{
  int d,i;
  char *c = (char *)&d;

  i = read(0,(void*)&d,1); // 0 = stdin
  if((i < 1) || (*c == EOF))
    return NULL;

  printf("%c",d);
  if(*c == '\n' || *c == 0xd)
  {
    // input is complete
    if(*c == 0xd)
      printf("\n");
    int_stdbuf[int_stdbuf_ptr] = 0x00;
    int_stdbuf_ptr = 0;
    return int_stdbuf;
  }

  int_stdbuf[int_stdbuf_ptr++] = *c;
  if((int_stdbuf_ptr + 1) == sizeof(int_stdbuf))
  {
    // buffer is full
    int_stdbuf[int_stdbuf_ptr] = 0x00;
    int_stdbuf_ptr = 0;
    return int_stdbuf;
  }

  return NULL;
}
// Implementation of menu commands
void menu_cmd()
{
	unsigned char *cmd;

	cmd = get_stdin();
    if(cmd != NULL)
    {
        if(cmd[0] == 'p')
        {
          printf("\nCVI Video Info\n");
          unsigned int datan = intel_cv2axi_get_active_sample_count(&cv2axi);
          printf("get_active_sample_count(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_is_stream_stable(&cv2axi);
          printf("is_stream_stable(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_total_sample_count(&cv2axi);
          printf("get_total_sample_count(&cv2axi) is %d\n",datan );
          datan = intel_cv2axi_get_active_line_count_f0(&cv2axi);
          printf("get_active_line_count_f0(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_active_line_count_f1(&cv2axi);
          printf("get_active_line_count_f1(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_is_interlaced(&cv2axi);
          printf("is_interlaced(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_is_valid_resolution(&cv2axi);
          printf("is_valid_resolution(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_is_locked(&cv2axi);
          printf("is_locked(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_color_pattern(&cv2axi);
          printf("get_color_pattern(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_vpid(&cv2axi, 1);
          printf("get_vpid_byte1(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_vpid(&cv2axi, 2);
          printf("get_vpid_byte2(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_vpid(&cv2axi, 3);
          printf("get_vpid_byte3(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_vpid(&cv2axi, 4);
          printf("get_vpid_byte4(&cv2axi) is %d\n", datan);
          datan = intel_cv2axi_get_vid_standard(&cv2axi);
          printf("get_vid_standrad(&cv2axi) is %d\n", datan);
          
          printf("SDI Tx Actv Pixels %d\n", (IORD(SDI_TX_MR_BASE, 0x55) & (0xFFFF)));
          printf("SDI Tx Actv Lines %d\n",  (IORD(SDI_TX_MR_BASE, 0x56) & (0xFFFF)));
        }
   }
}

//--Program the supported resolution to the cvo mode bank
void cvo_program_mode_bank(intel_axi2cv_instance* cvo, unsigned int sel, unsigned int vid_std, unsigned int vpid_byte1, unsigned int vpid_byte2, unsigned int vpid_byte3, unsigned int vpid_byte4) {
   switch (sel) {
      case 0:
         //intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_480P_MODE,0,0,0,0,0);
         break;
      case 1:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_720P_SDI_MODE,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 2:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_1080I_SDI_MODE,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 3:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_1080P_SDI_MODE_1920,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 4:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_2160P_SDI_MODE_3840,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 5:
         //intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_2160P_420_MODE,0,0,0,0,0);
         break;
      case 6:
         //intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_4320P_MODE,0,0,0,0,0);
         break;
      case 7:
         //intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_4320P_420_MODE,0,0,0,0,0);
         break;
      case 8:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_576I_SDI_MODE,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 9:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_486I_SDI_MODE,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 10:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_1080P_SDI_MODE_2048,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      case 11:
         intel_axi2cv_set_output_mode_sdi(cvo,0,CVO_2160P_SDI_MODE_4096,vid_std,vpid_byte1, vpid_byte2, vpid_byte3, vpid_byte4);
         break;
      default:
         break;
   }
}

// MVC
void cvi_res_switch(intel_cv2axi_instance* cvi,
                    struct image_config* current_image_config) {
// void cvi_res_switch(intel_cv2axi_instance* cvi, intel_axi2cv_instance* cvo, struct image_config *current_image_config) {
   struct image_config new_image_config = {.x = 0, .y = 0, .interlace = false, .image_color.space = 0, .image_color.depth = 0, .image_color.wcg = 0, .std=0, .vpid_byte1=0, .vpid_byte2=0, .vpid_byte3=0, .vpid_byte4=0};
   int res_changed = 0;

   new_image_config.x = intel_cv2axi_get_active_sample_count(cvi);  //-- Width
   new_image_config.y = intel_cv2axi_get_active_line_count_f0(cvi); //-- Using f0 height only

   new_image_config.interlace = intel_cv2axi_is_interlaced(cvi);
   new_image_config.image_color.space = intel_cv2axi_get_color_pattern(cvi) >> 7;
   new_image_config.image_color.depth = intel_cv2axi_get_bit_width(cvi);
   new_image_config.std = intel_cv2axi_get_vid_standard(cvi);
   new_image_config.vpid_byte1 = intel_cv2axi_get_vpid(cvi, 1);
   new_image_config.vpid_byte2 = intel_cv2axi_get_vpid(cvi, 2);
   new_image_config.vpid_byte3 = intel_cv2axi_get_vpid(cvi, 3);
   new_image_config.vpid_byte4 = intel_cv2axi_get_vpid(cvi, 4);

   if (new_image_config.interlace) {
      new_image_config.y += intel_cv2axi_get_active_line_count_f1(cvi); // height is f0 + f1
   }

   //Compare current_image_config and new_image_config
   if (new_image_config.x != current_image_config->x || 
       new_image_config.y != current_image_config->y || 
       new_image_config.interlace != current_image_config->interlace ||
       new_image_config.std != current_image_config->std ||
       new_image_config.vpid_byte1 != current_image_config->vpid_byte1 ||
       new_image_config.vpid_byte2 != current_image_config->vpid_byte2 ||
       new_image_config.vpid_byte3 != current_image_config->vpid_byte3 ||
       new_image_config.vpid_byte4 != current_image_config->vpid_byte4 ) {

      res_changed = 1;
   }
   
   if (res_changed) {
      current_image_config->x = new_image_config.x;
      current_image_config->y = new_image_config.y;
      current_image_config->interlace = new_image_config.interlace;
      current_image_config->image_color = new_image_config.image_color;
      current_image_config->std = new_image_config.std;
      current_image_config->vpid_byte1 = new_image_config.vpid_byte1;
      current_image_config->vpid_byte2 = new_image_config.vpid_byte2;
      current_image_config->vpid_byte3 = new_image_config.vpid_byte3;
      current_image_config->vpid_byte4 = new_image_config.vpid_byte4;
   }      
   
   intel_cv2axi_start(cvi);
}

// MVC
void cvo_res_switch(intel_axi2cv_instance* cvo, unsigned int dim_x, unsigned int dim_y, unsigned int fps_val)
{
    
// HD 1280x720 P60 10b YUV422
// vpid_byte1 = 132 
// vpid_byte2 = 203
// vpid_byte3 = 128
// vpid_byte4 = 1
// STD        = 1

// 3G 1920x1080 P60 10b YUV422
// vpid_byte1 = 137 
// vpid_byte2 = 203
// vpid_byte3 = 128
// vpid_byte4 = 1
// STD        = 3

// 6G 3840x2160 P60 10b YUV422
// vpid_byte1 = 192 
// vpid_byte2 = 199
// vpid_byte3 = 128
// vpid_byte4 = 1
// STD        = 21

// 12G 3840x2160 P60 10b YUV422
// vpid_byte1 = 206 
// vpid_byte2 = 203
// vpid_byte3 = 128
// vpid_byte4 = 1
// STD        = 23
    
     switch (dim_y) 
     {
        case 720:
              cvo_program_mode_bank(cvo,1, 1, 132, 203, 128, 1);
           break;
        case 1080:
              cvo_program_mode_bank(cvo,3, 3, 137, 203, 128, 1);
           break;
        case 2160:
            if (fps_val == 30) {
              cvo_program_mode_bank(cvo,4, 4, 192, 199, 128, 1);
            }
            else {
              cvo_program_mode_bank(cvo,4, 23, 206, 203, 128, 1);
            }
           break;
        default:
           break;
     }
     
}

// Write to a register using data/mask logic
void send_raw_cmd_masked(long i2c_base, unsigned char slave_addr, unsigned char reg, unsigned char data, unsigned char mask) {
    unsigned char orig = intel_fpga_i2c_read_extended(i2c_base, slave_addr, reg);
    unsigned char newval = (orig & ~mask) | (data & mask);
    intel_fpga_i2c_write_extended(i2c_base, slave_addr, reg, newval);
    printf("RAW %02X %02X %02X : orig=%02X, new=%02X\n", reg, data, mask, orig, newval);
}

