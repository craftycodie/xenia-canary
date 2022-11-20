/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings.h"
#include "xenia/base/cvar.h"

#include "sets/general_set.h"

// TODO: new design for settings
// Qt settings widgets

namespace xe {
namespace cvars {}  // namespace cvars

namespace app {
namespace settings {

Settings& Settings::Instance() {
  static Settings* settings = nullptr;
  if (!settings) {
    settings = new Settings();
    settings->Init();
    settings->LoadSettingsItems();
  }
  return *settings;
}

void Settings::LoadSettingsItems() {
    settings_["General"]->LoadSettings(); // TODO: move away from map
}

void Settings::Init() {
  settings_["General"] = std::make_unique<GeneralSet>("General");
}

SettingsSet* Settings::FindSettingsSet(const std::string& title) {
  auto it = settings_.find(title);
  if (it == settings_.end()) {
    return nullptr;
  }

  return it->second.get();
}

}  // namespace settings
}  // namespace app
}  // namespace xe
