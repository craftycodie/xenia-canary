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

#if XE_PLATFORM_WIN32
#include <Windows.h>
#endif

namespace xe {
namespace app {
namespace settings {
void GeneralSet::LoadSettings() {
  namespace fs = std::filesystem;

  if (auto cvar_discord = FindTypedConfigVar<bool>("discord"); cvar_discord) {
    auto field = SwitchBuilder()
                     .key("test")
                     .title("Test Switch")
                     .description("A test switch")
                     .owner(this)
                     .valueStore(CvarValueStore<bool>::Create(*cvar_discord))
                     .Build();

    AddSettingsItem("General Settings", std::move(field));
  }

  if (auto cvar_gpu = FindTypedConfigVar<std::string>("gpu"); cvar_gpu) {
    auto field = MultiChoiceBuilder<std::string>()
                     .key("gpu")
                     .title("Graphics System")
                     .description("Choose the graphics impl.")
                     .owner(this)
                     .type(MultiChoiceItemType::Combo)
                     .valueStore(CvarValueStore<std::string>::Create(*cvar_gpu))
                     .option("Any", "any")
                     .option("Direct3D 12", "d3d12")
                     .option("Vulkan", "vulkan")
                     .option("None", "null")
                     .Build();

    AddSettingsItem("General Settings", std::move(field));
  }

  if (auto cvar_cache_root = FindTypedConfigVar<fs::path>("cache_root");
      cvar_cache_root) {
    auto field =
        PathInputBuilder()
            .title("Cache Directory")
            .description("-")
            .key("cache_root")
            .owner(this)
            .valueStore(CvarValueStore<fs::path>::Create(*cvar_cache_root))
            .selectionType(Folder)
            .requireValidPath(true)
            .Build();

    AddSettingsItem("Additional Settings", std::move(field));
  }

  auto test_callback = []() {
#if XE_PLATFORM_WIN32
    MessageBox(nullptr, L"Test Alert", L"A Test Alert", 0);
#endif
  };

  auto field = ActionBuilder()
                   .title("test")
                   .description("")
                   .handler(test_callback)
                   .Build();

  AddSettingsItem("Additional Settings", std::move(field));
}

void GeneralSet::OnSettingChanged(std::string_view key,
                                  const SettingsValue& value) {}
}  // namespace settings
}  // namespace app
}  // namespace xe
