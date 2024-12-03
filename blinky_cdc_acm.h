#ifndef BLINKY_CDC_ACM_H__
#define BLINKY_CDC_ACM_H__

#include "blinky_types.h"

 /* USB Communications Device Class */
 /* Subclass Abstract Control Model */

void blinky_cdc_acm_init(command_cb_t on_command);
void blinky_cdc_acm_loop(void);

/* Send a string over USB, max size = SEND_SIZE */
bool blinky_cdc_acm_send_str(const char* str);


#endif //BLINKY_CDC_ACM_H__