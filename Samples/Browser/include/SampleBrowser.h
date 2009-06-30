#ifndef __SampleBrowser_H__
#define __SampleBrowser_H__

#include "SampleContext.h"
#include "SamplePlugin.h"

namespace OgreBites
{
	class SampleBrowser : public OgreBites::SampleContext
	{
	public:

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

			// loop through all sample plugins...
			for (Ogre::StringVector::iterator it = sampleList.begin(); it != sampleList.end(); it++)
			{
				// load the plugin and acquire the SamplePlugin instance
				mRoot->loadPlugin(sampleDir + *it);
				Ogre::Plugin* p = mRoot->getInstalledPlugins().back();
				SamplePlugin* sp = dynamic_cast<SamplePlugin*>(p);

				if (!sp)  // this is not a SamplePlugin
				{
					Ogre::String desc = p->getName() + " is not a SamplePlugin!";
					Ogre::String src = "SampleBrowser::loadSamples";
					OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, desc, src); 
				}

				SampleQueue sq = sp->getSamples();
				while (!sq.empty())
				{
					queueSample(sq.front());
					sq.pop();
				}
			}
		}
			
		/*-----------------------------------------------------------------------------
		| Override default frameEnded to ignore the queue.
		-----------------------------------------------------------------------------*/
		virtual bool frameEnded(const Ogre::FrameEvent& evt)
		{
			// manually call sample callback to ensure correct order
			if (mCurrentSample && !mCurrentSample->frameEnded(evt)) return false;
			// quit current sample if it has ended
			if (mCurrentSample && mCurrentSample->isDone()) runSample(0);
			// quit if window was closed
			if (mWindow->isClosed()) return false;

			return true;
		}
	};
}

#endif
