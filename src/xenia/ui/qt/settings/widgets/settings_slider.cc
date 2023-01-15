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

SettingsSlider::SettingsSlider(SliderSettingsItem& item)
    : XSlider(), item_(item) {
  bool success = Initialize();
  assert_true(success, "Could not initialize SettingsSlider");
}

bool SettingsSlider::Initialize() {
  this->setMinimum(item_.min());
  this->setMaximum(item_.max());

  setValue(item_.value());

  connect(this, &XSlider::valueChanged,
          [&](int value) { item_.set_value(value); });

  return true;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe