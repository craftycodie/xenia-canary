#include "xenia/ui/qt/main_window.h"

#include <QVBoxLayout>

#include "build/version.h"
#include "xenia/base/logging.h"
#include "xenia/ui/qt/widgets/status_bar.h"

#include "xenia/hid/sdl/sdl_hid.h"
#if XE_PLATFORM_WIN32
#include "xenia/hid/xinput/xinput_hid.h"
#endif

DECLARE_string(hid);

namespace xe {
namespace ui {
namespace qt {

MainWindow::MainWindow(WindowedAppContext& app_context, std::string_view title,
                       uint32_t width, uint32_t height)
    : Themeable<QtWindow>("MainWindow", app_context, title, width, height) {
  CreateInputSystem();
}

void MainWindow::CreateInputSystem() {
  input_system_ = std::make_unique<hid::InputSystem>(this);
  size_t z_order = 0;  // z-order unused for controller inputs in the UI

  // TODO: in future this may need logic to determine if a game is running
  auto driver_active_cb = [this]() -> bool { return true; };

  std::string real_hid = cvars::hid;
  if (cvars::hid == "any") {
#if XE_PLATFORM_WIN32
    real_hid = "xinput";
#else
    real_hid = "sdl";
#endif
  }

  std::vector<std::unique_ptr<hid::InputDriver>> drivers;

  // We are only interested in SDL or XInput here as Qt can handle kb+mouse events itself
  if (real_hid == "sdl") {
    auto driver = xe::hid::sdl::Create(this, z_order);
    drivers.emplace_back(std::move(driver));
  } else if (real_hid == "xinput") {
#if XE_PLATFORM_WIN32
    auto driver = xe::hid::xinput::Create(this, z_order);
    drivers.emplace_back(std::move(driver));
#else
    assert_always("XInput selected for unsupported OS");
#endif
  }

  for (auto& driver : drivers) {
    if (XSUCCEEDED(driver->Setup())) {
      driver->set_is_active_callback(driver_active_cb);
      input_system_->AddDriver(std::move(driver));
    }
  }
}

bool MainWindow::OpenImpl() {
  // Custom Frame Border
  // Disable for now until windows aero additions are added
  // setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

  this->setFocusPolicy(Qt::StrongFocus);

  shell_ = new XShell(this);
  this->setCentralWidget(shell_);

  status_bar_ = new XStatusBar(this);
  this->setStatusBar(status_bar_);

  QLabel* build_label = new QLabel;
  build_label->setObjectName("buildLabel");
  build_label->setText(QStringLiteral("Xenia: %1 / %2 / %3")
                           .arg(XE_BUILD_BRANCH)
                           .arg(XE_BUILD_COMMIT_SHORT)
                           .arg(XE_BUILD_DATE));
  status_bar_->addPermanentWidget(build_label);

  ThemeManager& theme_manager = ThemeManager::Instance();
  connect(&theme_manager, &ThemeManager::OnActiveThemeChanged, this,
          &MainWindow::OnActiveThemeChanged);

  Theme* active_theme = theme_manager.current_theme();
  if (active_theme) {
    connect(active_theme, &Theme::ThemeReloaded, this,
            &MainWindow::OnThemeReloaded);
  }

  // set up input poll timer as Qt's event loop has no tick() function
  {
    using namespace std::chrono_literals;

    input_process_timer_ = new QTimer(this);
    input_process_timer_->setInterval(16ms);

    connect(input_process_timer_, &QTimer::timeout, this,
            &MainWindow::PollInputStatus);

    input_process_timer_->start();
  }

  QtWindow::OpenImpl();
  return true;
}

void MainWindow::AddStatusBarWidget(QWidget* widget, bool permanent) {
  if (permanent) {
    status_bar_->addPermanentWidget(widget);
  } else {
    status_bar_->addWidget(widget);
  }
}

void MainWindow::RemoveStatusBarWidget(QWidget* widget) {
  return status_bar_->removeWidget(widget);
}

void MainWindow::OnActiveThemeChanged(Theme* theme) {
  assert_true(theme != nullptr);

  disconnect(hot_reload_signal_);
  connect(theme, &Theme::ThemeReloaded, this, &MainWindow::OnThemeReloaded);
}

void MainWindow::OnThemeReloaded() {
  status_bar_->showMessage(QStringLiteral("Theme Reloaded"), 3000);
}

void MainWindow::PollInputStatus() {
  using namespace xe::hid;

  int user_index = 0; // only allow main user to control UI

  X_INPUT_STATE state{};
  if (input_system_->GetState(user_index, &state) != X_ERROR_SUCCESS) {
    return;
  }

  const auto& gamepad = state.gamepad;

  if (gamepad.buttons & X_INPUT_GAMEPAD_A) {
    DebugBreak();
  }
}

}  // namespace qt
}  // namespace ui
}  // namespace xe