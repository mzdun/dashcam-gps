#include "QmlTrip.hh"

#include <QMediaPlaylist>
#include <QQmlEngine>
#include <mgps/library.hh>

namespace mgps::declarative {
	using namespace std::chrono;
	using namespace date;

	QmlTrip::QmlTrip() {}

	void QmlTrip::setTrip(mgps::trip const* trip) {
		trip_ = trip;

		if (trip_) {
			duration_ = trip_->playlist.duration;
			populateLines();
		} else {
			duration_ = {};
			lines_.clear();
		}

		populatePlaylist();

		emit tripChanged();
	}

	/*********************************************************************
	 *  CALLABLES
	 */

	void QmlTrip::playerAvailable(QObject* qmlMediaPlayer) {
		auto player = qvariant_cast<QMediaPlayer*>(
		    qmlMediaPlayer->property("mediaObject"));
		player_ = player;
		populatePlaylist();
	}

	/*********************************************************************
	 *  SETTERS FOR PROPERTIES
	 */

	void QmlTrip::setPlayback(unsigned long long new_millis, bool forced) {
		auto const millis =
		    milliseconds{new_millis} + getOffset(currentIndex());
		if (!forced && millis == playback_) return;

		auto const old_playback = floor<seconds>(playback_);

		playback_ = millis;

		auto const [new_position, new_speed] = travelToPosition(millis);
		auto const emit_position = new_position != car_position_;
		car_position_ = new_position;
		auto const emit_speed = new_speed != car_speed_;
		car_speed_ = new_speed;

		emit playbackChanged();
		if (forced || floor<seconds>(playback_) != old_playback)
			emit playbackStringChanged();
		if (forced || emit_position) emit carPositionChanged();
		if (forced || emit_speed) emit carSpeedChanged();
	}

	/*********************************************************************
	 *  GETTERS FOR PROPERTIES
	 */

	QGeoRectangle QmlTrip::visibleRegion() const noexcept {
		if (!trip_) return QGeoRectangle{};
		static constexpr auto MARGIN = 0.01;
		auto const bounds = trip_->trace.boundary_box();
		auto const center = bounds.center();
		auto const width = bounds.width().as_float() + 2 * MARGIN;
		auto const height = bounds.height().as_float() + 2 * MARGIN;
		return {QGeoCoordinate{center.lat.as_float(), center.lon.as_float()},
		        width, height};
	}

	QString QmlTrip::playbackString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(playback_).time_since_epoch()};
		return QString::fromStdString(ostr.str());
	}

	QString QmlTrip::durationString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(duration_)};
		return QString::fromStdString(ostr.str());
	}

	/*********************************************************************
	 *  HELPERS
	 */

	void QmlTrip::populatePlaylist() {
		if (!player_) return;
		player_->stop();

		using namespace std::chrono;
		auto offset = playback_ms{};

		if (trip_) {
			auto playlist = new QMediaPlaylist(player_);

			offsets_.reserve(trip_->playlist.media.size());
			for (auto const& video : trip_->playlist.media) {
				auto file = trip_->footage(video);
				auto url = QUrl::fromLocalFile(
				    QString::fromStdString(file->filename));

				offsets_.push_back(offset);
				offset += video.duration;

				playlist->addMedia(url);
			}

			player_->setPlaylist(playlist);
		}

		setPlayback(0, true);
		player_->play();
	}

	void QmlTrip::populateLines() {
		lines_.clear();
		lines_.reserve(int(trip_->trace.lines.size()));
		for (auto const& line : trip_->trace.lines) {
			QList<QGeoCoordinate> path;
			path.reserve(int(line.points.size()));
			for (auto const& pt : line.points)
				path.push_back({pt.lat.as_float(), pt.lon.as_float()});
			lines_.push_back(QVariant::fromValue(QGeoPath{path}));
		}
	}

	size_t QmlTrip::currentIndex() {
		if (!player_) return 0;
		auto playlist = player_->playlist();
		if (!playlist) return 0;
		return size_t(playlist->currentIndex());
	}

	local_ms QmlTrip::playbackToLocal(playback_ms millis) const {
		if (trip_) return trip_->travel_to_local(millis);

		// without the trip start time, any answer here is wrong; here's one of
		// them: 1970-01-01 00:00:00
		return {};
	}

	std::pair<QGeoCoordinate, track::speed> QmlTrip::travelToPosition(
	    playback_ms trip_millis) const {
		if (!trip_) return {};

		auto pt = trip_->trace.playback_to_position(trip_millis);
		return {{pt.lat.as_float(), pt.lon.as_float()}, pt.kmph};
	}
}  // namespace mgps::declarative
