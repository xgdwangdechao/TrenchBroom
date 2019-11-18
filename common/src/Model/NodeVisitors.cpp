/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "NodeVisitors.h"

#include "Model/EditorContext.h"

namespace TrenchBroom {
    namespace Model {
        // MatchSelectableNodes

        MatchSelectableNodes::MatchSelectableNodes(const Model::EditorContext& editorContext) :
        m_editorContext(editorContext) {}

        bool MatchSelectableNodes::operator()(const Model::World* world) const   { return m_editorContext.selectable(world); }
        bool MatchSelectableNodes::operator()(const Model::Layer* layer) const   { return m_editorContext.selectable(layer); }
        bool MatchSelectableNodes::operator()(const Model::Group* group) const   { return m_editorContext.selectable(group); }
        bool MatchSelectableNodes::operator()(const Model::Entity* entity) const { return m_editorContext.selectable(entity); }
        bool MatchSelectableNodes::operator()(const Model::Brush* brush) const   { return m_editorContext.selectable(brush); }

        // MatchNodesByVisibility

        MatchNodesByVisibility::MatchNodesByVisibility(const VisibilityState visibility) :
        m_visibility(visibility) {}

        bool MatchNodesByVisibility::operator()(const Model::World* world) const   { return match(world);  }
        bool MatchNodesByVisibility::operator()(const Model::Layer* layer) const   { return match(layer);  }
        bool MatchNodesByVisibility::operator()(const Model::Group* group) const   { return match(group);  }
        bool MatchNodesByVisibility::operator()(const Model::Entity* entity) const { return match(entity); }
        bool MatchNodesByVisibility::operator()(const Model::Brush* brush) const   { return match(brush);  }

        bool MatchNodesByVisibility::match(const Model::Node* node) const {
            return node->visibilityState() == m_visibility;
        }

        // CollectLayersStrategy

        const LayerList& CollectLayersStrategy::layers() const { return m_layers; }
        void CollectLayersStrategy::addLayer(TrenchBroom::Model::Layer* layer) { m_layers.push_back(layer); }

        const LayerList& SkipLayersStrategy::layers() const { return EmptyLayerList; }
        void SkipLayersStrategy::addLayer(TrenchBroom::Model::Layer* /* layer */) {}


        const GroupList& CollectGroupsStrategy::groups() const { return m_groups; }
        void CollectGroupsStrategy::addGroup(Group* group) { m_groups.push_back(group); }

        const GroupList& SkipGroupsStrategy::groups() const { return EmptyGroupList; }
        void SkipGroupsStrategy::addGroup(Group* /* group */) {}


        const EntityList& CollectEntitiesStrategy::entities() const { return m_entities; }
        void CollectEntitiesStrategy::addEntity(Entity* entity) { m_entities.push_back(entity); }

        const EntityList& SkipEntitiesStrategy::entities() const { return EmptyEntityList; }
        void SkipEntitiesStrategy::addEntity(Entity* /* entity */) {}


        const BrushList& CollectBrushesStrategy::brushes() const { return m_brushes; }
        void CollectBrushesStrategy::addBrush(Brush* brush) { m_brushes.push_back(brush); }


        const BrushList& SkipBrushesStrategy::brushes() const { return EmptyBrushList; }
        void SkipBrushesStrategy::addBrush(Brush* /* brush */) {}
    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        BoundsContainsNodeVisitor::BoundsContainsNodeVisitor(const vm::bbox3& bounds) :
        m_bounds(bounds) {}

        void BoundsContainsNodeVisitor::doVisit(const World*)         { setResult(false); }
        void BoundsContainsNodeVisitor::doVisit(const Layer*)         { setResult(false); }
        void BoundsContainsNodeVisitor::doVisit(const Group* group)   { setResult(m_bounds.contains(group->logicalBounds())); }
        void BoundsContainsNodeVisitor::doVisit(const Entity* entity) { setResult(m_bounds.contains(entity->logicalBounds())); }
        void BoundsContainsNodeVisitor::doVisit(const Brush* brush)   { setResult(m_bounds.contains(brush->logicalBounds())); }
    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        BoundsIntersectsNodeVisitor::BoundsIntersectsNodeVisitor(const vm::bbox3& bounds) :
        m_bounds(bounds) {}

        void BoundsIntersectsNodeVisitor::doVisit(const World*)         { setResult(false); }
        void BoundsIntersectsNodeVisitor::doVisit(const Layer*)         { setResult(false); }
        void BoundsIntersectsNodeVisitor::doVisit(const Group* group)   { setResult(m_bounds.intersects(group->logicalBounds())); }
        void BoundsIntersectsNodeVisitor::doVisit(const Entity* entity) { setResult(m_bounds.intersects(entity->logicalBounds())); }
        void BoundsIntersectsNodeVisitor::doVisit(const Brush* brush)   {
            for (const BrushVertex* vertex : brush->vertices()) {
                if (m_bounds.contains(vertex->position())) {
                    setResult(true);
                    return;
                }
            }
            setResult(false);
        }
    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const AttributableNodeList& CollectAttributableNodesVisitor::nodes() const {
            return m_nodes;
        }

        void CollectAttributableNodesVisitor::doVisit(World* world) {
            addNode(world);
        }

        void CollectAttributableNodesVisitor::doVisit(Layer*) {}
        void CollectAttributableNodesVisitor::doVisit(Group*) {}

        void CollectAttributableNodesVisitor::doVisit(Entity* entity) {
            addNode(entity);
        }

        void CollectAttributableNodesVisitor::doVisit(Brush* brush) {
            Model::AttributableNode* entity = brush->entity();
            ensure(entity != nullptr, "entity is null");
            addNode(entity);
        }

        void CollectAttributableNodesVisitor::addNode(AttributableNode* node) {
            if (m_addedNodes.insert(node).second)
                m_nodes.push_back(node);
        }
    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */


namespace TrenchBroom {
    namespace Model {
        NodeCollectionStrategy::~NodeCollectionStrategy() {}

        const NodeList& NodeCollectionStrategy::nodes() const {
            return m_nodes;
        }

        StandardNodeCollectionStrategy::~StandardNodeCollectionStrategy() {}

        void StandardNodeCollectionStrategy::addNode(Node* node) {
            m_nodes.push_back(node);
        }

        UniqueNodeCollectionStrategy::~UniqueNodeCollectionStrategy() {}

        void UniqueNodeCollectionStrategy::addNode(Node* node) {
            if (m_addedNodes.insert(node).second)
                m_nodes.push_back(node);
        }

        bool NeverStopRecursion::operator()(const Node* /* node */, bool /* matched */) const { return false; }

        bool StopRecursionIfMatched::operator()(const Node* /* node */, bool matched) const { return matched; }

    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        MatchNodesWithDescendantSelectionCount::MatchNodesWithDescendantSelectionCount(const size_t count) :
        m_count(count) {}

        bool MatchNodesWithDescendantSelectionCount::operator()(const Node* node) const {
            return node->descendantSelectionCount() == m_count;
        }

        CollectNodesWithDescendantSelectionCountVisitor::CollectNodesWithDescendantSelectionCountVisitor(const size_t descendantSelectionCount) :
        CollectMatchingNodesVisitor(MatchNodesWithDescendantSelectionCount(descendantSelectionCount)) {}
    }
}
/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        MatchRecursivelySelectedNodes::MatchRecursivelySelectedNodes(const bool selected) :
        m_selected(selected) {}

        bool MatchRecursivelySelectedNodes::operator()(const Node* node) const {
            return node->parentSelected() == m_selected;
        }

        CollectRecursivelySelectedNodesVisitor::CollectRecursivelySelectedNodesVisitor(const bool selected) :
        CollectMatchingNodesVisitor(MatchRecursivelySelectedNodes(selected)) {}

        // MatchSelectableBrushFaces

        MatchSelectableBrushFaces::MatchSelectableBrushFaces(const EditorContext& editorContext, FacePredicate predicate) :
        m_editorContext(editorContext),
        m_predicate(predicate) {}

        bool MatchSelectableBrushFaces::testPredicate(const BrushFace* face) const {
            if (!m_predicate) {
                return true;
            }
            return m_predicate(face);
        }

        bool MatchSelectableBrushFaces::operator()(const BrushFace* face) const {
            return m_editorContext.selectable(face) && testPredicate(face);
        }

        // CollectSelectableBrushFacesVisitor

        CollectSelectableBrushFacesVisitor::CollectSelectableBrushFacesVisitor(const EditorContext& editorContext, FacePredicate predicate) :
        CollectMatchingBrushFacesVisitor(MatchSelectableBrushFaces(editorContext, predicate)) {}

        // MatchNodesWithFilePosition

        MatchNodesWithFilePosition::MatchNodesWithFilePosition(const std::vector<size_t>& positions) :
        m_positions(positions) {}

        bool MatchNodesWithFilePosition::operator()(const Model::Node* node) const {
            for (const size_t position : m_positions) {
                if (node->containsLine(position))
                    return true;
            }
            return false;
        }

        // CollectSelectableNodesWithFilePositionVisitor
        CollectSelectableNodesWithFilePositionVisitor::CollectSelectableNodesWithFilePositionVisitor(const EditorContext& editorContext, const std::vector<size_t>& positions) :
        CollectMatchingNodesVisitor(NodePredicates::And<MatchSelectableNodes, MatchNodesWithFilePosition>(MatchSelectableNodes(editorContext), MatchNodesWithFilePosition(positions))) {}

        // ComputeNodeBoundsVisitor

        ComputeNodeBoundsVisitor::ComputeNodeBoundsVisitor(const BoundsType type, const vm::bbox3& defaultBounds) :
        m_initialized(false),
        m_boundsType(type),
        m_defaultBounds(defaultBounds) {}

        const vm::bbox3& ComputeNodeBoundsVisitor::bounds() const {
            if (m_builder.initialized()) {
                return m_builder.bounds();
            } else {
                return m_defaultBounds;
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const World*) {}
        void ComputeNodeBoundsVisitor::doVisit(const Layer*) {}

        void ComputeNodeBoundsVisitor::doVisit(const Group* group) {
            if (m_boundsType == BoundsType::Physical) {
                m_builder.add(group->physicalBounds());
            } else {
                m_builder.add(group->logicalBounds());
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const Entity* entity) {
            if (m_boundsType == BoundsType::Physical) {
                m_builder.add(entity->physicalBounds());
            } else {
                m_builder.add(entity->logicalBounds());
            }
        }

        void ComputeNodeBoundsVisitor::doVisit(const Brush* brush) {
            if (m_boundsType == BoundsType::Physical) {
                m_builder.add(brush->physicalBounds());
            } else {
                m_builder.add(brush->logicalBounds());
            }
        }

        vm::bbox3 computeLogicalBounds(const Model::NodeList& nodes) {
            return computeLogicalBounds(std::begin(nodes), std::end(nodes));
        }

        vm::bbox3 computePhysicalBounds(const Model::NodeList& nodes) {
            return computePhysicalBounds(std::begin(nodes), std::end(nodes));
        }

        // FindContainerVisitor

        void FindContainerVisitor::doVisit(World* world) {
            setResult(world);
            cancel();
        }

        void FindContainerVisitor::doVisit(Layer* layer) {
            setResult(layer);
            cancel();
        }

        void FindContainerVisitor::doVisit(Group* group) {
            setResult(group);
            cancel();
        }

        void FindContainerVisitor::doVisit(Entity* entity) {
            setResult(entity);
            cancel();
        }

        void FindContainerVisitor::doVisit(Brush*) {}

        // FindGroupVisitor

        void FindGroupVisitor::doVisit(World*) {}
        void FindGroupVisitor::doVisit(Layer*) {}

        void FindGroupVisitor::doVisit(Group* group) {
            setResult(group);
            cancel();
        }

        void FindGroupVisitor::doVisit(Entity*) {}
        void FindGroupVisitor::doVisit(Brush*) {}

        // FindOutermostClosedGroupVisitor

        void FindOutermostClosedGroupVisitor::doVisit(World*) {}
        void FindOutermostClosedGroupVisitor::doVisit(Layer*) {}

        void FindOutermostClosedGroupVisitor::doVisit(Group* group) {
            const bool closed = !(group->opened() || group->hasOpenedDescendant());

            if (closed) {
                setResult(group);
            }
        }

        void FindOutermostClosedGroupVisitor::doVisit(Entity*) {}
        void FindOutermostClosedGroupVisitor::doVisit(Brush*) {}

        // Helper functions

        Model::Group* findGroup(Model::Node* node) {
            FindGroupVisitor visitor;
            node->escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        Model::Group* findOutermostClosedGroup(Model::Node* node) {
            FindOutermostClosedGroupVisitor visitor;
            node->escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        // FindLayerVisitor

        void FindLayerVisitor::doVisit(World*) {}

        void FindLayerVisitor::doVisit(Layer* layer) {
            setResult(layer);
            cancel();
        }

        void FindLayerVisitor::doVisit(Group*) {}
        void FindLayerVisitor::doVisit(Entity*) {}
        void FindLayerVisitor::doVisit(Brush*) {}

        Model::Layer* findLayer(Model::Node* node) {
            FindLayerVisitor visitor;
            node->acceptAndEscalate(visitor);
            return visitor.result();
        }

        // MergeNodesIntoWorldVisitor

        MergeNodesIntoWorldVisitor::MergeNodesIntoWorldVisitor(World* world, Node* parent) :
        m_world(world),
        m_parent(parent != nullptr ? parent : m_world->defaultLayer()) {
            ensure(m_world != nullptr, "world is null");
            assert(m_parent->isDescendantOf(m_world));
        }

        const ParentChildrenMap& MergeNodesIntoWorldVisitor::result() const {
            detachNodes();
            deleteNodes();
            return m_result;
        }

        void MergeNodesIntoWorldVisitor::doVisit(World* world) {
            world->iterate(*this);
            deleteNode(world);
        }

        void MergeNodesIntoWorldVisitor::doVisit(Layer* layer) {
            layer->iterate(*this);
            deleteNode(layer);
        }

        void MergeNodesIntoWorldVisitor::doVisit(Group* group) {
            addNode(group);
        }

        void MergeNodesIntoWorldVisitor::doVisit(Entity* entity) {
            if (isWorldspawn(entity->classname(), entity->attributes())) {
                entity->iterate(*this);
                deleteNode(entity);
            } else {
                addNode(entity);
            }
        }

        void MergeNodesIntoWorldVisitor::doVisit(Brush* brush) {
            addNode(brush);
        }

        void MergeNodesIntoWorldVisitor::addNode(Node* node) {
            m_result[m_parent].push_back(node);
            detachNode(node);
        }

        void MergeNodesIntoWorldVisitor::deleteNode(Node* node) {
            detachNode(node);
            m_nodesToDelete.push_back(node);
        }

        void MergeNodesIntoWorldVisitor::detachNode(Node* node) {
            Node* parent = node->parent();
            if (parent != nullptr)
                m_nodesToDetach.push_back(node);
        }

        void MergeNodesIntoWorldVisitor::deleteNodes() const {
            VectorUtils::clearAndDelete(m_nodesToDelete);
        }

        void MergeNodesIntoWorldVisitor::detachNodes() const {
            for (Node* node : m_nodesToDetach) {
                Node* parent = node->parent();
                ensure(parent != nullptr, "parent is null");
                parent->removeChild(node);
            }
            m_nodesToDetach.clear();
        }

        // TakeSnapshotVisitor

        const NodeSnapshotList& TakeSnapshotVisitor::result() const {
            return m_result;
        }

        void TakeSnapshotVisitor::doVisit(World* world)   { handleNode(world); }
        void TakeSnapshotVisitor::doVisit(Layer* layer)   { handleNode(layer); }
        void TakeSnapshotVisitor::doVisit(Group* group)   { handleNode(group); }
        void TakeSnapshotVisitor::doVisit(Entity* entity) { handleNode(entity); }
        void TakeSnapshotVisitor::doVisit(Brush* brush)   { handleNode(brush); }

        void TakeSnapshotVisitor::handleNode(Node* node) {
            NodeSnapshot* snapshot = node->takeSnapshot();
            if (snapshot != nullptr)
                m_result.push_back(snapshot);
        }

        // TransformObjectVisitor

        TransformObjectVisitor::TransformObjectVisitor(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) :
        m_transformation(transformation),
        m_lockTextures(lockTextures),
        m_worldBounds(worldBounds) {}

        void TransformObjectVisitor::doVisit(World*)         {}
        void TransformObjectVisitor::doVisit(Layer*)         {}
        void TransformObjectVisitor::doVisit(Group* group)   {  group->transform(m_transformation, m_lockTextures, m_worldBounds); }
        void TransformObjectVisitor::doVisit(Entity* entity) { entity->transform(m_transformation, m_lockTextures, m_worldBounds); }
        void TransformObjectVisitor::doVisit(Brush* brush)   {  brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
    }
}
