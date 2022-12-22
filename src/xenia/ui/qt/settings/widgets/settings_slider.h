/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2021 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_SETTINGS_SLIDER_H_
#define XENIA_SETTINGS_SLIDER_H_

#include "xenia/app/settings/settings.h"
#include "xenia/base/cvar.h"
#include "xenia/ui/qt/widgets/slider.h"

namespace xe {
namespace ui {
namespace qt {

using namespace xe::app::settings;
using namespace xe::cvar;

class SettingsSlider : public XSlider {
  Q_OBJECT;

 public:
  explicit SettingsSlider(SliderSettingsItem& item);

  bool Initialize();

private:
  SliderSettingsItem& item_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif