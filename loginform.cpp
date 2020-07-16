#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LoginForm)
{
	m_accepted = false;
	ui->setupUi(this);
	setVisible(false);
	ui->loginButtons->setStyleSheet("");
}

LoginForm::~LoginForm()
{
	delete ui;
}

QString LoginForm::username() const
{
	return ui->usernameEdit->text();
}

QString LoginForm::password() const
{
	return ui->passwordEdit->text();
}


void LoginForm::showEvent(QShowEvent* s)
{
	m_accepted = false;
	if (parentWidget()) {
		parentWidget()->installEventFilter(this);
		setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
	}
	QWidget::showEvent(s);

	ui->usernameEdit->setFocus();

#if QT_VERSION >= 0x050000
	QApplication *app = dynamic_cast<QApplication*>(QApplication::instance());
	if (app)
		app->inputMethod()->show();
#endif

	setEnabled(true);
}

void LoginForm::hideEvent(QHideEvent* s)
{
	if (parentWidget())
		parentWidget()->removeEventFilter(this);
	QWidget::hideEvent(s);

	ui->usernameEdit->setText("");
	ui->passwordEdit->setText("");
	ui->errMessage->setText("");
}

bool LoginForm::eventFilter(QObject *, QEvent * e)
{
	if (e->type() == QEvent::Resize)
		if (parentWidget()) {
			setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
		}
	return false;
}

void LoginForm::setEnabled(bool enabled) {
	ui->loginButtons->setEnabled(enabled);
	ui->usernameEdit->setEnabled(enabled);
	ui->passwordEdit->setEnabled(enabled);
}

void LoginForm::accept(bool accept) {
	m_accepted = accept;
	if (m_accepted) {
		ui->errMessage->setText("");
		setEnabled(false);
		// This delay is just for visual feedback ( error message is cleared and will be eventally set again )
		QTimer::singleShot(500, this, SIGNAL(accepted()));
	} else
		emit rejected();
}

void LoginForm::on_loginButtons_accepted()
{
	accept(true);
}

void LoginForm::on_loginButtons_rejected()
{
	accept(false);
}

void LoginForm::on_usernameEdit_returnPressed()
{
	accept(true);
}

void LoginForm::on_passwordEdit_returnPressed()
{
	accept(true);
}

void LoginForm::reject(const QString& err)
{
	ui->errMessage->setText(err.isEmpty() ? "Authentication failed" : err);
	ui->passwordEdit->setText("");
	setEnabled(true);
}
