#ifndef THESCENE_H
#define THESCENE_H

#include <QWidget>
#include <QList>
#include <QRect>
#include <QSet>
#include <QTimer>
#include <QAtomicInteger>
#include <mutex>

#include "pge_edit_scene_item.h"
#include "RTree.h"

class PGE_EditScene : public QWidget
{
    Q_OBJECT
public:
    explicit PGE_EditScene(QWidget *parent = 0);
    virtual ~PGE_EditScene();

    /**
     * @brief Creates a new rectangular body
     * @param x Position X
     * @param y Position Y
     */
    void addRect(int x, int y);

    /**
     * @brief Clear selection list
     */
    void clearSelection();
    /**
     * @brief Move all selected bodies by relative offset
     * @param deltaX Offset X
     * @param deltaY Offset Y
     */
    void moveSelection(int deltaX, int deltaY);

    /**
     * @brief Calculate size of abstract rectangular zone
     */
    void captureSelectionRect();

    /**
     * @brief Add element into selection list
     * @param item reference to scene element
     */
    void select(PGE_EditSceneItem &item);
    /**
     * @brief Remove element from selection list
     * @param item reference to scene element
     */
    void deselect(PGE_EditSceneItem &item);
    /**
     * @brief Change element's selection state to opposite
     * @param item reference to scene element
     */
    void toggleselect(PGE_EditSceneItem &item);
    /**
     * @brief Set selection state of the element
     * @param item reference to scene element
     * @param selected selection state flag
     */
    void setItemSelected(PGE_EditSceneItem &item, bool selected);

    /**
     * @brief Begin elements moving by mouse
     */
    void moveStart();
    /**
     * @brief End elements moving by mouse
     * @param esc Deselect all elements and return to their initial positions
     */
    void moveEnd(bool esc = false);

    /**
     * @brief Begin asynchronius initializing process (scene window will be inactive, but application can be used)
     */
    virtual void startInitAsync();
    /**
     * @brief Initializing processing function (running in the separated thread)
     */
    virtual void initThread();

    typedef QList<PGE_EditSceneItem *> PGE_EditItemList;
    typedef RTree<PGE_EditSceneItem *, int, 2, int > IndexTree;
    QList<PGE_EditSceneItem> m_items;
    typedef int RPoint[2];
    IndexTree m_tree;
    struct RRect
    {
        int l;
        int t;
        int r;
        int b;
    };
    /**
     * @brief Collect elements in the rectangular area
     * @param zone Rectangular area to collect elements
     * @param resultList Pointer to list where collected elements are will be stored
     */
    void queryItems(RRect &zone,  PGE_EditItemList *resultList);
    /**
     * @brief Collect elements in the specific point
     * @param x Position X
     * @param y Position Y
     * @param resultList Pointer to list where collected elements are will be stored
     */
    void queryItems(int x, int y, PGE_EditItemList *resultList);
    /**
     * @brief Register element in the RTree
     * @param item Pointer to element to register
     */
    void registerElement(PGE_EditSceneItem *item);
    /**
     * @brief Unregister element from the RTree
     * @param item Pointer to element to unregister
     */
    void unregisterElement(PGE_EditSceneItem *item);

    typedef QSet<PGE_EditSceneItem *> SelectionMap;
    //! Map of selected elements
    SelectionMap    m_selectedItems;
    //! Rectangular area around selected elements
    PGE_Rect<int>   m_selectionRect;
    //! Previous mouse position
    QPoint          m_mouseOld;
    //! Mouse position since button press
    QPoint          m_mouseBegin;
    //! Mouse position since button releasing
    QPoint          m_mouseEnd;

    //! Is mouse moved after button pressing
    bool            m_mouseMoved;
    //! Ignore mouse move event until button release
    bool            m_ignoreMove;
    //! Ignore mouse release event until button release (will be skipped with resetting this flag)
    bool            m_ignoreRelease;
    //! Is elements moving in process
    bool            m_moveInProcess;
    //! Is rectangular selection in process
    bool            m_rectSelect;

    //! Camera position
    QPoint          m_cameraPos;
    //! Zoom factor
    double          m_zoom;

    std::mutex      m_busyMutex;
    std::unique_lock<std::mutex> m_isBusy;
    QAtomicInteger<bool> m_abortThread;

    //! Map relative mouse cursor position to world coordinates
    QPoint       mapToWorld(const QPoint &mousePos);
    //! Map world rectangle coordinates to screen with applying zoom factor
    QRect        applyZoom(const QRect &r);

    struct Mover
    {
        Mover():
            speedX(0),
            speedY(0),
            scrollStep(32),
            m_keys(K_NONE)
        {}
        QTimer  timer;
        int     speedX;
        int     speedY;

        int     scrollStep;

        enum Keys
        {
            K_NONE  = 0x00,
            K_LEFT  = 0x01,
            K_RIGHT = 0x02,
            K_UP    = 0x04,
            K_DOWN  = 0x08,
            K_SHIFT = 0x10,
        };

        unsigned int    m_keys;

        inline void set(Keys k, bool v)
        {
            if(v)
                m_keys |= k;
            else
                m_keys &= ~k;
        }

        inline bool key(Keys k)
        {
            return (m_keys & k) != 0;
        }

        inline bool noKeys()
        {
            return (m_keys & 0x0F) == 0;
        }

        void setLeft(bool key)
        {
            set(K_LEFT, key);
            updTimer();
        }

        void setRight(bool key)
        {
            set(K_RIGHT, key);
            updTimer();
        }

        void setUp(bool key)
        {
            set(K_UP, key);
            updTimer();
        }

        void setDown(bool key)
        {
            set(K_DOWN, key);
            updTimer();
        }

        void setFaster(bool key)
        {
            set(K_SHIFT, key);
            updTimer();
        }

        void reset()
        {
            m_keys = K_NONE;
            updTimer();
        }

        void updTimer()
        {
            speedX = 0;
            speedY = 0;
            if(key(K_LEFT) ^ key(K_RIGHT))
            {
                speedX = scrollStep * (key(K_LEFT) ? -1 : 1);
            }

            if(key(K_UP) ^ key(K_DOWN))
            {
                speedY = scrollStep * (key(K_UP) ? -1 : 1);
            }

            if(noKeys())
                timer.stop();
            else
            {
                int interval = key(K_SHIFT) ? 8 : 32;
                if(timer.isActive())
                    timer.setInterval(interval);
                else
                    timer.start(interval);
            }
        }
    } m_mover;

    bool mouseOnScreen();
    bool onScreen(const QPoint &point);
    bool onScreen(int x, int y);

    double zoom();
    double zoomPercents();
    void setZoom(double zoomFactor);
    void setZoomPercent(double percentZoom);
    void addZoom(double zoomDelta);
    void multipleZoom(double zoomDelta);

    void moveCamera();
    void moveCamera(int deltaX, int deltaY);
    void moveCameraUpdMouse(int deltaX, int deltaY);
    void moveCameraTo(int x, int y);

    bool selectOneAt(int x, int y, bool isCtrl = false);

    void closeEvent(QCloseEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
};



#endif // THESCENE_H
