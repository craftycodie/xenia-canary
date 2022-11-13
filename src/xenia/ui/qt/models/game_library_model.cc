#include "xenia/ui/qt/models/game_library_model.h"
#include "xenia/base/string_util.h"

#include <QIcon>
#include <QLabel>

namespace xe {
namespace ui {
namespace qt {

GameLibraryModel::GameLibraryModel(QObject* parent) {}

QVariant GameLibraryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  GameLibrary& library = GameLibrary::Instance();

  const GameEntry& entry = library.games()[index.row()];

  GameColumn column = (GameColumn)index.column();
  switch (column) {
    case GameColumn::kIconColumn:
      if (role == Qt::DisplayRole) {
        QImage image;
        QByteArray image_data(
            reinterpret_cast<const char*>(entry.icon_data().data()),
            static_cast<int>(entry.icon_size()));

        image.loadFromData(image_data);
        return image;
      }
      break;
    case GameColumn::kTitleColumn:
      if (role == Qt::DisplayRole) {
        return QString::fromUtf8(entry.title().c_str());
      }
      break;
    case GameColumn::kTitleIdColumn:
      if (role == Qt::DisplayRole) {
        auto title_id = xe::string_util::to_hex_string(entry.title_id());
        return QString::fromUtf8(title_id.c_str());
      }
      break;
    case GameColumn::kMediaIdColumn:
      if (role == Qt::DisplayRole) {
        auto media_id = xe::string_util::to_hex_string(entry.media_id());
        return QString::fromUtf8(media_id.c_str());
      }
      break;
    case GameColumn::kPathColumn:
      if (role == Qt::DisplayRole) {
        return QString::fromWCharArray(entry.file_path().c_str());
      }
      break;
    case GameColumn::kVersionColumn:
      if (role == Qt::DisplayRole) {
        return QString(entry.version().c_str());
      }
    case GameColumn::kGenreColumn:
      if (role == Qt::DisplayRole) {
        return QString::fromUtf8(entry.genre().c_str());
      }
      break;
    case GameColumn::kReleaseDateColumn:
      if (role == Qt::DisplayRole) {
        return QString::fromUtf8(entry.release_date().c_str());
      }
      break;
    case GameColumn::kBuildDateColumn:
      if (role == Qt::DisplayRole) {
        return QString::fromUtf8(entry.build_date().c_str());
      }
      break;
    case GameColumn::kLastPlayedColumn:
      return QVariant();  // TODO
    case GameColumn::kTimePlayedColumn:
      return QVariant();  // TODO
    case GameColumn::kAchievementsUnlockedColumn:
      return QVariant();  // TODO
    case GameColumn::kGamerscoreUnlockedColumn:
      return QVariant();  // TODO
    case GameColumn::kGameRatingColumn:
      return QVariant();  // TODO
    case GameColumn::kGameRegionColumn:
      if (role == Qt::DisplayRole) {
        const auto& regions = entry.regions();
        if (!regions.empty()) {
          return QString(regions[0].c_str());
        }
        return QVariant();
      }
      break;
    case GameColumn::kCompatabilityColumn:
      return QVariant();  // TODO
    case GameColumn::kPlayerCountColumn:
      if (role == Qt::DisplayRole) {
        return QString::number(entry.player_count());
      }
      break;
  }

  return QVariant();
}

QVariant GameLibraryModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
  if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
    return QVariant();
  }

  GameColumn column = (GameColumn)section;
  switch (column) {
    case GameColumn::kIconColumn:
      return tr("");
    case GameColumn::kTitleColumn:
      return tr("Title");
    case GameColumn::kTitleIdColumn:
      return tr("Title ID");
    case GameColumn::kMediaIdColumn:
      return tr("Media ID");
    case GameColumn::kPathColumn:
      return tr("Path");
    case GameColumn::kVersionColumn:
      return tr("Version");
    case GameColumn::kGenreColumn:
      return tr("Genre");
    case GameColumn::kReleaseDateColumn:
      return tr("Release Date");
    case GameColumn::kBuildDateColumn:
      return tr("Build Date");
    case GameColumn::kLastPlayedColumn:
      return tr("Last Played");
    case GameColumn::kTimePlayedColumn:
      return tr("Time Played");
    case GameColumn::kAchievementsUnlockedColumn:
      return tr("Achievements");
    case GameColumn::kGamerscoreUnlockedColumn:
      return tr("Gamerscore");
    case GameColumn::kGameRatingColumn:
      return tr("Rating");
    case GameColumn::kGameRegionColumn:
      return tr("Region");
    case GameColumn::kCompatabilityColumn:
      return tr("Compatibility");
    case GameColumn::kPlayerCountColumn:
      return tr("# Players");
    default:
      return QVariant();  // Should not be seeing this
  }
}

int GameLibraryModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {  // TODO
    return 0;
  }

  GameLibrary& library = GameLibrary::Instance();

  return static_cast<int>(library.size());
}

int GameLibraryModel::columnCount(const QModelIndex& parent) const {
  return static_cast<int>(GameColumn::kColumnCount);
}

void GameLibraryModel::refresh() {
  beginResetModel();
  endResetModel();
}

}  // namespace qt
}  // namespace ui
}  // namespace xe