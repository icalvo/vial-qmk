#include "layer_rgb.h"

#include <string.h>

#include "color.h"
#include "eeconfig.h"
#include "quantum.h"
#include "rgb_matrix.h"
#include "transactions.h"
#include "via.h"

#define LAYER_RGB_MAGIC 0x4C /* 'L' */
#define LAYER_RGB_VERSION 1

typedef struct __attribute__((packed)) {
    uint8_t magic;
    uint8_t version;
    HSV     slots[LAYER_RGB_SLOTS]; /* index 0 => layer 1, ... */
} layer_rgb_config_t;

static layer_rgb_config_t g_layer_rgb;
static const HSV          k_defaults[LAYER_RGB_SLOTS] = {
    {HSV_GREEN},
    {HSV_ORANGE},
    {HSV_WHITE},
    {HSV_BLUE},
};

static bool layer_in_range(uint8_t layer) {
    return layer >= LAYER_RGB_FIRST && layer <= LAYER_RGB_LAST;
}

static uint8_t slot_index(uint8_t layer) {
    return (uint8_t)(layer - LAYER_RGB_FIRST);
}

static void set_defaults(void) {
    g_layer_rgb.magic   = LAYER_RGB_MAGIC;
    g_layer_rgb.version = LAYER_RGB_VERSION;
    for (uint8_t i = 0; i < LAYER_RGB_SLOTS; i++) {
        g_layer_rgb.slots[i] = k_defaults[i];
    }
}

#ifdef SPLIT_KEYBOARD
static void layer_rgb_slave_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (in_buflen < sizeof(layer_rgb_config_t)) {
        return;
    }
    memcpy(&g_layer_rgb, in_data, sizeof(layer_rgb_config_t));
    layer_rgb_apply(get_highest_layer(layer_state | default_layer_state));
}

static void layer_rgb_sync_to_slave(void) {
    if (is_keyboard_master()) {
        transaction_rpc_send(LAYER_RGB_SYNC, sizeof(g_layer_rgb), &g_layer_rgb);
    }
}
#else
static void layer_rgb_sync_to_slave(void) {}
#endif

void layer_rgb_save(void) {
    g_layer_rgb.magic   = LAYER_RGB_MAGIC;
    g_layer_rgb.version = LAYER_RGB_VERSION;
    eeconfig_update_user_datablock(&g_layer_rgb, 0, sizeof(g_layer_rgb));
}

void layer_rgb_load(void) {
    if (!eeconfig_is_user_datablock_valid()) {
        set_defaults();
        return;
    }
    eeconfig_read_user_datablock(&g_layer_rgb, 0, sizeof(g_layer_rgb));
    if (g_layer_rgb.magic != LAYER_RGB_MAGIC || g_layer_rgb.version != LAYER_RGB_VERSION) {
        set_defaults();
    }
}

void layer_rgb_init(void) {
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(LAYER_RGB_SYNC, layer_rgb_slave_handler);
#endif
    layer_rgb_load();
    if (is_keyboard_master()) {
        layer_rgb_sync_to_slave();
    }
}

bool layer_rgb_get(uint8_t layer, uint8_t *h, uint8_t *s, uint8_t *v) {
    if (!layer_in_range(layer) || !h || !s || !v) {
        return false;
    }
    HSV hsv = g_layer_rgb.slots[slot_index(layer)];
    *h      = hsv.h;
    *s      = hsv.s;
    *v      = hsv.v;
    return true;
}

void layer_rgb_set(uint8_t layer, uint8_t h, uint8_t s, uint8_t v) {
    if (!layer_in_range(layer)) {
        return;
    }
    g_layer_rgb.slots[slot_index(layer)] = (HSV){h, s, v};
    if (get_highest_layer(layer_state | default_layer_state) == layer) {
        layer_rgb_apply(layer);
    }
    layer_rgb_sync_to_slave();
}

void layer_rgb_apply(uint8_t layer) {
    if (layer == 0) {
        rgb_matrix_reload_from_eeprom();
        return;
    }
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    if (layer_in_range(layer)) {
        HSV hsv = g_layer_rgb.slots[slot_index(layer)];
        rgb_matrix_sethsv_noeeprom(hsv.h, hsv.s, hsv.v);
        return;
    }
    rgb_matrix_sethsv_noeeprom(HSV_PURPLE);
}

void eeconfig_init_user(void) {
    set_defaults();
    layer_rgb_save();
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (length < 6 || data[0] != LAYER_RGB_HID_CMD) {
        data[0] = id_unhandled;
        return;
    }

    uint8_t sub = data[1];
    switch (sub) {
        case LAYER_RGB_HID_GET: {
            uint8_t layer = data[2];
            uint8_t h = 0, s = 0, v = 0;
            if (!layer_rgb_get(layer, &h, &s, &v)) {
                data[1] = 0xFF;
                break;
            }
            data[1] = LAYER_RGB_HID_GET;
            data[2] = layer;
            data[3] = h;
            data[4] = s;
            data[5] = v;
            break;
        }
        case LAYER_RGB_HID_SET: {
            uint8_t layer = data[2];
            layer_rgb_set(layer, data[3], data[4], data[5]);
            data[1] = LAYER_RGB_HID_SET;
            break;
        }
        case LAYER_RGB_HID_SAVE: {
            layer_rgb_save();
            layer_rgb_sync_to_slave();
            data[1] = LAYER_RGB_HID_SAVE;
            break;
        }
        case LAYER_RGB_HID_LOAD: {
            layer_rgb_load();
            layer_rgb_sync_to_slave();
            layer_rgb_apply(get_highest_layer(layer_state | default_layer_state));
            data[1] = LAYER_RGB_HID_LOAD;
            break;
        }
        default:
            data[0] = id_unhandled;
            break;
    }
}
