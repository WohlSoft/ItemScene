#ifndef LVL_QUAD_TREE_H
#define LVL_QUAD_TREE_H

#include "pge_edit_scene_item.h"
#include <memory>

struct PgeQuadTree_private;
class PgeQuadTree
{
    std::unique_ptr<PgeQuadTree_private> p;
public:
    PgeQuadTree();
    PgeQuadTree(const PgeQuadTree &qt) = delete;
    ~PgeQuadTree();

    bool insert(PGE_EditSceneItem*obj);
    bool update(PGE_EditSceneItem*obj);
    bool remove(PGE_EditSceneItem*obj);
    void clear();
    typedef bool (*t_resultCallback)(PGE_EditSceneItem*, void*);
    void query(PGE_Rect<int64_t> &zone, t_resultCallback a_resultCallback, void *context);
};

#endif // LVL_QUAD_TREE_H

