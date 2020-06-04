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

#ifndef TrenchBroom_BrushRendererBrushCache
#define TrenchBroom_BrushRendererBrushCache

#include "Renderer/GLVertexType.h"

#include <vector>
#include <tuple>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushNode;
        class BrushFace;
    }

    namespace Renderer {
        namespace BrushRendererBrushCache {
            using VertexSpec = Renderer::GLVertexTypes::P3NT2C4;
            using Vertex = VertexSpec::Vertex;
            
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

        std::tuple<std::vector<BrushRendererBrushCache::Vertex>,
                   std::vector<BrushRendererBrushCache::CachedFace>,
                   std::vector<BrushRendererBrushCache::CachedEdge>> validateVertexCache(const Model::BrushNode* brushNode);       
    }
}

#endif /* defined(TrenchBroom_BrushRendererBrushCache) */
