/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_SET_H_
#define XENIA_APP_SETTINGS_SETTINGS_SET_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "settings_item.h"
#include "xenia/base/cvar.h"

namespace xe {
namespace app {
namespace settings {
class SettingsValue;

using SettingsItemPtr = std::unique_ptr<class ISettingsItem>;
using SettingsGroupPtr = std::unique_ptr<class SettingsGroup>;

using namespace xe::cvar;

class SettingsGroup {
 public:
  explicit SettingsGroup(std::string_view title) : title_(title) {}

  const std::string& title() const { return title_; }
  std::vector<ISettingsItem*> items() const;

  void AddSettingsItem(SettingsItemPtr item);

 private:
  std::string title_;
  std::unordered_map<std::string, SettingsItemPtr> items_;
};

class SettingsSet {
 public:
  explicit SettingsSet(std::string_view title) : title_(title), groups_() {}

  virtual ~SettingsSet() = default;

  std::string title() const { return title_; }

  virtual void LoadSettings() = 0;

  virtual void OnSettingChanged(std::string_view key,
                                const SettingsValue& value) = 0;

  /**
   * Returns a vector containing a reference to every settings item in this set,
   * traversing all settings groups owned by this set
   *
   * @return a list of all settings items owned by this set
   */
  std::vector<ISettingsItem*> GetSettingsItems() const;

  /**
   * @return a vector of all settings groups owned by this set
   */
  std::vector<SettingsGroup*> GetSettingsGroups() const;

 protected:
  void AddSettingsItem(const std::string& group_name, SettingsItemPtr item);

  IConfigVar* FindConfigVar(const std::string& cvar_name) const;
  template <typename T>
  ConfigVar<T>* FindTypedConfigVar(const std::string& cvar_name) const;

  template <typename T>
  std::unique_ptr<CvarValueStore<T>> CreateCvarStore(
      ConfigVar<T>& target) const;

 private:
  std::string title_;
  std::unordered_map<std::string, SettingsGroupPtr> groups_;
};

template <typename T>
ConfigVar<T>* SettingsSet::FindTypedConfigVar(const std::string& name) const {
  auto base_cvar = FindConfigVar(name);
  if (base_cvar) {
    return dynamic_cast<ConfigVar<T>*>(base_cvar);
  }
  return nullptr;
}

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif