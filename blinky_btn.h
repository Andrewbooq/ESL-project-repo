#ifndef BLINKY_BTN_H__
#define BLINKY_BTN_H__

/* button clicks callback */
typedef void (*click_cb_t)(void);

/* need to call app_timer_init first */
void blinky_btns_init(click_cb_t on_hold, click_cb_t on_release, click_cb_t on_click, click_cb_t on_double_click, click_cb_t on_triple_click);

#endif //BLINKY_BTN_H__

