#ifndef _BOARD_H_
#define _BOARD_H_
#include <unistd.h>

char *get_board_name(uint32_t board);
char *get_connector_name(uint32_t connector);

void start_systempll(void);
void board_init(void);

void board_set_txclk(double freq, int tbcr);
void tmds1204_monitor(int rx_5v, int rx_tbcr);

#endif
