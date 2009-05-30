#ifndef __SampleContext_H__
#define __SampleContext_H__

#include "Ogre.h"
#include "Sample.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>
#endif

/*-----------------------------------------------------------------------------
| This function will return the appropriate working directory depending
| on the platform. For Windows, a blank string will suffice. For OS X,
| however, we need to do a little extra work.
-----------------------------------------------------------------------------*/
std::string getWorkingDirectory()
{
    #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);
    CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
    assert(mainBundleURL);
    CFStringRef cfStringRef = 
    CFURLCopyFileSystemPath(mainBundleURL, kCFURLPOSIXPathStyle);
    assert(cfStringRef);
    CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
    CFRelease(mainBundleURL);
    CFRelease(cfStringRef);
    return std::string(path) + "/Contents/Resources/";
    #else
    return "";
    #endif
}

namespace OgreBites
{
    /*-----------------------------------------------------------------------------
    | To be documented...
    -----------------------------------------------------------------------------*/
    class SampleContext : public Ogre::FrameListener, public Ogre::WindowEventListener
	{
	public:

		SampleContext()
		{
			mRoot = 0;
			mWindow = 0;
			mSample = 0;
            mBlankSample = new Sample();
		}

        ~SampleContext()
        {
            if (mBlankSample) delete mBlankSample;
        }

		virtual void switchSample(Sample* sample)
		{
            // to be implemented...

			mSample = sample;
		}

		virtual Sample* getCurrentSample()
		{
			return mSample;
		}

		virtual void go(Sample* initialSample = 0)
		{
			if (!setup()) return;

            if (initialSample) switchSample(initialSample);
			else switchSample(mBlankSample);

			mRoot->startRendering();

			shutdown();
		}

	protected:

		virtual bool setup()
		{
            // NOTE: override to setup common elements like menus, sorta like this...
            /*
                virtual bool setup()
                {
                    if (!SampleContext::setup()) return false;
                    // setup menu
                    return true;
                }
            */

            Ogre::String workDir = getWorkingDirectory();
            mRoot = OGRE_NEW Ogre::Root(workDir + "Plugins.cfg",
                                        workDir + "Ogre.cfg",
                                        workDir + "Ogre.log");

            if (!configure()) return false;

            createWindow();

            setupInput();

            locateResources();

            loadResources();

            mRoot->addFrameListener(this);

			return true;
		}

        virtual bool configure()
        {
            return mRoot->showConfigDialog();
        }

        virtual void createWindow()
        {
            mWindow = mRoot->initialise(true);
            mWindow->addViewport(0);
        }

        virtual void setupInput()
        {
            // to be implemented...
        }

        virtual void locateResources()
        {
        }

        virtual void loadResources()
        {
            Ogre::ResourceGroupManager::getSingletonPtr()->initialiseAllResourceGroups();
        }

		virtual void shutdown()
		{
            if (mRoot) OGRE_DELETE mRoot;
		}

		Ogre::Root* mRoot;
		Ogre::RenderWindow* mWindow;
		Sample* mSample;
        Sample* mBlankSample;
	};
}

#endif