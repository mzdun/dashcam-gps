#pragma once

#include <QIcon>
#include <QQuickWindow>

class AppWindow : public QQuickWindow {
	Q_OBJECT

	Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY
	               iconSourceChanged)
public:
	explicit AppWindow(QQuickWindow* parent = nullptr);

	inline QString const& iconSource() { return iconSource_; }
	void setIconSource(QString const& icon);

signals:
	void iconSourceChanged();

private:
	QString iconSource_;
};
