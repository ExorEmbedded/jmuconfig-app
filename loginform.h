#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
	Q_OBJECT
	
public:
	explicit LoginForm(QWidget *parent = 0);
	~LoginForm();
	
	QString username() const;
	QString password() const;
	
	bool isAccepted() const { return m_accepted; }
signals:
	void accepted();
	void rejected();
	
protected:
	void showEvent(QShowEvent *);
	void hideEvent(QHideEvent *);
	bool eventFilter(QObject *, QEvent *);
	
private slots:
	void on_loginButtons_accepted();
	
	void on_loginButtons_rejected();
	
	void on_usernameEdit_returnPressed();
	
	void on_passwordEdit_returnPressed();
	
private:
	Ui::LoginForm *ui;
	bool m_accepted;
};

#endif // LOGINFORM_H
