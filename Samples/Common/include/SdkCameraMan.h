#ifndef __SdkCameraMan_H__
#define __SdkCameraMan_H__

#include "Ogre.h"

namespace OgreBites
{
	/*=============================================================================
	| Utility class for controlling the camera in samples. Features different
	| camera styles and customisable controls.
	=============================================================================*/
	class SdkCameraMan
    {
    public:

		Ogre::Camera* getCamera()
		{
			return mCamera;
		}

		/*-----------------------------------------------------------------------------
		| Updates camera movement.
		-----------------------------------------------------------------------------*/
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			return true;
		}

    protected:

		Ogre::Camera* mCamera;
    };
}

#endif
