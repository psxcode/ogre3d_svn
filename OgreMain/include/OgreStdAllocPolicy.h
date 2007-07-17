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

#ifndef STDALLOCPOLICY_H
#define STDALLOCPOLICY_H

#include <limits>
#include <cstddef>
#include <memory>

#include "OgreMemoryManager.h"

namespace Ogre{

    /**
    * A "standard" allocation policy for use with Ogre's custom
    * allocator type. By standard it is implied that this policy will
    * allocate and release its memory using the global new and delete
    * operators, this matches the standard default allocator for STL
    * containers.
    *
    * It takes the following template parameters:-
    * T : the type we will be allocting, (see rebind later)
    *
    */
    template<typename T>
    class StdAllocPolicy
    {
    public :
        /// define our types, as per ISO C++
        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*  const_pointer;
        typedef value_type&         reference;
        typedef const value_type&  const_reference;
        typedef std::size_t        size_type;
        typedef std::ptrdiff_t      difference_type;

    public :
        /// convert StdAllocPolicy<T> to StdAllocPolicy<U>
        template<typename U>
        struct rebind
        {
            typedef class StdAllocPolicy<U> other;
        };

    protected:
        /// ctor
        inline explicit StdAllocPolicy()
        { }

        /// dtor
        inline ~StdAllocPolicy()
        { }

        /// copy ctor
        inline explicit StdAllocPolicy(StdAllocPolicy const&)
        { }

        /// converstion
        template <typename U>
        inline explicit StdAllocPolicy(StdAllocPolicy<U> const&)
        { }

        /**
         * memory allocation
         * @param count number of bytes to allocate
         * (NOT number of elements of T)
         */
        inline pointer allocate(size_type count,
                                typename std::allocator<void>::const_pointer = 0)
        {
            return reinterpret_cast<pointer>
              (MemoryManager::getSingleton().allocMem(count));
        }

        /**
         * memory deallocations
         * @param ptr pointer to memory block
         */
        inline void deallocate(pointer ptr, size_type)
        {
            MemoryManager::getSingleton().purgeMem(ptr);
        }

        /// maximum allocation size
        inline size_type max_size() const
        {
            return std::numeric_limits<size_type>::max();
        }
        
        /**
         * return the size of a memory block allocation
         * @param ptr start of the allocated block
         */
        inline size_type block_size(typename std::allocator<void>::const_pointer ptr = 0)
        {
        	// TODO: this need to be added for profiler
        	(void)ptr;
        	return 0;
        }
        
        /**
         * return the size of an allocation, this will include any 
         * memory manager overhead
         * @param size the requested allocation size
         */
        inline size_type alloc_size(size_type size)
        {
        	// TODO: we only return 0 for now, to match block_size()
        	return 0;
        }

    private:
        /// determin equality
        template<typename oT, typename oT2>
        friend bool operator==( StdAllocPolicy<oT> const&,
                                StdAllocPolicy<oT2> const&);

        /// determin equality
        template<typename oT, typename OtherAllocator>
        friend bool operator==( StdAllocPolicy<oT> const&,
                                OtherAllocator const&);
    };

    /// determin equality, can memory from another allocator
    /// be released by this allocator, (ISO C++)
    template<typename T, typename T2>
    inline bool operator==(StdAllocPolicy<T> const&,
                           StdAllocPolicy<T2> const&)
    {
        return true;
    }

    /// determin equality, can memory from another allocator
    /// be released by this allocator, (ISO C++)
    template<typename T, typename OtherAllocator>
    inline bool operator==(StdAllocPolicy<T> const&,
                           OtherAllocator const&)
    {
        return false;
    }

}// namesoace Ogre
#endif // STDALLOCPOLICY_H
