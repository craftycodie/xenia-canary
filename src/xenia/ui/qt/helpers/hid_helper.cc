/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "hid_helper.h"
#include "xenia/base/logging.h"
#include "xenia/hid/sdl/sdl_hid.h"
#include "xenia/ui/qt/events/hid_event.h"

#if XE_PLATFORM_WIN32
#include "xenia/hid/xinput/xinput_hid.h"
#endif

#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QWindow>

DECLARE_string(hid);

namespace xe {
namespace ui {
namespace qt {

static constexpr int kMaxUsers = 8;

static constexpr X_INPUT_GAMEPAD_BUTTON kInputButtons[] = {
    X_INPUT_GAMEPAD_DPAD_UP,
    X_INPUT_GAMEPAD_DPAD_DOWN,
    X_INPUT_GAMEPAD_DPAD_LEFT,
    X_INPUT_GAMEPAD_DPAD_RIGHT,
    X_INPUT_GAMEPAD_START,
    X_INPUT_GAMEPAD_BACK,
    X_INPUT_GAMEPAD_LEFT_THUMB,
    X_INPUT_GAMEPAD_RIGHT_THUMB,
    X_INPUT_GAMEPAD_LEFT_SHOULDER,
    X_INPUT_GAMEPAD_RIGHT_SHOULDER,
    X_INPUT_GAMEPAD_GUIDE,
    X_INPUT_GAMEPAD_A,
    X_INPUT_GAMEPAD_B,
    X_INPUT_GAMEPAD_X,
    X_INPUT_GAMEPAD_Y};

HidHelper::HidHelper(QtWindow* parent)
    : QObject(parent),
      window_(parent),
      input_process_timer_(nullptr),
      last_states_(),
      simulates_keyboard_(false) {
  button_key_mapping_[kInputButtonA] = Qt::Key_Return;
  button_key_mapping_[kInputButtonB] = Qt::Key_Back;
  button_key_mapping_[kInputButtonX] = Qt::Key_Back;
  button_key_mapping_[kInputButtonY] = Qt::Key_Back;
  button_key_mapping_[kInputDpadDown] = Qt::Key_Down;
  button_key_mapping_[kInputDpadUp] = Qt::Key_Up;
  button_key_mapping_[kInputDpadLeft] = Qt::Key_Left;
  button_key_mapping_[kInputDpadRight] = Qt::Key_Right;
  button_key_mapping_[kInputRightShoulder] = Qt::Key_Return;
  button_key_mapping_[kInputLeftShoulder] = Qt::Key_Back;
  button_key_mapping_[kInputRightThumb] = Qt::Key_Return;
  button_key_mapping_[kInputLeftThumb] = Qt::Key_Back;
  button_key_mapping_[kInputStart] = Qt::Key_Return;
  button_key_mapping_[kInputBack] = Qt::Key_Back;
  button_key_mapping_[kInputGuide] = Qt::Key_Return;
}

bool HidHelper::Initialize() {
  if (!window_) {
    XELOGW("HidHelper has invalid window reference");
    return false;
  }

  input_system_ = std::make_unique<hid::InputSystem>(window_);
  size_t z_order = 0;  // z-order unused for controller inputs in the UI

  // TODO: in future this may need logic to determine if a game is running
  auto driver_active_cb = [this]() -> bool { return true; };

  std::string real_hid = cvars::hid;

  // convert "any" hid device to appropriate string based on current platform
  if (cvars::hid == "any") {
#if XE_PLATFORM_WIN32
    real_hid = "xinput";
#else
    real_hid = "sdl";
#endif
  }

  std::vector<std::unique_ptr<hid::InputDriver>> drivers;

  // We are only interested in SDL or XInput here as Qt can handle kb+mouse
  // events itself
  if (real_hid == "sdl") {
    auto driver = xe::hid::sdl::Create(window_, z_order);
    drivers.emplace_back(std::move(driver));
  } else if (real_hid == "xinput") {
#if XE_PLATFORM_WIN32
    auto driver = xe::hid::xinput::Create(window_, z_order);
    drivers.emplace_back(std::move(driver));
#else
    assert_always("XInput selected for unsupported OS");
#endif
  }

  for (auto& driver : drivers) {
    if (XSUCCEEDED(driver->Setup())) {
      driver->set_is_active_callback(driver_active_cb);
      input_system_->AddDriver(std::move(driver));
    } else {
      return false;
    }
  }

  // set up input poll timer running every 16ms as Qt's event loop has no
  // tick() function
  {
    using namespace std::chrono_literals;

    input_process_timer_ = new QTimer(this);
    input_process_timer_->setInterval(16ms);

    connect(input_process_timer_, &QTimer::timeout, this,
            &HidHelper::PollInputState);

    input_process_timer_->start();
  }

  return true;
}

void HidHelper::PollInputState() {
  using namespace xe::hid;

  std::vector<HidEvent*> events_to_dispatch;

  for (int i = 0; i < kMaxUsers; i++) {
    X_INPUT_STATE new_state{};
    if (input_system_->GetState(i, &new_state) != X_ERROR_SUCCESS) {
      break;
    }

    const auto& new_gamepad = new_state.gamepad;
    const auto& old_gamepad = last_states_[i].gamepad;

    // process button inputs

    uint16_t pressed_buttons = 0x0;
    uint16_t released_buttons = 0x0;
    uint16_t held_buttons = 0x0;

    for (const auto& button : kInputButtons) {
      // button has been pressed
      if (new_gamepad.buttons & button && !(old_gamepad.buttons & button)) {
        pressed_buttons |= button;
      }
      // button has been released
      else if (!(new_gamepad.buttons & button) &&
               old_gamepad.buttons & button) {
        released_buttons |= button;
      }
      // button has been held down for multiple ticks
      else if (new_gamepad.buttons & button && old_gamepad.buttons & button) {
        held_buttons |= button;
      }
    }

    if (pressed_buttons != 0x0) {
      events_to_dispatch.push_back(
          new ButtonPressEvent(i, new_state, pressed_buttons, false));
    }

    if (held_buttons != 0x0) {
      events_to_dispatch.push_back(
          new ButtonPressEvent(i, new_state, held_buttons, true));
    }

    if (released_buttons != 0x0) {
      events_to_dispatch.push_back(
          new ButtonReleaseEvent(i, new_state, pressed_buttons));
    }

    // process triggers

    uint8_t left_trigger_pressure = new_gamepad.left_trigger;
    uint8_t right_trigger_pressure = new_gamepad.right_trigger;

    if (old_gamepad.right_trigger != right_trigger_pressure) {
      events_to_dispatch.push_back(new TriggerActionEvent(
          i, new_state, RightTrigger, right_trigger_pressure));
    }
    if (old_gamepad.left_trigger != left_trigger_pressure) {
      events_to_dispatch.push_back(new TriggerActionEvent(
          i, new_state, LeftTrigger, left_trigger_pressure));
    }

    // process thumbsticks

    int16_t thumb_rx = new_gamepad.thumb_rx;
    int16_t thumb_ry = new_gamepad.thumb_ry;
    int16_t thumb_lx = new_gamepad.thumb_lx;
    int16_t thumb_ly = new_gamepad.thumb_ly;

    if (old_gamepad.thumb_lx != thumb_lx || old_gamepad.thumb_ly != thumb_ly) {
      events_to_dispatch.push_back(
          new AxisMoveEvent(i, new_state, LeftThumb, thumb_lx, thumb_ly));
    }

    if (old_gamepad.thumb_rx != thumb_rx || old_gamepad.thumb_ry != thumb_ry) {
      events_to_dispatch.push_back(
          new AxisMoveEvent(i, new_state, RightThumb, thumb_rx, thumb_ry));
    }

    last_states_[i] = new_state;
  }

  for (const auto& event : events_to_dispatch) {
    QWindow* focused_window = QGuiApplication::focusWindow();
    QObject* focused_object =
        focused_window ? focused_window->focusObject() : nullptr;

    if (simulates_keyboard_events()) {
      QKeyEvent* key_event = CreateKeyEventFromHidEvent(*event);
      // send key event to focused window as Qt will automatically propagate key
      // events correctly
      if (focused_window && key_event) {
        QGuiApplication::postEvent(focused_window, key_event);
      }
    }

    // send HID event to focused child widget if possible, or HidHelper's parent
    // if not
    QObject* target_object = focused_object ? focused_object : parent();
    QApplication::postEvent(target_object, event);
  }
}

QKeyEvent* HidHelper::CreateKeyEventFromHidEvent(const HidEvent& event) {
  QKeyEvent* key_event = nullptr;

  QEvent::Type type = event.type();
  if (type == HidEvent::ButtonPressType) {
    const auto& button_press_event =
        static_cast<const ButtonPressEvent&>(event);

    auto pressed_gamepad_buttons = button_press_event.buttons();
    int simulated_keys = 0x0;
    for (const auto& button : kInputButtons) {
      if (pressed_gamepad_buttons & button) {
        simulated_keys |= button_key_mapping_[button];
      }
    }

    if (simulated_keys != 0) {
      key_event = new QKeyEvent(
          QEvent::KeyPress, button_key_mapping_[kInputDpadDown], Qt::NoModifier,
          QString(), button_press_event.is_repeat());
    }
  } else if (type == HidEvent::ButtonReleaseType) {
    const auto& button_release_event =
        static_cast<const ButtonPressEvent&>(event);

    auto pressed_gamepad_buttons = button_release_event.buttons();
    int simulated_keys = 0x0;
    for (const auto& button : kInputButtons) {
      if (pressed_gamepad_buttons & button) {
        simulated_keys |= button_key_mapping_[button];
      }
    }

    if (simulated_keys != 0) {
      key_event =
          new QKeyEvent(QEvent::KeyRelease, button_key_mapping_[kInputDpadDown],
                        Qt::NoModifier);
    }
  }

  return key_event;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe