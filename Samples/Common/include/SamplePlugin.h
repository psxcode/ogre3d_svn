#ifndef __SamplePlugin_H__
#define __SamplePlugin_H__

#include "OgrePlugin.h"
#include "Sample.h"

// Export macro to export the sample's main dll functions.
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(__MINGW32__)
#	define _OgreSampleExport __declspec(dllexport)
#	define _OgreSampleClassExport
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreSampleExport  __attribute__ ((visibility("default")))
#   define _OgreSampleClassExport  __attribute__ ((visibility("default")))
#else
#	define _OgreSampleExport
#	define _OgreSampleClassExport
#endif


namespace OgreBites
{
	/*=============================================================================
	| Utility class used to hold a set of samples in an OGRE plugin.
	=============================================================================*/
	class _OgreSampleClassExport SamplePlugin : public Ogre::Plugin
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
