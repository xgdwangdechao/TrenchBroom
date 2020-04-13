/*
Copyright (C) 2020 Kristian Duske

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

#ifndef BrushFaceHandle_hpp
#define BrushFaceHandle_hpp

#include "FloatType.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Model {
        class BrushFace;
        class BrushFaceAttributes;
        class BrushNode;
        class TagManager;
        class TexCoordSystemSnapshot;
        enum class WrapStyle;

        /**
         * A brush face handle represents a brush face and additionally gives access to its containing brush node.
         *
         * Note that brush faces are volatile and may be deleted when a brush is modified. Care must be taken to prevent
         * stale brush face handles which reference non existing brush faces.
         */
        class BrushFaceHandle {
        private:
            BrushNode* m_node;
            BrushFace* m_face;
        public:
            /**
             * Creates a n brush face handle.
             *
             * @param node the containing brush node, must not be null
             * @param face the brush face, must not be null and must belong to the given brush node
             */
            BrushFaceHandle(BrushNode* node, BrushFace* face);

            /**
             * Returns the containing brush node.
             */
            BrushNode* node() const;

            /**
             * Returns the brush face.
             */
            const BrushFace* face() const;

            void selectFace();
            void deselectFace();
            
            const BrushFaceAttributes& attributes() const;
            void setAttributes(const BrushFaceAttributes& attribs);
            
            void copyTexCoordSystemFromFace(const TexCoordSystemSnapshot& coordSystemSnapshot, const BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, WrapStyle wrapStyle);
            void restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot& snapshot);
            void resetTextureAxes();

            void moveTexture(const vm::vec3& up, const vm::vec3& right, const vm::vec2f& offset);
            void rotateTexture(float angle);
            void shearTexture(const vm::vec2f& factors);

            void setTexture(Assets::Texture* texture);
            
            void updateFaceTags(TagManager& tagManager);
            
            /**
             * Returns true if the given handles represent the same face.
             */
            friend bool operator==(const BrushFaceHandle& lhs, const BrushFaceHandle& rhs);

            /**
             * Returns true if the given handles do not represent the same face.
             */
            friend bool operator!=(const BrushFaceHandle& lhs, const BrushFaceHandle& rhs);
        };

        /**
         * Returns a vector containing the faces represented by the given handles.
         */
        std::vector<const BrushFace*> toFaces(const std::vector<BrushFaceHandle>& handles);
    }
}

#endif /* BrushFaceHandle_hpp */
