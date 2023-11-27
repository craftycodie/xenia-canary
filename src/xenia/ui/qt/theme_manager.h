/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */
#ifndef XENIA_UI_QT_THEMEMANAGER_H_
#define XENIA_UI_QT_THEMEMANAGER_H_

#include <QFileSystemWatcher>
#include <QScopedPointer>
#include <QString>
#include <QVector>

#include "xenia/ui/qt/theme.h"

namespace xe {
namespace ui {
namespace qt {

class ThemeManager : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ThemeManager)

 public:
  static ThemeManager& Instance();

  ~ThemeManager() = default;

  Theme* current_theme() const;
  QVector<Theme*> themes() const;
  const QString& base_style() const;

  void LoadThemes();
  const Theme* LoadTheme(const QString& path);

  void SetActiveTheme(Theme* theme);

  void EnableHotReload();
  void DisableHotReload();

 private slots:
  void OnThemeDirectoryChanged(const QString& path);

 private:
  ThemeManager(QObject* parent = nullptr);

  QVector<QSharedPointer<Theme>> themes_;
  Theme* active_theme_;
  QScopedPointer<QFileSystemWatcher> watcher_;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_QT_THEMEMANAGER_H_