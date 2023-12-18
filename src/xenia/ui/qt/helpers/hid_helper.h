/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_HIDHELPER_H_
#define XENIA_UI_QT_HIDHELPER_H_

#include <QObject>

#include <map>

#include "xenia/hid/input_system.h"
#include "xenia/ui/qt/window_qt.h"

class QTimer;

namespace xe {
namespace ui {
namespace qt {

class HidEvent;

class HidHelper : public QObject {
  Q_OBJECT

 public:
  explicit HidHelper(QtWindow* parent);

  bool Initialize();

  /**
   * @return whether this helper emits key events simulated from gamepad button
   * presses
   */
  bool simulates_keyboard_events() const { return simulates_keyboard_; }

  /**
   * Set whether this helper simulates key events from gamepad button presses
   */
  void set_simulates_keyboard_events(bool simulate_keyboard) {
    simulates_keyboard_ = simulate_keyboard;
  }

  /**
   * Converts a given HidEvent to an appropriate Qt key event if possible.
   * Returns nullptr if conversion wasn't possible (e.g. the HidEvent didn't
   * contain any button presses)
   * @param event the HidEvent to convert
   * @return a simulated QKeyEvent that was created from any button presses in
   * the HidEvent, or null if none were present
   */
  QKeyEvent* CreateKeyEventFromHidEvent(const HidEvent& event);

 protected:
  void PollInputState();

 private:
  QtWindow* window_;
  std::unique_ptr<hid::InputSystem> input_system_;
  QTimer* input_process_timer_;
  std::map<int, hid::X_INPUT_STATE> last_states_;  // controller_id:state
  bool simulates_keyboard_;
  std::map<uint16_t, Qt::Key> button_key_mapping_;  // xinput_btn_flags:Qt::Key
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_QT_HIDHELPER_H_