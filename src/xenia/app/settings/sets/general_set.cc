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
#include "xenia/config.h"

#include "xenia/app/settings/settings_item.h"

namespace xe {
namespace app {
namespace settings {

void GeneralSet::LoadSettings() {
#define FIND_CVAR(name) auto cvar_##name = config.FindConfigVarByName(##name)
#define FIND_TYPED_CVAR(name, type) \
  auto cvar_##name = config.FindTypedConfigVarByName<type>(#name)

  auto& config = Config::Instance();

  FIND_TYPED_CVAR(discord, bool);
  FIND_TYPED_CVAR(gpu, std::string);

  if (cvar_discord) {
    auto test_switch = std::make_unique<SwitchSettingsItem>(
        "test", "Test Switch", "A test switch",
        CvarValueStore<bool>::Create(*cvar_discord));

    AddSettingsItem("General Settings", std::move(test_switch));
  }

  if (cvar_gpu) {
    using FieldType = MultiChoiceSettingsItem<std::string>;

    std::vector<FieldType::Option> options = {{"any", "Any"},
                                              {"d3d12", "Direct3D 12"},
                                              {"vulkan", "Vulkan"},
                                              {"null", "Null"}};

    auto field = std::make_unique<FieldType>(
        "gpu", "Graphics System", "Choose the graphics impl.",
        CvarValueStore<std::string>::Create(*cvar_gpu), options);

    AddSettingsItem("General Settings", std::move(field));
  }
}

void GeneralSet::OnSettingChanged(std::string_view key,
                                  const SettingsValue& value) {}

}  // namespace settings
}  // namespace app
}  // namespace xe