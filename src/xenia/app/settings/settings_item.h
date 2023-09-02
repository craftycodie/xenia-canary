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

#include "xenia/base/assert.h"
#include "xenia/base/cvar.h"
#include "xenia/base/delegate.h"
#include "xenia/base/logging.h"

namespace xe {
namespace app {
namespace settings {
class SettingsSet;

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

enum PathInputSelectionType : uint8_t {
  File = 1 << 0,
  Folder = 1 << 1,
  Any = File | Folder
};

enum class MultiChoiceItemType : uint8_t { Combo, Radio, Default = Combo };

template <typename T>
struct ValueStore {
  virtual ~ValueStore() = default;

  virtual const T& GetDefaultValue() const = 0;
  virtual const T& GetValue() const = 0;
  virtual bool SetValue(const T& value) = 0;
  virtual bool ResetValue() = 0;
};

template <typename T>
struct RawValueStore : ValueStore<T> {
  static std::unique_ptr<RawValueStore> Create(T& location,
                                               const T& default_value) {
    return std::unique_ptr<RawValueStore>(
        new RawValueStore(location, default_value));
  }

  const T& GetDefaultValue() const override { return default_value_; }
  const T& GetValue() const override { return *value_; }
  bool SetValue(const T& value) override { return *value_ = value; }
  bool ResetValue() override { return SetValue(default_value_); }

 private:
  RawValueStore(T& location, const T& default_value)
      : default_value_(default_value), value_(&location) {}

  T default_value_;
  T* value_;
};

template <typename T>
struct CvarValueStore : ValueStore<T> {
  static std::unique_ptr<CvarValueStore> Create(cvar::ConfigVar<T>& cvar) {
    return std::unique_ptr<CvarValueStore>(new CvarValueStore(cvar));
  }

  const T& GetDefaultValue() const override { return cvar_->default_value(); }
  const T& GetValue() const override { return *cvar_->current_value(); }
  bool SetValue(const T& value) override {
    cvar_->SetConfigValue(value);
    return true;
  }
  bool ResetValue() override {
    cvar_->ResetConfigValueToDefault();
    return true;
  }

 private:
  CvarValueStore(cvar::ConfigVar<T>& cvar) : cvar_(&cvar) {}

  cvar::ConfigVar<T>* cvar_;
};

class ISettingsItem {
 public:
  ISettingsItem(SettingsType type, std::string_view key, std::string_view title,
                std::string_view description, SettingsSet& owning_set)
      : settings_type_(type),
        key_(key),
        title_(title),
        description_(description),
        owning_set_(owning_set) {}

  SettingsType settings_type() const { return settings_type_; }
  const std::string& key() const { return key_; }
  const std::string& title() const { return title_; }
  const std::string& description() const { return description_; }
  SettingsSet& owner() { return owning_set_; }

 private:
  SettingsType settings_type_;
  std::string key_;
  std::string title_;
  std::string description_;
  SettingsSet& owning_set_;
};

template <SettingsType Type, typename T>
class ValueSettingsItem : public ISettingsItem {
 public:
  using ValueType = T;

  virtual ~ValueSettingsItem() = default;

  ValueSettingsItem(std::string_view key, std::string_view title,
                    std::string_view description, SettingsSet& owning_set,
                    std::unique_ptr<ValueStore<T>> store)
      : ISettingsItem(Type, key, title, description, owning_set),
        value_store_(std::move(store)) {}

  virtual const T& value() const { return value_store_->GetValue(); }
  virtual bool set_value(const T& value) {
    return validate(value) && value_store_->SetValue(value);
  }

  virtual bool validate(const T& value) const { return true; }

 private:
  std::unique_ptr<ValueStore<T>> value_store_;
};

#define DECLARE_SETTINGS_ITEM(settings_type, value_type) \
  using settings_type##SettingsItem =                    \
      ValueSettingsItem<SettingsType::settings_type, value_type>

DECLARE_SETTINGS_ITEM(Switch, bool);

DECLARE_SETTINGS_ITEM(NumberInput, int64_t);

class TextInputSettingsItem
    : public ValueSettingsItem<SettingsType::TextInput, std::string> {
 public:
  TextInputSettingsItem(const std::string_view& key,
                        const std::string_view& title,
                        const std::string_view& description,
                        SettingsSet& owning_set,
                        std::unique_ptr<ValueStore<std::string>> store,
                        bool accept_empty_values = true)
      : ValueSettingsItem<SettingsType::TextInput, std::string>(
            key, title, description, owning_set, std::move(store)),
        accept_empty_values_(accept_empty_values) {}

  bool validate(const std::string& value) const override {
    if (value.empty() && !accept_empty_values_) {
      return false;
    }

    return true;
  }

 private:
  bool accept_empty_values_;
};

class PathInputSettingsItem
    : public ValueSettingsItem<SettingsType::PathInput, std::filesystem::path> {
 public:
  PathInputSettingsItem(
      const std::string_view& key, const std::string_view& title,
      const std::string_view& description, SettingsSet& owning_set,
      std::unique_ptr<ValueStore<std::filesystem::path>> store,
      PathInputSelectionType selection_type = Any,
      bool require_valid_path = true)
      : ValueSettingsItem<SettingsType::PathInput, std::filesystem::path>(
            key, title, description, owning_set, std::move(store)),
        selection_type_(selection_type),
        require_valid_path_(require_valid_path) {}

  bool validate(const std::filesystem::path& value) const override {
    if (require_valid_path_) {
      if (!std::filesystem::exists(value)) {
        return false;
      }
    }

    if ((selection_type_ & File && !std::filesystem::is_regular_file(value)) ||
        (selection_type_ & Folder && !std::filesystem::is_directory(value))) {
      return false;
    }

    return true;
  }

  PathInputSelectionType selection_type() const { return selection_type_; }

 private:
  PathInputSelectionType selection_type_;
  bool require_valid_path_;
};

class SliderSettingsItem
    : public ValueSettingsItem<SettingsType::Slider, int32_t> {
 public:
  SliderSettingsItem(const std::string_view& key, const std::string_view& title,
                     const std::string_view& description,
                     SettingsSet& owning_set,
                     std::unique_ptr<ValueStore<int32_t>> store, int min,
                     int max, int step)
      : ValueSettingsItem<SettingsType::Slider, int>(
            key, title, description, owning_set, std::move(store)),
        min_(min),
        max_(max),
        step_(step) {}

  int min() const { return min_; }
  int max() const { return max_; }
  int step() const { return step_; }

 private:
  int min_;
  int max_;
  int step_;
};

class IMultiChoiceSettingsItem : public ISettingsItem {
 public:
  virtual ~IMultiChoiceSettingsItem() = default;
  /**
   * @return the string titles of all available options for this multi-choice
   * item
   */
  virtual std::vector<std::string> option_names() const = 0;

  /**
   * @return the index of the current selection
   */
  virtual unsigned int current_index() const = 0;

  /**
   * Sets the currently selected item based on its item index
   * @param index the index of the item to set
   * @return whether setting this item was successful
   */
  virtual bool set_selection(unsigned int index) = 0;

  /**
   * Hints to the UI system consuming this settings item how it should prefer to
   * display the item (e.g. as a combobox or radio button)
   * @return recommended display type
   */
  virtual MultiChoiceItemType item_type() const = 0;

  /**
   * @return the std::type_info for the target value type this multi-choice item
   * supports
   */
  virtual const std::type_info& target_type() const = 0;

 protected:
  IMultiChoiceSettingsItem(std::string_view key, std::string_view title,
                           std::string_view description,
                           SettingsSet& owning_set)
      : ISettingsItem(SettingsType::MultiChoice, key, title, description,
                      owning_set) {}
};

template <typename T>
class MultiChoiceSettingsItem : public IMultiChoiceSettingsItem {
 public:
  using ValueType = T;

  struct Option {
    T value;
    std::string title;
  };

  MultiChoiceSettingsItem(
      std::string_view key, std::string_view title,
      std::string_view description, SettingsSet& owning_set,
      std::unique_ptr<ValueStore<T>> store,
      std::initializer_list<Option> options,
      MultiChoiceItemType item_type = MultiChoiceItemType::Default)
      : IMultiChoiceSettingsItem(key, title, description, owning_set),
        value_store_(std::move(store)),
        options_(options),
        item_type_(item_type) {
    Initialize();
  }

  MultiChoiceSettingsItem(
      std::string_view key, std::string_view title,
      std::string_view description, SettingsSet& owning_set,
      std::unique_ptr<ValueStore<T>> store, const std::vector<Option>& options,
      MultiChoiceItemType item_type = MultiChoiceItemType::Default)
      : IMultiChoiceSettingsItem(key, title, description, owning_set),
        value_store_(std::move(store)),
        options_(options),
        item_type_(item_type) {
    Initialize();
  }

  void Initialize() {
    const T& value = value_store_->GetValue();
    for (int i = 0; i < options_.size(); i++) {
      if (options_[i].value == value) {
        selection_ = i;
      }
    }

    if (selection_ < 0) {
      assert_always("failed to find option that matches current index");
    }
  }

  std::vector<std::string> option_names() const override {
    std::vector<std::string> option_names;
    option_names.reserve(options_.size());

    // copy option titles into local vector
    for (const auto& [k, v] : options_) {
      option_names.push_back(v);
    }

    return option_names;
  }

  unsigned int current_index() const override { return selection_; }

  const Option* current_selection() const {
    if (selection_ >= options_.size()) {
      assert_always("Selection outside of options bounds");
      return nullptr;
    }

    return &options_.at(selection_);
  }

  bool set_selection(unsigned int index) override {
    if (index >= options_.size()) {
      assert_always("Provided index outside of options bounds");
      return false;
    }

    selection_ = index;

    value_store_->SetValue(options_[index].value);
    return true;
  }

  MultiChoiceItemType item_type() const override { return item_type_; }

  const std::type_info& target_type() const override { return typeid(T); }

 private:
  void SetInitialIndex() {
    const T& default_value = value_store_->GetDefaultValue();
    // TODO:
  }

  std::unique_ptr<ValueStore<T>> value_store_;
  std::vector<Option> options_;
  MultiChoiceItemType item_type_;
  int selection_ = -1;
};

using StringMultiChoiceSettingsItem = MultiChoiceSettingsItem<std::string>;
using IntMultiChoiceSettingsItem = MultiChoiceSettingsItem<int32_t>;

struct ActionSettingsItem : ISettingsItem {
  ActionSettingsItem(std::string_view key, std::string_view title,
                     std::string_view description, SettingsSet& owning_set,
                     std::function<void()> handler)
      : ISettingsItem(SettingsType::Action, key, title, description,
                      owning_set),
        handler_(std::move(handler)) {}

  void Trigger() { handler_(); }

 private:
  std::function<void()> handler_;
};
}  // namespace settings
}  // namespace app
}  // namespace xe

#endif
