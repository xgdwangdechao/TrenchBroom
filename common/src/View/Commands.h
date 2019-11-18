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

#ifndef Commands_h
#define Commands_h

#include "Assets/ColorRange.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/Path.h"
#include "Model/BrushGeometry.h"
#include "Model/EntityAttributeSnapshot.h"
#include "Model/ModelTypes.h"
#include "Model/BrushFaceReference.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "TrenchBroom.h"
#include "StringList.h"
#include "StringType.h"
#include "View/Command.h"
#include "View/VertexHandleManager.h"

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>

#include <map>
#include <memory>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class MapDocumentCommandFacade;

        class UndoableCommand : public Command {
        public:
            using Ptr = std::shared_ptr<UndoableCommand>;
        public:
            UndoableCommand(CommandType type, const String& name);
            virtual ~UndoableCommand();

            virtual bool performUndo(MapDocumentCommandFacade* document);

            bool isRepeatDelimiter() const;
            bool isRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr repeat(MapDocumentCommandFacade* document) const;

            virtual bool collateWith(UndoableCommand::Ptr command);
        private:
            virtual bool doPerformUndo(MapDocumentCommandFacade* document) = 0;

            virtual bool doIsRepeatDelimiter() const;
            virtual bool doIsRepeatable(MapDocumentCommandFacade* document) const = 0;
            virtual UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;

            virtual bool doCollateWith(UndoableCommand::Ptr command) = 0;
        public: // this method is just a service for DocumentCommand and should never be called from anywhere else
            virtual size_t documentModificationCount() const;
        private:
            UndoableCommand(const UndoableCommand& other);
            UndoableCommand& operator=(const UndoableCommand& other);
        };

        class DocumentCommand : public UndoableCommand {
        private:
            size_t m_modificationCount;
        public:
            DocumentCommand(CommandType type, const String& name);
            virtual ~DocumentCommand() override;
        public:
            bool performDo(MapDocumentCommandFacade* document) override;
            bool performUndo(MapDocumentCommandFacade* document) override;
            bool collateWith(UndoableCommand::Ptr command) override;
        private:
            size_t documentModificationCount() const override;
        private:
            DocumentCommand(const DocumentCommand& other);
            DocumentCommand& operator=(const DocumentCommand& other);
        };

        class SnapshotCommand : public DocumentCommand {
        private:
            Model::Snapshot* m_snapshot;
        protected:
            SnapshotCommand(CommandType type, const String& name);
            virtual ~SnapshotCommand() override;
        public:
            bool performDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;
        private:
            void takeSnapshot(MapDocumentCommandFacade* document);
            bool restoreSnapshot(MapDocumentCommandFacade* document);
            void deleteSnapshot();
        private:
            virtual Model::Snapshot* doTakeSnapshot(MapDocumentCommandFacade* document) const;
        };

        class MapDocument;
        class VertexTool;

        class VertexCommand : public DocumentCommand {
        private:
            Model::BrushList m_brushes;
            std::unique_ptr<Model::Snapshot> m_snapshot;
        protected:
            VertexCommand(CommandType type, const String& name, const Model::BrushList& brushes);
        public:
            ~VertexCommand() override;
        protected:
            template <typename H, typename C>
            static void extract(const std::map<H, Model::BrushSet, C>& handleToBrushes, Model::BrushList& brushes, std::map<Model::Brush*, std::vector<H>>& brushToHandles, std::vector<H>& handles) {

                for (const auto& entry : handleToBrushes) {
                    const H& handle = entry.first;
                    const Model::BrushSet& mappedBrushes = entry.second;
                    for (Model::Brush* brush : mappedBrushes) {
                        const auto result = brushToHandles.insert(std::make_pair(brush, std::vector<H>()));
                        if (result.second)
                            brushes.push_back(brush);
                        result.first->second.push_back(handle);
                    }
                    handles.push_back(handle);
                }
            }

            static void extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, std::vector<vm::vec3>& vertexPositions);
            static void extractEdgeMap(const Model::EdgeToBrushesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const Model::FaceToBrushesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            // TODO 1720: Remove these methods if possible.
            static void extractEdgeMap(const Model::VertexToEdgesMap& edges, Model::BrushList& brushes, Model::BrushEdgesMap& brushEdges, std::vector<vm::segment3>& edgePositions);
            static void extractFaceMap(const Model::VertexToFacesMap& faces, Model::BrushList& brushes, Model::BrushFacesMap& brushFaces, std::vector<vm::polygon3>& facePositions);

            static Model::BrushVerticesMap brushVertexMap(const Model::BrushEdgesMap& edges);
            static Model::BrushVerticesMap brushVertexMap(const Model::BrushFacesMap& faces);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;
            void restoreAndTakeNewSnapshot(MapDocumentCommandFacade* document);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        private:
            void takeSnapshot();
            void deleteSnapshot();
        protected:
            bool canCollateWith(const VertexCommand& other) const;
        private:
            virtual bool doCanDoVertexOperation(const MapDocument* document) const = 0;
            virtual bool doVertexOperation(MapDocumentCommandFacade* document) = 0;
        public:
            void removeHandles(VertexHandleManagerBase& manager);
            void addHandles(VertexHandleManagerBase& manager);
        public:
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            void selectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
            void selectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
        private:
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const;
            virtual void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
            virtual void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const;
        };

        class AddBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<AddBrushVerticesCommand>;
        private:
            Model::VertexToBrushesMap m_vertices;
        public:
            static Ptr add(const Model::VertexToBrushesMap& vertices);
        protected:
            AddBrushVerticesCommand(CommandType type, const String& name, const Model::BrushList& brushes, const Model::VertexToBrushesMap& vertices);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class AddRemoveNodesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<AddRemoveNodesCommand>;
        private:
            typedef enum {
                Action_Add,
                Action_Remove
            } Action;

            Action m_action;
            Model::ParentChildrenMap m_nodesToAdd;
            Model::ParentChildrenMap m_nodesToRemove;
        public:
            static Ptr add(Model::Node* parent, const Model::NodeList& children);
            static Ptr add(const Model::ParentChildrenMap& nodes);
            static Ptr remove(const Model::ParentChildrenMap& nodes);
            ~AddRemoveNodesCommand() override;
        private:
            AddRemoveNodesCommand(Action action, const Model::ParentChildrenMap& nodes);
            static String makeName(Action action);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class ChangeBrushFaceAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ChangeBrushFaceAttributesCommand>;
        private:

            Model::ChangeBrushFaceAttributesRequest m_request;
            Model::Snapshot* m_snapshot;
        public:
            static Ptr command(const Model::ChangeBrushFaceAttributesRequest& request);
        private:
            explicit ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request);
        public:
            ~ChangeBrushFaceAttributesCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            ChangeBrushFaceAttributesCommand(const ChangeBrushFaceAttributesCommand& other);
            ChangeBrushFaceAttributesCommand& operator=(const ChangeBrushFaceAttributesCommand& other);
        };

        class ChangeEntityAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ChangeEntityAttributesCommand>;
        private:
            typedef enum {
                Action_Set,
                Action_Remove,
                Action_Rename
            } Action;

            Action m_action;
            Model::AttributeName m_oldName;
            Model::AttributeName m_newName;
            Model::AttributeValue m_newValue;

            Model::EntityAttributeSnapshot::Map m_snapshots;
        public:
            static Ptr set(const Model::AttributeName& name, const Model::AttributeValue& value);
            static Ptr remove(const Model::AttributeName& name);
            static Ptr rename(const Model::AttributeName& oldName, const Model::AttributeName& newName);
        protected:
            void setName(const Model::AttributeName& name);
            void setNewName(const Model::AttributeName& newName);
            void setNewValue(const Model::AttributeValue& newValue);
        private:
            ChangeEntityAttributesCommand(Action action);
            static String makeName(Action action);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class ConvertEntityColorCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ConvertEntityColorCommand>;
        private:
            Model::AttributeName m_attributeName;
            Assets::ColorRange::Type m_colorRange;

            Model::EntityAttributeSnapshot::Map m_snapshots;
        public:
            static Ptr convert(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange);
        private:
            ConvertEntityColorCommand(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class CopyTexCoordSystemFromFaceCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<CopyTexCoordSystemFromFaceCommand>;
        private:

            Model::Snapshot* m_snapshot;
            std::unique_ptr<Model::TexCoordSystemSnapshot> m_coordSystemSanpshot;
            const vm::plane3 m_sourceFacePlane;
            const Model::WrapStyle m_wrapStyle;
            const Model::BrushFaceAttributes m_attribs;
        public:
            static Ptr command(const Model::TexCoordSystemSnapshot& coordSystemSanpshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
        private:
            CopyTexCoordSystemFromFaceCommand(const Model::TexCoordSystemSnapshot& coordSystemSanpshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
        public:
            ~CopyTexCoordSystemFromFaceCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            CopyTexCoordSystemFromFaceCommand(const CopyTexCoordSystemFromFaceCommand& other);
            CopyTexCoordSystemFromFaceCommand& operator=(const CopyTexCoordSystemFromFaceCommand& other);
        };

        class CurrentGroupCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<CurrentGroupCommand>;
        private:
            Model::Group* m_group;
        public:
            static Ptr push(Model::Group* group);
            static Ptr pop();
        private:
            CurrentGroupCommand(Model::Group* group);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        };

        class DuplicateNodesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<DuplicateNodesCommand>;
        private:
            Model::NodeList m_previouslySelectedNodes;
            Model::NodeList m_nodesToSelect;
            Model::ParentChildrenMap m_addedNodes;
            bool m_firstExecution;
        public:
            static Ptr duplicate();
        private:
            DuplicateNodesCommand();
        public:
            ~DuplicateNodesCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            class CloneParentQuery;
            bool cloneParent(const Model::Node* node) const;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class EntityDefinitionFileCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<EntityDefinitionFileCommand>;
        private:
            Assets::EntityDefinitionFileSpec m_oldSpec;
            Assets::EntityDefinitionFileSpec m_newSpec;
        public:
            static Ptr set(const Assets::EntityDefinitionFileSpec& spec);
        private:
            EntityDefinitionFileCommand(const String& name, const Assets::EntityDefinitionFileSpec& spec);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class FindPlanePointsCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<FindPlanePointsCommand>;
        public:
            static Ptr findPlanePoints();
        private:
            FindPlanePointsCommand();

            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class MoveBrushEdgesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveBrushEdgesCommand>;
        private:
            Model::BrushEdgesMap m_edges;
            std::vector<vm::segment3> m_oldEdgePositions;
            std::vector<vm::segment3> m_newEdgePositions;
            vm::vec3 m_delta;
        public:
            static Ptr move(const Model::EdgeToBrushesMap& edges, const vm::vec3& delta);
        private:
        private:
            MoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta);

            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
        };

        class MoveBrushFacesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveBrushFacesCommand>;
        private:
            Model::BrushFacesMap m_faces;
            std::vector<vm::polygon3> m_oldFacePositions;
            std::vector<vm::polygon3> m_newFacePositions;
            vm::vec3 m_delta;
        public:
            static Ptr move(const Model::FaceToBrushesMap& faces, const vm::vec3& delta);
        private:
            MoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta);

            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
        };

        class MoveBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveBrushVerticesCommand>;
        private:
            Model::BrushVerticesMap m_vertices;
            std::vector<vm::vec3> m_oldVertexPositions;
            std::vector<vm::vec3> m_newVertexPositions;
            vm::vec3 m_delta;
        public:
            static Ptr move(const Model::VertexToBrushesMap& vertices, const vm::vec3& delta);
            bool hasRemainingVertices() const;
        private:
            MoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta);

            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::vec3>& manager) const override;
        };

        class MoveTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveTexturesCommand>;
        private:
            vm::vec3f m_cameraUp;
            vm::vec3f m_cameraRight;
            vm::vec2f m_delta;
        public:
            static Ptr move(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);
        private:
            MoveTexturesCommand(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            void moveTextures(MapDocumentCommandFacade* document, const vm::vec2f& delta) const;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            MoveTexturesCommand(const MoveTexturesCommand& other);
            MoveTexturesCommand& operator=(const MoveTexturesCommand& other);
        };

        class RemoveBrushElementsCommand : public VertexCommand {
        private:
            Model::BrushVerticesMap m_vertices;
        protected:
            RemoveBrushElementsCommand(CommandType type, const String& name, const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class RemoveBrushEdgesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RemoveBrushEdgesCommand>;
        private:
            std::vector<vm::segment3> m_oldEdgePositions;
        public:
            static Ptr remove(const Model::EdgeToBrushesMap& edges);
        private:
            RemoveBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::segment3>& edgePositions);

            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::segment3>& manager) const override;
        };

        class RemoveBrushFacesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RemoveBrushFacesCommand>;
        private:
            std::vector<vm::polygon3> m_oldFacePositions;
        public:
            static Ptr remove(const Model::FaceToBrushesMap& faces);
        private:
            RemoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::polygon3>& facePositions);

            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
        };

        class RemoveBrushVerticesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RemoveBrushVerticesCommand>;
        private:
            std::vector<vm::vec3> m_oldVertexPositions;
        public:
            static Ptr remove(const Model::VertexToBrushesMap& vertices);
        private:
            RemoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const std::vector<vm::vec3>& vertexPositions);
        public:
            void selectNewHandlePositions(VertexHandleManager& manager) const;
            void selectOldHandlePositions(VertexHandleManager& manager) const;
        };

        class RenameGroupsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RenameGroupsCommand>;
        private:
            const String m_newName;
            Model::GroupNameMap m_oldNames;
        public:
            static Ptr rename(const String& newName);
        private:
            RenameGroupsCommand(const String& newName);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class ReparentNodesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ReparentNodesCommand>;
        private:
            Model::ParentChildrenMap m_nodesToAdd;
            Model::ParentChildrenMap m_nodesToRemove;
        public:
            static Ptr reparent(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove);
        private:
            ReparentNodesCommand(const Model::ParentChildrenMap& nodesToAdd, const Model::ParentChildrenMap& nodesToRemove);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class ResizeBrushesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ResizeBrushesCommand>;
        private:
            std::vector<vm::polygon3> m_faces;
            std::vector<vm::polygon3> m_newFaces;
            vm::vec3 m_delta;
        public:
            static Ptr resize(const std::vector<vm::polygon3>& faces, const vm::vec3& delta);
        private:
            ResizeBrushesCommand(const std::vector<vm::polygon3>& faces, const vm::vec3& delta);

            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class RotateTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RotateTexturesCommand>;
        private:
            float m_angle;
        public:
            static Ptr rotate(float angle);
        private:
            RotateTexturesCommand(float angle);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool rotateTextures(MapDocumentCommandFacade* document, float angle) const;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            RotateTexturesCommand(const RotateTexturesCommand& other);
            RotateTexturesCommand& operator=(const RotateTexturesCommand& other);
        };

        class SelectionCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SelectionCommand>;
        private:
            typedef enum {
                Action_SelectNodes,
                Action_SelectFaces,
                Action_SelectAllNodes,
                Action_SelectAllFaces,
                Action_ConvertToFaces,
                Action_DeselectNodes,
                Action_DeselectFaces,
                Action_DeselectAll
            } Action;

            Action m_action;

            Model::NodeList m_nodes;
            Model::BrushFaceReference::List m_faceRefs;

            Model::NodeList m_previouslySelectedNodes;
            Model::BrushFaceReference::List m_previouslySelectedFaceRefs;
        public:
            static Ptr select(const Model::NodeList& nodes);
            static Ptr select(const Model::BrushFaceList& faces);

            static Ptr convertToFaces();
            static Ptr selectAllNodes();
            static Ptr selectAllFaces();

            static Ptr deselect(const Model::NodeList& nodes);
            static Ptr deselect(const Model::BrushFaceList& faces);
            static Ptr deselectAll();
        private:
            SelectionCommand(Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces);
            static String makeName(Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatDelimiter() const override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class SetLockStateCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SetLockStateCommand>;
        private:
            Model::NodeList m_nodes;
            Model::LockState m_lockState;
            Model::LockStateMap m_oldLockState;
        public:
            static Ptr lock(const Model::NodeList& nodes);
            static Ptr unlock(const Model::NodeList& nodes);
            static Ptr reset(const Model::NodeList& nodes);
        private:
            SetLockStateCommand(const Model::NodeList& nodes, Model::LockState lockState);
            static String makeName(Model::LockState lockState);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        };

        class SetModsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SetModsCommand>;
        private:
            StringList m_oldMods;
            StringList m_newMods;
        public:
            static Ptr set(const StringList& mods);
        private:
            SetModsCommand(const String& name, const StringList& mods);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class SetTextureCollectionsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SetTextureCollectionsCommand>;
        private:
            IO::Path::List m_paths;
            IO::Path::List m_oldPaths;
        public:
            static Ptr set(const IO::Path::List& paths);
        private:
            SetTextureCollectionsCommand(const IO::Path::List& paths);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class SetVisibilityCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SetVisibilityCommand>;
        private:
            typedef enum {
                Action_Reset,
                Action_Hide,
                Action_Show,
                Action_Ensure
            } Action;

            Model::NodeList m_nodes;
            CommandType m_action;
            Model::VisibilityMap m_oldState;
        public:
            static Ptr show(const Model::NodeList& nodes);
            static Ptr hide(const Model::NodeList& nodes);
            static Ptr ensureVisible(const Model::NodeList& nodes);
            static Ptr reset(const Model::NodeList& nodes);
        private:
            SetVisibilityCommand(const Model::NodeList& nodes, Action action);
            static String makeName(Action action);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
        };

        class ShearTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ShearTexturesCommand>;
        private:
            vm::vec2f m_factors;
        public:
            static Ptr shear(const vm::vec2f& factors);
        private:
            ShearTexturesCommand(const vm::vec2f& factors);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool shearTextures(MapDocumentCommandFacade* document, const vm::vec2f& factors);

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            ShearTexturesCommand(const ShearTexturesCommand& other);
            ShearTexturesCommand& operator=(const ShearTexturesCommand& other);
        };

        class SnapBrushVerticesCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SnapBrushVerticesCommand>;
        private:
            FloatType m_snapTo;
        public:
            static SnapBrushVerticesCommand::Ptr snap(FloatType snapTo);
        private:
            explicit SnapBrushVerticesCommand(FloatType snapTo);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class TransformObjectsCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<TransformObjectsCommand>;
        private:
            typedef enum {
                Action_Translate,
                Action_Rotate,
                Action_Flip,
                Action_Shear,
                Action_Scale
            } Action;

            Action m_action;
            vm::mat4x4 m_transform;
            bool m_lockTextures;
        public:
            static Ptr translate(const vm::vec3& delta, bool lockTextures);
            static Ptr rotate(const vm::vec3& center, const vm::vec3& axis, FloatType angle, bool lockTextures);
            static Ptr scale(const vm::bbox3& oldBBox, const vm::bbox3& newBBox, bool lockTextures);
            static Ptr scale(const vm::vec3& center, const vm::vec3& scaleFactors, bool lockTextures);
            static Ptr shearBBox(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta, bool lockTextures);
            static Ptr flip(const vm::vec3& center, vm::axis::type axis, bool lockTextures);
        private:
            TransformObjectsCommand(Action action, const String& name, const vm::mat4x4& transform, bool lockTextures);

            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class UpdateEntitySpawnflagCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<UpdateEntitySpawnflagCommand>;
        private:
            bool m_setFlag;
            Model::AttributeName m_attributeName;
            size_t m_flagIndex;
        public:
            static Ptr update(const Model::AttributeName& name, const size_t flagIndex, const bool setFlag);
        private:
            UpdateEntitySpawnflagCommand(const Model::AttributeName& attributeName, const size_t flagIndex, const bool setFlag);
            static String makeName(const bool setFlag);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif
