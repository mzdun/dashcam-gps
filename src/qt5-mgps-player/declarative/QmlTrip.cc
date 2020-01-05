#include "QmlTrip.hh"

#include <QMediaPlaylist>
#include <QQmlEngine>

namespace mgps::declarative {
	using namespace std::chrono;
	using namespace date;

	QmlTrip::QmlTrip() {}

	void QmlTrip::setTrip(mgps::library::trip const* trip) {
		trip_ = trip;

		if (trip_) {
			populateJumpsAndDuration();
			populatePlots();
		} else {
			duration_ = {};
			jumps_.clear();
			segments_.clear();
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
		auto const old_timeline = floor<seconds>(timeline_);

		playback_ = millis;

		milliseconds new_timeline = playbackToTimeline(millis);
		auto const emit_timeline = new_timeline != timeline_;
		timeline_ = new_timeline;

		auto const new_position = positionForTimeline(new_timeline);
		auto const emit_position = new_position != car_position_;
		car_position_ = new_position;

		emit playbackChanged();
		if (forced || floor<seconds>(playback_) != old_playback)
			emit playbackStringChanged();
		if (forced || emit_timeline) emit timelineChanged();
		if (forced || floor<seconds>(timeline_) != old_timeline)
			emit timelineStringChanged();
		if (forced || emit_position) emit carPositionChanged();
	}

	/*********************************************************************
	 *  GETTERS FOR PROPERTIES
	 */

	QGeoRectangle QmlTrip::visibleRegion() const noexcept {
		if (!trip_) return QGeoRectangle{};
		static constexpr auto MARGIN = 0.01;
		auto const bounds = trip_->plot.boundary_box();
		auto const center = bounds.center();
		auto const width = bounds.width().as_float() + 2 * MARGIN;
		auto const height = bounds.height().as_float() + 2 * MARGIN;
		return {QGeoCoordinate{center.lat.as_float(), center.lon.as_float()},
		        width, height};
	}

	QDateTime QmlTrip::timeline() const noexcept {
		auto start = [](auto* jump) {
			return jump ? jump->start : local_milliseconds{};
		};
		auto const secs = floor<seconds>(start(trip_) + timeline_);
		auto const date = floor<days>(secs);
		auto const time = secs - date;

		auto const ymd = year_month_day{local_days{date}};
		auto const hms = hh_mm_ss{time};
		return {QDate{int(ymd.year()), int(unsigned(ymd.month())),
		              int(unsigned(ymd.day()))},
		        QTime{int(hms.hours().count()), int(hms.minutes().count()),
		              int(hms.seconds().count())}};
	}

	QString QmlTrip::playbackString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(playback_)};
		return QString::fromStdString(ostr.str());
	}

	QString QmlTrip::durationString() const {
		std::ostringstream ostr;
		ostr << hh_mm_ss{floor<seconds>(duration_)};
		return QString::fromStdString(ostr.str());
	}

	QString QmlTrip::timelineString() const {
		auto const start = [&] {
			if (!trip_) return local_milliseconds{};
			return trip_->start;
		}();
		std::ostringstream ostr;
		ostr << floor<seconds>(start + timeline_);
		return QString::fromStdString(ostr.str());
	}

	/*********************************************************************
	 *  HELPERS
	 */

	void QmlTrip::populatePlaylist() {
		if (!player_) return;
		player_->stop();

		using namespace std::chrono;
		auto offset = milliseconds{0};

		size_t count{};
		if (trip_) {
			for (auto const& stride : trip_->strides) {
				count += stride.clips.size();
			}
			offsets_.reserve(count);

			auto playlist = new QMediaPlaylist(player_);
			for (auto const& stride : trip_->strides) {
				for (auto const& film : stride.clips) {
					auto url = QUrl::fromLocalFile(
					    QString::fromStdString(film.filename.string()));

					offsets_.push_back(offset);
					offset += film.duration;

					playlist->addMedia(url);
				}
			}
			player_->setPlaylist(playlist);
		}

		setPlayback(0, true);
		player_->play();
	}

	void QmlTrip::populateJumpsAndDuration() {
		milliseconds timeline{};
		duration_ = {};

		jumps_.clear();
		jumps_.reserve(trip_->strides.size());
		for (auto const& slice : trip_->strides) {
			if (timeline != slice.offset) {
				jumps_.push_back({duration_, slice.offset - duration_});
			}
			duration_ += slice.duration;
			timeline = slice.offset + slice.duration;
		}
	}

	void QmlTrip::populatePlots() {
		segments_.clear();
		segments_.reserve(int(trip_->plot.segments.size()));
		for (auto const& seg : trip_->plot.segments) {
			QList<QGeoCoordinate> path;
			path.reserve(int(seg.points.size()));
			for (auto const& pt : seg.points)
				path.push_back({pt.lat.as_float(), pt.lon.as_float()});
			segments_.push_back(QVariant::fromValue(QGeoPath{path}));
		}
	}

	size_t QmlTrip::currentIndex() {
		if (!player_) return 0;
		auto playlist = player_->playlist();
		if (!playlist) return 0;
		return size_t(playlist->currentIndex());
	}

	milliseconds QmlTrip::playbackToTimeline(milliseconds millis) const {
		if (!jumps_.empty()) {
			auto it =
			    std::upper_bound(begin(jumps_), end(jumps_), jump{millis, {}},
			                     [](auto const& lhs, auto const& rhs) {
				                     return lhs.playback < rhs.playback;
			                     });
			if (it != begin(jumps_)) {
				return millis + std::prev(it)->fixup;
			}
		}
		return millis;
	}

	QGeoCoordinate QmlTrip::positionForTimeline(
	    milliseconds trip_millis) const {
		if (!trip_) return {};

		// TODO: support trips w/out GPS
		auto it_seg = std::upper_bound(begin(trip_->plot.segments),
		                               end(trip_->plot.segments),
		                               library::timeline_item{trip_millis, {}},
		                               [](auto const& lhs, auto const& rhs) {
			                               return lhs.offset < rhs.offset;
		                               });

		if (it_seg != begin(trip_->plot.segments)) std::advance(it_seg, -1);
		if (it_seg == end(trip_->plot.segments)) return {};

		auto const& seg = *it_seg;
		auto const time = [=, offset = seg.offset]() {
			library::track::gps_data out{};
			out.time = trip_millis - offset;
			return out;
		}();

		auto it = std::upper_bound(begin(seg.points), end(seg.points), time,
		                           [](auto const& lhs, auto const& rhs) {
			                           return lhs.time < rhs.time;
		                           });

		if (it != begin(seg.points)) std::advance(it, -1);
		if (it == end(seg.points)) return {};

		auto const& pt = *it;
		return {pt.lat.as_float(), pt.lon.as_float()};
	}
}  // namespace mgps::declarative
