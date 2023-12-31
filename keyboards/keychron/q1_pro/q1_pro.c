/* Copyright 2023 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "q1_pro.h"
#include "print.h"
#ifdef KC_BLUETOOTH_ENABLE
#    include "ckbt51.h"
#    include "bluetooth.h"
#    include "indicator.h"
#    include "transport.h"
#    include "battery.h"
#    include "bat_level_animation.h"
#    include "lpm.h"
#endif

#ifdef ENABLE_FACTORY_TEST
#    include "factory_test.h"
#endif

typedef struct PACKED {
    uint8_t len;
    uint8_t keycode[3];
} key_combination_t;

static uint16_t random_key_timer = 0;
static uint16_t random_speed_ms = 50;
static uint16_t random_delay = 0;
static bool ran_alph_held = false;
static bool ran_num_held = false;
static bool ran_alpn_held = false;
static bool ran_pass_held = false;

static bool print_M0 = false;
static bool print_M1 = false;
static bool print_M2 = false;
static int counter = 0;

key_combination_t key_comb_list[4] = {
    {2, {KC_LWIN, KC_TAB}},        // Task (win)
    {2, {KC_LWIN, KC_E}},          // Files (win)
    {3, {KC_LSFT, KC_LGUI, KC_4}}, // Snapshot (mac)
    {2, {KC_LWIN, KC_C}}           // Cortana (win)
};

#ifdef KC_BLUETOOTH_ENABLE
bool                   firstDisconnect  = true;
bool                   bt_factory_reset = false;
static virtual_timer_t pairing_key_timer;
extern uint8_t         g_pwm_buffer[DRIVER_COUNT][192];

static void pairing_key_timer_cb(void *arg) {
    bluetooth_pairing_ex(*(uint8_t *)arg, NULL);
}
#endif

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (index == 0) {
        default_layer_set(1UL << (active ? 2 : 0));
    }
    dip_switch_update_user(index, active);

    return true;
}

#ifdef KC_BLUETOOTH_ENABLE
bool process_record_kb_bt(uint16_t keycode, keyrecord_t *record) {
    static uint8_t host_idx = 0;
#else
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
#endif

    switch (keycode) {
#ifdef KC_BLUETOOTH_ENABLE
        case BT_HST1 ... BT_HST3:
            if (get_transport() == TRANSPORT_BLUETOOTH) {
                if (record->event.pressed) {
                    host_idx = keycode - BT_HST1 + 1;
                    chVTSet(&pairing_key_timer, TIME_MS2I(2000), (vtfunc_t)pairing_key_timer_cb, &host_idx);
                    bluetooth_connect_ex(host_idx, 0);
                } else {
                    host_idx = 0;
                    chVTReset(&pairing_key_timer);
                }
            }
            break;
        case BAT_LVL:
            if (get_transport() == TRANSPORT_BLUETOOTH && !usb_power_connected()) {
                bat_level_animiation_start(battery_get_percentage());
            }
            break;
#endif
        case PB_1:
            if (record->event.pressed) {
                random_key_timer = timer_read();
                ran_alph_held = true;
            } else {
                ran_alph_held = false;
                random_key_timer = 0;
            }
            return false;
        case PB_2:
            if (record->event.pressed) {
                random_key_timer = timer_read();
                ran_num_held = true;
            } else {
                ran_num_held = false;
                random_key_timer = 0;
            }
            return false;
        case PB_3:
            if (record->event.pressed) {
                random_key_timer = timer_read();
                ran_alpn_held = true;
            } else {
                ran_alpn_held = false;
                random_key_timer = 0;
            }
            return false;
        case PB_4:
            if (record->event.pressed) {
                random_key_timer = timer_read();
                ran_pass_held = true;
            } else {
                ran_pass_held = false;
                random_key_timer = 0;
            }
            return false;
        case MC_0:
            print_M0 = true;
            random_key_timer = timer_read();
            return false;
        case MC_1:
            print_M1 = true;
            random_key_timer = timer_read();
            return false;
        case MC_2:
            print_M2 = true;
            random_key_timer = timer_read();
            return false;
        case KC_PSCR: {
            if (get_highest_layer(layer_state | default_layer_state) == 0) {
                static bool ss3_reg;
                static bool ss4_reg;
                uint8_t mod_state = get_mods();  // Fetch mod_state here
                if (record->event.pressed && (mod_state & MOD_MASK_GUI)) {
                    del_mods(MOD_MASK_GUI);
                    register_code(KC_LSFT);
                    register_code(KC_LGUI);
                    register_code(KC_3);
                    ss3_reg = true;
                    set_mods(mod_state);
                    return false;
                }
				else if (record->event.pressed && (mod_state & MOD_MASK_ALT)) {
                    del_mods(MOD_MASK_ALT);
                    register_code(KC_LSFT);
                    register_code(KC_LGUI);
                    register_code(KC_SPC);
                    register_code(KC_4);
                    ss4_reg = true;
                    set_mods(mod_state);
                    return false;
                }
                else if (ss3_reg) {
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                    unregister_code(KC_3);
                    ss3_reg = false;
                    return false;
                }
                else if (ss4_reg) {
                    unregister_code(KC_LSFT);
                    unregister_code(KC_LGUI);
                    unregister_code(KC_4);
                    ss4_reg = false;
                    return false;
                }
            }
            return true;
        }
        default:
#ifdef FACTORY_RESET_CHECK
            FACTORY_RESET_CHECK(keycode, record);
#endif
            break;
    }
    return true;
}


#ifdef ENCODER_ENABLE
static void encoder0_pad_cb(void *param) {
    (void)param;
    encoder_inerrupt_read(0);
}
#endif

#ifdef BLUETOOTH_ENABLE
static void own_bt_param_init(void) {
    /* Set bluetooth device name */
    ckbt51_set_local_name(STR(PRODUCT));

    module_param_t param = {.event_mode             = 0x02,
                            .connected_idle_timeout = 7200,
                            .pairing_timeout        = 180,
                            .pairing_mode           = 0,
                            .reconnect_timeout      = 5,
                            .report_rate            = 90,
                            .vendor_id_source       = 1,
                            .verndor_id             = 0, // Must be 0x3434
                            .product_id             = PRODUCT_ID};
    ckbt51_set_param(&param);
}
#endif

void keyboard_post_init_kb(void) {
    dip_switch_read(true);

#ifdef KC_BLUETOOTH_ENABLE
    setPinOutput(A9);
    writePinLow(A9);
    /* IMPORTANT: DO NOT enable internal pull-up resistor
     * as there is an external pull-down resistor.
     */
    palSetLineMode(USB_BT_MODE_SELECT_PIN, PAL_MODE_INPUT);

    ckbt51_init(false);
    bluetooth_init();
#endif

#ifdef ENCODER_ENABLE
    pin_t encoders_pad_a[] = ENCODERS_PAD_A;
    pin_t encoders_pad_b[] = ENCODERS_PAD_B;
    palEnableLineEvent(encoders_pad_a[0], PAL_EVENT_MODE_BOTH_EDGES);
    palEnableLineEvent(encoders_pad_b[0], PAL_EVENT_MODE_BOTH_EDGES);
    palSetLineCallback(encoders_pad_a[0], encoder0_pad_cb, NULL);
    palSetLineCallback(encoders_pad_b[0], encoder0_pad_cb, NULL);
#endif

    keyboard_post_init_user();
}

void matrix_scan_kb(void) {
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char digits[] = "0123456789";
    char symbols[] = "!#$%&()*+,-./:;<=>?";
	
    char M0[] = "XRegPasWdX";
    char M1[] = "XAlexCPasswordX";
    char M2[] = "XKarmaPasswordX";

    if (print_M0 && timer_elapsed(random_key_timer) > random_delay) {
       	random_key_timer = timer_read();

    	// Get the current character from the hardcoded string
    	char current_char = M0[counter];
    
    	// Print the current character
    	char char_string[2] = {current_char, '\0'};
    	send_string(char_string);
    
    	// Increment the counter
    	counter++;
    
    	// Check if the counter has reached the end of the string
    	if (counter == sizeof(M0)) {
    		counter = 0;
    		print_M0 = false;
    		random_key_timer = timer_read();
			random_delay = 0;
    	}
		random_delay = rand() % 350 + 10;
    }
	
    if (print_M1 && timer_elapsed(random_key_timer) > random_delay) {
       	random_key_timer = timer_read();

    	// Get the current character from the hardcoded string
    	char current_char = M1[counter];
    
    	// Print the current character
    	char char_string[2] = {current_char, '\0'};
    	send_string(char_string);
    
    	// Increment the counter
    	counter++;
    
    	// Check if the counter has reached the end of the string
    	if (counter == sizeof(M1)) {
    		counter = 0;
    		print_M1 = false;
    		random_key_timer = timer_read();
    	}
		random_delay = rand() % 350 + 10;

    }
	
    if (print_M2 && timer_elapsed(random_key_timer) > random_delay) {
       	random_key_timer = timer_read();

    	// Get the current character from the hardcoded string
    	char current_char = M2[counter];
    
    	// Print the current character
    	char char_string[2] = {current_char, '\0'};
    	send_string(char_string);
    
    	// Increment the counter
    	counter++;
    
    	// Check if the counter has reached the end of the string
    	if (counter == sizeof(M2)) {
    		counter = 0;
    		print_M2 = false;
    		random_key_timer = timer_read();
    	}
		random_delay = rand() % 350 + 10;

    }
    
    if (ran_alph_held && timer_elapsed(random_key_timer) > random_speed_ms) {
        random_key_timer = timer_read();
    
        int random_index = rand() % (sizeof(letters) - 1);
        
        char letter_string[2] = {letters[random_index], '\0'};
        send_string(letter_string);
    }
    
    if (ran_num_held && timer_elapsed(random_key_timer) > random_speed_ms) {
        random_key_timer = timer_read();
    
        int random_index = rand() % (sizeof(digits) - 1);
    
        char digit_string[2] = {digits[random_index], '\0'};
        send_string(digit_string);
    }
    
    if (ran_alpn_held && timer_elapsed(random_key_timer) > random_speed_ms) {
    	// Reset the timer
    	random_key_timer = timer_read();
    	int rand_val = rand() % 2;
    
    	if (rand_val == 0) {
    		int random_index = rand() % (sizeof(letters) - 1);
    		char letter_string[2] = {letters[random_index], '\0'};
    		send_string(letter_string);
    	} else {
    		int random_index = rand() % (sizeof(digits) - 1);
    		char digit_string[2] = {digits[random_index], '\0'};
    		send_string(digit_string);
    	}
    }
    if (ran_pass_held && timer_elapsed(random_key_timer) > random_speed_ms) {
        random_key_timer = timer_read();
        
        int rand_val = rand() % 8;
        if (rand_val < 2) {
            int random_index = rand() % (sizeof(symbols) - 1);
            char symbol_string[2] = {symbols[random_index], '\0'};
            send_string(symbol_string);
        } 
        else if (rand_val < 5) {
            int random_index = rand() % (sizeof(digits) - 1);
            char digit_string[2] = {digits[random_index], '\0'};
            send_string(digit_string);
        }
        else {
            int random_index = rand() % (sizeof(letters) - 1);
            char char_string[2] = {letters[random_index], '\0'};
            send_string(char_string);
        }
    }

#ifdef FACTORY_RESET_TASK
    FACTORY_RESET_TASK();
#endif
    matrix_scan_user();
}

#ifdef KC_BLUETOOTH_ENABLE
static void ckbt51_param_init(void) {
    /* Set bluetooth device name */
    ckbt51_set_local_name(STR(PRODUCT));
    /* Set bluetooth parameters */
    module_param_t param = {.event_mode             = 0x02,
                            .connected_idle_timeout = 7200,
                            .pairing_timeout        = 180,
                            .pairing_mode           = 0,
                            .reconnect_timeout      = 5,
                            .report_rate            = 90,
                            .vendor_id_source       = 1,
                            .verndor_id             = 0, // Must be 0x3434
                            .product_id             = PRODUCT_ID};
    ckbt51_set_param(&param);
}

void bluetooth_enter_disconnected_kb(uint8_t host_idx) {
    if (bt_factory_reset) {
        bt_factory_reset = false;
        ckbt51_param_init();
    }
    /* CKBT51 bluetooth module boot time is slower, it enters disconnected after boot,
       so we place initialization here. */
    if (firstDisconnect && sync_timer_read32() < 1000 && get_transport() == TRANSPORT_BLUETOOTH) {
        ckbt51_param_init();
        bluetooth_connect();
        firstDisconnect = false;
    }
}

void bluetooth_pre_task(void) {
    static uint8_t mode = 1;

    if (readPin(USB_BT_MODE_SELECT_PIN) != mode) {
        if (readPin(USB_BT_MODE_SELECT_PIN) != mode) {
            mode = readPin(USB_BT_MODE_SELECT_PIN);
            set_transport(mode == 0 ? TRANSPORT_BLUETOOTH : TRANSPORT_USB);
        }
    }
}

void battery_calculte_voltage(uint16_t value) {
    uint16_t voltage = ((uint32_t)value) * 2246 / 1000;

#    ifdef RGB_MATRIX_ENABLE
    if (rgb_matrix_is_enabled()) {
        uint32_t totalBuf = 0;

        for (uint8_t i = 0; i < DRIVER_COUNT; i++)
            for (uint8_t j = 0; j < 192; j++)
                totalBuf += g_pwm_buffer[i][j];
        /* We assumpt it is linear relationship*/
        uint32_t compensation = 60 * totalBuf / RGB_MATRIX_LED_COUNT / 255 / 3;
        voltage += compensation;
    }
#    endif
    battery_set_voltage(voltage);
}
#endif

bool via_command_kb(uint8_t *data, uint8_t length) {
    switch (data[0]) {
#ifdef KC_BLUETOOTH_ENABLE
        case 0xAA:
            ckbt51_dfu_rx(data, length);
            break;
#endif
#ifdef ENABLE_FACTORY_TEST
        case 0xAB:
            factory_test_rx(data, length);
            break;
#endif
        default:
            return false;
    }

    return true;
}

#if !defined(VIA_ENABLE)
void raw_hid_receive(uint8_t *data, uint8_t length) {
    switch (data[0]) {
        case RAW_HID_CMD:
            via_command_kb(data, length);
            break;
    }
}

#endif




