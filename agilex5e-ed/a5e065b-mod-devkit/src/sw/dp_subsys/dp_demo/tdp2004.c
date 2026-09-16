#include <stdio.h>
#include <system.h>
#include <io.h>
#include <unistd.h>
#include "alt_types.h"
#include "intel_fpga_i2c.h"
#include "system.h"
#include "tdp2004.h"

unsigned int tdp2004_read_eq (unsigned int lane)
{
    unsigned int tdp2004_data;
    unsigned char tdp2004_eqstage2;
    unsigned char tdp2004_eqstage1;
    unsigned char tdp2004_stage1_bypass;
    unsigned char tdp2004_eqprofile;
    unsigned char tdp2004_flatgain;
    unsigned int db_eq_index;

    if (lane == 0)
    {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN0_ADDR);
    } else if (lane == 1) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN1_ADDR);
    } else if (lane == 2) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN2_ADDR);
    } else if (lane == 3) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN3_ADDR);
    } else {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_ALL_ADDR);
    }
    tdp2004_eqstage2 = (tdp2004_data & 0x7);
    tdp2004_eqstage1 = ((tdp2004_data >> 3) & 0xf); 
    tdp2004_stage1_bypass = ((tdp2004_data >> 7) & 0x1); 

    if (lane == 0)
    {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN0_ADDR);
    } else if (lane == 1) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN1_ADDR);
    } else if (lane == 2) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN2_ADDR);
    } else if (lane == 3) {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN3_ADDR);
    } else {
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_ALL_ADDR);
    }
    // printf("Read 0x%08x\n" , tdp2004_data);
    //reserved, eqprofile[3:0], flatgain[2:0]
    tdp2004_flatgain = (tdp2004_data & 0x7); 
    tdp2004_eqprofile = ((tdp2004_data >> 3) & 0xf); 

    // printf("%d %d %d %d %d\n", tdp2004_eqstage1, tdp2004_eqstage2, tdp2004_eqprofile, tdp2004_stage1_bypass, tdp2004_flatgain);

    if (tdp2004_stage1_bypass)
    {
       if ((tdp2004_eqstage1 == 0) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 0)) {
           printf("TDP2004 Lane:%d EQ Index:0 Boost=4.0dB ", lane);
           db_eq_index = 0;
       } else if ((tdp2004_eqstage1 == 1) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 0)) {
           printf("TDP2004 Lane:%d EQ Index:1 Boost=5.0dB ", lane);
           db_eq_index = 1;
       } else if ((tdp2004_eqstage1 == 3) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 0)) {
           printf("TDP2004 Lane:%d EQ Index:2 Boost=7.0dB ", lane);
           db_eq_index = 2;
        } else {
           printf("TDP2004 Lane:%d EQ Index: not recognised ", lane);
           db_eq_index = 1; 
        }
    } else {
        if ((tdp2004_eqstage1 == 0) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 1)) {
            printf("TDP2004 Lane:%d EQ Index:5 Boost=8.0dB ", lane);
            db_eq_index = 3;
        } else if ((tdp2004_eqstage1 == 1) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 1)) {
            printf("TDP2004 Lane:%d EQ Index:6 Boost=9.0dB ", lane);
            db_eq_index = 4;
        } else if ((tdp2004_eqstage1 == 2) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 1)) {
            printf("TDP2004 Lane:%d EQ Index:7 Boost=9.5dB ", lane);
            db_eq_index = 5;
        } else if ((tdp2004_eqstage1 == 3) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 3)) {
            printf("TDP2004 Lane:%d EQ Index:8 Boost=10dB ", lane);
            db_eq_index = 6;
        } else if ((tdp2004_eqstage1 == 4) && (tdp2004_eqstage2 == 0) && (tdp2004_eqprofile == 3)) {
            printf("TDP2004 Lane:%d EQ Index:9 Boost=11dB ", lane);
            db_eq_index = 7;
        } else if ((tdp2004_eqstage1 == 5) && (tdp2004_eqstage2 == 1) && (tdp2004_eqprofile == 7)) {
            printf("TDP2004 Lane:%d EQ Index:10 Boost=12dB ", lane);
            db_eq_index = 8;
        } else if ((tdp2004_eqstage1 == 6) && (tdp2004_eqstage2 == 1) && (tdp2004_eqprofile == 7)) {
            printf("TDP2004 Lane:%d EQ Index:11 Boost=12.5dB ", lane);
            db_eq_index = 9;
        } else if ((tdp2004_eqstage1 == 8) && (tdp2004_eqstage2 == 1) && (tdp2004_eqprofile == 7)) {
            printf("TDP2004 Lane:%d EQ Index:12 Boost=13.5dB ", lane);
            db_eq_index = 10;
        } else if ((tdp2004_eqstage1 == 10) && (tdp2004_eqstage2 == 1) && (tdp2004_eqprofile == 7)) {
            printf("TDP2004 Lane:%d EQ Index:13 Boost=14.5dB ", lane);
            db_eq_index = 11;
        } else if ((tdp2004_eqstage1 == 10) && (tdp2004_eqstage2 == 2) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:14 Boost=15dB ", lane);
            db_eq_index = 12;
        } else if ((tdp2004_eqstage1 == 11) && (tdp2004_eqstage2 == 3) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:15 Boost=15.5dB ", lane);
            db_eq_index = 13;
        } else if ((tdp2004_eqstage1 == 12) && (tdp2004_eqstage2 == 4) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:16 Boost=16.5dB ", lane);
            db_eq_index = 14;
        } else if ((tdp2004_eqstage1 == 13) && (tdp2004_eqstage2 == 5) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:17 Boost=17dB ", lane);
            db_eq_index = 15;
        } else if ((tdp2004_eqstage1 == 14) && (tdp2004_eqstage2 == 6) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:18 Boost=18dB ", lane);
            db_eq_index = 16;
        } else if ((tdp2004_eqstage1 == 15) && (tdp2004_eqstage2 == 7) && (tdp2004_eqprofile == 15)) {
            printf("TDP2004 Lane:%d EQ Index:19 Boost=19dB ", lane);
            db_eq_index = 17;
        } else {
            printf("TDP2004 EQ Index: not recognised ");
            db_eq_index = 1;
        }
    }

    if (tdp2004_flatgain == 0) {
        printf("FlatGain -5.6dB\n");
    } else if (tdp2004_flatgain == 1) {
        printf("FlatGain -3.8dB\n");
    } else if (tdp2004_flatgain == 3) {
        printf("FlatGain -1.2dB\n");
    } else if (tdp2004_flatgain == 5) {
        printf("FlatGain -0.6dB\n");
    } else if (tdp2004_flatgain == 7) {
        printf("FlatGain +2.6dB\n");
    }

    return db_eq_index;

}


void tdp2004_set_gain(unsigned char tdp2004_flatgain) 
{

    unsigned int tdp2004_data;
    unsigned char tdp2004_eqprofile;

    if (tdp2004_flatgain == 0) {
        printf("FlatGain -5.6dB\n");
    } else if (tdp2004_flatgain == 1) {
        printf("FlatGain -3.8dB\n");
    } else if (tdp2004_flatgain == 3) {
        printf("FlatGain -1.2dB\n");
    } else if (tdp2004_flatgain == 5) {
        printf("FlatGain -0.6dB\n");
    } else if (tdp2004_flatgain == 7) {
        printf("FlatGain +2.6dB\n");
    } else {
        printf("Invalid Gain\n");
        return;
    }

    tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_ALL_ADDR);
    //reserved, eqprofile[3:0], flatgain[2:0]
    tdp2004_eqprofile = ((tdp2004_data >> 3) & 0xf); 

    tdp2004_data = (tdp2004_eqprofile << 3) | tdp2004_flatgain;
    intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_ALL_ADDR, tdp2004_data);

}

void tdp2004_set_eq(unsigned int lane, unsigned int db_eq_index) 
{

    unsigned int tdp2004_data;
    unsigned char tdp2004_eqstage2;
    unsigned char tdp2004_eqstage1;
    unsigned char tdp2004_stage1_bypass;
    unsigned char tdp2004_eqprofile;
    unsigned char tdp2004_flatgain;

    switch(db_eq_index)
    {
    case 0:
        tdp2004_stage1_bypass = 1;
        tdp2004_eqprofile = 0;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 0;
        break;
    case 1:
        tdp2004_stage1_bypass = 1;
        tdp2004_eqprofile = 0;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 1;
        break;
    case 2:
        tdp2004_stage1_bypass = 1;
        tdp2004_eqprofile = 0;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 3;
        break;
    case 3:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 1;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 0;
        break;
    case 4:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 1;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 1;
        break;
    case 5:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 1;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 2;
        break;
    case 6:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 3;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 3;
        break;
    case 7:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 3;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 4;
        break;
    case 8:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 7;
        tdp2004_eqstage2 = 1;
        tdp2004_eqstage1 = 5;
        break;
    case 9:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 7;
        tdp2004_eqstage2 = 1;
        tdp2004_eqstage1 = 6;
        break;
    case 10:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 7;
        tdp2004_eqstage2 = 1;
        tdp2004_eqstage1 = 8;
        break;
    case 11:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 7;
        tdp2004_eqstage2 = 1;
        tdp2004_eqstage1 = 10;
        break;
    case 12:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 2;
        tdp2004_eqstage1 = 10;
        break;
    case 13:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 3;
        tdp2004_eqstage1 = 11;
        break;
    case 14:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 4;
        tdp2004_eqstage1 = 12;
        break;
    case 15:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 5;
        tdp2004_eqstage1 = 13;
        break;
    case 16:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 6;
        tdp2004_eqstage1 = 14;
        break;
    case 17:
        tdp2004_stage1_bypass = 0;
        tdp2004_eqprofile = 15;
        tdp2004_eqstage2 = 7;
        tdp2004_eqstage1 = 15;
        break;
    default:
        tdp2004_stage1_bypass = 1;
        tdp2004_eqprofile = 0;
        tdp2004_eqstage2 = 0;
        tdp2004_eqstage1 = 0;
        break;
    }

    // printf("%d %d %d %d %d\n", tdp2004_eqstage1, tdp2004_eqstage2, tdp2004_eqprofile, tdp2004_stage1_bypass, tdp2004_flatgain);

    tdp2004_data = (tdp2004_stage1_bypass << 7) | (tdp2004_eqstage1 << 3) | tdp2004_eqstage2;
    if (lane == 0)
    {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN0_ADDR, tdp2004_data);
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN0_ADDR);
    } else if (lane == 1) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN1_ADDR, tdp2004_data);
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN1_ADDR);
    } else if (lane == 2) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN2_ADDR, tdp2004_data);
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN2_ADDR);
    } else if (lane == 3) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_LN3_ADDR, tdp2004_data);
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN3_ADDR);
    } else {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN0_ALL_ADDR, tdp2004_data);
        tdp2004_data  = intel_fpga_i2c_read_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_ALL_ADDR);    
    }

    //reserved, eqprofile[3:0], flatgain[2:0]
    tdp2004_flatgain = (tdp2004_data & 0x7); 

    tdp2004_data = (tdp2004_eqprofile << 3) | tdp2004_flatgain;
    if (lane == 0)
    {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN0_ADDR, tdp2004_data);
    } else if (lane == 1) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN1_ADDR, tdp2004_data);
    } else if (lane == 2) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN2_ADDR, tdp2004_data);
    } else if (lane == 3) {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_LN3_ADDR, tdp2004_data);
    } else {
        intel_fpga_i2c_write_extended(I2C_MASTER_BASE, PARRETTO_I2C_TX_ADDR, TDP2004_DEVICE_EQGAIN1_ALL_ADDR, tdp2004_data);
    }
}
