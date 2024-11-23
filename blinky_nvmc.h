#ifndef BLINKY_NVMC_H__
#define BLINKY_NVMC_H__

/* Wtiting to one page number 0. Once the writing reached the end of the page - the page erases. */

/* writable_block_size - size of the struct in bytes*/
void blinky_nvmc_init(uint32_t writable_block_size);

/* data - buffer to write
size - a size of the data in bytes */
uint32_t blinky_nvmc_read_last_data(uint32_t* data, uint32_t size);

/* data - buffer to write
size - a size of the data in bytes */
bool blinky_nvmc_write_data(uint32_t* data, uint32_t size);

bool blinky_nvmc_write_done_check();

#endif /* BLINKY_NVMC_H__ */