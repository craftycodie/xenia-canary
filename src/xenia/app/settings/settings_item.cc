/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "settings_item.h"

namespace xe {
namespace app {
namespace settings {

SwitchSettingsItem::SwitchSettingsItem(std::string_view key,
                                       std::string_view title,
                                       std::string_view description, bool value,
                                       bool default_value)
    : ISettingsItem(SettingsType::Switch, key, title, description,
                    default_value, value) {}

bool SwitchSettingsItem::SetValue(const SettingsValue& value) {
  return value.is<bool>() && ISettingsItem::SetValue(value);
}

TextInputSettingsItem::TextInputSettingsItem(std::string_view key,
                                             std::string_view title,
                                             std::string_view description,
                                             std::string value,
                                             std::string default_value)
    : ISettingsItem(SettingsType::TextInput, key, title, description,
                    default_value, value) {}

bool TextInputSettingsItem::SetValue(const SettingsValue& value) {
  return value.is<std::string>() && ISettingsItem::SetValue(value);
}

PathInputSettingsItem::PathInputSettingsItem(
    std::string_view key, std::string_view title, std::string_view description,
    std::filesystem::path value, std::filesystem::path default_value,
    bool require_valid_path)
    : TextInputSettingsItem(key, title, description, default_value.string(),
                            value.string()),
      require_valid_path_(require_valid_path) {}

bool PathInputSettingsItem::SetValue(const SettingsValue& value) {
  namespace fs = std::filesystem;

  fs::path path;
  // if SettingsValue isn't already a path, check if it's a string too
  if (!value.as<fs::path>(path)) {
    std::string path_str;
    if (!value.as<std::string>(path_str)) {
      XELOGE("Provided SettingsValue not convertible to path");
    }

    path = path_str;
  }

  if (require_valid_path_) {
    auto status = fs::status(path);
    auto file_type = status.type();

    if (file_type == fs::file_type::none ||
        file_type == fs::file_type::unknown) {
      XELOGW("Provided file path {} is invalid", path.string());
      return false;
    }
  }

  return TextInputSettingsItem::SetValue(path);
}

NumberInputSettingsItem::NumberInputSettingsItem(
    SettingsType settings_type, std::string_view key, std::string_view title,
    std::string_view description, int value, int min, int max,
    int default_value)
    : ISettingsItem(settings_type, key, title, description,
                    default_value, value),
      min_(min),
      max_(max) {}

bool NumberInputSettingsItem::SetValue(const SettingsValue& setting) {
  return ISettingsItem::SetValue(setting);
}

NumberRangeSettingsItem::NumberRangeSettingsItem(std::string_view key, std::string_view title,
    std::string_view description, int value, int min, int max,
    int default_value, int step)
    : NumberInputSettingsItem(SettingsType::Slider, key, title, description,
                              value, min, max, default_value),
      step_(step) {}

MultiChoiceSettingsItem::MultiChoiceSettingsItem(
    std::string_view key, std::string_view title, std::string_view description,
    std::initializer_list<Option> options)
    : MultiChoiceSettingsItem(key, title, description, std::vector(options)) {}

MultiChoiceSettingsItem::MultiChoiceSettingsItem(
    std::string_view key, std::string_view title, std::string_view description,
    const std::vector<Option>& options)
    : ISettingsItem(SettingsType::MultiChoice, key, title, description,
                    options[0].value, options[0].value) {}

std::vector<std::string> MultiChoiceSettingsItem::GetOptionNames() const {
  std::vector<std::string> option_names(options_.size());

  // copy option titles into local vector
  std::transform(options_.begin(), options_.end(),
                 std::back_inserter(option_names),
                 [](const Option& opt) { return opt.title; });

  return option_names;
}

unsigned int MultiChoiceSettingsItem::GetCurrentIndex() const {
  return selection_;
}

const MultiChoiceSettingsItem::Option*
MultiChoiceSettingsItem::GetSelectedItem() const {
  if (options_.size() < selection_) return nullptr;

  return &options_.at(selection_);
}

bool MultiChoiceSettingsItem::SetSelectedItem(unsigned int index) {
  if (index >= options_.size()) {
    XELOGW("Provided index outside bounds of item list");
    return false;
  }

  selection_ = index;
  return ISettingsItem::SetValue(options_[index].value);
}

bool MultiChoiceSettingsItem::SetValue(const SettingsValue& value) {
  assert_always(
      "SetValue not valid for multi-choice items. "
      "use SetSelectedItem instead");

  return false;
}

ActionSettingsItem::ActionSettingsItem(std::string_view key,
                                       std::string_view title,
                                       std::string_view description)
    : ISettingsItem(SettingsType::Action, key, title, description,
                    SettingsValue(), SettingsValue()) {}

}  // namespace settings
}  // namespace app
}  // namespace xe
