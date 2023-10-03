#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkCookie>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "autoLoginManager.h"

void AutoLoginManager::setUser(QString username)
{
    m_username = username;
}

void AutoLoginManager::setPassword(QString password)
{
    m_password = password;
}

void AutoLoginManager::attemptAutoLogin()
{
    qDebug() << "Autologin: Getting CSRF token";
    connect(this, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));
    get(QNetworkRequest(m_loginUrl));
}

void AutoLoginManager::replyFinished(QNetworkReply* reply)
{
    qDebug() << "Autologin: Attempting login ";
//	    << m_username << " and password " << m_password;

    QNetworkCookieJar * cookieJar = this->cookieJar();

    const QList<QNetworkCookie> cookies = cookieJar->cookiesForUrl(m_loginUrl);

    QByteArray csrfToken;
    for (int i = 0; i < cookies.count(); ++i)
    {
        if (cookies.at(i).name().contains("XSRF-TOKEN"))
        {
            csrfToken = cookies.at(i).value();
        }
    }

    QNetworkRequest request(m_loginUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-XSRF-TOKEN", csrfToken);

    QByteArray data;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery params;
    params.addQueryItem("username", m_username);
    params.addQueryItem("password", m_password);
    data = params.query(QUrl::FullyEncoded).toUtf8();
#else
    QUrl params;
    params.addQueryItem("username", m_username);
    params.addQueryItem("password", m_password);
    data = params.encodedQuery();
#endif

    disconnect(this, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));
    connect(this, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyPostFinished(QNetworkReply*)));
    post(request, data);
}

void AutoLoginManager::replyPostFinished(QNetworkReply* reply)
{
    qDebug() << "AutoLogin: Post reply received";
    disconnect(this, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyPostFinished(QNetworkReply*)));
    emit autoLoggedIn();
}
