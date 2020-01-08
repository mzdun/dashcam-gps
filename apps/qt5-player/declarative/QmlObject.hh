#pragma once
#include <QObject>

namespace mgps::declarative {
    class QmlObject : public QObject {
		Q_OBJECT
	public:
		QmlObject(QObject* parent = nullptr) : QObject(parent) {}
		QmlObject(QmlObject const&) {}
		QmlObject& operator=(QmlObject const&) { return *this; }
		QmlObject(QmlObject&&) {}
		QmlObject& operator=(QmlObject&&) { return *this; }
	};
}

Q_DECLARE_METATYPE(mgps::declarative::QmlObject);
