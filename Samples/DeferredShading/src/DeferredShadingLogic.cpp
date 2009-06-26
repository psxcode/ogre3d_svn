#include "DeferredShadingLogic.h"

#include <OgreCompositorInstance.h>
#include "SharedData.h"

using namespace Ogre;

class DeferredCompositionListener : public Ogre::CompositorInstance::Listener
{
public:
	DeferredCompositionListener(Ogre::CompositorInstance* instance) : mInstance(instance) {}
	
	virtual void notifyResourcesCreated(bool resizeOnly)
	{
		String mrt0 = mInstance->getTextureInstanceName("mrt_output", 0);
		String mrt1 = mInstance->getTextureInstanceName("mrt_output", 1);

		SharedData::getSingleton().iSystem->setLightTextures(mrt0, mrt1);
	}

private:
	Ogre::CompositorInstance* mInstance;
};

Ogre::CompositorInstance::Listener* DeferredShadingLogic::createListener(Ogre::CompositorInstance* instance)
{
	return new DeferredCompositionListener(instance);
}