/* Kaleidoscope-Hardware-Keyboardio-Preonic -- Cesare's Custom Configuration
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

// =============================================================================
// INCLUDES
// =============================================================================

#include "Kaleidoscope.h"

// Core functionality
#include "Kaleidoscope-EEPROM-Settings.h"
#include "Kaleidoscope-EEPROM-Keymap.h"
#include "Kaleidoscope-FocusSerial.h"
#include "Kaleidoscope-FirmwareVersion.h"

// Key behavior plugins
#include "Kaleidoscope-Escape-OneShot.h"
#include "Kaleidoscope-OneShot.h"
#include "Kaleidoscope-Qukeys.h"
#include "Kaleidoscope-SpaceCadet.h"
#include "Kaleidoscope-TapDance.h"

// Macro and layer plugins
#include "Kaleidoscope-Macros.h"
#include "Kaleidoscope-DynamicMacros.h"
#include "Kaleidoscope-LayerNames.h"
#include "Kaleidoscope-MouseKeys.h"

// Hardware and connectivity
#include "Kaleidoscope-HostPowerManagement.h"
#include "Kaleidoscope-Keyclick.h"

// LED effects and control
#include "Kaleidoscope-LEDControl.h"
#include "Kaleidoscope-LEDEffect-Rainbow.h"
#include "Kaleidoscope-LEDEffect-BootGreeting.h"
#include "Kaleidoscope-LED-Palette-Theme.h"
#include "Kaleidoscope-PreonicColormap.h"
#include "Kaleidoscope-DefaultLEDModeConfig.h"
#include "Kaleidoscope-LEDBrightnessConfig.h"
#include "Kaleidoscope-LEDIndicators.h"

// =============================================================================
// LAYER DEFINITIONS
// =============================================================================

enum {
  QWERTY,  // Base layer
  RAISE,   // Numbers, symbols, F-keys
  LOWER,   // Numbers, navigation
  ADJUST,  // Mouse, advanced F-keys (activated by RAISE+LOWER)
  FUN      // Bluetooth, system controls
};

// =============================================================================
// MACRO DEFINITIONS
// =============================================================================

enum {
  MACRO_VERSION_INFO,
  MACRO_ANY,
  MACRO_TOGGLE_MODE,
  MACRO_BT_SELECT_1, MACRO_BT_SELECT_2, MACRO_BT_SELECT_3, MACRO_BT_SELECT_4,
  MACRO_BT_PAIR,
  MACRO_BT_OFF,
  MACRO_BATTERY_LEVEL,
};

// =============================================================================
// CUSTOM PLUGIN: MOMENTARY ADJUST LAYER
// =============================================================================

namespace kaleidoscope {
namespace plugin {

/**
 * MomentaryAdjustLayer activates the ADJUST layer only while both
 * RAISE and LOWER keys are held simultaneously. The layer deactivates
 * immediately when either key is released.
 */
class MomentaryAdjustLayer : public Plugin {
 public:
  EventHandlerResult afterEachCycle() {
    // Check if both RAISE (row 5, col 7) and LOWER (row 5, col 4) are pressed
    bool raise_pressed = Runtime.device().isKeyswitchPressed(KeyAddr(5, 7));
    bool lower_pressed = Runtime.device().isKeyswitchPressed(KeyAddr(5, 4));
    
    if (raise_pressed && lower_pressed) {
      if (!Layer.isActive(ADJUST)) {
        Layer.activate(ADJUST);
      }
    } else {
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

// =============================================================================
// TAP DANCE CONFIGURATION
// =============================================================================

// Convenient key definitions
#define Key_Star LSHIFT(Key_8)
#define Key_Plus LSHIFT(Key_Equals)

enum {
  TD_CAPS_LOCK,  // Tap: Left Shift, Double-tap: Caps Lock
  TD_AW1, TD_AW2, TD_AW3, TD_AW4, TD_AW5,  // Alt+Number window switching
  TD_AW6, TD_AW7, TD_AW8, TD_AW9
};

/**
 * Helper function for Alt+Number window switching TapDance actions.
 * Single tap: Alt+Number, Double tap: Shift+Alt+Number
 */
void handleAltWindowSwitch(uint8_t number, uint8_t tap_count) {
  Key number_key;
  switch (number) {
    case 1: number_key = Key_1; break;
    case 2: number_key = Key_2; break;
    case 3: number_key = Key_3; break;
    case 4: number_key = Key_4; break;
    case 5: number_key = Key_5; break;
    case 6: number_key = Key_6; break;
    case 7: number_key = Key_7; break;
    case 8: number_key = Key_8; break;
    case 9: number_key = Key_9; break;
    default: return;
  }
  
  if (tap_count == 1) {
    Macros.play(MACRO(I(50), D(LeftAlt), Tr(number_key), U(LeftAlt)));
  } else if (tap_count == 2) {
    Macros.play(MACRO(I(50), D(LeftShift), D(LeftAlt), Tr(number_key), U(LeftAlt), U(LeftShift)));
  }
}

void tapDanceAction(uint8_t tap_dance_index, KeyAddr key_addr, uint8_t tap_count,
                    kaleidoscope::plugin::TapDance::ActionType tap_dance_action) {
  switch (tap_dance_index) {
    case TD_CAPS_LOCK:
      return tapDanceActionKeys(tap_count, tap_dance_action, Key_LeftShift, Key_CapsLock);
      
    // Alt+Number window switching (TD_AW1 through TD_AW9)
    case TD_AW1: case TD_AW2: case TD_AW3: case TD_AW4: case TD_AW5:
    case TD_AW6: case TD_AW7: case TD_AW8: case TD_AW9:
      if (tap_dance_action == kaleidoscope::plugin::TapDance::Timeout) {
        uint8_t number = (tap_dance_index - TD_AW1) + 1;  // Convert to 1-9
        handleAltWindowSwitch(number, tap_count);
      }
      break;
  }
}

// =============================================================================
// KEYMAPS
// =============================================================================

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
// clang-format on

// =============================================================================
// LED COLOR PALETTE CONFIGURATION
// =============================================================================

// EGA color palette for LED themes
enum {
  BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY,
  DARK_GRAY, BRIGHT_BLUE, BRIGHT_GREEN, BRIGHT_CYAN, 
  BRIGHT_RED, BRIGHT_MAGENTA, YELLOW, WHITE
};

PALETTE(
  CRGB(0x00, 0x00, 0x00),  // black
  CRGB(0x00, 0x00, 0xaa),  // blue
  CRGB(0x00, 0xaa, 0x00),  // green
  CRGB(0x00, 0xaa, 0xaa),  // cyan
  CRGB(0xaa, 0x00, 0x00),  // red
  CRGB(0xaa, 0x00, 0xaa),  // magenta
  CRGB(0xaa, 0x55, 0x00),  // brown
  CRGB(0xaa, 0xaa, 0xaa),  // light gray
  CRGB(0x55, 0x55, 0x55),  // dark gray
  CRGB(0x55, 0x55, 0xff),  // bright blue
  CRGB(0x55, 0xff, 0x55),  // bright green
  CRGB(0x55, 0xff, 0xff),  // bright cyan
  CRGB(0xff, 0x55, 0x55),  // bright red
  CRGB(0xff, 0x55, 0xff),  // bright magenta
  CRGB(0xff, 0xff, 0x55),  // yellow
  CRGB(0xff, 0xff, 0xff)   // white
)

// Default colormap (all LEDs off)
COLORMAPS(
  [0] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [1] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [2] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [3] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [4] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [5] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [6] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [7] = COLORMAP(BLACK, BLACK, BLACK, BLACK),
  [8] = COLORMAP(BLACK, BLACK, BLACK, BLACK)
)

// =============================================================================
// PLUGIN INITIALIZATION
// =============================================================================

KALEIDOSCOPE_INIT_PLUGINS(
  // Core functionality
  EEPROMSettings, EEPROMKeymap, Focus, FocusSettingsCommand, FocusEEPROMCommand,
  FirmwareVersion, LayerNames,

  // Key behavior
  Qukeys, SpaceCadet, SpaceCadetConfig,
  OneShot, OneShotConfig, EscapeOneShot, EscapeOneShotConfig,
  Macros, DynamicMacros, MouseKeys, MouseKeysConfig, TapDance,

  // Custom functionality
  MomentaryAdjustLayer,

  // LED control and effects
  LEDControl, DefaultLEDModeConfig, LEDOff, LEDPaletteTheme,
  PreonicColormapEffect, DefaultPreonicColormap, LEDBrightnessConfig,
  LEDRainbowEffect, Keyclick, LEDIndicators
);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

void configureIndicators() {
  // Map LEDs to the four encoder positions
  static const KeyAddr indicator_leds[] = {
    KeyAddr(0, 0), KeyAddr(0, 1), KeyAddr(0, 2), KeyAddr(0, 3)
  };
  LEDIndicators.setSlots(4, indicator_leds);
}

/**
 * Helper function for Bluetooth device selection macros.
 * Selects the specified device slot and switches to BLE mode.
 */
void selectBluetoothDevice(uint8_t device_slot) {
  if (keyToggledOn(kaleidoscope::Runtime.device().pressedKeyswitchCount())) {
    kaleidoscope::Runtime.device().ble().selectDevice(device_slot);
    kaleidoscope::Runtime.device().setHostConnectionMode(MODE_BLE);
  }
}

// =============================================================================
// MACRO IMPLEMENTATIONS
// =============================================================================

static void versionInfoMacro(uint8_t key_state) {
  if (keyToggledOn(key_state)) {
    Macros.type(PSTR("Keyboardio Preonic - Firmware version "));
    Macros.type(PSTR(BUILD_INFORMATION));
  }
}

static void anyKeyMacro(KeyEvent &event) {
  if (keyToggledOn(event.state)) {
    // Generate random alphanumeric key (A-Z, 0-9)
    event.key.setKeyCode(Key_A.getKeyCode() + (uint8_t)(millis() % 36));
    event.key.setFlags(0);
  }
}

static void batteryLevelMacro(uint8_t key_state) {
  if (!keyToggledOn(key_state)) return;
  
  auto& device = kaleidoscope::Runtime.device();
  auto& gauge = device.batteryGauge();
  auto& charger = device.batteryCharger();
  
  // Battery level and basic info
  uint8_t battery_level = gauge.getBatteryLevel();
  uint16_t voltage = gauge.getVoltage();
  int16_t charge_rate = gauge.getChargeRate();
  
  Macros.type(PSTR("Battery Status:\n"));
  
  // Format and output battery information
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "Level: %d%%\n", battery_level);
  Macros.type(buffer);
  
  snprintf(buffer, sizeof(buffer), "Voltage: %.2fV\n", voltage / 1000.0f);
  Macros.type(buffer);
  
  snprintf(buffer, sizeof(buffer), "Rate: %.2f%%/hr\n", charge_rate * 0.208f);
  Macros.type(buffer);
  
  // Power source and charging status
  Macros.type(PSTR("Power: "));
  if (charger.hasPower()) {
    Macros.type(PSTR("Connected - "));
    
    uint8_t charging_state = charger.getChargingState();
    typedef kaleidoscope::driver::battery_charger::BQ24075<kaleidoscope::device::keyboardio::PreonicBatteryChargerProps> BQ24075;
    
    switch (charging_state) {
      case BQ24075::CHARGING: Macros.type(PSTR("Charging")); break;
      case BQ24075::CHARGE_COMPLETE: Macros.type(PSTR("Complete")); break;
      case BQ24075::CHARGE_FAULT: Macros.type(PSTR("Fault")); break;
      case BQ24075::NO_BATTERY: Macros.type(PSTR("No Battery")); break;
      case BQ24075::BATTERY_DISCONNECTED: Macros.type(PSTR("Disconnected")); break;
      default: Macros.type(PSTR("Not Charging")); break;
    }
  } else {
    Macros.type(PSTR("Battery"));
  }
  Macros.type(PSTR("\n"));
  
  // Alert status summary
  Macros.type(PSTR("Status: "));
  if (gauge.isHibernating()) {
    Macros.type(PSTR("Hibernating"));
  } else if (gauge.hasLowBatteryAlert() || gauge.hasLowVoltageAlert()) {
    Macros.type(PSTR("Low Battery"));
  } else if (gauge.hasChangeAlert()) {
    Macros.type(PSTR("Level Changed"));
  } else {
    Macros.type(PSTR("Normal"));
  }
  Macros.type(PSTR("\n"));
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
    case MACRO_BT_SELECT_1: case MACRO_BT_SELECT_2:
    case MACRO_BT_SELECT_3: case MACRO_BT_SELECT_4:
      if (keyToggledOn(event.state)) {
        uint8_t device_slot = (macro_id - MACRO_BT_SELECT_1) + 1;
        kaleidoscope::Runtime.device().ble().selectDevice(device_slot);
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
        auto& ble = kaleidoscope::Runtime.device().ble();
        ble.stopAdvertising();
        ble.disconnect();
        kaleidoscope::Runtime.device().setHostConnectionMode(MODE_USB);
      }
      break;
    case MACRO_BATTERY_LEVEL:
      batteryLevelMacro(event.state);
      break;
  }
  return MACRO_NONE;
}

// =============================================================================
// SETUP AND MAIN LOOP
// =============================================================================

void setup() {
  // Configure Qukeys for modifier behavior on home row
  QUKEYS(
    kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 1), Key_LeftShift),   // A
    kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 10), Key_RightShift), // ;
    kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 4), Key_LeftAlt),     // F
    kaleidoscope::plugin::Qukey(QWERTY, KeyAddr(3, 7), Key_RightAlt)     // J
  );

  Kaleidoscope.setup();
  configureIndicators();

  // Configure EEPROM and storage
  EEPROMKeymap.setup(9);
  DynamicMacros.reserve_storage(512);
  LayerNames.reserve_storage(128);

  // Configure LED effects
  PreonicColormapEffect.max_layers(9);
  LEDRainbowEffect.brightness(25);
  DefaultLEDModeConfig.activateLEDModeIfUnconfigured(&LEDOff);

  // Configure plugins
  Keyclick.disable();  // Start with keyclick disabled
  SpaceCadetConfig.disableSpaceCadetIfUnconfigured();
  
  // Set default layer from EEPROM
  Layer.move(EEPROMSettings.default_layer());
}

void loop() {
  Kaleidoscope.loop();
}