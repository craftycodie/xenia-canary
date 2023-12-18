/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "hid_event.h"

namespace xe {
namespace ui {
namespace qt {

// register custom types
const QEvent::Type HidEvent::ButtonPressType =
    static_cast<QEvent::Type>(registerEventType());
const QEvent::Type HidEvent::ButtonReleaseType =
    static_cast<QEvent::Type>(registerEventType());
const QEvent::Type HidEvent::AxisMoveType =
    static_cast<QEvent::Type>(registerEventType());
const QEvent::Type HidEvent::TriggerActionType =
    static_cast<QEvent::Type>(registerEventType());

HidEvent::HidEvent(QEvent::Type type, int controller_id, X_INPUT_STATE state)
    : QEvent(type), controller_id_(controller_id), input_state_(state) {
  assert_true(type == ButtonPressType || type == ButtonReleaseType ||
              type == AxisMoveType || type == TriggerActionType);
  QEvent::setAccepted(false);
}

ButtonPressEvent::ButtonPressEvent(int controller_id, X_INPUT_STATE state,
                                   uint16_t buttons, bool is_repeat)
    : HidEvent(HidEvent::ButtonPressType, controller_id, state),
      buttons_(buttons),
      is_repeat_(is_repeat) {}

ButtonReleaseEvent::ButtonReleaseEvent(int controller_id, X_INPUT_STATE state,
                                       uint16_t buttons)
    : HidEvent(HidEvent::ButtonReleaseType, controller_id, state),
      buttons_(buttons) {}

AxisMoveEvent::AxisMoveEvent(int controller_id, X_INPUT_STATE state,
                             ThumbStick thumbstick, int16_t x, int16_t y)
    : HidEvent(HidEvent::AxisMoveType, controller_id, state),
      thumbstick_(thumbstick),
      x_(x),
      y_(y) {}

TriggerActionEvent::TriggerActionEvent(int controller_id,
                                       const X_INPUT_STATE& state,
                                       Trigger trigger, uint8_t value)
    : HidEvent(HidEvent::TriggerActionType, controller_id, state),
      trigger_(trigger),
      value_(value) {}

}  // namespace qt
}  // namespace ui
}  // namespace xe