#ifndef __CubeMapping_H__
#define __CubeMapping_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class CubeMappingSample : public SdkSample
{
public:

	CubeMappingSample()
	{
		mInfo["Title"] = "Cube Mapping";
		mInfo["Description"] = "Demonstrates the cube mapping feature where a wrap-around environment is reflected "
			"off of an object. We also apply Perlin noise to the surface, because we can.";
		mInfo["Thumbnail"] = "thumb_cubemap.png";
		mInfo["Category"] = "Unsorted";
	}

    bool frameRenderingQueued(const FrameEvent& evt)
    {
		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent updates!
    }

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
        if (!caps->hasCapability(RSC_CUBEMAPPING))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support cube mapping, "
				"so you cannot run this sample. Sorry!", "CubeMappingSample::testCapabilities");
        }
	}

protected:

	void setupContent()
	{
	}

	void cleanupContent()
	{
	}
};

#endif
