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

#include "BrushRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/Polyhedron.h"
#include "Model/TagAttribute.h"
#include "Renderer/BrushRendererArrays.h"
#include "Renderer/RenderContext.h"

#include <cassert>
#include <cstring>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        // BrushRendererBrushCache

        namespace BrushRendererBrushCache {
            using VertexSpec = Renderer::GLVertexTypes::P3NT2C4;
            using Vertex = VertexSpec::Vertex;

            using EdgeVertexSpec = Renderer::GLVertexTypes::P3C4;
            using EdgeVertex = EdgeVertexSpec::Vertex;
            
            struct CachedFace {
                const Assets::Texture* texture;
                const Model::BrushFace* face;
                size_t vertexCount;
                size_t indexOfFirstVertexRelativeToBrush;

                CachedFace(const Model::BrushFace* i_face,
                           size_t i_indexOfFirstVertexRelativeToBrush);
            };

            struct CachedEdge {
                const Model::BrushFace* face1;
                const Model::BrushFace* face2;
                size_t vertexIndex1RelativeToBrush;
                size_t vertexIndex2RelativeToBrush;

                CachedEdge(const Model::BrushFace* i_face1,
                           const Model::BrushFace* i_face2,
                           size_t i_vertexIndex1RelativeToBrush,
                           size_t i_vertexIndex2RelativeToBrush);
            };
        }

        BrushRendererBrushCache::CachedFace::CachedFace(const Model::BrushFace* i_face,
                                                        const size_t i_indexOfFirstVertexRelativeToBrush)
                : texture(i_face->texture()),
                  face(i_face),
                  vertexCount(i_face->vertexCount()),
                  indexOfFirstVertexRelativeToBrush(i_indexOfFirstVertexRelativeToBrush) {}

        BrushRendererBrushCache::CachedEdge::CachedEdge(const Model::BrushFace* i_face1,
                                                        const Model::BrushFace* i_face2,
                                                        const size_t i_vertexIndex1RelativeToBrush,
                                                        const size_t i_vertexIndex2RelativeToBrush)
                : face1(i_face1),
                  face2(i_face2),
                  vertexIndex1RelativeToBrush(i_vertexIndex1RelativeToBrush),
                  vertexIndex2RelativeToBrush(i_vertexIndex2RelativeToBrush) {}

        /**
         * Rendering overview:
         * There are 2 things to render: brush faces (filled/textured polygons) and brush edges.
         *
         * For faces, we need to write a copy of each vertex for each face it's used on, because
         * of the texture coordinates/normal and face selection state being unique per face.
         *
         * For edges, it's a it more complicated because we only want to draw each edge once, even
         * though it's shared between 2 faces.
         */
        static std::tuple<std::vector<BrushRendererBrushCache::Vertex>,
                         std::vector<BrushRendererBrushCache::CachedFace>,
                         std::vector<BrushRendererBrushCache::CachedEdge>> validateVertexCache(const Model::BrushNode* brushNode) {

             std::vector<BrushRendererBrushCache::Vertex> m_cachedVertices;
             std::vector<BrushRendererBrushCache::CachedFace> m_cachedFacesSortedByTexture;
             std::vector<BrushRendererBrushCache::CachedEdge> m_cachedEdges;
        
            // build vertex cache and face cache
            const Model::Brush& brush = brushNode->brush();

            m_cachedVertices.clear();
            m_cachedVertices.reserve(brush.vertexCount()); // FIXME: too small reserve size

            m_cachedFacesSortedByTexture.clear();
            m_cachedFacesSortedByTexture.reserve(brush.faceCount());

            for (const Model::BrushFace& face : brush.faces()) {
                const auto indexOfFirstVertexRelativeToBrush = m_cachedVertices.size();
                const vm::vec3f faceNormal = vm::vec3f(face.boundary().normal);

                // The boundary is in CCW order, but the renderer expects CW order:
                auto& boundary = face.geometry()->boundary();
                for (auto it = std::rbegin(boundary), end = std::rend(boundary); it != end; ++it) {
                    Model::BrushHalfEdge* current = *it;
                    Model::BrushVertex* vertex = current->origin();

                    // Set the vertex payload to the index, relative to the brush's first vertex being 0.
                    // This is used below when building the edge cache.
                    // NOTE: we'll overwrite the payload as we visit the same vertex several times while visiting
                    // different faces, this is fine.
                    const auto currentIndex = m_cachedVertices.size();
                    vertex->setPayload(static_cast<GLuint>(currentIndex));

                    const auto& position = vertex->position();
                    m_cachedVertices.emplace_back(vm::vec3f(position), faceNormal, face.textureCoords(position), vm::vec4f(1.0f, 0.0f, 0.0f, 1.0f));

                    current = current->previous();
                }

                // face cache
                m_cachedFacesSortedByTexture.emplace_back(&face, indexOfFirstVertexRelativeToBrush);
            }

            // Sort by texture so BrushRenderer can efficiently step through the BrushFaces
            // grouped by texture (via `BrushRendererBrushCache::cachedFacesSortedByTexture()`), without needing to build an std::map

            std::sort(m_cachedFacesSortedByTexture.begin(),
                      m_cachedFacesSortedByTexture.end(),
                      [](const BrushRendererBrushCache::CachedFace& a, const BrushRendererBrushCache::CachedFace& b){ return a.texture < b.texture; });

            // Build edge index cache

            m_cachedEdges.clear();
            m_cachedEdges.reserve(brush.edgeCount());

            for (const Model::BrushEdge* currentEdge : brush.edges()) {
                const auto faceIndex1 = currentEdge->firstFace()->payload();
                const auto faceIndex2 = currentEdge->secondFace()->payload();
                assert(faceIndex1 && faceIndex2);
                
                const auto& face1 = brush.face(*faceIndex1);
                const auto& face2 = brush.face(*faceIndex2);
                
                const auto vertexIndex1RelativeToBrush = currentEdge->firstVertex()->payload();
                const auto vertexIndex2RelativeToBrush = currentEdge->secondVertex()->payload();

                m_cachedEdges.emplace_back(&face1, &face2, vertexIndex1RelativeToBrush, vertexIndex2RelativeToBrush);
            }

            return { m_cachedVertices, m_cachedFacesSortedByTexture, m_cachedEdges };
        }

#if 0
        bool BrushRenderer::DefaultFilter::visible(const Model::BrushNode* brush) const    {
            return m_context.visible(brush);
        }

        bool BrushRenderer::DefaultFilter::visible(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            return m_context.visible(brush, face);
        }

        bool BrushRenderer::DefaultFilter::editable(const Model::BrushNode* brush) const {
            return m_context.editable(brush);
        }

        bool BrushRenderer::DefaultFilter::editable(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            return m_context.editable(brush, face);
        }

        bool BrushRenderer::DefaultFilter::selected(const Model::BrushNode* brush) const {
            return brush->selected() || brush->parentSelected();
        }

        bool BrushRenderer::DefaultFilter::selected(const Model::BrushNode*, const Model::BrushFace& face) const {
            return face.selected();
        }

        bool BrushRenderer::DefaultFilter::hasSelectedFaces(const Model::BrushNode* brush) const {
            return brush->descendantSelected();
        }

        // SelectedBrushRendererFilter

        BrushRenderer::SelectedBrushRendererFilter::SelectedBrushRendererFilter(const Model::EditorContext& context) :
        DefaultFilter(context) {}

        BrushRenderer::Filter::RenderSettings BrushRenderer::SelectedBrushRendererFilter::markFaces(const Model::BrushNode* brushNode) const {
            if (!(visible(brushNode) && editable(brushNode))) {
                return renderNothing();
            }

            const bool brushSelected = selected(brushNode);
            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushFace& face : brush.faces()) {
                face.setMarked(brushSelected || selected(brushNode, face));
            }
            return std::make_tuple(FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderIfEitherFaceMarked);
        }

        // LockedBrushRendererFilter

        BrushRenderer::LockedBrushRendererFilter::LockedBrushRendererFilter(const Model::EditorContext& context) :
        DefaultFilter(context) {}

        BrushRenderer::Filter::RenderSettings BrushRenderer::LockedBrushRendererFilter::markFaces(const Model::BrushNode* brushNode) const {
            if (!visible(brushNode)) {
                return renderNothing();
            }

            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushFace& face : brush.faces()) {
                face.setMarked(true);
            }

            return std::make_tuple(FaceRenderPolicy::RenderMarked,
                                   EdgeRenderPolicy::RenderAll);
        }

        // UnselectedBrushRendererFilter
        
        BrushRenderer::UnselectedBrushRendererFilter::UnselectedBrushRendererFilter(const Model::EditorContext& context) :
        DefaultFilter(context) {}

        BrushRenderer::Filter::RenderSettings BrushRenderer::UnselectedBrushRendererFilter::markFaces(const Model::BrushNode* brushNode) const {
            const bool brushVisible = visible(brushNode);
            const bool brushEditable = editable(brushNode);

            const bool renderFaces = (brushVisible && brushEditable);
                  bool renderEdges = (brushVisible && !selected(brushNode));

            if (!renderFaces && !renderEdges) {
                return renderNothing();
            }

            const Model::Brush& brush = brushNode->brush();
            
            bool anyFaceVisible = false;
            for (const Model::BrushFace& face : brush.faces()) {
                const bool faceVisible = !selected(brushNode, face) && visible(brushNode, face);
                face.setMarked(faceVisible);
                anyFaceVisible |= faceVisible;
            }

            if (!anyFaceVisible) {
                return renderNothing();
            }

            // Render all edges if only one face is visible.
            renderEdges |= anyFaceVisible;

            return std::make_tuple(renderFaces ? FaceRenderPolicy::RenderMarked : FaceRenderPolicy::RenderNone,
                                   renderEdges ? EdgeRenderPolicy::RenderAll : EdgeRenderPolicy::RenderNone);
        }
#endif

        // BrushRenderer

        BrushRenderer::BrushRenderer() :
        m_showEdges(false),
        m_grayscale(false),
        m_tint(false),
        m_showOccludedEdges(false),
        m_forceTransparent(false),
        m_transparencyAlpha(1.0f),
        m_showHiddenBrushes(false) {
            clear();
        }

        void BrushRenderer::addBrushes(const std::vector<Model::BrushNode*>& brushes) {
            for (auto* brush : brushes) {
                addBrush(brush);
            }
        }

        void BrushRenderer::setBrushes(const std::vector<Model::BrushNode*>& brushes) {
            // start with adding nothing, and removing everything
            std::unordered_set<const Model::BrushNode*> toAdd;
            std::unordered_set<const Model::BrushNode*> toRemove = m_allBrushes;

            // update toAdd and toRemove using the input list
            for (const auto* brush : brushes) {
                if (toRemove.erase(brush) == 0u) {
                    toAdd.insert(brush);
                }
            }

            for (auto brush : toRemove) {
                removeBrush(brush);
            }
            for (auto brush : toAdd) {
                addBrush(brush);
            }
        }

        void BrushRenderer::invalidate() {
            for (auto& brush : m_allBrushes) {
                // this will also invalidate already invalid brushes, which
                // is unnecessary
                removeBrushFromVbo(brush);
            }
            m_invalidBrushes = m_allBrushes;

            assert(m_brushInfo.empty());
            assert(m_transparentFaces->empty());
            assert(m_opaqueFaces->empty());
        }

        void BrushRenderer::invalidateBrushes(const std::vector<Model::BrushNode*>& brushes) {
            for (auto& brush : brushes) {
                // skip brushes that are not in the renderer
                if (m_allBrushes.find(brush) == std::end(m_allBrushes)) {
                    assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
                    assert(m_invalidBrushes.find(brush) == std::end(m_invalidBrushes));
                    continue;
                }
                // if it's not in the invalid set, put it in
                if (m_invalidBrushes.insert(brush).second) {
                    removeBrushFromVbo(brush);
                }
            }
        }

        bool BrushRenderer::valid() const {
            return m_invalidBrushes.empty();
        }

        void BrushRenderer::clear() {
            m_brushInfo.clear();
            m_allBrushes.clear();
            m_invalidBrushes.clear();

            m_vertexArray = std::make_shared<BrushVertexArray>();
            m_edgeIndices = std::make_shared<BrushIndexArray>();
            m_transparentFaces = std::make_shared<TextureToBrushIndicesMap>();
            m_opaqueFaces = std::make_shared<TextureToBrushIndicesMap>();

            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, m_edgeIndices);
        }

        void BrushRenderer::setFaceColor(const Color& faceColor) {
            m_faceColor = faceColor;
        }

        void BrushRenderer::setShowEdges(const bool showEdges) {
            m_showEdges = showEdges;
        }

        void BrushRenderer::setEdgeColor(const Color& edgeColor) {
            m_edgeColor = edgeColor;
        }

        void BrushRenderer::setGrayscale(const bool grayscale) {
            m_grayscale = grayscale;
        }

        void BrushRenderer::setTint(const bool tint) {
            m_tint = tint;
        }

        void BrushRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        void BrushRenderer::setShowOccludedEdges(const bool showOccludedEdges) {
            m_showOccludedEdges = showOccludedEdges;
        }

        void BrushRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
            m_occludedEdgeColor = occludedEdgeColor;
        }

        void BrushRenderer::setForceTransparent(const bool transparent) {
            if (transparent != m_forceTransparent) {
                m_forceTransparent = transparent;
                invalidate();
            }
        }

        void BrushRenderer::setTransparencyAlpha(const float transparencyAlpha) {
            if (transparencyAlpha != m_transparencyAlpha) {
                m_transparencyAlpha = transparencyAlpha;
                invalidate();
            }
        }

        void BrushRenderer::setShowHiddenBrushes(const bool showHiddenBrushes) {
            if (showHiddenBrushes != m_showHiddenBrushes) {
                m_showHiddenBrushes = showHiddenBrushes;
                invalidate();
            }
        }

        void BrushRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            renderOpaque(renderContext, renderBatch);
            renderTransparent(renderContext, renderBatch);
        }

        void BrushRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_allBrushes.empty()) {
                if (!valid()) {
                    validate();
                }
                if (renderContext.showFaces()) {
                    renderOpaqueFaces(renderBatch);
                }
                if (renderContext.showEdges() || m_showEdges) {
                    renderEdges(renderBatch);
                }
            }
        }

        void BrushRenderer::renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_allBrushes.empty()) {
                if (!valid()) {
                    validate();
                }
                if (renderContext.showFaces()) {
                    renderTransparentFaces(renderBatch);
                }
            }
        }

        void BrushRenderer::renderOpaqueFaces(RenderBatch& renderBatch) {
            m_opaqueFaceRenderer.setGrayscale(m_grayscale);
            m_opaqueFaceRenderer.setTint(m_tint);
            m_opaqueFaceRenderer.setTintColor(m_tintColor);
            m_opaqueFaceRenderer.render(renderBatch);
        }

        void BrushRenderer::renderTransparentFaces(RenderBatch& renderBatch) {
            m_transparentFaceRenderer.setGrayscale(m_grayscale);
            m_transparentFaceRenderer.setTint(m_tint);
            m_transparentFaceRenderer.setTintColor(m_tintColor);
            m_transparentFaceRenderer.setAlpha(m_transparencyAlpha);
            m_transparentFaceRenderer.render(renderBatch);
        }

        void BrushRenderer::renderEdges(RenderBatch& renderBatch) {
            if (m_showOccludedEdges) {
                m_edgeRenderer.renderOnTop(renderBatch, m_occludedEdgeColor);
            }
            m_edgeRenderer.render(renderBatch); //, m_edgeColor); -- disable uniform color 
        }

        void BrushRenderer::validate() {
            assert(!valid());

            for (auto brush : m_invalidBrushes) {
                validateBrush(brush);
            }
            m_invalidBrushes.clear();
            assert(valid());

            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, m_edgeIndices);
        }

        static size_t triIndicesCountForPolygon(const size_t vertexCount) {
            assert(vertexCount >= 3);
            const size_t indexCount = 3 * (vertexCount - 2);
            return indexCount;
        }

        static void addTriIndicesForPolygon(GLuint* dest, const GLuint baseIndex, const size_t vertexCount) {
            assert(vertexCount >= 3);
            for (size_t i = 0; i < vertexCount - 2; ++i) {
                *(dest++) = baseIndex;
                *(dest++) = baseIndex + static_cast<GLuint>(i + 1);
                *(dest++) = baseIndex + static_cast<GLuint>(i + 2);
            }
        }

#if 0
        static inline bool shouldRenderEdge(const BrushRendererBrushCache::CachedEdge& edge,
                                            const BrushRenderer::Filter::EdgeRenderPolicy policy) {
            using EdgeRenderPolicy = BrushRenderer::Filter::EdgeRenderPolicy;

            switch (policy) {
                case EdgeRenderPolicy::RenderAll:
                    return true;
                case EdgeRenderPolicy::RenderIfEitherFaceMarked:
                    return (edge.face1 && edge.face1->isMarked()) || (edge.face2 && edge.face2->isMarked());
                case EdgeRenderPolicy::RenderIfBothFacesMarked:
                    return (edge.face1 && edge.face1->isMarked()) && (edge.face2 && edge.face2->isMarked());
                case EdgeRenderPolicy::RenderNone:
                    return false;
                switchDefault()
            }
        }
#endif

        static size_t countMarkedEdgeIndices(const std::vector<BrushRendererBrushCache::CachedEdge>& cachedEdges) {
            size_t indexCount = 0;
            for (const auto& edge : cachedEdges) {
                if (true /* shouldRenderEdge(edge, policy)*/) {
                    indexCount += 2;
                }
            }
            return indexCount;
        }

        static void getMarkedEdgeIndices(const std::vector<BrushRendererBrushCache::CachedEdge>& cachedEdges,
                                         const GLuint brushVerticesStartIndex,
                                         GLuint* dest) {
            size_t i = 0;
            for (const auto& edge : cachedEdges) {
                if (true /* shouldRenderEdge(edge, policy) */) {
                    dest[i++] = static_cast<GLuint>(brushVerticesStartIndex + edge.vertexIndex1RelativeToBrush);
                    dest[i++] = static_cast<GLuint>(brushVerticesStartIndex + edge.vertexIndex2RelativeToBrush);
                }
            }
        }

        bool BrushRenderer::shouldDrawFaceInTransparentPass(const Model::BrushNode* brush, const Model::BrushFace& face) const {
            if (m_transparencyAlpha >= 1.0f) {
                // In this case, draw everything in the opaque pass
                // see: https://github.com/kduske/TrenchBroom/issues/2848
                return false;
            }

            if (m_forceTransparent) {
                return true;
            }
            if (brush->hasAttribute(Model::TagAttributes::Transparency)) {
                return true;
            }
            if (face.hasAttribute(Model::TagAttributes::Transparency)) {
                return true;
            }
            return false;
        }

        void BrushRenderer::validateBrush(const Model::BrushNode* brush) {
            assert(m_allBrushes.find(brush) != std::end(m_allBrushes));
            assert(m_invalidBrushes.find(brush) != std::end(m_invalidBrushes));
            assert(m_brushInfo.find(brush) == std::end(m_brushInfo));

            // At this point, brush is not in the VBO's and will not be rendered.

            // FIXME: handle m_showHiddenBrushes
#if 0
            const NoFilter filter;
            const FilterWrapper wrapper(filter, m_showHiddenBrushes);

            // evaluate filter. only evaluate the filter once per brush.
            const auto settings = wrapper.markFaces(brush);
            const auto [facePolicy, edgePolicy] = settings;

            if (facePolicy == Filter::FaceRenderPolicy::RenderNone &&
                edgePolicy == Filter::EdgeRenderPolicy::RenderNone) {
                // NOTE: this skips inserting the brush into m_brushInfo
                return;
            }
#endif
            BrushInfo& info = m_brushInfo[brush];

            // collect vertices
            const auto [cachedVertices, facesSortedByTex, cachedEdges] = validateVertexCache(brush);
            ensure(!cachedVertices.empty(), "Brush must have cached vertices");

            assert(m_vertexArray != nullptr);
            auto [vertBlock, dest] = m_vertexArray->getPointerToInsertVerticesAt(cachedVertices.size());
            std::memcpy(dest, cachedVertices.data(), cachedVertices.size() * sizeof(*dest));
            static_assert(sizeof(dest[0]) == sizeof(cachedVertices.data()[0]));
            info.vertexHolderKey = vertBlock;

            const auto brushVerticesStartIndex = static_cast<GLuint>(vertBlock->pos);

            // insert edge indices into VBO
            {
                const size_t edgeIndexCount = countMarkedEdgeIndices(cachedEdges);
                if (edgeIndexCount > 0) {
                    auto [key, insertDest] = m_edgeIndices->getPointerToInsertElementsAt(edgeIndexCount);
                    info.edgeIndicesKey = key;
                    getMarkedEdgeIndices(cachedEdges, brushVerticesStartIndex, insertDest);
                } else {
                    // it's possible to have no edges to render
                    // e.g. select all faces of a brush, and the unselected brush renderer
                    // will hit this branch.
                    ensure(info.edgeIndicesKey == nullptr, "BrushInfo not initialized");
                }
            }

            // insert face indices

            const size_t facesSortedByTexSize = facesSortedByTex.size();

            size_t nextI;
            for (size_t i = 0; i < facesSortedByTexSize; i = nextI) {
                const Assets::Texture* texture = facesSortedByTex[i].texture;

                size_t opaqueIndexCount = 0;
                size_t transparentIndexCount = 0;

                // find the i value for the next texture
                for (nextI = i + 1; nextI < facesSortedByTexSize && facesSortedByTex[nextI].texture == texture; ++nextI) {}

                // process all faces with this texture (they'll be consecutive)
                for (size_t j = i; j < nextI; ++j) {
                    const BrushRendererBrushCache::CachedFace& cache = facesSortedByTex[j];
                    if (true /* cache.face->isMarked() */) {
                        assert(cache.texture == texture);
                        if (shouldDrawFaceInTransparentPass(brush, *cache.face)) {
                            transparentIndexCount += triIndicesCountForPolygon(cache.vertexCount);
                        } else {
                            opaqueIndexCount += triIndicesCountForPolygon(cache.vertexCount);
                        }
                    }
                }

                if (transparentIndexCount > 0) {
                    TextureToBrushIndicesMap& faceVboMap = *m_transparentFaces;
                    auto& holderPtr = faceVboMap[texture];
                    if (holderPtr == nullptr) {
                        // inserts into map!
                        holderPtr = std::make_shared<BrushIndexArray>();
                    }

                    auto [key, insertDest] = holderPtr->getPointerToInsertElementsAt(transparentIndexCount);
                    info.transparentFaceIndicesKeys.push_back({texture, key});

                    // process all faces with this texture (they'll be consecutive)
                    GLuint *currentDest = insertDest;
                    for (size_t j = i; j < nextI; ++j) {
                        const BrushRendererBrushCache::CachedFace& cache = facesSortedByTex[j];
                        if (/*cache.face->isMarked() && */ shouldDrawFaceInTransparentPass(brush, *cache.face)) {
                            addTriIndicesForPolygon(currentDest,
                                                    static_cast<GLuint>(brushVerticesStartIndex +
                                                                        cache.indexOfFirstVertexRelativeToBrush),
                                                    cache.vertexCount);

                            currentDest += triIndicesCountForPolygon(cache.vertexCount);
                        }
                    }
                    assert(currentDest == (insertDest + transparentIndexCount));
                }

                if (opaqueIndexCount > 0) {
                    TextureToBrushIndicesMap& faceVboMap = *m_opaqueFaces;
                    auto& holderPtr = faceVboMap[texture];
                    if (holderPtr == nullptr) {
                        // inserts into map!
                        holderPtr = std::make_shared<BrushIndexArray>();
                    }

                    auto [key, insertDest] = holderPtr->getPointerToInsertElementsAt(opaqueIndexCount);
                    info.opaqueFaceIndicesKeys.push_back({texture, key});

                    // process all faces with this texture (they'll be consecutive)
                    GLuint *currentDest = insertDest;
                    for (size_t j = i; j < nextI; ++j) {
                        const BrushRendererBrushCache::CachedFace& cache = facesSortedByTex[j];
                        if (/* cache.face->isMarked() && */ !shouldDrawFaceInTransparentPass(brush, *cache.face)) {
                            addTriIndicesForPolygon(currentDest,
                                                    static_cast<GLuint>(brushVerticesStartIndex +
                                                                        cache.indexOfFirstVertexRelativeToBrush),
                                                    cache.vertexCount);

                            currentDest += triIndicesCountForPolygon(cache.vertexCount);
                        }
                    }
                    assert(currentDest == (insertDest + opaqueIndexCount));
                }
            }
        }

        void BrushRenderer::addBrush(const Model::BrushNode* brush) {
            // i.e. insert the brush as "invalid" if it's not already present.
            // if it is present, its validity is unchanged.
            if (m_allBrushes.insert(brush).second) {
                assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
                assertResult(m_invalidBrushes.insert(brush).second);
            }
        }

        void BrushRenderer::removeBrush(const Model::BrushNode* brush) {
            // update m_brushValid
            assertResult(m_allBrushes.erase(brush) > 0u);

            if (m_invalidBrushes.erase(brush) > 0u) {
                // invalid brushes are not in the VBO, so we can return  now.
                assert(m_brushInfo.find(brush) == std::end(m_brushInfo));
                return;
            }

            removeBrushFromVbo(brush);
        }

        void BrushRenderer::removeBrushFromVbo(const Model::BrushNode* brush) {
            auto it = m_brushInfo.find(brush);

            if (it == std::end(m_brushInfo)) {
                // This means BrushRenderer::validateBrush skipped rendering the brush, so it was never
                // uploaded to the VBO's
                return;
            }

            const BrushInfo& info = it->second;

            // update Vbo's
            m_vertexArray->deleteVerticesWithKey(info.vertexHolderKey);
            if (info.edgeIndicesKey != nullptr) {
                m_edgeIndices->zeroElementsWithKey(info.edgeIndicesKey);
            }

            for (const auto& [texture, opaqueKey] : info.opaqueFaceIndicesKeys) {
                std::shared_ptr<BrushIndexArray> faceIndexHolder = m_opaqueFaces->at(texture);
                faceIndexHolder->zeroElementsWithKey(opaqueKey);

                if (!faceIndexHolder->hasValidIndices()) {
                    // There are no indices left to render for this texture, so delete the <Texture, BrushIndexArray> entry from the map
                    m_opaqueFaces->erase(texture);
                }
            }
            for (const auto& [texture, transparentKey] : info.transparentFaceIndicesKeys) {
                std::shared_ptr<BrushIndexArray> faceIndexHolder = m_transparentFaces->at(texture);
                faceIndexHolder->zeroElementsWithKey(transparentKey);

                if (!faceIndexHolder->hasValidIndices()) {
                    // There are no indices left to render for this texture, so delete the <Texture, BrushIndexArray> entry from the map
                    m_transparentFaces->erase(texture);
                }
            }

            m_brushInfo.erase(it);
        }
    }
}
