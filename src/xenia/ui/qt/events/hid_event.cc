/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "hid_event.h"

namespace xe {
namespace ui {
namespace qt {

const QEvent::Type HidEvent::Type =
    static_cast<QEvent::Type>(registerEventType());

}
}  // namespace ui
}  // namespace xe