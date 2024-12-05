#include "nrfx_nvmc.h"

#include "blinky_nvmc_driver.h"
#include "blinky_log.h"

void blinky_read_block(uint32_t* addr, uint32_t* payload, uint32_t word_cnt)
{
    NRF_LOG_INFO("NVMC DRV: blinky_read_block: read addr = 0x%x, payload addr = 0x%x, count = %u", (uint32_t)addr, (uint32_t)payload, word_cnt);

    ASSERT(NULL != addr);
    ASSERT(NULL != payload);
    ASSERT(word_cnt > 0);

    memcpy(payload, addr, word_cnt * BLINKY_WORD_SIZE);
}

bool blinky_block_writable_check(uint32_t* addr, uint32_t* payload, uint32_t word_cnt)
{
    NRF_LOG_INFO("NVMC DRV: blinky_block_writable_check: addr=0x%x, payload=0x%x, word_cnt=%u", addr, payload, word_cnt);
    
    ASSERT(NULL != addr);
    ASSERT(NULL != payload);
    ASSERT(0 != word_cnt);

    for (uint32_t i = 0; i < word_cnt; ++i)
    {   
        
        if (!nrfx_nvmc_word_writable_check((uint32_t)addr, *payload))
        {
            //NRF_LOG_INFO("NVMC DRV: blinky_block_writable_check: payload data, addr=0x%x, value=0x%x is NOT writable", (uint32_t)addr, *payload);
            return false;
        }

        addr++;
        payload++;
    }

    return true;
 }

void blinky_block_write(uint32_t* addr, uint32_t* payload, uint32_t word_cnt)
{
    NRF_LOG_INFO("NVMC DRV: blinky_block_write: addr=0x%x, payload=0x%x, word_cnt=%u", addr, payload, word_cnt);

    ASSERT(NULL != addr);
    ASSERT(NULL != payload);
    ASSERT(word_cnt > 0);

    for (uint32_t i = 0; i < word_cnt; ++i)
    {   
        //NRF_LOG_INFO("NVMC DRV: blinky_block_write: writing payload data, addr=0x%x, value=0x%x", (uint32_t)addr, *payload);
        nrfx_nvmc_word_write((uint32_t)addr, *payload);

        addr++;
        payload++;
    }
}

nrfx_err_t blinky_nvmc_erase_page(uint32_t* page_start_addr)
{
    NRF_LOG_INFO("NVMC DRV: blinky_nvmc_erase_page: page_start_addr=0x%x", page_start_addr);

    return nrfx_nvmc_page_erase((uint32_t)page_start_addr);
}

bool blinky_driver_write_done_check()
{
    return nrfx_nvmc_write_done_check();
}