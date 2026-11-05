#pragma once

#define VIAL_KEYBOARD_UID {0x89, 0x36, 0x2A, 0xC7, 0xFA, 0xD8, 0x89, 0x45}
#define VIAL_UNLOCK_COMBO_ROWS {0, 0}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}

/* Sync active layer to the other half so both sides change color together. */
#define SPLIT_LAYER_STATE_ENABLE

/* Persist layer 1-4 HSV in user EEPROM datablock. */
#define EECONFIG_USER_DATA_SIZE 32

/* Sync layer RGB config master -> slave. */
#define SPLIT_TRANSACTION_IDS_USER LAYER_RGB_SYNC
