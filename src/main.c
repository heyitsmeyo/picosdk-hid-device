/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *tud_hid_report_complete_cb()
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED     = 1000,
  BLINK_SUSPENDED   = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  // init device stack on configured roothub port
  tusb_rhport_init_t dev_init = {
    .role  = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

uint8_t char_to_hid(char c, uint8_t* modifier) {
    *modifier = 0;  // default no modifier

    if (c >= 'a' && c <= 'z') {
        return HID_KEY_A + (c - 'a');
    } else if (c >= 'A' && c <= 'Z') {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        return HID_KEY_A + (c - 'A');
    } else if (c == ' ') {
        return HID_KEY_SPACE;
    } else if (c == '.') {
        return HID_KEY_PERIOD;
    } else if (c == ',') {
        return HID_KEY_COMMA;
    } else {
        return 0;
    }
}

void open_terminal() {
    uint8_t keycode[6] = {0};

    // Send CTRL+ALT+T to open terminal
    keycode[0] = HID_KEY_T;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTALT, keycode);
    board_delay(500);

    // Release all keys
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    board_delay(2000);  // Wait a bit longer for terminal to open
}


void send_string(const char* str) {
    uint8_t keycode[6] = {0};
     

    while (*str) {
        char c = *str;
        uint8_t modifier = 0;
        uint8_t hid_code = char_to_hid(c, &modifier);

        if (hid_code != 0) {
            keycode[0] = hid_code;

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
            board_delay(100);

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            board_delay(1000);
        }

        str++;
    }

    // Finally, press Enter
    keycode[0] = HID_KEY_ENTER;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    board_delay(100);

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    board_delay(1000);
}


void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static bool reports_sent = false;

static void send_hid_report()
{
  if (!tud_hid_ready()) return;

  if (reports_sent) {
    return;
  }
  
  open_terminal(); 

  const char* strings[] = {
    "cd",
    "cd Desktop",
    "gedit tarek.txt",
    "Hello world My name is Tarek"
  };

  int n = sizeof(strings) / sizeof(strings[0]);
  printf("Total number of strings: %d\n", n);

  for (int i = 0; i < n; i++) {
    send_string(strings[i]);
  }

  reports_sent = true;
}

void hid_task(void)
{
  if (!reports_sent) {
	board_delay(2000);
    send_hid_report();
  }
}

/*static void send_stylus_touch(uint16_t x, uint16_t y, bool state)
{
  if (!tud_hid_ready()) return;

  static bool has_stylus_pen = false;

  hid_stylus_report_t report =
  {
    .attr = 0,
    .x    = x,
    .y    = y
  };

  if (state)
  {
    report.attr = STYLUS_ATTR_TIP_SWITCH | STYLUS_ATTR_IN_RANGE;
    tud_hid_report(REPORT_ID_STYLUS_PEN, &report, sizeof(report));
    has_stylus_pen = true;
  }
  else
  {
    report.attr = 0;
    if (has_stylus_pen)
    {
      tud_hid_report(REPORT_ID_STYLUS_PEN, &report, sizeof(report));
      has_stylus_pen = false;
    }
  }
}*/

// Invoked when sent REPORT successfully to host
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    //send_hid_report();
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == REPORT_ID_KEYBOARD)
  {
    if (bufsize < 1) return;

    uint8_t const kbd_leds = buffer[0];

    if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
    {
      blink_interval_ms = 0;
      board_led_write(true);
    }
    else
    {
      board_led_write(false);
      blink_interval_ms = BLINK_MOUNTED;
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  if (!blink_interval_ms) return;

  if (board_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  led_state = !led_state;
  board_led_write(led_state);
}
