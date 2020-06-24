#ifndef EPADINTERFACE_H
#define EPADINTERFACE_H

#include <QtDBus/QtDBus>

//
// EPAD Proxy
//
class ComExorEpadInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
	static inline const char *staticInterfaceName() { return "com.exor.EPAD"; }

	ComExorEpadInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	QDBusPendingReply<int> mouseStatus(const QString &display);

Q_SIGNALS: // SIGNALS
	void mousePlugEvent(int bPlugged);
};

//
// EPAD-based authenticator
//
class ComExorSecurityInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
	static inline const char *staticInterfaceName() { return "com.exor.EPAD.Security"; }

	ComExorSecurityInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	QDBusPendingReply<int> checkPassword(const QString &userName, const QString &password);
};

//
// EPAD Backlight Interface
//
class ComExorBacklightInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
	static inline const char *staticInterfaceName() { return "com.exor.EPAD.Backlight"; }

	ComExorBacklightInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	QDBusPendingReply<int> state(const QString &display);

Q_SIGNALS: // SIGNALS
	void stateChanged(const QString &displayName, int state);
};

#endif // EPADINTERFACE_H
