#ifndef XENIA_UI_QT_THEMEABLEWIDGET_H_
#define XENIA_UI_QT_THEMEABLEWIDGET_H_

#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include "theme_manager.h"

#include <QThread>
#include <QtDebug>

#include "xenia/base/assert.h"

namespace xe {
namespace ui {
namespace qt {

template <typename T>
class Themeable : public T {
 public:
  template <typename... Args>
  Themeable(QString name, Args&&... args) : T(args...) {
    static_assert(std::is_base_of<QWidget, T>::value,
                  "T is not derived from QWidget");

    ApplyTheme(name);
  }

  /**
   * Loads the specified theme from the theme manager for this object
   * widget.
   * @param object_name object name to lookup style for
   */
  void ApplyTheme(const QString& object_name) {
    if (!object_name.isEmpty()) {
      reinterpret_cast<QWidget*>(this)->setObjectName(object_name);
    }

    ThemeManager& manager = ThemeManager::Instance();
    Theme* theme = manager.current_theme();
    if (theme) {
      QWidget::connect(theme, &Theme::ThemeReloaded, this,
                       &Themeable::OnThemeReloaded);

      LoadAndApplyStylesheet(*theme, object_name);
    }
  }

  /**
   * Loads any relevant stylesheets for this component and applies them.
   */
  void LoadAndApplyStylesheet(const Theme& theme, const QString& object_name) {
    ThemeManager& manager = ThemeManager::Instance();
    QString style = theme.StylesheetForComponent(object_name);
    QString base_style = manager.base_style();

    QString stylesheet = QString("%1 %2").arg(style, base_style);

    if (!stylesheet.isEmpty()) {
      reinterpret_cast<QWidget*>(this)->setStyleSheet(stylesheet);
    }
  }

  /**
   * Refreshes the style of the current widget.
   * Call this after applying dynamic properties (e.g. with setProperty())
   */
  void RefreshStyle() {
    this->style()->unpolish(this);
    this->style()->polish(this);
  }

  /**
   * Called when the global theme is reloaded (e.g. if Hot Reload is enabled)
   */
  void OnThemeReloaded() {
    // Check theme reload callback was fired from valid thread context
    assert_true(QThread::currentThread() ==
                    QApplication::instance()->thread());

    QString object_name = reinterpret_cast<QWidget*>(this)->objectName();

    ThemeManager& manager = ThemeManager::Instance();
    Theme* theme = manager.current_theme();
    if (theme) {
      LoadAndApplyStylesheet(*theme, object_name);
      RefreshStyle();
    }
  }

  virtual void paintEvent(QPaintEvent* event) override {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    reinterpret_cast<QWidget*>(this)->style()->drawPrimitive(QStyle::PE_Widget,
                                                             &opt, &p, this);
    T::paintEvent(event);
  }
};

}  // namespace qt
}  // namespace ui
}  // namespace xe
#endif