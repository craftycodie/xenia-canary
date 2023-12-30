#include "settings_tab.h"

#include <QButtonGroup>
#include <QGraphicsEffect>
#include <QScrollArea>

#include "xenia/ui/qt/settings/panes/advanced_pane.h"
#include "xenia/ui/qt/settings/panes/controls_pane.h"
#include "xenia/ui/qt/settings/panes/cpu_pane.h"
#include "xenia/ui/qt/settings/panes/general_pane.h"
#include "xenia/ui/qt/settings/panes/gpu_pane.h"
#include "xenia/ui/qt/settings/panes/interface_pane.h"
#include "xenia/ui/qt/settings/panes/library_pane.h"

#include "xenia/ui/qt/widgets/separator.h"

namespace xe {
namespace ui {
namespace qt {

SettingsTab::SettingsTab() : SplitTab("Settings", "SettingsTab") {
  settings_panes_ = {new GeneralPane(this),  new CPUPane(this),
                     new GPUPane(this),      new InterfacePane(this),
                     new ControlsPane(this), new LibraryPane(this),
                     new AdvancedPane(this)};

  for (SettingsPane* pane : settings_panes_) {
    pane->Build();
    AddSidebarItem(*pane);
  }

  Build();
}

}  // namespace qt
}  // namespace ui
}  // namespace xe