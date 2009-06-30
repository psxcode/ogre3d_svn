#ifndef __SamplePlugin_H__
#define __SamplePlugin_H__

#include "OgrePlugin.h"
#include "Sample.h"

namespace OgreBites
{
	/*=============================================================================
	| Utility class used to hold a queue of samples in an OGRE plugin.
	=============================================================================*/
	class SamplePlugin : public Ogre::Plugin
    {
    public:

		SamplePlugin(const Ogre::String& name)
		{
			mName = name;
		}

		const Ogre::String& getName() const
		{
			return mName;
		}
		
		void install() {}
		void uninstall() {}
		void initialise() {}
		void shutdown() {}

		/*-----------------------------------------------------------------------------
		| Adds a sample to the queue.
		-----------------------------------------------------------------------------*/
		void addSample(Sample* s)
		{
			mSamples.push_back(s);
		}

		/*-----------------------------------------------------------------------------
		| Retrieves the queue of samples.
		-----------------------------------------------------------------------------*/
		const SampleList& getSamples()
		{
			return mSamples;
		}

	protected:

		Ogre::String mName;
		SampleList mSamples;
    };
}

#endif
