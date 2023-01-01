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
#include "xenia/base/cvar.h"
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

template <typename T>
struct ValueStore {
  virtual ~ValueStore() = default;

  virtual const T& GetDefaultValue() const = 0;
  virtual const T& GetValue() const = 0;
  virtual void SetValue(const T& value) = 0;
  virtual void ResetValue() = 0;
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
  void SetValue(const T& value) override { *value_ = value; }
  void ResetValue() override { SetValue(default_value_); }

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
  void SetValue(const T& value) override { cvar_->SetConfigValue(value); }
  void ResetValue() override { cvar_->ResetConfigValueToDefault(); }

 private:
  CvarValueStore(cvar::ConfigVar<T>& cvar) : cvar_(&cvar) {}
  cvar::ConfigVar<T>* cvar_;
};

class ISettingsItem {
 public:
  ISettingsItem(SettingsType type, std::string_view key, std::string_view title,
                std::string_view description)
      : settings_type_(type),
        key_(key),
        title_(title),
        description_(description) {}

  virtual ~ISettingsItem() = default;

  SettingsType settings_type() const { return settings_type_; }
  const std::string& key() const { return key_; }
  const std::string& title() const { return title_; }
  const std::string& description() const { return description_; }

 private:
  SettingsType settings_type_;
  std::string key_;
  std::string title_;
  std::string description_;
};

template <SettingsType Type, typename T>
class ValueSettingsItem : public ISettingsItem {
 public:
  using ValueType = T;

  virtual ~ValueSettingsItem() = default;

  ValueSettingsItem(std::string_view key, std::string_view title,
                    std::string_view description,
                    std::unique_ptr<ValueStore<T>> store)
      : ISettingsItem(Type, key, title, description),
        value_store_(std::move(store)) {}

  const T& value() const { return value_store_->GetValue(); }
  void set_value(const T& value) { value_store_->SetValue(value); }

 private:
  std::unique_ptr<ValueStore<T>> value_store_;
};

#define DECLARE_SETTINGS_ITEM(settings_type, value_type) \
  using settings_type##SettingsItem =                    \
      ValueSettingsItem<SettingsType::settings_type, value_type>

DECLARE_SETTINGS_ITEM(Switch, bool);

DECLARE_SETTINGS_ITEM(TextInput, std::string);

DECLARE_SETTINGS_ITEM(PathInput, std::filesystem::path);

DECLARE_SETTINGS_ITEM(NumberInput, int64_t);

class SliderSettingsItem
    : public ValueSettingsItem<SettingsType::Slider, int32_t> {
 public:
  SliderSettingsItem(const std::string_view& key, const std::string_view& title,
                     const std::string_view& description,
                     std::unique_ptr<ValueStore<int32_t>> store, int min,
                     int max, int step)
      : ValueSettingsItem<SettingsType::Slider, int>(key, title, description,
                                                     std::move(store)),
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
   * @return the std::type_info for the target value type this multi-choice item
   * supports
   */
  virtual const std::type_info& target_type() const = 0;

 protected:
  IMultiChoiceSettingsItem(std::string_view key, std::string_view title,
                           std::string_view description)
      : ISettingsItem(SettingsType::MultiChoice, key, title, description) {}
};

template <typename T>
class MultiChoiceSettingsItem : public IMultiChoiceSettingsItem {
 public:
  using ValueType = T;

  struct Option {
    T value;
    std::string title;
  };

  MultiChoiceSettingsItem(std::string_view key, std::string_view title,
                          std::string_view description,
                          std::unique_ptr<ValueStore<T>> store,
                          std::initializer_list<Option> options)
      : IMultiChoiceSettingsItem(key, title, description),
        value_store_(std::move(store)),
        options_(options) {}

  MultiChoiceSettingsItem(std::string_view key, std::string_view title,
                          std::string_view description,
                          std::unique_ptr<ValueStore<T>> store,
                          const std::vector<Option>& options)
      : IMultiChoiceSettingsItem(key, title, description),
        value_store_(std::move(store)),
        options_(options) {}

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

  const std::type_info& target_type() const override {
    return typeid(T);
  }

 private:
  void SetInitialIndex() {
    const T& default_value = value_store_->GetDefaultValue();
  }

  std::unique_ptr<ValueStore<T>> value_store_;
  std::vector<Option> options_;
  unsigned int selection_ = 0;
};

using StringMultiChoiceSettingsItem = MultiChoiceSettingsItem<std::string>;
using IntMultiChoiceSettingsItem = MultiChoiceSettingsItem<int32_t>;

struct ActionSettingsItem : ISettingsItem {
  ActionSettingsItem(std::string_view key, std::string_view title,
                     std::string_view description)
      : ISettingsItem(SettingsType::Action, key, title, description) {}
};

// class SwitchSettingsItem : ValueSettingsItem<SettingsType::Switch, bool> {
//   SwitchSettingsItem(std::string_view key, std::string_view title,
//                      std::string_view description, bool& value,
//                      const bool& default_value = false)
//     : ValueSettingsItem<SettingsType::Switch, bool>(
//         key, title, description, value, default_value) {}
// };

//
// class SettingsValue {
//  using ValueStore =
//      std::variant<std::monostate, int32_t, int64_t, uint32_t, uint64_t, bool,
//                   double, std::string, std::filesystem::path>;
//
// public:
//  SettingsValue(bool value) : value_(value) {}
//  SettingsValue(int32_t value) : value_(value) {}
//  SettingsValue(int64_t value) : value_(value) {}
//  SettingsValue(uint32_t value) : value_(value) {}
//  SettingsValue(uint64_t value) : value_(value) {}
//  SettingsValue(double value) : value_(value) {}
//  SettingsValue(const std::string& value) : value_(value) {}
//  SettingsValue(const std::filesystem::path& value) : value_(value) {}
//  SettingsValue() : value_(std::monostate{}) {}
//
//  template <typename T>
//  bool is() const {
//    return std::holds_alternative<T>(value_);
//  }
//
//  template <typename T>
//  bool as(T& out) const {
//    const T* ptr = std::get_if<T>(&value_);
//    if (ptr) out = *ptr;
//    return !!ptr;
//  }
//
// private:
//  ValueStore value_;
//};
//
// struct ISettingsItem {
//  virtual ~ISettingsItem() = default;
//
//  SettingsType settings_type() const { return settings_type_; }
//  const std::string& key() const { return key_; }
//  const std::string& title() const { return title_; }
//  const std::string& description() const { return description_; }
//
//  virtual SettingsValue GetValue() const { return value_; }
//
// protected:
//
//
//  virtual bool SetValue(const SettingsValue& value) {
//    value_ = value;
//    return true;
//  }
//
//  virtual bool ResetValue() { return SetValue(default_value_); }
//
// private:
//  SettingsType settings_type_;
//  std::string key_;
//  std::string title_;
//  std::string description_;
//  SettingsValue value_;
//  const SettingsValue default_value_;
//};
//
// struct SwitchSettingsItem : ISettingsItem {
//  SwitchSettingsItem(std::string_view key, std::string_view title,
//                     std::string_view description, bool value,
//                     bool default_value = false);
//
//  bool SetValue(const SettingsValue& value) override;
//};
//
// struct TextInputSettingsItem : ISettingsItem {
//  TextInputSettingsItem(std::string_view key, std::string_view title,
//                        std::string_view description, std::string value,
//                        std::string default_value = "");
//
//  bool SetValue(const SettingsValue& value) override;
//};
//
// struct PathInputSettingsItem : TextInputSettingsItem {
//  PathInputSettingsItem(std::string_view key, std::string_view title,
//                        std::string_view description,
//                        std::filesystem::path value,
//                        std::filesystem::path default_value = "",
//                        bool require_valid_path = false);
//
//  bool SetValue(const SettingsValue& value) override;
// private:
//  bool require_valid_path_;
//};
//
// struct NumberInputSettingsItem : ISettingsItem {
//  NumberInputSettingsItem(SettingsType settings_type,
//                          std::string_view key, std::string_view title,
//                          std::string_view description, int value,
//                          int min, int max,
//                          int default_value);
//
//  int min() const { return min_; }
//  int max() const { return max_; }
//
//  bool SetValue(const SettingsValue& setting) override;
//
// private:
//  const int min_;
//  const int max_;
//};
//
// struct NumberRangeSettingsItem : NumberInputSettingsItem {
//  NumberRangeSettingsItem(std::string_view key, std::string_view title,
//                          std::string_view description, int value,
//                          int min,
//                          int max, int default_value, int step = 1);
//
//  int step() const { return step_; }
//
// private:
//  int step_;
//};
//
// struct MultiChoiceSettingsItem : ISettingsItem {
//  struct Option {
//    SettingsValue value;
//    std::string title;
//  };
//
//  MultiChoiceSettingsItem(std::string_view key,
//                          std::string_view title, std::string_view
//                          description, std::initializer_list<Option> options);
//  MultiChoiceSettingsItem(std::string_view key,
//                          std::string_view title, std::string_view
//                          description, const std::vector<Option>& options);
//
//  /**
//   * @return the string titles of all available options for this multi-choice
//   item
//   */
//  virtual std::vector<std::string> option_names() const;
//
//  /**
//   * @return the index of the current selection
//   */
//  virtual unsigned int current_index() const;
//
//  /**
//   * @return the current selected item, or null
//   */
//  virtual const Option* current_selection() const;
//
//  /**
//   * Sets the currently selected item based on its item index
//   * @param index the index of the item to set
//   * @return whether setting this item was successful
//   */
//  virtual bool set_selection(unsigned int index);
//
// protected:
//  // don't use SetValue for multi choice settings
//  bool SetValue(const SettingsValue& value) override;
//
// private:
//  std::vector<Option> options_;
//  unsigned int selection_ = 0;
//};
//
// struct ActionSettingsItem : ISettingsItem {
//  ActionSettingsItem(std::string_view key, std::string_view title,
//                     std::string_view description);
//};

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif