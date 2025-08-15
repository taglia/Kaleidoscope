/* Kaleidoscope-Hardware-Keyboardio-Preonic -- Keyboardio Preonic hardware support for Kaleidoscope
 * Copyright 2018-2025 Keyboard.io, inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * Additional Permissions:
 * As an additional permission under Section 7 of the GNU General Public
 * License Version 3, you may link this software against a Vendor-provided
 * Hardware Specific Software Module under the terms of the MCU Vendor
 * Firmware Library Additional Permission Version 1.0.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BUILD_INFORMATION
#define BUILD_INFORMATION "locally built on " __DATE__ " at " __TIME__
#endif

#include "Kaleidoscope.h"
#include "Kaleidoscope-EEPROM-Settings.h"
#include "Kaleidoscope-EEPROM-Keymap.h"
#include "Kaleidoscope-Escape-OneShot.h"
#include "Kaleidoscope-FirmwareVersion.h"
#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-Macros.h"
#include "Kaleidoscope-MouseKeys.h"
#include "Kaleidoscope-OneShot.h"
#include "Kaleidoscope-Qukeys.h"
#include "Kaleidoscope-SpaceCadet.h"
#include "Kaleidoscope-DynamicMacros.h"
#include "Kaleidoscope-LayerNames.h"
#include "Kaleidoscope-HostPowerManagement.h"
#include "Kaleidoscope-LEDControl.h"
#include "Kaleidoscope-Keyclick.h"
#include "Kaleidoscope-TapDance.h"

#include "Kaleidoscope-LEDEffect-Rainbow.h"
#include "Kaleidoscope-LEDEffect-BootGreeting.h"
// Support for shared palettes for other plugins, like Colormap below
#include "Kaleidoscope-LED-Palette-Theme.h"

// Support for an LED mode that lets one configure per-layer color maps
#include "Kaleidoscope-PreonicColormap.h"


// Support for setting and saving the default LED mode
#include "Kaleidoscope-DefaultLEDModeConfig.h"

// Support for changing the brightness of the LEDs
#include "Kaleidoscope-LEDBrightnessConfig.h"

// Support for showing device status with the LED Indicators
#include "Kaleidoscope-LEDIndicators.h"


/** This 'enum' is a list of all the macros used by the Model 100's firmware
  * The names aren't particularly important. What is important is that each
  * is unique.
  *
  * These are the names of your macros. They'll be used in two places.
  * The first is in your keymap definitions. There, you'll use the syntax
  * `M(MACRO_NAME)` to mark a specific keymap position as triggering `MACRO_NAME`
  *
  * The second usage is in the 'switch' statement in the `macroAction` function.
  * That switch statement actually runs the code associated with a macro when
  * a macro key is pressed.
  */

enum {
  MACRO_VERSION_INFO,
  MACRO_ANY,
  MACRO_TOGGLE_MODE,
  MACRO_BT_SELECT_1,  // Select slot 1
  MACRO_BT_SELECT_2,  // Select slot 2
  MACRO_BT_SELECT_3,  // Select slot 3
  MACRO_BT_SELECT_4,  // Select slot 4
  MACRO_BT_PAIR,      // Start pairing for selected slot
  MACRO_BT_OFF,
  MACRO_BATTERY_LEVEL,  // Report current battery level
  // Cesare's Macros
};

// Layers
enum {
  QWERTY,
  RAISE,
  LOWER,
  ADJUST,
  FUN
};

// Custom plugin for momentary ADJUST layer activation
namespace kaleidoscope {
namespace plugin {

class MomentaryAdjustLayer : public Plugin {
 public:
  EventHandlerResult afterEachCycle() {
    // Check if both RAISE and LOWER are currently pressed
    bool raise_pressed = Runtime.device().isKeyswitchPressed(KeyAddr(5, 7));
    bool lower_pressed = Runtime.device().isKeyswitchPressed(KeyAddr(5, 4));
    
    if (raise_pressed && lower_pressed) {
      // Both keys pressed - activate ADJUST layer if not already active
      if (!Layer.isActive(ADJUST)) {
        Layer.activate(ADJUST);
      }
    } else {
      // At least one key released - deactivate ADJUST layer if active
      if (Layer.isActive(ADJUST)) {
        Layer.deactivate(ADJUST);
      }
    }
    
    return EventHandlerResult::OK;
  }
};

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::MomentaryAdjustLayer MomentaryAdjustLayer;

#define Key_Star LSHIFT(Key_8)
#define Key_Plus LSHIFT(Key_Equals)

//////////////////////
// TapDance config

enum {
  TD_CAPS_LOCK,
  TD_AW1,
  TD_AW2,
  TD_AW3,
  TD_AW4,
  TD_AW5,
  TD_AW6,
  TD_AW7,
  TD_AW8,
  TD_AW9
};

void tapDanceAction(uint8_t tap_dance_index, KeyAddr key_addr, uint8_t tap_count,
                    kaleidoscope::plugin::TapDance::ActionType tap_dance_action) {
  switch (tap_dance_index) {
    case TD_CAPS_LOCK:
      return tapDanceActionKeys(tap_count, tap_dance_action, Key_LeftShift, Key_CapsLock);
    case TD_AW1:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(1), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(1), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW2:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(2), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(2), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW3:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(3), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(3), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW4:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(4), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(4), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW5:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(5), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(5), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW6:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(6), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(6), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW7:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(7), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(7), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW8:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(8), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(8), U(LeftAlt), U(LeftShift)));
      }
      break;
    case TD_AW9:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        if (tap_count == 1)
          Macros.play(MACRO(I(50), D(LeftAlt), T(9), U(LeftAlt)));
        else if (tap_count == 2)
          Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), T(9), U(LeftAlt), U(LeftShift)));
      }
      break;
  }
}

// clang-format off
KEYMAPS(
  [QWERTY] = KEYMAP
  (
   Consumer_VolumeDecrement,  Consumer_VolumeIncrement,  M(MACRO_ANY),  ShiftToLayer(FUN),  Consumer_PlaySlashPause,
   Key_F1,                    TD(TD_AW1),                TD(TD_AW2),    TD(TD_AW3),         TD(TD_AW4),               TD(TD_AW5),       TD(TD_AW6),  TD(TD_AW7),           TD(TD_AW8),    TD(TD_AW9),    Key_0,         LGUI(LCTRL(Key_Q)),
   Key_Backtick,              Key_Q,                     Key_W,         Key_E,              Key_R,                    Key_T,            Key_Y,       Key_U,                Key_I,         Key_O,         Key_P,               Key_Backspace,
   Key_Tab,                   Key_A,                     Key_S,         Key_D,              Key_F,                    Key_G,            Key_H,       Key_J,                Key_K,         Key_L,         Key_Semicolon,       Key_Quote,
   TD(TD_CAPS_LOCK),          Key_Z,                     Key_X,         Key_C,              Key_V,                    Key_B,            Key_N,       Key_M,                Key_Comma,     Key_Period,    Key_Slash,           Key_Enter,
   Key_Escape,                Key_LeftControl,           Key_LeftAlt,   Key_LeftGui,        ShiftToLayer(LOWER),      Key_LeftControl,  Key_Space,   ShiftToLayer(RAISE),  Key_RightGui,  Key_RightAlt,  Key_RightControl,    Key_Hyper
  ),

  [RAISE] = KEYMAP
  (
   ___,                   ___,            ___,             ___,                         ___,
   Key_Escape,            Key_F1,         Key_F2,          Key_F3,                      Key_F4,                   Key_F5,                  Key_F6,              Key_F7,         Key_F8,         Key_F9,                   Key_F10,                   Key_F11,
   LSHIFT(Key_Backtick),  LSHIFT(Key_1),  LSHIFT(Key_2),   LSHIFT(Key_3),               LSHIFT(Key_4),            LSHIFT(Key_5),           LSHIFT(Key_6),       LSHIFT(Key_7),  LSHIFT(Key_8),  LSHIFT(Key_9),            LSHIFT(Key_0),             Key_Delete,
   ___,                   ___,            Key_VolumeDown,  Key_Mute,                    Key_VolumeUp,             ___,                     LSHIFT(Key_Equals),  Key_Minus,      ___,            LSHIFT(Key_LeftBracket),  LSHIFT(Key_RightBracket),  Key_Backslash,
   ___,                   ___,            ___,             Consumer_ScanPreviousTrack,  Consumer_PlaySlashPause,  Consumer_ScanNextTrack,  Key_Equals,          ___,            ___,            Key_LeftBracket,          Key_RightBracket,          ___,
   ___,                   ___,            ___,             ___,                         ___,                      ___,                     ___,                 ___,            ___,            ___,                      ___,                       ___
  ),

  [LOWER] = KEYMAP
  (
   ___,         ___,     ___,     ___,     ___,
   Key_Escape,  Key_F1,  Key_F2,  Key_F3,  Key_F4,  Key_F5,  Key_F6,         Key_F7,         Key_F8,       Key_F9,          Key_F10,  Key_Backspace,
   ___,         Key_1,   Key_2,   Key_3,   Key_4,   Key_5,   Key_6,          Key_7,          Key_8,        Key_9,           Key_0,    Key_Delete,
   ___,         ___,     ___,     ___,     ___,     ___,     Key_LeftArrow,  Key_DownArrow,  Key_UpArrow,  Key_RightArrow,  ___,      LSHIFT(Key_Backslash),
   ___,         ___,     ___,     ___,     ___,     ___,     Key_Home,       Key_PageDown,   Key_PageUp,   Key_End,         ___,      ___,
   ___,         ___,     ___,     ___,     ___,     ___,     ___,            ___,            ___,          ___,             ___,      ___
  ),

  [ADJUST] = KEYMAP
  (
   ___,  ___,      ___,      ___,      ___,
   ___,  ___,      ___,      ___,      ___,      ___,      ___,               ___,                ___,                ___,               ___,            ___,
   ___,  Key_F1,   Key_F2,   Key_F3,   Key_F4,   Key_F5,   Key_F6,            Key_F7,             Key_F8,             Key_F9,            Key_F10,        ___,
   ___,  Key_F11,  Key_F12,  Key_F13,  Key_F14,  Key_F15,  Key_mouseL,        Key_mouseDn,        Key_mouseUp,        Key_mouseR,        Key_mouseBtnL,  Key_mouseBtnR,
   ___,  ___,      ___,      ___,      ___,      ___,      Key_mouseScrollL,  Key_mouseScrollDn,  Key_mouseScrollUp,  Key_mouseScrollR,  ___,            ___,
   ___,  ___,      ___,      ___,      ___,      ___,      ___,               ___,                ___,                ___,               ___,            ___
  ),

  [FUN] = KEYMAP
  (
   ___,                     ___,                                                                             ___,                   ___,                   ___,
   Key_BLEOff,              Key_BLESelectDevice1,                                                            Key_BLESelectDevice2,  Key_BLESelectDevice3,  Key_BLESelectDevice4,  Key_BLEStartPairing,  ___,  ___,  ___,  ___,  ___,  M(MACRO_VERSION_INFO),
   Key_LEDEffectNext,___,   Consumer_VolumeIncrement,___,                                                    ___,                   ___,                   ___,                   ___,                  ___,  ___,  ___,  ___,
   ___,                     Consumer_ScanPreviousTrack,Consumer_VolumeDecrement,Consumer_ScanNextTrack,___,  ___,                   ___,                   ___,                   ___,                  ___,  ___,  ___,
   Key_ToggleKeyclick,___,  Consumer_Mute,                                                                   ___,                   ___,                   ___,                   ___,                  ___,  ___,  ___,  ___,  ___,
   M(MACRO_BATTERY_LEVEL),  ___,                                                                             ___,                   ___,                   ___,                   ___,                  ___,  ___,  ___,  ___,  ___,  ___
  )
);


// Colors names of the EGA palette, for convenient use in colormaps. Should
// match the palette definition below. Optional, one can just use the indexes
// directly, too.
enum {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  LIGHT_GRAY,
  DARK_GRAY,
  BRIGHT_BLUE,
  BRIGHT_GREEN,
  BRIGHT_CYAN,
  BRIGHT_RED,
  BRIGHT_MAGENTA,
  YELLOW,
  WHITE
};

// Define an EGA palette. Conveniently, that's exactly 16 colors, just like the
// limit of LEDPaletteTheme.
PALETTE(
    CRGB(0x00, 0x00, 0x00),  // [0x0] black
    CRGB(0x00, 0x00, 0xaa),  // [0x1] blue
    CRGB(0x00, 0xaa, 0x00),  // [0x2] green
    CRGB(0x00, 0xaa, 0xaa),  // [0x3] cyan
    CRGB(0xaa, 0x00, 0x00),  // [0x4] red
    CRGB(0xaa, 0x00, 0xaa),  // [0x5] magenta
    CRGB(0xaa, 0x55, 0x00),  // [0x6] brown
    CRGB(0xaa, 0xaa, 0xaa),  // [0x7] light gray
    CRGB(0x55, 0x55, 0x55),  // [0x8] dark gray
    CRGB(0x55, 0x55, 0xff),  // [0x9] bright blue
    CRGB(0x55, 0xff, 0x55),  // [0xa] bright green
    CRGB(0x55, 0xff, 0xff),  // [0xb] bright cyan
    CRGB(0xff, 0x55, 0x55),  // [0xc] bright red
    CRGB(0xff, 0x55, 0xff),  // [0xd] bright magenta
    CRGB(0xff, 0xff, 0x55),  // [0xe] yellow
    CRGB(0xff, 0xff, 0xff)   // [0xf] white
)

COLORMAPS(
    [0] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [1] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [2] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [3] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [4] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [5] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [6] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [7] = COLORMAP (BLACK,  BLACK, BLACK, BLACK),
    [8] = COLORMAP (BLACK,  BLACK, BLACK, BLACK)  
)


// clang-format on

KALEIDOSCOPE_INIT_PLUGINS(
  // ----------------------------------------------------------------------
  // Chrysalis plugins

  // The EEPROMSettings & EEPROMKeymap plugins make it possible to have an
  // editable keymap in EEPROM.
  EEPROMSettings,
  EEPROMKeymap,

  // Focus allows bi-directional communication with the host, and is the
  // interface through which the keymap in EEPROM can be edited.
  Focus,

  // FocusSettingsCommand adds a few Focus commands, intended to aid in
  // changing some settings of the keyboard, such as the default layer (via the
  // `settings.defaultLayer` command)
  FocusSettingsCommand,

  // FocusEEPROMCommand adds a set of Focus commands, which are very helpful in
  // both debugging, and in backing up one's EEPROM contents.
  FocusEEPROMCommand,

  // The FirmwareVersion plugin lets Chrysalis query the version of the firmware
  // programmatically.
  FirmwareVersion,

  // The LayerNames plugin allows Chrysalis to display - and edit - custom layer
  // names, to be shown instead of the default indexes.
  LayerNames,

  // ----------------------------------------------------------------------
  // Keystroke-handling plugins

  // The Qukeys plugin enables the "Secondary action" functionality in
  // Chrysalis. Keys with secondary actions will have their primary action
  // performed when tapped, but the secondary action when held.
  Qukeys,

  // SpaceCadet can turn your shifts into parens on tap, while keeping them as
  // Shifts when held. SpaceCadetConfig lets Chrysalis configure some aspects of
  // the plugin.
  SpaceCadet,
  SpaceCadetConfig,

  // Enables the "Sticky" behavior for modifiers, and the "Layer shift when
  // held" functionality for layer keys.
  OneShot,
  OneShotConfig,
  EscapeOneShot,
  EscapeOneShotConfig,

  // The macros plugin adds support for macros
  Macros,

  // Enables dynamic, Chrysalis-editable macros.
  DynamicMacros,

  // The MouseKeys plugin lets you add keys to your keymap which move the mouse.
  MouseKeys,
  MouseKeysConfig,

  // Custom plugin for momentary ADJUST layer when RAISE+LOWER are held
  MomentaryAdjustLayer,

  // LEDControl provides support for other LED modes
  LEDControl,

  // Enables setting, saving (via Chrysalis), and restoring (on boot) the
  // default LED mode.
  DefaultLEDModeConfig,


  // We start with the LED effect that turns off all the LEDs.
  LEDOff,

  LEDPaletteTheme,

  // The Colormap effect makes it possible to set up per-layer colormaps
  PreonicColormapEffect,
  DefaultPreonicColormap,

  // The HostPowerManagement plugin allows us to turn LEDs off when then host
  // goes to sleep, and resume them when it wakes up.
  // HostPowerManagement,

  LEDBrightnessConfig,
  LEDRainbowEffect,

  // The Keyclick plugin for audible feedback
  Keyclick,
  // LEDIndicators shows device status indicators using the LEDs
  // It should be listed after all other LED plugins
  LEDIndicators,
  TapDance);


void configureIndicators() {
  // Configure LED mapping to use our four LEDs
  static const KeyAddr indicator_leds[] = {
    KeyAddr(0, 0),
    KeyAddr(0, 1),
    KeyAddr(0, 2),
    KeyAddr(0, 3)};
  LEDIndicators.setSlots(4, indicator_leds);
}


static void versionInfoMacro(uint8_t key_state) {
  if (keyToggledOn(key_state)) {
    Macros.type(PSTR("Keyboardio Preonic - Firmware version "));
    Macros.type(PSTR(BUILD_INFORMATION));
  }
}

/** anyKeyMacro is used to provide the functionality of the 'Any' key.
 *
 * When the 'any key' macro is toggled on, a random alphanumeric key is
 * selected. While the key is held, the function generates a synthetic
 * keypress event repeating that randomly selected key.
 *
 */

static void anyKeyMacro(KeyEvent &event) {
  if (keyToggledOn(event.state)) {
    event.key.setKeyCode(Key_A.getKeyCode() + (uint8_t)(millis() % 36));
    event.key.setFlags(0);
  }
}

/** macroAction dispatches keymap events that are tied to a macro
    to that macro. It takes two uint8_t parameters.

    The first is the macro being called (the entry in the 'enum' earlier in this file).
    The second is the state of the keyswitch. You can use the keyswitch state to figure out
    if the key has just been toggled on, is currently pressed or if it's just been released.

    The 'switch' statement should have a 'case' for each entry of the macro enum.
    Each 'case' statement should call out to a function to handle the macro in question.

 */

// Helper function to report battery level
static void batteryLevelMacro(uint8_t key_state) {
  if (keyToggledOn(key_state)) {
    // Get the battery level
    uint8_t battery_level = kaleidoscope::Runtime.device().batteryGauge().getBatteryLevel();

    // Report battery level with a message
    Macros.type(PSTR("Battery Status:\n"));

    // Battery percentage
    char percentage[5];
    snprintf(percentage, sizeof(percentage), "%d%%", battery_level);
    Macros.type(PSTR("Level: "));
    Macros.type(percentage);
    Macros.type(PSTR("\n"));

    // Raw battery level
    char raw_level[5];
    snprintf(raw_level, sizeof(raw_level), "%d", kaleidoscope::Runtime.device().batteryGauge().getRawBatteryLevel());
    Macros.type(PSTR("Raw Level: "));
    Macros.type(raw_level);
    Macros.type(PSTR("\n"));

    // Battery voltage
    char voltage[8];
    snprintf(voltage, sizeof(voltage), "%.2fV", kaleidoscope::Runtime.device().batteryGauge().getVoltage() / 1000.0f);
    Macros.type(PSTR("Voltage: "));
    Macros.type(voltage);
    Macros.type(PSTR("\n"));

    // Charge rate
    int16_t charge_rate = kaleidoscope::Runtime.device().batteryGauge().getChargeRate();
    char rate[8];
    snprintf(rate, sizeof(rate), "%.2f%%/hr", charge_rate * 0.208f);
    Macros.type(PSTR("Charge Rate: "));
    Macros.type(rate);
    Macros.type(PSTR("\n"));

    // Charging status
    Macros.type(PSTR("Power Source: "));
    if (kaleidoscope::Runtime.device().batteryCharger().hasPower()) {
      Macros.type(PSTR("Connected\n"));

      Macros.type(PSTR("Charging Status: "));
      uint8_t charging_state = kaleidoscope::Runtime.device().batteryCharger().getChargingState();

      // Using defined enum values from BQ24075
      typedef kaleidoscope::driver::battery_charger::BQ24075<kaleidoscope::device::keyboardio::PreonicBatteryChargerProps> BQ24075;

      switch (charging_state) {
      case BQ24075::CHARGING:
        Macros.type(PSTR("Charging\n"));
        break;
      case BQ24075::CHARGE_COMPLETE:
        Macros.type(PSTR("Charge Complete\n"));
        break;
      case BQ24075::CHARGE_FAULT:
        Macros.type(PSTR("Fault Detected\n"));
        break;
      case BQ24075::NO_BATTERY:
        Macros.type(PSTR("No Battery\n"));
        break;
      case BQ24075::BATTERY_DISCONNECTED:
        Macros.type(PSTR("Battery Disconnected\n"));
        break;
      default:
        Macros.type(PSTR("Not Charging\n"));
        break;
      }

      // No need to print static configuration values in the status report
    } else {
      Macros.type(PSTR("Battery\n"));
    }

    // Alert status
    Macros.type(PSTR("Alerts: "));
    if (kaleidoscope::Runtime.device().batteryGauge().hasLowBatteryAlert()) {
      Macros.type(PSTR("Low Battery "));
    }
    if (kaleidoscope::Runtime.device().batteryGauge().hasChangeAlert()) {
      Macros.type(PSTR("Level Changed "));
    }
    if (kaleidoscope::Runtime.device().batteryGauge().hasLowVoltageAlert()) {
      Macros.type(PSTR("Low Voltage "));
    }
    if (kaleidoscope::Runtime.device().batteryGauge().hasHighVoltageAlert()) {
      Macros.type(PSTR("High Voltage "));
    }
    if (!kaleidoscope::Runtime.device().batteryGauge().hasLowBatteryAlert() &&
        !kaleidoscope::Runtime.device().batteryGauge().hasChangeAlert() &&
        !kaleidoscope::Runtime.device().batteryGauge().hasLowVoltageAlert() &&
        !kaleidoscope::Runtime.device().batteryGauge().hasHighVoltageAlert()) {
      Macros.type(PSTR("None"));
    }
    Macros.type(PSTR("\n"));

    // Hibernate status
    if (kaleidoscope::Runtime.device().batteryGauge().isHibernating()) {
      Macros.type(PSTR("Status: Hibernating\n"));
    } else {
      Macros.type(PSTR("Status: Active\n"));
    }

    // Firmware version
    char version[6];
    snprintf(version, sizeof(version), "0x%04X", kaleidoscope::Runtime.device().batteryGauge().getVersion());
    Macros.type(PSTR("Gauge IC: "));
    Macros.type(version);
    Macros.type(PSTR("\n"));
  }
}

const macro_t *macroAction(uint8_t macro_id, KeyEvent &event) {
  switch (macro_id) {
  case MACRO_VERSION_INFO:
    versionInfoMacro(event.state);
    break;
  case MACRO_ANY:
    anyKeyMacro(event);
    break;
  case MACRO_TOGGLE_MODE:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().toggleHostConnectionMode();
    }
    break;
  case MACRO_BT_SELECT_1:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().selectDevice(1);
      kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
    }
    break;
  case MACRO_BT_SELECT_2:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().selectDevice(2);
      kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
    }
    break;
  case MACRO_BT_SELECT_3:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().selectDevice(3);
      kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
    }
    break;
  case MACRO_BT_SELECT_4:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().selectDevice(4);
      kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
    }
    break;
  case MACRO_BT_PAIR:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().startDiscoverableAdvertising();
    }
    break;
  case MACRO_BT_OFF:
    if (keyToggledOn(event.state)) {
      kaleidoscope::Runtime.device().ble().stopAdvertising();
      kaleidoscope::Runtime.device().ble().disconnect();
      kaleidoscope::Runtime.device().setHostConnectionMode(MODE_USB);
    }
    break;
  case MACRO_BATTERY_LEVEL:
    batteryLevelMacro(event.state);
    break;
  }
  return MACRO_NONE;
}

void setup() {
  /////////////////////
  // Qukeys
  QUKEYS(
      kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 1), Key_LeftShift),
      kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 10), Key_RightShift),
      kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 4), Key_LeftAlt),
      kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 7), Key_RightAlt)
      );

  Kaleidoscope.setup();
  configureIndicators();

  EEPROMKeymap.setup(9);
  PreonicColormapEffect.max_layers(9);
  LEDRainbowEffect.brightness(25);

  DynamicMacros.reserve_storage(512);

  LayerNames.reserve_storage(128);

  // Disable Keyclick by default
  Keyclick.disable();

  Layer.move(EEPROMSettings.default_layer());

  // To avoid any surprises, SpaceCadet is turned off by default. However, it
  // can be permanently enabled via Chrysalis, so we should only disable it if
  // no configuration exists.
  SpaceCadetConfig.disableSpaceCadetIfUnconfigured();

  //  DefaultLEDModeConfig.activateLEDModeIfUnconfigured(&LEDRainbowEffect);

  DefaultLEDModeConfig.activateLEDModeIfUnconfigured(&LEDOff);

  //kaleidoscope::Runtime.device().ble().selectDevice(1);
  //kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
}

void loop() {
  Kaleidoscope.loop();
}
