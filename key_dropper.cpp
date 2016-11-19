#include "key_dropper.h"
#include "itemscene.h"

#include <QKeyEvent>
#include <QApplication>

KeyDropper::KeyDropper(QWidget *parent) :
    QDockWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(parent);
}

void KeyDropper::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        if(parent())
        {
            ItemScene* s = qobject_cast<ItemScene*>(parent());
            qApp->setActiveWindow(s);
            s->setFocus(Qt::MouseFocusReason);
            s->keyPressEvent(event);
            return;
        }
    default: break;
    }
    QDockWidget::keyPressEvent(event);
}

void KeyDropper::keyReleaseEvent(QKeyEvent *event)
{
    QDockWidget::keyReleaseEvent(event);
}

