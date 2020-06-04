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

#include "ObjectRenderer.h"

#include "Model/GroupNode.h"

namespace TrenchBroom {
    namespace Renderer {
        void ObjectRenderer::setObjects(const std::vector<Model::GroupNode*>& groups, const std::vector<Model::EntityNode*>& entities) {
            m_groupRenderer.setGroups(groups);
            m_entityRenderer.setEntities(entities);
        }

        void ObjectRenderer::invalidate() {
            m_groupRenderer.invalidate();
            m_entityRenderer.invalidate();
        }

        void ObjectRenderer::clear() {
            m_groupRenderer.clear();
            m_entityRenderer.clear();
        }

        void ObjectRenderer::reloadModels() {
            m_entityRenderer.reloadModels();
        }

        void ObjectRenderer::setShowOverlays(const bool showOverlays) {
            m_groupRenderer.setShowOverlays(showOverlays);
            m_entityRenderer.setShowOverlays(showOverlays);
        }

        void ObjectRenderer::setEntityOverlayTextColor(const Color &overlayTextColor) {
            m_entityRenderer.setOverlayTextColor(overlayTextColor);
        }

        void ObjectRenderer::setGroupOverlayTextColor(const Color &overlayTextColor) {
            m_groupRenderer.setOverlayTextColor(overlayTextColor);
        }

        void ObjectRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_groupRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
            m_entityRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
        }

        void ObjectRenderer::setTint(const bool tint) {
            m_entityRenderer.setTint(tint);
        }

        void ObjectRenderer::setTintColor(const Color& tintColor) {
            m_entityRenderer.setTintColor(tintColor);
        }

        void ObjectRenderer::setShowOccludedObjects(const bool showOccludedObjects) {
            m_groupRenderer.setShowOccludedBounds(showOccludedObjects);
            m_groupRenderer.setShowOccludedOverlays(showOccludedObjects);
            m_entityRenderer.setShowOccludedBounds(showOccludedObjects);
            m_entityRenderer.setShowOccludedOverlays(showOccludedObjects);
        }

        void ObjectRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
            m_groupRenderer.setOccludedBoundsColor(occludedEdgeColor);
            m_entityRenderer.setOccludedBoundsColor(occludedEdgeColor);
        }

        void ObjectRenderer::setShowEntityAngles(const bool showAngles) {
            m_entityRenderer.setShowAngles(showAngles);
        }

        void ObjectRenderer::setEntityAngleColor(const Color& color) {
            m_entityRenderer.setAngleColor(color);
        }

        void ObjectRenderer::setOverrideGroupBoundsColor(const bool overrideGroupBoundsColor) {
            m_groupRenderer.setOverrideBoundsColor(overrideGroupBoundsColor);
        }

        void ObjectRenderer::setGroupBoundsColor(const Color& color) {
            m_groupRenderer.setBoundsColor(color);
        }

        void ObjectRenderer::setOverrideEntityBoundsColor(const bool overrideEntityBoundsColor) {
            m_entityRenderer.setOverrideBoundsColor(overrideEntityBoundsColor);
        }

        void ObjectRenderer::setEntityBoundsColor(const Color& color) {
            m_entityRenderer.setBoundsColor(color);
        }

        void ObjectRenderer::setShowHiddenObjects(const bool showHiddenObjects) {
            m_entityRenderer.setShowHiddenEntities(showHiddenObjects);
        }

        void ObjectRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_entityRenderer.render(renderContext, renderBatch);
            m_groupRenderer.render(renderContext, renderBatch);
        }
    }
}
