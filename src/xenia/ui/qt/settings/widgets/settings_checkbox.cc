/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_checkbox.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

SettingsCheckBox::SettingsCheckBox(SwitchSettingsItem& item)
    : XCheckBox(item.title().c_str()), item_(item) {
  bool success = Initialize();
  assert_true(success, "Could not initialize SettingsCheckBox");
}

SettingsCheckBox::~SettingsCheckBox() {}

bool SettingsCheckBox::Initialize() {
  setCheckState(item_.value() ? Qt::Checked : Qt::Unchecked);

  // update settings item when checkbox state changes
  connect(this, &XCheckBox::stateChanged, [&](int state) {
    if (state == Qt::Checked) {
      item_.set_value(true);
    } else if (state == Qt::Unchecked) {
      item_.set_value(false);
    } else {
      assert_always(
          "PartiallyChecked state not supported for SettingsCheckBox");
    }
  });

  return true;
}

// void SettingsCheckBox::OnValueUpdated(const ICommandVar& var) {
//   const auto& new_value = item_.cvar()->current_value();
//   if (!this->is_value_updating_) {
//     // emit state change on ui thread
//     QMetaObject::invokeMethod(this, "setChecked", Qt::QueuedConnection,
//                               Q_ARG(bool, *new_value));
//   }
// }

}  // namespace qt
}  // namespace ui
}  // namespace xe