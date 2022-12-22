/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_combobox.h"

namespace xe {
namespace ui {
namespace qt {
  
using namespace xe::app::settings;
using namespace xe::cvar;

const int kComboboxMaxWidth = 120;

SettingsComboBox::SettingsComboBox(MultiChoiceSettingsItem& item)
    : XComboBox(), item_(item) {
  assert_true(Initialize(), "Could not initialize SettingsComboBox");
}


bool SettingsComboBox::Initialize() {
  setMaximumWidth(kComboboxMaxWidth);

  // load combobox items from setting
  for (const std::string& option : item_.GetOptionNames()) {
    addItem(option.c_str());
  }

  // set default value to match cvar
  int index = static_cast<int>(item_.GetCurrentIndex());
  setCurrentIndex(index);

  // update selected multi-choice item when index changes
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [&item = item_](int index) {
                       item.SetSelectedItem(index);
                     });

  return true;
}

//void SettingsComboBox::OnValueUpdated(const ICommandVar& var) {
//  if (!is_value_updating_) {
//    int index = item_.GetCurrentIndex();
//    // emit state change on ui thread
//    QMetaObject::invokeMethod(this, "setCurrentIndex", Qt::QueuedConnection,
//                              Q_ARG(int, index));
//  }
//}

}  // namespace qt
}  // namespace ui
}  // namespace xe