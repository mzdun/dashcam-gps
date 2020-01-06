#include "declarative.hh"

#include <QQmlEngine>

#include "QmlAppWindow.hh"
#include "QmlDrive.hh"

namespace mgps {
	namespace {
		template <typename QmlType>
		void qmlRegisterType(const char* name) {
			::qmlRegisterType<QmlType>("mGPS", 1, 0, name);
		}
	}  // namespace

	void qmlRegisterTypes() {
		using namespace declarative;
#define REGISTER(NAME) qmlRegisterType<Qml##NAME>(#NAME)
		REGISTER(Drive);
		REGISTER(AppWindow);
#undef REGISTER
	}
}  // namespace mgps
