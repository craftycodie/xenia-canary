/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_WIDGET_FACTORY_H_
#define XENIA_SETTINGS_WIDGET_FACTORY_H_

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include "xenia/app/settings/settings.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;

class SettingsWidgetFactory {
 public:
  explicit SettingsWidgetFactory(Settings& settings = Settings::Instance())
      : settings_(settings) {}

  QWidget* BuildSettingsWidget(const std::string& set_name);
  QWidget* CreateWidgetForSettingsItem(ISettingsItem& item);

 private:
  QWidget* CreateCheckBoxWidget(SwitchSettingsItem& item);
  QWidget* CreateTextInputWidget(TextInputSettingsItem& item);
  QWidget* CreatePathInputWidget(PathInputSettingsItem& item);
  QWidget* CreateNumberInputWidget(NumberInputSettingsItem& item);
  QWidget* CreateRangeInputWidget(NumberRangeSettingsItem& item);
  QWidget* CreateMultiChoiceWidget(MultiChoiceSettingsItem& item);
  QWidget* CreateActionWidget(ActionSettingsItem& item);

  QWidget* CreateWidgetContainer(QWidget* target);

  Settings& settings_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif
