/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_UI_QT_SPLIT_TAB_H_
#define XENIA_UI_QT_SPLIT_TAB_H_

#include "xenia/ui/qt/widgets/tab.h"

class QStackedWidget;

namespace xe {
namespace ui {
namespace qt {

class XSideBar;

class SidebarItem {
  QChar glyph_;
  QString title_;
  QWidget* widget_;

 public:
  SidebarItem(QChar glyph, const QString& title, QWidget* widget)
      : glyph_(glyph), title_(title), widget_(widget) {}

  SidebarItem(const SidebarItem& other) = default;
  SidebarItem& operator=(const SidebarItem& other) = default;

  QChar glyph() const { return glyph_; }
  const QString& title() const { return title_; }
  QWidget* widget() const { return widget_; }
};

class SplitTab : public XTab {
  Q_OBJECT

 public:
  explicit SplitTab(const QString& tab_name);

  explicit SplitTab(const QString& tab_name, const QString& object_name);

 protected:
  void Build();

  void AddSidebarItem(const SidebarItem& item);

 private:
  QList<SidebarItem> sidebar_items_;

  QHBoxLayout* layout_ = nullptr;
  QWidget* sidebar_container_ = nullptr;
  XSideBar* sidebar_ = nullptr;
  QStackedWidget* content_widget_ = nullptr;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_QT_SPLIT_TAB_H_