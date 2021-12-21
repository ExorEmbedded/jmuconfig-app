#include "browsersettings.h"
#include "ui_browsersettings.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QApplication>
#ifdef linux
#include <unistd.h>
#include <sys/file.h> // for locking
#include <sys/resource.h>
#endif


// Global browser settings shared by all application classes
QSettings* browserSettings = NULL;

BrowserSettings::BrowserSettings(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BrowserSettings)
{
	setVisible(false);
	
	QString settingsFile = QDir(qApp->applicationDirPath()).filePath("browser.ini");
	if (!QFile::exists(settingsFile)) {
		qDebug() << "Initializing settings file: " << settingsFile;
		QFile f(settingsFile);
		f.open(QIODevice::WriteOnly);
		f.close();
	}
	
	// Test settings file
	{
		QFile f(settingsFile);
		if (!f.open(QIODevice::ReadWrite)) {
			qWarning() << "Error opening settings file " << settingsFile << " in read/write mode " << f.errorString();
			// Global settings file
			browserSettings = new QSettings("Exor", "fancybrowser");
		} else {
			// Local settings file
			browserSettings = new QSettings(settingsFile,QSettings::IniFormat);			
		}
	}	
	
	// init settings from settings file
	initFromSettings();
	
	// Overwrite settings file with command line
	initFromCommandLine();
	
	ui->setupUi(this);

	// Apply settings to settings dialog
	applySettings();

	ui->holdValue->setValidator( new QIntValidator(1, 99, this));
	m_flickCharm = new FlickCharm(this);
	m_flickCharm->activateOn(ui->scrollArea);
	m_flickCharm->activateSubWidgetOn(ui->groupBox_1, ui->scrollArea);
	m_flickCharm->activateSubWidgetOn(ui->groupBox_2, ui->scrollArea);
	m_flickCharm->activateSubWidgetOn(ui->groupBox_3, ui->scrollArea);

	// Keep horizontal scrollbar
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	connect(ui->showNavigationControls, SIGNAL(toggled(bool)), this, SIGNAL(showNavigationToggled(bool)));
	connect(ui->showLoadingControls, SIGNAL(toggled(bool)), this, SIGNAL(showLoadingToggled(bool)));
	connect(ui->showSettingsButton, SIGNAL(toggled(bool)), this, SIGNAL(showSettingsToggled(bool)));
	connect(ui->showLocationBar, SIGNAL(toggled(bool)), this, SIGNAL(showLocationToggled(bool)));
	connect(ui->closeButton, SIGNAL(clicked(bool)), this, SIGNAL(closeClicked(bool)));
	connect(ui->exitButton, SIGNAL(clicked(bool)), qApp, SLOT(quit()));
	ui->exitButton->setVisible(settings.exitButton);
	connect(ui->enableTouchNavigation, SIGNAL(toggled(bool)), this, SIGNAL(touchNavigationToggled(bool)));
	connect(ui->zoomToFit, SIGNAL(toggled(bool)), this, SIGNAL(zoomToFitToggled(bool)));
	connect(ui->showZoomControls, SIGNAL(toggled(bool)), this, SIGNAL(showZoomButtonToggled(bool)));
	connect(ui->hideScrollbars, SIGNAL(toggled(bool)), this, SIGNAL(hideScrollbarsToggled(bool)));

	connect(ui->showSettingsButton, SIGNAL(toggled(bool)), this, SLOT(toolbarVisibilityChangedSlot()));
	connect(ui->locButtonAuto, SIGNAL(toggled(bool)), this, SLOT(toolbarVisibilityChangedSlot()));
	connect(ui->locButtonShow, SIGNAL(toggled(bool)), this, SLOT(toolbarVisibilityChangedSlot()));
	connect(ui->locButtonHide, SIGNAL(toggled(bool)), this, SLOT(toolbarVisibilityChangedSlot()));
	
	connect(ui->pinEdit, SIGNAL(returnPressed()), this, SLOT(on_lockDialogBoxButtons_accepted()));

	if (parent) {
		parent->installEventFilter(this);
	}
}

BrowserSettings::~BrowserSettings()
{
	delete ui;
}

bool BrowserSettings::inputHasFocus() { 
	return (ui->defaultUrl->hasFocus() || ui->userAgentEdit->hasFocus());
}

void BrowserSettings::initFromSettings()
{
	qDebug() << "Loading settings from file " << browserSettings->fileName();

	settings.showLocation  = browserSettings->value("showLocation", true).toBool();
	settings.showNavigation = browserSettings->value("showNavigation", true).toBool();
	settings.showLoading = browserSettings->value("showLoading", true).toBool();
	settings.showSettings = browserSettings->value("showSettings", true).toBool();
	settings.hideScrollbars = browserSettings->value("hideScrollbars", false).toBool();
	settings.fallbackToDefaultUrl = browserSettings->value("fallbackToDefaultUrl", true).toBool();
	QString toolbarVisibility = browserSettings->value("toolbarVisibility", "visible").toString();
	settings.fullscreen = browserSettings->value("fullscreen", false).toBool();
	settings.dragOnTouch = browserSettings->value("dragOnTouch", true).toBool();
	if (toolbarVisibility == "hidden") {
		settings.toolbarVisibility = Settings::ToolbarHidden;
	} else if (toolbarVisibility == "auto") {
		settings.toolbarVisibility = Settings::ToolbarAuto;	
	} else {
		settings.toolbarVisibility = Settings::ToolbarVisible;
	}
	settings.url = browserSettings->value("url", "").toString();
	settings.defaultUrl = browserSettings->value("defaultUrl", "http://www.google.com/").toString();
	settings.rememberLastUrl = browserSettings->value("rememberLastUrl", true).toBool();
	settings.exitPattern = browserSettings->value("exitPattern").toString();
	settings.exitButton = browserSettings->value("exitButton", false).toBool();
	settings.pin = browserSettings->value("pin", "").toByteArray();
	settings.wheelWorkaround = browserSettings->value("wheelWorkaround", true).toBool();
	settings.zoomToFit = browserSettings->value("zoomToFit", false).toBool();
	settings.showZoomButton = browserSettings->value("showZoomButton", false).toBool();
	settings.settingsTimeout = browserSettings->value("settingsTimeout", 5).toInt();
	settings.userAgent = browserSettings->value("userAgent", "WebkitBrowser").toString();
	settings.inactivityTimeout = browserSettings->value("inactivityTimeout", 0).toInt(); // disabled by default
	settings.inactivityFromBacklightOffTimeout = browserSettings->value("inactivityFromBacklightOffTimeout", 0).toInt(); // disabled by default
	settings.memAvailableRefreshMB = browserSettings->value("memAvailableRefreshMB", 70).toInt();
	settings.memAvailableExitMB = browserSettings->value("memAvailableExitMB", 40).toInt();
	settings.memAvailableExitFromBacklightOffMB = browserSettings->value("memAvailableExitFromBacklightOffMB", 40).toInt();
    settings.numLoadRetries = browserSettings->value("numLoadRetries", 3).toInt();
    settings.reloadRetryTimeout = browserSettings->value("reloadRetryTimeout", 5).toInt();
    if ( settings.reloadRetryTimeout < 5 )
        settings.reloadRetryTimeout = 5;
	settings.disableJSTimeout = browserSettings->value("noJSTimeout", false).toBool();
	settings.reloadOnJSTimeout = browserSettings->value("reloadOnJSTimeout", false).toBool();
}

void BrowserSettings::initFromCommandLine()
{
	static QRegExp parameterizedValues("-[xk]");
	// default url
	QUrl url;
	QRegExp urlOptionRegExp("^[^- ][^ ]+");
	const QStringList& args = qApp->arguments();
	
	int urlPosition = args.indexOf(urlOptionRegExp, 1);
	while (urlPosition>=0 && urlPosition < args.size()) {
		args.at(urlPosition-1);
		if (urlPosition == 0 || 
				(!parameterizedValues.exactMatch( args.at(urlPosition-1)) ) // x and k arguments requires a parameter
				) {
			url = QUrl(args.at(urlPosition));
			// Set the URL as default and last visited URL
			settings.defaultUrl = settings.url = url.toString();
			qDebug() << "Loaded default url: " << settings.defaultUrl << settings.url;
			break;
		}
		
		urlPosition = args.indexOf(urlOptionRegExp, urlPosition+1);
	}
	
	// parse flags
	for(int i=0; i<args.size(); i++) {
		const QString& argument = args.at(i);
		if (argument == "-f") {
			qDebug() << "Enter fullscreen mode";
			settings.fullscreen = true;			
		} else if (argument == "-nf") {
			qDebug() << "Exit fullscreen mode";
			settings.fullscreen = false;			
		} else if (argument == "-e") {
			qDebug() << "Enable exit button";
			settings.exitButton = true;			
		} else if (argument == "-g") {
			qDebug() << "Enable drag by touch";
			settings.dragOnTouch = true;			
		} else if (argument == "-ng") {
			qDebug() << "Disable drag by touch";
			settings.dragOnTouch = false;			
		} else if (argument == "-l") {
			qDebug() << "Hide location bar";
			settings.toolbarVisibility = Settings::ToolbarHidden;
		} else if (argument == "-nl") {
			qDebug() << "Show location bar";
			settings.toolbarVisibility = Settings::ToolbarVisible;
		} else if (argument == "-w") {
			qDebug() << "Enable wheel workaround";
			settings.wheelWorkaround = true;
		} else if (argument == "-nw") {
			qDebug() << "Disable wheel workaround";
			settings.wheelWorkaround = false;
		} else if (argument == "-z") {
			qDebug() << "Show zoom controls";
			settings.showZoomButton = true;
		} else if (argument == "-nz") {
			qDebug() << "Hide zoom controls";
			settings.showZoomButton = false;
		} else if (argument == "-zf") {
			qDebug() << "Set zoom to fit";
			settings.zoomToFit = true;
		} else if (argument == "-nzf") {
			qDebug() << "Unset zoom to fit";
			settings.zoomToFit = false;
		} else if (argument == "-y") {
			qDebug() << "Lock configuration";
			settings.configurationLockedFromCmdLine = true;
		} else if (argument == "-k") {
			// lock file to use
			if (i<args.size()-1) {
				i++;
				qDebug() << "using lockfile " << args.at(i);
				QFile* f = new QFile(args.at(i));
				f->open(QIODevice::ReadWrite);
				if (flock(f->handle(), LOCK_EX | LOCK_NB)) {
					qWarning() << "Error acquiring lock on file " << f->fileName();
					perror("Got error: ");
					qWarning() << "Another instance appears running while single instance mode is required";
					exit(-1);
				}
				// intentional leak of QFile f
			}
		} else if (argument == "-u") {
			// lock file to use
			if (i<args.size()-1) {
				i++;
				settings.defaultUser = args.at(i);
			}
		} else if (argument == "-p") {
			// lock file to use
			if (i<args.size()-1) {
				i++;
				settings.defaultPassword = args.at(i);
			}
		} else if (argument == "-X") {
			settings.exitOnHttpAuthFail = true;
		} else if (argument == "-ck") {
			settings.enableCookies = true;
		} else if (argument == "-st") {
			if (i<args.size()-1) {
				i++;
				int timeout = args.at(i).toInt();
				if ( timeout > 0 )
					settings.settingsTimeout = (timeout<100 ? timeout : 99);
			}
		} else if (argument == "-i") {
			if (i<args.size()-1) {
				i++;
				settings.userAgent = args.at(i);
			}
		} else if (argument == "-noJSTimeout") {
			settings.disableJSTimeout = true;
		} else if (argument == "-reloadOnJSTimeout") {
			settings.reloadOnJSTimeout = true;
		} else if (argument == "-wff") {
			if (i<args.size()-1) {
				i++;
				settings.waitFlagFile = args.at(i);
			}
		}
#ifdef linux
		else if (argument == "-r") {
			// set nice priority
			if (i<args.size()-1) {
				i++;
				setpriority(PRIO_PROCESS, 0, args.at(i).toInt());
			}
		}
#endif
	}

	/*! Exit handler pattern, used to exit app when a url containing the path is loaded */
	if (args.contains("-x")) {
		int exitIdx = args.indexOf("-x");
		if (exitIdx >= 0 && exitIdx < (args.size()-1)) {
			qDebug() << "Quit app on loading url matching pattern exit " << args.at(exitIdx+1);
			settings.exitPattern = args.at(exitIdx+1);
		}
	}


}

void BrowserSettings::showEvent(QShowEvent* s)
{
	m_urlChanged=false;
	m_timeoutChanged=false;
	
	// sync ui
	blockSignals(true);
	applySettings();
	blockSignals(false);
	
	if (parentWidget()) {
		parentWidget()->installEventFilter(this);
		setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
	}
	QWidget::showEvent(s);
	
	ui->incorrectPinLabel->hide();
	ui->pinEdit->setText("");
	if (!settings.pin.isEmpty()) {
		m_validating=true;
		ui->stackedWidget->setCurrentIndex(1);				
	} else {
		m_validating=false;
		ui->stackedWidget->setCurrentIndex(0);
	}
	
}

void BrowserSettings::hideEvent(QHideEvent* s)
{
	if (parentWidget())
		parentWidget()->removeEventFilter(this);

	// Save settings on hide
	syncSettings();
	saveSettings();
	QWidget::hideEvent(s);
	
	if (m_urlChanged)
		emit urlChanged(settings.url);
	if (m_timeoutChanged)
		emit timeoutChanged(settings.settingsTimeout);
}

bool BrowserSettings::eventFilter(QObject *, QEvent * e)
{
	if (e->type() == QEvent::Resize)
		if (parentWidget()) {
			setGeometry(QRect(0,0,parentWidget()->width(), parentWidget()->height()));
		}
	return false;
}

void BrowserSettings::on_lockSettingsButton_toggled(bool checked)
{
	if (checked) {
	    ui->stackedWidget->setCurrentIndex(1);
		ui->pinEdit->clear();
	} else {
		settings.pin.clear();
	}
}

void BrowserSettings::on_lockDialogBoxButtons_accepted()
{
	ui->incorrectPinLabel->hide();
	if (m_validating) {
		if (checkPin(ui->pinEdit->text())) {
			m_validating = false;
			// valid pin
			ui->stackedWidget->setCurrentIndex(0);			
		}  else {
			ui->pinEdit->text().clear	();
			// show incorrect pin
			ui->incorrectPinLabel->show();
		}
	} else {
		// Save pin
		if (!ui->pinEdit->text().isEmpty()) {
			ui->lockSettingsButton->setChecked(true);
			settings.pin = QCryptographicHash::hash(ui->pinEdit->text().toUtf8(), QCryptographicHash::Md5).toHex();
		} else {
			settings.pin.clear();
			ui->lockSettingsButton->setChecked(false);
		}
		ui->stackedWidget->setCurrentIndex(0);
	}
}


void BrowserSettings::on_lockDialogBoxButtons_rejected()
{
	if (m_validating) {
		emit closeClicked(true);
	} else {
		ui->incorrectPinLabel->hide();
		// Clear pin
		if (!settings.pin.isEmpty()) {
			ui->lockSettingsButton->setChecked(true);
		} else {
			ui->lockSettingsButton->setChecked(false);		
		}
		ui->stackedWidget->setCurrentIndex(0);
	}
}

void BrowserSettings::on_showPinCheckbox_toggled(bool checked)
{
	ui->pinEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password );
}

void BrowserSettings::toolbarVisibilityChangedSlot()
{
	ui->settingsWarning->setVisible(false);
	if (ui->locButtonAuto->isChecked()) {
		ui->settingsWarning->setVisible(true);
		emit toolbarVisibilityChanged(Settings::ToolbarAuto);
	} else if (ui->locButtonHide->isChecked()) {
		ui->settingsWarning->setVisible(true);
		emit toolbarVisibilityChanged(Settings::ToolbarHidden);
	} else if (ui->locButtonShow->isChecked()) {
		emit toolbarVisibilityChanged(Settings::ToolbarVisible);
	}
	if (!ui->showSettingsButton->isChecked()) {
		ui->settingsWarning->setVisible(true);
	}
}

void BrowserSettings::saveSettings()
{	
	browserSettings->setValue("showLocation", settings.showLocation);
	browserSettings->setValue("showNavigation", settings.showNavigation);
	browserSettings->setValue("showLoading", settings.showLoading);
	browserSettings->setValue("showSettings", settings.showSettings);
	browserSettings->setValue("hideScrollbars", settings.hideScrollbars);
	browserSettings->setValue("fallbackToDefaultUrl", settings.fallbackToDefaultUrl);
	browserSettings->setValue("fullscreen", settings.fullscreen);
	browserSettings->setValue("dragOnTouch", settings.dragOnTouch );
	browserSettings->setValue("toolbarVisibility", settings.toolbarVisibility == Settings::ToolbarHidden 
										  ? "hidden" 
										  : settings.toolbarVisibility == Settings::ToolbarAuto 
											? "auto"
											: "visible");
	browserSettings->setValue("url", settings.url);
	browserSettings->setValue("defaultUrl", settings.defaultUrl);
	browserSettings->setValue("rememberLastUrl", settings.rememberLastUrl);
	browserSettings->setValue("pin", settings.pin);
	browserSettings->setValue("wheelWorkaround", settings.wheelWorkaround);
	browserSettings->setValue("zoomToFit", settings.zoomToFit);
	browserSettings->setValue("showZoomButton", settings.showZoomButton);
	browserSettings->setValue("settingsTimeout", settings.settingsTimeout);
	browserSettings->setValue("userAgent", settings.userAgent);
	browserSettings->sync();
#ifdef linux
	::sync();
#endif
}

void BrowserSettings::applySettings()
{
	// Apply settings
	ui->showLocationBar->setChecked(settings.showLocation);
	ui->showNavigationControls->setChecked(settings.showNavigation);
	ui->showLoadingControls->setChecked(settings.showLoading);
	ui->showSettingsButton->setChecked(settings.showSettings);
	switch(settings.toolbarVisibility)
	{
	case Settings::ToolbarVisible:
		ui->locButtonShow->setChecked(true);
		break;
	case Settings::ToolbarHidden:
		ui->locButtonHide->setChecked(true);
		break;
	case Settings::ToolbarAuto:
		ui->locButtonAuto->setChecked(true);
		break;
	}
	ui->defaultUrl->setText(settings.defaultUrl);
	ui->startFromLastVisitedPage->setChecked(settings.rememberLastUrl);
	ui->enableTouchNavigation->setChecked(settings.dragOnTouch);
	if (!settings.pin.isEmpty()) {
		ui->lockSettingsButton->setChecked(true);
	}
	ui->holdValue->setText(QString::number(settings.settingsTimeout));
	toolbarVisibilityChangedSlot();
	// Do not disable the default URL. The default URL will be loaded when last visited page is blank
	// ui->defaultUrl->setDisabled(settings.rememberLastUrl);
	ui->zoomToFit->setChecked(settings.zoomToFit);
	ui->hideScrollbars->setChecked(settings.hideScrollbars);
	ui->fallbackToDefaultUrl->setChecked(settings.fallbackToDefaultUrl);
	ui->showZoomControls->setChecked(settings.showZoomButton);
	ui->userAgentEdit->setText(settings.userAgent);

}

void BrowserSettings::syncSettings()
{
	// sync settings from ui
	settings.showLocation = ui->showLocationBar->isChecked();
	settings.showNavigation = ui->showNavigationControls->isChecked();
	settings.showLoading = ui->showLoadingControls->isChecked();
	settings.showSettings = ui->showSettingsButton->isChecked();
	settings.toolbarVisibility = ui->locButtonShow->isChecked() 
			? Settings::ToolbarVisible
			: ui->locButtonAuto->isChecked() 
			  ? Settings::ToolbarAuto : Settings::ToolbarHidden;
	settings.defaultUrl = ui->defaultUrl->text();
	settings.rememberLastUrl = ui->startFromLastVisitedPage->isChecked();
	settings.dragOnTouch = ui->enableTouchNavigation->isChecked();
	settings.zoomToFit = ui->zoomToFit->isChecked();
	settings.hideScrollbars = ui->hideScrollbars->isChecked();
	settings.fallbackToDefaultUrl = ui->fallbackToDefaultUrl->isChecked();
	settings.showZoomButton = ui->showZoomControls->isChecked();
	
	if (!settings.rememberLastUrl)
		settings.url.clear();
}

bool BrowserSettings::checkPin(const QString &plainTextPin)
{
	return QCryptographicHash::hash(plainTextPin.toUtf8(), QCryptographicHash::Md5).toHex() == settings.pin;
}

void BrowserSettings::on_defaultUrl_editingFinished()
{
	settings.url = ui->defaultUrl->text();
	m_urlChanged=true;
}

void BrowserSettings::on_holdValue_editingFinished()
{
	int timeout = ui->holdValue->text().toInt();
	if ( timeout > 0 ) {
		settings.settingsTimeout = (timeout<100 ? timeout : 99);
		m_timeoutChanged=true;
	}
}

void BrowserSettings::on_userAgentEdit_editingFinished()
{
    settings.userAgent = ui->userAgentEdit->text();
	qApp->setApplicationName(settings.userAgent);
}
