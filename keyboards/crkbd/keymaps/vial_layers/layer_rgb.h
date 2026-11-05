#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Custom raw-HID command (avoids 0x07-0x09, which Vial uses for lighting). */
#define LAYER_RGB_HID_CMD 0xFC
#define LAYER_RGB_HID_GET 0x01
#define LAYER_RGB_HID_SET 0x02
#define LAYER_RGB_HID_SAVE 0x03
#define LAYER_RGB_HID_LOAD 0x04

/* Configurable solid colors for layers 1..4 (layer 0 stays on Vial Lighting). */
#define LAYER_RGB_FIRST 1
#define LAYER_RGB_LAST 4
#define LAYER_RGB_SLOTS (LAYER_RGB_LAST - LAYER_RGB_FIRST + 1)

void layer_rgb_init(void);
void layer_rgb_apply(uint8_t layer);
bool layer_rgb_get(uint8_t layer, uint8_t *h, uint8_t *s, uint8_t *v);
void layer_rgb_set(uint8_t layer, uint8_t h, uint8_t s, uint8_t v);
void layer_rgb_save(void);
void layer_rgb_load(void);
