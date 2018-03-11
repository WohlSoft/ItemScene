#ifndef PGE_EDIT_SCENE_ITEM_H
#define PGE_EDIT_SCENE_ITEM_H

#include <QRect>
#include <cmath>
#include <type_traits>

template <class T>
class PGE_Rect
{
    T m_x;
    T m_y;
    T m_width;
    T m_height;
    T m_right;
    T m_bottom;

public:
    static_assert(std::is_arithmetic<T>::value,
                  "Type of Rect must be arithmetic");

    PGE_Rect()
        : PGE_Rect(0, 0, 0, 0)
    {
    }
    PGE_Rect(T x, T y, T w, T h)
        : m_x(x)
        , m_y(y)
        , m_width(w)
        , m_height(h)
        , m_right(x + w)
        , m_bottom(y + h)
    {
    }

    inline QRect toQRect() const
    {
        return QRect(m_x, m_y, m_width, m_height);
    }
    inline QRectF toQRectF() const
    {
        return QRectF(m_x, m_y, m_width, m_height);
    }
    inline void reset()
    {
        m_x = 0;
        m_y = 0;
        m_width = 0;
        m_height = 0;
        m_right = 0;
        m_bottom = 0;
    }

    T x() const
    {
        return m_x;
    }
    void setX(T x)
    {
        m_x = x;
        m_right = x + m_width;
    }

    T y() const
    {
        return m_y;
    }
    void setY(T y)
    {
        m_y = y;
        m_bottom = y + m_height;
    }

    T w() const
    {
        return m_width;
    }
    T width() const
    {
        return m_width;
    }
    void setW(T width)
    {
        m_width = width;
        m_right = m_x + width;
    }

    T h() const
    {
        return m_height;
    }
    T height() const
    {
        return m_height;
    }
    void setH(T h)
    {
        m_height = h;
        m_bottom = m_y + h;
    }

    T left() const
    {
        return m_x;
    }
    T top() const
    {
        return m_y;
    }
    T right() const
    {
        return m_right;
    }
    T bottom() const
    {
        return m_bottom;
    }

    void setLeft(T left)
    {
        m_x = left;
        m_width = abs(m_right - m_x);
    }
    void setRight(T right)
    {
        m_right = right;
        m_width = abs(m_right - m_x);
    }
    void setTop(T top)
    {
        m_y = top;
        m_height = abs(m_bottom - m_y);
    }
    void setBottom(T bottom)
    {
        m_bottom = bottom;
        m_height = abs(m_bottom - m_y);
    }

    void expendLeft(T left)
    {
        if(left < m_x)
            setLeft(left);
    }
    void expendRight(T right)
    {
        if(right > m_right)
            setRight(right);
    }
    void expendTop(T top)
    {
        if(top < m_y)
            setTop(top);
    }
    void expendBottom(T bottom)
    {
        if(bottom > m_bottom)
            setBottom(bottom);
    }

    void expandByRect(const PGE_Rect &rect)
    {
        if(rect.m_x < m_x)
            setLeft(rect.m_x);

        if(rect.m_right > m_right)
            setRight(rect.m_right);

        if(rect.m_y < m_y)
            setTop(rect.m_y);

        if(rect.m_bottom > m_bottom)
            setBottom(rect.m_bottom);
    }

    void setPos(T x, T y)
    {
        m_x = x;
        m_y = y;
        m_right = m_width + x;
        m_bottom = m_height + y;
    }
    void moveBy(T deltaX, T deltaY)
    {
        m_x += deltaX;
        m_y += deltaY;
        m_right = m_width + m_x;
        m_bottom = m_height + m_y;
    }
    void setRect(T x, T y, T w, T h)
    {
        m_x = x;
        m_y = y;
        m_width = w;
        m_height = h;
        m_right = m_x + m_width;
        m_bottom = m_y + m_height;
    }
    void setCoords(T l, T t, T r, T b)
    {
        m_x = l;
        m_y = t;
        m_right = r;
        m_bottom = b;
        m_width = std::abs(m_right - m_x);
        m_height = std::abs(m_bottom - m_y);
    }
    QPoint topLeft() const
    {
        return QPoint(m_x, m_y);
    }
    QPointF topLeftF() const
    {
        return QPointF(m_x, m_y);
    }
};

class QPainter;
class PGE_EditScene;
class PGE_EditSceneItem
{
    friend class PGE_EditScene;
    PGE_EditScene *m_scene;
    bool m_selected;

public:
    explicit PGE_EditSceneItem(PGE_EditScene *parent);
    PGE_EditSceneItem(const PGE_EditSceneItem &it);
    virtual ~PGE_EditSceneItem();

    void setSelected(bool selected);
    bool selected() const;

    bool isTouching(int64_t x, int64_t y) const;
    bool isTouching(const QRect &rect) const;
    bool isTouching(const QRectF &rect) const;
    bool isTouching(const PGE_Rect<int64_t> &rect) const;

    int64_t x() const;
    int64_t y() const;
    int64_t w() const;
    int64_t h() const;

    int64_t left() const;
    int64_t top() const;
    int64_t right() const;
    int64_t bottom() const;

    virtual void paint(QPainter *painter, const QPointF &camera = QPoint(0, 0),
                       const double &zoom = 1.0);

    PGE_Rect<int64_t> m_posRect;
};

#endif // PGE_EDIT_SCENE_ITEM_H
