/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif
#include <QTimer>
#include <QWebPage>
#include "browsersettings.h"
#ifndef BASIC_AUTH
#include "autoLoginManager.h"
#endif

class QWebView;
QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

//! [1]

class CustomWebPage : public QWebPage
{
	Q_OBJECT

public:

	enum JSTimeoutMode {
		JST_Disabled,
		JST_ShowDialog,
		JST_ReloadPage
	};

	CustomWebPage(): onJSTimeout(JST_ShowDialog) {}
	void setJSTimeoutMode( JSTimeoutMode mode );

protected:

	JSTimeoutMode onJSTimeout;

	void javaScriptConsoleMessage (const QString &message, int lineNumber, const QString &sourceID);

public slots:
	bool shouldInterruptJavaScript();
};

class MyQLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    MyQLineEdit(QWidget* parent = 0) : QLineEdit(parent)
    {
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(0);
        buttonLayout->setMargin(0);
        setLayout(buttonLayout);
        m_clearButton = new QPushButton;
        m_clearButton->setMaximumSize(16,16);
        m_clearButton->setFocusPolicy(Qt::NoFocus);
        m_clearButton->setStyleSheet("background-color: rgba(0,0,0,0)");
        m_clearButton->setIcon(QPixmap(":/clear.png"));
        m_clearButton->setIconSize(QSize(14,14));
        connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clear()));
        connect(m_clearButton, SIGNAL(clicked()), this, SLOT(cleared()));
        buttonLayout->setAlignment(m_clearButton, Qt::AlignRight);
        buttonLayout->addStretch(100);
        buttonLayout->addWidget(m_clearButton);
        m_clearButton->show();
#ifdef Q_WS_WINCE
        //connect(this, SIGNAL(editingFinished()), this, SLOT(finished()));
#endif

    }
	~MyQLineEdit() {
		//qDebug() << "line edit destroyed";
	}

#ifdef Q_WS_QWS
    void keyPressEvent(QKeyEvent * e) {
        QLineEdit::keyPressEvent(e);
    }
    void keyReleaseEvent(QKeyEvent * e) {
        QLineEdit::keyReleaseEvent(e);
        if (!e->spontaneous() && !e->isAccepted()) {
            if (e->key() == Qt::Key_Backspace) {
                backspace();
            }
            else if (e->key() == Qt::Key_Return) {
                emit editingFinished();
                emit returnPressed();
            }
        }
    }
#endif
private slots:
    void cleared() {
#if QT_VERSION < 0x050000
        qApp->sendEvent(qApp->inputContext(), new QEvent(QEvent::RequestSoftwareInputPanel));
#endif
    }
    void finished() {
#if QT_VERSION < 0x050000
        qApp->sendEvent(qApp->inputContext(), new QEvent(QEvent::CloseSoftwareInputPanel));
#endif
    }

private:
    QPushButton* m_clearButton;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(BrowserSettings* settings);
	QWebView* webview() { return m_view; }

public slots:
	/*! Toggle page scroll on drag */
	void toggleDragging(bool);

	/*! Changes toolbar visibility according to browsersettings attribute */
	void setToolbarVisibility(int visibility);

	/*! Changes prev / next arrows visibility*/
	void setHistoryButtonsVisible(bool visible);
	
	
	/*! Changes reload / cancel buttons visibility*/
	void setLoadButtonsVisible(bool visible);
	/*! Changes settings button visibility*/
	void setSettingsButtonVisible(bool visible);
	/*! Changes location line edit visibility */
	void setLocationVisible(bool visible);
	/*! Changes cursor state */
	void setCursorVisible(bool visible);
	/*! Changes cursor waiting state */
	void setCursorWaiting(bool waiting);

	/*! Toggle zoom to fit content */
	void toggleZoomToFit(bool);
	void zoomIn();
	void zoomOut();

	/*! Changes zoom buttons visibility */
	void setZoomButtonVisible(bool);

    /*! Changes settings hold time */
	void setSettingsTimeout(int);
	
	/*! Changes scrollbars visibility */
	void setScrollbarsVisible(bool);
	
	/*! The loaded urls are matched against this pattern
     * When matching, the application exits
	 */
	void setExitPattern(const QString& pattern);
	
	void setFullscreen(bool);
protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent *);

protected slots:

	void inactivityTimeout();
	void inactivityFromBacklightOffTimeout();

	/*! Settings view */
	void updateSettingsVisibility();

	/*! Creates the toolbar and show it */
	void createToolBar();


	void loadUrl(const QString& url);
    void adjustLocation();
    void changeLocation();
    void adjustTitle();
    void setProgress(int p);
    void finishLoading(bool);
    void resetProgress();
    void storeUrl(const QUrl& url);

    void viewSource();
    void slotSourceDownloaded();

    void highlightAllLinks();
    void rotateImages(bool invert);
    void removeGifImages();
    void removeInlineFrames();
    void removeObjectElements();
    void removeEmbeddedElements();

	void networkRequestFinished(class QNetworkReply * reply);
#ifdef BASIC_AUTH
    void authenticationRequired(class QNetworkReply *, class QAuthenticator *);
#else
	void autoLoginCompleted();
#endif
	void calcZoom();
	void adjustScrollbars();
	void adjustVerticalScrollBar();

	void onMousePlugEvent(int bPlugged);
	void onBacklightStateChanged(const QString &displayName, int bPlugged);

	void waitForFlagFileFinished();

private:
	bool backlightOn();
	int memAvailableMB();
	void readInitialState();
	void resetInactivityTimers(bool backlightOn);

    //QString m_jQuery;
    QWebView *m_view;
    QLineEdit *m_locationEdit;
    //QAction *rotateAction;
	QAction *m_dragAction, *m_settingsAction;
	QAction* m_zoomInAction, *m_zoomOutAction, *m_zoomToFitAction;
	QWidgetAction *m_locationEditAction;
	QToolBar *m_toolBar;
	int m_progress;
	int m_retryTimer;
	class FlickCharm *m_flickCharm;
	class CustomNetworkAccessManager* m_nman;
	class BrowserSettings* m_settings;
	class LoginForm* m_loginForm;
	class ComExorEpadInterface * m_epad;
	class ComExorBacklightInterface * m_epadBacklight;

#ifndef BASIC_AUTH
	AutoLoginManager * m_autoLoginMan;
#endif

	/*! Show location bar after 3 connection attempts */
	bool m_autoShowLocationBar;
	bool m_loadingInitialPage;
	int m_retryCount;
	int m_authRetries;
	bool m_useDefaultAuth;

	// Show settings logic
	QPoint m_lastPressedPos;
	QTimer m_settingsTimeout;
	bool m_settingsPopup;
	QTimer m_inactivityTimeout;
	QTimer m_inactivityFromBacklightOffTimeout;

	QLabel* m_loader;

	double m_zoomFactor;
	bool m_loadRequested;
	bool m_mousePlugged;
	bool m_forceExit;
	bool m_loadPageOn;

	QFutureWatcher<void> m_watcher;
	QFuture<void> m_future;
	void waitForFlagFile();

#ifdef EXITPATTERN_CHECK_ON_REPLY
	QString m_exitHandler;
#endif
};

#define LOADPAGE_HTML \
"<html><body><div>" \
"<style>" \
    "#svg-preloader {" \
      "display:block; border: 0px ; position:absolute;" \
      "top:0%; left:0%; width:100%; height:100%; background:#555;" \
    "}" \
"</style>" \
\
    "<svg id='svg-preloader' viewBox='-50 -50 612 612' xmlns='http://www.w3.org/2000/svg' >" \
    "<path xmlns='http://www.w3.org/2000/svg' d='M507.592,232.111c0,0-9.296-3.496-36.19-5.977c-32.884-3.04-42.804-15.365-53.742-30.709h-0.22  c-1.362-3.647-2.821-7.248-4.427-10.776l0.127-0.127c-3.122-18.58-4.812-34.316,16.292-59.719  \
c17.25-20.766,21.365-29.818,21.365-29.818c-4.048-10.273-13.781-20.004-13.781-20.004s-9.736-9.734-20.004-13.776  c0,0-9.052,4.104-29.818,21.361c-25.403,21.102-41.14,19.407-59.719,16.292l-0.128,0.126c-3.524-1.605-7.12-3.058-10.775-4.427  \
v-0.214c-15.345-10.934-27.669-20.865-30.71-53.749c-2.485-26.881-5.976-36.189-5.976-36.189C269.757,0,255.997,0,255.997,0  s-13.766,0-23.887,4.405c0,0-3.498,9.309-5.979,36.189c-3.041,32.884-15.372,42.815-30.709,53.749v0.214  \
c-3.647,1.369-7.25,2.821-10.776,4.421l-0.122-0.12c-18.579,3.127-34.316,4.815-59.719-16.285  C104.041,65.322,94.977,61.21,94.977,61.21c-10.273,4.053-19.992,13.771-19.992,13.771S65.25,84.724,61.208,94.99  \
c0,0,4.099,9.053,21.362,29.813c21.1,25.402,19.389,41.139,16.284,59.719l0.128,0.127c-1.607,3.528-3.059,7.129-4.429,10.776H94.34 c-10.927,15.35-20.859,27.669-53.741,30.709c-26.883,2.486-36.202,5.989-36.202,5.989C0.003,242.254,0.003,256,0.003,256  \
s0,13.771,4.405,23.887c0,0,9.297,3.503,36.185,5.978c32.883,3.041,42.803,15.38,53.747,30.71h0.214  \
c1.37,3.655,2.821,7.251,4.429,10.776l-0.128,0.127c3.128,18.582,4.815,34.316-16.284,59.719  c-17.252,20.765-21.368,29.829-21.368,29.829c4.059,10.268,13.775,19.993,13.775,19.993s9.742,9.736,20.01,13.771  \
c0,0,9.047-4.094,29.813-21.357c25.402-21.1,41.145-19.389,59.718-16.288l0.128-0.128c3.526,1.605,7.129,3.052,10.776,4.427v0.22  c15.349,10.928,27.668,20.859,30.709,53.743c2.485,26.881,5.983,36.2,5.983,36.2C242.25,512,255.997,512,255.997,512 \
s13.771,0,23.889-4.405c0,0,3.504-9.298,5.976-36.189c3.041-32.884,15.379-42.805,30.71-53.743v-0.22  c3.655-1.375,7.251-2.821,10.775-4.427l0.128,0.128c18.579-3.122,34.315-4.812,59.719,16.288 \
c20.767,17.253,29.832,21.369,29.832,21.369c10.267-4.06,19.99-13.782,19.99-13.782s9.733-9.736,13.781-20.004  c0,0-4.104-9.054-21.365-29.818c-21.104-25.402-19.403-41.137-16.292-59.719l-0.127-0.127c1.605-3.525,3.064-7.121,4.427-10.776  \
h0.22c10.928-15.341,20.858-27.669,53.742-30.71c26.881-2.485,36.201-5.978,36.201-5.978c4.395-10.127,4.395-23.887,4.395-23.887  S511.997,242.229,507.592,232.111z M255.997,375.727c-66.125,0-119.728-53.602-119.728-119.727s53.603-119.727,119.728-119.727  \
c66.124,0,119.727,53.602,119.727,119.727S322.121,375.727,255.997,375.727z' fill='#eee'/>" \
    "</svg>" \
\
"</div></body></html>" \
