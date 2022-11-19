#ifndef XENIA_APP_GAME_ENTRY_H_
#define XENIA_APP_GAME_ENTRY_H_

#include <map>
#include <vector>
#include <optional>
#include "xenia/app/library/scanner_utils.h"
#include "xenia/kernel/util/xex2_info.h"

namespace xe {
namespace app {
namespace library {

// these are defined in greater detail in xex2_info.h
enum class GameRatingRegulator : uint8_t {
  Esrb,
  Pegi,
  PegiFI, // finland
  PegiPT, // portugal
  Bbfc,
  Cero,
  Usk,
  OflcAU,
  OflcNZ,
  Kmrb,
  Brazil,
  Fpb
};

class GameEntry {
 public:
  using IconData = std::vector<uint8_t>;
  using GameRatingMap = std::map<GameRatingRegulator, uint32_t>; //key: regulator, value: rating

  static std::optional<GameEntry> FromGameInfo(const GameInfo& info);

  explicit GameEntry();
  explicit GameEntry(const GameEntry& other) = default;

  bool is_valid() const;
  bool is_missing_data() const;
  bool apply_info(const GameInfo& info);

  const XGameFormat& format() const { return format_; }
  std::filesystem::path file_path() const { return file_path_; }
  std::filesystem::path file_name() const { return file_path_.filename(); }
  const std::map<std::filesystem::path, uint32_t> launch_paths() const {
    return launch_paths_;
  }
  const std::map<uint32_t, std::filesystem::path> default_launch_paths() const {
    return default_launch_paths_;
  }

  const std::string& title() const { return title_; }
  const IconData& icon_data() const { return icon_data_; }
  const uint32_t title_id() const { return title_id_; }
  const uint32_t media_id() const { return media_id_; }
  const std::vector<uint32_t>& alt_title_ids() const { return alt_title_ids_; }
  const std::vector<uint32_t>& alt_media_ids() const { return alt_media_ids_; }
  const std::map<uint8_t, uint32_t>& disc_map() const { return disc_map_; }
  const std::string& version() const { return version_; }
  const std::string& base_version() const { return base_version_; }
  const GameRatingMap& ratings() const { return ratings_; }
  std::vector<std::string> regions() const;
  const std::string& genre() const { return genre_; }
  const std::string& build_date() const { return build_date_; }
  const std::string& release_date() const { return release_date_; }
  const uint8_t& player_count() const { return player_count_; }

 private:
  // File Info
  XGameFormat format_;
  std::filesystem::path file_path_;
  std::map<std::filesystem::path, uint32_t> launch_paths_;  // <Path, MediaId>
  std::map<uint32_t, std::filesystem::path>
      default_launch_paths_;  // <MediaId, Path>

  // Game Metadata
  std::string title_;
  IconData icon_data_;
  uint32_t title_id_ = 0;
  uint32_t media_id_ = 0;
  std::vector<uint32_t> alt_title_ids_;
  std::vector<uint32_t> alt_media_ids_;
  std::map<uint8_t, uint32_t> disc_map_;  // <Disc #, MediaID>
  std::string version_;
  std::string base_version_;
  GameRatingMap ratings_;
  xex2_region_flags regions_;
  std::string build_date_;
  std::string genre_;
  std::string release_date_;
  uint8_t player_count_ = 0;
};

}  // namespace library
}  // namespace app
}  // namespace xe

#endif