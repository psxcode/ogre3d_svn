/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2003 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "O_IStream.h"
#include <Iex.h>
using namespace Imf;


namespace Ogre {

bool O_IStream::read(char c[], int n) {
    size_t s = _chunk.read(c, n);
    if(s != n)
        throw Iex::InputExc ("Unexpected end of file.");
    return _chunk.isEOF();
}

Int64 O_IStream::tellg() {
    return _chunk.getCurrentPtr() - _chunk.getPtr();
}

void O_IStream::seekg(Int64 pos) {
    _chunk.seek(pos);
}

void O_IStream::clear() {
    /* Clear error flags -- there are no error flags */
}


};
