#include <QDebug>
#include <QNetworkCookie>
#include <QStringList>
#include "cookiejar.h"

CookieJar::CookieJar (QObject *parent) :
    QNetworkCookieJar(parent)
{
    m_settings = new QSettings("Exor", "fancybrowser-cookies");

    if (m_settings == NULL)
        return;

    load();
}

CookieJar::~CookieJar ()
{
    if (m_settings)
        delete m_settings;
}

/* Retrieve cookies for a given URL */
QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    Q_UNUSED(url);

    return QNetworkCookieJar::allCookies();
}

/* Save cookies from a given URL */
bool CookieJar::setCookiesFromUrl (const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    Q_UNUSED(url);

    m_settings->remove("cookies");
    m_settings->beginGroup("cookies");

    Q_FOREACH (QNetworkCookie cookie, cookieList)
    {
        qDebug() << qPrintable(QString("Setting cookie: (%1=%2)")
            .arg(QString(cookie.name()))
            .arg(QString(cookie.value())));

        if (!cookie.value().isEmpty())
                m_settings->setValue(cookie.name(), cookie.value());
    }

    m_settings->endGroup();
    m_settings->sync();

    return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

/* Save all cookies - currently unused because we have no reliable exit hook (quit crashes) */
void CookieJar::save () const
{
    if (m_settings == NULL)
        return;

    qDebug() << "Saving cookie jar to " << m_settings->fileName();

    m_settings->beginGroup("cookies");

    Q_FOREACH (QNetworkCookie cookie, allCookies())
        m_settings->setValue(cookie.name(), cookie.value());

    m_settings->endGroup();
}

/* Load all cookies */
void CookieJar::load ()
{
    if (m_settings == NULL)
        return;

    QList<QNetworkCookie> cookies;

    qDebug() << "Loading cookies from jar: " << m_settings->fileName();

    m_settings->beginGroup("cookies");

    Q_FOREACH (QString key, m_settings->allKeys())
    {
        QNetworkCookie cookie;

        cookie.setName(key.toUtf8());
        cookie.setValue(m_settings->value(key).toString().toUtf8());

        qDebug() << qPrintable(QString("Loading cookie: (%1=%2)")
            .arg(QString(cookie.name()))
            .arg(QString(cookie.value())));

        cookies += cookie;
    }
    
    m_settings->endGroup();

    setAllCookies(cookies);
}
