#include "DeferredLightCP.h"

#include "Ogre.h"
using namespace Ogre;

#include "LightMaterialGenerator.h"

//-----------------------------------------------------------------------
DeferredLightRenderOperation::DeferredLightRenderOperation(
	CompositorInstance* instance, const CompositionPass* pass)
{
	mViewport = instance->getChain()->getViewport();
	
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
void injectTechnique(SceneManager* sm, Technique* tech, Renderable* rend, Ogre::Light* light)
{
    for(unsigned short i=0; i<tech->getNumPasses(); ++i)
	{
		Ogre::Pass* pass = tech->getPass(i);
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
//-----------------------------------------------------------------------
void DeferredLightRenderOperation::execute(SceneManager *sm, RenderSystem *rs)
{
    Ogre::Camera* cam = mViewport->getCamera();

	mAmbientLight->updateFromCamera(cam);
    Technique* tech = mAmbientLight->getMaterial()->getBestTechnique();
	injectTechnique(sm, tech, mAmbientLight, 0);

	int i=0;
	const LightList& lightList = sm->_getLightsAffectingFrustum();
    for (LightList::const_iterator it = lightList.begin(); it != lightList.end(); it++) 
	{
        Light* light = *it;
		
		//if (++i != 2) continue;
        //if (light->getType() != Light::LT_SPOTLIGHT) continue;
		//if (light->getDiffuseColour() != ColourValue::Red) continue;

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
		dLight->updateFromCamera(cam);
		tech = dLight->getMaterial()->getBestTechnique();

		//Update shadow texture
		bool castShadows = sm->isShadowTechniqueInUse() && light->getCastShadows() && 
			light->getType() == Light::LT_SPOTLIGHT;
		if (castShadows)
		{
			SceneManager::RenderContext* context = sm->_pauseRendering();
			sm->_prepareShadowTexturesPerLight(cam, mViewport, light);
			sm->_resumeRendering(context);
			
			//TODO : Organise this code?
			Pass* pass = tech->getPass(0);
			TextureUnitState* tus = pass->getTextureUnitState("ShadowMap");
			if (tus == 0)
			{
				tus = pass->createTextureUnitState();
				tus->setContentType(TextureUnitState::CONTENT_SHADOW);
				tus->setName("ShadowMap");
			}
			const String& shadowTexName = sm->getShadowTexture(0)->getName();
			if (tus->getTextureName() != shadowTexName)
			{
				tus->_setTexturePtr(sm->getShadowTexture(0));
			}
			
		}
		
		
        injectTechnique(sm, tech, dLight, light);
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
