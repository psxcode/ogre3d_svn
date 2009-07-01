#ifndef __SampleBrowser_H__
#define __SampleBrowser_H__

#include "SampleContext.h"
#include "SamplePlugin.h"

namespace OgreBites
{
	/*=============================================================================
	| The OGRE Sample Browser. Features a menu accessible from all samples,
	| dynamic configuration, resource reloading, node labelling, and more.
	=============================================================================*/
	class SampleBrowser : public OgreBites::SampleContext
	{
	public:

		/*-----------------------------------------------------------------------------
		| Extends runSample to handle creation and destruction of dummy scene.
		-----------------------------------------------------------------------------*/
		virtual void runSample(Sample* s)
		{
			if (mCurrentSample)    // create dummy scene after quitting a sample
			{
				mCurrentSample->_shutdown();
				mCurrentSample = 0;
				createDummyScene();
			}

			if (s)       // destroy dummy scene before starting a sample
			{
				destroyDummyScene();
				SampleContext::runSample(s);
			}
		}

	protected:

		/*-----------------------------------------------------------------------------
		| Restores config instead of using a dialog to save time. For the dialog,
		| use the OGRE Sample Browser Configurator utility instead.
		-----------------------------------------------------------------------------*/
		virtual bool oneTimeConfig()
		{
			return mRoot->restoreConfig();
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
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Browser");
			Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Common");
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
			mWindow->addViewport(mRoot->createSceneManager(Ogre::ST_GENERIC, "Dummy Scene")->createCamera("Dummy Camera"));
		}

		/*-----------------------------------------------------------------------------
		| Creates the main browser menu.
		-----------------------------------------------------------------------------*/
		virtual void createMenu()
		{
		}		

		/*-----------------------------------------------------------------------------
		| Extends shutdown to include browser-specific procedures.
		-----------------------------------------------------------------------------*/
		virtual void shutdown()
		{
			destroyMenu();
			destroyDummyScene();
			SampleContext::shutdown();
		}

		/*-----------------------------------------------------------------------------
		| Destroys dummy scene.
		-----------------------------------------------------------------------------*/
		virtual void destroyDummyScene()
		{
			mWindow->removeAllViewports();
			mRoot->destroySceneManager(mRoot->getSceneManager("Dummy Scene"));
		}	

		/*-----------------------------------------------------------------------------
		| Destroys the main browser menu.
		-----------------------------------------------------------------------------*/
		virtual void destroyMenu()
		{
		}	

		SampleSet mLoadedSamples;
		std::set<Ogre::String> mSampleCategories;
	};
}

#endif
