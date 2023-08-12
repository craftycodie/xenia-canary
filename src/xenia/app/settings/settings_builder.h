/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_SETTINGS_SETTINGS_BUILDER_H_
#define XENIA_APP_SETTINGS_SETTINGS_BUILDER_H_

#include "settings_item.h"

namespace xe {
namespace app {
namespace settings {
template <typename TDerived>
struct SettingsBuilder {
  TDerived& key(std::string_view key) {
    key_ = key;
    return static_cast<TDerived&>(*this);
  }

  TDerived& title(std::string_view title) {
    title_ = title;
    return static_cast<TDerived&>(*this);
  }

  TDerived& description(std::string_view desc) {
    description_ = desc;
    return static_cast<TDerived&>(*this);
  }

  TDerived& owner(SettingsSet* owner) {
    owner_ = owner;
    return static_cast<TDerived&>(*this);
  }

  std::unique_ptr<ISettingsItem> Build() {
    return static_cast<TDerived*>(this)->BuildImpl();
  }

  bool IsValid() const {
    return !(key_.empty() || title_.empty() || description_.empty() || !owner_);
  }

 protected:
  std::string_view key_;
  std::string_view title_;
  std::string_view description_;
  class SettingsSet* owner_ = nullptr;
};

template <typename TDerived, typename TTarget>
struct ValueStoreSettingsBuilder : SettingsBuilder<TDerived> {
  using Super = SettingsBuilder<TDerived>;
  using ValueType = typename TTarget::ValueType;

  TDerived& valueStore(std::unique_ptr<ValueStore<ValueType>> store) {
    value_store_ = std::move(store);
    return static_cast<TDerived&>(*this);
  }

  bool IsValid() const { return value_store_ != nullptr && Super::IsValid(); }

 protected:
  std::unique_ptr<ValueStore<ValueType>> value_store_;
};

template <typename TDerived, typename TTarget>
struct ValueSettingsBuilder : ValueStoreSettingsBuilder<TDerived, TTarget> {
  std::unique_ptr<ISettingsItem> BuildImpl() {
    bool valid = this->IsValid();
    assert_true(valid, "Unset fields or invalid state for this builder");
    return valid ? std::make_unique<TTarget>(this->key_, this->title_,
                                             this->description_, *this->owner_,
                                             std::move(this->value_store_))
                 : nullptr;
  }
};

struct SliderBuilder
    : ValueStoreSettingsBuilder<SliderBuilder, SliderSettingsItem> {
  using Super = ValueStoreSettingsBuilder<SliderBuilder, SliderSettingsItem>;

  SliderBuilder& min(int min) {
    min_ = min;
    return *this;
  }

  SliderBuilder& max(int max) {
    max_ = max;
    return *this;
  }

  SliderBuilder& step(int step) {
    step_ = step;
    return *this;
  }

  std::unique_ptr<ISettingsItem> BuildImpl() {
    bool valid = IsValid();
    assert_true(valid, "Unset fields or invalid state for this builder");
    return valid ? std::make_unique<SliderSettingsItem>(
                       key_, title_, description_, *owner_,
                       std::move(value_store_), min_, max_, step_)
                 : nullptr;
  }

  bool IsValid() const { return step_ != 0 && Super::IsValid(); }

 protected:
  int min_ = 0;
  int max_ = 0;
  int step_ = 0;
};

template <typename T>
struct MultiChoiceBuilder
    : ValueStoreSettingsBuilder<MultiChoiceBuilder<T>,
                                MultiChoiceSettingsItem<T>> {
  using Super = ValueStoreSettingsBuilder<MultiChoiceBuilder<T>,
                                          MultiChoiceSettingsItem<T>>;
  using OptionType = typename MultiChoiceSettingsItem<T>::Option;

  MultiChoiceBuilder& option(std::string_view title, const T& value) {
    options_.push_back(OptionType{value, std::string(title)});
    return *this;
  }

  std::unique_ptr<ISettingsItem> BuildImpl() {
    bool valid = IsValid();
    assert_true(valid, "Unset fields or invalid state for this builder");
    return valid ? std::make_unique<MultiChoiceSettingsItem<T>>(
                       this->key_, this->title_, this->description_,
                       *this->owner_, std::move(this->value_store_), options_)
                 : nullptr;
  }

  bool IsValid() const { return !options_.empty() && Super::IsValid(); }

 protected:
  std::vector<OptionType> options_;
};

struct SwitchBuilder : ValueSettingsBuilder<SwitchBuilder, SwitchSettingsItem> {
  SwitchBuilder() = default;
};

struct TextInputBuilder
    : ValueStoreSettingsBuilder<TextInputBuilder, TextInputSettingsItem> {
  std::unique_ptr<ISettingsItem> BuildImpl() {
    bool valid = IsValid();
    assert_true(valid, "Unset fields or invalid state for this builder");
    return valid ? std::make_unique<TextInputSettingsItem>(
                       key_, title_, description_, *owner_,
                       std::move(value_store_), accept_empty_strings_)
                 : nullptr;
  }

  TextInputBuilder& acceptEmptyStrings(bool value) {
    accept_empty_strings_ = value;
    return *this;
  }

 protected:
  bool accept_empty_strings_ = true;
};

struct PathInputBuilder
    : ValueStoreSettingsBuilder<PathInputBuilder, PathInputSettingsItem> {
  std::unique_ptr<ISettingsItem> BuildImpl() {
    bool valid = IsValid();
    assert_true(valid, "Unset fields or invalid state for this builder");
    return valid ? std::make_unique<PathInputSettingsItem>(
                       key_, title_, description_, *owner_,
                       std::move(value_store_), require_valid_path_)
                 : nullptr;
  }

  PathInputBuilder& requireValidPath(bool value) {
    require_valid_path_ = value;
    return *this;
  }

 protected:
  bool require_valid_path_ = true;
};

struct ActionBuilder : SettingsBuilder<ActionBuilder> {
  ActionBuilder& handler(std::function<void()> func) {
    func_ = std::move(func);
    return *this;
  }

  std::unique_ptr<ISettingsItem> BuildImpl() {
    return std::make_unique<ActionSettingsItem>(
        this->key_, this->title_, this->description_, *this->owner_,
        std::move(func_));
  }

 protected:
  std::function<void()> func_;
};
}  // namespace settings
}  // namespace app
}  // namespace xe

#endif
