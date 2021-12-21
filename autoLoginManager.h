#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "browsersettings.h"
#include "cookiejar.h"

class AutoLoginManager : public QNetworkAccessManager
{
	Q_OBJECT

public:
	void attemptAutoLogin();
	void setUser(QString username);
	void setPassword(QString password);

signals:
	void autoLoggedIn();

protected slots:
	void replyFinished(QNetworkReply* reply);
	void replyPostFinished(QNetworkReply* reply);

private:
	const QUrl m_loginUrl = QUrl("http://localhost:8080/restsession/api/v1/authentication/login");
	QString m_username;
	QString m_password;
};

