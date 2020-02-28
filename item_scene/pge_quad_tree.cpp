#include "pge_quad_tree.h"
#include "pge_edit_scene_item.h"

#include "LooseQuadtree.h"

class QTreePGE_Phys_ObjectExtractor
{
public:
    static void ExtractBoundingBox(const PGE_EditSceneItem *object, loose_quadtree::BoundingBox<int64_t> *bbox)
    {
        auto r = object->boundingRectI();
        bbox->left      = r.x();
        bbox->top       = r.y();
        bbox->width     = r.width();
        bbox->height    = r.height();
    }
};

struct PgeQuadTree_private
{
    typedef loose_quadtree::LooseQuadtree<int64_t, PGE_EditSceneItem, QTreePGE_Phys_ObjectExtractor> IndexTreeQ;
    IndexTreeQ tree;
    PgeQuadTree::ItemsSet items;
};


PgeQuadTree::PgeQuadTree() :
    p(new PgeQuadTree_private)
{}

PgeQuadTree::~PgeQuadTree()
{}

bool PgeQuadTree::insert(PGE_EditSceneItem *obj)
{
    p->items.insert(obj);
    return p->tree.Insert(obj);
}

bool PgeQuadTree::update(PGE_EditSceneItem *obj)
{
    return p->tree.Update(obj);
}

bool PgeQuadTree::remove(PGE_EditSceneItem *obj)
{
    p->items.remove(obj);
    return p->tree.Remove(obj);
}

bool PgeQuadTree::removeAndDestroy(PGE_EditSceneItem *obj)
{
    p->items.remove(obj);
    bool ret = p->tree.Remove(obj);
    if(!obj)
        return false;
    delete obj;
    return ret;
}

void PgeQuadTree::clear()
{
    p->items.clear();
    p->tree.Clear();
}

void PgeQuadTree::clearAndDestroy()
{
    std::vector<PGE_EditSceneItem*> killList;
    killList.reserve((size_t)p->items.size());
    for(PGE_EditSceneItem *it : p->items)
        killList.push_back(it);
    p->items.clear();
    for(PGE_EditSceneItem *it : killList)
        delete it;
    killList.clear();
    clear();
}

void PgeQuadTree::query(PGE_Rect<int64_t> &zone, PgeQuadTree::t_resultCallback a_resultCallback, void *context) const
{
    PgeQuadTree_private::IndexTreeQ::Query q = p->tree.QueryIntersectsRegion(loose_quadtree::BoundingBox<int64_t>(zone.x(), zone.y(), zone.width(), zone.height()));
    while(!q.EndOfQuery())
    {
        a_resultCallback(q.GetCurrent(), context);
        q.Next();
    }
}

const PgeQuadTree::ItemsSet &PgeQuadTree::allItems() const
{
    return p->items;
}

size_t PgeQuadTree::count() const
{
    return (size_t)p->items.count();
}

bool PgeQuadTree::empty() const
{
    return p->items.isEmpty();
}
