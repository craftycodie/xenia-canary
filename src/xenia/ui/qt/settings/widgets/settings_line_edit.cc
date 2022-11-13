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
    : XLineEdit(), item_(item), type_(Type::Text) {
  assert_true(Initialize(), "Could not initialize SettingsLineEdit");
}

SettingsLineEdit::SettingsLineEdit(PathInputSettingsItem& item)
    : XLineEdit(), item_(item), type_(Type::Path) {
  assert_true(Initialize(), "Could not initialize SettingsLineEdit");
}

bool SettingsLineEdit::Initialize() {
  namespace fs = std::filesystem;

  this->setPlaceholderText(item_.description().c_str());
  this->setMaximumWidth(kLineEditMaxWidth);

  if (type_ == Type::Path) {
    auto item = static_cast<PathInputSettingsItem*>(&item_);
    if (!item) return false;

    fs::path current_path;
    if (item->GetValue().as<fs::path>(current_path)) {
      std::string current_path_str = std::string(current_path.u8string());
      this->setText(QString(current_path_str.c_str()));

      XLineEdit::connect(this, &XLineEdit::textChanged,
                         [=](const QString& text) {
                           auto path = std::string(text.toUtf8());
                           item->SetValue(path);
                         });

      return true;
    }
  } else if (type_ == Type::Text) {
    auto item = dynamic_cast<TextInputSettingsItem*>(&item_);
    if (!item) return false;

    std::string current_text_str;
    if (item->GetValue().as<std::string>(current_text_str)) {
      this->setText(QString(current_text_str.c_str()));

      XLineEdit::connect(this, &XLineEdit::textChanged,
                         [=](const QString& text) {
                           item->SetValue(std::string(text.toUtf8()));
                         });

      return true;
    }
  }

  return false;
}

}  // namespace qt
}  // namespace ui
}  // namespace xe