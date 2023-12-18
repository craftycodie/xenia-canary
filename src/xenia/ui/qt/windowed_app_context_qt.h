/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_WINDOWED_APP_CONTEXT_QT_H_
#define XENIA_UI_WINDOWED_APP_CONTEXT_QT_H_

#include "application.h"
#include "xenia/ui/windowed_app_context.h"

namespace xe {
namespace ui {
namespace qt {

class QtWindowedAppContext : public WindowedAppContext {
 public:
  explicit QtWindowedAppContext(int& argc, char** argv);

  ~QtWindowedAppContext() override;

  int RunMainMessageLoop();

 protected:
  void NotifyUILoopOfPendingFunctions() override;
  void PlatformQuitFromUIThread() override;

 private:
  XApplication instance_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_WINDOWED_APP_CONTEXT_WIN_H_