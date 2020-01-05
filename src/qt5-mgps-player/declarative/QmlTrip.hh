#pragma once

#include <QDate>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoRectangle>
#include <QMediaPlayer>
#include <QUrl>
#include "QmlObject.hh"
#include "mgps/library.hh"

namespace mgps::declarative {
	class QmlTrip : public QmlObject {
		Q_OBJECT
		Q_PROPERTY(
		    QGeoRectangle visibleRegion READ visibleRegion NOTIFY tripChanged)
		Q_PROPERTY(long long playback READ playback WRITE setPlayback NOTIFY
		               playbackChanged)
		Q_PROPERTY(QString playbackString READ playbackString NOTIFY
		               playbackStringChanged)
		Q_PROPERTY(long long duration READ duration NOTIFY tripChanged)
		Q_PROPERTY(
		    QString durationString READ durationString NOTIFY tripChanged)
		Q_PROPERTY(QDateTime timeline READ timeline NOTIFY timelineChanged)
		Q_PROPERTY(QString timelineString READ timelineString NOTIFY
		               timelineStringChanged)
		Q_PROPERTY(QGeoCoordinate carPosition READ carPosition NOTIFY
		               carPositionChanged)
		Q_PROPERTY(QVariantList segments READ segments NOTIFY tripChanged)
	public:
		QmlTrip();

		void setTrip(mgps::library::trip const* trip);

		Q_INVOKABLE void playerAvailable(QObject*);

		void setPlayback(unsigned long long milliseconds, bool forced = false);

		QGeoRectangle visibleRegion() const noexcept;
		long long playback() const noexcept { return playback_.count(); }
		long long duration() const noexcept { return duration_.count(); }
		QDateTime timeline() const noexcept;
		QGeoCoordinate const& carPosition() const noexcept {
			return car_position_;
		}
		QVariantList const& segments() const noexcept { return segments_; }

		QString playbackString() const;
		QString durationString() const;
		QString timelineString() const;

	signals:
		void tripChanged();
		void playbackChanged();
		void playbackStringChanged();
		void timelineChanged();
		void timelineStringChanged();
		void carPositionChanged();

	private:
		using milliseconds = std::chrono::milliseconds;
		void populatePlaylist();
		void populateJumpsAndDuration();
		void populatePlots();
		size_t currentIndex();
		milliseconds playbackToTimeline(milliseconds) const;
		QGeoCoordinate positionForTimeline(milliseconds) const;

		std::chrono::milliseconds getOffset(size_t currentFile) {
			if (currentFile >= offsets_.size()) {
				if (offsets_.empty()) return {};
				return offsets_.back();
			}
			return offsets_[currentFile];
		}

		struct jump {
			std::chrono::milliseconds playback;
			std::chrono::milliseconds fixup;
		};

		mgps::library::trip const* trip_{nullptr};
		std::chrono::milliseconds playback_{};
		std::chrono::milliseconds duration_{};
		std::chrono::milliseconds timeline_{};
		std::vector<jump> jumps_{};
		QGeoCoordinate car_position_{};
		QVariantList segments_{};
		std::vector<std::chrono::milliseconds> offsets_{};
		QMediaPlayer* player_{nullptr};
	};
}  // namespace mgps::declarative

Q_DECLARE_METATYPE(mgps::declarative::QmlTrip);
