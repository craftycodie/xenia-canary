/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_HIDEVENT_H_
#define XENIA_UI_QT_HIDEVENT_H_

#include <QEvent>

#include "xenia/hid/input.h"

namespace xe {
namespace hid {

enum InputButtons : uint16_t {
  kInputDpadUp = X_INPUT_GAMEPAD_DPAD_UP,
  kInputDpadDown = X_INPUT_GAMEPAD_DPAD_DOWN,
  kInputDpadLeft = X_INPUT_GAMEPAD_DPAD_LEFT,
  kInputDpadRight = X_INPUT_GAMEPAD_DPAD_RIGHT,
  kInputStart = X_INPUT_GAMEPAD_START,
  kInputBack = X_INPUT_GAMEPAD_BACK,
  kInputLeftThumb = X_INPUT_GAMEPAD_LEFT_THUMB,
  kInputRightThumb = X_INPUT_GAMEPAD_RIGHT_THUMB,
  kInputLeftShoulder = X_INPUT_GAMEPAD_LEFT_SHOULDER,
  kInputRightShoulder = X_INPUT_GAMEPAD_RIGHT_SHOULDER,
  kInputGuide = X_INPUT_GAMEPAD_GUIDE,
  kInputButtonA = X_INPUT_GAMEPAD_A,
  kInputButtonB = X_INPUT_GAMEPAD_B,
  kInputButtonX = X_INPUT_GAMEPAD_X,
  kInputButtonY = X_INPUT_GAMEPAD_Y
};

}

namespace ui {
namespace qt {

using namespace xe::hid;

enum ThumbStick : uint8_t { LeftThumb, RightThumb };

enum Trigger : uint8_t { LeftTrigger, RightTrigger };

class HidEvent : public QEvent {
 public:
  static const QEvent::Type ButtonPressType;
  static const QEvent::Type ButtonReleaseType;
  static const QEvent::Type AxisMoveType;
  static const QEvent::Type TriggerActionType;

  int controller_id() const { return controller_id_; }

  const X_INPUT_STATE& input_state() const { return input_state_; }

 protected:
  HidEvent(QEvent::Type type, int controller_id, X_INPUT_STATE state);

 private:
  int controller_id_;
  X_INPUT_STATE input_state_;
};

class ButtonPressEvent : public HidEvent {
 public:
  ButtonPressEvent(int controller_id, X_INPUT_STATE state, uint16_t buttons,
                   bool is_repeat);

  /**
   * Gets the buttons that triggered this event. repeat presses are kept in a
   * separate event
   */
  uint16_t buttons() const { return buttons_; }

  /**
   * Gets whether this event is for repeat presses or new presses
   */
  bool is_repeat() const { return is_repeat_; }

 private:
  uint16_t buttons_;
  bool is_repeat_;
};

class ButtonReleaseEvent : public HidEvent {
 public:
  ButtonReleaseEvent(int controller_id, X_INPUT_STATE state, uint16_t buttons);

  uint16_t buttons() const { return buttons_; }

 private:
  uint16_t buttons_;
};

class AxisMoveEvent : public HidEvent {
 public:
  AxisMoveEvent(int controller_id, X_INPUT_STATE state, ThumbStick thumbstick,
                int16_t x, int16_t y);

  ThumbStick thumbstick() const { return thumbstick_; }
  int16_t x() const { return x_; }
  int16_t y() const { return y_; }

 private:
  ThumbStick thumbstick_;
  int16_t x_;
  int16_t y_;
};

class TriggerActionEvent : public HidEvent {
 public:
  TriggerActionEvent(int controller_id, const X_INPUT_STATE& state,
                     Trigger trigger, uint8_t value);

  Trigger trigger() const { return trigger_; }
  uint8_t value() const { return value_; }

 private:
  Trigger trigger_;
  uint8_t value_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif