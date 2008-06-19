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
#ifndef __Lod_Strategy_H__
#define __Lod_Strategy_H__

#include "OgrePrerequisites.h"

#include "OgreMesh.h"

#include "OgreMovableObject.h"
#include "OgreCamera.h"

namespace Ogre {

    /** Strategy for determining level of detail */
    class _OgreExport LodStrategy
    {
    protected:
        /** Name of this strategy. */
        String mName;

        /** Compute the lod value for a given movable object relative to a given camera. */
        virtual Real getValueImpl(const MovableObject *movableObject, const Camera *camera) const = 0;

    public:
        /** Constructor accepting name. */
        LodStrategy(const String& name);

        /** Virtual destructor. */
        virtual ~LodStrategy();

        /** Get the value of the first (highest) level of detail. */
        virtual Real getBaseValue() const;

        /** Transform lod bias so it only needs to be multiplied by the lod value. */
        virtual Real transformBias(Real factor) const = 0;

        /** Compute the lod value for a given movable object relative to a given camera. */
        Real getValue(const MovableObject *movableObject, const Camera *camera) const;

        /** Get the index of the lod usage which applies to a given value. */
        virtual ushort getIndex(Real value, const Mesh::MeshLodUsageList& meshLodUsageList) const;

        /** Get the index of the lod usage which applies to a given value. */
        virtual ushort getIndex(Real value, const Material::LodValueList& materialLodValueList) const;

        // This would be required, but Material::LodValueList is the same type.
        ////** Get the index of the lod usage which applies to a given value. */
        //virtual ushort getIndex(Real value, const Mesh::LodValueList& meshLodValueList) const;

        /** Sort mesh lod usage list from greatest to least detail */
        virtual void sort(Mesh::MeshLodUsageList& meshLodUsageList) const;

        /** Determine if the lod values are sorted from greatest detail to least detail. */
        virtual bool isSorted(const Mesh::LodValueList& values) const;

        /** Assert that the lod values are sorted from greatest detail to least detail. */
        void assertSorted(const Mesh::LodValueList& values) const;

        /** Get the name of this strategy. */
        const String& getName() const { return mName; };

    protected:
        static bool isSortedAscending(const Mesh::LodValueList& values);
        static bool isSortedDecending(const Mesh::LodValueList& values);

        static void sortAscending(Mesh::MeshLodUsageList& meshLodUsageList);
        static void sortDecending(Mesh::MeshLodUsageList& meshLodUsageList);

        static ushort getIndexAscending(Real value, const Mesh::MeshLodUsageList& meshLodUsageList);

        static ushort getIndexAscending(Real value, const Material::LodValueList& materialLodValueList);

        //static ushort getIndexAscending(Real value, const Mesh::LodValueList& meshLodValueList);
    };

} // namespace

#endif
