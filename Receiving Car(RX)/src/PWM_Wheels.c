#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

static uint TLRedWheel;
static uint TLBlackWheel;

void pwm_xy_init(uint RedWheel, uint BlackWheel)
{
    TLRedWheel = RedWheel;
    TLBlackWheel = BlackWheel;

    gpio_set_function(TLRedWheel, GPIO_FUNC_PWM);   // set to GPIO X to PWM mode
    gpio_set_function(TLBlackWheel, GPIO_FUNC_PWM); // set to GPIO Y to PWM mode

    // get slice and channel for gpio 16
    uint slice_redTLWheel = pwm_gpio_to_slice_num(TLRedWheel);
    uint channel_redTLWheel = pwm_gpio_to_channel(TLRedWheel);

    // get slice and channel for gpio17
    uint slice_blackTLWheel = pwm_gpio_to_slice_num(TLBlackWheel);
    uint channel_blackTLWheel = pwm_gpio_to_channel(TLBlackWheel);

    pwm_set_wrap(slice_redTLWheel, 1561); // got it from choosing 4.0f (random) since max of 65535 wrap got me 0.127f. Stuck with roughly 15k frequnecy
    pwm_set_wrap(slice_blackTLWheel, 1561);
    pwm_set_clkdiv(slice_redTLWheel, 4.0f); // pwm_frequency = [picos_system_clock (125M) / clkdic (4.0)] / [wrap (2082) + 1]
    pwm_set_clkdiv(slice_blackTLWheel, 4.0f);
    pwm_set_enabled(slice_redTLWheel, true); // true to enable specified pwm, false to disable it
    pwm_set_enabled(slice_blackTLWheel, true);
}

void set_wheel_value(uint16_t xvalue,
                     uint16_t yvalue,
                     uint TLRedWheel, uint TLBlackWheel,
                     uint BLRedWheel, uint BLBlackWheel,
                     uint TRRedWheel, uint TRBlackWheel,
                     uint BRRedWheel, uint BRBlackWheel)
{
    static bool must_reset_to_neutral = false;
    float duty;


    // Setup Left Wheels
    uint slice_TL_red = pwm_gpio_to_slice_num(TLRedWheel);
    uint slice_TL_black = pwm_gpio_to_slice_num(TLBlackWheel);
    uint channel_TL_red = pwm_gpio_to_channel(TLRedWheel);
    uint channel_TL_black = pwm_gpio_to_channel(TLBlackWheel);

    uint slice_BL_red = pwm_gpio_to_slice_num(BLRedWheel);
    uint slice_BL_black = pwm_gpio_to_slice_num(BLBlackWheel);
    uint channel_BL_red = pwm_gpio_to_channel(BLRedWheel);
    uint channel_BL_black = pwm_gpio_to_channel(BLBlackWheel);

    // Setup Right Wheels
    uint slice_TR_red = pwm_gpio_to_slice_num(TRRedWheel);
    uint slice_TR_black = pwm_gpio_to_slice_num(TRBlackWheel);
    uint channel_TR_red = pwm_gpio_to_channel(TRRedWheel);
    uint channel_TR_black = pwm_gpio_to_channel(TRBlackWheel);

    uint slice_BR_red = pwm_gpio_to_slice_num(BRRedWheel);
    uint slice_BR_black = pwm_gpio_to_slice_num(BRBlackWheel);
    uint channel_BR_red = pwm_gpio_to_channel(BRRedWheel);
    uint channel_BR_black = pwm_gpio_to_channel(BRBlackWheel);

    bool is_neutral = (xvalue > 500 && xvalue < 3500) && (yvalue > 500 && yvalue < 3500);

    if (is_neutral)
    {
        // Reset all motors
        pwm_set_chan_level(slice_TL_red, channel_TL_red, 0);
        pwm_set_chan_level(slice_TL_black, channel_TL_black, 0);
        pwm_set_chan_level(slice_BL_red, channel_BL_red, 0);
        pwm_set_chan_level(slice_BL_black, channel_BL_black, 0);
        pwm_set_chan_level(slice_TR_red, channel_TR_red, 0);
        pwm_set_chan_level(slice_TR_black, channel_TR_black, 0);
        pwm_set_chan_level(slice_BR_red, channel_BR_red, 0);
        pwm_set_chan_level(slice_BR_black, channel_BR_black, 0);
        must_reset_to_neutral = false;
    }
    else if (!must_reset_to_neutral)
    {
        // Forward
        if (xvalue < 100 && (yvalue > 500 && yvalue < 3500))
        {
            duty = (2000 - xvalue) / 2000.0f * 1796.0f;

            // All wheels forward
            pwm_set_chan_level(slice_TL_red, channel_TL_red, duty);//Left Side
            pwm_set_chan_level(slice_TL_black, channel_TL_black, 0);
            pwm_set_chan_level(slice_BL_red, channel_BL_red, duty);
            pwm_set_chan_level(slice_BL_black, channel_BL_black, 0);

            pwm_set_chan_level(slice_TR_red, channel_TR_red, 0);//Right Side
            pwm_set_chan_level(slice_TR_black, channel_TR_black, duty);
            pwm_set_chan_level(slice_BR_red, channel_BR_red, 0);
            pwm_set_chan_level(slice_BR_black, channel_BR_black, duty);

            must_reset_to_neutral = true;
        }
        // Reverse
        else if (xvalue > 4000 && (yvalue > 500 && yvalue < 3500))
        {
            duty = (xvalue - 2200) / (4095.0f - 2200.0f) * 1796.0f;

            // All wheels reverse
            pwm_set_chan_level(slice_TL_red, channel_TL_red, 0);        //Left Side
            pwm_set_chan_level(slice_TL_black, channel_TL_black, duty);
            pwm_set_chan_level(slice_BL_red, channel_BL_red, 0);
            pwm_set_chan_level(slice_BL_black, channel_BL_black, duty);

            pwm_set_chan_level(slice_TR_red, channel_TR_red, duty);        //Right Side
            pwm_set_chan_level(slice_TR_black, channel_TR_black, 0);
            pwm_set_chan_level(slice_BR_red, channel_BR_red, duty);
            pwm_set_chan_level(slice_BR_black, channel_BR_black, 0);

            must_reset_to_neutral = true;
        }
        // Right Turn: left wheels forward, right wheels reverse
        else if (yvalue < 100 && (xvalue > 500 && xvalue < 3500))
        {
            duty = (2000 - yvalue) / 2000.0f * 1796.0f;

            pwm_set_chan_level(slice_TL_red, channel_TL_red, duty);    //Left Side
            pwm_set_chan_level(slice_TL_black, channel_TL_black, 0);
            pwm_set_chan_level(slice_BL_red, channel_BL_red, duty);
            pwm_set_chan_level(slice_BL_black, channel_BL_black, 0);

            pwm_set_chan_level(slice_TR_red, channel_TR_red, duty);        //Right Side
            pwm_set_chan_level(slice_TR_black, channel_TR_black, 0);
            pwm_set_chan_level(slice_BR_red, channel_BR_red, duty);
            pwm_set_chan_level(slice_BR_black, channel_BR_black, 0);

            must_reset_to_neutral = true;
        }
        // Left Turn: left wheels reverse, right wheels forward
        else if (yvalue > 4000 && (xvalue > 500 && xvalue < 3500))
        {
            duty = (yvalue - 2200) / (4095.0f - 2200.0f) * 1796.0f;

            pwm_set_chan_level(slice_TL_red, channel_TL_red, 0);        //Left Side
            pwm_set_chan_level(slice_TL_black, channel_TL_black, duty);
            pwm_set_chan_level(slice_BL_red, channel_BL_red, 0);
            pwm_set_chan_level(slice_BL_black, channel_BL_black, duty);

            pwm_set_chan_level(slice_TR_red, channel_TR_red, 0);     //Right Side
            pwm_set_chan_level(slice_TR_black, channel_TR_black, duty);
            pwm_set_chan_level(slice_BR_red, channel_BR_red, 0);
            pwm_set_chan_level(slice_BR_black, channel_BR_black, duty);

            must_reset_to_neutral = true;
        }
    }
}

