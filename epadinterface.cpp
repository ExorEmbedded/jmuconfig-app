#include "epadinterface.h"

//
// com.exor.EPAD
//
ComExorEpadInterface::ComExorEpadInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
	    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

QDBusPendingReply<int> ComExorEpadInterface::mouseStatus(const QString &display)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(display);
    return asyncCallWithArgumentList(QLatin1String("mouseStatus"), argumentList);
}

//
// com.exor.EPAD.Security
//
ComExorSecurityInterface::ComExorSecurityInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
	    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

QDBusPendingReply<int> ComExorSecurityInterface::checkPassword(const QString &userName, const QString &password)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(userName) << QVariant::fromValue(password);
    return asyncCallWithArgumentList(QLatin1String("checkPassword"), argumentList);
}

//
// com.exor.EPAD.Backlight
//
ComExorBacklightInterface::ComExorBacklightInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
	    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

QDBusPendingReply<int> ComExorBacklightInterface::state(const QString &display)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(display);
    return asyncCallWithArgumentList(QLatin1String("state"), argumentList);
}
