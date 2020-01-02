#include <QCommandLineParser>
#include <QDebug>
#include <QGeoCoordinate>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <cinttypes>

#include "mgps-70mai/loader.hh"
#include "mgps/library.hh"

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

QVariant variant(point const& pt) {
	return QVariant::fromValue(QGeoCoordinate{pt.lat.as_float(), pt.lon.as_float()});
}

void load_library(std::string const& dirname,
                  trip& current_trip,
                  point& center) {
	using namespace std::literals;
	constexpr auto gap = 10min;

	mgps::library::mai::loader builder{};
	builder.add_directory(dirname);
	auto library = builder.build(gap);

	size_t strides{}, clips{}, segments{}, points{};
	bool no_trip = true;
	bool no_center = true;
	for (auto const& trip : library) {
		strides += trip.strides.size();
		segments += trip.plot.segments.size();
		for (auto& stride : trip.strides) {
			clips += stride.clips.size();
		}
		for (auto& segment : trip.plot.segments) {
			points += segment.points.size();
			if (no_center && !segment.points.empty()) {
				no_center = false;
				center = segment.points.front();
			}
		}
		if (!trip.strides.empty() && !trip.plot.segments.empty())
			current_trip = trip;
	}

	qDebug() << "Loaded" << pl{library.size(), "trip"} << "containig"
	         << pl{strides, "movie strip"} << "with"
	         << pl{clips, "individual clip"} << "and"
	         << pl{segments, "route plot"} << "with" << pl{points, "point"};
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
	point center{{52'06'780'330ull, NESW::N}, {19'48'010'210ull, NESW::E}};
	if (parser.isSet(DCIM)) {
		load_library(parser.value(DCIM).toUtf8().toStdString(), current_trip,
		             center);
	}

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("centerPoint", variant(center));
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
