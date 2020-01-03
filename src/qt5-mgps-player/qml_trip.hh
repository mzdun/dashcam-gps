#pragma once

#include <QDate>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QObject>
#include "mgps/library.hh"

class QmlTrip : public QObject {
	Q_OBJECT
	Q_PROPERTY(
	    QGeoRectangle visibleRegion READ visibleRegion NOTIFY tripChanged)
	Q_PROPERTY(long long playback READ playback WRITE setPlayback NOTIFY
	               playbackChanged)
	Q_PROPERTY(
	    QString playbackString READ playbackString NOTIFY playbackStringChanged)
	Q_PROPERTY(long long duration READ duration NOTIFY tripChanged)
	Q_PROPERTY(QString durationString READ durationString NOTIFY tripChanged)
	Q_PROPERTY(QDateTime timeline READ timeline NOTIFY timelineChanged)
	Q_PROPERTY(
	    QString timelineString READ timelineString NOTIFY timelineStringChanged)
	Q_PROPERTY(QGeoCoordinate position READ position NOTIFY positionChanged)
public:
	QmlTrip();

	QGeoRectangle visibleRegion() const noexcept;
	long long playback() const noexcept { return playback_.count(); }
	long long duration() const noexcept { return duration_.count(); }
	QDateTime timeline() const noexcept;
	QGeoCoordinate const& position() const noexcept { return car_position_; }

	QString playbackString() const;
	QString durationString() const;
	QString timelineString() const;

	void setTrip(mgps::library::trip const* trip);

signals:
	void tripChanged();
	void playbackChanged();
	void playbackStringChanged();
	void timelineChanged();
	void timelineStringChanged();
	void positionChanged();

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
};
