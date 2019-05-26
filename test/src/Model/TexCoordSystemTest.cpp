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

#include <gtest/gtest.h>

#include "TrenchBroom.h"
#include "Exceptions.h"
#include "TestUtils.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/TexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ModelTypes.h"

#include <vecmath/vec.h>

#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Model {
        // Disable a clang warning when using ASSERT_DEATH
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

        TEST(TexCoordSystemTest, testSnapshotTypeSafety) {
            BrushFaceAttributes attribs("");

            ParaxialTexCoordSystem paraxial(vm::vec3::pos_z, attribs);
            ASSERT_EQ(nullptr, paraxial.takeSnapshot());

            ParallelTexCoordSystem parallel(vm::vec3::pos_y, vm::vec3::pos_x);
            auto parallelSnapshot = parallel.takeSnapshot();
            ASSERT_NE(nullptr, parallelSnapshot);

            ASSERT_DEATH(parallelSnapshot->restore(paraxial), "");
            parallelSnapshot->restore(parallel);
        }

#ifdef __clang__
#pragma clang diagnostic pop
#endif

        TEST(TexCoordSystemTest, testParallelCoordSystemRotation) {
            // 1 unit of X in world space = 1 unit of X in texture space
            // 1 unit of Y in world space = 0.1 unit of Y in texture space
            ParallelTexCoordSystem texCoordSystem(vm::vec3::pos_x, vm::vec3(0.0, 0.1, 0.0));

            // Check the world->tex matrix

            const vm::mat4x4 worldToTex = texCoordSystem.toMatrix(vm::vec2f::zero, vm::vec2f::one);
            const vm::mat4x4 texToWorld = texCoordSystem.fromMatrix(vm::vec2f::zero, vm::vec2f::one);

            //            World(X,   Y,   Z,   1)
            ASSERT_EQ(vm::vec4d(1,   0,   0,   0), vm::row<0>(worldToTex)); // Tex X
            ASSERT_EQ(vm::vec4d(0,   0.1, 0,   0), vm::row<1>(worldToTex)); // Tex Y
            ASSERT_EQ(vm::vec4d(0,   0,   1,   0), vm::row<2>(worldToTex)); // Tex Z
            ASSERT_EQ(vm::vec4d(0,   0,   0,   1), vm::row<3>(worldToTex)); // 1

            // Test a 45 degree CCW rotation.

            ParallelTexCoordSystem rotatedTexCoordSystem(texCoordSystem.xAxis(), texCoordSystem.yAxis());
            rotatedTexCoordSystem.setRotation(vm::vec3::NaN, 0.0f, 45.0f);

            const vm::mat4x4 texToWorldForRotated = rotatedTexCoordSystem.fromMatrix(vm::vec2f::zero, vm::vec2f::one);

            // Measure the angles in world space between the two tex coord system X axes. It should be 45 degrees.

            const vm::vec3d worldVec1 = vm::vec3d(texToWorld * vm::vec4d(1, 0, 0, 1));
            const vm::vec3d worldVec2 = vm::vec3d(texToWorldForRotated * vm::vec4d(1, 0, 0, 1));

            const double angle =
                vm::toDegrees(vm::measureAngle(
                    vm::normalize(worldVec2),
                    vm::normalize(worldVec1),
                    vm::vec3::pos_z));

            ASSERT_NEAR(45.0, angle, 0.00001);
        }
    }
}
