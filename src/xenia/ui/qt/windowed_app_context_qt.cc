/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "windowed_app_context_qt.h"
#include <QApplication>
#include <QThread>
#include <QTimer>

namespace xe {
namespace ui {
namespace qt {

QtWindowedAppContext::QtWindowedAppContext() = default;
QtWindowedAppContext::~QtWindowedAppContext() = default;

void QtWindowedAppContext::NotifyUILoopOfPendingFunctions() {
  // this code utilizes a Qt timer that is queued and fired from the UI thread
  // then on timeout calls our pending functions
  QThread* main_thread = qApp->thread();
  QTimer* timer = new QTimer();
  timer->moveToThread(main_thread);
  timer->setSingleShot(true);

  // connect the timer's `timeout` method to a callback that executes pending
  // functions
  QObject::connect(timer, &QTimer::timeout, [=]() {
    // we are now in the gui thread, execute pending functions
    ExecutePendingFunctionsFromUIThread();
    timer->deleteLater();
  });

  // invoke `QTimer::start(uint64_t delay)` on the UI thread
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection,
                            Q_ARG(uint64_t, 0));
}

int QtWindowedAppContext::RunMainMessageLoop(QApplication& app) {
  return app.exec();
}

void QtWindowedAppContext::PlatformQuitFromUIThread() {
  // fire the QApplication::quit method on the UI thread
  QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe
