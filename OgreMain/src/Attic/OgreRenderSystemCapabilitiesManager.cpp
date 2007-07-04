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
#include "OgreMaterialManager.h"

#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreArchiveManager.h"
#include "OgreArchive.h"
#include "OgreStringConverter.h"

#include "OgreException.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"



namespace Ogre {

    //-----------------------------------------------------------------------
    template<> RenderSystemCapabilitiesManager* Singleton<RenderSystemCapabilitiesManager>::ms_Singleton = 0;
    RenderSystemCapabilitiesManager* RenderSystemCapabilitiesManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    RenderSystemCapabilitiesManager& RenderSystemCapabilitiesManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }

    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::RenderSystemCapabilitiesManager() : mScriptPattern("*.rendercaps")
    {
        mSerializer = new RenderSystemCapabilitiesSerializer();
    }
    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::~RenderSystemCapabilitiesManager()
    {
        CapabilitiesMap::iterator iter = mCapabilitiesMap.begin();
        while(iter != mCapabilitiesMap.end())
        {
        // free memory in RenderSystemCapabilities*
            delete iter->second;
            iter++;
        }

        delete mSerializer;
    }

    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesManager::parseCapabilitiesFromArchive(const String& filename, const String& archiveType, bool recursive)
    {
        // get the list of .rendercaps files
        Archive* arch = ArchiveManager::getSingleton().load(filename, archiveType);
        StringVectorPtr files = arch->find(mScriptPattern, recursive);

        // loop through .rendercaps files and load each one
        StringVector::iterator iter = files->begin();
        while(iter != files->end())
        {
            DataStreamPtr stream = arch->open(*iter);
            mSerializer->parseScript(stream);
            stream->close();
            iter++;
        }
    }

    RenderSystemCapabilities* RenderSystemCapabilitiesManager::loadParsedCapabilities(const String& name)
    {
        return mCapabilitiesMap[name];
    }

    /** Method used by RenderSystemCapabilitiesSerializer::parseScript */
    void RenderSystemCapabilitiesManager::_addRenderSystemCapabilities(const String& name, RenderSystemCapabilities* caps)
    {
        mCapabilitiesMap.insert(CapabilitiesMap::value_type(name, caps));
    }
}





