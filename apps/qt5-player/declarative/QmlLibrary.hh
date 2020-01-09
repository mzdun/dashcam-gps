#ifndef QMLLIBRARY_HH
#define QMLLIBRARY_HH

#include <QObject>

class QmlLibrary : public QObject
{
	Q_OBJECT
public:
	explicit QmlLibrary(QObject *parent = nullptr);

signals:

public slots:
};

#endif // QMLLIBRARY_HH
