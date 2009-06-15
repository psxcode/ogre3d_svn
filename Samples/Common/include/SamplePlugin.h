#ifndef __SamplePlugin_H__
#define __SamplePlugin_H__

#include "Sample.h"

namespace OgreBites
{
	/*=============================================================================
	| Utility class used to hold a queue of samples in an OGRE plugin.
	=============================================================================*/
	class SamplePlugin : public Ogre::Plugin
    {
    public:

		/*-----------------------------------------------------------------------------
		| Must be implemented to provide a name.
		-----------------------------------------------------------------------------*/
		virtual const String& getName() const = 0;
		
		virtual void install() 
		{
			// create and add your samples here
		}

		virtual void uninstall()
		{
			// destroy your samples here
		}

		virtual void initialise() {}
		virtual void shutdown() {}

		/*-----------------------------------------------------------------------------
		| Retrieves the queue of samples.
		-----------------------------------------------------------------------------*/
		const SampleQueue& getSamples()
		{
			return mSamples;
		}

	protected:

		/*-----------------------------------------------------------------------------
		| Adds a sample to the queue.
		-----------------------------------------------------------------------------*/
		void addSample(Sample* s)
		{
			mSamples.push(s);
		}

		SampleQueue mSamples;
    };
}

#endif
