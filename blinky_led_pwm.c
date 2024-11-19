#include "nrfx_pwm.h"

#include "blinky_led.h"
#include "blinky_log.h"
#include "blinky_led_pwm.h"

#define BLINKY_LEDS_ON_BOARD        4

static nrfx_pwm_t g_pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_individual_t g_seq_values = 
{
    .channel_0 = 0, /* Duty cycle value mapped on NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE for channel 0. */
    .channel_1 = 0, /* Duty cycle value mapped on NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE for channel 1. */
    .channel_2 = 0, /* Duty cycle value mapped on NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE for channel 2. */
    .channel_3 = 0  /* Duty cycle value mapped on NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE for channel 3. */
};

static nrf_pwm_sequence_t const g_seq =
{
    .values.p_individual = &g_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(g_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};

void blinky_led_pwm_init(void)
{
    blinky_leds_init();
    nrfx_pwm_config_t config = NRFX_PWM_DEFAULT_CONFIG;
    config.output_pins[0] = BLINKY_LED_0_PIN | NRFX_PWM_PIN_INVERTED; // channel 0
    config.output_pins[1] = BLINKY_LED_1_PIN | NRFX_PWM_PIN_INVERTED; // channel 1
    config.output_pins[2] = BLINKY_LED_2_PIN | NRFX_PWM_PIN_INVERTED; // channel 2
    config.output_pins[3] = BLINKY_LED_3_PIN | NRFX_PWM_PIN_INVERTED; // channel 3

    nrfx_err_t resx = nrfx_pwm_init(&g_pwm, &config, NULL);
    UNUSED_VARIABLE(resx);
    ASSERT(NRFX_SUCCESS == resx);

    nrfx_pwm_simple_playback(&g_pwm, &g_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void blinky_led_pwm_set(uint8_t led_idx, uint8_t value)
{
    ASSERT(blinky_check_idx(led_idx));
    uint16_t channel_value = value * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / (uint16_t)100;
    
    switch (led_idx)
    {
        case 0: g_seq_values.channel_0 = channel_value; break;
        case 1: g_seq_values.channel_1 = channel_value; break;
        case 2: g_seq_values.channel_2 = channel_value; break;
        case 3: g_seq_values.channel_3 = channel_value; break;
        default: break;
    }
}
