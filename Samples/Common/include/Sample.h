#ifndef __Sample_H__
#define __Sample_H__

#include "Ogre.h"

namespace OgreBites
{
    class Sample : public Ogre::FrameListener, public Ogre::WindowEventListener
    {
    public:

        Sample()
        {
            mCamera = 0;
        }

        Ogre::Camera* getCamera()
        {
            return mCamera;
        }

    protected:

        Ogre::Camera* mCamera;
    };
}

#endif
