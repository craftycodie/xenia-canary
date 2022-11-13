/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_slider.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

SettingsSlider::SettingsSlider(NumberRangeSettingsItem& item)
    : XSlider(), item_(item) {
  assert_true(Initialize(), "Could not initialize SettingsSlider");
}

bool SettingsSlider::Initialize() {
  this->setMinimum(item_.min());
  this->setMaximum(item_.max());

  if (int current_value; item_.GetValue().as<int>(current_value)) {
    setValue(current_value);
  } else {
    XELOGE("Could not load current value for settings item \"{}\"",
           item_.title());
    return false;
  }

  XSlider::connect(this, &XSlider::valueChanged, [&](int value) {
    // TODO: handle SetValue returning false for value out of range
    item_.SetValue(value);
  });

  return true;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe