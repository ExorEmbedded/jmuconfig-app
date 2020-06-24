#include "loginform.h"
#include "ui_loginform.h"

LoginForm::LoginForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LoginForm)
{
	m_accepted = false;
	ui->setupUi(this);
	setVisible(false);
	connect(ui->loginButtons, SIGNAL(accepted()), this, SIGNAL(accepted()));
	connect(ui->loginButtons, SIGNAL(rejected()), this, SIGNAL(rejected()));
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
	ui->usernameEdit->setFocus();
	if (parentWidget()) {
		parentWidget()->installEventFilter(this);
		setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
	}
	QWidget::showEvent(s);
}

void LoginForm::hideEvent(QHideEvent* s)
{
	if (parentWidget())
		parentWidget()->removeEventFilter(this);	
	QWidget::hideEvent(s);
}

bool LoginForm::eventFilter(QObject *, QEvent * e)
{
	if (e->type() == QEvent::Resize)
		if (parentWidget()) {
			setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
		}
	return false;
}

void LoginForm::on_loginButtons_accepted()
{
    m_accepted = true;
}

void LoginForm::on_loginButtons_rejected()
{
	m_accepted = false;   
}

void LoginForm::on_usernameEdit_returnPressed()
{
    m_accepted = true;
	emit accepted();
}

void LoginForm::on_passwordEdit_returnPressed()
{
    m_accepted = true;
	emit accepted();
}
