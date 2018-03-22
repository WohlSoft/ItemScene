#ifndef LVL_QUAD_TREE_H
#define LVL_QUAD_TREE_H

#include "pge_rect.h"
#include <memory>
#include <QSet>

struct PgeQuadTree_private;
class PGE_EditSceneItem;
class PgeQuadTree
{
    friend struct PgeQuadTree_private;
    std::unique_ptr<PgeQuadTree_private> p;
public:
    typedef QSet<PGE_EditSceneItem* > ItemsSet;
    PgeQuadTree();
    PgeQuadTree(const PgeQuadTree &qt) = delete;
    ~PgeQuadTree();

    /**
     * @brief Insert element into the tree
     * @param obj Pointer to an element
     * @return true if success
     */
    bool insert(PGE_EditSceneItem* obj);
    /**
     * @brief Update element's position inside of the tree
     * @param obj Pointer to an element
     * @return true if success
     */
    bool update(PGE_EditSceneItem* obj);
    /**
     * @brief Unregister element from the tree without of destruction
     * @param obj Pointer to an element
     * @return true if no errors have occouped
     */
    bool remove(PGE_EditSceneItem* obj);
    /**
     * @brief Unregister element from the tree and destroy it
     * @param obj Pointer to element
     * @return true if no errors have occouped
     */
    bool removeAndDestroy(PGE_EditSceneItem* obj);
    /**
     * @brief Clear tree without destruction of pointed objects (when there are held externally)
     */
    void clear();
    /**
     * @brief Clear and destroy all objects (when there are held inside of tree)
     */
    void clearAndDestroy();

    //! Search callback function
    typedef bool (*t_resultCallback)(PGE_EditSceneItem*, void*);

    /**
     * @brief Search elements in a specific area
     * @param zone Rectangular area to find elements
     * @param a_resultCallback Callback function to return found elements
     * @param context Any user data (for example, a pointer to the container where found items would be inserted)
     */
    void query(PGE_Rect<int64_t> &zone, t_resultCallback a_resultCallback, void *context) const;
    /**
     * @brief Get a set of all elements on the tree
     * @return Set of elements on the tree
     */
    const ItemsSet & allItems() const;
    /**
     * @brief Total count of elements in the tree
     * @return Count of elements on the tree
     */
    size_t count() const;
    /**
     * @brief Is tree empty?
     * @return true if tree is empty
     */
    bool empty() const;
};

#endif // LVL_QUAD_TREE_H

