/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __SimpleSpline_H__
#define __Simplepline_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre {


    /** A very simple spline class which implements the Catmull-Rom class of splines.
    @remarks
        Splines are bendy lines. You define a series of points, and the spline forms
        a smoother line between the points to eliminate the sharp angles.
    @par
        Catmull-Rom splines are a specialisation of the general Hermite spline. With
        a Hermite spline, you define the start and end point of the line, and 2 tangents,
        one at the start of the line and one at the end. The Catmull-Rom spline simplifies
        this by just asking you to define a series of points, and the tangents are 
        created for you. 
    */
    class _OgreExport SimpleSpline
    {
    public:
        SimpleSpline();
        ~SimpleSpline();

        /** Adds a point to the spline. */
        void addPoint(const Vector3& p);

        /** Returns an interpolated point based on a parametric value over the whole series.
        @remarks
            Given a t value between 0 and 1 representing the parametric distance along the
            whole length of the spline, this method returns an interpolated point.
        @param t Parametric value.
        */
        Vector3 interpolate(Real t);

        /** Interpolates a single segment of the spline given a parametric value.
        @param fromIndex The point index to treat as t=0. fromIndex + 1 is deemed to be t=1
        @param t Parametric value
        */
        Vector3 interpolate(unsigned int fromIndex, Real t);

    protected:

        /** Recalculates the tangents associated with this spline. */
        void recalcTangents(void);


        std::vector<Vector3> mPoints;
        std::vector<Vector3> mTangents;

        /// Matrix of coefficients 
        Matrix4 mCoeffs;



    };


}


#endif

