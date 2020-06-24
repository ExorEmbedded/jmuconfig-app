#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QNetworkCookieJar>
#include <QSettings>

/** 
 * Lightweight Cookie Jar implementation - intended for local usage only.
 */
class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT

    public:
        explicit CookieJar (QObject *parent = 0);
        virtual ~CookieJar ();

    protected:
        QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
        bool setCookiesFromUrl (const QList<QNetworkCookie> &cookieList, const QUrl &url);
        void save () const;
        void load ();

        QSettings *m_settings;
};

#endif  // COOKIEJAR_H
