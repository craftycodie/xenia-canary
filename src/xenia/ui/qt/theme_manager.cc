/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2018 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

#include "theme_manager.h"

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/vfs/devices/host_path_device.h"

DEFINE_string(
    theme_path, "",
    "Choose a path to a Xenia theme to override the default theme with",
    "Interface");

namespace xe {
namespace ui {
namespace qt {

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {}

ThemeManager& ThemeManager::Instance() {
  static ThemeManager manager;
  return manager;
}
Theme* ThemeManager::current_theme() {
  if (!themes_.empty()) {
    return themes_.front().get();
  }

  return nullptr;
}
QVector<Theme*> ThemeManager::themes() const {
  QVector<Theme*> out_vec;

  for (const auto& theme : themes_) {
    out_vec.append(theme.get());
  }

  return out_vec;
}

const QString& ThemeManager::base_style() const {
  QFile file(":/resources/themes/base.css");
  file.open(QFile::ReadOnly | QFile::Text);

  static QString* style = nullptr;
  if (!style) {
    style = new QString(file.readAll());
    style->remove(QRegularExpression("[\\n\\t\\r]"));
  }

  return *style;
}

void ThemeManager::LoadThemes() {
  QString theme_dir;

  if (cvars::theme_path.empty()) {
    theme_dir = ":/resources/themes/";
  } else {
    theme_dir = cvars::theme_path.c_str();
  }

  QDirIterator iter(theme_dir, QDir::Dirs | QDir::NoDotAndDotDot);

  while (iter.hasNext()) {
    LoadTheme(iter.next());
  }
}

const Theme* ThemeManager::LoadTheme(const QString& path) {
  XELOGD("Loading UI theme at directory '{}'", path.toUtf8());
  QSharedPointer<Theme> theme(new Theme(path));

  ThemeStatus status = theme->LoadTheme();
  if (status == THEME_LOAD_OK) {
    themes_.push_back(theme);
    return theme.get();
  }

  XELOGW("Could not load theme at directory '{}'", path.toUtf8());
  return nullptr;
}

void ThemeManager::EnableHotReload() {
  XELOGD("Enabling UI Hot Reload");

  // reset watcher and rebind callback signal
  watcher_.reset(new QFileSystemWatcher());
  connect(watcher_.get(), SIGNAL(fileChanged(QString)), this,
          SLOT(OnThemeDirectoryChanged(QString)));

  // iterate over all files in theme dir, adding each to the file watcher
  for (const auto& theme : themes_) {
    QDirIterator iter(theme->directory(), QDir::Files,
                      QDirIterator::Subdirectories);

    while (iter.hasNext()) {
      watcher_->addPath(iter.next());
    }
  }
}

void ThemeManager::DisableHotReload() {
  XELOGD("Disabling UI Hot Reload");
  watcher_.reset();
}

void ThemeManager::OnThemeDirectoryChanged(const QString& path) {
  // check if changed file belongs to any tracked theme by checking path roots
  for (auto& theme : themes_) {
    if (path.startsWith(theme->directory())) {
      theme->ReloadTheme();
    }
  }
}

}  // namespace qt
}  // namespace ui
}  // namespace xe