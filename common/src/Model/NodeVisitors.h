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

#ifndef TrenchBroom_NodesVisitors
#define TrenchBroom_NodesVisitors

#include "Model/NodeVisitor.h"
#include "Model/NodePredicates.h"
#include "Model/Brush.h"
#include "Model/BrushFacePredicates.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;

        template <bool MatchSelected>
        class MatchSelectedNodes {
        public:
            bool operator()(const Model::World*) const   { return false; }
            bool operator()(const Model::Layer*) const   { return false; }
            bool operator()(const Model::Group* group) const   { return MatchSelected == group->selected(); }
            bool operator()(const Model::Entity* entity) const { return MatchSelected == entity->selected(); }
            bool operator()(const Model::Brush* brush) const   { return MatchSelected == brush->selected(); }
        };

        template <bool MatchSelected>
        class MatchTransitivelySelectedNodes {
        public:
            bool operator()(const Model::World*) const   { return false; }
            bool operator()(const Model::Layer*) const   { return false; }
            bool operator()(const Model::Group* group) const   { return MatchSelected == group->transitivelySelected(); }
            bool operator()(const Model::Entity* entity) const { return MatchSelected == entity->transitivelySelected(); }
            bool operator()(const Model::Brush* brush) const   { return MatchSelected == brush->transitivelySelected(); }
        };

        class MatchSelectableNodes {
        private:
            const Model::EditorContext& m_editorContext;
        public:
            MatchSelectableNodes(const Model::EditorContext& editorContext);

            bool operator()(const Model::World* world) const;
            bool operator()(const Model::Layer* layer) const;
            bool operator()(const Model::Group* group) const;
            bool operator()(const Model::Entity* entity) const;
            bool operator()(const Model::Brush* brush) const;
        };

        class MatchNodesByVisibility {
        private:
            VisibilityState m_visibility;
        public:
            MatchNodesByVisibility(VisibilityState visibility);

            bool operator()(const Model::World* world) const;
            bool operator()(const Model::Layer* layer) const;
            bool operator()(const Model::Group* group) const;
            bool operator()(const Model::Entity* entity) const;
            bool operator()(const Model::Brush* brush) const;
        private:
            bool match(const Model::Node* node) const;
        };

        class NodeCollectionStrategy {
        protected:
            NodeList m_nodes;
        public:
            virtual ~NodeCollectionStrategy();

            virtual void addNode(Node* node) = 0;
            const NodeList& nodes() const;
        };

        class StandardNodeCollectionStrategy : public NodeCollectionStrategy {
        public:
            virtual ~StandardNodeCollectionStrategy() override;
        public:
            void addNode(Node* node) override;
        };

        class UniqueNodeCollectionStrategy : public NodeCollectionStrategy {
        private:
            NodeSet m_addedNodes;
        public:
            virtual ~UniqueNodeCollectionStrategy() override;
        public:
            void addNode(Node* node) override;
        };

        template <class D>
        class FilteringNodeCollectionStrategy {
        private:
            D m_delegate;
        public:
            virtual ~FilteringNodeCollectionStrategy() {}

            const NodeList& nodes() const {
                return m_delegate.nodes();
            }

            template <typename T>
            void addNode(T* node) {
                Node* actual = getNode(node);
                if (actual != nullptr)
                    m_delegate.addNode(actual);
            }
        private:
            virtual Node* getNode(World* world) const   { return world;  }
            virtual Node* getNode(Layer* layer) const   { return layer;  }
            virtual Node* getNode(Group* group) const   { return group;  }
            virtual Node* getNode(Entity* entity) const { return entity; }
            virtual Node* getNode(Brush* brush) const   { return brush;  }
        };

        template <
            typename P,
            typename C = StandardNodeCollectionStrategy,
            typename S = NeverStopRecursion
        >
        class CollectMatchingNodesVisitor : public C, public MatchingNodeVisitor<P,S> {
        public:
            CollectMatchingNodesVisitor(const P& p = P(), const S& s = S()) : MatchingNodeVisitor<P,S>(p, s) {}
        private:
            void doVisit(World* world)   override { C::addNode(world);  }
            void doVisit(Layer* layer)   override { C::addNode(layer);  }
            void doVisit(Group* group)   override { C::addNode(group);  }
            void doVisit(Entity* entity) override { C::addNode(entity); }
            void doVisit(Brush* brush)   override { C::addNode(brush);  }
        };

        template <typename V, typename I>
        Model::NodeList collectMatchingNodes(I cur, I end, Node* root) {
            NodeList result;
            while (cur != end) {
                V visitor(*cur);
                root->acceptAndRecurse(visitor);
                result = VectorUtils::setUnion(result, visitor.nodes());
                ++cur;
            }
            return result;
        }

        class CollectLayersStrategy {
        private:
            LayerList m_layers;
        public:
            const LayerList& layers() const;
        protected:
            void addLayer(Layer* layer);
        };

        class SkipLayersStrategy {
        public:
            const LayerList& layers() const;
        protected:
            void addLayer(Layer* layer);
        };

        class CollectGroupsStrategy {
        private:
            GroupList m_groups;
        public:
            const GroupList& groups() const;
        protected:
            void addGroup(Group* group);
        };

        class SkipGroupsStrategy {
        public:
            const GroupList& groups() const;
        protected:
            void addGroup(Group* group);
        };

        class CollectEntitiesStrategy {
        private:
            EntityList m_entities;
        public:
            const EntityList& entities() const;
        protected:
            void addEntity(Entity* entity);
        };

        class SkipEntitiesStrategy {
        public:
            const EntityList& entities() const;
        protected:
            void addEntity(Entity* entity);
        };

        class CollectBrushesStrategy {
        private:
            BrushList m_brushes;
        public:
            const BrushList& brushes() const;
        protected:
            void addBrush(Brush* brush);
        };

        class SkipBrushesStrategy {
        public:
            const BrushList& brushes() const;
        protected:
            void addBrush(Brush* brush);
        };

        template <class LayerStrategy, class GroupStrategy, class EntityStrategy, class BrushStrategy>
        class AssortNodesVisitorT : public NodeVisitor, public LayerStrategy, public GroupStrategy, public EntityStrategy, public BrushStrategy {
        private:
            void doVisit(World* /* world */)   override {}
            void doVisit(Layer* layer)   override {  LayerStrategy::addLayer(layer); }
            void doVisit(Group* group)   override {  GroupStrategy::addGroup(group); }
            void doVisit(Entity* entity) override { EntityStrategy::addEntity(entity); }
            void doVisit(Brush* brush)   override {  BrushStrategy::addBrush(brush); }
        };

        using AssortNodesVisitor = AssortNodesVisitorT<CollectLayersStrategy, CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy>;
        using CollectLayersVisitor = AssortNodesVisitorT<CollectLayersStrategy, SkipGroupsStrategy,    SkipEntitiesStrategy,    SkipBrushesStrategy>   ;
        using CollectGroupsVisitor = AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, SkipEntitiesStrategy,    SkipBrushesStrategy>   ;
        using CollectObjectsVisitor = AssortNodesVisitorT<SkipLayersStrategy,    CollectGroupsStrategy, CollectEntitiesStrategy, CollectBrushesStrategy>   ;
        using CollectBrushesVisitor = AssortNodesVisitorT<SkipLayersStrategy,    SkipGroupsStrategy,    SkipEntitiesStrategy,    CollectBrushesStrategy>   ;

        class BoundsContainsNodeVisitor : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const vm::bbox3 m_bounds;
        public:
            BoundsContainsNodeVisitor(const vm::bbox3& bounds);
        private:
            void doVisit(const World* world) override;
            void doVisit(const Layer* layer) override;
            void doVisit(const Group* group) override;
            void doVisit(const Entity* entity) override;
            void doVisit(const Brush* brush) override;
        };

        class BoundsIntersectsNodeVisitor : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const vm::bbox3 m_bounds;
        public:
            BoundsIntersectsNodeVisitor(const vm::bbox3& bounds);
        private:
            void doVisit(const World* world) override;
            void doVisit(const Layer* layer) override;
            void doVisit(const Group* group) override;
            void doVisit(const Entity* entity) override;
            void doVisit(const Brush* brush) override;
        };

        class CollectAttributableNodesVisitor : public NodeVisitor {
        private:
            NodeSet m_addedNodes;
            AttributableNodeList m_nodes;
        public:
            const AttributableNodeList& nodes() const;
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;

            void addNode(AttributableNode* node);
        };

        template <typename I>
        class MatchContainedNodes {
        private:
            const I m_begin;
            const I m_end;
        public:
            MatchContainedNodes(I begin, I end) :
            m_begin(begin),
            m_end(end) {}

            bool operator()(const Node* node) const {
                I cur = m_begin;
                while (cur != m_end) {
                    if (*cur != node && (*cur)->contains(node))
                        return true;
                    ++cur;
                }
                return false;
            }
        };

        template <typename I>
        class CollectContainedNodesVisitor : public CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched> {
        public:
            CollectContainedNodesVisitor(I begin, I end, const Model::EditorContext& editorContext) :
            CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched>(NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >(MatchSelectableNodes(editorContext), MatchContainedNodes<I>(begin, end))) {}
        };

        template <typename P>
        class CollectMatchingBrushFacesVisitor : public NodeVisitor {
        private:
            P m_p;
            BrushFaceList m_faces;
        public:
            CollectMatchingBrushFacesVisitor(const P& p = P()) : m_p(p) {}
            const BrushFaceList& faces() const { return m_faces; }
        private:
            void doVisit(World*)  override {}
            void doVisit(Layer*)  override {}
            void doVisit(Group*)  override {}
            void doVisit(Entity*) override {}
            void doVisit(Brush* brush) override {
                for (BrushFace* face : brush->faces()) {
                    if (m_p(face))
                        m_faces.push_back(face);
                }
            }
        };

        using CollectBrushFacesVisitor = CollectMatchingBrushFacesVisitor<BrushFacePredicates::True>;

        template <typename P>
        class CollectMatchingIssuesVisitor : public NodeVisitor {
        private:
            const IssueGeneratorList& m_issueGenerators;
            P m_p;
            IssueList m_issues;
        public:
            CollectMatchingIssuesVisitor(const IssueGeneratorList& issueGenerators, const P& p = P()) :
            m_issueGenerators(issueGenerators),
            m_p(p) {}

            const IssueList& issues() const {
                return m_issues;
            }
        private:
            void doVisit(World* world)   override { collectIssues(world);  }
            void doVisit(Layer* layer)   override { collectIssues(layer);  }
            void doVisit(Group* group)   override { collectIssues(group);  }
            void doVisit(Entity* entity) override { collectIssues(entity); }
            void doVisit(Brush* brush)   override { collectIssues(brush);  }

            void collectIssues(Node* node) {
                for (Issue* issue : node->issues(m_issueGenerators)) {
                    if (m_p(issue))
                        m_issues.push_back(issue);
                }
            }
        };

        template <template <typename> class Op, typename M>
        class CollectNodesByVisibilityTemplate : public CollectMatchingNodesVisitor<Op<M>, StandardNodeCollectionStrategy, NeverStopRecursion> {
        public:
            CollectNodesByVisibilityTemplate(const VisibilityState visibility) :
            CollectMatchingNodesVisitor<Op<M>, StandardNodeCollectionStrategy, NeverStopRecursion>(Op<M>(M(visibility))) {}
        };

        using CollectNodesWithVisibilityVisitor = CollectNodesByVisibilityTemplate<NodePredicates::Id, MatchNodesByVisibility>;
        using CollectNodesWithoutVisibilityVisitor = CollectNodesByVisibilityTemplate<NodePredicates::Not, MatchNodesByVisibility>;

        using CollectNodesVisitor = CollectMatchingNodesVisitor<NodePredicates::True>;

        class MatchNodesWithDescendantSelectionCount {
        private:
            size_t m_count;
        public:
            MatchNodesWithDescendantSelectionCount(size_t count);
            bool operator()(const Node* node) const;
        };

        class CollectNodesWithDescendantSelectionCountVisitor : public CollectMatchingNodesVisitor<MatchNodesWithDescendantSelectionCount, StandardNodeCollectionStrategy> {
        public:
            CollectNodesWithDescendantSelectionCountVisitor(size_t descendantSelectionCount);
        };

        class MatchRecursivelySelectedNodes {
        private:
            bool m_selected;
        public:
            MatchRecursivelySelectedNodes(bool selected);
            bool operator()(const Node* node) const;
        };

        class CollectRecursivelySelectedNodesVisitor : public CollectMatchingNodesVisitor<MatchRecursivelySelectedNodes, UniqueNodeCollectionStrategy> {
        public:
            CollectRecursivelySelectedNodesVisitor(bool selected);
        };

        using FacePredicate = std::function<bool(const BrushFace*)>;

        class MatchSelectableBrushFaces {
        private:
            const EditorContext& m_editorContext;
            FacePredicate m_predicate;
        private:
            bool testPredicate(const BrushFace* face) const;
        public:
            MatchSelectableBrushFaces(const EditorContext& editorContext, FacePredicate predicate);
            bool operator()(const BrushFace* face) const;
        };

        class CollectSelectableBrushFacesVisitor : public CollectMatchingBrushFacesVisitor<MatchSelectableBrushFaces> {
        public:
            CollectSelectableBrushFacesVisitor(const EditorContext& editorContext, FacePredicate predicate = FacePredicate());
        };

        class EditorContext;

        template <typename C, typename S>
        class CollectSelectableNodesTemplate : public CollectMatchingNodesVisitor<MatchSelectableNodes, C, S> {
        public:
            CollectSelectableNodesTemplate(const EditorContext& editorContext) :
            CollectMatchingNodesVisitor<MatchSelectableNodes, C, S>(MatchSelectableNodes(editorContext)) {}
        };

        using CollectSelectableNodesVisitor = CollectSelectableNodesTemplate<StandardNodeCollectionStrategy, StopRecursionIfMatched>;
        using CollectSelectableUniqueNodesVisitor = CollectSelectableNodesTemplate<UniqueNodeCollectionStrategy, StopRecursionIfMatched>;

        class MatchNodesWithFilePosition {
        private:
            const std::vector<size_t> m_positions;
        public:
            MatchNodesWithFilePosition(const std::vector<size_t>& positions);
            bool operator()(const Model::Node* node) const;
        };

        class CollectSelectableNodesWithFilePositionVisitor :
        public CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchNodesWithFilePosition>, UniqueNodeCollectionStrategy> {
        public:
            CollectSelectableNodesWithFilePositionVisitor(const EditorContext& editorContext, const std::vector<size_t>& positions);
        };

        class EditorContext;

        template <typename C, typename M>
        class CollectSelectedNodesTemplate : public CollectMatchingNodesVisitor<M, C, NeverStopRecursion> {
        public:
            CollectSelectedNodesTemplate() :
            CollectMatchingNodesVisitor<M, C, NeverStopRecursion>(M()) {}
        };

        using CollectSelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchSelectedNodes<true> > ;
        using CollectUnselectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchSelectedNodes<false> >;
        using CollectTransitivelySelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedNodes<true> > ;
        using CollectTransitivelyUnselectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedNodes<false> >;

        template <typename I>
        class MatchTouchingNodes {
        private:
            const I m_begin;
            const I m_end;
        public:
            MatchTouchingNodes(I begin, I end) :
            m_begin(begin),
            m_end(end) {}

            bool operator()(const Node* node) const {
                // if `node` is one of the search query nodes, don't count it as touching
                if (std::any_of(m_begin, m_end, [node](const auto* cur) { return node == cur; })) {
                    return false;
                }
                return std::any_of(m_begin, m_end, [node](const auto* cur) { return cur->intersects(node); });
            }
        };

        template <typename I>
        class CollectTouchingNodesVisitor : public CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchTouchingNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched> {
        public:
                CollectTouchingNodesVisitor(I begin, I end, const Model::EditorContext& editorContext) :
                CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchTouchingNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched>(NodePredicates::And<MatchSelectableNodes, MatchTouchingNodes<I> >(MatchSelectableNodes(editorContext), MatchTouchingNodes<I>(begin, end))) {}
        };

        using CollectUniqueNodesVisitor = CollectMatchingNodesVisitor<NodePredicates::True, UniqueNodeCollectionStrategy>;

        enum class BoundsType {
            /**
             * See Node::logicalBounds()
             */
            Logical,
            /**
             * See Node::physicalBounds()
             */
            Physical
        };

        class ComputeNodeBoundsVisitor : public ConstNodeVisitor {
        private:
            bool m_initialized;
            BoundsType m_boundsType;
            vm::bbox3 m_defaultBounds;
            vm::bbox3::builder m_builder;
        public:
            explicit ComputeNodeBoundsVisitor(BoundsType type, const vm::bbox3& defaultBounds = vm::bbox3());
            const vm::bbox3& bounds() const;
        private:
            void doVisit(const World* world) override;
            void doVisit(const Layer* layer) override;
            void doVisit(const Group* group) override;
            void doVisit(const Entity* entity) override;
            void doVisit(const Brush* brush) override;
            void mergeWith(const vm::bbox3& bounds);
        };

        vm::bbox3 computeLogicalBounds(const Model::NodeList& nodes);

        template <typename I>
        vm::bbox3 computeLogicalBounds(I cur, I end) {
            auto visitor = ComputeNodeBoundsVisitor(BoundsType::Logical);
            Node::accept(cur, end, visitor);
            return visitor.bounds();
        }

        vm::bbox3 computePhysicalBounds(const Model::NodeList& nodes);

        template <typename I>
        vm::bbox3 computePhysicalBounds(I cur, I end) {
            auto visitor = ComputeNodeBoundsVisitor(BoundsType::Physical);
            Node::accept(cur, end, visitor);
            return visitor.bounds();
        }

        class FindContainerVisitor : public NodeVisitor, public NodeQuery<Node*> {
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
        };

        class FindGroupVisitor : public NodeVisitor, public NodeQuery<Group*> {
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
        };

        class FindOutermostClosedGroupVisitor : public NodeVisitor, public NodeQuery<Group*> {
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
        };

        Model::Group* findGroup(Model::Node* node);

        /**
         * Searches the ancestor chain of `node` for the outermost closed group and returns
         * it if one is found, otherwise returns nullptr.
         */
        Model::Group* findOutermostClosedGroup(Model::Node* node);

        class FindLayerVisitor : public NodeVisitor, public NodeQuery<Layer*> {
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
        };

        Model::Layer* findLayer(Model::Node* node);

        template <typename P>
        class FindMatchingBrushFaceVisitor : public NodeVisitor, public NodeQuery<BrushFace*> {
        private:
            P m_p;
        public:
            FindMatchingBrushFaceVisitor(const P& p = P()) : m_p(p) {}
        private:
            void doVisit(World*)  override {}
            void doVisit(Layer*)  override {}
            void doVisit(Group*)  override {}
            void doVisit(Entity*) override {}
            void doVisit(Brush* brush)   override {
                for (BrushFace* face : brush->faces()) {
                    if (m_p(face)) {
                        setResult(face);
                        cancel();
                        return;
                    }
                }
            }
        };

        class MergeNodesIntoWorldVisitor : public NodeVisitor {
        private:
            World* m_world;
            Node* m_parent;

            ParentChildrenMap m_result;
            mutable NodeList m_nodesToDetach;
            mutable NodeList m_nodesToDelete;
        public:
            MergeNodesIntoWorldVisitor(World* world, Node* parent);

            const ParentChildrenMap& result() const;
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;

            void addNode(Node* node);
            void deleteNode(Node* node);
            void detachNode(Node* node);

            void deleteNodes() const;
            void detachNodes() const;
        };

        class TakeSnapshotVisitor : public NodeVisitor {
        private:
            NodeSnapshotList m_result;
        public:
            const NodeSnapshotList& result() const;
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
            void handleNode(Node* node);
        };

        class TransformObjectVisitor : public NodeVisitor {
        private:
            const vm::mat4x4& m_transformation;
            bool m_lockTextures;
            const vm::bbox3& m_worldBounds;
        public:
            TransformObjectVisitor(const vm::mat4x4& transformation, bool lockTextures, const vm::bbox3& worldBounds);
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;
        };
    }
}

#endif
