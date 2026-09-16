
#ifndef __BOARD_H__
#define __BOARD_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "alt_types.h"

void board_configure();
void board_tx_freq(int freq);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOARD_H__ */
