#include "InputPanelProxy.h"

#if QT_VERSION < 0x050000


ComExorVKeyboardInterface::ComExorVKeyboardInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
	: QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ComExorVKeyboardInterface::~ComExorVKeyboardInterface()
{
}


InputPanelProxy::InputPanelProxy(QObject* parent)
	: QObject(parent)
	, m_dbusObject(NULL)
	, m_vkeyboardConnection(NULL)
{
}

void InputPanelProxy::init()
{
	m_vkeyboardConnection = new QDBusConnection(QDBusConnection::sessionBus());
	if (!m_vkeyboardConnection->isConnected()) {
		qWarning() << "Failed connecting to DBUS session bus";
		delete m_vkeyboardConnection;
		m_vkeyboardConnection = NULL;
	}

    // Try connection
    connect(&m_connectRetryTimer, SIGNAL(timeout()), this, SLOT(connectRetry()));
    connectRetry();	
}

InputPanelProxy::~InputPanelProxy()
{
	delete m_dbusObject;
	delete m_vkeyboardConnection;
}

void InputPanelProxy::show(const QString& layout, int x=0, int y=0, int width=0, int height=0)
{
	if (!m_dbusObject) init(); // lazy init
	if (m_dbusObject)
		m_dbusObject->show(layout, x, y, width, height);
}

void InputPanelProxy::showFullScreen(const QString &layout)
{
	if (!m_dbusObject) init(); // lazy init
	if (m_dbusObject)
		m_dbusObject->showFullScreen(layout);
}

void InputPanelProxy::showX11FullScreen(const QString &layout, int x, int y, int width, int height)
{
	if (!m_dbusObject) init(); // lazy init
	if (m_dbusObject)
		m_dbusObject->showX11FullScreen(layout, x, y, width, height);
}

void InputPanelProxy::hide()
{
	if (!m_dbusObject) init(); // lazy init
	if (m_dbusObject)
        m_dbusObject->hide();
}

void InputPanelProxy::connectRetry()
{
    if (m_vkeyboardConnection) {
        // Dummy call to autostart the EPAD service if not running
        m_vkeyboardConnection->call(QDBusMessage::createMethodCall("com.exor.VKeyboard", "/", QString(), "q_autostart"), QDBus::Block, 5000);

        m_dbusObject = new ComExorVKeyboardInterface("com.exor.VKeyboard", "/", *m_vkeyboardConnection);
        if (m_dbusObject->isValid()) {
            connect(m_dbusObject, SIGNAL(keyPressed(int)), this, SIGNAL(keyPressed(int)));
            connect(m_dbusObject, SIGNAL(keyReleased(int)), this, SIGNAL(keyReleased(int)));
            connect(m_dbusObject, SIGNAL(closed()), this, SIGNAL(closed()));
        } else {
            delete m_dbusObject;
            m_dbusObject = NULL;
            qWarning() << "Cannot connect to dbus vkeyboard interface, retry in 5sec";
            m_connectRetryTimer.setSingleShot(true);
            m_connectRetryTimer.start(5000);
        }
    }
}

QDBusError InputPanelProxy::lastError()
{
	return m_dbusObject->lastError();
}

#endif
