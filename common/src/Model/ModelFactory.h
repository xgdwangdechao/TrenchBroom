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

#ifndef TrenchBroom_ModelFactory
#define TrenchBroom_ModelFactory

#include "FloatType.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        enum class BrushError;
        class BrushFace;
        class BrushNode;
        class BrushFaceAttributes;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        enum class MapFormat;
        class WorldNode;

        class ModelFactory {
        public:
            virtual ~ModelFactory();

            MapFormat format() const;
            WorldNode* createWorld() const;
            LayerNode* createLayer(const std::string& name) const;
            GroupNode* createGroup(const std::string& name) const;
            EntityNode* createEntity() const;
            BrushNode* createBrush(Brush brush) const;

            kdl::result<BrushFace, BrushError> createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const;
            kdl::result<BrushFace, BrushError> createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const;
        private:
            virtual MapFormat doGetFormat() const = 0;
            virtual WorldNode* doCreateWorld() const = 0;
            virtual LayerNode* doCreateLayer(const std::string& name) const = 0;
            virtual GroupNode* doCreateGroup(const std::string& name) const = 0;
            virtual EntityNode* doCreateEntity() const = 0;
            virtual BrushNode* doCreateBrush(Brush brush) const;
            virtual kdl::result<BrushFace, BrushError> doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const = 0;
            virtual kdl::result<BrushFace, BrushError> doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_ModelFactory) */
