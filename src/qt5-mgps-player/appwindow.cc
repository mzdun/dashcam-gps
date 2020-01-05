#include "appwindow.hh"

AppWindow::AppWindow(QQuickWindow* parent) : QQuickWindow(parent) {}

void AppWindow::setIconSource(QString const& icon) {
	if (iconSource_ != icon) {
		iconSource_ = icon;
		QQuickWindow::setIcon(QIcon(iconSource_));
		emit iconSourceChanged();
	}
}
