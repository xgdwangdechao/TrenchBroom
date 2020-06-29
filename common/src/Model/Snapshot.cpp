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

#include "Snapshot.h"

#include "Exceptions.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/Node.h"
#include "Model/NodeSnapshot.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        Snapshot::~Snapshot() {
            kdl::vec_clear_and_delete(m_nodeSnapshots);
        }

        Snapshot::SnapshotResult Snapshot::restoreNodes(const vm::bbox3& worldBounds) {
            std::vector<SnapshotException> errors;
            for (NodeSnapshot* snapshot : m_nodeSnapshots) {
                const auto restoreResult = snapshot->restore(worldBounds);
                kdl::visit_result(kdl::overload {
                    []() {},
                    [&](const std::vector<SnapshotException>& e) { kdl::vec_append(errors, e); }
                }, restoreResult);
            }
            return errors.empty()
                ? SnapshotResult::success()
                : SnapshotResult::error(std::move(errors));
        }

        void Snapshot::takeSnapshot(Node* node) {
            NodeSnapshot* snapshot = node->takeSnapshot();
            if (snapshot != nullptr)
                m_nodeSnapshots.push_back(snapshot);
        }
    }
}
