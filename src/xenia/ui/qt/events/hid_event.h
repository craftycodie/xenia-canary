/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */
#ifndef XENIA_UI_QT_HIDEVENT_H_
#define XENIA_UI_QT_HIDEVENT_H_

#include <QEvent>

namespace xe {
namespace ui {
namespace qt {

class HidEvent : public QEvent {
public:
  static const QEvent::Type Type;

};

}
}  // namespace ui
}  // namespace xe

#endif
