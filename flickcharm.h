#ifndef FLICKCHARM_H
#define FLICKCHARM_H

#include <QObject>

class FlickCharmPrivate;
class QWidget;

class FlickCharm: public QObject
{
    Q_OBJECT
public:
    FlickCharm(QObject *parent = 0);
    ~FlickCharm();	
	/*! Wheel on drag workaround sends wheel events when a dragging 
	 * is started on a web element intercepting wheel events. This
	 * workaround allows to scroll-by-drag on a div with overflow:scroll
	 * by can have unexpected results on a webview intercepting mouse
	 * wheel events like google maps
	 */
	void activateOn(QWidget *widget, bool whellOnDragWorkaround=false);
	void activateSubWidgetOn(QWidget *subWidget, QWidget *widget);
    void deactivateFrom(QWidget *widget);
    bool eventFilter(QObject *object, QEvent *event);

protected:
    void timerEvent(QTimerEvent *event);

private:
    FlickCharmPrivate *d;
};

#endif // FLICKCHARM_H
