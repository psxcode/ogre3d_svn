/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General  License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General  License for more details.

You should have received a copy of the GNU Lesser General  License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

-----------------------------------------------------------------------------
*/

/***************************************************************************
OgreScrollEvent.h  -  
  A semantic event which indicates that a component-defined Scroll occured.
  This high-level event is generated by a component (such as a Button) when
  the component-specific Scroll occurs (such as being pressed).
  The event is passed to every every ScrollListener object
  that registered to receive such events using the component's
  addScrollListener method.

  The object that implements the ScrollListener interface
  gets this ScrollEvent when the event occurs. The listener
  is therefore spared the details of processing individual mouse movements
  and mouse clicks, and can instead process a "meaningful" (semantic)
  event like "button pressed".

-------------------
begin                : Nov 19 2002
copyright            : (C) 2002 by Kenny Sabir
email                : kenny@sparksuit.com
***************************************************************************/


#ifndef __ScrollEvent_H__
#define __ScrollEvent_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreInputEvent.h"

namespace Ogre {

/**
 *  
 */
    class _OgreExport ScrollEvent : public InputEvent
    {
    protected:
	/**
	 * The nonlocalized string that gives more details
	 * of what actually caused the event.
	 * This information is very specific to the component
	 * that fired it.

	 */
	size_t mTopVisible;
	size_t mRange;
	size_t mTotal;



	public:
	enum
	{
	/**
	 * The first number in the range of ids used for Scroll events.
	 */
		SE_SCROLL_FIRST				= 2001,

	/**
	 * The last number in the range of ids used for Scroll events.
	 */
		SE_SCROLL_LAST		        = 2001
	};

	/**
	 * This event id indicates that a meaningful Scroll occured.
	 */
	enum
	{
		SE_SCROLL_PERFORMED	= SE_SCROLL_FIRST 
	};


		/**
		 * Constructs a ScrollEvent object with the specified source GuiElement,
		 * type, modifiers, coordinates, and click count.
		 *
		 * @param source       the GuiElement that originated the event
		 * @param id           the integer that identifies the event
		 * @param when         a long int that gives the time the event occurred
		 * @param modifiers    the modifier keys down during event
		 *                     (shift, ctrl, alt, meta)
		 * @param x            the horizontal x coordinate for the mouse location
		 * @param y            the vertical y coordinate for the mouse location
		 * @param ScrollCommand The nonlocalized string that gives more details
		 * of what actually caused the event.
		 */
		 ScrollEvent(ScrollTarget* source, int id, Real when, int modifiers,
			size_t topVisible, size_t range, size_t total) ;

		size_t getTopVisible() const;
		size_t getRange() const;
		size_t getTotal() const;


		/**
		 * Returns a parameter string identifying this Scroll event.
		 * This method is useful for event-logging and for debugging.
		 * 
		 * @return a string identifying the event and its associated command 
		 */
		String paramString() const;

    };


}


#endif 

