#include <EEPROM.h>

#include <stdint.h>

#include "globals.h"

#define EEPROM_ADDRESS_ESC_MODE 0x00
#define EEPROM_ADDRESS_TH_REVERSED 0x01


// ****************************************************************************
void load_persistent_storage(void)
{
  esc_mode = EEPROM.read(EEPROM_ADDRESS_ESC_MODE);
  if (esc_mode >= ESC_MODE_COUNT) {
    esc_mode = 0;
  }

  channel[TH].reversed = EEPROM.read(EEPROM_ADDRESS_TH_REVERSED);
  if (channel[TH].reversed > 1) {
    channel[TH].reversed = 0;
  }
}


// ****************************************************************************
void write_persistent_storage(void)
{
  EEPROM.write(EEPROM_ADDRESS_ESC_MODE, esc_mode);
  EEPROM.write(EEPROM_ADDRESS_TH_REVERSED, channel[TH].reversed);
}
