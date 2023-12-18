/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "application.h"

#include <QWidget>

namespace xe {
namespace ui {
namespace qt {

bool XApplication::notify(QObject* object, QEvent* event) {
  // Custom event types aren't propagated automatically
  // so we must do so ourselves.
  if (event->type() >= QEvent::Type::User) {
    // first run event filters on this event
    if (!qApp->eventFilter(object, event) &&
        !object->eventFilter(object, event)) {
      QWidget* w = qobject_cast<QWidget*>(object);
      event->setAccepted(false);
      // iterate up the widget hierarchy starting with 'object'
      while (w) {
        bool result = QApplication::notify(w, event);

        // if event was accepted or reached a window exit iteration
        if ((result && event->isAccepted()) || w->isWindow()) {
          return true;
        }

        w = w->parentWidget();
      }
    }
  }

  return QApplication::notify(object, event);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe