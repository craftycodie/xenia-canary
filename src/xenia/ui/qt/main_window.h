#ifndef XENIA_UI_QT_MAINWINDOW_H_
#define XENIA_UI_QT_MAINWINDOW_H_

#include <QMainWindow>
#include <QTimer>

#include "window_qt.h"
#include "xenia/hid/input_system.h"
#include "xenia/ui/qt/themeable_widget.h"
#include "xenia/ui/qt/widgets/shell.h"

namespace xe {
namespace ui {
namespace qt {

class XStatusBar;

class MainWindow final : public Themeable<QtWindow> {
  Q_OBJECT
 public:
  MainWindow(WindowedAppContext& app_context, std::string_view title,
             uint32_t width, uint32_t height);

  bool OpenImpl() override;

  void AddStatusBarWidget(QWidget* widget, bool permanent = false);
  void RemoveStatusBarWidget(QWidget* widget);

  const XStatusBar* status_bar() const { return status_bar_; }

 protected slots:
  void OnActiveThemeChanged(Theme* theme);
  void OnThemeReloaded();

 protected:
  void CreateInputSystem();
  void PollInputStatus();

 private:
  XShell* shell_ = nullptr;
  XStatusBar* status_bar_ = nullptr;
  QMetaObject::Connection hot_reload_signal_;
  std::unique_ptr<hid::InputSystem> input_system_;
  QTimer* input_process_timer_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif