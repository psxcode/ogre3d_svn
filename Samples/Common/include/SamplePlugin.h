#ifndef __SamplePlugin_H__
#define __SamplePlugin_H__

#include "OgrePlugin.h"
#include "Sample.h"

namespace OgreBites
{
	/*=============================================================================
	| Utility class used to hold a set of samples in an OGRE plugin.
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
			mSamples.insert(s);
		}

		/*-----------------------------------------------------------------------------
		| Retrieves the queue of samples.
		-----------------------------------------------------------------------------*/
		const SampleSet& getSamples()
		{
			return mSamples;
		}

	protected:

		Ogre::String mName;
		SampleSet mSamples;
    };
}

#endif
