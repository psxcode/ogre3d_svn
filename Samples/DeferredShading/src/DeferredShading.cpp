/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "DeferredShading.h"

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreMaterialManager.h"

#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreRoot.h"

#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositorChain.h"
#include "OgreCompositorInstance.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"

#include "MLight.h"
#include "LightMaterialGenerator.h"

#include "AmbientLight.h"

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "OgreLogManager.h"

using namespace Ogre;

DeferredShadingSystem::DeferredShadingSystem(
		Viewport *vp, SceneManager *sm,  Camera *cam
	):
	mSceneMgr(sm), mViewport(vp), mCamera(cam),
		mLightMaterialGenerator(0)
{
	
}

void DeferredShadingSystem::initialize()
{
	for(int i=0; i<DSM_COUNT; ++i)
		mInstance[i]=0;

	createResources();
	createAmbientLight();

	mActive = false;
	
	mSSAO = false;
	mCurrentMode = DSM_SHOWLIT;
	setActive(true);
}

DeferredShadingSystem::~DeferredShadingSystem()
{
	// Delete mini lights
	for(set<MLight*>::type::iterator i=mLights.begin(); i!=mLights.end(); ++i)
	{
		delete (*i);
	}
	// Delete the ambient light
	delete mAmbientLight;

	CompositorChain *chain = CompositorManager::getSingleton().getCompositorChain(mViewport);
	for(int i=0; i<DSM_COUNT; ++i)
		chain->_removeInstance(mInstance[i]);

	delete mLightMaterialGenerator;
}

void DeferredShadingSystem::setMode(DSMode mode)
{
	assert( 0 <= mode && mode < DSM_COUNT);

	// prevent duplicate setups
	if (mCurrentMode == mode && mInstance[mode]->getEnabled()==mActive)
		return;

	for(int i=0; i<DSM_COUNT; ++i)
	{
		if(i == mode)
		{
			mInstance[i]->setEnabled(mActive);
		}
		else
		{
			mInstance[i]->setEnabled(false);
		}
	}

	mCurrentMode = mode;

	mSSAOInstance->setEnabled(mActive && mSSAO && mCurrentMode == DSM_SHOWLIT);
}

void DeferredShadingSystem::setSSAO(bool ssao)
{
	if (ssao != mSSAO) 
	{
		mSSAO = ssao;
		if (mActive && mCurrentMode == DSM_SHOWLIT)
		{
			mSSAOInstance->setEnabled(ssao);
		}
	}
}
	
bool DeferredShadingSystem::getSSAO() const
{
	return mSSAO;
}
void DeferredShadingSystem::setActive(bool active)
{
	if (mActive != active)
	{
		mActive = active;
		mGBufferInstance->setEnabled(active);

		// mCurrentMode could have changed with a prior call to setMode, so iterate all
		setMode(mCurrentMode);
	}
}

DeferredShadingSystem::DSMode DeferredShadingSystem::getMode(void) const
{
	return mCurrentMode;
}

MLight *DeferredShadingSystem::createMLight()
{
	MLight *rv = new MLight(mLightMaterialGenerator);
	mLights.insert(rv);

	if (!mTexName0.empty()) {
		setupMaterial(rv->getMaterial(), mTexName0, mTexName1);
	}

	return rv;
}

void DeferredShadingSystem::destroyMLight(MLight *m)
{
	mLights.erase(m);
	delete m;
}

void DeferredShadingSystem::createResources(void)
{
	CompositorManager &compMan = CompositorManager::getSingleton();

	// Create lights material generator
	if(Root::getSingleton().getRenderSystem()->getName()=="OpenGL Rendering Subsystem")
		mLightMaterialGenerator = new LightMaterialGenerator("glsl");
	else
		mLightMaterialGenerator = new LightMaterialGenerator("hlsl");

	// Create the main GBuffer compositor
	mGBufferInstance = compMan.addCompositor(mViewport, "DeferredShading/GBuffer");
	
	// Create filters
	mInstance[DSM_SHOWLIT] = compMan.addCompositor(mViewport, "DeferredShading/ShowLit");
	mInstance[DSM_SHOWNORMALS] = compMan.addCompositor(mViewport, "DeferredShading/ShowNormals");
	mInstance[DSM_SHOWDSP] = compMan.addCompositor(mViewport, "DeferredShading/ShowDepthSpecular");
	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");

	mInstance[DSM_SHOWCOLOUR] = compMan.addCompositor(mViewport, "DeferredShading/ShowColour");
	mSSAOInstance =  compMan.addCompositor(mViewport, "DeferredShading/SSAO");
}

void DeferredShadingSystem::setupLightMaterials()
{
	for(LightList::iterator it = mLights.begin(); it != mLights.end(); ++it)
	{
		setupMaterial((*it)->getMaterial(), mTexName0, mTexName1);
	}	
}

void DeferredShadingSystem::setupMaterial(const MaterialPtr &mat
										  , const String& texName0
										  , const String& texName1)
{
	for(unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(texName0);
		pass->getTextureUnitState(1)->setTextureName(texName1);
	}
}

void DeferredShadingSystem::createAmbientLight(void)
{
	mAmbientLight = new AmbientLight;
	mSceneMgr->getRootSceneNode()->attachObject(mAmbientLight);
}

void DeferredShadingSystem::setUpAmbientLightMaterial(void)
{
	setupMaterial(mAmbientLight->getMaterial(), mTexName0, mTexName1);
}

void DeferredShadingSystem::logCurrentMode(void)
{
	if (mActive==false)
	{
		LogManager::getSingleton().logMessage("No Compositor Enabled!");
		return;
	}

	CompositorInstance* ci = mInstance[mCurrentMode];
	assert(ci->getEnabled()==true);

	LogManager::getSingleton().logMessage("Current Mode: ");
	LogManager::getSingleton().logMessage(ci->getCompositor()->getName());
		
	if (mCurrentMode==DSM_SHOWLIT)
	{			
		LogManager::getSingleton().logMessage("Current mrt outputs are:");
		LogManager::getSingleton().logMessage(mTexName0);
		LogManager::getSingleton().logMessage(mTexName1);
	}
}

void DeferredShadingSystem::setLightTextures(const Ogre::String& texName0, const Ogre::String& texName1)
{
	mTexName0 = texName0;
	mTexName1 = texName1;
	setupLightMaterials();
	setUpAmbientLightMaterial();
}