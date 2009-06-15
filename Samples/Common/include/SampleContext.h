#ifndef __SampleContext_H__
#define __SampleContext_H__

#include "Ogre.h"
#include "Sample.h"

#define OIS_DYNAMIC_LIB
#include "OIS/OIS.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

/*-----------------------------------------------------------------------------
| This function will return the appropriate working directory depending
| on the platform. For Windows, a blank string will suffice. For OS X,
| however, we need to do a little extra work.
-----------------------------------------------------------------------------*/
std::string macBundlePath()
{
	char path[1024];
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	assert(mainBundle);
	CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
	assert(mainBundleURL);
	CFStringRef cfStringRef = CFURLCopyFileSystemPath(mainBundleURL, kCFURLPOSIXPathStyle);
	assert(cfStringRef);
	CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
	CFRelease(mainBundleURL);
	CFRelease(cfStringRef);
	return std::string(path);
}
#endif

namespace OgreBites
{
	/*=============================================================================
	| Base class responsible for setting up a common context for samples.
	| May be subclassed for specific sample types (not specific samples).
	| Allows one sample to run at a time, while maintaining a sample queue.
	=============================================================================*/
	class SampleContext : public Ogre::FrameListener, public Ogre::WindowEventListener
	{
	public:

		SampleContext()
		{
			mRoot = 0;
			mWindow = 0;
			mCurrentSample = 0;
		}

		virtual Ogre::RenderWindow* getRenderWindow()
		{
			return mWindow;
		}

		virtual Sample* getCurrentSample()
		{
			return mCurrentSample;
		}

		/*-----------------------------------------------------------------------------
		| Adds a sample to the end of the queue.
		-----------------------------------------------------------------------------*/
		virtual void queueSample(Sample* s)
		{
			mSampleQueue.push(s);
		}

		/*-----------------------------------------------------------------------------
		| Retrieves the next sample in the queue without removing it.
		-----------------------------------------------------------------------------*/
		virtual Sample* getNextSample()
		{
			if (mSampleQueue.empty()) return 0;
			return mSampleQueue.front();
		}

		/*-----------------------------------------------------------------------------
		| Removes the next sample in the queue without running it.
		-----------------------------------------------------------------------------*/
		virtual void skipNextSample()
		{
			mSampleQueue.pop();
		}

		virtual unsigned int getNumQueuedSamples()
		{
			return mSampleQueue.size();
		}

		virtual void clearSampleQueue()
		{
			while (!mSampleQueue.empty()) mSampleQueue.pop();
		}

		/*-----------------------------------------------------------------------------
		| Quits the current sample and starts a new one.
		-----------------------------------------------------------------------------*/
		virtual void runSample(Sample* s)
		{
			if (!s) return;

			if (mCurrentSample) mCurrentSample->quit();

			mWindow->removeAllViewports();
			mWindow->addViewport(0);

			s->start(mWindow);

			mCurrentSample = s;
		}

		/*-----------------------------------------------------------------------------
		| This function quits the current sample and starts the next one.
		| Returns false if no more samples are queued.
		-----------------------------------------------------------------------------*/
		virtual bool runNextSample()
		{
			if (!mSampleQueue.empty())
			{
				runSample(mSampleQueue.front());
				mSampleQueue.pop();
				return true;
			}

			return false;
		}

		/*-----------------------------------------------------------------------------
		| This function encapsulates the entire lifetime of the context. It calls
		| setup, starts the first queued sample, and enters the rendering loop.
		| When rendering finishes, the context shuts down.
		-----------------------------------------------------------------------------*/
		virtual void go()
		{
			if (!setup()) return;
			if (runNextSample()) mRoot->startRendering();  // start initial sample and enter render loop
			shutdown();
		}

	protected:

		/*-----------------------------------------------------------------------------
		| Sets up the context - the parts that are required by every sample to run.
		-----------------------------------------------------------------------------*/
		virtual bool setup()
		{
			// get platform-specific working directory
			Ogre::String workDir = "";
			#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			workDir = macBundlePath() + "/Contents/Resources/";
			#endif

			mRoot = OGRE_NEW Ogre::Root(workDir + "Plugins.cfg", workDir + "Ogre.cfg", workDir + "Ogre.log");

			if (!configure()) return false;    // configure the settings

			createWindow();
			setupInput();
			locateResources();
			loadResources();

			Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

			// adds context as listener to process context-level (above the sample level) events
			mRoot->addFrameListener(this);
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

			return true;
		}

		/*-----------------------------------------------------------------------------
		| Configures the startup settings for OGRE. I use the config dialog here,
		| but you can also restore from a config file.
		-----------------------------------------------------------------------------*/
		virtual bool configure()
		{
			return mRoot->showConfigDialog();
			// return mRoot->restoreConfig();
		}

		/*-----------------------------------------------------------------------------
		| Creates the render window to be used for this context. I use an auto-created
		| window here, but you can also create an external window if you wish.
		| Just don't forget to initialise the root.
		-----------------------------------------------------------------------------*/
		virtual void createWindow()
		{
			mWindow = mRoot->initialise(true);
		}

		/*-----------------------------------------------------------------------------
		| Sets up OIS input.
		-----------------------------------------------------------------------------*/
		virtual void setupInput()
		{
			OIS::ParamList pl;
			size_t winHandle = 0;
			std::ostringstream winHandleStr;

			mWindow->getCustomAttribute("WINDOW", &winHandle);
			winHandleStr << winHandle;

			pl.insert(std::make_pair("WINDOW", winHandleStr.str()));

			mInputMgr = OIS::InputManager::createInputSystem(pl);

			createInputDevices();      // create the specific input devices

			windowResized(mWindow);    // do an initial adjustment of mouse area
		}

		/*-----------------------------------------------------------------------------
		| Creates the individual input devices. I only create unbuffered keyboard
		| and mouse here because they are the most common, but you can override this
		| method for other modes and devices.
		-----------------------------------------------------------------------------*/
		virtual void createInputDevices()
		{
			mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard, false));
			mMouse = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse, false));
		}

		/*-----------------------------------------------------------------------------
		| Finds context-wide resource groups. I load paths from a config file here,
		| but you can choose your resource locations however you want.
		-----------------------------------------------------------------------------*/
		virtual void locateResources()
		{
			// load resource paths from config file
			Ogre::ConfigFile cf;
			#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			cf.load(macBundlePath() + "/Contents/Resources/Resources.cfg");
			#else
			cf.load("Resources.cfg");
			#endif

			Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
			Ogre::String sec, type, arch;

			// go through all specified resource groups
			while (seci.hasMoreElements())
			{
				sec = seci.peekNextKey();
				Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
				Ogre::ConfigFile::SettingsMultiMap::iterator i;

				// go through all resource paths
				for (i = settings->begin(); i != settings->end(); i++)
				{
					type = i->first;
					arch = i->second;

					#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
					if (!Ogre::StringUtil::startsWith(archName, "/", false)) // only adjust relative dirs
						archName = String(macBundlePath() + "/" + archName);
					#endif
					Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
				}
			}
		}

		/*-----------------------------------------------------------------------------
		| Loads context-wide resource groups. I chose here to simply initialise all
		| groups, but you can fully load specific ones if you wish.
		-----------------------------------------------------------------------------*/
		virtual void loadResources()
		{
			Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		}

		/*-----------------------------------------------------------------------------
		| Cleans up and shuts down the context.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
			// remove window event listener before shutting down OIS
			Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);

			shutdownInput();

			if (mRoot) 
			{
				OGRE_DELETE mRoot;
				mRoot = 0;
			}
		}

		/*-----------------------------------------------------------------------------
		| Destroys OIS input devices and the input manager.
		-----------------------------------------------------------------------------*/
		virtual void shutdownInput()
		{
			if (mInputMgr)
			{
				mInputMgr->destroyInputObject(mKeyboard);
				mInputMgr->destroyInputObject(mMouse);

				OIS::InputManager::destroyInputSystem(mInputMgr);
				mInputMgr = 0;
			}
		}
			
		/*-----------------------------------------------------------------------------
		| Processes frame events.
		-----------------------------------------------------------------------------*/
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			// quit if window was closed
			if (mWindow->isClosed()) return false;
			// quit if current sample has ended and cannot run the next one
			if (mCurrentSample->isDone() && !runNextSample()) return false;

			captureInputDevices();

			return true;
		}

		/*-----------------------------------------------------------------------------
		| Captures input device states.
		-----------------------------------------------------------------------------*/
		virtual void captureInputDevices()
		{
			mKeyboard->capture();
			mMouse->capture();
		}

		/*-----------------------------------------------------------------------------
		| Processes window size change event. Adjusts mouse's region to match that
		| of the window. You could also override this method to prevent resizing.
		-----------------------------------------------------------------------------*/
		virtual void windowResized(Ogre::RenderWindow* rw)
		{
			const OIS::MouseState& ms = mMouse->getMouseState();
			ms.width = rw->getWidth();
			ms.height = rw->getHeight();
		}

		// OGRE objects
		Ogre::Root* mRoot;
		Ogre::RenderWindow* mWindow;

		// OIS objects
		OIS::InputManager* mInputMgr;
		OIS::Keyboard* mKeyboard;
		OIS::Mouse* mMouse;

		Sample* mCurrentSample;         // currently running sample
		SampleQueue mSampleQueue;       // queued samples
	};
}

#endif
