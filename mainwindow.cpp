#include <QtNetwork>
#include <QtDBus/QtDBus>
#if QT_VERSION < 0x050000
#include <QtGui>
#include <QtWebKit>
#else
#include <QtWidgets>
#include <QtWebKitWidgets>
#include <QWebPage>
#include <QtConcurrent>
#endif
#include <unistd.h>
#include <QNetworkAccessManager>
#include "flickcharm.h"
#include "mainwindow.h"
#include "browsersettings.h"
#include "cookiejar.h"
#include "loginform.h"
#include "epadinterface.h"

#define EXITCODE_LOWMEM 	10

// Track backlight state throughout exit()s - created upon backlight_off and
// contains 1 at first interval, 0 otherwise [#11542]
#define BACKLIGHT_STATE_FILE "/tmp/hmibrowser_backlight_off"

//! [1]

class CustomNetworkAccessManager : public QNetworkAccessManager
{
public:
	CustomNetworkAccessManager(BrowserSettings * settings, QObject * parent = 0) : QNetworkAccessManager ( parent )
	{
        if (settings->settings.enableCookies)
		    setCookieJar(&m_cookieJar);
	}
#ifndef EXITPATTERN_CHECK_ON_REPLY
	void setExitPattern(const QString& exitHandler) { m_exitHandler = exitHandler; }
protected:
	virtual QNetworkReply *	createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 )
	{
		QNetworkRequest r = req;

		r.setRawHeader("User-Agent", req.rawHeader("User-Agent") + " jmuconfig-app");

		if (!m_exitHandler.isEmpty()) {
			// Quits application is url matches the exit pattern
			if (req.url().toString().contains(m_exitHandler)) {
				qWarning() << "Quitting on exit url";
				qApp->quit();
			}
		}
		return QNetworkAccessManager::createRequest(op, r, outgoingData);
	}
private:
	QString m_exitHandler;
#endif
private:
	CookieJar m_cookieJar;
};

void CustomWebPage::setJSTimeoutMode( JSTimeoutMode mode )
{
	onJSTimeout = mode;
}

bool CustomWebPage::shouldInterruptJavaScript()
{
	qWarning() << "WARNING: JS timeout triggered. The page may be unresponsive";

	switch (onJSTimeout) {
	case JST_Disabled:
		return false;
	case JST_ReloadPage:
		qWarning() << "Reloading page...";
		triggerAction(QWebPage::Reload);
		return true;
	default:
		// Use the default implementation ( shows the dialog )
		return QWebPage::shouldInterruptJavaScript();
	}
}

void CustomWebPage::javaScriptConsoleMessage (const QString &message, int lineNumber, const QString &sourceID)
{
	qDebug() << qPrintable(QString("[console] [%1:%2] %3").arg(sourceID, QString::number(lineNumber), message));
}


MainWindow::MainWindow(BrowserSettings* settings)
	: m_dragAction(NULL)
	, m_settingsAction(NULL)
	, m_zoomInAction(NULL)
	, m_zoomOutAction(NULL)
	, m_zoomToFitAction(NULL)
	, m_settings(settings)
	, m_authRetries(0)
	, m_useDefaultAuth(true)
	, m_settingsPopup(false)
	, m_loader(NULL)
	, m_loadRequested(true) // first load is treated as requested
	, m_mousePlugged(false)
	, m_forceExit(false)
	, m_loadPageOn(false)
{
	m_progress = 0;
	m_retryTimer = -1;
	m_autoShowLocationBar = false;
	m_loadingInitialPage = true;
	m_retryCount = 0;
	m_locationEditAction = NULL;
	m_zoomFactor = 1.0;

	//    QFile file;
	//    file.setFileName(":/jquery.min.js");
	//    file.open(QIODevice::ReadOnly);
	//    jQuery = file.readAll();
	//    file.close();
	//! [1]

	QNetworkProxyFactory::setUseSystemConfiguration(true);

	CustomWebPage *page = new CustomWebPage();
	if ( m_settings->settings.disableJSTimeout )
		page->setJSTimeoutMode(CustomWebPage::JST_Disabled);
	else if ( m_settings->settings.reloadOnJSTimeout )
		page->setJSTimeoutMode(CustomWebPage::JST_ReloadPage);

	//! [2]
	m_view = new QWebView(this);
#ifdef linux
	// Touch events not working propertly with webview on linux
	//m_view->setAttribute( Qt::WA_NativeWindow, true );
	m_view->setAttribute(Qt::WA_AcceptTouchEvents, false);
#endif

	m_view->setAttribute(Qt::WA_InputMethodEnabled,true);
	m_view->setPage(page);
	qApp->setAutoSipEnabled(true);

	m_locationEdit = new MyQLineEdit(this);
	m_locationEdit->setSizePolicy(QSizePolicy::Expanding, m_locationEdit->sizePolicy().verticalPolicy());
	connect(m_locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

	m_locationEditAction = new QWidgetAction(this);
	m_locationEditAction->setDefaultWidget(m_locationEdit);

	m_dragAction = new QAction(this);
	m_dragAction->setToolTip(tr("Toggle page dragging mode"));
	m_dragAction->setCheckable(true);
	m_dragAction->setIcon(QPixmap(":/drag.png"));

	m_zoomInAction = new QAction(this);
	m_zoomInAction->setToolTip(tr("Zoom in"));
	m_zoomInAction->setCheckable(false);
	m_zoomInAction->setIcon(QPixmap(":/zoom_in.png"));

	m_zoomOutAction = new QAction(this);
	m_zoomOutAction->setToolTip(tr("Zoom out"));
	m_zoomOutAction->setCheckable(false);
	m_zoomOutAction->setIcon(QPixmap(":/zoom_out.png"));

	m_zoomToFitAction = new QAction(this);
	m_zoomToFitAction->setToolTip(tr("Zoom too fit content size"));
	m_zoomToFitAction->setCheckable(true);
	m_zoomToFitAction->setIcon(QPixmap(":/zoom_fit.png"));

	m_settingsAction = new QAction(this);
	m_settingsAction->setToolTip(tr("Settings"));
	m_settingsAction->setIcon(QPixmap(":/settings.png"));
	m_settingsAction->setCheckable(true);

	QString url = m_settings->settings.url;

	if (!url.isEmpty() && m_settings->settings.rememberLastUrl) {
		m_locationEdit->setText(url);
		killTimer(m_retryTimer); m_retryTimer=-1;
		m_retryCount = 0;
	} else {
		if (m_settings->settings.fallbackToDefaultUrl)
			url = m_settings->settings.defaultUrl;
	}

	m_nman = new CustomNetworkAccessManager(settings, this);


	connect(m_nman, SIGNAL(finished(QNetworkReply *)), this,
			SLOT(networkRequestFinished(QNetworkReply *)));

#ifdef BASIC_AUTH
	connect(m_nman, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)),
			this, SLOT(authenticationRequired(QNetworkReply *, QAuthenticator *)));
#else

	m_autoLoginMan = new AutoLoginManager();

	QNetworkCookieJar *cookies = m_nman->cookieJar();
	m_autoLoginMan->setCookieJar(cookies);

	connect(m_autoLoginMan, SIGNAL(autoLoggedIn()),
                     this, SLOT(autoLoginCompleted()));
#endif

	m_view->page()->setNetworkAccessManager(m_nman);
	//view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	//connect(view->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

	m_locationEdit->setText(url);

	//connect(m_view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
	connect(m_view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
	connect(m_view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
	connect(m_view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));
	connect(m_view, SIGNAL(loadStarted()), SLOT(resetProgress()));
	connect(m_view, SIGNAL(urlChanged(const QUrl&)), SLOT(storeUrl(const QUrl&)));


	if (!m_settings->settings.waitFlagFile.isEmpty() && !QFile::exists(m_settings->settings.waitFlagFile)) {
		m_view->setHtml(LOADPAGE_HTML);
		m_loadPageOn = true;

		connect(&m_watcher, SIGNAL(finished()), this, SLOT(waitForFlagFileFinished()));
		m_future = QtConcurrent::run(this, &MainWindow::waitForFlagFile);
		m_watcher.setFuture(m_future);
	} else {
#ifdef BASIC_AUTH
		m_view->load(url);
#else
		m_autoLoginMan->setUser(m_settings->settings.defaultUser);
		m_autoLoginMan->setPassword(m_settings->settings.defaultPassword);
		m_autoLoginMan->attemptAutoLogin();
#endif
	}

	m_flickCharm = new FlickCharm(this);

	createToolBar();

	connect(m_dragAction, SIGNAL(toggled(bool)), this, SLOT(toggleDragging(bool)));

	connect(m_zoomToFitAction, SIGNAL(toggled(bool)), this, SLOT(toggleZoomToFit(bool)));
	m_zoomToFitAction->setChecked(false);
	connect(m_zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(m_zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));


	//locationEdit->installEventFilter(this);
	//! [2]

	//    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
	//    QAction* viewSourceAction = new QAction("Page Source", this);
	//    connect(viewSourceAction, SIGNAL(triggered()), SLOT(viewSource()));
	//    viewMenu->addAction(viewSourceAction);

	////! [3]
	//    QMenu *effectMenu = menuBar()->addMenu(tr("&Effect"));
	//    effectMenu->addAction("Highlight all links", this, SLOT(highlightAllLinks()));

	//    rotateAction = new QAction(this);
	//    rotateAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	//    rotateActiotn->setCheckable(true);
	//    rotateAction->setText(tr("Turn images upside down"));
	//    connect(rotateAction, SIGNAL(toggled(bool)), this, SLOT(rotateImages(bool)));
	//    effectMenu->addAction(rotateAction);

	//    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
	//    toolsMenu->addAction(tr("Remove GIF images"), this, SLOT(removeGifImages()));
	//    toolsMenu->addAction(tr("Remove all inline frames"), this, SLOT(removeInlineFrames()));
	//    toolsMenu->addAction(tr("Remove all object elements"), this, SLOT(removeObjectElements()));
	//    toolsMenu->addAction(tr("Remove all embedded elements"), this, SLOT(removeEmbeddedElements()));

	setCentralWidget(m_view);
	setUnifiedTitleAndToolBarOnMac(true);

	this->layout()->addWidget(m_settings);
	m_settings->setAutoFillBackground(true);
	connect(m_settingsAction, SIGNAL(toggled(bool)), this, SLOT(updateSettingsVisibility()));
	connect(m_settingsAction, SIGNAL(toggled(bool)), m_settings, SLOT(setShown(bool)) );

	m_loginForm = new LoginForm;
	this->layout()->addWidget(m_loginForm);

	m_settingsTimeout.setSingleShot(true);
	m_inactivityTimeout.setInterval(m_settings->settings.inactivityTimeout * 1000);
	m_inactivityFromBacklightOffTimeout.setInterval(m_settings->settings.inactivityFromBacklightOffTimeout * 1000);

	connect(m_settings, SIGNAL(showLoadingToggled(bool)), this, SLOT(setLoadButtonsVisible(bool)));
	connect(m_settings, SIGNAL(showNavigationToggled(bool)), this, SLOT(setHistoryButtonsVisible(bool)));
	connect(m_settings, SIGNAL(showSettingsToggled(bool)), this, SLOT(setSettingsButtonVisible(bool)));
	connect(m_settings, SIGNAL(showLocationToggled(bool)), this, SLOT(setLocationVisible(bool)));
	connect(m_settings, SIGNAL(fullScreenButtonToggled(bool)), this, SLOT(setFullscreen(bool)));
	connect(m_settings, SIGNAL(touchNavigationToggled(bool)), m_dragAction, SLOT(setChecked(bool)));
	connect(m_settings, SIGNAL(zoomToFitToggled(bool)), m_zoomToFitAction, SLOT(setChecked(bool)));
	connect(m_settings, SIGNAL(closeClicked(bool)), m_settingsAction, SLOT(toggle()));
	connect(m_settings, SIGNAL(urlChanged(QString)), this, SLOT(loadUrl(QString)));
	connect(m_settings, SIGNAL(showZoomButtonToggled(bool)), this, SLOT(setZoomButtonVisible(bool)));
	connect(m_settings, SIGNAL(hideScrollbarsToggled(bool)), this, SLOT(setScrollbarsVisible(bool)));
	connect(m_settings, SIGNAL(timeoutChanged(int)), this, SLOT(setSettingsTimeout(int)));

	connect(m_settings, SIGNAL(toolbarVisibilityChanged(int)), this, SLOT(setToolbarVisibility(int)));

	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	//QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, true);
	//QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	//QWebSettings::globalSettings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
	//QWebSettings::globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, true);


	//set cache
	QWebSettings::setMaximumPagesInCache(4);
	QWebSettings::setObjectCacheCapacities(0, 2*1024*1024, 8*1024*1024);


	// apply startup settings from BrowserSettings
	setFullscreen(m_settings->settings.fullscreen);
	m_dragAction->setChecked(m_settings->settings.dragOnTouch);
	setLoadButtonsVisible(m_settings->settings.showLoading);
	setHistoryButtonsVisible(m_settings->settings.showNavigation);
	setSettingsButtonVisible(m_settings->settings.showSettings);
	setLocationVisible(m_settings->settings.showLocation);
	setToolbarVisibility(m_settings->settings.toolbarVisibility);
	setExitPattern(m_settings->settings.exitPattern);
	toggleZoomToFit(m_settings->settings.zoomToFit);
	setScrollbarsVisible(!m_settings->settings.hideScrollbars);
	setZoomButtonVisible(m_settings->settings.showZoomButton);
	setSettingsTimeout(m_settings->settings.settingsTimeout);
	qApp->setApplicationName(m_settings->settings.userAgent);

	connect(&m_settingsTimeout, SIGNAL(timeout()), m_settingsAction, SLOT(toggle()));

#ifdef linux
	connect(&m_inactivityTimeout, SIGNAL(timeout()), this, SLOT(inactivityTimeout()));
	connect(&m_inactivityFromBacklightOffTimeout, SIGNAL(timeout()), this, SLOT(inactivityFromBacklightOffTimeout()));
	if (m_settings->settings.inactivityTimeout) {
		qWarning() << "Inactivity timeout set to" << m_settings->settings.inactivityTimeout << "second(s)";
	} else {
		qWarning() << "Inactivity timeout unset";
	}
	if (m_settings->settings.inactivityFromBacklightOffTimeout) {
		qWarning() << "Inactivity from backlight off timeout set to" << m_settings->settings.inactivityFromBacklightOffTimeout << "second(s)";
	} else {
		qWarning() << "Inactivity from backlight off timeout unset";
	}
#endif

	updateSettingsVisibility();

    //
    // External DBUS communication
    //
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.isConnected()) {
        qWarning() << "Failed connecting to DBUS system bus -> fallback to session bus";
        connection = QDBusConnection::sessionBus();
    }

    m_epad = new ComExorEpadInterface("com.exor.EPAD", "/", connection, this);
    if (m_epad == NULL) {
        qWarning() << "Failed connecting to EPAD interface!";
        return;
    }
    connect(m_epad,SIGNAL(mousePlugEvent(int)),this,SLOT(onMousePlugEvent(int)));

    m_epadBacklight = new ComExorBacklightInterface("com.exor.EPAD", "/Backlight", connection, this);
    if (m_epadBacklight == NULL) {
        qWarning() << "Failed connecting to EPAD Backlight interface!";
        return;
    }
    connect(m_epadBacklight,SIGNAL(stateChanged(QString, int)),this,SLOT(onBacklightStateChanged(QString, int)));

    readInitialState();

    m_view->installEventFilter(this);
}


void MainWindow::waitForFlagFile() {
	// BSP-5166 Panel dies / hangs 40 seconds after booting configos on slow devices
	// We don't know how long System Settings will take, so might as well wait indefinitely
	// for (int i = 0; i < 15; i++) {
	while (true) {
		sleep(1);
		if (QFile::exists(m_settings->settings.waitFlagFile) || !m_loadPageOn)
			return;
	}

	qWarning() << "Timeout waiting for" << m_settings->settings.waitFlagFile;
	exit(1);
}

void MainWindow::waitForFlagFileFinished() {
	if (!m_loadPageOn)
		return;

#ifdef BASIC_AUTH
	m_view->load(m_settings->settings.defaultUrl);
#else
	m_autoLoginMan->setUser(m_settings->settings.defaultUser);
	m_autoLoginMan->setPassword(m_settings->settings.defaultPassword);
	m_autoLoginMan->attemptAutoLogin();
#endif
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
	Q_UNUSED(o);

    //qDebug() << "MainWindow::eventFilter event: " << e->type();

	switch(e->type())
	{
	case QEvent::KeyPress:
		resetInactivityTimers(true);
		break;

	case QEvent::MouseButtonPress:
		if (((QMouseEvent*)e)->button() == Qt::LeftButton)
		{
			if (m_settingsPopup && !m_settingsTimeout.isActive())
				m_settingsTimeout.start();
			resetInactivityTimers(true);
			m_lastPressedPos = ((QMouseEvent*)e)->pos();
		}
		break;

	case QEvent::MouseButtonRelease:
		m_settingsTimeout.stop();
		resetInactivityTimers(true);
		break;
		
	case QEvent::MouseMove:
		resetInactivityTimers(true);
		if ( !m_lastPressedPos.isNull() && ( m_lastPressedPos - ((QMouseEvent*)e)->pos()).manhattanLength() > 50) {
			m_settingsTimeout.stop();
		}
		break;
	default:
		break;
	}
#if 0
	static int i = 0;
	static QTime t;	
	static QHash<int, QTime> tt;
	if (e->type() == QEvent::Timer) {
		QTimerEvent* te = (QTimerEvent*)e;
		qDebug() << i++ << QAbstractEventDispatcher::instance()->registeredTimers(o).size() << "timer: " << te->timerId() << tt[te->timerId()].elapsed();
		tt[te->timerId()].restart();
	}
	if (t.elapsed() > 1000) {
		t.restart();
		int count = 0;
		foreach(const QTime& time, tt) {
			if (time.elapsed() < 5000) {
				count++;
			}
		}		
	}
#endif
	return false;
}

void MainWindow::resetInactivityTimers(bool backlightOn)
{
	//qDebug() << __FUNCTION__ << "backlightOn: " << backlightOn;

	if (backlightOn)
	{
		// deactivate backlight inactivity timeout and activate plain inactivity
		if (m_settings->settings.inactivityFromBacklightOffTimeout)
			m_inactivityFromBacklightOffTimeout.stop();
		if (m_settings->settings.inactivityTimeout)
			m_inactivityTimeout.start();
	}
	else
	{
		// deactivate plain inactivity timeout and activate backlight inactivity
		if (m_settings->settings.inactivityTimeout)
			m_inactivityTimeout.stop();
		if (m_settings->settings.inactivityFromBacklightOffTimeout)
			m_inactivityFromBacklightOffTimeout.start();
	}
}

void MainWindow::timerEvent(QTimerEvent * e)
{
	Q_UNUSED(e);

    if (focusWidget() != m_locationEdit && (!m_settings->inputHasFocus()) ) {
		changeLocation();
	}
}

// Plain inactivity: check memory thresholds to determine whether refresh or quit is necessary to free resources
void MainWindow::inactivityTimeout()
{
	//qDebug() << __FUNCTION__;

	int free = memAvailableMB();
	if (free < 0) {
		qWarning() << "Failed reading free memory";
		return;
	}

	if (free < m_settings->settings.memAvailableExitMB) {
		qWarning() << "Memory below lower threshold - exiting!";
		exit(EXITCODE_LOWMEM);
	}

	if (free < m_settings->settings.memAvailableRefreshMB) {
		qWarning() << "Memory below upper threshold - forcing refresh";
		m_view->reload();
	}
}

bool MainWindow::backlightOn()
{
	QByteArray display = qgetenv("DISPLAY");

	QDBusPendingReply<int> reply = m_epadBacklight->state(display);

	reply.waitForFinished();

	if (!reply.isValid()) {
		qDebug() << "backlightOn() Bad reply from EPAD!";
		return false;
	}

	bool on = reply.argumentAt<0>();

	qWarning() << "backlightOn: " << on;

	return on;
}

// Inactivity from backlight off: check memory thresholds to determine whether quit is necessary to free resources
void MainWindow::inactivityFromBacklightOffTimeout()
{
	//qDebug() << __FUNCTION__;

	bool firstInactivity = false;

	QFile f(BACKLIGHT_STATE_FILE);
	if (!f.open(QIODevice::ReadWrite)) {
		qWarning() << "Failed opening backlight state file!";
		return;
	}

	QString content = f.readAll().trimmed();
	if (content.isEmpty()) {
		f.write("1");
		firstInactivity = true;
	} else if (content == "1") {
		f.seek(0);
		f.write("0");
	}
	f.close();

	if (firstInactivity && m_forceExit) {
		if (backlightOn()) {
			qWarning() << "First inactivity from backlight off but now it's back on - not exiting!";
			return;
		}
		qWarning() << "First inactivity from backlight off - exiting!";
		exit(EXITCODE_LOWMEM);
	}

	int free = memAvailableMB();
	if (free < 0) {
		qWarning() << "Failed reading free memory";
		return;
	}

	if (free < m_settings->settings.memAvailableExitFromBacklightOffMB) {
		if (backlightOn()) {
			qWarning() << "Low mem from backlight off but now it's back on - not exiting!";
			return;
		}
		qWarning() << "Memory below lower threshold (from backlight off) - exiting!";
		exit(EXITCODE_LOWMEM);
	}
}

// Since MemAvailable might be unavailable in /proc, use MemFree + Buffers + Cached
int MainWindow::memAvailableMB()
{
	QFile file("/proc/meminfo");
	int memFree = 0;
	int buffers = 0;
	int cached = 0;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return -1;

	QTextStream in(&file);
	QString line;
	while (!((line = in.readLine()).isNull())) {

		QRegExp lineRx("^(\\S+):\\s+(\\d+)");
		if (lineRx.indexIn(line) == -1 ||
			lineRx.captureCount() != 2)
			continue;

		QString label = lineRx.cap(1);
		int kb = lineRx.cap(2).toInt();
		// we assume all values are in kB

		if (label == "MemFree")
			memFree = kb;
		else if (label == "Buffers")
			buffers = kb;
		else if (label == "Cached")
			cached = kb;
	}

	int availableMB = (memFree + buffers + cached) / 1024;

	//qDebug() << "availableMb: " << availableMB;

	return availableMB;
}

void MainWindow::updateSettingsVisibility()
{
	if (!m_settingsAction || !m_settingsAction->isVisible() || !m_toolBar || !m_toolBar->isVisible()) {
		if (!m_settings->settings.configurationLockedFromCmdLine) {
			m_settingsPopup = true;
		}
	} else {
		m_settingsPopup = false;
		setMouseTracking(false);
		m_settingsTimeout.stop();
	}
}

void MainWindow::createToolBar()
{
	qDebug() << "Creating toolbar...";

	m_toolBar = addToolBar(tr("Navigation"));
	m_toolBar->setFloatable(false);
	m_toolBar->setMovable(false);
	m_toolBar->addAction(m_view->pageAction(QWebPage::Back));
	m_toolBar->addAction(m_view->pageAction(QWebPage::Forward));
	m_toolBar->addAction(m_view->pageAction(QWebPage::Reload));
	m_toolBar->addAction(m_view->pageAction(QWebPage::Stop));
	m_toolBar->addAction(m_locationEditAction);

	m_toolBar->addSeparator();

	m_toolBar->addAction(m_zoomToFitAction);
	m_toolBar->addAction(m_zoomInAction);
	m_toolBar->addAction(m_zoomOutAction);

	m_toolBar->addAction(m_dragAction);
	m_toolBar->addAction(m_settingsAction);

#ifdef Q_WS_WINCE
	toolBar->addSeparator();

	bool m_bKioskMode=false;
	const QStringList &args = qApp->arguments();
	QStringList::const_iterator constIterator;
	for (constIterator = args.constBegin(); constIterator != args.constEnd(); ++constIterator)
	{
		if ((*constIterator).compare("-kiosk", Qt::CaseInsensitive) == 0)
			m_bKioskMode = true;
	}

	if (!m_bKioskMode) {
		QAction* closeAction = new QAction(this);
		closeAction->setToolTip("Exit");
		closeAction->setIcon(QPixmap(":/power.png"));
		connect(closeAction, SIGNAL(triggered()), qApp, SLOT(quit()));
		toolBar->addAction(closeAction);
	}
#endif
	if (m_settings && m_settings->isVisible())
		m_settings->raise();
}

void MainWindow::loadUrl(const QString &url)
{
	// reset auth retries
	m_authRetries = 0;
	m_useDefaultAuth = true;

	m_view->setUrl(url);
	qDebug() << "Load " << url;
	//m_view->setFocus();
	//m_view->setZoomFactor(1);

	adjustLocation();
}

void MainWindow::setCursorVisible(bool visible)
{
    qApp->restoreOverrideCursor();

	if (!visible)
		qApp->setOverrideCursor(Qt::BlankCursor);
}

void MainWindow::setCursorWaiting(bool waiting)
{
    if (!waiting)
    {
        setCursorVisible(m_mousePlugged);
        return;
    }

    if (qApp->overrideCursor())
        qApp->restoreOverrideCursor();

    if (m_mousePlugged)
        qApp->setOverrideCursor(Qt::WaitCursor);
    else
        qApp->setOverrideCursor(Qt::BlankCursor);
}

void MainWindow::setScrollbarsVisible(bool visible)
{
	m_settings->settings.hideScrollbars = (!visible);
	adjustScrollbars();
}

void MainWindow::toggleZoomToFit(bool zoomToFit)
{
	m_settings->settings.zoomToFit = zoomToFit;
	m_settings->saveSettings();

	adjustScrollbars();
	if (!m_settings->settings.zoomToFit) {
		m_zoomFactor = 1.0;
		m_view->setZoomFactor(m_zoomFactor);
	} else {
		calcZoom();
		m_view->setZoomFactor(m_zoomFactor);
	}

	m_view->triggerPageAction(QWebPage::Reload);
	m_zoomToFitAction->setChecked(zoomToFit);
	m_zoomInAction->setDisabled(zoomToFit);
	m_zoomOutAction->setDisabled(zoomToFit);
}

void MainWindow::zoomIn()
{
	m_view->setZoomFactor(m_view->zoomFactor() + 0.25);
}

void MainWindow::zoomOut()
{
	m_view->setZoomFactor(m_view->zoomFactor() - 0.25);
}

void MainWindow::setZoomButtonVisible(bool visible)
{
	m_zoomInAction->setVisible(visible);
	m_zoomOutAction->setVisible(visible);
	m_zoomToFitAction->setVisible(visible);
}

void MainWindow::setSettingsTimeout(int timeout)
{
	qDebug() << "Settings timeout:" << timeout;

	m_settingsTimeout.setInterval(timeout*1000);
}

//! [3]

void MainWindow::viewSource()
{
	QNetworkAccessManager* accessManager = m_view->page()->networkAccessManager();
	QNetworkRequest request(m_view->url());
	QNetworkReply* reply = accessManager->get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(slotSourceDownloaded()));
}

void MainWindow::slotSourceDownloaded()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(const_cast<QObject*>(sender()));
	QTextEdit* textEdit = new QTextEdit(NULL);
	textEdit->setAttribute(Qt::WA_DeleteOnClose);
	textEdit->show();
	textEdit->setPlainText(reply->readAll());
	reply->deleteLater();
}

//! [4]
#include <QSettings>
void MainWindow::adjustLocation()
{
	const QString& urlString = m_view->url().toString();
	if (!urlString.isEmpty()) {
		m_locationEdit->setText(urlString);
		if ((m_settings->settings.rememberLastUrl) && (urlString != m_settings->settings.defaultUrl)) {
			m_settings->settings.url = urlString;
			m_settings->saveSettings();
		}
		killTimer(m_retryTimer); m_retryTimer=-1;
		m_retryCount = 0;
		if (m_autoShowLocationBar && m_toolBar) {
			removeToolBar(m_toolBar);
		}
	}
}

void MainWindow::changeLocation()
{
	killTimer(m_retryTimer); m_retryTimer=-1;
	// m_retryCount = 0;
	QString location = m_locationEdit->text();
	if (!location.startsWith("http://")
			&& !location.startsWith("https://")
			&& !location.startsWith("file://")) {
		location = "http://"+location;
	}
	QUrl url = QUrl(location);
	m_loadRequested = true;
	m_view->load(url);
	qDebug() << "Load " << location;
	m_view->setFocus();
	m_view->setZoomFactor(1);
}
//! [4]

//! [5]
void MainWindow::adjustTitle()
{
	if (m_progress <= 0 || m_progress >= 100) {
		if (m_loader && m_loader->isVisible()) {
			m_loader->hide();
			m_loader->movie()->stop();
		}
		setWindowTitle(m_view->title());
	}
	else {
		if (m_loader && !m_loader->isVisible()) {
			QSize parentSize = m_loader->parentWidget()->size();
			m_loader->setGeometry(parentSize.width()/2 - 16, parentSize.height()/2 - 16, 32, 32);
			m_loader->show();
			m_loader->movie()->start();
		}
		setWindowTitle(QString("%1 (%2%)").arg(m_view->title()).arg(m_progress));
	}
}

void MainWindow::storeUrl(const QUrl& url) {
	if (!m_locationEdit->hasFocus())
		m_locationEdit->setText(url.toString());

	if ((m_settings->settings.rememberLastUrl) && (url.toString() != m_settings->settings.defaultUrl)) {
		if (m_settings->settings.url != m_view->url().toString())
		{
			m_settings->settings.url = m_view->url().toString();
			m_settings->saveSettings();
		}
	}
}

void MainWindow::resetProgress() {
	m_progress = 0;
	
	// set the scrollbar policy to "asneeded". In this way, when the page
	// has finished loading, I can determine whether the vertical scrollbar is actually required 
	// or not (see adjustVerticalScrollBar) by checking min and max values. If not set to 
	// "AsNeeded", min and max are not updated correctly
	m_view->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, m_settings->settings.hideScrollbars?Qt::ScrollBarAlwaysOff:Qt::ScrollBarAsNeeded);
}

void MainWindow::setProgress(int p)
{ 
	if (m_progress == -1)
		return;

	// If loadProgress is emitted but progress has not been incresed it may be that the page has active content and eventually
	// loading will nerver end. Stop the progress here to avoid cursor to be stuck in wait mode
	if (m_progress >= p ) {
		m_progress=-1;
		setCursorWaiting(false);
	}
	else {
		setCursorWaiting(true);
		m_progress = p;
	}
	adjustTitle();

	if (m_progress <= 0 || m_progress >= 100) {
		qWarning() << "Setting progress to"  << m_progress << "assuming load finished";
		finishLoading(true);
	}
}
//! [5]

//! [6]
void MainWindow::finishLoading(bool ok)
{
	Q_UNUSED(ok);

	m_loadPageOn = false;
	m_progress = -1;
	setCursorWaiting(false);
	adjustTitle();

	bool finishedWithError = false;
	if (m_loadRequested && !m_view->page()->bytesReceived()) { // some error in main request (no bytes received)
		finishedWithError = true;
		// retry only if not already editing the address, otherwise we lose the focus
		if ((!m_locationEdit->hasFocus()) && (!m_settings->inputHasFocus())) {
			killTimer(m_retryTimer);
			m_retryTimer = startTimer(m_settings->settings.reloadRetryTimeout * 1000);

			if (m_retryCount <= m_settings->settings.numLoadRetries)
				m_retryCount++;

			if (m_retryCount > m_settings->settings.numLoadRetries && ! (m_settings->settings.numLoadRetries < 0))
			{
				if (m_autoShowLocationBar && !m_toolBar) {
					createToolBar();
				}

				if (m_loadingInitialPage)
				{
					m_loadingInitialPage = false;
					if(m_settings->settings.fallbackToDefaultUrl)
					{
						killTimer(m_retryTimer);
						m_retryCount = 0;

						m_locationEdit->setText(m_settings->settings.defaultUrl);
						m_retryTimer = startTimer(100);
					}
				}
			}
		}
	} else
		killTimer(m_retryTimer);

	qWarning() << "Finished loading!";

	// finished main load
	if (m_loadRequested && finishedWithError) {
		if ((!m_locationEdit->hasFocus()) && (!m_settings->inputHasFocus()))
			qWarning() << "Load finished with error => retry in " << m_settings->settings.reloadRetryTimeout << " sec";
		else
			qWarning() << "Load finished with error, but edit field is focused";
	} else {
		qWarning() << "Page loaded successfully!";
		m_loadingInitialPage = false;

		if (m_loadRequested)
		{
			if (m_autoShowLocationBar && m_toolBar) {
				removeToolBar(m_toolBar);
			}
		}
	}

	m_loadRequested = false; // more finished load can occur because of ajax but the byteReceived will be zero

	if (m_settings->settings.zoomToFit) {
		calcZoom();
	}

	if (m_view->zoomFactor() != m_zoomFactor) {
		qDebug() << "Apply zoom factor " << m_zoomFactor;
		m_view->setZoomFactor(m_zoomFactor);
	}

	adjustVerticalScrollBar();

	//    view->page()->mainFrame()->evaluateJavaScript(jQuery);

	//    rotateImages(rotateAction->isChecked());
}
//! [6]

//! [7]
void MainWindow::highlightAllLinks()
{
	QString code = "$('a').each( function () { $(this).css('background-color', 'yellow') } )";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}
//! [7]

//! [8]
void MainWindow::rotateImages(bool invert)
{
	QString code;
	if (invert)
		code = "$('img').each( function () { $(this).css('-webkit-transition', '-webkit-transform 2s'); $(this).css('-webkit-transform', 'rotate(180deg)') } )";
	else
		code = "$('img').each( function () { $(this).css('-webkit-transition', '-webkit-transform 2s'); $(this).css('-webkit-transform', 'rotate(0deg)') } )";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}
//! [8]

//! [9]
void MainWindow::removeGifImages()
{
	QString code = "$('[src*=gif]').remove()";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}

void MainWindow::removeInlineFrames()
{
	QString code = "$('iframe').remove()";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}

void MainWindow::removeObjectElements()
{
	QString code = "$('object').remove()";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}

void MainWindow::removeEmbeddedElements()
{
	QString code = "$('embed').remove()";
	m_view->page()->mainFrame()->evaluateJavaScript(code);
}

#ifndef BASIC_AUTH
void MainWindow::autoLoginCompleted()
{
    qDebug() << "autoLoginCompleted. Loading default url" << m_settings->settings.defaultUrl;
    m_view->load(m_settings->settings.defaultUrl);
}
#endif

void MainWindow::networkRequestFinished(QNetworkReply * reply)
{
	Q_UNUSED(reply);

#ifdef EXITPATTERN_CHECK_ON_REPLY
	if (!m_exitHandler.isEmpty()) {
		// Quits application is url matches the exit pattern
		if (reply->url().toString().contains(m_exitHandler)) {
			qWarning() << "Quitting on reply exit url";
			qApp->quit();
		}
	}
#endif

	//	qDebug() << "Loading finished" << reply->url();
}


#ifdef BASIC_AUTH
void MainWindow::authenticationRequired(QNetworkReply * reply, QAuthenticator * authenticator)
{
	Q_UNUSED(reply);

	qDebug() << "Authentication required " << authenticator << "url" << reply->url();

	if (m_useDefaultAuth && !m_settings->settings.defaultUser.isEmpty()) {
		m_useDefaultAuth = false;
		if (authenticator) {
			authenticator->setUser(m_settings->settings.defaultUser);
			authenticator->setPassword(m_settings->settings.defaultPassword);
		}
	} else {
		m_loginForm->show();
		if ( m_authRetries > 0 )
			m_loginForm->reject();
		QEventLoop loop;
		connect(m_loginForm, SIGNAL(accepted()), &loop, SLOT(quit()));
		connect(m_loginForm, SIGNAL(rejected()), &loop, SLOT(quit()));
		loop.exec();

		if (m_loginForm->isAccepted() && authenticator) {
			authenticator->setPassword(m_loginForm->password());
			authenticator->setUser(m_loginForm->username());
		} else if (!m_loginForm->isAccepted()) {
			if (m_settings->settings.exitOnHttpAuthFail) {
				qWarning() << "Quitting on auth failed";
				qApp->quit();
			}
		}
		m_loginForm->hide();
		m_authRetries++;
	}
}
#endif //BASIC_AUTH

void MainWindow::calcZoom()
{
	qreal currentZoomFactor = m_view->zoomFactor();
	m_zoomFactor = 1.0;
	if (m_settings->settings.zoomToFit && m_view->page() && m_view->page()->mainFrame()) {
		QSizeF contentSize = m_view->page()->mainFrame()->contentsSize();
		QRect scrollRect = m_view->page()->mainFrame()->scrollBarGeometry(Qt::Vertical);
		qreal newFactor = (m_view->page()->viewportSize().width()-2*scrollRect.width()) / contentSize.width();
		if (contentSize.width() && !qFuzzyCompare(newFactor, (qreal) 1.0)) {
			m_zoomFactor = currentZoomFactor * newFactor;
		} else {
			m_zoomFactor = currentZoomFactor;
		}
	}
}

void MainWindow::adjustVerticalScrollBar()
{
}

void MainWindow::adjustScrollbars()
{
	if (m_settings->settings.zoomToFit || m_settings->settings.hideScrollbars) {
		m_view->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	} else {
		m_view->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
	}
}

void MainWindow::toggleDragging(bool activate)
{
	if (activate) {
		m_flickCharm->activateOn(m_view, m_settings->settings.wheelWorkaround);
		m_view->triggerPageAction(QWebPage::Reload);
	} else {
		m_flickCharm->deactivateFrom(m_view);
		m_view->triggerPageAction(QWebPage::Reload);
	}
		
	m_settings->settings.dragOnTouch = activate;
	m_settings->saveSettings();
}

void MainWindow::setToolbarVisibility(int visibility)
{
	m_autoShowLocationBar = false;
	m_retryCount = 0;
	switch(visibility) 
	{
	case BrowserSettings::Settings::ToolbarAuto:
		m_autoShowLocationBar = true;
		if (m_toolBar) {
			removeToolBar(m_toolBar);
			delete m_toolBar;
			m_toolBar = NULL;
		}
		break;
	case BrowserSettings::Settings::ToolbarVisible:
		if (!m_toolBar) {
			createToolBar();
		}
		break;
	case BrowserSettings::Settings::ToolbarHidden:
		if (m_toolBar) {
			removeToolBar(m_toolBar);
			delete m_toolBar;
			m_toolBar = NULL;
		}
		break;
	}
}

void MainWindow::setHistoryButtonsVisible(bool visible)
{
	m_view->pageAction(QWebPage::Back)->setVisible(visible);
	m_view->pageAction(QWebPage::Forward)->setVisible(visible);
}

void MainWindow::setLoadButtonsVisible(bool visible)
{
	m_view->pageAction(QWebPage::Reload)->setVisible(visible);
	m_view->pageAction(QWebPage::Stop)->setVisible(visible);
	
}

void MainWindow::setSettingsButtonVisible(bool visible)
{
	if (!m_settingsAction) return;
	if (!visible) {
		m_settingsAction->setVisible(false);
		m_dragAction->setVisible(false);
	} else {		
		m_settingsAction->setVisible(!m_settings->settings.configurationLockedFromCmdLine);
		m_dragAction->setVisible(true);
	}
}

void MainWindow::setLocationVisible(bool visible)
{
	m_locationEditAction->setVisible(visible);
}

void MainWindow::setExitPattern(const QString &pattern)
{
#ifdef EXITPATTERN_CHECK_ON_REPLY
	m_exitHandler = pattern;
#else
	m_nman->setExitPattern(pattern);
#endif
}

void MainWindow::setFullscreen(bool fullScreen)
{
	if (fullScreen) {
		setGeometry(qApp->desktop()->rect());
		// don't stay on top (when used as JMUConfig-app for local get/update operations, FileBrowser 
		// instances were sent to background) TODO check if breaks other cases
		// - if so we'll need to make this configurable
		//setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint );
		hide();
		show();
	} else {
		setGeometry(20,20,640,480);
		setWindowFlags(windowFlags() & (~Qt::FramelessWindowHint) & (~Qt::WindowStaysOnTopHint));
		hide();
		show();
	}
}

void MainWindow::readInitialState()
{
    enum {
        DBUS_RETRIES = 10
    };

    qDebug() << "Reading initial state";

    QByteArray display = qgetenv("DISPLAY");

    // get mouse status from EPAD
    for(int i=0; true; i++) {
        QDBusPendingReply<int>  reply = m_epad->mouseStatus(display);
        reply.waitForFinished();
        if(reply.isFinished() && reply.isValid() && !reply.isError()){
            m_mousePlugged = reply.argumentAt<0>();
            qDebug() << "Mouse plugged:" << m_mousePlugged;
            setCursorVisible(m_mousePlugged);
            break;
        } else {
            if (i > DBUS_RETRIES)
                return;
            qDebug() << __FILE__ << " Error contacting dbus server, retry #" << i << " ...";
            usleep(1000000);
        }
    }

    // get Backlight state from EPAD
    for(int i=0; true; i++) {
        QDBusPendingReply<int>  reply = m_epadBacklight->state(display);
        reply.waitForFinished();
        if(reply.isFinished() && reply.isValid() && !reply.isError()){
            bool backlightOn = reply.argumentAt<0>();
            qDebug() << "Backlight on:" << backlightOn;
            resetInactivityTimers(backlightOn);
            m_forceExit = backlightOn;
            break;
        } else {
            if (i > DBUS_RETRIES)
                return;
            qDebug() << __FILE__ << " Error contacting dbus server, retry #" << i << " ...";
            usleep(1000000);
        }
    }
}

void MainWindow::onMousePlugEvent(int bPlugged)
{
    qDebug() << "Browser received onMousePlugEvent: " << bPlugged;

    m_mousePlugged = bPlugged;

    // invisible cursor while mouse is unplugged
    setCursorVisible(bPlugged);
}

void MainWindow::onBacklightStateChanged(const QString &displayName, int state)
{
    Q_UNUSED(displayName);

    qDebug() << "Browser received onBacklightStateChanged: " << state;

    // Note: currently we only receive backlight off notifications, so result will always be false
	bool backlightOn = (state == 1);

	// Each time backlight turns off we need to force exit condition (independant of memory available)
	if (!backlightOn) {
		QFile::remove(BACKLIGHT_STATE_FILE);
		m_forceExit = true;
	}

	resetInactivityTimers(backlightOn);
}

//! [9]

