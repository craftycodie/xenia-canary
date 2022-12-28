#include "xenia/app/library/game_entry.h"
#include "third_party/fmt/include/fmt/format.h"

namespace xe {
namespace app {
namespace library {

std::optional<GameEntry> GameEntry::FromGameInfo(const GameInfo& info) {
  GameEntry entry;
  auto result = entry.apply_info(info);

  if (!result) return std::nullopt;
  return std::make_optional(entry);
};

GameEntry::GameEntry()
    : format_(XGameFormat::kUnknown), regions_(XGameRegions::XEX_REGION_ALL) {}

bool GameEntry::is_valid() const {
  // Minimum requirements
  return !file_path_.empty() && title_id_ && media_id_;
}

bool GameEntry::is_missing_data() const {
  return title_.length() == 0 || icon_data().empty() || disc_map_.empty();
  // TODO: Version
  // TODO: Base Version
  // TODO: Ratings
  // TODO: Regions
}

bool GameEntry::apply_info(const GameInfo& info) {
  const XexInfo& xex_info = info.xex_info;
  const NxeInfo& nxe_info = info.nxe_info;

  format_ = info.format;
  file_path_ = info.path;

  title_id_ = xex_info.execution_info.title_id;
  media_id_ = xex_info.execution_info.media_id;

  auto version_data = xex_info.execution_info.version();
  auto base_version_data = xex_info.execution_info.base_version();

  uint32_t major = version_data.major;
  uint32_t minor = version_data.minor;
  uint32_t build = version_data.build;
  uint32_t qfe = version_data.qfe;

  version_ = fmt::format("{0}.{1}.{2}.{3}", major, minor, build, qfe);

  auto ratings_info = xex_info.game_ratings;
  ratings_[GameRatingRegulator::Esrb] = ratings_info.esrb;
  ratings_[GameRatingRegulator::Pegi] = ratings_info.pegi;
  ratings_[GameRatingRegulator::PegiFI] = ratings_info.pegifi;
  ratings_[GameRatingRegulator::PegiPT] = ratings_info.pegipt;
  ratings_[GameRatingRegulator::Bbfc] = ratings_info.bbfc;
  ratings_[GameRatingRegulator::Cero] = ratings_info.cero;
  ratings_[GameRatingRegulator::Usk] = ratings_info.usk;
  ratings_[GameRatingRegulator::OflcAU] = ratings_info.oflcau;
  ratings_[GameRatingRegulator::OflcNZ] = ratings_info.oflcnz;
  ratings_[GameRatingRegulator::Kmrb] = ratings_info.kmrb;
  ratings_[GameRatingRegulator::Brazil] = ratings_info.brazil;
  ratings_[GameRatingRegulator::Fpb] = ratings_info.fpb;

  regions_ =
      static_cast<xex2_region_flags>(xex_info.security_info.region.get());

  // Add to disc map / launch paths
  auto disc_id = xex_info.execution_info.disc_number;
  disc_map_.insert_or_assign(disc_id, media_id_);
  launch_paths_.insert_or_assign(info.path, media_id_);
  if (!default_launch_paths_.count(media_id_)) {
    default_launch_paths_.insert(std::make_pair(media_id_, info.path));
  }

  if (xex_info.game_title.length() > 0) {
    title_ = xex_info.game_title;
  } else if (nxe_info.game_title.length() > 0) {
    title_ = nxe_info.game_title;
  } else {
    title_ = "<unknown>";
  }

  uint8_t* icon_data = nullptr;
  size_t icon_size = 0;
  // TODO: prefer nxe icon over xex icon?
  if (xex_info.icon) {
    icon_data = xex_info.icon;
    icon_size = xex_info.icon_size;
  } else if (nxe_info.icon) {
    icon_data = nxe_info.icon;
    icon_size = nxe_info.icon_size;
  }

  icon_data_ = IconData(icon_size);
  std::copy_n(icon_data, icon_size, icon_data_.begin());

  return true;
}

std::vector<std::string> GameEntry::regions() const {
  std::vector<std::string> regions;

  // TODO: implement
  return regions;
}

}  // namespace library
}  // namespace app
}  // namespace xe