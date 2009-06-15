#ifndef __Sample_H__
#define __Sample_H__

#include "Ogre.h"

namespace OgreBites
{
	/*=============================================================================
	| Base class responsible for everything specific to one sample.
	| Designed to be subclassed for each sample.
	=============================================================================*/
	class Sample : public Ogre::FrameListener, Ogre::WindowEventListener
    {
    public:

		Sample()
        {
			mWindow = 0;
			mSceneMgr = 0;
			mDone = true;       // sample has not started
        }

		/*-----------------------------------------------------------------------------
		| Returns name of the sample.
		-----------------------------------------------------------------------------*/
		virtual Ogre::String getName()
		{
			return "OGRE Sample";
		}

		Ogre::SceneManager* getSceneManager()
		{
			return mSceneMgr;
		}

		virtual bool isDone()
		{
			return mDone;
		}

		/*-----------------------------------------------------------------------------
		| Starts a sample. Used by the SampleContext class. Do not call directly.
		-----------------------------------------------------------------------------*/
		virtual void start(Ogre::RenderWindow* rw)
		{
			if (!mDone) return;    // sample already started

			mWindow = rw;          // save off render window from context

			testCapabilities();
			locateResources();
			loadResources();
			createSceneManager();
			setupScene();

			// add self as listener to process sample-specific events
			Ogre::Root::getSingleton().addFrameListener(this);
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

			mDone = false;         // sample now started
		}

		/*-----------------------------------------------------------------------------
		| Resets a sample's state (not resources or listeners, etc.). I only reset
		| the scene graph here, but you can override to reset any custom variables.
		-----------------------------------------------------------------------------*/
		virtual void reset()
		{
			mSceneMgr->clearScene();
			setupScene();
		}

		/*-----------------------------------------------------------------------------
		| Ends a sample. Used by the SampleContext class. Do not call directly.
		-----------------------------------------------------------------------------*/
		virtual void quit()
		{
			if (mDone) return;     // sample already ended

			// remove self as listener
			Ogre::Root::getSingleton().removeFrameListener(this);
			Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
			
			Ogre::Root::getSingleton().destroySceneManager(mSceneMgr);   // destroy the scene manager
			
			unloadResources();     // unload sample-specific resources

			mDone = true;          // sample now ended
		}

    protected:

		/*-----------------------------------------------------------------------------
		| Tests to see if target machine meets any special requirements of
		| this sample. Signal a failure by throwing an exception, which can then be
		| caught by an extended SampleContext if you wish.
		-----------------------------------------------------------------------------*/
		virtual void testCapabilities()
		{
		}

		/*-----------------------------------------------------------------------------
		| Finds sample-specific resources. No such effort is made for most samples,
		| but this is useful for special samples with large, exclusive resources.
		-----------------------------------------------------------------------------*/
		virtual void locateResources()
		{
		}

		/*-----------------------------------------------------------------------------
		| Loads sample-specific resources. No such effort is made for most samples,
		| but this is useful for special samples with large, exclusive resources.
		-----------------------------------------------------------------------------*/
		virtual void loadResources()
		{
		}

		/*-----------------------------------------------------------------------------
		| Creates a scene manager for the sample. A generic one is the default,
		| but many samples require a special kind of scene manager.
		-----------------------------------------------------------------------------*/
		virtual void createSceneManager()
		{
			mSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC);
		}

		/*-----------------------------------------------------------------------------
		| Sets up the scene. There is no default camera, so make your own.
		| See the SDK samples for a camera utility clas and how to use it.
		-----------------------------------------------------------------------------*/
		virtual void setupScene()
		{
		}

		/*-----------------------------------------------------------------------------
		| Unloads sample-specific resources. My method here is simple and good
		| enough for most small samples, but your needs may vary.
		-----------------------------------------------------------------------------*/
		virtual void unloadResources()
		{
			Ogre::ResourceGroupManager::ResourceManagerIterator& resMgrs =
			Ogre::ResourceGroupManager::getSingleton().getResourceManagerIterator();

			while (resMgrs.hasMoreElements())
			{
				resMgrs.getNext()->unloadUnreferencedResources();
			}
		}

		Ogre::RenderWindow* mWindow;     // render window used by the sample context
		Ogre::SceneManager* mSceneMgr;   // scene manager for this sample
		bool mDone;                      // flag to mark the end of the sample
    };

	typedef std::queue<Sample*> SampleQueue;    // typedef for convenience
}

#endif
