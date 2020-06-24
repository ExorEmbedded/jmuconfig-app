#pragma once

#include <QtGlobal>
#if QT_VERSION < 0x050000

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QTimer>

/*
 * Proxy class for interface com.exor.EPAD
 */
class ComExorVKeyboardInterface: public QDBusAbstractInterface
{
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return "com.exor.VKeyboard"; }

public:
	ComExorVKeyboardInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~ComExorVKeyboardInterface();

public slots:
	void show(const QString& layout, int x=0, int y=0, int width=0, int height=0)
	{
		QList<QVariant> argumentList;
		argumentList << QVariant::fromValue(layout)
					 << QVariant::fromValue(x)
					 << QVariant::fromValue(y)
					 << QVariant::fromValue(width)
					 << QVariant::fromValue(height);
		callWithArgumentList(QDBus::Block, QLatin1String("show"), argumentList);
	}

	void showFullScreen(const QString& layout)
	{
		QList<QVariant> argumentList;
		argumentList << QVariant::fromValue(layout);
		callWithArgumentList(QDBus::Block, QLatin1String("showFullScreen"), argumentList);
	}

	void showX11FullScreen(const QString& layout, int x, int y, int width, int height)
	{
		QList<QVariant> argumentList;
		argumentList << QVariant::fromValue(layout)
					 << QVariant::fromValue(x)
					 << QVariant::fromValue(y)
					 << QVariant::fromValue(width)
					 << QVariant::fromValue(height);
		callWithArgumentList(QDBus::Block, QLatin1String("showX11FullScreen"), argumentList);
	}
	
	void hide()
	{
		QList<QVariant> argumentList;
		callWithArgumentList(QDBus::Block, QLatin1String("hide"), argumentList);
	}

signals:
	void keyPressed(int character);
	void keyReleased(int character);
	void characterGenerated(int character);
	void closed();
};

/*!
 * \brief The InputPanelProxy class is a dbus proxy for the system virtual keyboard
 */
class InputPanelProxy : public QObject
{
    Q_OBJECT

public:
	InputPanelProxy(QObject* parent);
	~InputPanelProxy();

	QDBusError lastError();

public slots:
	void show(const QString& layout, int x, int y, int width, int height);
	void showFullScreen(const QString& layout);
	void showX11FullScreen(const QString& layout, int x, int y, int width, int height);
	void hide();

private slots:
    void connectRetry();
	/*! Supports lazy dbus connection for faster boot up */
	void init();

signals:
	void keyPressed(int character);
	void keyReleased(int character);
	void closed();
//	void characterGenerated(int character);

protected:
	class ComExorVKeyboardInterface* m_dbusObject;
	class QDBusConnection* m_vkeyboardConnection;
    QTimer m_connectRetryTimer;
};

#endif
