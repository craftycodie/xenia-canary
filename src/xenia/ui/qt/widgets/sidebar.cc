#include "xenia/ui/qt/widgets/sidebar.h"

#include <QButtonGroup>

#include "xenia/ui/qt/events/hid_event.h"

namespace xe {
namespace ui {
namespace qt {

XSideBar::XSideBar()
    : Themeable<QToolBar>("XSideBar"), buttons_(new QButtonGroup(this)) {}

XSideBarButton* XSideBar::addAction(const QString& text) {
  auto button = new XSideBarButton(text);

  buttons_->addButton(button);
  QToolBar::addWidget(button);

  return button;
}

XSideBarButton* XSideBar::addAction(QChar glyph, const QString& text) {
  auto button = new XSideBarButton(glyph, text);

  buttons_->addButton(button);
  QToolBar::addWidget(button);

  return button;
}

QWidget* XSideBar::addSpacing(int size) {
  QWidget* spacer = new QWidget(this);
  spacer->setFixedHeight(size);
  QToolBar::addWidget(spacer);
  return spacer;
}

bool XSideBar::event(QEvent* event) {
  if (event->type() == HidEvent::ButtonPressType) {
    auto button_press_event = static_cast<ButtonPressEvent*>(event);
    // skip repeat presses
    if (button_press_event->is_repeat()) {
      return false;
    }

    int direction = 0;

    if (button_press_event->buttons() & kInputDpadDown) {
      direction = -1;
    } else if (button_press_event->buttons() & kInputDpadUp) {
      direction = 1;
    }

    if (direction != 0) {
      for (const auto& btn : buttons_->buttons()) {
        if (btn->hasFocus()) {
          if (int id = buttons_->id(btn); id != -1) {
            if (auto next = buttons_->button(id + direction)) {
              next->setFocus();
              break;
            }
          }
        }
      }

      event->accept();
      return true;
    }
  }

  return Themeable<QToolBar>::event(event);
}

}  // namespace qt
}  // namespace ui
}  // namespace xe