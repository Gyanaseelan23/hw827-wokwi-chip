#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
  pin_t signal_pin;
  uint32_t bpm_attr;
  float phase;
} chip_state_t;

void chip_timer_callback(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;

  int bpm = attr_read(chip->bpm_attr);
  float freq = bpm / 60.0; // beats per second

  // Advance phase based on a fixed timer tick (e.g., every 20ms)
  chip->phase += freq * 0.02;
  if (chip->phase > 1.0) chip->phase -= 1.0;

  // Simple pulse waveform: sharp spike near phase=0, baseline elsewhere
  float voltage;
  if (chip->phase < 0.1) {
    voltage = 3.0; // simulate a "beat" peak
  } else {
    voltage = 1.5; // baseline
  }

  pin_dac_write(chip->signal_pin, voltage);
}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->signal_pin = pin_init("SIGNAL", ANALOG);
  chip->bpm_attr = attr_init("bpm", 75);
  chip->phase = 0;

  const timer_config_t config = {
    .callback = chip_timer_callback,
    .user_data = chip,
  };
  timer_t timer_id = timer_init(&config);
  timer_start(timer_id, 20000, true); // 20ms = 20000 microseconds, repeating
}
