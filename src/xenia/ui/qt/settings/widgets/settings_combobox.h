/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_COMBOBOX_H_
#define XENIA_SETTINGS_COMBOBOX_H_

#include "xenia/app/settings/settings.h"
#include "xenia/base/cvar.h"
#include "xenia/ui/qt/widgets/combobox.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

class SettingsComboBox : public XComboBox {
  Q_OBJECT

 public:
  explicit SettingsComboBox(MultiChoiceSettingsItem& item);

  bool Initialize();

 private:
  MultiChoiceSettingsItem& item_;
  std::atomic_bool is_value_updating_ = false;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif