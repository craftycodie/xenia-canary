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

#include "xenia/app/settings/settings_builder.h"
#include "xenia/app/settings/settings_item.h"

namespace xe {
namespace app {
namespace settings {

void GeneralSet::LoadSettings() {
#define FIND_CVAR(name) auto cvar_##name = config.FindConfigVarByName(#name)
#define FIND_TYPED_CVAR(name, type) \
  auto cvar_##name = config.FindTypedConfigVarByName<type>(#name)

  namespace fs = std::filesystem;

  auto& config = Config::Instance();

  FIND_TYPED_CVAR(discord, bool);
  FIND_TYPED_CVAR(gpu, std::string);
  FIND_TYPED_CVAR(cache_root, fs::path);

  if (cvar_discord) {
    auto field = SwitchBuilder()
                     .key("test")
                     .title("Test Switch")
                     .description("A test switch")
                     .valueStore(CvarValueStore<bool>::Create(*cvar_discord))
                     .Build();

    AddSettingsItem("General Settings", std::move(field));
  }

  if (cvar_gpu) {
    auto field = MultiChoiceBuilder<std::string>()
                     .key("gpu")
                     .title("Graphics System")
                     .description("Choose the graphics impl.")
                     .valueStore(CvarValueStore<std::string>::Create(*cvar_gpu))
                     .option("any", "Any")
                     .option("d3d12", "Direct3D 12")
                     .option("vulkan", "Vulkan")
                     .option("null", "None")
                     .Build();

    AddSettingsItem("General Settings", std::move(field));
  }

  if (cvar_cache_root) {
    using FieldType = PathInputSettingsItem;
    auto field = std::make_unique<FieldType>(
        "cache_root", "Cache Directory", "",
        CvarValueStore<fs::path>::Create(*cvar_cache_root));

    AddSettingsItem("Additional Settings", std::move(field));
  }
}

void GeneralSet::OnSettingChanged(std::string_view key,
                                  const SettingsValue& value) {}

}  // namespace settings
}  // namespace app
}  // namespace xe