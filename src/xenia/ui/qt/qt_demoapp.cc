/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <filesystem>

#include <QFile>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>

#include "main_window.h"
#include "window_qt.h"
#include "xenia/config.h"
#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/ui/window.h"
#include "xenia/ui/windowed_app.h"

DEFINE_bool(show_debug_tab, true, "Show the debug tab in the Qt UI", "General");

DEFINE_string(apu, "any", "Audio system. Use: [any, nop, sdl, xaudio2]", "APU");
DEFINE_string(gpu, "any", "Graphics system. Use: [any, d3d12, vulkan, null]",
              "GPU");
DEFINE_string(hid, "any", "Input system. Use: [any, nop, sdl, winkey, xinput]",
              "HID");

DEFINE_path(
    storage_root, "",
    "Root path for persistent internal data storage (config, etc.), or empty "
    "to use the path preferred for the OS, such as the documents folder, or "
    "the emulator executable directory if portable.txt is present in it.",
    "Storage");
DEFINE_path(
    content_root, "",
    "Root path for guest content storage (saves, etc.), or empty to use the "
    "content folder under the storage root.",
    "Storage");
DEFINE_path(
    cache_root, "",
    "Root path for files used to speed up certain parts of the emulator or the "
    "game. These files may be persistent, but they can be deleted without "
    "major side effects such as progress loss. If empty, the cache folder "
    "under the storage root, or, if available, the cache directory preferred "
    "for the OS, will be used.",
    "Storage");

DEFINE_bool(mount_scratch, false, "Enable scratch mount", "Storage");
DEFINE_bool(mount_cache, false, "Enable cache mount", "Storage");

DEFINE_transient_path(target, "",
                      "Specifies the target .xex or .iso to execute.",
                      "General");
DEFINE_transient_bool(portable, false,
                      "Specifies if Xenia should run in portable mode.",
                      "General");

DECLARE_bool(debug);

DEFINE_bool(discord, true, "Enable Discord rich presence", "General");

namespace xe {
namespace ui {
namespace qt {

class QtDemoApp final : public xe::ui::WindowedApp {
public:
  static std::unique_ptr<WindowedApp> Create(
      xe::ui::WindowedAppContext& app_context) {
    return std::unique_ptr<WindowedApp>(new QtDemoApp(app_context));
  }

  ~QtDemoApp() override;
  bool OnInitialize() override;
protected:
  void OnDestroy() override;

private:
  std::unique_ptr<QtWindow> window_;
  explicit QtDemoApp(WindowedAppContext& app_context);
};

QtDemoApp::QtDemoApp(WindowedAppContext& app_context)
    : WindowedApp(app_context, "xenia"),
      window_(std::make_unique<MainWindow>(app_context, "Xenia Qt Demo App", 1280, 720)) {}

QtDemoApp::~QtDemoApp() = default;

bool QtDemoApp::OnInitialize() {
  // Figure out where internal files and content should go.
  std::filesystem::path storage_root = xe::cvars::storage_root;
  if (storage_root.empty()) {
    storage_root = xe::filesystem::GetExecutableFolder();
    if (!cvars::portable &&
        !std::filesystem::exists(storage_root / "portable.txt")) {
      storage_root = xe::filesystem::GetUserFolder();
#if defined(XE_PLATFORM_WIN32) || defined(XE_PLATFORM_GNU_LINUX)
      storage_root = storage_root / "Xenia";
#else
      // TODO(Triang3l): Point to the app's external storage "files" directory
      // on Android.
#warning Unhandled platform for the data root.
      storage_root = storage_root / "Xenia";
#endif
    }
  }
  storage_root = std::filesystem::absolute(storage_root);
  XELOGI("Storage root: {}", xe::path_to_utf8(storage_root));

  Config::Instance().SetupConfig(storage_root);

  std::filesystem::path content_root = cvars::content_root;
  if (content_root.empty()) {
    content_root = storage_root / "content";
  } else {
    // If content root isn't an absolute path, then it should be relative to the
    // storage root.
    if (!content_root.is_absolute()) {
      content_root = storage_root / content_root;
    }
  }
  content_root = std::filesystem::absolute(content_root);
  XELOGI("Content root: {}", xe::path_to_utf8(content_root));

  std::filesystem::path cache_root = cvars::cache_root;
  if (cache_root.empty()) {
    cache_root = storage_root / "cache";
    // TODO(Triang3l): Point to the app's external storage "cache" directory on
    // Android.
  } else {
    // If content root isn't an absolute path, then it should be relative to the
    // storage root.
    if (!cache_root.is_absolute()) {
      cache_root = storage_root / cache_root;
    }
  }
  cache_root = std::filesystem::absolute(cache_root);
  XELOGI("Cache root: {}", xe::path_to_utf8(cache_root));

  window_->Open();

  QFile icon_file(":/resources/graphics/icon.ico");
  QByteArray icon_data = icon_file.readAll();
  window_->SetIcon(icon_data.data(), icon_data.size());
  window_->move(qApp->primaryScreen()->geometry().center() -
                 window_->rect().center());

  return true;
}

void QtDemoApp::OnDestroy() { Config::Instance().SaveConfig(); }

}  // namespace qt
}  // namespace ui
}  // namespace xe


XE_DEFINE_WINDOWED_APP(xenia_demoapp, xe::ui::qt::QtDemoApp::Create);