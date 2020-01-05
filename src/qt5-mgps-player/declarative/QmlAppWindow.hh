#pragma once

#include <QIcon>
#include <QQuickWindow>

namespace mgps::declarative {
	class QmlAppWindow : public QQuickWindow {
		Q_OBJECT

		Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY
		               iconSourceChanged)
	public:
		explicit QmlAppWindow(QQuickWindow* parent = nullptr);

		inline QString const& iconSource() { return iconSource_; }
		void setIconSource(QString const& icon);

	signals:
		void iconSourceChanged();

	private:
		QString iconSource_;
	};
}  // namespace mgps::declarative
