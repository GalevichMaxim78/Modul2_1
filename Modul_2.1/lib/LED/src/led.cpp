#include "led.h"
#include <driver/gpio.h>

bool Led::Init(gpio_num_t pin, gpio_mode_t mode)
{
    control_pin_ = pin;
    state_ = LEDState::OFF;

    gpio_reset_pin(control_pin_);
    gpio_set_direction(pin, mode);

    return true;
}

void Led::Set(LEDState state)
{
    state_ = state;
    switch (state_)
    {
        case LEDState::OFF:
        {
            gpio_set_level(control_pin_, 0);
            break;
        }

        case LEDState::ON:
        {
            gpio_set_level(control_pin_, 1);
            break;
        }
        
        default:
            break;
    }
}

LEDState Led::GetState() const
{
    return state_;
}
