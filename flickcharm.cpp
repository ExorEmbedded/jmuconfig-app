#include "flickcharm.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBasicTimer>
#include <QEvent>
#include <QHash>
#include <QList>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTime>
#include <QWebFrame>
#include <QWebView>

#include <QDebug>

const int fingerAccuracyThreshold = 10;

struct FlickData {
	typedef enum {
		Steady, // Interaction without scrolling
		ManualScroll, // Scrolling manually with the finger on the screen
		AutoScroll, // Scrolling automatically
		AutoScrollAcceleration // Scrolling automatically but a finger is on the screen
	} State;
	State state;
	QWidget *widget;
	QPoint pressPos;
	QPoint lastPos;
	QPoint speed;
	QTime speedTimer;
	QList<QEvent*> ignored;
	QTime accelerationTimer;
	bool lastPosValid:1;
	bool waitingAcceleration:1;
	bool scrollEventWorkaround:1;
	
	FlickData()
		: lastPosValid(false)
		, waitingAcceleration(false)
	{}
	
	void resetSpeed()
	{
		speed = QPoint();
		lastPosValid = false;
	}
	void updateSpeed(const QPoint &newPosition)
	{
		if (lastPosValid) {
			const int timeElapsed = speedTimer.elapsed();
			if (timeElapsed) {
				const QPoint newPixelDiff = (newPosition - lastPos);
				const QPoint pixelsPerSecond = newPixelDiff * (1000 / timeElapsed);
				// fingers are inacurates, we ignore small changes to avoid stopping the autoscroll because
				// of a small horizontal offset when scrolling vertically
				const int newSpeedY = (qAbs(pixelsPerSecond.y()) > fingerAccuracyThreshold) ? pixelsPerSecond.y() : 0;
				const int newSpeedX = (qAbs(pixelsPerSecond.x()) > fingerAccuracyThreshold) ? pixelsPerSecond.x() : 0;
#ifdef FLICKCHARM_ACCELERATION				
				if (state == AutoScrollAcceleration) {
					const int max = 4000; // px by seconds
					const int oldSpeedY = speed.y();
					const int oldSpeedX = speed.x();
					if ((oldSpeedY <= 0 && newSpeedY <= 0) ||  (oldSpeedY >= 0 && newSpeedY >= 0)
							&& (oldSpeedX <= 0 && newSpeedX <= 0) ||  (oldSpeedX >= 0 && newSpeedX >= 0)) {
						speed.setY(qBound(-max, (oldSpeedY + (newSpeedY / 4)), max));
						speed.setX(qBound(-max, (oldSpeedX + (newSpeedX / 4)), max));
					} else {
						speed = QPoint();
					}
				}
				else 
#endif
				{
					const int max = 2500; // px by seconds
					// we average the speed to avoid strange effects with the last delta
					if (!speed.isNull()) {
						speed.setX(qBound(-max, (speed.x() / 4) + (newSpeedX * 3 / 4), max));
						speed.setY(qBound(-max, (speed.y() / 4) + (newSpeedY * 3 / 4), max));
					} else {
						speed = QPoint(newSpeedX, newSpeedY);
					}
				}
			}
		} else {
			lastPosValid = true;
		}
		speedTimer.start();
		lastPos = newPosition;
	}
	
	// scroll by dx, dy
	// return true if the widget was scrolled
	bool scrollWidget(const int dx, const int dy, const QPoint& hotPoint = QPoint())
	{
		QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
		if (scrollArea) {
			const int x = scrollArea->horizontalScrollBar()->value();
			const int y = scrollArea->verticalScrollBar()->value();
			scrollArea->horizontalScrollBar()->setValue(x - dx);
			scrollArea->verticalScrollBar()->setValue(y - dy);
			return (scrollArea->horizontalScrollBar()->value() != x
					|| scrollArea->verticalScrollBar()->value() != y);
		}
		
		
		QWebView *webView = qobject_cast<QWebView*>(widget);
		if (webView) {
			QWebFrame *frame = webView->page()->mainFrame();
			const QPoint position = frame->scrollPosition();
			bool rc = false;
			if (!scrollEventWorkaround) { 
				// Map drag events into scroll events
				// this won't work with google maps or other ups intercepting scroll events
				frame->setScrollPosition(position - QPoint(dx, dy));
				rc = frame->scrollPosition() != position;
			} else {
				// frame->setScrollPosition(position - QPoint(dx, dy));
				if (dy) {
					// post a fake wheel event to webpage to allow div scrolling => if event is ignored scroll the webview
					QWheelEvent scrollEventVertical (hotPoint, 2*dy + !(dy % 120),  Qt::NoButton, Qt::NoModifier, Qt::Vertical);
					scrollEventVertical.setAccepted(false);
					bool rc = qApp->sendEvent(webView->page(), &scrollEventVertical);
					if (! scrollEventVertical.isAccepted() ) {
						// event not accepted by page (scroll on page divs) => try a view scroll
						frame->setScrollPosition(position - QPoint(0, dy));
						rc = frame->scrollPosition() != position;
					}
				}
				if (dx) {
					// post a fake wheel event to webpage to allow div scrolling => if event is ignored scroll the webview
					QWheelEvent scrollEventHorizontal (hotPoint, 2*dx + !(dx % 120),  Qt::NoButton, Qt::NoModifier, Qt::Horizontal);
					scrollEventHorizontal.setAccepted(false);
					bool rc = qApp->sendEvent(webView->page(), &scrollEventHorizontal);
					if (! scrollEventHorizontal.isAccepted() ) {
						// event not accepted by page (scroll on page divs) => try a view scroll
						//frame->setScrollPosition(position - QPoint(dx, 0));
						rc = frame->scrollPosition() != position;
					}
				}
			}
			return rc;				
		}
		return false;
	}
	
	bool scrollTo(const QPoint &newPosition)
	{
		const QPoint delta = newPosition - lastPos;
		updateSpeed(newPosition);
		return scrollWidget(delta.x(), delta.y(), pressPos);
	}
};

class FlickCharmPrivate
{
public:
	QHash<QWidget*, FlickData*> flickData;
	QHash<QWidget*, QWidget*> subwidgetsMap; /* subwidget -> parent widget */
	QBasicTimer ticker;
	QTime timeCounter;
	void startTicker(QObject *object)
	{
		if (!ticker.isActive())
			ticker.start(15, object);
		timeCounter.start();
	}
};

FlickCharm::FlickCharm(QObject *parent): QObject(parent)
{
	d = new FlickCharmPrivate;
}

FlickCharm::~FlickCharm()
{
	delete d;
}

void FlickCharm::activateOn(QWidget *widget, bool wheelOnDragWorkaround)
{
	QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
	if (scrollArea) {
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		
		QWidget *viewport = scrollArea->viewport();
		
		viewport->installEventFilter(this);
		scrollArea->installEventFilter(this);
		
		d->flickData.remove(viewport);
		d->flickData[viewport] = new FlickData;
		d->flickData[viewport]->widget = widget;
		d->flickData[viewport]->scrollEventWorkaround = wheelOnDragWorkaround;
		d->flickData[viewport]->state = FlickData::Steady;
		
		return;
	}
	
	QWebView *webView = qobject_cast<QWebView*>(widget);
	if (webView) {
		QWebFrame *frame = webView->page()->mainFrame();
		
		webView->installEventFilter(this);
		
		d->flickData.remove(webView);
		d->flickData[webView] = new FlickData;
		d->flickData[webView]->widget = webView;
		d->flickData[webView]->scrollEventWorkaround = wheelOnDragWorkaround;
		d->flickData[webView]->state = FlickData::Steady;
		
		return;
	}
	
	qWarning() << "FlickCharm only works on QAbstractScrollArea (and derived classes)";
	qWarning() << "or QWebView (and derived classes)";
}

void FlickCharm::deactivateFrom(QWidget *widget)
{
	QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
	if (scrollArea) {
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		
		QWidget *viewport = scrollArea->viewport();
		
		viewport->removeEventFilter(this);
		scrollArea->removeEventFilter(this);
		
		delete d->flickData[viewport];
		d->flickData.remove(viewport);
		
		return;
	}
	
	QWebView *webView = qobject_cast<QWebView*>(widget);
	if (webView) {
		QWebFrame *frame = webView->page()->mainFrame();
		frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
		frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
		
		webView->removeEventFilter(this);
		
		delete d->flickData[webView];
		d->flickData.remove(webView);
		
		return;
	}
}

void FlickCharm::activateSubWidgetOn(QWidget *subWidget, QWidget *widget)
{
	QWidget* parentWidget = NULL;
	QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
	if (scrollArea) {
		parentWidget = scrollArea->viewport();
	}
	if (!d->flickData.contains(parentWidget))
		parentWidget = NULL;

	if (parentWidget) {
		d->subwidgetsMap[subWidget] = parentWidget;
		subWidget->installEventFilter(this);
	}
}

static QPoint deaccelerate(const QPoint &speed, const int deltatime)
{
	const int deltaSpeed = deltatime;
	
	int x = speed.x();
	int y = speed.y();
	x = (x == 0) ? x : (x > 0) ? qMax(0, x - deltaSpeed) : qMin(0, x + deltaSpeed);
	y = (y == 0) ? y : (y > 0) ? qMax(0, y - deltaSpeed) : qMin(0, y + deltaSpeed);
	return QPoint(x, y);
}

bool FlickCharm::eventFilter(QObject *object, QEvent *event)
{
	if (!object->isWidgetType())
		return false;
	
	const QEvent::Type type = event->type();
	
	switch (type) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseMove:
	case QEvent::MouseButtonRelease:
		break;
	case QEvent::MouseButtonDblClick: // skip double click
		return true;
	default:
		return false;
	}
	
	QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
	if (type == QEvent::MouseMove && mouseEvent->buttons() != Qt::LeftButton)
		return false;
	
	if (mouseEvent->modifiers() != Qt::NoModifier)
		return false;
	
	QWidget *viewport = qobject_cast<QWidget*>(object);
	FlickData *data = d->flickData.value(viewport);
	if (data == NULL) {
		// search subwidgets
		viewport = d->subwidgetsMap.value(viewport, NULL);
		if (viewport) {
			data = d->flickData.value(viewport, NULL);
		}
	}
	if (!viewport || !data || data->ignored.removeAll(event))
		return false;
	
	const QPoint mousePos = mouseEvent->globalPos();
	const QPoint relMousePos = mouseEvent->pos();
	bool consumed = false;
	const QPoint delta = mousePos - data->pressPos;
	
	switch (data->state) {
	
	case FlickData::Steady:
		if (type == QEvent::MouseButtonPress) {
			consumed = true;
			data->pressPos = mousePos;
		} else if (type == QEvent::MouseButtonRelease) {
			consumed = true;
			QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,
												  relMousePos, Qt::LeftButton,
												  Qt::LeftButton, Qt::NoModifier);
			QMouseEvent *event2 = new QMouseEvent(QEvent::MouseButtonRelease,
												  relMousePos, Qt::LeftButton,
												  Qt::LeftButton, Qt::NoModifier);
			
			data->ignored << event1;
			data->ignored << event2;
			QApplication::postEvent(object, event1);
			QApplication::postEvent(object, event2);
		} else if (type == QEvent::MouseMove) {
			
			if (qAbs(delta.x()) > fingerAccuracyThreshold || qAbs(delta.y()) > fingerAccuracyThreshold) {
				consumed = true;
				data->scrollTo(mousePos);
				data->state = FlickData::ManualScroll;
			}
		}
		break;
		
	case FlickData::ManualScroll:
		if (type == QEvent::MouseMove) {
			consumed = true;
			data->scrollTo(mousePos);
		} else if (type == QEvent::MouseButtonRelease) {
			consumed = true;
			data->state = FlickData::AutoScroll;
			data->lastPosValid = false;
			d->startTicker(this);
		}
		break;
		
	case FlickData::AutoScroll:
		
		if (type == QEvent::MouseButtonPress) {
			consumed = true;
			data->state = FlickData::AutoScrollAcceleration;
			data->waitingAcceleration = true;
			data->accelerationTimer.start();
			data->updateSpeed(mousePos);
			data->pressPos = mousePos;
		} else if (type == QEvent::MouseButtonRelease) {
			consumed = true;
			data->state = FlickData::Steady;
			data->resetSpeed();
		}
		break;
		
	case FlickData::AutoScrollAcceleration:
		if (type == QEvent::MouseMove) {
			consumed = true;
			data->updateSpeed(mousePos);
			
			data->accelerationTimer.start();
			if (data->speed.isNull())
				data->state = FlickData::ManualScroll;
		} else if (type == QEvent::MouseButtonRelease) {
			consumed = true;
			data->state = FlickData::AutoScroll;
			data->waitingAcceleration = false;
			data->lastPosValid = false;
		}
		
		break;
	default:
		break;
	}
	data->lastPos = mousePos;
	return true;
}

void FlickCharm::timerEvent(QTimerEvent *event)
{
	int count = 0;
	QHashIterator<QWidget*, FlickData*> item(d->flickData);
	while (item.hasNext()) {
		item.next();
		FlickData *data = item.value();
		if (data->state == FlickData::AutoScrollAcceleration
				&& data->waitingAcceleration
				&& data->accelerationTimer.elapsed() > 40) {
			data->state = FlickData::ManualScroll;
			data->resetSpeed();
		}
		if (data->state == FlickData::AutoScroll || data->state == FlickData::AutoScrollAcceleration) {
			const int timeElapsed = d->timeCounter.elapsed();
			const QPoint delta = (data->speed) * timeElapsed / 1000;
			bool hasScrolled = data->scrollWidget(delta.x(), delta.y(), data->pressPos);
			
			if (data->speed.isNull() || !hasScrolled)
				data->state = FlickData::Steady;
			else
				count++;
			data->speed = deaccelerate(data->speed, timeElapsed);
		}
	}
	
	if (!count)
		d->ticker.stop();
	else
		d->timeCounter.start();
	
	QObject::timerEvent(event);
}
