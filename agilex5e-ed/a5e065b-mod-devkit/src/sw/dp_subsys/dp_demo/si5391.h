
#ifndef __SI5391_H__
#define __SI5391_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "alt_types.h"

#define I2C_ADDR_SI5391A (0x74)
#define I2C_ADDR_SI5391B (0x76)

void print_si5391(alt_u8 addr);
void print_si5391a(unsigned int base_addr);
void set_si5391a_out6_freq(unsigned int base_addr);


#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* __SI5338_CONTROL_H__ */
