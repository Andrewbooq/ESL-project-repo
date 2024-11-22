#ifndef BLINKY_NVMC_H__
#define BLINKY_NVMC_H__

#define ARRAY_UINT_COUNT(struct_tpye) (sizeof(struct_tpye) / sizeof(uint32_t))

typedef struct 
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint8_t d;
    float pi;
} test_t;

void blinky_nvmc_init(void);
/*
data - buffer to write
size - a size of the buffer in bytes
*/
uint32_t blinky_nvmc_read_last_data(uint32_t* data, uint32_t size);


void blinky_nvmc_test(uint32_t* array, uint32_t size);

#endif /* BLINKY_NVMC_H__ */