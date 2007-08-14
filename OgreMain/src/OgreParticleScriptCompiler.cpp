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

#include "OgreStableHeaders.h"
#include "OgreParticleScriptCompiler.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"

namespace Ogre{

	// ParticleScriptCompilerListener
	ParticleScriptCompilerListener::ParticleScriptCompilerListener()
	{
	}

	bool ParticleScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, Ogre::ParticleScriptCompiler *)
	{
		return false;
	}

	ParticleSystem *ParticleScriptCompilerListener::getParticleSystem(const Ogre::String &name, const Ogre::String &group)
	{
		// By default create a new template
		return ParticleSystemManager::getSingleton().createTemplate(name, group);
	}

	// ParticleScriptCompiler
	ParticleScriptCompiler::ParticleScriptCompiler()
		:mListener(0), mSystem(0)
	{
		mAllowNontypedObjects = true;
	}

	void ParticleScriptCompiler::setListener(ParticleScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	ParticleSystem *ParticleScriptCompiler::getParticleSystem() const
	{
		return mSystem;
	}

	bool ParticleScriptCompiler::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			// Delegate some processing to the listener
			if(!processNode(i, nodes->end()))
			{
				// The first just the name of the particle system, but ignore "abstract"
				if((*i)->token != "abstract")
				{
					compileParticleSystem(*i);
				}
				++i;
			}
		}
		return mErrors.empty();
	}

	bool ParticleScriptCompiler::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		if(mListener)
			return mListener->processNode(i, end, this);
		return false;
	}

	ScriptNodeListPtr ParticleScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		return nodes;
	}

	void ParticleScriptCompiler::compileParticleSystem(const ScriptNodePtr &node)
	{
		// Use the listener to get the particle system object
		if(mListener)
			mSystem = mListener->getParticleSystem(node->token, mGroup);
		else
			mSystem = ParticleSystemManager::getSingleton().createTemplate(node->token, mGroup);
		if(!mSystem)
		{
			addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, node->column);
			return;
		}

		// The particle system does not support extra option after its name, so skip ahead to the '{'
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				// Each property in the particle system has only 1 value associated with it
				String name = (*j)->token, value;

				if(name == "emitter")
				{
					compileEmitter(*j);
				}
				else if(name == "affector")
				{
					compileAffector(*j);
				}
				else
				{
					// Construct the parameter values from the children of the property
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
					if(!mSystem->setParameter(name, value))
					{
						if(mSystem->getRenderer())
						{
							if(!mSystem->getRenderer()->setParameter(name, value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
					}
				}
				++j;
			}
		}

		// Reset the pointer to the system
		mSystem = 0;
	}

	void ParticleScriptCompiler::compileEmitter(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleEmitter *emitter = 0;
		String type = node->children.front()->token;
		try{
			emitter = mSystem->addEmitter(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!emitter->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	void ParticleScriptCompiler::compileAffector(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleAffector *affector = 0;
		String type = node->children.front()->token;
		try{
			affector = mSystem->addAffector(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!affector->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	String ParticleScriptCompiler::getParameterValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		String retval;
		if(i != end)
		{
			if((*i)->type == SNT_WORD || (*i)->type == SNT_QUOTE)
				retval = (*i)->token;
			else if((*i)->type == SNT_NUMBER)
				retval = StringConverter::toString((*i)->data);
		}

		++i;
		while(i != end)
		{
			if((*i)->type == SNT_WORD || (*i)->type == SNT_QUOTE)
				retval = retval + " " + (*i)->token;
			else if((*i)->type == SNT_NUMBER)
				retval = retval + " " + StringConverter::toString((*i)->data);
			++i;
		}

		return retval;
	}
}