/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_GENERAL_SET_H_
#define XENIA_APP_SETTINGS_GENERAL_SET_H_

#include "xenia/app/settings/settings_set.h"

namespace xe {
namespace app {
namespace settings {

class GeneralSet final : public SettingsSet {
public:
  explicit GeneralSet(std::string_view title)
    : SettingsSet(title) {}
  ~GeneralSet() override = default;

  void LoadSettings() override;
  void OnSettingChanged(std::string_view key,
      const SettingsValue& value) override;
};

}
}  // namespace app
}  // namespace xe

#endif