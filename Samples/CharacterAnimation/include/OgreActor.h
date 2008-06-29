/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/


#ifndef __Actor_H__
#define __Actor_H__


#include "OgrePrerequisites.h"
#include "OgreAI.h"

namespace Ogre {

	/** An actor in action . 
	@remarks
	This class defines the interface for an actor that has action capability, whether it 
	is a character or an animal, whether it is autonomous or is controlled by users interactively,
	whatever it is driven by motion data or simulated by controllers. 
	It is an abstract class, you should subclass it to create your instance class. 
	@note
	Don't like the Mesh class in Ogre, you don't need to create a mesh and then attach it to an entity
	using attachObject().
	When you create a subclass of Actor, e.g. class Character, it already is an entity in OgreScene,
	it will be rendered.
	
	How does this actor will be displayed is its subclass' role, you should give any resources as needed
	to its constructor() or assign afterward, e.g. Character( const Mesh& ......)
	*/
	class _OgreExport Actor
	{
	public:
		virtual ~Actor();

		/** 
		Act as the actor's behavior is designed.
		*/
		void Act() = 0;

	protected:
		/** Protected unnamed constructor to prevent default construction. 
		*/
		Actor();
		/** 
		This is the real entity carries e.g. mesh and all the geometry stuff
		@remarks
		It is created using SceneManager::createEntity()
		*/
		Entity* m_Entity;
		/** 
		This is the sceneNode exists in the world space of a scene
		@remarks
		It is created using SceneManager::getRootSceneNode()->createChildSceneNode()
		*/
		SceneNode* m_SceneNode;
		AI_Base*   m_pAiProxy;

	};

	class _OgreExport Character : public Actor
	{
	public:
		virtual ~Character();

		/** Creates a character from with mesh and attached to scenenode
		@param 
		CharacterName, 
		scenemgr, this is the scenemanager of the scene in which this character is created 
		meshname, mesh will be created  mSceneMgr->createEntity() 
		scenenode, if this is assigned, the character is attached to a specific scenenode
		*/
		Character(const String& CharacterName, SceneManager*& scenemgr, const String& meshname, SceneNode*& scenenode);

		/** Creates a character from with mesh
		@param 
		scenemgr, this is the scenemanager of the scene in which this character is created 
		meshname, mesh will be created  mSceneMgr->createEntity() 
		@remark
		this character is attached to a child scenenode created by SceneRoot in default 
		*/
		Character(SceneManager*& scenemgr, const String& meshname);
		void Act();
	protected:
		
	private:
		SceneManager* m_SceneMgr;


	};

	Character::Character(const String& CharacterName, SceneManager*& scenemgr, const Ogre::String &meshname)
	{
		m_Entity = scenemgr->createEntity(CharacterName,meshname+".mesh");
		m_SceneNode = scenemgr->getRootSceneNode()->createChildSceneNode(CharacterName);
		m_SceneNode->attachObject(m_Entity);
		m_pAiProxy = new AI_Motiongraph(m_Entity->getSkeleton());

	}
}

#endif