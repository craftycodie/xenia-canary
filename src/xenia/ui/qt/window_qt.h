/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_WINDOW_QT_H_
#define XENIA_UI_QT_WINDOW_QT_H_

#include <QMainWindow>
#include "xenia/ui/window.h"

namespace xe {
namespace ui {
namespace qt {

using SurfacePtr = std::unique_ptr<Surface>;

class QtWindow : public QMainWindow, public Window {
 public:
  QtWindow(WindowedAppContext& app_context, std::string_view title,
           uint32_t width, uint32_t height);

  ~QtWindow() override;

 protected:
  // -- xe::ui::Window overrides --
  uint32_t GetLatestDpiImpl() const override;
  bool OpenImpl() override;
  void RequestCloseImpl() override;
  void ApplyNewFullscreen() override;
  void ApplyNewTitle() override;
  void LoadAndApplyIcon(const void* buffer, size_t size,
                        bool can_apply_state_in_current_phase) override;
  void ApplyNewMainMenu(MenuItem* old_main_menu) override;
  void ApplyNewCursorVisibility(
      CursorVisibility old_cursor_visibility) override;
  void FocusImpl() override;
  virtual SurfacePtr CreateSurfaceImpl(
      Surface::TypeFlags allowed_types) override;
  void RequestPaintImpl() override;
  // -- end xe::ui::Window overrides

  // -- Qt overrides --
  bool event(QEvent* event) override;
  void changeEvent(QEvent*) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  // -- end Qt overrides --
 private:
  void TryLoadMenuItems(MenuItem* root_item);

  bool main_menu_enabled_ = false;
};

class QtMenuItem : public MenuItem {
 public:
  QtMenuItem(Type type, const std::string& text, const std::string& hotkey,
             std::function<void()> callback);
  ~QtMenuItem() override;

  void SetEnabled(bool enabled) override;

  void OnChildAdded(MenuItem* child_item) override;
  void OnChildRemoved(MenuItem* child_item) override;

  QMenu* handle() const { return qt_menu_; }

 private:
  QMenu* qt_menu_;
};


}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif XENIA_UI_QT_WINDOW_QT_H_
