#ifndef XENIA_UI_QT_LIBRARY_PANE_H_
#define XENIA_UI_QT_LIBRARY_PANE_H_

#include "settings_pane.h"

namespace xe {
namespace ui {
namespace qt {

class LibraryPane : public SettingsPane {
  Q_OBJECT
 public:
  explicit LibraryPane(QWidget* parent = nullptr)
      : SettingsPane(QChar(0xE8F1), "Library", parent) {}

  void Build() override;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif