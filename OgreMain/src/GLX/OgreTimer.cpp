/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright (c) 2000-2006 The OGRE Team
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
#include "OgreTimer.h"
#include <sys/time.h>

using namespace Ogre;

//--------------------------------------------------------------------------------//
Timer::Timer()
{
	reset();
}

//--------------------------------------------------------------------------------//
Timer::~Timer()
{
}

//--------------------------------------------------------------------------------//
void Timer::reset()
{
	zeroClock = clock();
	gettimeofday(&start, NULL);
}

//--------------------------------------------------------------------------------//
unsigned long Timer::getMilliseconds()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
}

//--------------------------------------------------------------------------------//
unsigned long Timer::getMicroseconds()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec-start.tv_sec)*1000000+(now.tv_usec-start.tv_usec);
}

//-- Common Across All Timers ----------------------------------------------------//
unsigned long Timer::getMillisecondsCPU()
{
	clock_t newClock = clock();
	return (unsigned long)((float)(newClock-zeroClock) / ((float)CLOCKS_PER_SEC/1000.0)) ;
}

//-- Common Across All Timers ----------------------------------------------------//
unsigned long Timer::getMicrosecondsCPU()
{
	clock_t newClock = clock();
	return (unsigned long)((float)(newClock-zeroClock) / ((float)CLOCKS_PER_SEC/1000000.0)) ;
}
