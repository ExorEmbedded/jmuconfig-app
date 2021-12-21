#include <QtGui>

#define OS_INPUT_PANEL // use exor default input panel via dbus

#include "mainwindow.h"
#include "version.h"
#include <QSettings>


#ifdef Q_WS_QWS
#include <QWSSercdver>
#endif

// Global browser settings shared by all application classes
extern QSettings* browserSettings;

#if QT_VERSION < 0x050000

#ifdef OS_INPUT_PANEL
#include <InputPanel/InputPanelContext.h>
#else
#include <inputpanel/myinputpanelcontext.h>
#endif

#include <QWebView>
class WebkitInputPanelContextHook : public InputPanelContextHook
{
public:
	WebkitInputPanelContextHook(QWebView* view) : m_view(view) {}
	QRect inputMethodQuery(QWidget* w, Qt::InputMethodQuery query )
	{
		Q_UNUSED(query);

		if (w == m_view && m_view &&  m_view->page()) {
			return m_view->page()->inputMethodQuery(Qt::ImMicroFocus).toRect();
		}
		return QRect();
	}

private:
	QPointer<QWebView> m_view;
};
#endif

// Request SIP in one click
#include <QProxyStyle>
class RequestSoftwareInputPanelProxyStyle : public QProxyStyle
{
  public:
    int styleHint(StyleHint hint, const QStyleOption *option = 0,
                  const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
    {
        if (hint == QStyle::SH_RequestSoftwareInputPanel)
            return QStyle::RSIP_OnMouseClick;
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

int main(int argc, char * argv[])
{
#ifdef Q_WS_QWS
	qDebug() << "qws!!!";
	QWSServer::setBackground(Qt::black);
#endif

	QApplication app(argc, argv);

    // cursor is initially blank
    app.setOverrideCursor(Qt::BlankCursor);

	if (app.arguments().contains("-h")) {
		qDebug() << "./fancybrowser [url]\n"
					"\nOptions:\n"
					"[-f sets fullscreen mode]\n"
					"[-nf unsets fullscreen mode]\n"
					"[-l hides location bar]\n"
					"[-nl shows location bar]\n"
					"[-g enables gestures]\n"
					"[-ng disables gestures]\n"
					"[-w enable wheel workaround (dragging are translated to mouse wheel events)]\n"
					"[-nw disable wheel workaround (dragging are translated to mouse wheel events)]\n"
					"[-z show zoom controls]\n"
					"[-nz hides zoom controls]\n"
					"[-zf sets zoom to fit]\n"
					"[-nzf unsets zoom to fit]\n"
					"[-k <path_to_lock_file> tries to lock given file or exit (used to achieve single instace mode)]\n"
					"[-x <exit_url_patter> check if given pattern matches current url to quit apps]\n"
					"[-X exits if authentication fails]\n"
					"[-y lock configuration]\n"
					"[-h prints this message]\n"
					"[-u <defualt_user_name> default user name]\n"
					"[-p <default_password> default password]\n"
					"[-ck enable cookies]\n"
					"[-st <seconds> set settings dialog hold timeout]\n"
					"[-wff <file> wait for provided flag file for synchronization]\n"
#ifdef linux
					"[-r <nice level> renice application]\n"
#endif
					"[-ll show loading icon\n"
					"[-i <userAgent> browser identification]\n";
		return 0;
	}

#if QT_VERSION < 0x050000
#ifdef OS_INPUT_PANEL
	InputPanelContext ic(&app);
	app.setInputContext(&ic);
	app.setAutoSipEnabled(true);
#else
	MyInputPanelContext *ic = new MyInputPanelContext;
	app.setInputContext(ic);
#endif
#endif

	BrowserSettings settings;

	MainWindow *browser = new MainWindow(&settings);

#if QT_VERSION < 0x050000
	WebkitInputPanelContextHook* hook = new WebkitInputPanelContextHook(browser->webview());
	ic.registerInputPanelContextHook(hook);
#endif

	app.setStyle(new RequestSoftwareInputPanelProxyStyle);

	app.setApplicationVersion(WEBKITBROWSER_VERSION);

#if defined Q_OS_SYMBIAN || defined Q_WS_HILDON || defined Q_WS_MAEMO_5 || defined Q_WS_SIMULATOR
	browser->show();//FullScreen();
#else
	browser->show();//FullScreen();
#endif
	return app.exec();
}

