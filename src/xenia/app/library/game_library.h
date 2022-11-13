#ifndef XENIA_APP_GAME_LIBRARY_H_
#define XENIA_APP_GAME_LIBRARY_H_

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "xenia/app/library/game_entry.h"
#include "xenia/app/library/game_scanner.h"
#include "xenia/app/library/scanner_utils.h"

namespace xe {
namespace app {
namespace library {

class GameLibrary {
 public:
  using AsyncCallback = std::function<void(double, const GameEntry&)>;

  GameLibrary(GameLibrary const&) = delete;
  GameLibrary& operator=(GameLibrary const&) = delete;

  static GameLibrary& Instance();

  // Returns whether the library has scanned the provided path.
  bool ContainsPath(const std::filesystem::path& path) const;

  // Returns whether the library has scanned a game with the provided title id.
  bool ContainsGame(uint32_t title_id) const;

  // Searches the library for a scanned game entry, and returns it if present.
  const GameEntry* FindGame(const uint32_t title_id) const;

  // Scans path for supported games/apps synchronously.
  // Returns the number of games found.
  int ScanPath(const std::filesystem::path& path);

  // Scans path for supported games/apps asynchronously,
  // calling the provided callback for each game found.
  // Returns the number of games found.
  int ScanPathAsync(const std::filesystem::path& path, const AsyncCallback& cb);

  // Remove a path from the library.
  // If path is found, it is removed and the library is rescanned.
  bool RemovePath(const std::filesystem::path& path);

  // Add a manually crafted game entry to the library.
  void AddGame(const GameEntry& game);

  const std::vector<GameEntry>& games() const { return games_; }
  const size_t size() const { return games_.size(); }

  void Clear() { games_.clear(); }

  // TODO: Provide functions to load and save the library from a cache on disk.

 private:
  void AddPath(const std::filesystem::path& path);
  GameLibrary() = default;

  std::vector<GameEntry> games_;
  std::vector<std::filesystem::path> paths_;
  std::mutex mutex_;
};

}  // namespace library
}  // namespace app
}  // namespace xe

#endif