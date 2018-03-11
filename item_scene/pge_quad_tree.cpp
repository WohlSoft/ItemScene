
#include "pge_quad_tree.h"
#include "LooseQuadtree.h"

class QTreePGE_Phys_ObjectExtractor
{
public:
    static void ExtractBoundingBox(const PGE_EditSceneItem *object, loose_quadtree::BoundingBox<int64_t> *bbox)
    {
        bbox->left      = object->m_posRect.x();
        bbox->top       = object->m_posRect.y();
        bbox->width     = object->m_posRect.w();
        bbox->height    = object->m_posRect.h();
    }
};

struct PgeQuadTree_private
{
    typedef loose_quadtree::LooseQuadtree<int64_t, PGE_EditSceneItem, QTreePGE_Phys_ObjectExtractor> IndexTreeQ;
    IndexTreeQ tree;
};


PgeQuadTree::PgeQuadTree() :
    p(new PgeQuadTree_private)
{}

PgeQuadTree::~PgeQuadTree()
{}

bool PgeQuadTree::insert(PGE_EditSceneItem *obj)
{
    return p->tree.Insert(obj);
}

bool PgeQuadTree::update(PGE_EditSceneItem *obj)
{
    return p->tree.Update(obj);
}

bool PgeQuadTree::remove(PGE_EditSceneItem *obj)
{
    return p->tree.Remove(obj);
}

void PgeQuadTree::clear()
{
    p->tree.Clear();
}

void PgeQuadTree::query(PGE_Rect<int64_t> &zone, PgeQuadTree::t_resultCallback a_resultCallback, void *context)
{
    PgeQuadTree_private::IndexTreeQ::Query q = p->tree.QueryIntersectsRegion(loose_quadtree::BoundingBox<int64_t>(zone.x(), zone.y(), zone.width(), zone.height()));
    while(!q.EndOfQuery())
    {
        a_resultCallback(q.GetCurrent(), context);
        q.Next();
    }
}
