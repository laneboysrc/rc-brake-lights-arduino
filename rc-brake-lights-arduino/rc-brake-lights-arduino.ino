#include "globals.h"


/* RC-brake-lights-Arduino
 *  
 *  
 * IMPORTANT:
 * 
 * IO pins are defined in file global.h
 * 
 * This sketch requires the PinChangeInterrupt library. Use the library 
 * manager to install it
 * 
 */

GLOBAL_FLAGS_T global_flags;
uint32_t entropy;


// ****************************************************************************
static void service_systick(void)
{
  static unsigned long last_systick = 0;
  ++entropy;

  global_flags.systick = 0;

  if (millis() >= (last_systick + __SYSTICK_IN_MS)) {
    last_systick = millis();
    global_flags.systick = 1;
  }
}


// ****************************************************************************
static void init_softtimer(void)
{
  // Nothing to do on Arduino, we are using the millis() function
}


// ****************************************************************************
void setup() {
  global_flags.initializing = true;

  init_lights();
  init_setup();
  load_persistent_storage();
  init_softtimer();
  init_servo_reader();
}

// ****************************************************************************
void loop() {
  service_systick();
  read_all_servo_channels();
  process_drive_mode();
  process_setup();
  process_lights();
}
