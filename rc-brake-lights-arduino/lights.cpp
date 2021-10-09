#include <Arduino.h>

#include <stdint.h>

#include "globals.h"


#define LED_ON 1
#define LED_OFF 0

static uint8_t flash_counter;
static uint8_t esc_mode_counter;


// ****************************************************************************
static void process_setup_lights(void)
{
    ++flash_counter;

    switch (global_flags.setup) {
        case SETUP_THROTTLE_REVERSING:
            // Flash all LEDs at 5 Hz
            if (flash_counter == (100 / __SYSTICK_IN_MS)) {
              digitalWrite(BRAKE_LED, LED_ON);
              digitalWrite(REVERSING_LED, LED_ON);
            }

            if (flash_counter >= (200 / __SYSTICK_IN_MS)) {
              flash_counter = 0;
              digitalWrite(BRAKE_LED, LED_OFF);
              digitalWrite(REVERSING_LED, LED_OFF);
            }
            break;

        case SETUP_ESC_MODE:
            // Flash the ESC mode number (0 => 1 flash, 1 => 2 flashes ...)
            // Each on or off period takes 100 ms
            if (flash_counter >= (100 / __SYSTICK_IN_MS)) {
                flash_counter = 0;

                // The LEDs are turned off at every odd count
                if (esc_mode_counter & 1) {
                  digitalWrite(BRAKE_LED, LED_OFF);
                  digitalWrite(REVERSING_LED, LED_OFF);
                }
                // The LEDs are turned on at even counts which value is less
                // than or equal two times the ESC mode number
                else if (esc_mode_counter <= (esc_mode << 1)) {
                  digitalWrite(BRAKE_LED, LED_ON);
                  digitalWrite(REVERSING_LED, LED_ON);
                }

                //The sequence repeats every 15 counts.
                ++esc_mode_counter;
                if (esc_mode_counter == 15) {
                    esc_mode_counter = 0;
                }
            }
            break;

        default:
            break;
    }
}


// ****************************************************************************
void init_lights(void)
{
  pinMode(BRAKE_LED, OUTPUT);
  pinMode(REVERSING_LED, OUTPUT);
  digitalWrite(BRAKE_LED, LED_OFF);
  digitalWrite(REVERSING_LED, LED_OFF);
}


// ****************************************************************************
void process_lights(void)
{
    // Process the lights only every __SYSTICK_IN_MS
    // This way we can easily time blinking and flash effects
    if (!global_flags.systick) {
      return;
    }

    // The system is initializing: Brake light on
    if (global_flags.initializing) {
      digitalWrite(BRAKE_LED, LED_ON);
      digitalWrite(REVERSING_LED, LED_OFF);
      return;
    }

    // Setup modes
    if (global_flags.setup) {
      process_setup_lights();
      return;
    }

    // Normal operation
    digitalWrite(BRAKE_LED, global_flags.braking);
    digitalWrite(REVERSING_LED, global_flags.reversing);

    flash_counter = 0;
    esc_mode_counter = 0;
}
