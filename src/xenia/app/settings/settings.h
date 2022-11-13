/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_H_
#define XENIA_APP_SETTINGS_SETTINGS_H_

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "settings_item.h"
#include "settings_set.h"
#include "xenia/base/cvar.h"
#include "xenia/base/delegate.h"
#include "xenia/base/logging.h"

namespace xe {
namespace app {
namespace settings {

using cvar::ConfigVar;
using cvar::IConfigVar;

using SettingsSetPtr = std::unique_ptr<SettingsSet>;

class Settings {
 public:
  static Settings& Instance();

  void LoadSettingsItems();

  SettingsSet* FindSettingsSet(const std::string& title);

 private:
  void Init();

  std::unordered_map<std::string, SettingsSetPtr> settings_;
};

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif
