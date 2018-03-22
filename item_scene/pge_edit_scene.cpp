
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>

#include "pge_edit_scene.h"

PGE_EditScene::PGE_EditScene(QWidget *parent) :
    QWidget(parent),
    m_mouseMoved(false),
    m_ignoreMove(false),
    m_ignoreRelease(false),
    m_moveInProcess(false),
    m_rectSelect(false),
    m_zoom(1.0),
    m_isBusy(m_busyMutex, std::defer_lock),
    m_abortThread(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    connect(&m_mover.timer,
            &QTimer::timeout,
            this,
            static_cast<void (PGE_EditScene::*)()>(&PGE_EditScene::moveCamera));
}

PGE_EditScene::~PGE_EditScene()
{
    m_tree.clearAndDestroy();
}

PGE_EditSceneItem *PGE_EditScene::addRect(int64_t x, int64_t y)
{
    PGE_EditSceneItem *item = new PGE_EditSceneItem(this);
    item->m_posRect.setRect(x, y, 32, 32);
    registerElement(item);
    return item;
}

void PGE_EditScene::clearSelection()
{
    for(PGE_EditSceneItem *item : m_selectedItems)
        item->m_selected = false;
    m_selectedItems.clear();
    m_selectionRect.reset();
}

void PGE_EditScene::moveSelection(int64_t deltaX, int64_t deltaY)
{
    for(PGE_EditSceneItem *item : m_selectedItems)
    {
        item->m_posRect.moveBy(deltaX, deltaY);
        updateElement(item);
    }
    m_selectionRect.moveBy(deltaX, deltaY);
}

void PGE_EditScene::captureSelectionRect()
{
    if(m_selectedItems.empty())
    {
        m_selectionRect.reset();
        return;
    }

    bool first = true;
    for(PGE_EditSceneItem *item : m_selectedItems)
    {
        if(first)
        {
            m_selectionRect = item->m_posRect;
            first = false;
        }
        else
            m_selectionRect.expandByRect(item->m_posRect);
    }
}

void PGE_EditScene::select(PGE_EditSceneItem &item)
{
    item.m_selected = true;
    m_selectedItems.insert(&item);
}

void PGE_EditScene::deselect(PGE_EditSceneItem &item)
{
    item.m_selected = false;
    m_selectedItems.remove(&item);
}

void PGE_EditScene::toggleselect(PGE_EditSceneItem &item)
{
    item.m_selected = !item.m_selected;
    if(item.m_selected)
        m_selectedItems.insert(&item);
    else
        m_selectedItems.remove(&item);
}

void PGE_EditScene::setItemSelected(PGE_EditSceneItem &item, bool selected)
{
    item.m_selected = selected;
    if(item.m_selected)
        m_selectedItems.insert(&item);
    else
        m_selectedItems.remove(&item);
}

void PGE_EditScene::moveStart()
{
    m_moveInProcess = true;
}

void PGE_EditScene::moveEnd(bool /*esc*/)
{
    m_moveInProcess = false;
}

void PGE_EditScene::startInitAsync()
{
    m_busyMessage = tr("Loading...");
    m_busyIsClosing = false;
    m_isBusy.lock();
    QtConcurrent::run<void>(this, &PGE_EditScene::initThread);
}

void PGE_EditScene::initThread()
{
    if(!m_isBusy.owns_lock())
        m_isBusy.lock();

    bool offset = false;
    for(int y = -1024; y < 32000; y += 32)
        for(int x = -1024; x < 32000; x += 32)
        {
            if(m_abortThread) goto threadEnd;
            addRect(x,  y + (offset ? 16 : 0));
            offset = !offset;
        }
threadEnd:
    m_isBusy.unlock();
    metaObject()->invokeMethod(this, "repaint", Qt::QueuedConnection);
}

void PGE_EditScene::startDeInitAsync()
{
    m_busyMessage = tr("Closing...");
    m_busyIsClosing = true;
    m_isBusy.lock();
    QtConcurrent::run<void>(this, &PGE_EditScene::deInitThread);
}

void PGE_EditScene::deInitThread()
{
    if(!m_isBusy.owns_lock())
        m_isBusy.lock();
    metaObject()->invokeMethod(this, "repaint", Qt::QueuedConnection);

    m_tree.clearAndDestroy();

    m_busyIsClosing = false;
    m_isBusy.unlock();
    metaObject()->invokeMethod(this->parent(), "close", Qt::QueuedConnection);
}

struct _TreeSearchQuery
{
    bool requireChildren;
    PGE_EditScene::PGE_EditItemList *list;
    PGE_Rect<int64_t> *zone;
};

static bool _TreeSearchCallback(PGE_EditSceneItem *item, void *arg)
{
    _TreeSearchQuery* ar = static_cast<_TreeSearchQuery *>(arg);
    PGE_EditScene::PGE_EditItemList *list = ar->list;
    if(list)
    {
        if(item)
        {
            (*list).push_back(item);
            if(ar->requireChildren)
                item->queryChildren(*(ar->zone), _TreeSearchCallback, arg);
        }
    }
    return true;
}

void PGE_EditScene::queryItems(PGE_Rect<int64_t> &zone, PGE_EditScene::PGE_EditItemList *resultList, bool requireChildren)
{
    _TreeSearchQuery query = {requireChildren, resultList, &zone};
    m_tree.query(zone, _TreeSearchCallback, (void*)&query);
}

void PGE_EditScene::queryItems(int64_t x, int64_t y, PGE_EditScene::PGE_EditItemList *resultList, bool requireChildren)
{
    PGE_Rect<int64_t> z(x, y, 1, 1);
    _TreeSearchQuery query = {requireChildren, resultList, &z};
    m_tree.query(z, _TreeSearchCallback, (void*)&query);
}

void PGE_EditScene::registerElement(PGE_EditSceneItem *item)
{
    m_tree.insert(item);
}

void PGE_EditScene::updateElement(PGE_EditSceneItem *item)
{
    m_tree.update(item);
}

void PGE_EditScene::unregisterElement(PGE_EditSceneItem *item)
{
    m_tree.remove(item);
}






QPointF PGE_EditScene::mapToWorld(const QPointF &mousePos)
{
    QPointF w = mousePos;
    w.setX(std::round(qreal(w.x()) / m_zoom));
    w.setY(std::round(qreal(w.y()) / m_zoom));
    w += m_cameraPos;
    return w;
}

QRectF PGE_EditScene::applyZoom(const QRectF &r)
{
    QRectF t = r;
    t.moveTo(t.topLeft() - m_cameraPos);
    t.moveTo(std::round(qreal(t.x()) * m_zoom), std::round(qreal(t.y()) * m_zoom));
    t.setWidth(std::round(qreal(t.width()) * m_zoom));
    t.setHeight(std::round(qreal(t.height()) * m_zoom));
    return t;
}

void PGE_EditScene::deleteItem(PGE_EditSceneItem *item)
{
    if(item->m_selected)
    {
        m_selectedItems.remove(item);
        m_selectionRect.reset();
    }
    m_tree.removeAndDestroy(item);
}

void PGE_EditScene::deleteSelectedItems()
{
    for(PGE_EditSceneItem *item : m_selectedItems)
        m_tree.removeAndDestroy(item);
    m_selectedItems.clear();
    m_selectionRect.reset();
}

bool PGE_EditScene::mouseOnScreen()
{
    return onScreen(mapFromGlobal(QCursor::pos()));
}

bool PGE_EditScene::onScreen(const QPoint &point)
{
    return (point.x() >= 0) && (point.x() < width()) && (point.y() >= 0) && (point.y() < height());
}

bool PGE_EditScene::onScreen(const QPointF &point)
{
    return (point.x() >= 0) && (point.x() < width()) && (point.y() >= 0) && (point.y() < height());
}

bool PGE_EditScene::onScreen(int64_t x, int64_t y)
{
    return (x >= 0) && (x < width()) && (y >= 0) && (y < height());
}

double PGE_EditScene::zoom()
{
    return m_zoom;
}

double PGE_EditScene::zoomPercents()
{
    return m_zoom * 100.0;
}

void PGE_EditScene::setZoom(double zoomFactor)
{
    QPoint  scrPos = mapFromGlobal(QCursor::pos());
    QPointF oldPos = mapToWorld(scrPos);

    QPoint  anchor    = onScreen(scrPos) ? scrPos : QPoint(width() / 2, height() / 2);
    QPointF anchorPos = mapToWorld(anchor);

    m_zoom = zoomFactor;

    if(m_zoom <= 0.05)
        m_zoom = 0.05;

    QPointF delta = mapToWorld(anchor) - anchorPos;
    m_cameraPos -= delta;
    delta = mapToWorld(scrPos) - oldPos;
    moveCameraUpdMouse(delta.x(), delta.y());
    repaint();
}

void PGE_EditScene::setZoomPercent(double percentZoom)
{
    setZoom(percentZoom / 100.0);
}

void PGE_EditScene::addZoom(double zoomDelta)
{
    setZoom(m_zoom + zoomDelta);
}

void PGE_EditScene::multipleZoom(double zoomDelta)
{
    setZoom(m_zoom * zoomDelta);
}

void PGE_EditScene::moveCamera()
{
    moveCamera(m_mover.speedX, m_mover.speedY);
    moveCameraUpdMouse(m_mover.speedX, m_mover.speedY);
    repaint();
}

void PGE_EditScene::moveCamera(int deltaX, int deltaY)
{
    m_cameraPos.setX(m_cameraPos.x() + deltaX);
    m_cameraPos.setY(m_cameraPos.y() + deltaY);
}

void PGE_EditScene::moveCameraUpdMouse(double deltaX, double deltaY)
{
    if(m_moveInProcess || m_rectSelect)
    {
        m_mouseOld.setX(m_mouseOld.x() + deltaX);
        m_mouseOld.setY(m_mouseOld.y() + deltaY);
        if(m_moveInProcess)
            moveSelection(static_cast<int64_t>(deltaX), static_cast<int64_t>(deltaY));
    }
}

void PGE_EditScene::moveCameraTo(int64_t x, int64_t y)
{
    double deltaX = x - m_cameraPos.x();
    double deltaY = y - m_cameraPos.y();
    m_cameraPos.setX(x);
    m_cameraPos.setY(y);
    moveCameraUpdMouse(deltaX, deltaY);
    repaint();
}

bool PGE_EditScene::selectOneAt(int64_t x, int64_t y, bool isCtrl)
{
    bool catched = false;
    PGE_EditItemList list;
    queryItems(x, y, &list, false);
    for(PGE_EditSceneItem *item : list)
    {
        if(item->isTouching(x, y))
        {
            catched = true;
            if(isCtrl)
            {
                toggleselect(*item);
            }
            else if(!item->selected())
            {
                clearSelection();
                select(*item);
            }
            break;
        }
    }
    return catched;
}

void PGE_EditScene::closeEvent(QCloseEvent *event)
{
    m_abortThread = true;

    bool wasBusy = m_isBusy.owns_lock();
    if(wasBusy)
    {
        if(m_busyIsClosing)
        {
            event->ignore();
            return;
        }
        m_busyMutex.lock();
        m_busyMutex.unlock();
    }

    if(wasBusy && !m_busyIsClosing)
        QMessageBox::information(this, "Closed", "Ouuuuch.... :-P");

    if(!m_tree.empty())
    {
        startDeInitAsync();
        qDebug() << "Close delayed - run clean-up";
        event->ignore();
    } else {
        qDebug() << "Will be closed";
        event->accept();
    }
}

void PGE_EditScene::mousePressEvent(QMouseEvent *event)
{
    if(m_isBusy.owns_lock())
        return;

    bool isShift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool isCtrl = (event->modifiers() & Qt::ControlModifier) != 0;

    if((event->buttons() & Qt::MiddleButton) != 0)
    {
        QPointF pos = mapToWorld(event->pos());
        PGE_EditSceneItem *rect = addRect((int64_t)pos.x(), (int64_t)pos.y());
        if(isShift)
        {
            PGE_EditSceneItem *item, *item_prev;
            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(-30, -30, 20, 20);
            rect->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(-30, +30, 20, 20);
            rect->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(+30, -30, 20, 20);
            rect->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(+30, +30, 20, 20);
            rect->addChild(item);

            item_prev = item;
            //Child of child!
            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(-10, -10, 10, 10);
            item_prev->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(-10, +10, 10, 10);
            item_prev->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(+10, -10, 10, 10);
            item_prev->addChild(item);

            item = new PGE_EditSceneItem(this);
            item->m_posRect.setRect(+10, +10, 10, 10);
            item_prev->addChild(item);
        }

        repaint();
        return;
    }

    if(event->button() != Qt::LeftButton)
        return;

    QPointF pos = mapToWorld(event->pos());

    m_mouseBegin = pos;
    m_mouseOld   = pos;
    m_mouseMoved = false;

    if(!isShift)
    {
        bool catched = selectOneAt(D_TO_INT64(m_mouseOld.x()), D_TO_INT64(m_mouseOld.y()), isCtrl);

        if(!catched && !isCtrl)
            clearSelection();
        else if(catched)
        {
            moveStart();
            captureSelectionRect();
        }
    }

    if((m_selectedItems.isEmpty() && !isCtrl) || isShift)
        m_rectSelect = true;

    if(isCtrl && !isShift)
    {
        m_ignoreMove = true;
        m_ignoreRelease = true;
    }
    repaint();
    setWindowTitle(QString("Selected items: %1").arg(m_selectedItems.size()));
}

void PGE_EditScene::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isBusy.owns_lock())
        return;

    if((event->buttons() & Qt::LeftButton) == 0)
        return;

    QPointF pos = mapToWorld(event->pos());
    bool doRepaint = false;

    if(m_ignoreMove)
        return;

    QPointF delta = m_mouseOld - pos;
    if(!m_rectSelect)
    {
        moveSelection(-D_TO_INT64(delta.x()), -D_TO_INT64(delta.y()));
        doRepaint |= true;
    }

    if(m_moveInProcess || m_rectSelect)
        doRepaint |= true;

    m_mouseOld = pos;
    m_mouseMoved = true;

    if(doRepaint)
        repaint();
}

void PGE_EditScene::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isBusy.owns_lock())
        return;
    bool doRepaint = false;
    bool isShift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool isCtrl  = (event->modifiers() & Qt::ControlModifier) != 0;
    QPointF pos = mapToWorld(event->pos());

    if(m_moveInProcess)
    {
        moveEnd(false);
        doRepaint |= true;
    }

    if((event->button() == Qt::RightButton) && (event->buttons() == 0))
    {
        QMenu test;
        test.addAction("Meow  1");
        test.addAction("Meow  2");
        test.addAction("Meow :3");
        test.exec(mapToGlobal(event->pos()));
        return;
    }

    if(event->button() != Qt::LeftButton)
        return;

    if(!isShift && isCtrl && !m_mouseMoved)
        doRepaint |= true;

    bool skip = m_ignoreRelease;

    m_ignoreMove    = false;
    m_ignoreRelease = false;

    if(skip)
    {
        if(doRepaint)
            repaint();
        return;
    }

    m_mouseEnd = pos;

    if(!isShift && !isCtrl && (!m_selectedItems.isEmpty()) && (!m_mouseMoved))
    {
        clearSelection();
        selectOneAt(D_TO_INT64(m_mouseOld.x()), D_TO_INT64(m_mouseOld.y()));
        doRepaint |= true;
    }
    else if(m_rectSelect)
    {
        qreal left   = m_mouseBegin.x() < m_mouseEnd.x() ? m_mouseBegin.x() : m_mouseEnd.x();
        qreal right  = m_mouseBegin.x() > m_mouseEnd.x() ? m_mouseBegin.x() : m_mouseEnd.x();
        qreal top    = m_mouseBegin.y() < m_mouseEnd.y() ? m_mouseBegin.y() : m_mouseEnd.y();
        qreal bottom = m_mouseBegin.y() > m_mouseEnd.y() ? m_mouseBegin.y() : m_mouseEnd.y();

        PGE_EditItemList list;
        PGE_Rect<int64_t> selZone;
        //RRect vizArea = {left, top, right, bottom};
        selZone.setCoords(D_TO_INT64(left), D_TO_INT64(top), D_TO_INT64(right), D_TO_INT64(bottom));
        queryItems(selZone, &list, false);
        if(!list.isEmpty())
        {
            PGE_EditSceneItem *it = list.first();
            if(it->isTouching(selZone))
            {
                bool doSelect = it->m_selected;
                if(isShift && isCtrl)
                    doSelect = !doSelect;
                else
                    doSelect = true;
                if(doSelect)
                {
                    PGE_Rect<int64_t> &r = it->m_posRect;
                    m_selectionRect.setCoords(r.left(), r.top(), r.right(), r.bottom());
                }
            }
        }
        for(PGE_EditSceneItem *item : list)
        {
            if(item->isTouching(selZone))
            {
                if(isShift && isCtrl)
                    toggleselect(*item);
                else
                    select(*item);
                if(item->m_selected)
                    m_selectionRect.expandByRect(item->m_posRect);
            }
        }
        m_rectSelect = false;
        doRepaint |= true;
    }
    setWindowTitle(QString("Selected items: %1").arg(m_selectedItems.size()));
    if(doRepaint)
        repaint();
}

void PGE_EditScene::wheelEvent(QWheelEvent *event)
{
    if(m_isBusy.owns_lock())
        return;

    bool isShift = (event->modifiers() & Qt::ShiftModifier) != 0;
    bool isCtrl  = (event->modifiers() & Qt::ControlModifier) != 0;
    bool isAlt   = (event->modifiers() & Qt::AltModifier) != 0;

    if(isAlt)
    {
        if(event->delta() > 0)
            addZoom(0.1);
        else
            addZoom(-0.1);
    }
    else if(isCtrl)
    {
        int delta = m_mover.scrollStep * (event->delta() < 0 ? 1 : -1) * (isShift ? 4 : 1);
        moveCamera(delta, 0);
        moveCameraUpdMouse(delta, 0);
        repaint();
    }
    else
    {
        int delta = m_mover.scrollStep * (event->delta() < 0 ? 1 : -1) * (isShift ? 4 : 1);
        moveCamera(0, delta);
        moveCameraUpdMouse(0, delta);
        repaint();
    }
}

void PGE_EditScene::paintEvent(QPaintEvent */*event*/)
{
    QPainter p(this);
    if(m_isBusy.owns_lock())
    {
        p.setBrush(QBrush(Qt::black));
        p.setPen(QPen(Qt::black));
        p.drawText(QPointF(20.0, 20.0), m_busyMessage);
        p.end();
        return;
    }
    if(m_abortThread)
    {
        p.setBrush(QBrush(Qt::black));
        p.setPen(QPen(Qt::black));
        p.drawText(QPointF(20.0, 20.0), tr("Aborted :-("));
        p.end();
        return;
    }

    PGE_EditItemList list;
    PGE_Rect<int64_t> vizArea(D_TO_INT64(m_cameraPos.x()),
                              D_TO_INT64(m_cameraPos.y()),
                              D_TO_INT64(qreal(width()) / m_zoom),
                              D_TO_INT64(qreal(height()) / m_zoom));
    queryItems(vizArea, &list, true);

    for(PGE_EditSceneItem *item : list)
    {
        item->paint(&p, m_cameraPos, m_zoom);
    }

    if(m_rectSelect)
    {
        p.setBrush(QBrush(Qt::green));
        p.setPen(QPen(Qt::darkGreen));
        p.setOpacity(0.5);
        QRectF r = applyZoom(QRectF(m_mouseBegin, m_mouseOld));
        p.drawRect(r);
    }

    if(m_moveInProcess)
    {
        p.setBrush(QBrush(Qt::red));
        p.setPen(QPen(Qt::darkRed));
        p.setOpacity(0.2);
        QRectF r = applyZoom(m_selectionRect.toQRectF());
        p.drawRect(r);
    }

    p.end();
}

void PGE_EditScene::keyPressEvent(QKeyEvent *event)
{
    if(m_isBusy.owns_lock())
        return;

    bool isCtrl = (event->modifiers() & Qt::ControlModifier) != 0;
    switch(event->key())
    {
    case Qt::Key_Left:
        if(isCtrl)
        {
            moveSelection(-1, 0);
            repaint();
        }
        else
        {
            if(event->isAutoRepeat()) return;
            m_mover.setLeft(true);
        }
        break;
    case Qt::Key_Right:
        if(isCtrl)
        {
            moveSelection(1, 0);
            repaint();
        }
        else
        {
            if(event->isAutoRepeat()) return;
            m_mover.setRight(true);
        }
        break;
    case Qt::Key_Up:
        if(isCtrl)
        {
            moveSelection(0, -1);
            repaint();
        }
        else
        {
            if(event->isAutoRepeat()) return;
            m_mover.setUp(true);
        }
        break;
    case Qt::Key_Down:
        if(isCtrl)
        {
            moveSelection(0, 1);
            repaint();
        }
        else
        {
            if(event->isAutoRepeat()) return;
            m_mover.setDown(true);
        }
        break;
    case Qt::Key_Shift:
        if(event->isAutoRepeat()) return;
        m_mover.setFaster(true);
        break;
    case Qt::Key_Delete:
        deleteSelectedItems();
        repaint();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
}

void PGE_EditScene::keyReleaseEvent(QKeyEvent *event)
{
    if(m_isBusy.owns_lock())
        return;

    switch(event->key())
    {
    case Qt::Key_Escape:
        clearSelection();
        m_rectSelect = false;
        repaint();
        break;
    case Qt::Key_Left:
    {
        if(event->isAutoRepeat()) return;
        m_mover.setLeft(false);
    }
    break;
    case Qt::Key_Right:
    {
        if(event->isAutoRepeat()) return;
        m_mover.setRight(false);
    }
    break;
    case Qt::Key_Up:
    {
        if(event->isAutoRepeat()) return;
        m_mover.setUp(false);
    }
    break;
    case Qt::Key_Down:
    {
        if(event->isAutoRepeat()) return;
        m_mover.setDown(false);
    }
    break;
    case Qt::Key_Shift:
        if(event->isAutoRepeat()) return;
        m_mover.setFaster(false);
        break;
    default:
        QWidget::keyReleaseEvent(event);
        return;
    }
}

void PGE_EditScene::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
}

void PGE_EditScene::focusOutEvent(QFocusEvent *event)
{
    //releaseKeyboard();
    m_mover.reset();
    QWidget::focusOutEvent(event);
}
