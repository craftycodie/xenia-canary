/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "general_set.h"
#include "xenia/app/settings/settings_builder.h"

#include "xenia/app/settings/settings_item.h"

namespace xe {
namespace app {
namespace settings {

void GeneralSet::LoadSettings() {
    auto test_switch = std::make_unique<SwitchSettingsItem>(
        "test", "Test Switch", "A test switch", false, false);

  AddSettingsItem("General Settings", std::move(test_switch));
}

void GeneralSet::OnSettingChanged(std::string_view key,
                                  const SettingsValue& value) {
}

}  // namespace settings
}  // namespace app
}  // namespace xe
