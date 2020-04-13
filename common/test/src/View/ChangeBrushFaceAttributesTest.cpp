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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TestUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class ChangeBrushFaceAttributesTest : public MapDocumentTest {
        public:
            ChangeBrushFaceAttributesTest() :
            MapDocumentTest(Model::MapFormat::Valve) {}
        };

        TEST_CASE_METHOD(ChangeBrushFaceAttributesTest, "ChangeBrushFaceAttributesTest.resetAttributesOfValve220Face") {
            Model::BrushNode* brushNode = createBrushNode();
            document->addNode(brushNode, document->currentParent());

            const Model::BrushFaceHandle faceHandle = brushNode->faceHandles().front();
            const vm::vec3 initialX = faceHandle.face()->textureXAxis();
            const vm::vec3 initialY = faceHandle.face()->textureYAxis();

            document->select(faceHandle);

            Model::ChangeBrushFaceAttributesRequest rotate;
            rotate.addRotation(2.0);
            for (size_t i = 0; i < 5; ++i)
                document->setFaceAttributes(rotate);

            ASSERT_FLOAT_EQ(10.0, faceHandle.attributes().rotation());

            Model::ChangeBrushFaceAttributesRequest reset;
            reset.resetAll();

            document->setFaceAttributes(reset);

            ASSERT_FLOAT_EQ(0.0f, faceHandle.attributes().xOffset());
            ASSERT_FLOAT_EQ(0.0f, faceHandle.attributes().yOffset());
            ASSERT_FLOAT_EQ(0.0f, faceHandle.attributes().rotation());
            ASSERT_FLOAT_EQ(1.0f, faceHandle.attributes().xScale());
            ASSERT_FLOAT_EQ(1.0f, faceHandle.attributes().yScale());

            ASSERT_VEC_EQ(initialX, faceHandle.face()->textureXAxis());
            ASSERT_VEC_EQ(initialY, faceHandle.face()->textureYAxis());
        }
    }
}
