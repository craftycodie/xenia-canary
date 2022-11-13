/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_set.h"
#include "settings_item.h"
#include <algorithm>

namespace xe {
namespace app {
namespace settings {

// Settings Groups

void SettingsGroup::AddSettingsItem(SettingsItemPtr&& item) {
  items_.emplace(item->key(), std::move(item));
}

std::vector<ISettingsItem*> SettingsGroup::items() const {
  std::vector<ISettingsItem*> items(items_.size());

  // copy items from map to new vector
  std::transform(items_.begin(), items_.end(), std::back_inserter(items),
                 [](auto& kv) { return kv.second.get(); });

  return items;
}

// Settings Sets

void SettingsSet::AddSettingsItem(const std::string& group_name,
                                  SettingsItemPtr&& item) {
  if (groups_.find(group_name) == groups_.end()) {
    groups_[group_name] = std::make_unique<SettingsGroup>(group_name);
  }

  groups_[group_name]->AddSettingsItem(std::move(item));
}

std::vector<ISettingsItem*> SettingsSet::GetSettingsItems() const {
  std::vector<ISettingsItem*> items;

  for (const auto& [key, group] : groups_) {
    std::vector<ISettingsItem*> group_items = group->items();
    items.insert(items.end(), group_items.begin(), group_items.end());
  }

  return items;
}
std::vector<SettingsGroup*> SettingsSet::GetSettingsGroups() const {
  std::vector<SettingsGroup*> groups(groups_.size());

  // copy items from map to new vector
  std::transform(groups_.begin(), groups_.end(), std::back_inserter(groups),
                 [](const auto& kv) { return kv.second.get(); });

  return groups;
}

}  // namespace settings
}  // namespace app
}  // namespace xe
