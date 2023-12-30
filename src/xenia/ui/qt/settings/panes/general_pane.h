#ifndef XENIA_UI_QT_GENERAL_PANE_H_
#define XENIA_UI_QT_GENERAL_PANE_H_

#include "settings_pane.h"

namespace xe {
namespace ui {
namespace qt {

class XGroupBox;

class GeneralPane : public SettingsPane {
  Q_OBJECT
 public:
  explicit GeneralPane(QWidget* parent = nullptr)
      : SettingsPane(QChar(0xE713), "General", parent) {}

  void Build() override;
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif