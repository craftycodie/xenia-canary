/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_ITEM_H_
#define XENIA_APP_SETTINGS_SETTINGS_ITEM_H_

#include <filesystem>
#include <functional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "settings_set.h"
#include "xenia/base/assert.h"
#include "xenia/base/delegate.h"
#include "xenia/base/logging.h"

namespace xe {
namespace app {
namespace settings {

enum class SettingsType : uint8_t {
  Switch,
  TextInput,
  PathInput,
  NumberInput,
  Slider,
  MultiChoice,
  Action,
  Custom,
  UNKNOWN = 0xFF
};

class SettingsValue {
  using ValueStore =
      std::variant<std::monostate, int32_t, int64_t, uint32_t, uint64_t, bool,
                   double, std::string, std::filesystem::path>;

 public:
  SettingsValue(bool value) : value_(value) {}
  SettingsValue(int32_t value) : value_(value) {}
  SettingsValue(int64_t value) : value_(value) {}
  SettingsValue(uint32_t value) : value_(value) {}
  SettingsValue(uint64_t value) : value_(value) {}
  SettingsValue(double value) : value_(value) {}
  SettingsValue(const std::string& value) : value_(value) {}
  SettingsValue(const std::filesystem::path& value) : value_(value) {}
  SettingsValue() : value_(std::monostate{}) {}

  template <typename T>
  bool is() const {
    return std::holds_alternative<T>(value_);
  }

  template <typename T>
  bool as(T& out) const {
    const T* ptr = std::get_if<T>(&value_);
    if (ptr) out = *ptr;
    return !!ptr;
  }

 private:
  ValueStore value_;
};

struct ISettingsItem {
  virtual ~ISettingsItem() = default;

  SettingsType settings_type() const { return settings_type_; }
  const std::string& key() const { return key_; }
  const std::string& title() const { return title_; }
  const std::string& description() const { return description_; }

  virtual SettingsValue GetValue() const { return value_; }

 protected:
  ISettingsItem(SettingsType settings_type,
                std::string_view key, std::string_view title,
                std::string_view description,
                const SettingsValue& default_value,
                const SettingsValue& value)
      : settings_type_(settings_type),
        key_(key),
        title_(title),
        description_(description),
        value_(value),
        default_value_(default_value) {}

  virtual bool SetValue(const SettingsValue& value) {
    value_ = value;
    return true;
  }

  virtual bool ResetValue() { return SetValue(default_value_); }

 private:
  SettingsType settings_type_;
  std::string key_;
  std::string title_;
  std::string description_;
  SettingsValue value_;
  const SettingsValue default_value_;
};

struct SwitchSettingsItem : ISettingsItem {
  SwitchSettingsItem(std::string_view key, std::string_view title,
                     std::string_view description, bool value,
                     bool default_value = false);

  bool SetValue(const SettingsValue& value) override;
};

struct TextInputSettingsItem : ISettingsItem {
  TextInputSettingsItem(std::string_view key, std::string_view title,
                        std::string_view description, std::string value,
                        std::string default_value = "");

  bool SetValue(const SettingsValue& value) override;
};

struct PathInputSettingsItem : TextInputSettingsItem {
  PathInputSettingsItem(std::string_view key, std::string_view title,
                        std::string_view description,
                        std::filesystem::path value,
                        std::filesystem::path default_value = "",
                        bool require_valid_path = false);

  bool SetValue(const SettingsValue& value) override;
 private:
  bool require_valid_path_;
};

struct NumberInputSettingsItem : ISettingsItem {
  NumberInputSettingsItem(SettingsType settings_type,
                          std::string_view key, std::string_view title,
                          std::string_view description, int value,
                          int min, int max,
                          int default_value);

  int min() const { return min_; }
  int max() const { return max_; }

  bool SetValue(const SettingsValue& setting) override;

 private:
  const int min_;
  const int max_;
};

struct NumberRangeSettingsItem : NumberInputSettingsItem {
  NumberRangeSettingsItem(std::string_view key, std::string_view title,
                          std::string_view description, int value,
                          int min,
                          int max, int default_value, int step = 1);

  int step() const { return step_; }

 private:
  int step_;
};

struct MultiChoiceSettingsItem : ISettingsItem {
  struct Option {
    SettingsValue value;
    std::string title;
  };

  MultiChoiceSettingsItem(std::string_view key,
                          std::string_view title, std::string_view description,
                          std::initializer_list<Option> options);
  MultiChoiceSettingsItem(std::string_view key,
                          std::string_view title, std::string_view description,
                          const std::vector<Option>& options);

  /**
   * @return the string titles of all available options for this multi-choice item
   */
  virtual std::vector<std::string> GetOptionNames() const;

  /**
   * @return the index of the current selection
   */
  virtual unsigned int GetCurrentIndex() const;

  /**
   * @return the current selected item, or null
   */
  virtual const Option* GetSelectedItem() const;

  /**
   * Sets the currently selected item based on its item index
   * @param index the index of the item to set
   * @return whether setting this item was successful
   */
  virtual bool SetSelectedItem(unsigned int index);

protected:
  // don't use SetValue for multi choice settings
  bool SetValue(const SettingsValue& value) override;

private:
  std::vector<Option> options_;
  unsigned int selection_ = 0;
};

struct ActionSettingsItem : ISettingsItem {
  ActionSettingsItem(std::string_view key, std::string_view title,
                     std::string_view description);
};

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif
