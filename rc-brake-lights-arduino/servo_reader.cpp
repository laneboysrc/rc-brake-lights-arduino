#include <PinChangeInterruptSettings.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterrupt.h>
#include <PinChangeInterruptPins.h>

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"


#define SERVO_PULSE_CLAMP_LOW 800
#define SERVO_PULSE_CLAMP_HIGH 2300

#define INITIAL_ENDPOINT_DELTA  250
#define SERVO_PULSE_MIN 600
#define SERVO_PULSE_MAX 2500

#define STARTUP_TIME (2000 / __SYSTICK_IN_MS)


static volatile bool new_data = false;
static volatile uint16_t raw_data;

static enum {
    INITIALIZE = 0,
    WAIT_FOR_TIMEOUT,
    NORMAL_OPERATION
} servo_reader_state = INITIALIZE;

CHANNEL_T channel[1];


// ****************************************************************************
static void initialize_channel(CHANNEL_T *c) {
    c->endpoint.centre = c->raw_data;
    c->endpoint.left = c->raw_data - INITIAL_ENDPOINT_DELTA;
    c->endpoint.right = c->raw_data + INITIAL_ENDPOINT_DELTA;
}


// ****************************************************************************
static void normalize_channel(CHANNEL_T *c)
{
    if (c->raw_data < SERVO_PULSE_MIN  ||  c->raw_data > SERVO_PULSE_MAX) {
        c->normalized = 0;
        c->absolute = 0;
        return;
    }

    if (c->raw_data < SERVO_PULSE_CLAMP_LOW) {
        c->raw_data = SERVO_PULSE_CLAMP_LOW;
    }

    if (c->raw_data > SERVO_PULSE_CLAMP_HIGH) {
        c->raw_data = SERVO_PULSE_CLAMP_HIGH;
    }

    if (c->raw_data == c->endpoint.centre) {
        c->normalized = 0;
    }
    else if (c->raw_data < c->endpoint.centre) {
        if (c->raw_data < c->endpoint.left) {
            c->endpoint.left = c->raw_data;
        }
        // In order to acheive a stable 100% value we actually calculate the
        // percentage up to 101%, and then clamp to 100%.
        c->normalized = (c->endpoint.centre - c->raw_data) * 101 /
            (c->endpoint.centre - c->endpoint.left);
        if (c->normalized > 100) {
            c->normalized = 100;
        }
        if (!c->reversed) {
            c->normalized = -c->normalized;
        }
    }
    else {
        if (c->raw_data > c->endpoint.right) {
            c->endpoint.right = c->raw_data;
        }
        c->normalized = (c->raw_data - c->endpoint.centre) * 101 /
            (c->endpoint.right - c->endpoint.centre);
        if (c->normalized > 100) {
            c->normalized = 100;
        }
        if (c->reversed) {
            c->normalized = -c->normalized;
        }
    }

    if (c->normalized < 0) {
        c->absolute = -c->normalized;
    }
    else {
        c->absolute = c->normalized;
    }
}


// ****************************************************************************
static void throttle_pulse_interrupt(void) 
{
  static unsigned long start;
  unsigned long now;

  if (new_data) {
    return;
  }

  now = micros();

  if (digitalRead(TH_PIN) == HIGH) {
    start = now;
  }
  else {
    raw_data = now - start; 
    new_data = true;  
  }
}


// ****************************************************************************
void init_servo_reader(void)
{
  pinMode(TH_PIN, INPUT_PULLUP);
  attachPCINT(digitalPinToPCINT(TH_PIN), throttle_pulse_interrupt, CHANGE);
}


// ****************************************************************************
void read_all_servo_channels(void)
{
    static uint8_t servo_reader_timer;
    static uint8_t remaining_pulse_count;

    bool new_channel_data = false;

    if (global_flags.systick) {
        if (servo_reader_timer) {
            --servo_reader_timer;
        }
    }

    if (new_data) {
        channel[TH].raw_data = raw_data;
        new_channel_data = true;
        new_data = false;
    }

    switch (servo_reader_state) {
        case INITIALIZE:
            servo_reader_timer = STARTUP_TIME;
            servo_reader_state = WAIT_FOR_TIMEOUT;
            remaining_pulse_count = 50;
            break;

        case WAIT_FOR_TIMEOUT:
            if (new_channel_data) {
                if (remaining_pulse_count) {
                    --remaining_pulse_count;
                }
            }

            // Initialization is finished if START_UP time has passed and
            // more than 50 servo pulses have been received. This way we ensure
            // the RC system is up-and-running.
            //
            // At that point we take the throttle value as "neutral position"
            if (servo_reader_timer == 0  &&  remaining_pulse_count == 0) {
                initialize_channel(&channel[TH]);

                servo_reader_state = NORMAL_OPERATION;
                global_flags.initializing = 0;
            }
            break;

        case NORMAL_OPERATION:
            if (new_channel_data) {
                normalize_channel(&channel[TH]);
                global_flags.new_channel_data = true;
            }
            break;

        default:
            servo_reader_state = INITIALIZE;
            break;
    }
}
