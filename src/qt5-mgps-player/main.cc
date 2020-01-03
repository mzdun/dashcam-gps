#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <cinttypes>

#include "mgps-70mai/loader.hh"
#include "qml_trip.hh"

using namespace mgps::library;
using namespace mgps::library::track;

template <typename Counter>
struct pl {
	Counter count;
	const char* label;
	pl(Counter count, const char* label) : count{count}, label{label} {}
};

template <typename Counter>
inline QDebug operator<<(QDebug dbg, pl<Counter> val) {
	dbg.nospace() << val.count << ' ' << val.label;
	if (val.count != 1) dbg << 's';
	return dbg.space();
}

void load_library(std::string const& dirname, trip& current_trip) {
	using namespace std::literals;
	constexpr auto gap = 10min;

	mgps::library::mai::loader builder{};
	builder.add_directory(dirname);
	auto library = builder.build(gap);

	size_t strides{}, clips{}, segments{}, points{};
	bool no_trip = true;
	for (auto const& trip : library) {
		strides += trip.strides.size();
		segments += trip.plot.segments.size();
		for (auto& stride : trip.strides) {
			clips += stride.clips.size();
		}
		for (auto& segment : trip.plot.segments) {
			points += segment.points.size();
		}
		if (no_trip && !trip.strides.empty() && trip.plot.has_points()) {
			current_trip = trip;
			no_trip = false;
		}
	}

	qDebug() << "Loaded" << pl{library.size(), "trip"} << "containig"
	         << pl{strides, "movie strip"} << "with"
	         << pl{clips, "individual clip"} << "and"
	         << pl{segments, "route plot"} << "with" << pl{points, "point"};

	clips = points = 0;
	for (auto& stride : current_trip.strides) {
		clips += stride.clips.size();
	}
	for (auto& segment : current_trip.plot.segments) {
		points += segment.points.size();
	}
	qDebug() << "Using trip containig"
	         << pl{current_trip.strides.size(), "movie strip"} << "with"
	         << pl{clips, "individual clip"} << "and"
	         << pl{current_trip.plot.segments.size(), "route plot"} << "with"
	         << pl{points, "point"};
}

int main(int argc, char* argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	QCommandLineParser parser;
	QCommandLineOption DCIM("dcim", "Set the location of directory with clips.",
	                        "directory");
	parser.setApplicationDescription("Player for GSM data from 70mai dashcams");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption(DCIM);
	parser.parse(app.arguments());

	trip current_trip;
	if (parser.isSet(DCIM)) {
		load_library(parser.value(DCIM).toUtf8().toStdString(), current_trip);
	}

	QmlTrip trip{};
	trip.setTrip(&current_trip);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("trip", &trip);
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(
	    &engine, &QQmlApplicationEngine::objectCreated, &app,
	    [url](QObject* obj, const QUrl& objUrl) {
		    if (!obj && url == objUrl) QCoreApplication::exit(-1);
	    },
	    Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
