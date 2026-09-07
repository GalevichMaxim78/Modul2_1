#include <soc/gpio_num.h>
#include <hal/gpio_types.h>
#include <cstdint>

enum class LEDState : uint8_t
{
    ON,
    OFF
};

class Led
{
public:
    Led() = default;

    bool Init(gpio_num_t pin, gpio_mode_t mode);
    void Set(LEDState state);
    LEDState GetState() const;

private:
    gpio_num_t control_pin_;
    LEDState state_;

};