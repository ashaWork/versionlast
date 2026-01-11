/* Start Header ************************************************************************/
/*!
\file       undoRedo.h
\author     Hugo Low Ren Hao, low.h, 2402272
						- 100%
\par				low.h@digipen.edu
\date       November, 10th, 2025
\brief      Class for recording the steps and snapshots for undo and redo. UndoRedoManager
			should be the only public interface needed to track and execute undo and redo
			in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#pragma once
#include <string>
#include <memory>
#include <deque>
#include <functional>
#include "GameObjectManager.h"
#include "Editor/gameDebugLog.h"
#include "Editor/editorState.h"

// transform state of a game obj
struct TransformSnapshot {
	float x{}, y{}, z{};
	float rotation{};
	float scaleX{}, scaleY{}, scaleZ{};
};

// base class
class CmdInterface {
public:
	virtual ~CmdInterface() = default;

	// these function dont have implementation in this class, any derived class must implement them
	virtual void execute() = 0;
	virtual void undo() = 0;

	// to be override in derived class, store obj name (in case of rename)
	virtual void updateObjName(const std::string&, const std::string&) {}
};

// store transform changes
class TransformCmd : public CmdInterface {
public:
	// ctor with before/other state
	TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after);

	// apply AFTER transform snapshot (final position aft transform changed)
	void execute() override;

	// apply BEFORE transform snapshot (put back to initial transform)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	// helper to write transform data to a game obj
	void applySnapshot(const TransformSnapshot& snap);

	GameObjectManager& m_manager;
	std::string m_objectName;
	TransformSnapshot m_before;
	TransformSnapshot m_after;
};

// store obj creation
class CreateObjectCmd : public CmdInterface {
public:
	CreateObjectCmd(GameObjectManager& manager, const std::string& objName);

	// create obj (serialize to save it)
	void execute() override;

	// delete the created obj (undo the creation)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_serializedData;
	bool m_wasExecuted = false;
};

// store for obj deletion
class DeleteObjCmd : public CmdInterface {
public:
	DeleteObjCmd(GameObjectManager& manager, GameObject* obj);

	// handle deletion of bj
	void execute() override;

	// recreate obj from serialized data (undo the deletion)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_serializedData;
	int m_layer = 0;
};

// big boss for undo/redo operation (singleton)
class UndoRedoManager {
public:
	static UndoRedoManager& Instance() {
		static UndoRedoManager instance;
		return instance;
	}

	// exesute and store a command
	void executeCmd(std::unique_ptr<CmdInterface> cmd);

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

	// undo last command
	void undo();

	// redo last command
	void redo();

	// boolean to check if undo/redo is doable
	bool canUndo() const { return !m_undoDeque.empty(); }
	bool canRedo() const { return !m_redoDeque.empty(); }

	// clear all history (i.e. load scene, stop simulation)
	void clear();

	// start tracking transform changes
	void beginTransformEdit(GameObject* obj);

	// ebd tracking and create command if changed detected
	void endTransformEdit(GameObjectManager& manager, GameObject* obj);

	// check if currently editing transform
	bool isEditingTransform() const { return m_isEditingTransform; }

	static constexpr size_t MAX_UNDO_STEPS{ 20 };

private:
	UndoRedoManager() = default;

	std::deque<std::unique_ptr<CmdInterface>> m_undoDeque;
	std::deque<std::unique_ptr<CmdInterface>> m_redoDeque;

	// for transform
	bool m_isEditingTransform = false;
	std::string m_editingObjName;
	TransformSnapshot m_transformBefore;
};

// take a snapshot of an object's transform state
TransformSnapshot captureTransform(GameObject* obj);
#endif