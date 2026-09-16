#include <stdio.h>
#include <system.h>
#include <io.h>
#include <unistd.h>
#include "alt_types.h"
#include "intel_fpga_i2c.h"
#include "system.h"

void print_si5391a(unsigned int base_addr)
{

    alt_u8 addr = 0x74;

    int i, tmp, tmp2, tmp3, r, c;
    int plsb, pmsb, den;

    printf("Si5391 Configuration\n");
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x0);
    tmp = intel_fpga_i2c_read_extended(base_addr, addr, 0x0021);
    printf("IN_SEL = %i\n", ((tmp&0x1)?((tmp&0x6)>>1):4));

    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x2);
    tmp = intel_fpga_i2c_read_extended(base_addr, addr, 6);
    printf("Pxarb = 0x%X (divide by %i)\n", (tmp&0x3),1<<(tmp&0x3));
    for(i = 0; i < 4; i++){
      plsb = intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+8);
      plsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+9))<<8;
      plsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+10))<<16;
      pmsb = intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+11);
      pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+12))<<8;
      pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*10)+13))<<16;
      printf("P%i = 0x%06X%06X\n", i, pmsb, plsb);
    }
    plsb = intel_fpga_i2c_read_extended(base_addr, addr, 0x35);
    plsb += (intel_fpga_i2c_read_extended(base_addr, addr, 0x36))<<8;
    plsb += (intel_fpga_i2c_read_extended(base_addr, addr, 0x37))<<16;
    pmsb = intel_fpga_i2c_read_extended(base_addr, addr, 0x38);
    pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, 0x39))<<8;
    pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, 0x3A))<<16;
    den = intel_fpga_i2c_read_extended(base_addr, addr, 0x3B);
    den += (intel_fpga_i2c_read_extended(base_addr, addr, 0x3C))<<8;
    den += (intel_fpga_i2c_read_extended(base_addr, addr, 0x3D))<<16;
    den += (intel_fpga_i2c_read_extended(base_addr, addr, 0x3E))<<24;
    printf("M_NUM = 0x%06X%06X, M_DEN = 0x%08X\n", pmsb, plsb, den);

    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x3);
    for(i = 0; i < 5; i++){
      plsb =   intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+2);
      plsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+3))<<8;
      plsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+4))<<16;
      pmsb =   intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+5);
      pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+6))<<8;
      pmsb += (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+7))<<16;
      den =    intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+8);
      den +=  (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+9))<<8;
      den +=  (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+10))<<16;
      den +=  (intel_fpga_i2c_read_extended(base_addr, addr, (i*11)+11))<<24;
      printf("N%i_NUM = 0x%06X%06X, N%i_DEN = 0x%08X\n", i, pmsb, plsb, i, den);
    }

    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x1);
    for(i = 0; i < 12; i++){
      tmp3 = ' ';
      if(i==0 || i==11) tmp3 = 'A';
      c = (i<1) ? i : (i<11) ? i-1 : i-2;
      tmp = intel_fpga_i2c_read_extended(base_addr, addr, (i*5)+3);
      if((tmp&0x3) == 0x2){
        tmp2 = intel_fpga_i2c_read_extended(base_addr, addr, (i*5)+6);
        intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x2);
        r = intel_fpga_i2c_read_extended(base_addr, addr, (i*3)+0x47);
        r += (intel_fpga_i2c_read_extended(base_addr, addr, (i*3)+0x48) << 8);
        r += (intel_fpga_i2c_read_extended(base_addr, addr, (i*3)+0x49) << 16);
        intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x1);
        printf("OUT%i%c N = 0x%X, R = 0x%06X\n", c, (char)tmp3, (tmp2&0x7), ((tmp&04) ? 2 : (r==0) ? 0 : (r+1)*2));
      }
      else printf("OUT%i%c OFF\n", c, (char)tmp3);
    }

}

void set_si5391a_out6_freq(unsigned int base_addr)
{
    alt_u8 addr = 0x74;

    //VCXO is 13750MHz - need N2 to create 300MHz, and the R divide by 2 to give us 150MHz
    printf("Change N2 so that OUT6 is 150MHz\n");
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x3);
    
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0018, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0019, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x001A, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x001B, 0x60);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x001C, 0x22);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x001D, 0x00);

    intel_fpga_i2c_write_extended(base_addr, addr, 0x001E, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x001F, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0020, 0x00);
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0021, 0xC0); //for 100MHz
    //intel_fpga_i2c_write_extended(base_addr, addr, 0x000B, 0xA8); //for 87.5MHz

    printf("Write to N2_UPDATE\n");
    intel_fpga_i2c_write_extended(base_addr, addr, 0x0022, 0x1);

    intel_fpga_i2c_write_extended(base_addr, addr, 0x0001, 0x0);
    int lock = intel_fpga_i2c_read_extended(base_addr, addr, 0x000C);
    printf("SI5391A [0x000C] = 0x%02X\n", lock);

    while (lock!=0) {
      lock = intel_fpga_i2c_read_extended(base_addr, addr, 0x000C);
    }

    printf("SI5391A locked after reconfiguration\n");
}
