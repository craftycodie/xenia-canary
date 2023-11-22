/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */
#ifndef XENIA_UI_QT_THEME_H_
#define XENIA_UI_QT_THEME_H_

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>
#include "theme_configuration.h"

namespace xe {
namespace ui {
namespace qt {

enum ThemeStatus { THEME_LOAD_OK = 0, THEME_NOT_FOUND, THEME_MISCONFIGURED };

// Stylemap is in the format: {target, stylesheet}
// where target is component and stylesheet is qss for component
using StyleMap = QMap<QString, QString>;

/**
 * Represents a theme for xenia.
 *
 * On the FS a theme consists of a folder containing a
 * theme.json config file, and an optional sub-folder
 * called stylesheets containing QSS files.
 *
 * These QSS stylesheets support macros unlike default Qt
 * and have the format `$macro`.
 *
 * Macro values are defined in the config file.
 */
class Theme : public QObject {
  Q_OBJECT

 public:
  Theme(QObject* parent = nullptr) : QObject(parent) {}
  Theme(const QString& directory, QObject* parent = nullptr)
      : QObject(parent), directory_(directory) {}
  Theme(const ThemeConfiguration& config, QObject* parent = nullptr)
      : QObject(parent), config_(config) {}

  /**
   * Loads the current theme from the directory provided in the ctor
   * @return whether the theme was loaded correctly or had errors
   */
  ThemeStatus LoadTheme();

  /**
   * Reloads the current theme.
   * This is different to LoadTheme() as it also wipes the stylesheet cache
   */
  ThemeStatus ReloadTheme();

  QString StylesheetForComponent(const QString& component) const;

  // Check the return value is valid with color.isValid()
  QColor ColorForKey(const QString& key, QColor fallback = QColor()) const;

  const QString& directory() const { return directory_; }
  const ThemeConfiguration& config() const { return config_; }

 signals:
  void ThemeReloaded();

 private:
  QString PreprocessStylesheet(QString filename) const;

  QString directory_;
  ThemeConfiguration config_;
  mutable StyleMap styles_; // marked mutable as acts as a cache
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif  // XENIA_UI_QT_THEME_H_
