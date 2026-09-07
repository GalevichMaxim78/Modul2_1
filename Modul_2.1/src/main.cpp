#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "led.h"
#include <stdio.h>
#include "esp_log.h"
#include "inttypes.h"

enum class ModeState : uint8_t
{
    BLINK,
    ON,
    OFF
};

void Report(uint64_t iteration_count, float average_time);

struct Config
{
    inline static constexpr gpio_num_t LED_PIN = GPIO_NUM_15;
    inline static constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_16;
    inline static constexpr int64_t BLINK_INTERVAL_MS = 1000;
    inline static constexpr uint32_t ITERATION_COUNT_REPORT = 10;
};

volatile bool button_state = false;

static void IRAM_ATTR gpio_isr_handler(void* args)
{
    //gpio_intr_disable(Config::BUTTON_PIN);
    //gpio_isr_handler_remove(Config::BUTTON_PIN);

    button_state = true;

    gpio_isr_handler_add(Config::BUTTON_PIN, gpio_isr_handler, NULL);
    gpio_intr_enable(Config::BUTTON_PIN);
}

extern "C" void app_main() 
{
    Led led;
    if (!led.Init(Config::LED_PIN, GPIO_MODE_OUTPUT))
    {
        printf("Led initialization failed");
        return;
    }

    gpio_reset_pin(Config::BUTTON_PIN);
    gpio_set_direction(Config::BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(Config::BUTTON_PIN, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(Config::BUTTON_PIN, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(Config::BUTTON_PIN, gpio_isr_handler, NULL);
    gpio_intr_enable(Config::BUTTON_PIN);

    led.Set(LEDState::OFF);

    uint64_t iteration_index{0};
    float average_time{0.0f}; 
    int64_t time_point {0};
    ModeState mode_state{ModeState::BLINK};

    while(true)
    {
        int64_t duration = esp_timer_get_time(); 
        if (duration - time_point < Config::BLINK_INTERVAL_MS * 1000)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        ++iteration_index;
        time_point = duration;

        switch(mode_state) 
        {
            case ModeState::BLINK:
            {
                mode_state = button_state ? ModeState::ON : ModeState::BLINK;
                led.GetState() == LEDState::ON ? led.Set(LEDState::OFF) : led.Set(LEDState::ON);
                break;
            }
            case ModeState::OFF:
            {
                mode_state = button_state ? ModeState::BLINK : ModeState::OFF;
                led.Set(LEDState::OFF);
                break;
            }
            case ModeState::ON:
            {
                mode_state = button_state ? ModeState::OFF : ModeState::ON;
                led.Set(LEDState::ON);
                break;
            }
            default: break;
        }

        if (button_state)
        {
            button_state = false;
        }

        int64_t end_time_point = esp_timer_get_time();
        float difference = (end_time_point - time_point) / 1000.0f;
        average_time = average_time + (difference - average_time) / (float)iteration_index;
        if (iteration_index % Config::ITERATION_COUNT_REPORT == 0)
        {
            Report(iteration_index, average_time);
        }

        vTaskDelay(50);
    }
}

void Report(uint64_t iteration_count, float average_time)
{
    printf("Average iteration time - count: %llu time (ms): %f\n", iteration_count, average_time);
}