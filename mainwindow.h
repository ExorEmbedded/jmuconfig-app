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
    void authenticationRequired(class QNetworkReply *, class QAuthenticator *);
	void calcZoom();
	void adjustScrollbars();
	void adjustVerticalScrollBar();

	void onMousePlugEvent(int bPlugged);
	void onBacklightStateChanged(const QString &displayName, int bPlugged);

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
	
	/*! Show location bar after 3 connection attempts */
	bool m_autoShowLocationBar;
	bool m_loadingInitialPage;
	bool m_useDefaultAuth;
	int m_retryCount;
	int m_authRetries;
	
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
};
