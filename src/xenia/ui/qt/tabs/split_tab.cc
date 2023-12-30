/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "split_tab.h"

#include <QButtonGroup>

#include "xenia/ui/qt/widgets/sidebar.h"

#include <QGraphicsEffect>
#include <QStackedWidget>

#include "xenia/ui/qt/widgets/separator.h"

namespace xe {
namespace ui {
namespace qt {

SplitTab::SplitTab(const QString& tab_name) : XTab(tab_name) {}

SplitTab::SplitTab(const QString& tab_name, const QString& object_name)
    : XTab(tab_name, object_name) {}

void SplitTab::Build() {
  // build initial layout
  layout_ = new QHBoxLayout();
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->setSpacing(0);
  setLayout(layout_);

  // build sidebar
  {
    sidebar_container_ = new QWidget(this);
    sidebar_container_->setObjectName("sidebarContainer");

    QVBoxLayout* sidebar_layout = new QVBoxLayout;
    sidebar_layout->setContentsMargins(0, 0, 0, 0);
    sidebar_layout->setSpacing(0);

    sidebar_container_->setLayout(sidebar_layout);

    // Add drop shadow to sidebar widget
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(16);
    effect->setXOffset(4);
    effect->setYOffset(0);
    effect->setColor(QColor(0, 0, 0, 64));

    sidebar_container_->setGraphicsEffect(effect);

    // Create sidebar
    sidebar_ = new XSideBar;
    sidebar_->setOrientation(Qt::Vertical);
    sidebar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sidebar_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    // Create sidebar title
    QWidget* sidebar_title = new QWidget;
    sidebar_title->setObjectName("sidebarTitle");

    QVBoxLayout* title_layout = new QVBoxLayout;
    title_layout->setContentsMargins(0, 40, 0, 0);
    title_layout->setSpacing(0);

    sidebar_title->setLayout(title_layout);

    // Title labels
    QLabel* xenia_title = new QLabel(tab_name());
    xenia_title->setObjectName("sidebarTitleLabel");
    xenia_title->setAlignment(Qt::AlignCenter);
    title_layout->addWidget(xenia_title, 0, Qt::AlignCenter);

    // Title separator
    auto separator = new XSeparator;
    title_layout->addSpacing(32);
    title_layout->addWidget(separator, 0, Qt::AlignCenter);

    // Setup Sidebar toolbar
    sidebar_->addWidget(sidebar_title);

    sidebar_->addSpacing(20);

    QButtonGroup* bg = new QButtonGroup();

    // loop over sidebar button items and connect them to slots
    int counter = 0;
    for (auto it = sidebar_items_.begin(); it != sidebar_items_.end();
         ++it, ++counter) {
      SidebarItem& item = *it;
      auto btn = sidebar_->addAction(item.glyph(), item.title());
      btn->setCheckable(true);
      bg->addButton(btn);

      // set the first item to checked
      if (counter == 0) {
        btn->setChecked(true);
      }

      // link up the clicked signal
      connect(btn, &XSideBarButton::clicked,
              [&]() { content_widget_->setCurrentWidget(item.widget()); });
    }

    sidebar_layout->addWidget(sidebar_, 0, Qt::AlignHCenter | Qt::AlignTop);
    sidebar_layout->addStretch(1);

    // Add sidebar to tab widget
    layout_->addWidget(sidebar_container_, 0, Qt::AlignLeft);
  }

  // build content
  {
    content_widget_ = new QStackedWidget();

    for (const SidebarItem& item : sidebar_items_) {
      content_widget_->addWidget(item.widget());
    }

    layout_->addWidget(content_widget_);
  }
}

void SplitTab::AddSidebarItem(const SidebarItem& item) {
  sidebar_items_.push_back(item);
  // TODO: recreate sidebar and content here?
}

}  // namespace qt
}  // namespace ui
}  // namespace xe