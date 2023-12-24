/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_APPLICATION_H_
#define XENIA_UI_QT_APPLICATION_H_

#include <QApplication>

namespace xe {
namespace ui {
namespace qt {

class XApplication : public QApplication {
  Q_OBJECT

 public:
  XApplication(int& argc, char** argv);

  bool notify(QObject*, QEvent*) override;

protected:
  virtual void OnFocusChanged(QWidget* old_widget, QWidget* new_widget);
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_QT_APPLICATION_H_