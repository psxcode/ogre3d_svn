#ifndef __SampleContext_H__
#define __SampleContext_H__

#include "Ogre.h"
#include "OgrePlugin.h"
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
	class SampleContext :
		public Ogre::FrameListener,
		public Ogre::WindowEventListener,
		public OIS::KeyListener,
		public OIS::MouseListener
	{
	public:

		SampleContext()
		{
			mRoot = 0;
			mWindow = 0;
			mCurrentSample = 0;
		}

		Ogre::RenderWindow* getRenderWindow()
		{
			return mWindow;
		}

		Sample* getCurrentSample()
		{
			return mCurrentSample;
		}

		/*-----------------------------------------------------------------------------
		| Quits the current sample and starts a new one.
		-----------------------------------------------------------------------------*/
		virtual void runSample(Sample* s)
		{
			if (mCurrentSample) mCurrentSample->_shutdown();    // quit current sample

			mWindow->removeAllViewports();                  // wipe viewports

			if (s)
			{
				// retrieve sample's required plugins and currently installed plugins
				Ogre::Root::PluginInstanceList ips = mRoot->getInstalledPlugins();
				Ogre::StringVector rps = s->getRequiredPlugins();

				for (Ogre::StringVector::iterator rp = rps.begin(); rp != rps.end(); rp++)
				{
					bool found = false;
					// try to find the required plugin in the current installed plugins
					for (Ogre::Root::PluginInstanceList::iterator ip = ips.begin(); ip != ips.end(); ip++)
					{
						if ((*ip)->getName() == *rp)
						{
							found = true;
							break;
						}
					}
					if (!found)  // throw an exception if a plugin is not found
					{
						Ogre::String desc = "Missing required plugin: " + *rp;
						Ogre::String src = "SampleContext::runSample";
						OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, desc, src);
					}
				}

				// throw an exception if samples requires the use of another renderer
				Ogre::String rrs = s->getRequiredRenderSystem();
				if (!rrs.empty() && rrs != mRoot->getRenderSystem()->getName())
				{
					Ogre::String desc = "Not using required renderer: " + rrs;
					Ogre::String src = "SampleContext::runSample";
					OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, desc, src);
				}

				// test system capabilities against sample requirements
				s->testCapabilities(mRoot->getRenderSystem()->getCapabilities());

				s->_setup(mWindow, mKeyboard, mMouse);   // start new sample
			}

			mCurrentSample = s;
		}

		/*-----------------------------------------------------------------------------
		| This function encapsulates the entire lifetime of the context.
		-----------------------------------------------------------------------------*/
		virtual void go(Sample* initialSample = 0)
		{
			createRoot();                     // create root
			if (!oneTimeConfig()) return;     // configure startup settings
			setup();                          // setup context

			// start initial sample and enter render loop
			if (initialSample) runSample(initialSample);
			mRoot->startRendering();

			shutdown();                       // shutdown context
			if (mRoot) OGRE_DELETE mRoot;     // destroy root
		}

	protected:

		/*-----------------------------------------------------------------------------
		| Creates the OGRE root.
		-----------------------------------------------------------------------------*/
		virtual void createRoot()
		{
			// get platform-specific working directory
			Ogre::String workDir = "";
			#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			workDir = macBundlePath() + "/Contents/Resources/";
			#endif

			mRoot = OGRE_NEW Ogre::Root(workDir + "Plugins.cfg", workDir + "Ogre.cfg", workDir + "Ogre.log");
		}

		/*-----------------------------------------------------------------------------
		| Configures the startup settings for OGRE. I use the config dialog here,
		| but you can also restore from a config file. Note that this only happens
		| the first time you start the context.
		-----------------------------------------------------------------------------*/
		virtual bool oneTimeConfig()
		{
			return mRoot->showConfigDialog();
			// return mRoot->restoreConfig();
		}

		/*-----------------------------------------------------------------------------
		| Sets up the context after configuration.
		-----------------------------------------------------------------------------*/
		virtual void setup()
		{
			createWindow();
			setupInput();
			locateResources();
			loadResources();

			Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

			// adds context as listener to process context-level (above the sample level) events
			mRoot->addFrameListener(this);
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
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
		| Creates the individual input devices. I only create a keyboard and mouse
		| here because they are the most common, but you can override this method
		| for other modes and devices.
		-----------------------------------------------------------------------------*/
		virtual void createInputDevices()
		{
			mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard, true));
			mMouse = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse, true));

			mKeyboard->setEventCallback(this);
			mMouse->setEventCallback(this);
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
		| Reconfigures the context with a specific render system and options.
		| Attempts to preserve the current sample state.
		-----------------------------------------------------------------------------*/
		virtual void reconfigure(const Ogre::String& rsName, Ogre::NameValuePairList& rsOptions)
		{
			Sample* lastSample;
			Ogre::NameValuePairList sampleState;

			if (mCurrentSample)
			{
				// save current sample state and quit it
				mCurrentSample->saveState(sampleState);
				mCurrentSample->_shutdown();
				lastSample = mCurrentSample;
				mCurrentSample = 0;            // very important because we want shutdown to ignore it
			}

			shutdown();
			if (mRoot) OGRE_DELETE mRoot;
			createRoot();

			mRoot->setRenderSystem(mRoot->getRenderSystemByName(rsName));   // set new render system

			// set all given render system options
			for (Ogre::NameValuePairList::iterator it = rsOptions.begin(); it != rsOptions.end(); it++)
			{
				mRoot->getRenderSystem()->setConfigOption(it->first, it->second);
			}

			mRoot->saveConfig();    // save configuration

			setup();


			if (lastSample)
			{
				// restart sample and restore its state
				mCurrentSample = lastSample;
				mCurrentSample->_setup(mWindow, mKeyboard, mMouse);
				mCurrentSample->restoreState(sampleState);
			}
		}

		/*-----------------------------------------------------------------------------
		| Cleans up and shuts down the context.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
			if (mCurrentSample)
			{
				mCurrentSample->_shutdown();
				mCurrentSample = 0;
			}

			// remove window event listener before shutting down OIS
			Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);

			shutdownInput();
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
		| Processes frame started events.
		-----------------------------------------------------------------------------*/
		virtual bool frameStarted(const Ogre::FrameEvent& evt)
		{
			captureInputDevices();      // capture input

			// manually call sample callback to ensure correct order
			return mCurrentSample ? mCurrentSample->frameStarted(evt) : true;
		}
			
		/*-----------------------------------------------------------------------------
		| Processes rendering queued events.
		-----------------------------------------------------------------------------*/
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			// manually call sample callback to ensure correct order
			return mCurrentSample ? mCurrentSample->frameRenderingQueued(evt) : true;
		}
			
		/*-----------------------------------------------------------------------------
		| Processes frame ended events.
		-----------------------------------------------------------------------------*/
		virtual bool frameEnded(const Ogre::FrameEvent& evt)
		{
			// manually call sample callback to ensure correct order
			if (mCurrentSample && !mCurrentSample->frameEnded(evt)) return false;
			// quit if window was closed
			if (mWindow->isClosed()) return false;
			// go into idle mode if current sample has ended
			if (mCurrentSample && mCurrentSample->isDone()) runSample(0);

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
			// manually call sample callback to ensure correct order
			if (mCurrentSample) mCurrentSample->windowResized(rw);

			const OIS::MouseState& ms = mMouse->getMouseState();
			ms.width = rw->getWidth();
			ms.height = rw->getHeight();
		}

		// window event callbacks which manually call their respective sample callbacks to ensure correct order
		virtual void windowMoved(Ogre::RenderWindow* rw) { if (mCurrentSample) mCurrentSample->windowMoved(rw); }
		virtual bool windowClosing(Ogre::RenderWindow* rw) { return mCurrentSample ? mCurrentSample->windowClosing(rw) : true; }
		virtual void windowClosed(Ogre::RenderWindow* rw) { if (mCurrentSample) mCurrentSample->windowClosed(rw); }
		virtual void windowFocusChange(Ogre::RenderWindow* rw) { if (mCurrentSample) mCurrentSample->windowFocusChange(rw); }

		// keyboard and mouse callbacks which manually call their respective sample callbacks to ensure correct order
		virtual bool keyPressed(const OIS::KeyEvent& evt) { return mCurrentSample ? mCurrentSample->keyPressed(evt) : true; }
		virtual bool keyReleased(const OIS::KeyEvent& evt) { return mCurrentSample ? mCurrentSample->keyReleased(evt) : true; }
		virtual bool mouseMoved(const OIS::MouseEvent& evt) { return mCurrentSample ? mCurrentSample->mouseMoved(evt) : true; }
		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) { return mCurrentSample ? mCurrentSample->mousePressed(evt, id) : true; }
		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id) { return mCurrentSample ? mCurrentSample->mouseReleased(evt, id) : true; }

		Ogre::Root* mRoot;              // OGRE root
		Ogre::RenderWindow* mWindow;    // render window

		OIS::InputManager* mInputMgr;   // OIS input manager
		OIS::Keyboard* mKeyboard;       // keyboard device
		OIS::Mouse* mMouse;             // mouse device

		Sample* mCurrentSample;         // currently running sample
		SampleList mSampleList;       // queued samples
	};
}

#endif
