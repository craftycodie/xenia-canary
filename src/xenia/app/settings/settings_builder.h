/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#if 0 // TODO:

#ifndef XENIA_APP_SETTINGS_SETTINGS_BUILDER_H_
#define XENIA_APP_SETTINGS_SETTINGS_BUILDER_H_

#include "settings_item.h"

namespace xe {
namespace app {
namespace settings {

// Switch, TextInput, PathInput, NumberInput, Slider, MultiChoice, Action,
// Custom,
//   UNKNOWN = 0xFF

// class SwitchSettingBuilder;
// class TextInputSettingBuilder;
// class PathInputSettingBuilder;
// class NumberInputSettingBuilder;

// Abstract builder uses CRTP to allow for inheritance

template <typename T>
class AbstractBuilder;

// Builder templates

template <class TBuilder>
class AbstractBuilder {
  static_assert(std::is_base_of_v<AbstractBuilder<TBuilder>, TBuilder>,
                "Provided builder type not a builder");

 public:
  TBuilder& title(std::string_view title) {
    return *this;
  }
  TBuilder& description(std::string_view description);
  TBuilder& group(std::string_view group);

protected:

};

template <typename TValueType, class TBuilder>
struct BasicSettingBuilder
    : public AbstractBuilder<BasicSettingBuilder<TValueType, TBuilder>> {
  TBuilder& default_value(TValueType default_value);
  TBuilder& value(TValueType value);
};

template <typename TValueType, class TBuilder>
struct NumberInputSettingBuilder
    : public BasicSettingBuilder<TValueType, TBuilder> {
 public:
  TBuilder& min(TValueType min);
  TBuilder& max(TValueType max);
  TBuilder& step(TValueType step);
};

// Builder impls

struct SwitchSettingBuilder final
    : public BasicSettingBuilder<bool, SwitchSettingBuilder> {
};

struct TextInputSettingBuilder final
    : public BasicSettingBuilder<std::string, TextInputSettingBuilder> {
};

struct PathInputSettingBuilder final
    : public BasicSettingBuilder<std::filesystem::path,
                                 PathInputSettingBuilder> {
 public:
  PathInputSettingBuilder& require_valid_path(bool value);
};

struct IntegerInputSettingBuilder final
    : public NumberInputSettingBuilder<int32_t, IntegerInputSettingBuilder> {
 public:
};

struct DoubleInputSettingBuilder final
    : public NumberInputSettingBuilder<double, DoubleInputSettingBuilder> {
 public:
};

struct IntegerRangeSettingBuilder final
    : public NumberInputSettingBuilder<int32_t, IntegerRangeSettingBuilder> {
 public:
};

struct DoubleRangeSettingBuilder final
    : public NumberInputSettingBuilder<int32_t, DoubleRangeSettingBuilder> {
 public:
};

template <typename TValueType>
struct MultiChoiceSettingBuilder final
    : public AbstractBuilder<MultiChoiceSettingBuilder<TValueType>> {
 public:
  MultiChoiceSettingBuilder& add_item(std::string_view title, TValueType value,
                                     bool default_value = false);
};

}  // namespace settings
}  // namespace app
}  // namespace xe

#endif

#endif 