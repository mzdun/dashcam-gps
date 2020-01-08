#include "QmlAppWindow.hh"

namespace mgps::declarative {
	QmlAppWindow::QmlAppWindow(QQuickWindow* parent) : QQuickWindow(parent) {}

	void QmlAppWindow::setIconSource(QString const& icon) {
		if (iconSource_ != icon) {
			iconSource_ = icon;
			QQuickWindow::setIcon(QIcon(iconSource_));
			emit iconSourceChanged();
		}
	}
}  // namespace mgps::declarative
