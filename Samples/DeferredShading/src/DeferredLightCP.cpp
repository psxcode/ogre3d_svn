#include "DeferredLightCP.h"

#include "Ogre.h"
using namespace Ogre;

#include "LightMaterialGenerator.h"

//-----------------------------------------------------------------------
DeferredLightRenderOperation::DeferredLightRenderOperation(
	CompositorInstance* instance, const CompositionPass* pass)
{
	//Get the names of the GBuffer textures
	const CompositionPass::InputTex& input0 = pass->getInput(0);
	mTexName0 = instance->getTextureInstanceName(input0.name, input0.mrtIndex);
	const CompositionPass::InputTex& input1 = pass->getInput(1);
	mTexName1 = instance->getTextureInstanceName(input1.name, input1.mrtIndex);

	// Create lights material generator
	mLightMaterialGenerator = new LightMaterialGenerator();
	
	// Create the ambient light
	mAmbientLight = new AmbientLight();
	const MaterialPtr& mat = mAmbientLight->getMaterial();
	mat->load();
	for(unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(mTexName0);
		pass->getTextureUnitState(1)->setTextureName(mTexName1);
	}
}
//-----------------------------------------------------------------------
DLight* DeferredLightRenderOperation::createDLight(Ogre::Light* light)
{
	DLight *rv = new DLight(mLightMaterialGenerator,light);
	mLights[light] = rv;

	const MaterialPtr& mat = rv->getMaterial();
	mat->load();
	for(unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(mTexName0);
		pass->getTextureUnitState(1)->setTextureName(mTexName1);
	}
	mat->compile();
	return rv;
}
//-----------------------------------------------------------------------
void injectTechnique(SceneManager* sm, Technique* tech, Renderable* rend, const Vector3& farCorner,
					 Ogre::Light* light)
{
    for(unsigned short i=0; i<tech->getNumPasses(); ++i)
	{
        Ogre::Pass* pass = tech->getPass(i);
        // get the vertex shader parameters
        Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
        // set the camera's far-top-right corner
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);
        
        params = pass->getFragmentProgramParameters();
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);

		if (light != 0) 
		{
			Ogre::LightList list;
			list.push_back(light);
			sm->_injectRenderWithPass(pass, rend, false, false, &list);
		} 
		else
		{
			sm->_injectRenderWithPass(pass, rend, false);
		}
		
	}
}
void DeferredLightRenderOperation::execute(SceneManager *sm, RenderSystem *rs)
{
    // calculate the far-top-right corner in view-space
    Ogre::Camera* cam = sm->getCurrentViewport()->getCamera();
    Ogre::Vector3 farCorner = cam->getViewMatrix(true) * cam->getWorldSpaceCorners()[4];

    Technique* tech = mAmbientLight->getMaterial()->getBestTechnique();
	injectTechnique(sm, tech, mAmbientLight, farCorner, 0);

	const LightList& lightList = sm->_getLightsAffectingFrustum();
    for (LightList::const_iterator it = lightList.begin(); it != lightList.end(); it++) 
	{
        Light* light = *it;
		//HACK
		//if (light->getType() != Ogre::Light::LT_SPOTLIGHT)
		//	continue;

		LightsMap::iterator dLightIt = mLights.find(light);
		DLight* dLight = 0;
		if (dLightIt == mLights.end()) 
		{
			dLight = createDLight(light);
		}
		else 
		{
			dLight = dLightIt->second;
			dLight->updateFromParent();
		}
		
		tech = dLight->getMaterial()->getBestTechnique();
        injectTechnique(sm, tech, dLight, farCorner, light);
	}
}
//-----------------------------------------------------------------------
DeferredLightRenderOperation::~DeferredLightRenderOperation()
{
	for (LightsMap::iterator it = mLights.begin(); it != mLights.end(); ++it)
	{
		delete it->second;
	}
	mLights.clear();
	
	delete mAmbientLight;
	delete mLightMaterialGenerator;
}
//-----------------------------------------------------------------------
