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

#ifndef LongAttributeValueIssueGenerator_h
#define LongAttributeValueIssueGenerator_h

#include "Model/IssueGenerator.h"
#include "Model/ModelTypes.h"
#include "StringList.h"

namespace TrenchBroom {
    namespace Model {
        class LongAttributeValueIssueGenerator : public IssueGenerator {
        private:
            class LongAttributeValueIssue;
            class TruncateLongAttributeValueIssueQuickFix;
        private:
            size_t m_maxLength;
        public:
            LongAttributeValueIssueGenerator(size_t maxLength);
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class EmptyAttributeValueIssueGenerator : public IssueGenerator {
        private:
            class EmptyAttributeValueIssue;
            class EmptyAttributeValueIssueQuickFix;
        public:
            EmptyAttributeValueIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };

        class LinkSourceIssueGenerator : public IssueGenerator {
        private:
            class LinkSourceIssue;
            class LinkSourceIssueQuickFix;
        public:
            LinkSourceIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class MissingDefinitionIssueGenerator : public IssueGenerator {
        private:
            class MissingDefinitionIssue;
            class MissingDefinitionIssueQuickFix;
        public:
            MissingDefinitionIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class EmptyAttributeNameIssueGenerator : public IssueGenerator {
        private:
            class EmptyAttributeNameIssue;
            class EmptyAttributeNameIssueQuickFix;
        public:
            EmptyAttributeNameIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class AttributeValueWithDoubleQuotationMarksIssueGenerator : public IssueGenerator {
        private:
            class AttributeValueWithDoubleQuotationMarksIssue;
        public:
            AttributeValueWithDoubleQuotationMarksIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };

        class InvalidTextureScaleIssueGenerator : public IssueGenerator {
        private:
            class InvalidTextureScaleIssue;
            class InvalidTextureScaleIssueQuickFix;
        public:
            InvalidTextureScaleIssueGenerator();
        private:
            void doGenerate(Brush* brush, IssueList& issues) const override;
        };
        
        class EmptyBrushEntityIssueGenerator : public IssueGenerator {
        private:
            class EmptyBrushEntityIssue;
            class EmptyBrushEntityIssueQuickFix;
        public:
            EmptyBrushEntityIssueGenerator();
        private:
            void doGenerate(Entity* entity, IssueList& issues) const override;
        };
        
        class MissingClassnameIssueGenerator : public IssueGenerator {
        private:
            class MissingClassnameIssue;
            class MissingClassnameIssueQuickFix;
        public:
            MissingClassnameIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class LongAttributeNameIssueGenerator : public IssueGenerator {
        private:
            class LongAttributeNameIssue;
        private:
            size_t m_maxLength;
        public:
            LongAttributeNameIssueGenerator(size_t maxLength);
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class AttributeNameWithDoubleQuotationMarksIssueGenerator : public IssueGenerator {
        private:
            class AttributeNameWithDoubleQuotationMarksIssue;
        public:
            AttributeNameWithDoubleQuotationMarksIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class EmptyGroupIssueGenerator : public IssueGenerator {
        private:
            class EmptyGroupIssue;
            class EmptyGroupIssueQuickFix;
        public:
            EmptyGroupIssueGenerator();
        private:
            void doGenerate(Group* group, IssueList& issues) const override;
        };
        
        class NonIntegerPlanePointsIssueGenerator : public IssueGenerator {
        private:
            class NonIntegerPlanePointsIssue;
            class NonIntegerPlanePointsIssueQuickFix;
        public:
            NonIntegerPlanePointsIssueGenerator();
        private:
            void doGenerate(Brush* brush, IssueList& issues) const override;
        };

        class LinkTargetIssueGenerator : public IssueGenerator {
        private:
            class LinkTargetIssue;
            class LinkTargetIssueQuickFix;
        public:
            LinkTargetIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
            void processKeys(AttributableNode* node, const Model::AttributeNameList& names, IssueList& issues) const;
        };
        
        class PointEntityWithBrushesIssueGenerator : public IssueGenerator {
        private:
            class PointEntityWithBrushesIssue;
            class PointEntityWithBrushesIssueQuickFix;
        public:
            PointEntityWithBrushesIssueGenerator();
        private:
            void doGenerate(Entity* entity, IssueList& issues) const override;
        };
        
        class MissingModIssueGenerator : public IssueGenerator {
        private:
            class MissingModIssue;
            class MissingModIssueQuickFix;

            GameWPtr m_game;
            mutable StringList m_lastMods;
        public:
            MissingModIssueGenerator(GameWPtr game);
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
        
        class WorldBoundsIssueGenerator : public IssueGenerator {
        private:
            class WorldBoundsIssue;
            class WorldBoundsIssueQuickFix;
        private:
            const vm::bbox3 m_bounds;
        public:
            WorldBoundsIssueGenerator(const vm::bbox3& bounds);
        private:
            void doGenerate(Entity* brush, IssueList& issues) const override;
            void doGenerate(Brush* brush, IssueList& issues) const override;
        };
        
        class NonIntegerVerticesIssueGenerator : public IssueGenerator {
        private:
            class NonIntegerVerticesIssue;
            class NonIntegerVerticesIssueQuickFix;
        public:
            NonIntegerVerticesIssueGenerator();
        private:
            void doGenerate(Brush* brush, IssueList& issues) const override;
        };

        class MixedBrushContentsIssueGenerator : public IssueGenerator {
        private:
            class MixedBrushContentsIssue;
        public:
            MixedBrushContentsIssueGenerator();
        private:
            void doGenerate(Brush* brush, IssueList& issues) const override;
        };
    }
}

#endif /* defined(TrenchBroom_IssueGenerators) */
