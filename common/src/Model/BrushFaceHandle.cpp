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

#include "BrushFaceHandle.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        BrushFaceHandle::BrushFaceHandle(BrushNode* node, BrushFace* face) :
        m_node(node),
        m_face(face) {
            assert(m_node != nullptr);
            assert(m_face != nullptr);
            ensure(kdl::vec_contains(m_node->brush().faces(), face), "face must belong to node");
        }

        BrushNode* BrushFaceHandle::node() const {
            return m_node;
        }

        const BrushFace* BrushFaceHandle::face() const {
            return m_face;
        }

        void BrushFaceHandle::selectFace() {
            m_face->select();
        }

        void BrushFaceHandle::deselectFace() {
            m_face->deselect();
        }

        const BrushFaceAttributes& BrushFaceHandle::attributes() const {
            return m_face->attributes();
        }

        void BrushFaceHandle::setAttributes(const BrushFaceAttributes& attribs) {
            m_node->setFaceAttributes(m_face, attribs);
        }

        void BrushFaceHandle::copyTexCoordSystemFromFace(const TexCoordSystemSnapshot& coordSystemSnapshot, const BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const WrapStyle wrapStyle) {
            m_node->copyTexCoordSystemFromFace(m_face, coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
        }

        void BrushFaceHandle::restoreTexCoordSystemSnapshot(const TexCoordSystemSnapshot& snapshot) {
            m_node->restoreTexCoordSystemSnapshot(m_face, snapshot);
        }

        void BrushFaceHandle::moveTexture(const vm::vec3& up, const vm::vec3& right, const vm::vec2f& offset) {
            m_node->moveTexture(m_face, up, right, offset);
        }
        
        void BrushFaceHandle::rotateTexture(const float angle) {
            m_node->rotateTexture(m_face, angle);
        }
        
        void BrushFaceHandle::shearTexture(const vm::vec2f& factors) {
            m_node->shearTexture(m_face, factors);
        }

        void BrushFaceHandle::resetTextureAxes() {
            m_node->resetTextureAxes(m_face);
        }

        void BrushFaceHandle::setTexture(Assets::Texture* texture) {
            m_node->setTexture(m_face, texture);
        }

        void BrushFaceHandle::updateFaceTags(TagManager& tagManager) {
            m_node->updateFaceTags(m_face, tagManager);
        }

        bool operator==(const BrushFaceHandle& lhs, const BrushFaceHandle& rhs) {
            return lhs.m_node == rhs.m_node && lhs.m_face == rhs.m_face;
        }

        std::vector<const BrushFace*> toFaces(const std::vector<BrushFaceHandle>& handles) {
            return kdl::vec_transform(handles, [](const auto& handle) { return handle.face(); });
        }
    }
}
