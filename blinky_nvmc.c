#include "nrfx_nvmc.h"
#include "nrf_bootloader_info.h"
#include "nrf_dfu_types.h"

#include "blinky_nvmc.h"
#include "blinky_log.h"

#define BLINKY_UINT32_SIZE (sizeof(uint32_t))
static test_t test = 
{
    .a = 0x010307F,
    .b = 0xAABBCCDD,
    .c = 0xDEADBEEF,
    .d = 0xFF,
    .pi = 3.14159265
};

STATIC_ASSERT(sizeof(test) % BLINKY_UINT32_SIZE == 0, "struct must be aligned to 32 bit word");

//CRITICAL_REGION_ENTER();
//CRITICAL_REGION_EXIT();

#define BLINKY_APP_AREA_BEGIN_ADDR  (BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE)
#define BLINKY_APP_AREA_END_ADDR    (BOOTLOADER_START_ADDR - 1)
#define BLINKY_MAX_WRITABLE_SIZE    24 /* bytes */
#define BLINKY_EMPTY_VALUE          0xFFFFFFFF

uint32_t* g_last_addr = NULL;

uint32_t blinky_read_header(uint32_t* addr)
{
    NRF_LOG_INFO("blinky_read_header: read addr 0x%x", (uint32_t)addr);

    ASSERT((uint32_t)addr >= BLINKY_APP_AREA_BEGIN_ADDR && (uint32_t)addr <= BLINKY_APP_AREA_END_ADDR);

    uint32_t header = *addr;

    return header;
}

void blinky_read_block(uint32_t* addr, uint32_t* payload, uint32_t count)
{
    NRF_LOG_INFO("blinky_read_block: read addr = 0x%x, payload addr = 0x%x, count = %u", (uint32_t)addr, (uint32_t)payload, count);
    
    ASSERT((uint32_t)addr >= BLINKY_APP_AREA_BEGIN_ADDR && (uint32_t)addr <= BLINKY_APP_AREA_END_ADDR);

    for (uint32_t i = 0; i < count; ++i)
    {
        *payload = *addr;
        addr++;
        payload++;
    }
}

bool blinky_block_writable_check(uint32_t* addr, uint32_t* payload, uint32_t count)
{
    bool writable = false;
    NRF_LOG_INFO("blinky_block_writable_check: begin addr for writing 0x%x", addr);

    /* Header */
    /* Write payload size in bytes to be compatible with common rules */
    uint32_t header = count * sizeof(uint32_t);
    
    NRF_LOG_INFO("blinky_block_writable_check: check writable header data, addr=0x%x, value=%u", (uint32_t)addr, header);
    if(nrfx_nvmc_word_writable_check((uint32_t)addr, header))
    {
        writable = true;
        NRF_LOG_INFO("blinky_block_writable_check: header data is writable");
    }
    addr++;

    for (uint32_t i = 0; i < count; ++i)
    {   
        NRF_LOG_INFO("blinky_block_writable_check: payload data, addr=0x%x, value=0x%x", (uint32_t)addr, *payload);
        if(nrfx_nvmc_word_writable_check((uint32_t)addr, *payload))
        {
            writable = writable ? true : false;
            NRF_LOG_INFO("blinky_block_writable_check: payload data is writable");
        }
        else
        {
            writable = false;
            NRF_LOG_INFO("blinky_block_writable_check: payload data is NOT writable");
        }
        addr++;
        payload++;
    }

    return writable;
 }

void blinky_block_write(uint32_t* addr, uint32_t* payload, uint32_t count)
{
    NRF_LOG_INFO("blinky_block_write: begin addr for writing 0x%x", addr);

    /* Header */
    /* Write payload size in bytes to be compatible with common rules */
    uint32_t header = count * sizeof(uint32_t);
    
    NRF_LOG_INFO("blinky_block_write: writing header data, addr=0x%x, value=%u", (uint32_t)addr, header);
    nrfx_nvmc_word_write((uint32_t)addr, header);
    
    addr++;

    for (uint32_t i = 0; i < count; ++i)
    {   
        NRF_LOG_INFO("blinky_block_write: writing payload data, addr=0x%x, value=0x%x", (uint32_t)addr, *payload);
        nrfx_nvmc_word_write((uint32_t)addr, *payload);

        addr++;
        payload++;
    }
}

void blinky_nvmc_init(void)
{
    NRF_LOG_INFO("blinky_nvmc_init");
    NRF_LOG_INFO("blinky_nvmc_init: initialize g_last_addr to first byte of first page");
    uint32_t* g_last_addr = (uint32_t*)BLINKY_APP_AREA_BEGIN_ADDR;
    NRF_LOG_INFO("blinky_nvmc_init: g_last_addr=%u", g_last_addr);
}

uint32_t blinky_nvmc_read_last_data(uint32_t* data, uint32_t size)
{
    NRF_LOG_INFO("blinky_nvmc_read_last_data: data addr=0x%x, size=%u", (uint32_t)data, size);

    ASSERT(NULL != data);
    ASSERT(size > 0);

    if( size % BLINKY_UINT32_SIZE != 0)
    {
        NRF_LOG_INFO("blinky_nvmc_read_last_data: invalid parameter");
        return 0;
    }

    /* Read header */
    uint32_t block_size = blinky_read_header(g_last_addr);
    NRF_LOG_INFO("blinky_nvmc_read_last_data: blinky_read_header=%u", block_size);

    if(block_size != size || (block_size > BLINKY_MAX_WRITABLE_SIZE) || (block_size % BLINKY_UINT32_SIZE != 0))
    {
        NRF_LOG_INFO("blinky_nvmc_read_last_data: header value is wrong, does not correspond count");
        // Invalid value read, need to clean the page
        // TODO: call nrfx_nvmc_page_erase
        return 0;
    }

    if (block_size == BLINKY_EMPTY_VALUE)
    {
        NRF_LOG_INFO("blinky_nvmc_read_last_data: read memory is empty");
        // No data in memory
        return 0;
    }

    /* Header looks good, continue */
    g_last_addr++;
    
    /* Read data, skeep header */
    uint32_t count = size / BLINKY_UINT32_SIZE;
    blinky_read_block(g_last_addr, data, count);
    g_last_addr += block_size;

    return block_size;
}

void blinky_nvmc_test(uint32_t* array, uint32_t count)
{
    NRF_LOG_INFO("blinky_nvmc_test: array=0x%x, count=%u", (uint32_t)array, count);
    NRF_LOG_INFO("blinky_nvmc_test: don't passed data so far\n\n\n");

    blinky_nvmc_init();

//    /* Erase page */
//    nrfx_err_t resx = nrfx_nvmc_page_erase(BLINKY_APP_AREA_BEGIN_ADDR);
//    NRF_LOG_INFO("blinky_nvmc_test: nrfx_nvmc_page_erase returned %u", resx);

//    /* Writing */
//    uint32_t* payload_test = (uint32_t*)&test;
//    uint32_t count_test = sizeof(test) / sizeof(uint32_t);
//
//    if (true == blinky_block_writeble_check(addr_test, payload_test, count_test))
//    {
//        NRF_LOG_INFO("blinky_nvmc_test: blinky_block_writeble_check passed, able to write");
//        blinky_block_write(addr_test, payload_test, count_test);
//    }
//    
//    /* Waititng for complete writing */
//    while (!nrfx_nvmc_write_done_check())
//    {}
//    NRF_LOG_INFO("blinky_nvmc_test: writing complete");
    


/*

    uint32_t flash_page_size = nrfx_nvmc_flash_page_size_get();
    NRF_LOG_INFO("blinky_nvmc_test: flash_page_size %u bytes", flash_page_size);

    //initial erase whole app area
    //blinky_erase_page(BLINKY_APP_AREA_BEGIN_ADDR);
    //blinky_erase_page(BLINKY_APP_AREA_BEGIN_ADDR+flash_page_size);
    //blinky_erase_page(BLINKY_APP_AREA_BEGIN_ADDR+flash_page_size+flash_page_size);


    return;

    uint32_t flash_size = nrfx_nvmc_flash_size_get();
    NRF_LOG_INFO("blinky_nvmc_test: flash_size %u bytes", flash_size);

    uint32_t flash_page_cnt = nrfx_nvmc_flash_page_count_get();
    NRF_LOG_INFO("blinky_nvmc_test: flash_page_cnt %u", flash_page_cnt);
    
    NRF_LOG_INFO("blinky_nvmc_test: BOOTLOADER_START_ADDR %u", BOOTLOADER_START_ADDR);
    NRF_LOG_INFO("blinky_nvmc_test: NRF_DFU_APP_DATA_AREA_SIZE %u", NRF_DFU_APP_DATA_AREA_SIZE);
    
    uint32_t app_area_begin_addr = BOOTLOADER_START_ADDR - NRF_DFU_APP_DATA_AREA_SIZE;
    uint32_t app_area_end_addr = BOOTLOADER_START_ADDR - 1;

    NRF_LOG_INFO("blinky_nvmc_test: app_area_begin_addr 0x%x", app_area_begin_addr);
    NRF_LOG_INFO("blinky_nvmc_test: app_area_begin_end 0x%x", app_area_end_addr);
    uint32_t cnt = 5;
    for (uint32_t addr = app_area_end_addr; addr >= app_area_begin_addr; addr--)
    {
        uint32_t* readaddr = (uint32_t*)addr;
        NRF_LOG_INFO("blinky_nvmc_test: readaddr %u", *readaddr);
        
        cnt--;
        if (cnt == 0)
            break;
    }

    //nrfx_nvmc_word_writable_check
*/
/*
<info> app: blinky_nvmc_test: flash_size 1048576 bytes
<info> app: blinky_nvmc_test: flash_page_size 4096 bytes
<info> app: blinky_nvmc_test: flash_page_cnt 256
<info> app: blinky_nvmc_test: BOOTLOADER_START_ADDR 114688
<info> app: blinky_nvmc_test: NRF_DFU_APP_DATA_AREA_SIZE 12288

*/

/*
    nrfx_err_t resx = nrfx_nvmc_page_erase(0);
    ASSERT(NRFX_SUCCESS == resx);

    bool res = nrfx_nvmc_word_writable_check(0, 0);
    ASSERT(true == res);

    nrfx_nvmc_word_write(0, 0);

    res = nrfx_nvmc_write_done_check();
    while (!res)
    {}
*/
    //NRF_DFU_APP_DATA_AREA_SIZE
    //BOOTLOADER_START_ADDR
}