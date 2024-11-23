#ifndef BLINKY_NVMC_DRIVER_H__
#define BLINKY_NVMC_DRIVER_H__

/* Address to write to. Must be word-aligned. */

void blinky_read_block(uint32_t* addr, uint32_t* payload, uint32_t word_cnt);
bool blinky_block_writable_check(uint32_t* addr, uint32_t* payload, uint32_t word_cnt);
void blinky_block_write(uint32_t* addr, uint32_t* payload, uint32_t word_cnt);
nrfx_err_t blinky_nvmc_erase_page(uint32_t* page_start_addr);
bool blinky_driver_write_done_check();

#endif /* BLINKY_NVMC_DRIVER_H__ */