/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_set.h"
#include <algorithm>
#include "settings_item.h"

namespace xe {
namespace app {
namespace settings {

// Settings Groups

void SettingsGroup::AddSettingsItem(SettingsItemPtr item) {
  items_.emplace(item->key(), std::move(item));
}

std::vector<ISettingsItem*> SettingsGroup::items() const {
  std::vector<ISettingsItem*> items;
  items.reserve(items_.size());

  // copy items from map to new vector
  for (const auto& [k, v] : items_) {
    items.push_back(v.get());
  }

  return items;
}

// Settings Sets

void SettingsSet::AddSettingsItem(const std::string& group_name,
                                  SettingsItemPtr item) {
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
  std::vector<SettingsGroup*> groups;
  groups.reserve(groups_.size());
  // copy items from map to new vector
  for (const auto& [k, v] : groups_) {
    groups.push_back(v.get());
  }

  return groups;
}

IConfigVar* SettingsSet::FindConfigVar(const std::string& cvar_name) const {
    auto& config = Config::Instance();
    return config.FindConfigVarByName(cvar_name);
}

}  // namespace settings
}  // namespace app
}  // namespace xe
