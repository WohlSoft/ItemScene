
#include <QPainter>

#include "pge_edit_scene.h"
#include "pge_edit_scene_item.h"

PGE_EditSceneItem::PGE_EditSceneItem(PGE_EditScene *scene, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    m_scene(scene),
    m_selected(false)
{}

PGE_EditSceneItem::PGE_EditSceneItem(const PGE_EditSceneItem &it) :
    QGraphicsItem(it.parentItem()),
    m_scene(it.m_scene),
    m_selected(it.m_selected),
    m_posRect(it.m_posRect)
{}

PGE_EditSceneItem::~PGE_EditSceneItem()
{
    if(m_scene)
        m_scene->unregisterElement(this);
}

void PGE_EditSceneItem::setSelected(bool selected)
{
    m_scene->setItemSelected(*this, selected);
}

bool PGE_EditSceneItem::selected() const
{
    return m_selected;
}

bool PGE_EditSceneItem::isTouching(int64_t x, int64_t y) const
{
    if(m_posRect.left() > x)
        return false;

    if((m_posRect.right() + 1) < x)
        return false;

    if(m_posRect.top() > y)
        return false;

    if((m_posRect.bottom() + 1) < y)
        return false;

    return true;
}

bool PGE_EditSceneItem::isTouching(const QRect &rect) const
{
    if(m_posRect.left() > (rect.right() + 1))
        return false;

    if((m_posRect.right() + 1) < rect.left())
        return false;

    if(m_posRect.top() > (rect.bottom() + 1))
        return false;

    if((m_posRect.bottom() + 1) < rect.top())
        return false;

    return true;
}

bool PGE_EditSceneItem::isTouching(const QRectF &rect) const
{
    if(m_posRect.left() > (rect.right() + 1))
        return false;

    if((m_posRect.right() + 1) < rect.left())
        return false;

    if(m_posRect.top() > (rect.bottom() + 1))
        return false;

    if((m_posRect.bottom() + 1) < rect.top())
        return false;

    return true;
}

bool PGE_EditSceneItem::isTouching(const PGE_Rect<int64_t> &rect) const
{
    if(m_posRect.left() > rect.right())
        return false;

    if(m_posRect.right() < rect.left())
        return false;

    if(m_posRect.top() > rect.bottom())
        return false;

    if(m_posRect.bottom() < rect.top())
        return false;

    return true;
}

int64_t PGE_EditSceneItem::x() const
{
    return m_posRect.x();
}

int64_t PGE_EditSceneItem::y() const
{
    return m_posRect.y();
}

int64_t PGE_EditSceneItem::w() const
{
    return m_posRect.w();
}

int64_t PGE_EditSceneItem::h() const
{
    return m_posRect.h();
}

int64_t PGE_EditSceneItem::left() const
{
    return m_posRect.left();
}

int64_t PGE_EditSceneItem::top() const
{
    return m_posRect.top();
}

int64_t PGE_EditSceneItem::right() const
{
    return m_posRect.right();
}

int64_t PGE_EditSceneItem::bottom() const
{
    return m_posRect.bottom();
}


int64_t PGE_EditSceneItem::x_abs() const
{
    if(m_parent)
        return x() + m_parent->x_abs();
    return x();
}

int64_t PGE_EditSceneItem::y_abs() const
{
    if(m_parent)
        return y() + m_parent->y_abs();
    return y();
}

int64_t PGE_EditSceneItem::w_abs() const
{
    return w();
}

int64_t PGE_EditSceneItem::h_abs() const
{
    return h();
}

int64_t PGE_EditSceneItem::left_abs() const
{
    if(m_parent)
        return left() + m_parent->left_abs();
    return left();
}

int64_t PGE_EditSceneItem::top_abs() const
{
    if(m_parent)
        return top() + m_parent->top_abs();
    return top();
}

int64_t PGE_EditSceneItem::right_abs() const
{
    if(m_parent)
        return right() + m_parent->right_abs();
    return right();
}

int64_t PGE_EditSceneItem::bottom_abs() const
{
    if(m_parent)
        return bottom() + m_parent->bottom_abs();
    return bottom();
}

QRectF PGE_EditSceneItem::boundingRect() const
{
    return QRectF(m_posRect.x(), m_posRect.y(), m_posRect.width(), m_posRect.height());
}

PGE_Rect<int64_t> PGE_EditSceneItem::boundingRectI() const
{
    return m_posRect;
}

void PGE_EditSceneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    painter->setBrush(QColor(Qt::white));

    if(m_selected)
        painter->setPen(QColor(Qt::green));
    else
        painter->setPen(QColor(Qt::black));

    int x = 0;
    int y = 0;
    int w = static_cast<int>(qreal(m_posRect.w()));
    int h = static_cast<int>(qreal(m_posRect.h()));
    painter->drawRect(x, y, w, h);
}
