#include <QtCore>
#if QT_VERSION < 0x050000
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>
#ifdef 	HAS_WEBKIT
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QWebElementCollection>
#endif
#include "InputPanelProxy.h"
#include "InputPanelContext.h"

//! [0]

bool InputPanelContext::m_wmSupportOffscreenWindows = true;


InputPanelContext::InputPanelContext(QObject* parent)
#if QT_VERSION >= 0x050000
	: QPlatformInputContext()
	  #else
	: QInputContext(parent)
	#endif
	, m_opened(false)
	, m_enabled(true)
	, m_useX11Impl(true)
#ifdef 	HAS_WEBKIT
	, m_focusElement(NULL)
#endif
{
#if QT_VERSION < 0x050000
	// try to load ibus plugin
#endif

	inputPanel = new InputPanelProxy(parent);
	connect(inputPanel, SIGNAL(closed()), SLOT(closed()));
}

//! [0]

InputPanelContext::~InputPanelContext()
{
	
}

void InputPanelContext::sendKeyPressed(int key)
{
#if QT_VERSION >= 0x050000
	if (m_proxied) {
		if ( QMetaObject::invokeMethod(m_proxied, "x11FilterEvent", Q_ARG(uint, key), Q_ARG(uint, key), Q_ARG(uint, 0), Q_ARG(bool, true)) == true )
			return;
	}
#endif
	// Update the focus widget is changed to a new valid one
	if (qApp->focusWidget() && qApp->focusWidget() != m_focusWidget)
	{
		//qDebug() << "Changing focus widget to " << qApp->focusWidget();
		m_focusWidget = qApp->focusWidget();
	}

	if (!m_focusWidget)
		return;

#ifdef 	HAS_WEBKIT
	if (strcmp(m_focusWidget->metaObject()->className(), "QWebView") == 0)
	{
		//qDebug()<< "Checking HTML focused element";
		QWebView* webView = (QWebView*)m_focusWidget.data();
		QWebElement document=webView->page()->currentFrame()->documentElement();
		QWebElementCollection elements=document.findAll("input,select,option");
		QWebElementCollection::iterator iter=elements.begin();
		while(iter!=elements.end())
		{
			QWebElement elem=*iter;
			if(elem.hasFocus())
			{
				// The "elem" object has HTML focus
				//qDebug()<< "HTML element focused " << elem.localName();
				m_focusElement = &elem;
				break;
			}
			
			iter++;
		}		

		if (m_focusElement)
			m_focusElement->setFocus();
	}
	else {
		m_focusElement = NULL;
	}
#endif
	
	int characterChar = key;
	QString characterString = characterChar > 65536 ? QString("") : QString(key);

	QKeyEvent keyPress(QEvent::KeyPress, characterChar, Qt::NoModifier, characterString, false, 1);

	QApplication::sendEvent(m_focusWidget, &keyPress);
}

void InputPanelContext::sendKeyReleased(int key)
{
#if QT_VERSION >= 0x050000
	if (m_proxied) {
		if ( QMetaObject::invokeMethod(m_proxied, "x11FilterEvent", Q_ARG(uint, key), Q_ARG(uint, key), Q_ARG(uint, 0), Q_ARG(bool, false)) == true)
			return;
	}
#endif

		if (!m_focusWidget)
		return;

	int characterChar = key;
	QString characterString = characterChar > 65536 ? QString("") : QString(key);

	QKeyEvent keyPress(QEvent::KeyRelease, characterChar, Qt::NoModifier, QString(), false, 0);

	QApplication::sendEvent(m_focusWidget, &keyPress);
}


//! [1]
#ifdef WINCE
#include <windows.h>
#include "sipapi.h"
#endif

#ifdef Q_WS_X11
#include <QX11Info>

#include <X11/Xlib.h>
#endif

bool InputPanelContext::filterEvent(const QEvent* event)
{
	if (event->type() == QEvent::RequestSoftwareInputPanel) {
#ifdef Q_WS_WINCE
        SIPINFO si;
        memset(&si, 0, sizeof(SIPINFO));
        si.cbSize = sizeof(SIPINFO);
        SipGetInfo(&si);
        int kbHeight = si.rcSipRect.bottom - si.rcSipRect.top;
        int kbWidth = si.rcSipRect.right - si.rcSipRect.left;
        int GXRES = GetSystemMetrics(SM_CXSCREEN);
        int GYRES = GetSystemMetrics(SM_CYSCREEN);


        si.fdwFlags = SIPF_ON ;//SIPF_DOCKED;
        si.cbSize =  sizeof(SIPINFO);
        si.rcSipRect.left = GXRES-kbWidth-40;//rc.left;
        si.rcSipRect.top = GYRES-kbHeight-40;//rc.bottom;
        si.rcSipRect.right = GXRES-40;//ptLowerRight.x;//
        si.rcSipRect.bottom = GYRES-40;//ptLowerRight.y + kbHeight;
        SipSetInfo(&si);
#else

		if(!m_enabled)
			return false;
		// avoid multiple opens
		if (m_opened)
			return true;
#if QT_VERSION < 0x050000
		m_focusWidget = focusWidget();
#else
		if (!m_focusWidget)
			m_focusWidget = qobject_cast<QWidget*>(qApp->focusWidget());
		if (!m_focusWidget)
			return false;
#endif

		int x=0;
		int y=0;
		int width=-1;
		int height=-1;
		QRect rect;

		if (m_focusWidget) {
			rect = m_focusWidget->rect();

			// remove old hooks
			m_hooks.removeAll(QPointer<InputPanelContextHook>(NULL));
			for(int i=0; i<m_hooks.size(); i++) {
				InputPanelContextHook* hook = m_hooks.at(i);
				QRect preciseRect = hook->inputMethodQuery(m_focusWidget, Qt::ImMicroFocus);
				if (!preciseRect.isNull())
					rect = preciseRect;
			}

#ifdef HAS_WEBKIT
			QWebView* view = dynamic_cast<QWebView*>(m_focusWidget.data());
			if (view) {
				rect = view->page ()->inputMethodQuery(Qt::ImMicroFocus).toRect();
			}
#endif

			rect = QRect(m_focusWidget->mapToGlobal(rect.topLeft()), m_focusWidget->mapToGlobal(rect.bottomRight()));
			x = rect.x();
			y = rect.y();

			width = rect.width();
			height = qMax(10, rect.height());
		}

		if ( m_useX11Impl ) {
			inputPanel->showX11FullScreen(language(), x, y+20, width, height);

			// If the x11 implementation is not available fallback to the old one
			if ( inputPanel->lastError().type() ==  QDBusError::UnknownMethod ) {

				qWarning() << "Using showFullScreen vkeyboard IMPL";

				connect(inputPanel, SIGNAL(keyPressed(int)), SLOT(sendKeyPressed(int)));
				connect(inputPanel, SIGNAL(keyReleased(int)), SLOT(sendKeyReleased(int)));
				//    connect(inputPanel, SIGNAL(characterGenerated(int)), SLOT(sendCharacter(int)));

				m_useX11Impl = false;
			}
		}

		if( !m_useX11Impl ) {

			if (m_focusWidget) {
				// Get the toplevel parent widget
				m_parent = m_focusWidget;
				while(true) {
					if (m_parent->windowFlags() & Qt::Dialog)
						break;
					if (m_parent->parentWidget()) {
						m_parent = m_parent->parentWidget();
					} else {
						break;
					}
				}
				// ensures the text box is visible
				int midH = qApp->desktop()->size().height()/2;
				int dy = qMax(0, qMin( rect.top() + 40 - midH, midH ));

				// Save original positions
				m_origPos = m_parent->pos();
				m_newPos.setX(m_origPos.x());
				m_newPos.setY(m_origPos.y() - dy);

				m_offscreenMove = false; // flag set by using the x11 workaroud
				if (m_wmSupportOffscreenWindows) {
					m_parent->move(m_newPos);
					qApp->sendPostedEvents(m_parent, 0); // process the move event to skip first move event
					m_parent->removeEventFilter(this);
					m_parent->installEventFilter(this);
					// check if is actually supported
				} else {
					moveNoWM();
				}
			}

			inputPanel->showFullScreen(language());
		}

#if 0
		inputPanel->show(language(), x, y, width, height);
#endif

		m_opened = true;

#endif
        return true;
    } else if (event->type() == QEvent::CloseSoftwareInputPanel) {
		closed();
        return true;
	} else if (event->type() == QEvent::Timer) {
		QTimerEvent* te = (QTimerEvent*)(event);
		killTimer(te->timerId());
		if (m_parent) {
#ifdef Q_WS_X11
			// Reset the override_redirct flag
			XSetWindowAttributes attrs;
			attrs.override_redirect = 0;
			XChangeWindowAttributes(m_parent->x11Info().display(), m_parent->winId(), CWOverrideRedirect, &attrs);
			XMapWindow(m_parent->x11Info().display(), m_parent->winId());
#endif
		}
	}
#if QT_VERSION >= 0x050000
	else if (m_proxied)
		return m_proxied->filterEvent(event);
	else 
		return QPlatformInputContext::filterEvent(event);
#endif
	return false;
}

void InputPanelContext::closed()
{
#ifdef Q_WS_WINCE
	SIPINFO si;
	memset(&si, 0, sizeof(SIPINFO));
	si.cbSize = sizeof(SIPINFO);
	SipGetInfo(&si);
	int kbHeight = si.rcSipRect.bottom - si.rcSipRect.top;
	int kbWidth = si.rcSipRect.right - si.rcSipRect.left;
	int GXRES = GetSystemMetrics(SM_CXSCREEN);
	int GYRES = GetSystemMetrics(SM_CYSCREEN);


	si.fdwFlags = SIPF_OFF ;//SIPF_DOCKED;
	si.cbSize =  sizeof(SIPINFO);
	si.rcSipRect.left = GXRES-kbWidth-40;//rc.left;
	si.rcSipRect.top = GYRES-kbHeight-40;//rc.bottom;
	si.rcSipRect.right = GXRES-40;//ptLowerRight.x;//
	si.rcSipRect.bottom = GYRES-40;//ptLowerRight.y + kbHeight;
	SipSetInfo(&si);
#else
	m_opened = false;

	if( !m_useX11Impl ) {
		if (m_parent) {
			m_parent->removeEventFilter(this);
			if (m_offscreenMove) {
				m_newPos = m_origPos;
				moveNoWM();
			} else {
				m_parent->move(m_origPos);
			}
			m_parent->show();
			m_parent = NULL;
		}
	}

	inputPanel->hide();
#endif
}


void InputPanelContext::moveNoWM()
{
#ifdef Q_WS_X11
	if (m_parent->windowFlags() & Qt::FramelessWindowHint) { //
		m_offscreenMove = true;

		// Set override_redict flag to move without interference by the windows manager
		XSetWindowAttributes attrs;
		attrs.override_redirect = 1;
		XChangeWindowAttributes(m_parent->x11Info().display(), m_parent->winId(), CWOverrideRedirect, &attrs);
		XMoveWindow(m_parent->x11Info().display(), m_parent->winId(), m_newPos.x(), m_newPos.y() );
		XMapWindow(m_parent->x11Info().display(), m_parent->winId());
		// Starts a timer to immediately revert the override_redirect flag
		//startTimer(0);
	} else {
		m_parent->move(m_newPos);
	}
#endif
}



bool InputPanelContext::event(QEvent * event)
{
	return filterEvent(event);
}

bool InputPanelContext::eventFilter(QObject *, QEvent * e)
{
	if (e->type() == QEvent::Show) {
	} else if (/*e->type() == QEvent::Resize ||*/ e->type() == QEvent::Move ) {
		if (m_parent->pos() != m_newPos) {
			// WM changed the actual widget position -> need to try a workaroud
			qDebug() << "WM does not support offscreen windows properly -> try workaround method!";
			m_wmSupportOffscreenWindows = false;
			moveNoWM();
			m_parent->removeEventFilter(this);
		}
	}
	return false;
}


//! [1]

QString InputPanelContext::language()
{
	// Use system wide language settings
	return "";
}

#if QT_VERSION < 0x050000
QString InputPanelContext::identifierName()
{
	return "InputPanelContext";
}

bool InputPanelContext::isComposing() const
{
    return false;
}

void InputPanelContext::reset()
{
	//qDebug() << __FUNCTION__;
}
#endif


#if QT_VERSION >= 0x050000
void InputPanelContext::showInputPanel()
{
	QEvent event( QEvent::RequestSoftwareInputPanel );
	filterEvent(&event);
	emitInputPanelVisibleChanged();
}

void InputPanelContext::hideInputPanel()
{
	QEvent event( QEvent::CloseSoftwareInputPanel );
	filterEvent(&event);
	emitInputPanelVisibleChanged();
}

void InputPanelContext::setBackendInputContext(QPlatformInputContext *im)
{
	if (m_proxied == NULL && im) {
		m_proxied = im;
	}
}
#endif

bool InputPanelContext::isInputPanelVisible() const
{
	return m_opened;	
}

void InputPanelContext::setFocusObject(QObject *object)
{
	m_focusWidget = qobject_cast<QWidget*>(object);
}


bool InputPanelContext::isValid() const
{
	return true;
}

#if QT_VERSION >= 0x050000
bool InputPanelContext::hasCapability(QPlatformInputContext::Capability capability) const
{
	if (m_proxied)
		return m_proxied->hasCapability(capability);
	return QPlatformInputContext::hasCapability(capability);
}

void InputPanelContext::reset()
{
	if (m_proxied) {
		m_proxied->reset();
		return;
	}
	QPlatformInputContext::reset();
}

void InputPanelContext::commit()
{
	if (m_proxied) {
		m_proxied->commit();
		return;
	}
	QPlatformInputContext::commit();
}

void InputPanelContext::update(Qt::InputMethodQueries query)
{
	if (query == Qt::ImPlatformData) {
		QInputMethodQueryEvent ev(Qt::ImPlatformData);
		QVariantMap currentStatus;
		QVariantList currentHooks;
		for(int i=0; i<m_hooks.size(); i++) {
			QVariant v;
			v.setValue<InputPanelContextHook*>(m_hooks.at(i));
			currentHooks.append(v);
		}
		currentStatus["inputPanelHooks"] = currentHooks;
		currentStatus["inputPanelEnabled"] = m_enabled;
		ev.setValue(Qt::ImPlatformData, currentStatus);
		
		// Query current application settings
		qApp->sendEvent(qApp, &ev);

		const QVariantMap& data = ev.value(Qt::ImPlatformData).toMap();
		const QVariantList& hooks = data["inputPanelHooks"].toList();
		m_hooks.clear();
		for(int i=0; i<hooks.size(); i++) {
			m_hooks.append(QPointer<InputPanelContextHook>((InputPanelContextHook*)hooks.at(i).value<InputPanelContextHook*>()));
		}
		m_enabled = data["inputPanelEnabled"].toBool();
	}
	if (m_proxied) {
		m_proxied->update(query);
		return;
	}
	QPlatformInputContext::update(query);
}

void InputPanelContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
	if (m_proxied) {
		m_proxied->invokeAction(action, cursorPosition);
		return;
	}
	QPlatformInputContext::invokeAction(action, cursorPosition);
}

QRectF InputPanelContext::keyboardRect() const
{
	return QPlatformInputContext::keyboardRect();
}

bool InputPanelContext::x11FilterEvent(uint keyval, uint keycode, uint state, bool pressed)
{
	if (m_proxied) {
		return QMetaObject::invokeMethod(m_proxied, "x11FilterEvent", Q_ARG(uint, keyval), Q_ARG(uint, keycode), Q_ARG(uint, state), Q_ARG(bool, pressed));
	}
	return false;	
}

#endif
#endif


//! [2]

