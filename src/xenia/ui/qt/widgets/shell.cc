#include "xenia/ui/qt/widgets/shell.h"

#include <QMessageBox>
#include "xenia/ui/qt/events/hid_event.h"

namespace xe {
namespace ui {
namespace qt {

XShell::XShell(QMainWindow* window) : Themeable<QWidget>("XShell") {
  window_ = window;
  Build();
}

void XShell::Build() {
  // Build Main Layout
  layout_ = new QVBoxLayout();
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(0);
  this->setLayout(layout_);

  BuildNav();
  BuildTabStack();
}

void XShell::BuildNav() {
  nav_ = new XNav();
  connect(nav_, SIGNAL(TabChanged(XTab*)), this, SLOT(TabChanged(XTab*)));
  layout_->addWidget(nav_, 0, Qt::AlignTop);
}

void XShell::BuildTabStack() {
  tab_stack_ = new QStackedLayout(window_);
  layout_->addLayout(tab_stack_);

  for (XTab* tab : nav_->tabs()) {
    tab_stack_->addWidget(tab);
  }
}

void XShell::TabChanged(XTab* tab) {
  tab_stack_->setCurrentWidget(tab);
  tab->setFocus(Qt::FocusReason::TabFocusReason);
}

bool XShell::event(QEvent* event) {
  if (event->type() == HidEvent::ButtonPressType) {
    auto button_event = static_cast<ButtonPressEvent*>(event);
    if (button_event->is_repeat()) return true;  // skip repeat triggers

    int tab_index = nav_->active_tab_index();

    if (button_event->buttons() & kInputRightShoulder) {
      tab_index++;
      nav_->SetActiveTabByIndex(tab_index);

      event->accept();
      return true;
    } else if (button_event->buttons() & kInputLeftShoulder) {
      tab_index--;
      nav_->SetActiveTabByIndex(tab_index);

      event->accept();
      return true;
    }
  }

  return QWidget::event(event);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe