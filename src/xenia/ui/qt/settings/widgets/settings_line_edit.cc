/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_line_edit.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

const int kLineEditMaxWidth = 480;

SettingsLineEdit::SettingsLineEdit(TextInputSettingsItem& item)
    : XLineEdit(), item_(item) {
  bool success = Initialize();
  assert_true(success, "Could not initialize SettingsLineEdit");
}

SettingsLineEdit::SettingsLineEdit(PathInputSettingsItem& item)
    : XLineEdit(), item_(item) {
  bool success = Initialize();
  assert_true(success, "Could not initialize SettingsLineEdit");
}

bool SettingsLineEdit::Initialize() {
  namespace fs = std::filesystem;

  this->setPlaceholderText(item_.description().c_str());
  this->setMaximumWidth(kLineEditMaxWidth);

  // setup the current value, and textChanged callback
  // based on if we are implementing a path-input text box or not
  if (item_.settings_type() == SettingsType::PathInput) {
    auto item = static_cast<PathInputSettingsItem*>(&item_);
    if (!item) return false;

    fs::path current_path = item->value();

    std::string current_path_str = std::string(current_path.u8string());
    this->setText(QString(current_path_str.c_str()));

    connect(this, &XLineEdit::textChanged, [item](const QString& text) {
      auto path = std::string(text.toUtf8());
      item->set_value(path);
    });

    return true;

  } else if (item_.settings_type() == SettingsType::TextInput) {
    auto item = static_cast<TextInputSettingsItem*>(&item_);
    if (!item) return false;

    std::string current_text_str = item->value();

    this->setText(QString(current_text_str.c_str()));

    connect(this, &XLineEdit::textChanged, [item](const QString& text) {
      item->set_value(std::string(text.toUtf8()));
    });

    return true;
  }

  return false;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe