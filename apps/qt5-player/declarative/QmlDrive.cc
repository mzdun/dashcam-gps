#include "QmlDrive.hh"

#include <QMediaPlaylist>
#include <QQmlEngine>

namespace mgps::declarative {
	using namespace std::chrono;
	using namespace date;

	QmlDrive::QmlDrive() {}

	void QmlDrive::setDrive(mgps::drive const* drive) {
		drive_ = drive;

		if (drive_) {
			duration_ = drive_->playlist.duration;
			populateLines();
		} else {
			duration_ = {};
			lines_.clear();
		}

		populatePlaylist();

		emit driveChanged();
	}

	/*********************************************************************
	 *  CALLABLES
	 */

	void QmlDrive::playerAvailable(QObject* qmlMediaPlayer) {
		auto player = qvariant_cast<QMediaPlayer*>(
		    qmlMediaPlayer->property("mediaObject"));
		player_ = player;
		populatePlaylist();
	}

	/*********************************************************************
	 *  SETTERS FOR PROPERTIES
	 */

	void QmlDrive::setPlayback(unsigned long long new_millis, bool forced) {
		auto const millis =
		    milliseconds{new_millis} + getOffset(currentIndex());
		if (!forced && millis == playback_) return;

		auto const old_playback = floor<seconds>(playback_);
		auto const old_timeline = floor<seconds>(timeline_);

		playback_ = millis;

		auto const new_timeline = playbackToTravel(millis);
		auto const emit_timeline = new_timeline != timeline_;
		timeline_ = new_timeline;

		auto const [new_position, new_speed] = travelToPosition(new_timeline);
		auto const emit_position = new_position != car_position_;
		car_position_ = new_position;
		auto const emit_speed = new_speed != car_speed_;
		car_speed_ = new_speed;

		emit playbackChanged();
		if (forced || floor<seconds>(playback_) != old_playback)
			emit playbackStringChanged();
		if (forced || emit_timeline) emit timelineChanged();
		if (forced || floor<seconds>(timeline_) != old_timeline)
			emit timelineStringChanged();
		if (forced || emit_position) emit carPositionChanged();
		if (forced || emit_speed) emit carSpeedChanged();
	}

	/*********************************************************************
	 *  GETTERS FOR PROPERTIES
	 */

	QGeoRectangle QmlDrive::visibleRegion() const noexcept {
		if (!drive_) return QGeoRectangle{};
		static constexpr auto MARGIN = 0.01;
		auto const bounds = drive_->trace.boundary_box();
		auto const center = bounds.center();
		auto const width = bounds.width().as_float() + 2 * MARGIN;
		auto const height = bounds.height().as_float() + 2 * MARGIN;
		return {QGeoCoordinate{center.lat.as_float(), center.lon.as_float()},
		        width, height};
	}

	QDateTime QmlDrive::timeline() const noexcept {
		auto const world = travelToLocal(timeline_);
		auto const secs = floor<seconds>(world);
		auto const date = floor<days>(secs);
		auto const time = secs - date;

		auto const ymd = year_month_day{local_days{date}};
		auto const hms = hh_mm_ss{time};
		return {QDate{int(ymd.year()), int(unsigned(ymd.month())),
		              int(unsigned(ymd.day()))},
		        QTime{int(hms.hours().count()), int(hms.minutes().count()),
		              int(hms.seconds().count())}};
	}

	QString QmlDrive::playbackString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(playback_).time_since_epoch()};
		return QString::fromStdString(ostr.str());
	}

	QString QmlDrive::durationString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(duration_)};
		return QString::fromStdString(ostr.str());
	}

	QString QmlDrive::timelineString() const {
		std::ostringstream ostr;
		ostr << floor<seconds>(travelToLocal(timeline_));
		return QString::fromStdString(ostr.str());
	}

	/*********************************************************************
	 *  HELPERS
	 */

	void QmlDrive::populatePlaylist() {
		if (!player_) return;
		player_->stop();

		using namespace std::chrono;
		auto offset = playback_ms{};

		if (drive_) {
			auto playlist = new QMediaPlaylist(player_);

			offsets_.reserve(drive_->playlist.clips.size());
			for (auto const& video : drive_->playlist.clips) {
				auto url = QUrl::fromLocalFile(
				    QString::fromStdString(video.filename.string()));

				offsets_.push_back(offset);
				offset += video.duration;

				playlist->addMedia(url);
			}

			player_->setPlaylist(playlist);
		}

		setPlayback(0, true);
		player_->play();
	}

	void QmlDrive::populateLines() {
		lines_.clear();
		lines_.reserve(int(drive_->trace.lines.size()));
		for (auto const& line : drive_->trace.lines) {
			QList<QGeoCoordinate> path;
			path.reserve(int(line.points.size()));
			for (auto const& pt : line.points)
				path.push_back({pt.lat.as_float(), pt.lon.as_float()});
			lines_.push_back(QVariant::fromValue(QGeoPath{path}));
		}
	}

	size_t QmlDrive::currentIndex() {
		if (!player_) return 0;
		auto playlist = player_->playlist();
		if (!playlist) return 0;
		return size_t(playlist->currentIndex());
	}

	travel_ms QmlDrive::playbackToTravel(playback_ms millis) const {
		if (drive_) {
			return drive_->playlist.playback_to_travel(millis);
		}
		return travel_ms{millis.time_since_epoch()};
	}

	local_milliseconds QmlDrive::travelToLocal(travel_ms travel_time) const {
		if (drive_) return drive_->travel_to_local(travel_time);

		// without the drive start time, any answer here is wrong; here's one of
		// them: 1970-01-01 00:00:00
		return {};
	}

	std::pair<QGeoCoordinate, track::speed> QmlDrive::travelToPosition(
	    mgps::travel_ms drive_millis) const {
		if (!drive_) return {};

		auto pt = drive_->trace.travel_to_position(drive_millis);
		return {{pt.lat.as_float(), pt.lon.as_float()}, pt.kmph};
	}
}  // namespace mgps::declarative
