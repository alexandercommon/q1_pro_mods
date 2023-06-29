#ifdef ENABLE_RGB_MATRIX_BREATHING
RGB_MATRIX_EFFECT(BREATHING)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

uint8_t frac(uint16_t n, uint16_t d) {
    uint16_t result = (uint16_t)255 * n / d;
    return (uint8_t)result;
}

bool BREATHING(effect_params_t* params) {
    HSV hsv = rgb_matrix_config.hsv;  // Get the current HSV color
    RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
    uint16_t new_hue = 0;  
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t index = g_led_config.matrix_co[row][col];
            if (index >= led_min && index < led_max && index != NO_LED) {
                hsv = rgb_matrix_config.hsv;
                new_hue = 0;
                uint16_t keycode = keymap_key_to_keycode(get_highest_layer(layer_state | default_layer_state), (keypos_t){col,row});
                switch (keycode) {
                    case KC_LBRC:
                    	new_hue = 195;
                    	break;
                    case KC_RBRC:
                    	new_hue = 195;
                    	break;
                    case KC_COMM:
                    	new_hue = 25;
                    	break;
                    case KC_DOT:
                    	new_hue = 40;
                    	break;
                    case KC_SLSH:
                    	new_hue = 275;
                    	break;
                    case KC_EQL:
                    	new_hue = 115;
                    	break;
                    case KC_MINS:
                    	new_hue = 5;
                    	break;
                    case KC_9:
                    	new_hue = 230;
                    	break;
                    case KC_0:
                    	new_hue = 230;
                    	break;
                    case KC_4:
                    	new_hue = 155;
                    	break;
                    case KC_2:
                    	new_hue = 345;
                    	break;
                    case PB_1:
                    	new_hue = 0;
                    	break;
                    case PB_2:
                    	new_hue = 200;
                    	break;
                    case PB_3:
                    	new_hue = 290;
                    	break;
                    case PB_4:
                    	new_hue = 135;
                    	break;
                    case KC_PWR:
                    	new_hue = 85;
                    	break;
                    case QK_MACRO_0:
                    	new_hue = 165;
                    	break;
                    case QK_MACRO_1:
                    	new_hue = 200;
                    	break;
                    case QK_MACRO_2:
                    	new_hue = 300;
                    	break;
                    case QK_MACRO_3:
                    	new_hue = 340;
                    	break;
                    case KC_VOLU:
                    	new_hue = 90;
                    	break;
                    case KC_VOLD:
                    	new_hue = 230;
                    	break;
                    case KC_MUTE:
                    	new_hue = 0;
                    	break;
						
						
                    case RGB_TOG:
						rgb_matrix_set_color(index, 255, 255, 255);
                    	continue;
                    case RGB_MOD:
						rgb_matrix_set_color(index, 130, 200, 130);
                    	continue;
                    case RGB_RMOD:
						rgb_matrix_set_color(index, 200, 130, 130);
                    	continue;
                    case RGB_VAI:
						rgb_matrix_set_color(index, 0, 255, 0);
                    	continue;
                    case RGB_VAD:
						rgb_matrix_set_color(index, 255, 0, 0);
                    	continue;
                    case RGB_HUI:
						hsv.v = 200;
						rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
						rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    	continue;
                    case RGB_HUD:
						hsv.h = (hsv.h + 127) % 255;
						hsv.v = 200;
						rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
						rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    	continue;
                    case RGB_SAI:
						hsv.v = 200;
						hsv.s = 255;
						rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
						rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    	continue;
                    case RGB_SAD:
						hsv.v = 200;
						hsv.s = 0;
						rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
						rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    	continue;
                    case RGB_SPI:
						rgb_matrix_set_color(index, 255, 155, 0);
                    	continue;
                    case RGB_SPD:
						rgb_matrix_set_color(index, 0, 0, 255);
                    	continue;
					case QK_KB_0:
						rgb_matrix_set_color(index, 0, 0, 255);
                    	continue;
					case QK_KB_1:
						rgb_matrix_set_color(index, 0, 0, 255);
                    	continue;
					case QK_KB_2:
						rgb_matrix_set_color(index, 0, 0, 255);
                    	continue;
					case QK_KB_3:
						hsv.h = 85;
						hsv.s = 35;
						rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
						rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
                    	continue;
					case NK_TOGG:
						rgb_matrix_set_color(index, 255, 255, 255);
                    	continue;
					case DB_TOGG:
						rgb_matrix_set_color(index, 255, 255, 255);
                    	continue;
                    default:
						rgb_matrix_set_color(index, 0, 0, 0);
                    	continue;
                }
                hsv.h = (frac(new_hue, 360) + hsv.h) % 255;
                rgb = rgb_matrix_hsv_to_rgb(hsv);  // Convert the HSV color back to RGB
                rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_BREATHING
