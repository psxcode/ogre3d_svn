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
			mSelectedSample = 0;
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
				mTrayMgr->showBackdrop("SdkTrays/Backdrops/Bands");
				mTrayMgr->showAll();
				mTrayMgr->destroyWidget(mTrayMgr->getWidget("Quit"));
			}

			if (s)    // destroy dummy scene before starting a sample
			{
				mTrayMgr->createButton(TL_CENTER, "Quit", "Quit Sample");
				mTrayMgr->showBackdrop("SdkTrays/Backdrops/Shade");
				mTrayMgr->hideAll();
				destroyDummyScene();
				SampleContext::runSample(s);
			}
		}

		/*-----------------------------------------------------------------------------
		| Handles button widget events.
		-----------------------------------------------------------------------------*/
		virtual void buttonPushed(Button* b)
		{
			if (Ogre::StringUtil::startsWith(b->getName(), "Sample", false))   // if this is a sample button...
			{
				if (!mSelectedSample)   // if this is the first time selecting a sample...
				{
					// create a description box and start button
					mTrayMgr->createLabel(TL_CENTER, "SampleLabel", "");
					mTrayMgr->createTextBox(TL_CENTER, "SampleDesc", "", 250, 180);
					mTrayMgr->createButton(TL_CENTER, "Start", "Run Sample");
				}

				// show sample description
				mSelectedSample = Ogre::any_cast<Sample*>(b->getOverlayElement()->getUserAny());
				((TextBox*)mTrayMgr->getWidget("SampleDesc"))->setText(mSelectedSample->getInfo()["Description"]);
				((Label*)mTrayMgr->getWidget("SampleLabel"))->setText(mSelectedSample->getInfo()["Title"]);
			}
			else if (b->getName() == "Start") runSample(mSelectedSample);  // start button pressed
			else if (b->getName() == "Exit") mRoot->queueEndRendering();   // quit button pressed
			else if (b->getName() == "Quit") runSample(0);
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
		| Extends setup to include browser-specific procedures.
		-----------------------------------------------------------------------------*/
		virtual void setup()
		{
			SampleContext::setup();
			loadSamples();
			createDummyScene();
			createMenu();
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
		| Creates dummy scene to allow rendering GUI in viewport.
		-----------------------------------------------------------------------------*/
		virtual void createDummyScene()
		{
			mWindow->removeAllViewports();
			Ogre::SceneManager* sm = mRoot->createSceneManager(Ogre::ST_GENERIC, "DummyScene");
			Ogre::Camera* cam = sm->createCamera("DummyCamera");
			Ogre::Viewport* vp = mWindow->addViewport(cam);
			vp->setBackgroundColour(Ogre::ColourValue(0.5, 0.7, 0.5));
		}

		/*-----------------------------------------------------------------------------
		| Creates the main browser menu.
		-----------------------------------------------------------------------------*/
		virtual void createMenu()
		{
			mTrayMgr = new SdkTrayManager("BrowserControls", mMouse, this);
			mTrayMgr->setTrayPadding(4);
			mTrayMgr->showBackdrop("SdkTrays/Backdrops/Bands");

			Ogre::StringVector sv;
			sv.push_back("hay");
			sv.push_back("too longggggggggggggggggg");
			sv.push_back("hay again");
			sv.push_back("omg lulz");
			sv.push_back("hay2");
			sv.push_back("hay guyz2");
			sv.push_back("hay again2");
			sv.push_back("omg lulz2");
			sv.push_back("hay3");
			sv.push_back("hay guyz3");
			sv.push_back("hay again3");
			sv.push_back("omg lulz3");

			mTrayMgr->createLabel(TL_LEFT, "SamplesLabel", "Samples");
			mTrayMgr->createSelectMenu(TL_LEFT, "SampleCategoryMenu", "Sample Category", 220, 5)->setItems(sv);
			mTrayMgr->createSeparator(TL_LEFT, "SampleSeparator");

			mTrayMgr->setTrayWidgetAlignment(TL_LEFT, Ogre::GHA_CENTER);

			int j = 0;
			for (SampleSet::iterator i = mLoadedSamples.begin(); i != mLoadedSamples.end(); i++)
			{
				Button* b = mTrayMgr->createButton(TL_LEFT, "Sample" + Ogre::StringConverter::toString(j++), (*i)->getInfo()["Title"]);
				b->getOverlayElement()->setUserAny(Ogre::Any(*i));
			}

			mTrayMgr->createButton(TL_TOPRIGHT, "Exit", "Exit", 60);
		}

		/*-----------------------------------------------------------------------------
		| Extends shutdown to include browser-specific procedures.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
			destroyMenu();
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
		| Destroys the main browser menu.
		-----------------------------------------------------------------------------*/
		virtual void destroyMenu()
		{
			if (mTrayMgr) delete mTrayMgr;
			mTrayMgr = 0;
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

		Sample* mSelectedSample;      // sample currently selected in the menu
		SampleSet mLoadedSamples;                      // loaded samples
		std::set<Ogre::String> mSampleCategories;      // sample categories
		SdkTrayManager* mTrayMgr;
		std::vector<Ogre::Overlay*> mHiddenOverlays;   // sample overlays hidden for pausing
	};
}

#endif
