#ifndef PWM_WHEELS_H
#define PWM_WHEELS_H

void pwm_xy_init(uint RedWheel, uint BlackWheel);
void set_wheel_value(uint16_t xvalue,
                     uint16_t yvalue,
                     uint TopLeftRedWheel,
                     uint TopLeftBlackWheel,
                     uint BottomLeftRedWheel,
                     uint BottomLeftBlackWheel,
                     uint TopRightRedWheel,
                     uint TopRightBlackWheel,
                     uint BottomRightRedWheel,
                     uint BottomRightBlackWheel);

#endif