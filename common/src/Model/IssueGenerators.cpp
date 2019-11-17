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

#include "Model/IssueGenerators.h"

#include "StringUtils.h"
#include "Model/Brush.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/TransformEntityAttributesQuickFix.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/Group.h"
#include "Model/MapFacade.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "CollectionUtils.h"
#include "Model/AttributableNode.h"
#include "Model/EntityAttributes.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/Game.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <cassert>
#include <algorithm>
#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace Model {
        class MissingClassnameIssueGenerator::MissingClassnameIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            MissingClassnameIssue(AttributableNode* node) :
            Issue(node) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Entity has no classname property";
            }
        };

        const IssueType MissingClassnameIssueGenerator::MissingClassnameIssue::Type = Issue::freeType();

        class MissingClassnameIssueGenerator::MissingClassnameIssueQuickFix : public IssueQuickFix {
        public:
            MissingClassnameIssueQuickFix() :
            IssueQuickFix(MissingClassnameIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        MissingClassnameIssueGenerator::MissingClassnameIssueGenerator() :
        IssueGenerator(MissingClassnameIssue::Type, "Missing entity classname") {
            addQuickFix(new MissingClassnameIssueQuickFix());
        }

        void MissingClassnameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (!node->hasAttribute(AttributeNames::Classname))
                issues.push_back(new MissingClassnameIssue(node));
        }

        // MixedBrushContentsIssueGenerator

        class MixedBrushContentsIssueGenerator::MixedBrushContentsIssue : public Issue {
        public:
            friend class MixedBrushContentsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            MixedBrushContentsIssue(Brush* brush) :
            Issue(brush) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Brush has mixed content flags";
            }
        };

        const IssueType MixedBrushContentsIssueGenerator::MixedBrushContentsIssue::Type = Issue::freeType();

        MixedBrushContentsIssueGenerator::MixedBrushContentsIssueGenerator() :
        IssueGenerator(MixedBrushContentsIssue::Type, "Mixed brush content flags") {}

        void MixedBrushContentsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it = std::begin(faces);
            BrushFaceList::const_iterator end = std::end(faces);
            assert(it != end);

            const int contentFlags = (*it)->surfaceContents();
            ++it;
            while (it != end) {
                if ((*it)->surfaceContents() != contentFlags)
                    issues.push_back(new MixedBrushContentsIssue(brush));
                ++it;
            }
        }

        // EmptyBrushEntityIssueGenerator

        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            EmptyBrushEntityIssue(Entity* entity) :
            Issue(entity) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const Entity* entity = static_cast<Entity*>(node());
                return "Entity '" + entity->classname() + "' does not contain any brushes";
            }
        };

        const IssueType EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue::Type = Issue::freeType();

        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueQuickFix : public IssueQuickFix {
        public:
            EmptyBrushEntityIssueQuickFix() :
            IssueQuickFix(EmptyBrushEntityIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueGenerator() :
        IssueGenerator(EmptyBrushEntityIssue::Type, "Empty brush entity") {
            addQuickFix(new EmptyBrushEntityIssueQuickFix());
        }

        void EmptyBrushEntityIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            ensure(entity != nullptr, "entity is null");
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition != nullptr && definition->type() == Assets::EntityDefinition::Type_BrushEntity && !entity->hasChildren())
                issues.push_back(new EmptyBrushEntityIssue(entity));
        }

        // MissingDefinitionIssueGenerator

        class MissingDefinitionIssueGenerator::MissingDefinitionIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            MissingDefinitionIssue(AttributableNode* node) :
            Issue(node) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->classname() + " not found in entity definitions";
            }
        };

        const IssueType MissingDefinitionIssueGenerator::MissingDefinitionIssue::Type = Issue::freeType();

        class MissingDefinitionIssueGenerator::MissingDefinitionIssueQuickFix : public IssueQuickFix {
        public:
            MissingDefinitionIssueQuickFix() :
            IssueQuickFix(MissingDefinitionIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        MissingDefinitionIssueGenerator::MissingDefinitionIssueGenerator() :
        IssueGenerator(MissingDefinitionIssue::Type, "Missing entity definition") {
            addQuickFix(new MissingDefinitionIssueQuickFix());
        }

        void MissingDefinitionIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->definition() == nullptr)
                issues.push_back(new MissingDefinitionIssue(node));
        }

        // WorldBoundsIssueGenerator

        class WorldBoundsIssueGenerator::WorldBoundsIssue : public Issue {
        public:
            friend class WorldBoundsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            WorldBoundsIssue(Node* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Object is out of world bounds";
            }
        };

        class WorldBoundsIssueGenerator::WorldBoundsIssueQuickFix : public IssueQuickFix {
        public:
            WorldBoundsIssueQuickFix() :
            IssueQuickFix(WorldBoundsIssue::Type, "Delete objects") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        const IssueType WorldBoundsIssueGenerator::WorldBoundsIssue::Type = Issue::freeType();

        WorldBoundsIssueGenerator::WorldBoundsIssueGenerator(const vm::bbox3& bounds) :
        IssueGenerator(WorldBoundsIssue::Type, "Objects out of world bounds"),
        m_bounds(bounds) {
            addQuickFix(new WorldBoundsIssueQuickFix());
        }

        void WorldBoundsIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            if (!m_bounds.contains(entity->logicalBounds()))
                issues.push_back(new WorldBoundsIssue(entity));
        }

        void WorldBoundsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            if (!m_bounds.contains(brush->logicalBounds()))
                issues.push_back(new WorldBoundsIssue(brush));
        }
        
        // LinkTargetIssueGenerator

        class LinkTargetIssueGenerator::LinkTargetIssue : public Issue {
        public:
            friend class LinkTargetIssueQuickFix;
        private:
            const AttributeName m_name;
        public:
            static const IssueType Type;
        public:
            LinkTargetIssue(AttributableNode* node, const AttributeName& name) :
            Issue(node),
            m_name(name) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->classname() + " has missing target for key '" + m_name + "'";
            }
        };

        const IssueType LinkTargetIssueGenerator::LinkTargetIssue::Type = Issue::freeType();

        class LinkTargetIssueGenerator::LinkTargetIssueQuickFix : public IssueQuickFix {
        private:
            using AttributeNameMap = std::map<AttributeName, NodeList>;
        public:
            LinkTargetIssueQuickFix() :
            IssueQuickFix(LinkTargetIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                const LinkTargetIssue* targetIssue = static_cast<const LinkTargetIssue*>(issue);
                const AttributeName& attributeName = targetIssue->m_name;

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(attributeName);
            }
        };

        LinkTargetIssueGenerator::LinkTargetIssueGenerator() :
        IssueGenerator(LinkTargetIssue::Type, "Missing entity link source") {
            addQuickFix(new LinkTargetIssueQuickFix());
        }

        void LinkTargetIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            processKeys(node, node->findMissingLinkTargets(), issues);
            processKeys(node, node->findMissingKillTargets(), issues);
        }

        void LinkTargetIssueGenerator::processKeys(AttributableNode* node, const Model::AttributeNameList& names, IssueList& issues) const {
            issues.reserve(issues.size() + names.size());
            for (const Model::AttributeName& name : names)
                issues.push_back(new LinkTargetIssue(node, name));
        }

        // LinkSourceIssueGenerator

        class LinkSourceIssueGenerator::LinkSourceIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            LinkSourceIssue(AttributableNode* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->classname() + " has unused targetname key";
            }
        };

        const IssueType LinkSourceIssueGenerator::LinkSourceIssue::Type = Issue::freeType();

        class LinkSourceIssueGenerator::LinkSourceIssueQuickFix : public IssueQuickFix {
        public:
            LinkSourceIssueQuickFix() :
            IssueQuickFix(LinkSourceIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(AttributeNames::Targetname);
            }
        };

        LinkSourceIssueGenerator::LinkSourceIssueGenerator() :
        IssueGenerator(LinkSourceIssue::Type, "Missing entity link source") {
            addQuickFix(new LinkSourceIssueQuickFix());
        }

        void LinkSourceIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->hasMissingSources())
                issues.push_back(new LinkSourceIssue(node));
        }
        
        // AttributeValueWithDoubleQuotationMarksIssueGenerator

        class AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            AttributeValueWithDoubleQuotationMarksIssue(AttributableNode* node, const AttributeName& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const AttributeName& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "The value of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };

        const IssueType AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssue::Type = Issue::freeType();

        AttributeValueWithDoubleQuotationMarksIssueGenerator::AttributeValueWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeValueWithDoubleQuotationMarksIssue::Type, "Invalid entity property values") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeValueWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const AttributeName& name)   { return name; },
                                                              [] (const AttributeValue& value) { return StringUtils::replaceAll(value, "\"", "'"); }));
        }

        void AttributeValueWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                const AttributeValue& attributeValue = attribute.value();
                if (attributeValue.find('"') != String::npos)
                    issues.push_back(new AttributeValueWithDoubleQuotationMarksIssue(node, attributeName));
            }
        }

        // PointEntityWithBrushesIssueGenerator

        class PointEntityWithBrushesIssueGenerator::PointEntityWithBrushesIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            PointEntityWithBrushesIssue(Entity* entity) :
            Issue(entity) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const Entity* entity = static_cast<Entity*>(node());
                return entity->classname() + " contains brushes";
            }
        };

        const IssueType PointEntityWithBrushesIssueGenerator::PointEntityWithBrushesIssue::Type = Issue::freeType();

        class PointEntityWithBrushesIssueGenerator::PointEntityWithBrushesIssueQuickFix : public IssueQuickFix {
        public:
            PointEntityWithBrushesIssueQuickFix() :
            IssueQuickFix(PointEntityWithBrushesIssue::Type, "Move brushes to world") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const override {
                NodeList affectedNodes;
                ParentChildrenMap nodesToReparent;

                for (const Issue* issue : issues) {
                    Node* node = issue->node();
                    nodesToReparent[node->parent()] = node->children();

                    affectedNodes.push_back(node);
                    VectorUtils::append(affectedNodes, node->children());
                }

                facade->deselectAll();
                facade->reparentNodes(nodesToReparent);
                facade->select(affectedNodes);
            }
        };

        PointEntityWithBrushesIssueGenerator::PointEntityWithBrushesIssueGenerator() :
        IssueGenerator(PointEntityWithBrushesIssue::Type, "Point entity with brushes") {
            addQuickFix(new PointEntityWithBrushesIssueQuickFix());
        }

        void PointEntityWithBrushesIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            ensure(entity != nullptr, "entity is null");
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition != nullptr && definition->type() == Assets::EntityDefinition::Type_PointEntity && entity->hasChildren())
                issues.push_back(new PointEntityWithBrushesIssue(entity));
        }

        // LongAttributeValueIssueGenerator

        class LongAttributeValueIssueGenerator::LongAttributeValueIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            LongAttributeValueIssue(AttributableNode* node, const AttributeName& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const AttributeName& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "The value of entity property '" + m_attributeName + "' is too long.";
            }
        };

        const IssueType LongAttributeValueIssueGenerator::LongAttributeValueIssue::Type = Issue::freeType();

        class LongAttributeValueIssueGenerator::TruncateLongAttributeValueIssueQuickFix : public IssueQuickFix {
        private:
            size_t m_maxLength;
        public:
            TruncateLongAttributeValueIssueQuickFix(const size_t maxLength) :
            IssueQuickFix(LongAttributeValueIssue::Type, "Truncate property values"),
            m_maxLength(maxLength) {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                const LongAttributeValueIssue* attrIssue = static_cast<const LongAttributeValueIssue*>(issue);
                const AttributeName& attributeName = attrIssue->attributeName();
                const AttributeValue& attributeValue = attrIssue->attributeValue();

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->setAttribute(attributeName, attributeValue.substr(0, m_maxLength));
            }
        };

        LongAttributeValueIssueGenerator::LongAttributeValueIssueGenerator(const size_t maxLength) :
        IssueGenerator(LongAttributeValueIssue::Type, "Long entity property value"),
        m_maxLength(maxLength) {
            addQuickFix(new RemoveEntityAttributesQuickFix(LongAttributeValueIssue::Type));
            addQuickFix(new TruncateLongAttributeValueIssueQuickFix(m_maxLength));
        }

        void LongAttributeValueIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                const AttributeValue& attributeValue = attribute.value();
                if (attributeValue.size() >= m_maxLength)
                    issues.push_back(new LongAttributeValueIssue(node, attributeName));
            }
        }

        // AttributeNameWithDoubleQuotationMarksIssueGenerator

        class AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            AttributeNameWithDoubleQuotationMarksIssue(AttributableNode* node, const AttributeName& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const AttributeName& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "The key of entity property '" + m_attributeName + "' contains double quotation marks. This may cause errors during compilation or in the game.";
            }
        };

        const IssueType AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssue::Type = Issue::freeType();

        AttributeNameWithDoubleQuotationMarksIssueGenerator::AttributeNameWithDoubleQuotationMarksIssueGenerator() :
        IssueGenerator(AttributeNameWithDoubleQuotationMarksIssue::Type, "Invalid entity property keys") {
            addQuickFix(new RemoveEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type));
            addQuickFix(new TransformEntityAttributesQuickFix(AttributeNameWithDoubleQuotationMarksIssue::Type,
                                                              "Replace \" with '",
                                                              [] (const AttributeName& name)   { return StringUtils::replaceAll(name, "\"", "'"); },
                                                              [] (const AttributeValue& value) { return value; }));
        }

        void AttributeNameWithDoubleQuotationMarksIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                if (attributeName.find('"') != String::npos)
                    issues.push_back(new AttributeNameWithDoubleQuotationMarksIssue(node, attributeName));
            }
        }

        // EmptyAttributeNameIssueGenerator

        class EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            EmptyAttributeNameIssue(AttributableNode* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return attributableNode->classname() + " has a property with an empty name.";
            }
        };

        const IssueType EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssue::Type = Issue::freeType();

        class EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssueQuickFix : public IssueQuickFix {
        public:
            EmptyAttributeNameIssueQuickFix() :
            IssueQuickFix(EmptyAttributeNameIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute("");
            }
        };

        EmptyAttributeNameIssueGenerator::EmptyAttributeNameIssueGenerator() :
        IssueGenerator(EmptyAttributeNameIssue::Type, "Empty property name") {
            addQuickFix(new EmptyAttributeNameIssueQuickFix());
        }

        void EmptyAttributeNameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            if (node->hasAttribute(""))
                issues.push_back(new EmptyAttributeNameIssue(node));
        }
        
        // EmptyAttributeValueIssueGenerator

        class EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            AttributeName m_attributeName;
        public:
            EmptyAttributeValueIssue(AttributableNode* node, const AttributeName& attributeName) :
            Issue(node),
            m_attributeName(attributeName) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
                return "Attribute '" + m_attributeName + "' of " + attributableNode->classname() + " has an empty value.";
            }

            const AttributeName& attributeName() const {
                return m_attributeName;
            }
        };

        const IssueType EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssue::Type = Issue::freeType();

        class EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssueQuickFix : public IssueQuickFix {
        public:
            EmptyAttributeValueIssueQuickFix() :
            IssueQuickFix(EmptyAttributeValueIssue::Type, "Delete property") {}
        private:
            void doApply(MapFacade* facade, const Issue* issue) const override {
                const EmptyAttributeValueIssue* actualIssue = static_cast<const EmptyAttributeValueIssue*>(issue);
                const AttributeName& attributeName = actualIssue->attributeName();

                const PushSelection push(facade);

                // If world node is affected, the selection will fail, but if nothing is selected,
                // the removeAttribute call will correctly affect worldspawn either way.

                facade->deselectAll();
                facade->select(issue->node());
                facade->removeAttribute(attributeName);
            }
        };

        EmptyAttributeValueIssueGenerator::EmptyAttributeValueIssueGenerator() :
        IssueGenerator(EmptyAttributeValueIssue::Type, "Empty property value") {
            addQuickFix(new EmptyAttributeValueIssueQuickFix());
        }

        void EmptyAttributeValueIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                if (attribute.value().empty())
                    issues.push_back(new EmptyAttributeValueIssue(node, attribute.name()));
            }
        }

        // NonIntegerVerticesIssueGenerator

        class NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssue : public Issue {
        public:
            friend class NonIntegerVerticesIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            NonIntegerVerticesIssue(Brush* brush) :
            Issue(brush) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Brush has non-integer vertices";
            }
        };

        const IssueType NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssue::Type = Issue::freeType();

        class NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssueQuickFix : public IssueQuickFix {
        public:
            NonIntegerVerticesIssueQuickFix() :
            IssueQuickFix(NonIntegerVerticesIssue::Type, "Convert vertices to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->snapVertices(1);
            }
        };

        NonIntegerVerticesIssueGenerator::NonIntegerVerticesIssueGenerator() :
        IssueGenerator(NonIntegerVerticesIssue::Type, "Non-integer vertices") {
            addQuickFix(new NonIntegerVerticesIssueQuickFix());
        }

        void NonIntegerVerticesIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            for (const BrushVertex* vertex : brush->vertices()) {
                if (!vm::is_integral(vertex->position())) {
                    issues.push_back(new NonIntegerVerticesIssue(brush));
                    return;
                }
            }
        }

        // InvalidTextureScaleIssueGenerator

        class InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssue : public BrushFaceIssue {
        public:
            friend class InvalidTextureScaleIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            explicit InvalidTextureScaleIssue(BrushFace* face) :
            BrushFaceIssue(face) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Face has invalid texture scale.";
            }
        };

        const IssueType InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssue::Type = Issue::freeType();

        class InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssueQuickFix : public IssueQuickFix {
        public:
            InvalidTextureScaleIssueQuickFix() :
            IssueQuickFix(InvalidTextureScaleIssue::Type, "Reset texture scale") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const override {
                const PushSelection push(facade);

                BrushFaceList faces;
                for (const auto* issue : issues) {
                    if (issue->type() == InvalidTextureScaleIssue::Type) {
                        auto* face = static_cast<const InvalidTextureScaleIssue*>(issue)->face();
                        faces.push_back(face);
                    }
                }

                ChangeBrushFaceAttributesRequest request;
                request.setScale(vm::vec2f::one());

                facade->deselectAll();
                facade->select(faces);
                facade->setFaceAttributes(request);
            }
        };

        InvalidTextureScaleIssueGenerator::InvalidTextureScaleIssueGenerator() :
        IssueGenerator(InvalidTextureScaleIssue::Type, "Invalid texture scale") {
            addQuickFix(new InvalidTextureScaleIssueQuickFix());
        }

        void InvalidTextureScaleIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            for (const auto& face : brush->faces()) {
                if (!face->attribs().valid()) {
                    issues.push_back(new InvalidTextureScaleIssue(face));
                }
            }
        }

        // LongAttributeNameIssueGenerator

        class LongAttributeNameIssueGenerator::LongAttributeNameIssue : public AttributeIssue {
        public:
            static const IssueType Type;
        private:
            const AttributeName m_attributeName;
        public:
            LongAttributeNameIssue(AttributableNode* node, const AttributeName& attributeName) :
            AttributeIssue(node),
            m_attributeName(attributeName) {}

            const AttributeName& attributeName() const override {
                return m_attributeName;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Entity property key '" + m_attributeName.substr(0, 8) + "...' is too long.";
            }
        };

        const IssueType LongAttributeNameIssueGenerator::LongAttributeNameIssue::Type = Issue::freeType();

        LongAttributeNameIssueGenerator::LongAttributeNameIssueGenerator(const size_t maxLength) :
        IssueGenerator(LongAttributeNameIssue::Type, "Long entity property keys"),
        m_maxLength(maxLength) {
            addQuickFix(new RemoveEntityAttributesQuickFix(LongAttributeNameIssue::Type));
        }

        void LongAttributeNameIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            for (const EntityAttribute& attribute : node->attributes()) {
                const AttributeName& attributeName = attribute.name();
                if (attributeName.size() >= m_maxLength)
                    issues.push_back(new LongAttributeNameIssue(node, attributeName));
            }
        }

        // EmptyGroupIssueGenerator

        class EmptyGroupIssueGenerator::EmptyGroupIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            EmptyGroupIssue(Group* group) :
            Issue(group) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                const Group* group = static_cast<Group*>(node());
                return "Group '" + group->name() + "' does not contain any objects";
            }
        };

        const IssueType EmptyGroupIssueGenerator::EmptyGroupIssue::Type = Issue::freeType();

        class EmptyGroupIssueGenerator::EmptyGroupIssueQuickFix : public IssueQuickFix {
        public:
            EmptyGroupIssueQuickFix() :
            IssueQuickFix(EmptyGroupIssue::Type, "Delete groups") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        EmptyGroupIssueGenerator::EmptyGroupIssueGenerator() :
        IssueGenerator(EmptyGroupIssue::Type, "Empty group") {
            addQuickFix(new EmptyGroupIssueQuickFix());
        }

        void EmptyGroupIssueGenerator::doGenerate(Group* group, IssueList& issues) const {
            ensure(group != nullptr, "group is null");
            if (!group->hasChildren())
                issues.push_back(new EmptyGroupIssue(group));
        }

        // NonIntegerPlanePointsIssueGenerator

        class NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssue : public Issue {
        public:
            friend class NonIntegerPlanePointsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            NonIntegerPlanePointsIssue(Brush* brush) :
            Issue(brush) {}

            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Brush has non-integer plane points";
            }
        };

        const IssueType NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssue::Type = Issue::freeType();

        class NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssueQuickFix : public IssueQuickFix {
        public:
            NonIntegerPlanePointsIssueQuickFix() :
            IssueQuickFix(NonIntegerPlanePointsIssue::Type, "Convert plane points to integer") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->findPlanePoints();
            }
        };

        NonIntegerPlanePointsIssueGenerator::NonIntegerPlanePointsIssueGenerator() :
        IssueGenerator(NonIntegerPlanePointsIssue::Type, "Non-integer plane points") {
            addQuickFix(new NonIntegerPlanePointsIssueQuickFix());
        }

        void NonIntegerPlanePointsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            for (const BrushFace* face : brush->faces()) {
                const BrushFace::Points& points = face->points();
                for (size_t i = 0; i < 3; ++i) {
                    const vm::vec3& point = points[i];
                    if (!vm::is_integral(point)) {
                        issues.push_back(new NonIntegerPlanePointsIssue(brush));
                        return;
                    }
                }
            }
        }

        // MissingModIssueGenerator

        class MissingModIssueGenerator::MissingModIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            String m_mod;
            String m_message;
        public:
            MissingModIssue(AttributableNode* node, const String& mod, const String& message) :
            Issue(node),
            m_mod(mod),
            m_message(message) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Mod '" + m_mod + "' could not be used: " + m_message;
            }
        public:
            const String& mod() const {
                return m_mod;
            }
        };

        const IssueType MissingModIssueGenerator::MissingModIssue::Type = Issue::freeType();

        class MissingModIssueGenerator::MissingModIssueQuickFix : public IssueQuickFix {
        public:
            MissingModIssueQuickFix() :
            IssueQuickFix(MissingModIssue::Type, "Remove mod") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const override {
                const PushSelection pushSelection(facade);

                 // If nothing is selected, attribute changes will affect only world.
                facade->deselectAll();

                const StringList oldMods = facade->mods();
                const StringList newMods = removeMissingMods(oldMods, issues);
                facade->setMods(newMods);
            }

            StringList removeMissingMods(StringList mods, const IssueList& issues) const {
                for (const Issue* issue : issues) {
                    if (issue->type() == MissingModIssue::Type) {
                        const MissingModIssue* modIssue = static_cast<const MissingModIssue*>(issue);
                        const String missingMod = modIssue->mod();
                        VectorUtils::erase(mods, missingMod);
                    }
                }
                return mods;
            }
        };

        MissingModIssueGenerator::MissingModIssueGenerator(GameWPtr game) :
        IssueGenerator(MissingModIssue::Type, "Missing mod directory"),
        m_game(game) {
            addQuickFix(new MissingModIssueQuickFix());
        }

        void MissingModIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            assert(node != nullptr);

            if (node->classname() != AttributeValues::WorldspawnClassname) {
                return;
            }

            if (expired(m_game)) {
                return;
            }

            GameSPtr game = lock(m_game);
            const StringList mods = game->extractEnabledMods(*node);

            if (mods == m_lastMods) {
                return;
            }

            const IO::Path::List additionalSearchPaths = IO::Path::asPaths(mods);
            const Game::PathErrors errors = game->checkAdditionalSearchPaths(additionalSearchPaths);
            using PathError = Game::PathErrors::value_type;

            std::transform(std::begin(errors), std::end(errors), std::back_inserter(issues), [node](const PathError& error) {
                const IO::Path& searchPath = error.first;
                const String& message = error.second;
                return new MissingModIssue(node, searchPath.asString(), message);
            });

            m_lastMods = mods;
        }
    }
}
