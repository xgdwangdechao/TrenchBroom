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

#include "Commands.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType AddBrushVerticesCommand::Type = Command::freeType();

        AddBrushVerticesCommand::Ptr AddBrushVerticesCommand::add(const Model::VertexToBrushesMap& vertices) {
            Model::BrushSet allBrushSet;
            for (const auto& entry : vertices) {
                const Model::BrushSet& brushes = entry.second;
                SetUtils::merge(allBrushSet, brushes);
            }

            const Model::BrushList allBrushList(std::begin(allBrushSet), std::end(allBrushSet));
            const String actionName = StringUtils::safePlural(vertices.size(), "Add Vertex", "Add Vertices");
            return Ptr(new AddBrushVerticesCommand(Type, actionName, allBrushList, vertices));
        }

        AddBrushVerticesCommand::AddBrushVerticesCommand(CommandType type, const String& name, const Model::BrushList& brushes, const Model::VertexToBrushesMap& vertices) :
        VertexCommand(type, name, brushes),
        m_vertices(vertices) {}

        bool AddBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                const vm::vec3& position = entry.first;
                const Model::BrushSet& brushes = entry.second;
                for (const Model::Brush* brush : brushes) {
                    if (!brush->canAddVertex(worldBounds, position))
                        return false;
                }
            }
            return true;
        }

        bool AddBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            document->performAddVertices(m_vertices);
            return true;
        }

        bool AddBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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


#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/Node.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType AddRemoveNodesCommand::Type = Command::freeType();

        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::add(Model::Node* parent, const Model::NodeList& children) {
            ensure(parent != nullptr, "parent is null");
            Model::ParentChildrenMap nodes;
            nodes[parent] = children;

            return add(nodes);
        }

        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::add(const Model::ParentChildrenMap& nodes) {
            return Ptr(new AddRemoveNodesCommand(Action_Add, nodes));
        }

        AddRemoveNodesCommand::Ptr AddRemoveNodesCommand::remove(const Model::ParentChildrenMap& nodes) {
            return Ptr(new AddRemoveNodesCommand(Action_Remove, nodes));
        }

        AddRemoveNodesCommand::~AddRemoveNodesCommand() {
            MapUtils::clearAndDelete(m_nodesToAdd);
        }

        AddRemoveNodesCommand::AddRemoveNodesCommand(const Action action, const Model::ParentChildrenMap& nodes) :
        DocumentCommand(Type, makeName(action)),
        m_action(action) {
            switch (m_action) {
                case Action_Add:
                    m_nodesToAdd = nodes;
                    break;
                case Action_Remove:
                    m_nodesToRemove = nodes;
                    break;
                switchDefault()
            }
        }

        String AddRemoveNodesCommand::makeName(const Action action) {
            switch (action) {
                case Action_Add:
                    return "Add Objects";
                case Action_Remove:
                    return "Remove Objects";
				switchDefault()
            }
        }

        bool AddRemoveNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performAddNodes(m_nodesToAdd);
                    break;
                case Action_Remove:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);

            return true;
        }

        bool AddRemoveNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Add:
                    document->performRemoveNodes(m_nodesToRemove);
                    break;
                case Action_Remove:
                    document->performAddNodes(m_nodesToAdd);
                    break;
            }

            using std::swap;
            std::swap(m_nodesToAdd, m_nodesToRemove);

            return true;
        }

        bool AddRemoveNodesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool AddRemoveNodesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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


#include "Model/BrushFace.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeBrushFaceAttributesCommand::Type = Command::freeType();

        ChangeBrushFaceAttributesCommand::Ptr ChangeBrushFaceAttributesCommand::command(const Model::ChangeBrushFaceAttributesRequest& request) {
            return Ptr(new ChangeBrushFaceAttributesCommand(request));
        }

        ChangeBrushFaceAttributesCommand::ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request) :
        DocumentCommand(Type, request.name()),
        m_request(request),
        m_snapshot(nullptr) {}

        ChangeBrushFaceAttributesCommand::~ChangeBrushFaceAttributesCommand() {
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        bool ChangeBrushFaceAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const Model::BrushFaceList faces = document->allSelectedBrushFaces();
            assert(!faces.empty());

            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(faces), std::end(faces));

            document->performChangeBrushFaceAttributes(m_request);
            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreSnapshot(m_snapshot);
            delete m_snapshot;
            m_snapshot = nullptr;
            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        UndoableCommand::Ptr ChangeBrushFaceAttributesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new ChangeBrushFaceAttributesCommand(m_request));
        }

        bool ChangeBrushFaceAttributesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ChangeBrushFaceAttributesCommand* other = static_cast<ChangeBrushFaceAttributesCommand*>(command.get());
            return m_request.collateWith(other->m_request);
        }
    }
}
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


#include "Macros.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeEntityAttributesCommand::Type = Command::freeType();

        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::set(const Model::AttributeName& name, const Model::AttributeValue& value) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Set));
            command->setName(name);
            command->setNewValue(value);
            return command;
        }

        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::remove(const Model::AttributeName& name) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Remove));
            command->setName(name);
            return command;
        }

        ChangeEntityAttributesCommand::Ptr ChangeEntityAttributesCommand::rename(const Model::AttributeName& oldName, const Model::AttributeName& newName) {
            Ptr command(new ChangeEntityAttributesCommand(Action_Rename));
            command->setName(oldName);
            command->setNewName(newName);
            return command;
        }

        void ChangeEntityAttributesCommand::setName(const Model::AttributeName& name) {
            m_oldName = name;
        }

        void ChangeEntityAttributesCommand::setNewName(const Model::AttributeName& newName) {
            assert(m_action == Action_Rename);
            m_newName = newName;
        }

        void ChangeEntityAttributesCommand::setNewValue(const Model::AttributeValue& newValue) {
            assert(m_action == Action_Set);
            m_newValue = newValue;
        }

        ChangeEntityAttributesCommand::ChangeEntityAttributesCommand(const Action action) :
        DocumentCommand(Type, makeName(action)),
        m_action(action) {}

        String ChangeEntityAttributesCommand::makeName(const Action action) {
            switch (action) {
                case Action_Set:
                    return "Set Property";
                case Action_Remove:
                    return "Remove Property";
                case Action_Rename:
                    return "Rename Property";
				switchDefault()
            }
        }

        bool ChangeEntityAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Set:
                    m_snapshots = document->performSetAttribute(m_oldName, m_newValue);
                    break;
                case Action_Remove:
                    m_snapshots = document->performRemoveAttribute(m_oldName);
                    break;
                case Action_Rename:
                    m_snapshots = document->performRenameAttribute(m_oldName, m_newName);
                    break;
            };
            return true;
        }

        bool ChangeEntityAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            m_snapshots.clear();
            return true;
        }

        bool ChangeEntityAttributesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ChangeEntityAttributesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ChangeEntityAttributesCommand* other = static_cast<ChangeEntityAttributesCommand*>(command.get());
            if (other->m_action != m_action)
                return false;
            if (other->m_oldName != m_oldName)
                return false;
            m_newName = other->m_newName;
            m_newValue = other->m_newValue;
            return true;
        }
    }
}
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


#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ConvertEntityColorCommand::Type = Command::freeType();

        ConvertEntityColorCommand::Ptr ConvertEntityColorCommand::convert(const Model::AttributeName& attributeName, const Assets::ColorRange::Type colorRange) {
            return Ptr(new ConvertEntityColorCommand(attributeName, colorRange));
        }

        ConvertEntityColorCommand::ConvertEntityColorCommand(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange) :
        DocumentCommand(Type, "Convert Color"),
        m_attributeName(attributeName),
        m_colorRange(colorRange) {}

        bool ConvertEntityColorCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_snapshots = document->performConvertColorRange(m_attributeName, m_colorRange);
            return true;
        }

        bool ConvertEntityColorCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            return true;
        }

        bool ConvertEntityColorCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ConvertEntityColorCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "TrenchBroom.h"
#include "Model/BrushFace.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/plane.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CopyTexCoordSystemFromFaceCommand::Type = Command::freeType();

        CopyTexCoordSystemFromFaceCommand::Ptr CopyTexCoordSystemFromFaceCommand::command(const Model::TexCoordSystemSnapshot& coordSystemSanpshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle) {
            return Ptr(new CopyTexCoordSystemFromFaceCommand(coordSystemSanpshot, attribs, sourceFacePlane, wrapStyle));
        }

        CopyTexCoordSystemFromFaceCommand::CopyTexCoordSystemFromFaceCommand(const Model::TexCoordSystemSnapshot& coordSystemSnapshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle) :
        DocumentCommand(Type, "Copy Texture Alignment"),
        m_snapshot(nullptr),
        m_coordSystemSanpshot(coordSystemSnapshot.clone()),
        m_sourceFacePlane(sourceFacePlane),
        m_wrapStyle(wrapStyle),
        m_attribs(attribs) {}

        CopyTexCoordSystemFromFaceCommand::~CopyTexCoordSystemFromFaceCommand() {
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        bool CopyTexCoordSystemFromFaceCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const Model::BrushFaceList faces = document->allSelectedBrushFaces();
            assert(!faces.empty());

            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(faces), std::end(faces));

            document->performCopyTexCoordSystemFromFace(*m_coordSystemSanpshot, m_attribs, m_sourceFacePlane, m_wrapStyle);
            return true;
        }

        bool CopyTexCoordSystemFromFaceCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreSnapshot(m_snapshot);
            delete m_snapshot;
            m_snapshot = nullptr;
            return true;
        }

        bool CopyTexCoordSystemFromFaceCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        UndoableCommand::Ptr CopyTexCoordSystemFromFaceCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new CopyTexCoordSystemFromFaceCommand(*m_coordSystemSanpshot, m_attribs, m_sourceFacePlane, m_wrapStyle));
        }

        bool CopyTexCoordSystemFromFaceCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CurrentGroupCommand::Type = Command::freeType();

        CurrentGroupCommand::Ptr CurrentGroupCommand::push(Model::Group* group) {
            return Ptr(new CurrentGroupCommand(group));
        }

        CurrentGroupCommand::Ptr CurrentGroupCommand::pop() {
            return Ptr(new CurrentGroupCommand(nullptr));
        }

        CurrentGroupCommand::CurrentGroupCommand(Model::Group* group) :
        UndoableCommand(Type, group != nullptr ? "Push Group" : "Pop Group"),
        m_group(group) {}

        bool CurrentGroupCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_group != nullptr) {
                document->performPushGroup(m_group);
                m_group = nullptr;
            } else {
                m_group = document->currentGroup();
                document->performPopGroup();
            }
            return true;
        }

        bool CurrentGroupCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            if (m_group == nullptr) {
                m_group = document->currentGroup();
                document->performPopGroup();
            } else {
                document->performPushGroup(m_group);
                m_group = nullptr;
            }
            return true;
        }

        bool CurrentGroupCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }

        bool CurrentGroupCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        DocumentCommand::DocumentCommand(const CommandType type, const String& name) :
        UndoableCommand(type, name),
        m_modificationCount(1) {}

        DocumentCommand::~DocumentCommand() {}

        bool DocumentCommand::performDo(MapDocumentCommandFacade* document) {
            if (UndoableCommand::performDo(document)) {
                document->incModificationCount(m_modificationCount);
                return true;
            }
            return false;
        }

        bool DocumentCommand::performUndo(MapDocumentCommandFacade* document) {
            if (UndoableCommand::performUndo(document)) {
                document->decModificationCount(m_modificationCount);
                return true;
            }
            return false;
        }

        bool DocumentCommand::collateWith(UndoableCommand::Ptr command) {
            if (UndoableCommand::collateWith(command)) {
                m_modificationCount += command->documentModificationCount();
                return true;
            }
            return false;
        }

        size_t DocumentCommand::documentModificationCount() const {
            return m_modificationCount;
        }
    }
}
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

#include "CollectionUtils.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType DuplicateNodesCommand::Type = Command::freeType();

        DuplicateNodesCommand::Ptr DuplicateNodesCommand::duplicate() {
            return Ptr(new DuplicateNodesCommand());
        }

        DuplicateNodesCommand::DuplicateNodesCommand() :
        DocumentCommand(Type, "Duplicate Objects"),
        m_firstExecution(true) {}

        DuplicateNodesCommand::~DuplicateNodesCommand() {
            if (state() == CommandState_Default)
                MapUtils::clearAndDelete(m_addedNodes);
        }

        bool DuplicateNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_firstExecution) {
                using NodeMapInsertPos = std::pair<bool, Model::NodeMap::iterator>;

                Model::NodeMap newParentMap;

                const vm::bbox3& worldBounds = document->worldBounds();
                m_previouslySelectedNodes = document->selectedNodes().nodes();

                for (const Model::Node* original : m_previouslySelectedNodes) {
                    Model::Node* clone = original->cloneRecursively(worldBounds);

                    Model::Node* parent = original->parent();
                    if (cloneParent(parent)) {
                        NodeMapInsertPos insertPos = MapUtils::findInsertPos(newParentMap, parent);
                        Model::Node* newParent = nullptr;
                        if (insertPos.first) {
                            assert(insertPos.second != std::begin(newParentMap));
                            newParent = std::prev(insertPos.second)->second;
                        } else {
                            newParent = parent->clone(worldBounds);
                            newParentMap.insert(insertPos.second, std::make_pair(parent, newParent));
                            m_addedNodes[document->currentParent()].push_back(newParent);
                        }

                        newParent->addChild(clone);
                    } else {
                        m_addedNodes[document->currentParent()].push_back(clone);
                    }

                    m_nodesToSelect.push_back(clone);
                }

                m_firstExecution = false;
            }

            document->performAddNodes(m_addedNodes);
            document->performDeselectAll();
            document->performSelect(m_nodesToSelect);
            return true;
        }

        bool DuplicateNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            document->performRemoveNodes(m_addedNodes);
            document->performSelect(m_previouslySelectedNodes);
            return true;
        }

        class DuplicateNodesCommand::CloneParentQuery : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
        private:
            void doVisit(const Model::World*) override  { setResult(false); }
            void doVisit(const Model::Layer*) override  { setResult(false); }
            void doVisit(const Model::Group*) override  { setResult(false);  }
            void doVisit(const Model::Entity*) override { setResult(true);  }
            void doVisit(const Model::Brush*) override  { setResult(false); }
        };

        bool DuplicateNodesCommand::cloneParent(const Model::Node* node) const {
            CloneParentQuery query;
            node->accept(query);
            return query.result();
        }

        bool DuplicateNodesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }

        UndoableCommand::Ptr DuplicateNodesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new DuplicateNodesCommand());
        }

        bool DuplicateNodesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType EntityDefinitionFileCommand::Type = Command::freeType();

        EntityDefinitionFileCommand::Ptr EntityDefinitionFileCommand::set(const Assets::EntityDefinitionFileSpec& spec) {
            return Ptr(new EntityDefinitionFileCommand("Set Entity Definitions", spec));
        }

        EntityDefinitionFileCommand::EntityDefinitionFileCommand(const String& name, const Assets::EntityDefinitionFileSpec& spec) :
        DocumentCommand(Type, name),
        m_newSpec(spec) {}

        bool EntityDefinitionFileCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldSpec = document->entityDefinitionFile();
            document->performSetEntityDefinitionFile(m_newSpec);
            return true;
        }

        bool EntityDefinitionFileCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetEntityDefinitionFile(m_oldSpec);
            return true;
        }

        bool EntityDefinitionFileCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool EntityDefinitionFileCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType FindPlanePointsCommand::Type = Command::freeType();

        FindPlanePointsCommand::Ptr FindPlanePointsCommand::findPlanePoints() {
            return Ptr(new FindPlanePointsCommand());
        }

        FindPlanePointsCommand::FindPlanePointsCommand() :
        SnapshotCommand(Type, "Find Plane Points") {}

        bool FindPlanePointsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performFindPlanePoints();
            return true;
        }

        bool FindPlanePointsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool FindPlanePointsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "Constants.h"
#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushEdgesCommand::Type = Command::freeType();

        MoveBrushEdgesCommand::Ptr MoveBrushEdgesCommand::move(const Model::EdgeToBrushesMap& edges, const vm::vec3& delta) {
            Model::BrushList brushes;
            Model::BrushEdgesMap brushEdges;
            std::vector<vm::segment3> edgePositions;
            extractEdgeMap(edges, brushes, brushEdges, edgePositions);

            return Ptr(new MoveBrushEdgesCommand(brushes, brushEdges, edgePositions, delta));
        }

        MoveBrushEdgesCommand::MoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Edges", brushes),
        m_edges(edges),
        m_oldEdgePositions(edgePositions),
        m_delta(delta) {
            assert(!vm::is_zero(m_delta, vm::C::almost_zero()));
        }

        bool MoveBrushEdgesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_edges) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::segment3>& edges = entry.second;
                if (!brush->canMoveEdges(worldBounds, edges, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushEdgesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newEdgePositions = document->performMoveEdges(m_edges, m_delta);
            return true;
        }

        bool MoveBrushEdgesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushEdgesCommand* other = static_cast<MoveBrushEdgesCommand*>(command.get());

            if (!canCollateWith(*other)) {
                return false;
            }

            if (!VectorUtils::equals(m_newEdgePositions, other->m_oldEdgePositions)) {
                return false;
            }

            m_newEdgePositions = other->m_newEdgePositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }

        void MoveBrushEdgesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_newEdgePositions), std::end(m_newEdgePositions));
        }

        void MoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }
    }
}
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


#include "CollectionUtils.h"
#include "Constants.h"
#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushFacesCommand::Type = Command::freeType();

        MoveBrushFacesCommand::Ptr MoveBrushFacesCommand::move(const Model::FaceToBrushesMap& faces, const vm::vec3& delta) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            std::vector<vm::polygon3> facePositions;
            extractFaceMap(faces, brushes, brushFaces, facePositions);

            return Ptr(new MoveBrushFacesCommand(brushes, brushFaces, facePositions, delta));
        }

        MoveBrushFacesCommand::MoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Faces", brushes),
        m_faces(faces),
        m_oldFacePositions(facePositions),
        m_delta(delta) {
            assert(!vm::is_zero(m_delta, vm::C::almost_zero()));
        }

        bool MoveBrushFacesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_faces) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::polygon3>& faces = entry.second;
                if (!brush->canMoveFaces(worldBounds, faces, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushFacesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newFacePositions = document->performMoveFaces(m_faces, m_delta);
            return true;
        }

        bool MoveBrushFacesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushFacesCommand* other = static_cast<MoveBrushFacesCommand*>(command.get());

            if (!canCollateWith(*other)) {
                return false;
            }

            if (!VectorUtils::equals(m_newFacePositions, other->m_oldFacePositions)) {
                return false;
            }

            m_newFacePositions = other->m_newFacePositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }


        void MoveBrushFacesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_newFacePositions), std::end(m_newFacePositions));
        }

        void MoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
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

#include "CollectionUtils.h"
#include "Constants.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveBrushVerticesCommand::Type = Command::freeType();

        MoveBrushVerticesCommand::Ptr MoveBrushVerticesCommand::move(const Model::VertexToBrushesMap& vertices, const vm::vec3& delta) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);

            return Ptr(new MoveBrushVerticesCommand(brushes, brushVertices, vertexPositions, delta));
        }

        bool MoveBrushVerticesCommand::hasRemainingVertices() const {
            return !m_newVertexPositions.empty();
        }

        MoveBrushVerticesCommand::MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta) :
        VertexCommand(Type, "Move Brush Vertices", brushes),
        m_vertices(vertices),
        m_oldVertexPositions(vertexPositions),
        m_delta(delta) {
            assert(!vm::is_zero(m_delta, vm::C::almost_zero()));
        }

        bool MoveBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::vec3>& vertices = entry.second;
                if (!brush->canMoveVertices(worldBounds, vertices, m_delta))
                    return false;
            }
            return true;
        }

        bool MoveBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performMoveVertices(m_vertices, m_delta);
            return true;
        }

        bool MoveBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            MoveBrushVerticesCommand* other = static_cast<MoveBrushVerticesCommand*>(command.get());

            if (!canCollateWith(*other)) {
                return false;
            }

            if (!VectorUtils::equals(m_newVertexPositions, other->m_oldVertexPositions)) {
                return false;
            }

            m_newVertexPositions = other->m_newVertexPositions;
            m_delta = m_delta + other->m_delta;

            return true;
        }

        void MoveBrushVerticesCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_newVertexPositions), std::end(m_newVertexPositions));
        }

        void MoveBrushVerticesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            manager.select(std::begin(m_oldVertexPositions), std::end(m_oldVertexPositions));
        }
    }
}
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

#include "Model/BrushFace.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType MoveTexturesCommand::Type = Command::freeType();

        MoveTexturesCommand::Ptr MoveTexturesCommand::move(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) {
            return Ptr(new MoveTexturesCommand(cameraUp, cameraRight, delta));
        }

        MoveTexturesCommand::MoveTexturesCommand(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta) :
        DocumentCommand(Type, "Move Textures"),
        m_cameraUp(cameraUp),
        m_cameraRight(cameraRight),
        m_delta(delta) {}

        bool MoveTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            moveTextures(document, m_delta);
            return true;
        }

        bool MoveTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            moveTextures(document, -m_delta);
            return true;
        }

        void MoveTexturesCommand::moveTextures(MapDocumentCommandFacade* document, const vm::vec2f& delta) const {
            document->performMoveTextures(m_cameraUp, m_cameraRight, delta);
        }

        bool MoveTexturesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return true;
        }

        UndoableCommand::Ptr MoveTexturesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new MoveTexturesCommand(m_cameraUp, m_cameraRight, m_delta));
        }

        bool MoveTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            const MoveTexturesCommand* other = static_cast<MoveTexturesCommand*>(command.get());

            if (other->m_cameraUp != m_cameraUp ||
                other->m_cameraRight != m_cameraRight)
                return false;

            m_delta = m_delta + other->m_delta;
            return true;
        }
    }
}
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


#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushEdgesCommand::Type = Command::freeType();

        RemoveBrushEdgesCommand::Ptr RemoveBrushEdgesCommand::remove(const Model::EdgeToBrushesMap& edges) {
            Model::BrushList brushes;
            Model::BrushEdgesMap brushEdges;
            std::vector<vm::segment3> edgePositions;

            extractEdgeMap(edges, brushes, brushEdges, edgePositions);
            Model::BrushVerticesMap brushVertices = brushVertexMap(brushEdges);

            return Ptr(new RemoveBrushEdgesCommand(brushes, brushVertices, edgePositions));
        }

        RemoveBrushEdgesCommand::RemoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::segment3>& edgePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Edges", brushes, vertices),
        m_oldEdgePositions(edgePositions) {}

        void RemoveBrushEdgesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            manager.select(std::begin(m_oldEdgePositions), std::end(m_oldEdgePositions));
        }
    }
}
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

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        RemoveBrushElementsCommand::RemoveBrushElementsCommand(const CommandType type, const String& name, const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices) :
        VertexCommand(type, name, brushes),
        m_vertices(vertices) {}

        bool RemoveBrushElementsCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::vec3>& vertices = entry.second;
                if (!brush->canRemoveVertices(worldBounds, vertices))
                    return false;
            }
            return true;
        }

        bool RemoveBrushElementsCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            document->performRemoveVertices(m_vertices);
            return true;
        }

        bool RemoveBrushElementsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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


#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushFacesCommand::Type = Command::freeType();

        RemoveBrushFacesCommand::Ptr RemoveBrushFacesCommand::remove(const Model::FaceToBrushesMap& faces) {
            Model::BrushList brushes;
            Model::BrushFacesMap brushFaces;
            std::vector<vm::polygon3> facePositions;

            extractFaceMap(faces, brushes, brushFaces, facePositions);
            const Model::BrushVerticesMap brushVertices = brushVertexMap(brushFaces);

            return Ptr(new RemoveBrushFacesCommand(brushes, brushVertices, facePositions));
        }

        RemoveBrushFacesCommand::RemoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::polygon3>& facePositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Faces", brushes, vertices),
        m_oldFacePositions(facePositions) {}

        void RemoveBrushFacesCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            manager.select(std::begin(m_oldFacePositions), std::end(m_oldFacePositions));
        }
    }
}
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

#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RemoveBrushVerticesCommand::Type = Command::freeType();

        RemoveBrushVerticesCommand::Ptr RemoveBrushVerticesCommand::remove(const Model::VertexToBrushesMap& vertices) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            std::vector<vm::vec3> vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);

            return Ptr(new RemoveBrushVerticesCommand(brushes, brushVertices, vertexPositions));
        }

        RemoveBrushVerticesCommand::RemoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions) :
        RemoveBrushElementsCommand(Type, "Remove Brush Vertices", brushes, vertices),
        m_oldVertexPositions(vertexPositions) {}
    }
}
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


#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RenameGroupsCommand::Type = Command::freeType();

        RenameGroupsCommand::Ptr RenameGroupsCommand::rename(const String& newName) {
            return Ptr(new RenameGroupsCommand(newName));
        }

        RenameGroupsCommand::RenameGroupsCommand(const String& newName) :
        DocumentCommand(Type, "Rename Groups"),
        m_newName(newName) {}

        bool RenameGroupsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldNames = document->performRenameGroups(m_newName);
            return true;
        }

        bool RenameGroupsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performUndoRenameGroups(m_oldNames);
            return true;
        }

        bool RenameGroupsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool RenameGroupsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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


#include "CollectionUtils.h"
#include "Model/ModelUtils.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ReparentNodesCommand::Type = Command::freeType();

        ReparentNodesCommand::Ptr ReparentNodesCommand::reparent(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove) {
            return Ptr(new ReparentNodesCommand(nodesToAdd, nodesToRemove));
        }

        ReparentNodesCommand::ReparentNodesCommand(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove) :
        DocumentCommand(Type, "Reparent Objects"),
        m_nodesToAdd(nodesToAdd),
        m_nodesToRemove(nodesToRemove) {}

        bool ReparentNodesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToRemove);
            document->performAddNodes(m_nodesToAdd);
            return true;
        }

        bool ReparentNodesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performRemoveNodes(m_nodesToAdd);
            document->performAddNodes(m_nodesToRemove);
            return true;
        }

        bool ReparentNodesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ReparentNodesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

#include "TrenchBroom.h"
#include <vecmath/vec.h>
#include <vecmath/polygon.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ResizeBrushesCommand::Type = Command::freeType();

        ResizeBrushesCommand::Ptr ResizeBrushesCommand::resize(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) {
            return Ptr(new ResizeBrushesCommand(faces, delta));
        }

        ResizeBrushesCommand::ResizeBrushesCommand(const std::vector<vm::polygon3>& faces, const vm::vec3& delta) :
        SnapshotCommand(Type, "Resize Brushes"),
        m_faces(faces),
        m_delta(delta) {}

        bool ResizeBrushesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_newFaces = document->performResizeBrushes(m_faces, m_delta);
            return !m_newFaces.empty();
        }


        bool ResizeBrushesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ResizeBrushesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ResizeBrushesCommand* other = static_cast<ResizeBrushesCommand*>(command.get());
            if (other->m_faces == m_newFaces) {
                m_newFaces = other->m_newFaces;
                m_delta = m_delta + other->m_delta;
                return true;
            } else {
                return false;
            }
        }
    }
}
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

#include "Model/BrushFace.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType RotateTexturesCommand::Type = Command::freeType();

        RotateTexturesCommand::Ptr RotateTexturesCommand::rotate(const float angle) {
            return Ptr(new RotateTexturesCommand(angle));
        }

        RotateTexturesCommand::RotateTexturesCommand(const float angle) :
        DocumentCommand(Type, "Move Textures"),
        m_angle(angle) {}

        bool RotateTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return rotateTextures(document, m_angle);
        }

        bool RotateTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            return rotateTextures(document, -m_angle);
        }

        bool RotateTexturesCommand::rotateTextures(MapDocumentCommandFacade* document, const float angle) const {
            document->performRotateTextures(angle);
            return true;
        }

        bool RotateTexturesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return true;
        }

        UndoableCommand::Ptr RotateTexturesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new RotateTexturesCommand(m_angle));
        }

        bool RotateTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            const RotateTexturesCommand* other = static_cast<RotateTexturesCommand*>(command.get());

            m_angle += other->m_angle;
            return true;
        }
    }
}
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


#include "Macros.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/World.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand::Ptr SelectionCommand::select(const Model::NodeList& nodes) {
            return Ptr(new SelectionCommand(Action_SelectNodes, nodes, Model::EmptyBrushFaceList));
        }

        SelectionCommand::Ptr SelectionCommand::select(const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(Action_SelectFaces, Model::EmptyNodeList, faces));
        }

        SelectionCommand::Ptr SelectionCommand::convertToFaces() {
            return Ptr(new SelectionCommand(Action_ConvertToFaces, Model::EmptyNodeList, Model::EmptyBrushFaceList));
        }

        SelectionCommand::Ptr SelectionCommand::selectAllNodes() {
            return Ptr(new SelectionCommand(Action_SelectAllNodes, Model::EmptyNodeList, Model::EmptyBrushFaceList));
        }

        SelectionCommand::Ptr SelectionCommand::selectAllFaces() {
            return Ptr(new SelectionCommand(Action_SelectAllFaces, Model::EmptyNodeList, Model::EmptyBrushFaceList));
        }

        SelectionCommand::Ptr SelectionCommand::deselect(const Model::NodeList& nodes) {
            return Ptr(new SelectionCommand(Action_DeselectNodes, nodes, Model::EmptyBrushFaceList));
        }

        SelectionCommand::Ptr SelectionCommand::deselect(const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(Action_DeselectFaces, Model::EmptyNodeList, faces));
        }

        SelectionCommand::Ptr SelectionCommand::deselectAll() {
            return Ptr(new SelectionCommand(Action_DeselectAll, Model::EmptyNodeList, Model::EmptyBrushFaceList));
        }

        static Model::BrushFaceReference::List faceRefs(const Model::BrushFaceList& faces) {
            Model::BrushFaceReference::List result;
            for (Model::BrushFace* face : faces)
                result.push_back(Model::BrushFaceReference(face));
            return result;
        }

        static Model::BrushFaceList resolveFaceRefs(const Model::BrushFaceReference::List& refs) {
            Model::BrushFaceList result;
            for (const Model::BrushFaceReference& ref : refs)
                result.push_back(ref.resolve());
            return result;
        }

        SelectionCommand::SelectionCommand(const Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces) :
        UndoableCommand(Type, makeName(action, nodes, faces)),
        m_action(action),
        m_nodes(nodes),
        m_faceRefs(faceRefs(faces)) {}

        String SelectionCommand::makeName(const Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces) {
            StringStream result;
            switch (action) {
                case Action_SelectNodes:
                    result << "Select " << nodes.size() << " " << StringUtils::safePlural(nodes.size(), "Object", "Objects");
                    break;
                case Action_SelectFaces:
                    result << "Select " << faces.size() << " " << StringUtils::safePlural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_SelectAllNodes:
                    result << "Select All Objects";
                    break;
                case Action_SelectAllFaces:
                    result << "Select All Brush Faces";
                    break;
                case Action_ConvertToFaces:
                    result << "Convert to Brush Face Selection";
                    break;
                case Action_DeselectNodes:
                    result << "Deselect " << nodes.size() << " " << StringUtils::safePlural(nodes.size(), "Object", "Objects");
                    break;
                case Action_DeselectFaces:
                    result << "Deselect " << faces.size() << " " << StringUtils::safePlural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_DeselectAll:
                    return "Select None";
                switchDefault()
            }
            return result.str();
        }

        bool SelectionCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_previouslySelectedNodes = document->selectedNodes().nodes();
            m_previouslySelectedFaceRefs = faceRefs(document->selectedBrushFaces());

            switch (m_action) {
                case Action_SelectNodes:
                    document->performSelect(m_nodes);
                    break;
                case Action_SelectFaces:
                    document->performSelect(resolveFaceRefs(m_faceRefs));
                    break;
                case Action_SelectAllNodes:
                    document->performSelectAllNodes();
                    break;
                case Action_SelectAllFaces:
                    document->performSelectAllBrushFaces();
                    break;
                case Action_ConvertToFaces:
                    document->performConvertToBrushFaceSelection();
                    break;
                case Action_DeselectNodes:
                    document->performDeselect(m_nodes);
                    break;
                case Action_DeselectFaces:
                    document->performDeselect(resolveFaceRefs(m_faceRefs));
                    break;
                case Action_DeselectAll:
                    document->performDeselectAll();
                    break;
            }
            return true;
        }

        bool SelectionCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            if (!m_previouslySelectedNodes.empty())
                document->performSelect(m_previouslySelectedNodes);
            if (!m_previouslySelectedFaceRefs.empty())
                document->performSelect(resolveFaceRefs(m_previouslySelectedFaceRefs));
            return true;
        }

        bool SelectionCommand::doIsRepeatDelimiter() const {
            return true;
        }

        bool SelectionCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SelectionCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "Macros.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetLockStateCommand::Type = Command::freeType();

        SetLockStateCommand::Ptr SetLockStateCommand::lock(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Locked));
        }

        SetLockStateCommand::Ptr SetLockStateCommand::unlock(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Unlocked));
        }

        SetLockStateCommand::Ptr SetLockStateCommand::reset(const Model::NodeList& nodes) {
            return Ptr(new SetLockStateCommand(nodes, Model::Lock_Inherited));
        }

        SetLockStateCommand::SetLockStateCommand(const Model::NodeList& nodes, const Model::LockState lockState) :
        UndoableCommand(Type, makeName(lockState)),
        m_nodes(nodes),
        m_lockState(lockState) {}

        String SetLockStateCommand::makeName(const Model::LockState state) {
            switch (state) {
                case Model::Lock_Inherited:
                    return "Reset Locking";
                case Model::Lock_Locked:
                    return "Lock Objects";
                case Model::Lock_Unlocked:
                    return "Unlock Objects";
		switchDefault()
            }
        }

        bool SetLockStateCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldLockState = document->setLockState(m_nodes, m_lockState);
            return true;
        }

        bool SetLockStateCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreLockState(m_oldLockState);
            return true;
        }

        bool SetLockStateCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }

        bool SetLockStateCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }
    }
}
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


#include "StringList.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetModsCommand::Type = Command::freeType();

        SetModsCommand::Ptr SetModsCommand::set(const StringList& mods) {
            return Ptr(new SetModsCommand("Set Mods", mods));
        }

        SetModsCommand::SetModsCommand(const String& name, const StringList& mods) :
        DocumentCommand(Type, name),
        m_newMods(mods) {}

        bool SetModsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldMods = document->mods();
            document->performSetMods(m_newMods);
            return true;
        }

        bool SetModsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetMods(m_oldMods);
            return true;
        }

        bool SetModsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SetModsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetTextureCollectionsCommand::Type = Command::freeType();

        SetTextureCollectionsCommand::Ptr SetTextureCollectionsCommand::set(const IO::Path::List& paths) {
            return Ptr(new SetTextureCollectionsCommand(paths));
        }

        SetTextureCollectionsCommand::SetTextureCollectionsCommand(const IO::Path::List& paths) :
        DocumentCommand(Type, "Set Texture Collections"),
        m_paths(paths) {}

        bool SetTextureCollectionsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldPaths = document->enabledTextureCollections();
            document->performSetTextureCollections(m_paths);
            return true;
        }

        bool SetTextureCollectionsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetTextureCollections(m_oldPaths);
            m_oldPaths.clear();
            return true;
        }

        bool SetTextureCollectionsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SetTextureCollectionsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "Macros.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetVisibilityCommand::Type = Command::freeType();

        SetVisibilityCommand::Ptr SetVisibilityCommand::show(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Show));
        }

        SetVisibilityCommand::Ptr SetVisibilityCommand::hide(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Hide));
        }

        SetVisibilityCommand::Ptr SetVisibilityCommand::ensureVisible(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Ensure));
        }

        SetVisibilityCommand::Ptr SetVisibilityCommand::reset(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Reset));
        }

        SetVisibilityCommand::SetVisibilityCommand(const Model::NodeList& nodes, const Action action) :
        UndoableCommand(Type, makeName(action)),
        m_nodes(nodes),
        m_action(action) {}

        String SetVisibilityCommand::makeName(const Action action) {
            switch (action) {
                case Action_Reset:
                    return "Reset Visibility";
                case Action_Hide:
                    return "Hide Objects";
                case Action_Show:
                    return "Show Objects";
                case Action_Ensure:
                    return "Ensure Objects Visible";
                switchDefault()
            }
        }

        bool SetVisibilityCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Reset:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Inherited);
                    break;
                case Action_Hide:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Hidden);
                    break;
                case Action_Show:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Shown);
                    break;
                case Action_Ensure:
                    m_oldState = document->setVisibilityEnsured(m_nodes);
                    break;
                switchDefault()
            }
            return true;
        }

        bool SetVisibilityCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreVisibilityState(m_oldState);
            return true;
        }

        bool SetVisibilityCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }

        bool SetVisibilityCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }
    }
}
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

#include "View/MapDocumentCommandFacade.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ShearTexturesCommand::Type = Command::freeType();

        ShearTexturesCommand::Ptr ShearTexturesCommand::shear(const vm::vec2f& factors) {
            return Ptr(new ShearTexturesCommand(factors));
        }

        ShearTexturesCommand::ShearTexturesCommand(const vm::vec2f& factors) :
        DocumentCommand(Type, "Shear Textures"),
        m_factors(factors) {
            assert(factors.x() != 0.0f || factors.y() != 0.0f);
        }

        bool ShearTexturesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return shearTextures(document, m_factors);
        }

        bool ShearTexturesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            return shearTextures(document, -m_factors);
        }

        bool ShearTexturesCommand::shearTextures(MapDocumentCommandFacade* document, const vm::vec2f& factors) {
            document->performShearTextures(factors);
            return true;
        }

        bool ShearTexturesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return true;
        }

        UndoableCommand::Ptr ShearTexturesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new ShearTexturesCommand(m_factors));
        }

        bool ShearTexturesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ShearTexturesCommand* other = static_cast<ShearTexturesCommand*>(command.get());
            m_factors = m_factors + other->m_factors;
            return true;
        }
    }
}
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

#include "Model/Brush.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snap(const FloatType snapTo) {
            return Ptr(new SnapBrushVerticesCommand(snapTo));
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(const FloatType snapTo) :
        SnapshotCommand(Type, "Snap Brush Vertices"),
        m_snapTo(snapTo) {}

        bool SnapBrushVerticesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return document->performSnapVertices(m_snapTo);
        }

        bool SnapBrushVerticesCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            SnapBrushVerticesCommand* other = static_cast<SnapBrushVerticesCommand*>(command.get());
            return other->m_snapTo == m_snapTo;
        }
    }
}
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

#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        SnapshotCommand::SnapshotCommand(Command::CommandType type, const String &name) :
        DocumentCommand(type, name),
        m_snapshot(nullptr) {}

        SnapshotCommand::~SnapshotCommand() {
            if (m_snapshot != nullptr) {
                deleteSnapshot();
            }
        }

        bool SnapshotCommand::performDo(MapDocumentCommandFacade *document) {
            takeSnapshot(document);
            if (DocumentCommand::performDo(document)) {
                return true;
            } else {
                deleteSnapshot();
                return false;
            }
        }

        bool SnapshotCommand::doPerformUndo(MapDocumentCommandFacade *document) {
            return restoreSnapshot(document);
        }

        void SnapshotCommand::takeSnapshot(MapDocumentCommandFacade *document) {
            assert(m_snapshot == nullptr);
            m_snapshot = doTakeSnapshot(document);
        }

        bool SnapshotCommand::restoreSnapshot(MapDocumentCommandFacade *document) {
            ensure(m_snapshot != nullptr, "snapshot is null");
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            return true;
        }

        void SnapshotCommand::deleteSnapshot() {
            assert(m_snapshot != nullptr);
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        Model::Snapshot *SnapshotCommand::doTakeSnapshot(MapDocumentCommandFacade *document) const {
            const auto& nodes = document->selectedNodes().nodes();
            return new Model::Snapshot(std::begin(nodes), std::end(nodes));
        }
    }
}
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

#include "TrenchBroom.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType TransformObjectsCommand::Type = Command::freeType();

        TransformObjectsCommand::Ptr TransformObjectsCommand::translate(const vm::vec3& delta, const bool lockTextures) {
            const auto transform = vm::translation_matrix(delta);
            return Ptr(new TransformObjectsCommand(Action_Translate, "Move Objects", transform, lockTextures));
        }

        TransformObjectsCommand::Ptr TransformObjectsCommand::rotate(const vm::vec3& center, const vm::vec3& axis, const FloatType angle, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::rotation_matrix(axis, angle) * vm::translation_matrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Rotate, "Rotate Objects", transform, lockTextures));
        }

        TransformObjectsCommand::Ptr TransformObjectsCommand::scale(const vm::bbox3& oldBBox, const vm::bbox3& newBBox, const bool lockTextures) {
            const auto transform = vm::scale_bbox_matrix(oldBBox, newBBox);
            return Ptr(new TransformObjectsCommand(Action_Scale, "Scale Objects", transform, lockTextures));
        }

        TransformObjectsCommand::Ptr TransformObjectsCommand::scale(const vm::vec3& center, const vm::vec3& scaleFactors, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::scaling_matrix(scaleFactors) * vm::translation_matrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Scale, "Scale Objects", transform, lockTextures));
        }

        TransformObjectsCommand::Ptr TransformObjectsCommand::shearBBox(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta, const bool lockTextures) {
            const auto transform = vm::shear_bbox_matrix(box, sideToShear, delta);
            return Ptr(new TransformObjectsCommand(Action_Shear, "Shear Objects", transform, lockTextures));
        }

        TransformObjectsCommand::Ptr TransformObjectsCommand::flip(const vm::vec3& center, const vm::axis::type axis, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::mirror_matrix<FloatType>(axis) * vm::translation_matrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Flip, "Flip Objects", transform, lockTextures));
        }

        TransformObjectsCommand::TransformObjectsCommand(const Action action, const String& name, const vm::mat4x4& transform, const bool lockTextures) :
        SnapshotCommand(Type, name),
        m_action(action),
        m_transform(transform),
        m_lockTextures(lockTextures) {}

        bool TransformObjectsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            return document->performTransform(m_transform, m_lockTextures);
        }

        bool TransformObjectsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }

        UndoableCommand::Ptr TransformObjectsCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new TransformObjectsCommand(m_action, m_name, m_transform, m_lockTextures));
        }

        bool TransformObjectsCommand::doCollateWith(UndoableCommand::Ptr command) {
            auto* other = static_cast<TransformObjectsCommand*>(command.get());
            if (other->m_lockTextures != m_lockTextures) {
                return false;
            } else if (other->m_action != m_action) {
                return false;
            } else {
                m_transform = m_transform * other->m_transform;
                return true;
            }
        }
    }
}
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

#include "Exceptions.h"

namespace TrenchBroom {
    namespace View {
        UndoableCommand::UndoableCommand(const CommandType type, const String& name) :
        Command(type, name) {}

        UndoableCommand::~UndoableCommand() {}

        bool UndoableCommand::performUndo(MapDocumentCommandFacade* document) {
            m_state = CommandState_Undoing;
            if (doPerformUndo(document)) {
                m_state = CommandState_Default;
                return true;
            } else {
                m_state = CommandState_Done;
                return false;
            }
        }

        bool UndoableCommand::isRepeatDelimiter() const {
            return doIsRepeatDelimiter();
        }

        bool UndoableCommand::isRepeatable(MapDocumentCommandFacade* document) const {
            return doIsRepeatable(document);
        }

        UndoableCommand::Ptr UndoableCommand::repeat(MapDocumentCommandFacade* document) const {
            return doRepeat(document);
        }

        bool UndoableCommand::collateWith(UndoableCommand::Ptr command) {
            assert(command.get() != this);
            if (command->type() != m_type)
                return false;
            return doCollateWith(command);
        }

        bool UndoableCommand::doIsRepeatDelimiter() const {
            return false;
        }

        UndoableCommand::Ptr UndoableCommand::doRepeat(MapDocumentCommandFacade*) const {
            throw CommandProcessorException("Command is not repeatable");
        }

        size_t UndoableCommand::documentModificationCount() const {
            throw CommandProcessorException("Command does not modify the document");
        }
    }
}
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

#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType UpdateEntitySpawnflagCommand::Type = Command::freeType();

        UpdateEntitySpawnflagCommand::Ptr UpdateEntitySpawnflagCommand::update(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag) {
            return Ptr(new UpdateEntitySpawnflagCommand(name, flagIndex, setFlag));
        }

        UpdateEntitySpawnflagCommand::UpdateEntitySpawnflagCommand(const Model::AttributeName& attributeName, const size_t flagIndex, const bool setFlag) :
        DocumentCommand(Type, makeName(setFlag)),
        m_setFlag(setFlag),
        m_attributeName(attributeName),
        m_flagIndex(flagIndex) {}

        String UpdateEntitySpawnflagCommand::makeName(const bool setFlag) {
            return setFlag ? "Set Spawnflag" : "Unset Spawnflag";
        }

        bool UpdateEntitySpawnflagCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performUpdateSpawnflag(m_attributeName, m_flagIndex, m_setFlag);
            return true;
        }

        bool UpdateEntitySpawnflagCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performUpdateSpawnflag(m_attributeName, m_flagIndex, !m_setFlag);
            return true;
        }

        bool UpdateEntitySpawnflagCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool UpdateEntitySpawnflagCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexTool.h"

#include <iterator>

namespace TrenchBroom {
    namespace View {
        VertexCommand::VertexCommand(const CommandType type, const String& name, const Model::BrushList& brushes) :
        DocumentCommand(type, name),
        m_brushes(brushes) {}

        VertexCommand::~VertexCommand() = default;

        void VertexCommand::extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, std::vector<vm::vec3>& vertexPositions) {
            extract(vertices, brushes, brushVertices, vertexPositions);
        }

        void VertexCommand::extractEdgeMap(const Model::EdgeToBrushesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions) {
            extract(edges, brushes, brushEdges, edgePositions);
        }

        void VertexCommand::extractFaceMap(const Model::FaceToBrushesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions) {
            extract(faces, brushes, brushFaces, facePositions);
        }

        void VertexCommand::extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions) {

            for (const auto& entry : edges) {
                const Model::BrushEdgeSet& mappedEdges = entry.second;
                for (Model::BrushEdge* edge : mappedEdges) {
                    Model::Brush* brush = edge->firstFace()->payload()->brush();
                    const vm::segment3 edgePosition(edge->firstVertex()->position(), edge->secondVertex()->position());

                    const auto result = brushEdges.insert(std::make_pair(brush, std::vector<vm::segment3>()));
                    if (result.second) {
                        brushes.push_back(brush);
                    }
                    result.first->second.push_back(edgePosition);
                    edgePositions.push_back(edgePosition);
                }
            }

            assert(!brushes.empty());
            assert(brushes.size() == brushEdges.size());
        }

        void VertexCommand::extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions) {

            for (const auto& entry : faces) {
                const Model::BrushFaceSet& mappedFaces = entry.second;
                for (Model::BrushFace* face : mappedFaces) {
                    Model::Brush* brush = face->brush();
                    const auto result = brushFaces.insert(std::make_pair(brush, std::vector<vm::polygon3>()));
                    if (result.second) {
                        brushes.push_back(brush);
                    }

                    const vm::polygon3 facePosition = face->polygon();
                    result.first->second.push_back(facePosition);
                    facePositions.push_back(facePosition);
                }
            }

            VectorUtils::sort(facePositions);

            assert(!brushes.empty());
            assert(brushes.size() == brushFaces.size());
        }

        Model::BrushVerticesMap VertexCommand::brushVertexMap(const Model::BrushEdgesMap& edges) {
            Model::BrushVerticesMap result;
            for (const auto& entry : edges) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::segment3>& edgeList = entry.second;

                std::vector<vm::vec3> vertices;
                vertices.reserve(2 * edgeList.size());
                vm::segment3::get_vertices(std::begin(edgeList), std::end(edgeList), std::back_inserter(vertices));
                VectorUtils::sortAndRemoveDuplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }

        Model::BrushVerticesMap VertexCommand::brushVertexMap(const Model::BrushFacesMap& faces) {
            Model::BrushVerticesMap result;
            for (const auto& entry : faces) {
                Model::Brush* brush = entry.first;
                const std::vector<vm::polygon3>& faceList = entry.second;

                std::vector<vm::vec3> vertices;
                vm::polygon3::get_vertices(std::begin(faceList), std::end(faceList), std::back_inserter(vertices));
                VectorUtils::sortAndRemoveDuplicates(vertices);
                result.insert(std::make_pair(brush, vertices));
            }
            return result;
        }

        bool VertexCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (m_snapshot != nullptr) {
                restoreAndTakeNewSnapshot(document);
                return true;
            } else {
                if (!doCanDoVertexOperation(document)) {
                    return false;
                }

                takeSnapshot();
                return doVertexOperation(document);
            }
        }

        bool VertexCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            restoreAndTakeNewSnapshot(document);
            return true;
        }

        void VertexCommand::restoreAndTakeNewSnapshot(MapDocumentCommandFacade* document) {
            ensure(m_snapshot != nullptr, "snapshot is null");

            auto snapshot = std::move(m_snapshot);
            takeSnapshot();
            document->restoreSnapshot(snapshot.get());
        }

        bool VertexCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        void VertexCommand::takeSnapshot() {
            assert(m_snapshot == nullptr);
            m_snapshot = std::make_unique<Model::Snapshot>(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::deleteSnapshot() {
            ensure(m_snapshot != nullptr, "snapshot is null");
            m_snapshot.reset();
        }

        bool VertexCommand::canCollateWith(const VertexCommand& other) const {
            return VectorUtils::equals(m_brushes, other.m_brushes);
        }

        void VertexCommand::removeHandles(VertexHandleManagerBase& manager) {
            manager.removeHandles(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::addHandles(VertexHandleManagerBase& manager) {
            manager.addHandles(std::begin(m_brushes), std::end(m_brushes));
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            doSelectNewHandlePositions(manager);
        }

        void VertexCommand::selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const {
            doSelectOldHandlePositions(manager);
        }

        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>&) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>&) const {}
        void VertexCommand::doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}
        void VertexCommand::doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>&) const {}
    }
}
