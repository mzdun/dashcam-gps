#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <cinttypes>

#include "declarative/QmlDrive.hh"
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

void load_library(std::string const& dirname, drive& current_drive) {
	using namespace std::literals;
	constexpr auto gap = 10min;

	mgps::mai::loader builder{};
	builder.add_directory(dirname);
	auto library = builder.build(gap);

	size_t clips{}, jumps{}, lines{}, points{};
	int skipped = 0;
	for (auto const& drive : library) {
		clips += drive.playlist.clips.size();
		jumps += drive.playlist.jumps.size();
		lines += drive.trace.lines.size();
		for (auto& line : drive.trace.lines) {
			points += line.points.size();
		}
		if (skipped < 2 && !drive.playlist.clips.empty() &&
		    drive.trace.has_points()) {
			current_drive = drive;
			++skipped;
		}
	}

	qDebug() << "Loaded" << pl{library.size(), "drive"} << "containig"
	         << pl{clips, "video clip"} << "and" << pl{lines, "route line"}
	         << "with" << pl{points, "point"};

	points = 0;
	for (auto& line : current_drive.trace.lines) {
		points += line.points.size();
	}
	qDebug() << "Using drive containig"
	         << pl{current_drive.playlist.clips.size(), "video clip"} << "and"
	         << pl{current_drive.trace.lines.size(), "route plot"} << "with"
	         << pl{points, "point"};
}

int main(int argc, char* argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	mgps::qmlRegisterTypes();

	QCommandLineParser parser;
	QCommandLineOption DCIM("dcim", "Set the location of directory with clips.",
	                        "directory");
	parser.setApplicationDescription("Player for GSM data from 70mai dashcams");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption(DCIM);
	parser.parse(app.arguments());

	drive current_drive{};
	if (parser.isSet(DCIM)) {
		load_library(parser.value(DCIM).toUtf8().toStdString(), current_drive);
	}

	mgps::declarative::QmlDrive drive{};
	drive.setDrive(&current_drive);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("currentDrive", &drive);
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
