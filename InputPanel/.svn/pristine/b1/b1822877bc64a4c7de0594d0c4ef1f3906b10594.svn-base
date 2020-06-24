#ifndef MYINPUTPANELCONTEXT_H
#define MYINPUTPANELCONTEXT_H

#include <QObject>

#include <QtCore/qglobal.h>
#if defined(INPUTPANELCONTEXTPLUGINSHARED_LIBRARY)
#  define INPUTPANELCONTEXTPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define INPUTPANELCONTEXTPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#if QT_VERSION >= 0x050000
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextplugin_p.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <QtPlugin>
#include <QPoint>

#else

#include <QInputContext>
#endif
#include <QPointer>
#ifdef 	HAS_WEBKIT
#include <QWebElement>
#endif



class InputPanelProxy;

/*!
 * \brief The InputPanelContextHook class
 * Allow to decouple webkit widgets.
 */
class INPUTPANELCONTEXTPLUGINSHARED_EXPORT InputPanelContextHook : public QObject
{
public:
	InputPanelContextHook(QObject* parent = NULL) : QObject(parent) {}
	/*! This hook might provide an alternative input rect for the given widget */
	virtual QRect inputMethodQuery(QWidget* w, Qt::InputMethodQuery query ) = 0;
};

 Q_DECLARE_METATYPE(InputPanelContextHook*);

class InputPanelContext 
		#if QT_VERSION >= 0x050000
		: public QPlatformInputContext
		#else
		: public QInputContext
		#endif
{
    Q_OBJECT
	
public:
	Q_PROPERTY(bool enable  WRITE enable)
	InputPanelContext(QObject* parent = 0);
	virtual ~InputPanelContext();
	virtual bool filterEvent(const QEvent *event);

#if QT_VERSION >= 0x050000

	virtual bool isValid() const;
    virtual bool hasCapability(Capability capability) const;

    virtual void reset();
    virtual void commit();
    virtual void update(Qt::InputMethodQueries);
    virtual void invokeAction(QInputMethod::Action, int cursorPosition);
    virtual QRectF keyboardRect() const;
    //void emitKeyboardRectChanged();

    //virtual bool isAnimating() const;
    //void emitAnimatingChanged();

    virtual void showInputPanel();
    virtual void hideInputPanel();
    virtual bool isInputPanelVisible() const;
    //void emitInputPanelVisibleChanged();

    //virtual QLocale locale() const;
    //void emitLocaleChanged();
    //virtual Qt::LayoutDirection inputDirection() const;
    //void emitInputDirectionChanged(Qt::LayoutDirection newDirection);

    virtual void setFocusObject(QObject *object);
    //bool inputMethodAccepted() const;
	
	/*! Uses an external input context as proxy (for instance ibus)
	 * This function fails if the proxy is already set != 0, so only 1
	 * call is required
	 */
	void setBackendInputContext(QPlatformInputContext* im);
#else
	virtual bool isValid() const;
	virtual bool isInputPanelVisible() const;
	virtual void setFocusObject(QObject *object);

	QString identifierName();
	bool isComposing() const;
	virtual void reset();
#endif	
	
    bool event(QEvent *);
	bool eventFilter(QObject *, QEvent *);
	void enable(bool bEnable){ m_enabled = bEnable;}

	QString language();

public slots:
	/*! Sets a new hook for this input panel context */
	void registerInputPanelContextHook(InputPanelContextHook* hook) { m_hooks.push_back(hook); }
	
#if QT_VERSION >= 0x050000
	bool x11FilterEvent(uint keyval, uint keycode, uint state, bool pressed);
#endif

private slots:
//    void sendCharacter(int character);
	void sendKeyPressed(int character);
	void sendKeyReleased(int character);
	void closed();

private:
	/*! Move the parent widget bypassing the window manager
	 * when window manager does not support moving windows offscreen */
	void moveNoWM();
private:
	InputPanelProxy *inputPanel;
	QPointer<QWidget> m_focusWidget;
	QPointer<QWidget> m_parent;
#ifdef HAS_WEBKIT
	QWebElement* m_focusElement;
#endif

	QPoint m_origPos;
	QPoint m_newPos;
	bool m_offscreenMove;
	bool m_opened;
	bool m_enabled;
	static bool m_wmSupportOffscreenWindows;
	bool m_useX11Impl;

	QList<QPointer<InputPanelContextHook> > m_hooks;
#if QT_VERSION >= 0x050000
	QPointer<QPlatformInputContext> m_proxied;
#endif
};

//! [0]

#if QT_VERSION >= 0x050000
class INPUTPANELCONTEXTPLUGINSHARED_EXPORT  InputPanelContextPlugin : public QPlatformInputContextPlugin
{	
	Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "InputPanelContextPlugin.json")
public:
    QPlatformInputContext *create(const QString& system, const QStringList& paramList)
	{
		Q_UNUSED(paramList);
		
		if (system.compare(system, QStringLiteral("exor"), Qt::CaseInsensitive) == 0) {
			InputPanelContext* ctx = new InputPanelContext;
			for(int i=0; i<paramList.size(); i++) {
				const QString& backendInputContext = paramList.at(i);
				QPlatformInputContext* backend = QPlatformInputContextFactory::create(backendInputContext);
				if (backend) {
					ctx->setBackendInputContext(backend);
					break;
				}
			}		
			return ctx;
		}
		return 0;		
	}
	
};

//Q_IMPORT_PLUGIN (InputPanelContextPlugin);
#endif



#endif // MYINPUTPANELCONTEXT_H
