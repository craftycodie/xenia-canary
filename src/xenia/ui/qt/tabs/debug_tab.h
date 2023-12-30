#ifndef XENIA_UI_QT_TABS_DEBUG_H_
#define XENIA_UI_QT_TABS_DEBUG_H_

#include <QHBoxLayout>
#include <QStackedLayout>
#include <QStackedWidget>

#include "xenia/ui/qt/tabs/split_tab.h"
#include "xenia/ui/qt/widgets/sidebar.h"

namespace xe {
namespace ui {
namespace qt {

class XTabSelector;

class DebugTab final : public SplitTab {
  Q_OBJECT
 public:
  explicit DebugTab();

 private:

  QWidget* CreateComponentsTab();
  QWidget* CreateNavigationTab();
  QWidget* CreateThemeTab();
  QWidget* CreateLibraryTab();

  // create widgets for "components" tab
  QWidget* CreateButtonGroup();
  QWidget* CreateSliderGroup();
  QWidget* CreateCheckboxGroup();
  QWidget* CreateRadioButtonGroup();
  QWidget* CreateInputGroup();

  // create widgets for "navigation" tab
  QWidget* CreateTab1Widget(XTabSelector* tab_selector,
                            QStackedLayout* tab_stack_layout);
  QWidget* CreateTab2Widget(XTabSelector* tab_selector,
                            QStackedLayout* tab_stack_layout);
  QWidget* CreateTab3Widget(XTabSelector* tab_selector,
                            QStackedLayout* tab_stack_layout);
};

}  // namespace qt
}  // namespace ui
}  // namespace xe

#endif