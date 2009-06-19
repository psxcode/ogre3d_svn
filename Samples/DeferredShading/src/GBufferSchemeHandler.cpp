#include "GBufferSchemeHandler.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

using namespace Ogre;

Technique* GBufferSchemeHandler::handleSchemeNotFound(unsigned short schemeIndex, 
		const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
		const Renderable* rend)
{
	String deferredMaterialName = "DeferredDemo/" + originalMaterial->getName();
	MaterialManager& matMgr = MaterialManager::getSingleton();
	const MaterialPtr& deferredMaterial = 
		matMgr.getByName(deferredMaterialName);
	
	Technique* newTechnique = originalMaterial->createTechnique();
		
	if (!deferredMaterial.isNull()) 
	{
		deferredMaterial->load();
		String curSchemeName = schemeName;
		matMgr.setActiveScheme(MaterialManager::DEFAULT_SCHEME_NAME);
		Technique* gBufferTechnique = deferredMaterial->getBestTechnique(lodIndex, rend);
		matMgr.setActiveScheme(curSchemeName);
		//Copy technique details from deferred material
		*newTechnique = *gBufferTechnique;
	}
	else 
	{
		//This material doesn't have a GBuffer alternative, so don't render it in this scheme
		newTechnique->removeAllPasses();
	}

	newTechnique->setSchemeName(schemeName);

	return newTechnique;
}