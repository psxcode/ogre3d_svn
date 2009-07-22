#ifndef __SampleBrowser_H__
#define __SampleBrowser_H__

#include "SampleContext.h"
#include "SamplePlugin.h"
#include "SdkTrays.h"

namespace OgreBites
{
	/*=============================================================================
	| The OGRE Sample Browser. Features a menu accessible from all samples,
	| dynamic configuration, resource reloading, node labelling, and more.
	=============================================================================*/
	class SampleBrowser : public SampleContext, public SdkTrayListener
	{
	public:

		SampleBrowser()
		{
			mTrayMgr = 0;
		}

		/*-----------------------------------------------------------------------------
		| Extends runSample to handle creation and destruction of dummy scene.
		-----------------------------------------------------------------------------*/
		virtual void runSample(Sample* s)
		{
			if (mCurrentSample)    // create dummy scene after quitting a sample
			{
				mCurrentSample->_shutdown();
				mCurrentSample = 0;
				mSamplePaused = false;     // don't pause next sample

				createDummyScene();
				mTrayMgr->showBackdrop("SdkTrays/Bands");
				mTrayMgr->showAll();
				mTrayMgr->destroyWidget(mTrayMgr->getWidget("Quit"));
			}

			if (s)    // destroy dummy scene before starting a sample
			{
				mTrayMgr->createButton(TL_CENTER, "Quit", "Quit Sample");
				mTrayMgr->showBackdrop("SdkTrays/Shade");
				mTrayMgr->hideAll();
				destroyDummyScene();

				SampleContext::runSample(s);
			}
		}

		/*-----------------------------------------------------------------------------
		| Process frame events. Updates frame statistics widget set.
		-----------------------------------------------------------------------------*/
		bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			mTrayMgr->frameRenderingQueued(evt);
			return SampleContext::frameRenderingQueued(evt);
		}

		/*-----------------------------------------------------------------------------
		| Handles button widget events.
		-----------------------------------------------------------------------------*/
		virtual void buttonHit(Button* b)
		{
		}

		/*-----------------------------------------------------------------------------
		| Handles keypresses.
		-----------------------------------------------------------------------------*/
		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (evt.key == OIS::KC_ESCAPE && mCurrentSample)    // pause menu
			{
				if (mTrayMgr->isCursorVisible())
				{
					mTrayMgr->hideAll();
					unpauseCurrentSample();
				}
				else
				{
					pauseCurrentSample();
					mTrayMgr->showAll();
				}
			}

			return SampleContext::keyPressed(evt);
		}

		/*-----------------------------------------------------------------------------
		| Extends mousePressed to inject mouse press into tray manager.
		-----------------------------------------------------------------------------*/
		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (!mTrayMgr->mousePressed(evt, id)) return false;
			return SampleContext::mousePressed(evt, id);
		}

		/*-----------------------------------------------------------------------------
		| Extends mouseReleased to inject mouse release into tray manager.
		-----------------------------------------------------------------------------*/
		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (!mTrayMgr->mouseReleased(evt, id)) return false;
			return SampleContext::mouseReleased(evt, id);
		}

		/*-----------------------------------------------------------------------------
		| Extends mouseMoved to inject mouse position into tray manager.
		-----------------------------------------------------------------------------*/
		virtual bool mouseMoved(const OIS::MouseEvent& evt)
		{
			if (!mTrayMgr->mouseMoved(evt)) return false;
			return SampleContext::mouseMoved(evt);
		}

	protected:

		/*-----------------------------------------------------------------------------
		| Restores config instead of using a dialog to save time.
		| If that fails, the config dialog is shown.
		-----------------------------------------------------------------------------*/
		virtual bool oneTimeConfig()
		{
			if (!mRoot->restoreConfig()) return mRoot->showConfigDialog();
			return true;
		}		

		/*-----------------------------------------------------------------------------
		| Extends setup to create dummy scene and tray interface.
		-----------------------------------------------------------------------------*/
		virtual void setup()
		{
			SampleContext::setup();

			createDummyScene();

			mTrayMgr = new SdkTrayManager("BrowserControls", mWindow, mMouse, this);
			mTrayMgr->showBackdrop("SdkTrays/Bands");

			loadSamples();

			setupMainMenu();
		}

		/*-----------------------------------------------------------------------------
		| Overrides the default window title.
		-----------------------------------------------------------------------------*/
		virtual void createWindow()
		{
			mWindow = mRoot->initialise(true, "OGRE Sample Browser");
		}

		/*-----------------------------------------------------------------------------
		| Initialises only the browser's resources and those most commonly used
		| by samples. This way, additional special content can be initialised by
		| the samples that use them, so startup time is unaffected.
		-----------------------------------------------------------------------------*/
		virtual void loadResources()
		{
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Essential");
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Popular");
		}

		/*-----------------------------------------------------------------------------
		| Creates dummy scene to allow rendering GUI in viewport.
		-----------------------------------------------------------------------------*/
		virtual void createDummyScene()
		{
			mWindow->removeAllViewports();
			Ogre::SceneManager* sm = mRoot->createSceneManager(Ogre::ST_GENERIC, "DummyScene");
			Ogre::Camera* cam = sm->createCamera("DummyCamera");
			Ogre::Viewport* vp = mWindow->addViewport(cam);
		}

		/*-----------------------------------------------------------------------------
		| Loads sample plugins from a configuration file.
		-----------------------------------------------------------------------------*/
		virtual void loadSamples()
		{
			Ogre::ConfigFile cfg;
			cfg.load("Samples.cfg");

			Ogre::String sampleDir = cfg.getSetting("SampleFolder");        // Mac OS X just uses Resources/ directory
			Ogre::StringVector sampleList = cfg.getMultiSetting("Sample");

			#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE
			if (sampleDir.empty()) sampleDir = ".";   // user didn't specify plugins folder, try current one
			#endif

			// add slash or backslash based on platform
			char lastChar = sampleDir[sampleDir.length() - 1];
			if (lastChar != '/' && lastChar != '\\')
			{
				#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				sampleDir += "\\";
				#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
				sampleDir += "/";
				#endif
			}

			mLoadedSamples.clear();
			mSampleCategories.clear();

			// loop through all sample plugins...
			for (Ogre::StringVector::iterator i = sampleList.begin(); i != sampleList.end(); i++)
			{
				// load the plugin and acquire the SamplePlugin instance
				mRoot->loadPlugin(sampleDir + *i);
				Ogre::Plugin* p = mRoot->getInstalledPlugins().back();
				SamplePlugin* sp = dynamic_cast<SamplePlugin*>(p);

				if (!sp)  // this is not a SamplePlugin
				{
					Ogre::String desc = p->getName() + " is not a SamplePlugin!";
					Ogre::String src = "SampleBrowser::loadSamples";
					OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, desc, src); 
				}

				// go through every sample in the plugin...
				SampleSet newSamples = sp->getSamples();
				for (SampleSet::iterator j = newSamples.begin(); j != newSamples.end(); j++)
				{
					Ogre::NameValuePairList& info = (*j)->getInfo();   // acquire custom sample info
					Ogre::NameValuePairList::iterator k;

					// give sample default title and category if none found
					k= info.find("Title");
					if (k == info.end() || k->second.empty()) info["Title"] = "Untitled";
					k = info.find("Category");
					if (k == info.end() || k->second.empty()) info["Category"] = "Unsorted";

					mLoadedSamples.insert(*j);                    // add sample only after ensuring title for sorting
					mSampleCategories.insert(info["Category"]);   // add sample category
				}
			}
		}

		/*-----------------------------------------------------------------------------
		| Sets up main menu for browsing samples.
		-----------------------------------------------------------------------------*/
		virtual void setupMainMenu()
		{
			mTrayMgr->destroyAllWidgets();

			// create main navigation tray
			mTrayMgr->showLogo(TL_RIGHT);
			mTrayMgr->createSeparator(TL_RIGHT, "LogoSep");
			mTrayMgr->createButton(TL_RIGHT, "Start", "Start Sample");
			mTrayMgr->createButton(TL_RIGHT, "Refresh", "Refresh Samples");
			mTrayMgr->createButton(TL_RIGHT, "Configure", "Configure");
			mTrayMgr->createButton(TL_RIGHT, "Quit", "Quit");

			// create sample viewing controls
			mTrayMgr->createLabel(TL_LEFT, "SampleTitle", "Sample Title");
			mTrayMgr->createTextBox(TL_LEFT, "SampleInfo", "Sample Info", 250, 132);
			mTrayMgr->createThickSelectMenu(TL_LEFT, "CategoryMenu", "Select Category", 250, 10); 
			mTrayMgr->createThickSelectMenu(TL_LEFT, "SampleMenu", "Select Sample", 250, 10);
			mTrayMgr->createThickSlider(TL_LEFT, "SampleSlider", "Slide Samples", 250, 80, 0, 99, 100);
		}

		/*-----------------------------------------------------------------------------
		| Extends shutdown to destroy dummy scene and tray interface.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
			if (mTrayMgr)
			{
				delete mTrayMgr;
				mTrayMgr = 0;
			}
			if (!mCurrentSample) destroyDummyScene();
			SampleContext::shutdown();
		}

		/*-----------------------------------------------------------------------------
		| Destroys dummy scene.
		-----------------------------------------------------------------------------*/
		virtual void destroyDummyScene()
		{
			mWindow->removeAllViewports();
			mRoot->destroySceneManager(mRoot->getSceneManager("DummyScene"));
		}	

		/*-----------------------------------------------------------------------------
		| Extend to temporarily hide a sample's overlays while in the pause menu.
		-----------------------------------------------------------------------------*/
		virtual void pauseCurrentSample()
		{
			SampleContext::pauseCurrentSample();

			Ogre::OverlayManager::OverlayMapIterator it = Ogre::OverlayManager::getSingleton().getOverlayIterator();
			mHiddenOverlays.clear();

			while (it.hasMoreElements())
			{
				Ogre::Overlay* o = it.getNext();
				if (o->isVisible())                  // later, we don't want to unhide the initially hidden overlays
				{
					mHiddenOverlays.push_back(o);    // save off hidden overlays so we can unhide them later
					o->hide();
				}
			}
		}

		/*-----------------------------------------------------------------------------
		| Extend to unnhide all of sample's temporarily hidden overlays.
		-----------------------------------------------------------------------------*/
		virtual void unpauseCurrentSample()
		{
			SampleContext::unpauseCurrentSample();

			for (std::vector<Ogre::Overlay*>::iterator i = mHiddenOverlays.begin(); i != mHiddenOverlays.end(); i++)
			{
				(*i)->show();
			}

			mHiddenOverlays.clear();
		}

		SampleSet mLoadedSamples;                      // loaded samples
		std::set<Ogre::String> mSampleCategories;      // sample categories
		SdkTrayManager* mTrayMgr;                      // SDK tray interface
		std::vector<Ogre::Overlay*> mHiddenOverlays;   // sample overlays hidden for pausing
	};
}

#endif
