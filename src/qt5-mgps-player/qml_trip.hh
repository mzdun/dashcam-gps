#pragma once

#include <QDate>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoRectangle>
#include <QObject>
#include <QUrl>
#include "mgps/library.hh"

namespace mGPS {
	class QmlTrip : public QObject {
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
		Q_PROPERTY(QVariantList segments READ segments NOTIFY segmentsChanged)
		Q_PROPERTY(QUrl clipSource READ clipSource NOTIFY tripChanged)
		Q_PROPERTY(int clipOffset READ clipOffset NOTIFY tripChanged)
	public:
		QmlTrip();

		static void qmlRegisterType(char const* url, int major, int minor);

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

		QUrl clipSource() const;
		int clipOffset() const;

		void setTrip(mgps::library::trip const* trip);

	signals:
		void tripChanged();
		void playbackChanged();
		void playbackStringChanged();
		void timelineChanged();
		void timelineStringChanged();
		void carPositionChanged();
		void removeSegments();
		void segmentsChanged();

	public slots:
		void setPlayback(unsigned long long milliseconds);

	private:
		struct jump {
			std::chrono::milliseconds playback;
			std::chrono::milliseconds fixup;
		};
		bool force_updates_{false};
		mgps::library::trip const* trip_{nullptr};
		std::chrono::milliseconds playback_{};
		std::chrono::milliseconds duration_{};
		std::chrono::milliseconds timeline_{};
		std::vector<jump> jumps_{};
		QGeoCoordinate car_position_{};
		QVariantList segments_{};
	};
}  // namespace mGPS
