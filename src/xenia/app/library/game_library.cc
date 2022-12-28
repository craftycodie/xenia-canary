#include "xenia/app/library/game_library.h"
#include <algorithm>
#include "xenia/app/library/game_scanner.h"

namespace xe {
namespace app {
namespace library {

using AsyncCallback = GameLibrary::AsyncCallback;

GameLibrary& GameLibrary::Instance() {
  static GameLibrary gameLibrary;

  return gameLibrary;
}

bool GameLibrary::ContainsPath(const std::filesystem::path& path) const {
  auto existing = std::find(paths_.begin(), paths_.end(), path);
  if (existing != paths_.end()) {
    return false;  // Path already exists.
  }
  return true;
}

bool GameLibrary::ContainsGame(uint32_t title_id) const {
  return FindGame(title_id) != nullptr;
}

const GameEntry* GameLibrary::FindGame(const uint32_t title_id) const {
  auto result = std::find_if(games_.begin(), games_.end(),
                             [title_id](const GameEntry& entry) {
                               return entry.title_id() == title_id;
                             });
  if (result != games_.end()) {
    return &*result;
  }
  return nullptr;
}

bool GameLibrary::RemovePath(const std::filesystem::path& path) {
  auto existing = std::find(paths_.begin(), paths_.end(), path);
  if (existing == paths_.end()) {
    return false;  // Path does not exist.
  }

  paths_.erase(existing);
  return true;
}

int GameLibrary::ScanPath(const std::filesystem::path& path) {
  int count = 0;

  AddPath(path);
  const auto& results = GameScanner::ScanPath(path);
  for (const GameEntry& result : results) {
    count++;
    AddGame(result);
  }
  return count;
}

int GameLibrary::ScanPathAsync(const std::filesystem::path& path,
                               const AsyncCallback& cb) {
  AddPath(path);

  auto paths = GameScanner::FindGamesInPath(path);
  int count = static_cast<int>(paths.size());
  return GameScanner::ScanPathsAsync(
      paths, [=](const GameEntry& entry, int scanned) {
        AddGame(entry);
        if (cb) {
          cb((static_cast<double>(scanned) / count) * 100.0, entry);
        }
      });
}

void GameLibrary::AddGame(const GameEntry& game) {
  std::lock_guard<std::mutex> lock(mutex_);

  uint32_t title_id = game.title_id();

  const auto& begin = games_.begin();
  const auto& end = games_.end();

  auto result = end;
  // check if title already exists in library (but skip titles with a title id
  // of 00000000)
  if (title_id != 0x00000000) {
    result = std::find_if(begin, end, [title_id](const GameEntry& entry) {
      return entry.title_id() == title_id;
    });
  }

  // title already exists in library, overwrite existing
  if (result != games_.end()) {
    *result = game;
  } else {
    games_.push_back(game);
  }
}

void GameLibrary::AddPath(const std::filesystem::path& path) {
  auto result = std::find(paths_.begin(), paths_.end(), path);

  // only save unique paths
  if (result == paths_.end()) {
    paths_.push_back(path);
  }
}

}  // namespace library
}  // namespace app
}  // namespace xe
