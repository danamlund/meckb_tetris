/* Copyright 2017 Dan Amlund Thomsen
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
#include "tetris.h"
#include "tetris_text.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[0] = KEYMAP(
  F(0),    KC_UP,   KC_TRNS,   \
  KC_LEFT, KC_DOWN, KC_RIGHT \
)
};

const uint16_t PROGMEM fn_actions[] = {
  [0] = ACTION_FUNCTION(0),  // Calls action_function()
};

static uint8_t key_presses = 0;
static uint16_t timer = 0;

static uint8_t tetris_running = 0;
static int tetris_keypress = 0;

void action_function(keyrecord_t *record, uint8_t id, uint8_t opt) {
  if (id == 0 && record->event.pressed) {
    tetris_running = ! tetris_running;
    if (tetris_running) {
      tetris_keypress = 0;
      timer = 0;
      // set randomness using number of keys pressed
      tetris_start(key_presses);
    }
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    key_presses++;
  }

  if (tetris_running && record->event.pressed) {
    tetris_keypress = 0;
    switch (keycode) {
    case KC_UP: tetris_keypress = 1; break;
    case KC_LEFT: tetris_keypress = 2; break;
    case KC_DOWN: tetris_keypress = 3; break;
    case KC_RIGHT: tetris_keypress = 4; break;
    }
    if (tetris_keypress != 0) {
      return false;
    }
  }

  return true;
}

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
}


// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
  if (tetris_running) {
    timer++;
    if (timer > 1000) {
      // every 1000 times this is run is about 100 ms.
      if (!tetris_tick(100)) {
        // game over
        tetris_running = 0;
      }
      timer = 0;
    }
  }      
}

void send_keycode(uint16_t keycode) {
  register_code(keycode);
  unregister_code(keycode);
}

void send_keycode_shift(uint16_t keycode) {
  register_code(KC_LSFT);
  register_code(keycode);
  unregister_code(keycode);
  unregister_code(KC_LSFT);
}

void tetris_send_up(void) {
  send_keycode(KC_UP);
}
void tetris_send_left(void) {
  send_keycode(KC_LEFT);
}
void tetris_send_down(void) {
  send_keycode(KC_DOWN);
}
void tetris_send_right(void) {
  send_keycode(KC_RGHT);
}

void tetris_send_home(void) {
  send_keycode(KC_HOME);
}
void tetris_send_end(void) {
  send_keycode(KC_END);
}

void tetris_send_backspace(void) {
  send_keycode(KC_BSPC);
}
void tetris_send_delete(void) {
  send_keycode(KC_DEL);
}

void tetris_send_string(const char *s) {
  for (int i = 0; s[i] != 0; i++) {
    if (s[i] >= 'a' && s[i] <= 'z') {
      send_keycode(KC_A + (s[i] - 'a'));
    } else if (s[i] >= 'A' && s[i] <= 'Z') {
      send_keycode_shift(KC_A + (s[i] - 'A'));
    } else if (s[i] >= '1' && s[i] <= '9') {
      send_keycode(KC_1 + (s[i] - '1'));
    } else {
      switch (s[i]) {
      case ' ': send_keycode(KC_SPACE); break;
      case '.': send_keycode(KC_DOT); break;
      case '0': send_keycode(KC_0); break;
      }
    }
  }
}

void tetris_send_newline(void) {
  send_keycode(KC_ENT);
}

int tetris_get_keypress(void) {
  int out = tetris_keypress;
  tetris_keypress = 0;
  return out;
}
