#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <cinttypes>
#include <mgps/version.hh>
#include "declarative/QmlTrip.hh"
#include "declarative/declarative.hh"
#include "mgps-70mai/loader.hh"

using namespace mgps;
using namespace mgps::track;

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

	mgps::mai::loader builder{};
	builder.add_directory(dirname);
	auto library = builder.build(gap);

	size_t clips{}, jumps{}, lines{}, points{};
	int skipped = 0;
	for (auto const& trip : library) {
		clips += trip.playlist.clips.size();
		jumps += trip.playlist.jumps.size();
		lines += trip.trace.lines.size();
		for (auto& line : trip.trace.lines) {
			points += line.points.size();
		}
		if (skipped < 2 && !trip.playlist.clips.empty() &&
		    trip.trace.has_points()) {
			current_trip = trip;
			++skipped;
		}
	}

	qDebug() << "Loaded" << pl{library.size(), "trip"} << "containig"
	         << pl{clips, "video clip"} << "and" << pl{lines, "route line"}
	         << "with" << pl{points, "point"};

	points = 0;
	for (auto& line : current_trip.trace.lines) {
		points += line.points.size();
	}
	qDebug() << "Using trip containig"
	         << pl{current_trip.playlist.clips.size(), "video clip"} << "and"
	         << pl{current_trip.trace.lines.size(), "route plot"} << "with"
	         << pl{points, "point"};
}

int main(int argc, char* argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);
	app.setOrganizationName("midnightBITS");
	app.setOrganizationDomain("midnightbits.com");
	app.setApplicationName("Dashcam GPS Viewer");
	app.setApplicationVersion(mgps::version::string_ui);

	mgps::qmlRegisterTypes();

	QCommandLineParser parser;
	QCommandLineOption DCIM("dcim", "Set the location of directory with clips.",
	                        "directory");
	parser.setApplicationDescription("Player for GSM data from 70mai dashcams");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption(DCIM);
	parser.parse(app.arguments());

	if (parser.isSet(DCIM)) {
		QSettings settings;
		settings.beginGroup(mgps::version::string_short);
		settings.setValue("library", parser.value(DCIM));
	}

	trip current_trip{};

	{
		QSettings settings;
		settings.beginGroup(mgps::version::string_short);
		auto dir = settings.value("library").toString();
		if (!dir.isEmpty()) {
			load_library(parser.value(DCIM).toUtf8().toStdString(),
			             current_trip);
		}
	}

	mgps::declarative::QmlTrip trip{};
	trip.setTrip(&current_trip);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("currentTrip", &trip);
	const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
	QObject::connect(
	    &engine, &QQmlApplicationEngine::objectCreated, &app,
	    [url](QObject* obj, const QUrl& objUrl) {
		    if (!obj && url == objUrl) QCoreApplication::exit(-1);
	    },
	    Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}
