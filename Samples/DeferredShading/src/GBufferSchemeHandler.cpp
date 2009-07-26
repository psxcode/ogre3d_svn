#include "GBufferSchemeHandler.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

using namespace Ogre;

const String GBufferSchemeHandler::NORMAL_MAP_PATTERN = "normal";

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

bool GBufferSchemeHandler::checkNormalMap(
	TextureUnitState* tus, GBufferSchemeHandler::MaterialProperties& props)
{
	bool isNormal = false;
	if (Ogre::StringUtil::match(tus->getTextureNameAlias(), NORMAL_MAP_PATTERN, false))
	{
		isNormal = true;
	}
	else if (Ogre::StringUtil::match(tus->getTextureName(), NORMAL_MAP_PATTERN, false))
	{
		isNormal = true;
	}

	if (isNormal)
	{
		if (props.normalMap == 0)
		{
			props.normalMap = tus;
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
				"Multiple normal map patterns matches",
				"GBufferSchemeHandler::inspectMaterial");
		}
	}
	return isNormal;
}

GBufferSchemeHandler::MaterialProperties GBufferSchemeHandler::inspectMaterial(
	Material* material, unsigned short lodIndex, const Renderable* rend)
{
	MaterialProperties props;

	Ogre::MaterialManager& matMgr = Ogre::MaterialManager::getSingleton();
	String curSchemeName = matMgr.getActiveScheme();
	matMgr.setActiveScheme(MaterialManager::DEFAULT_SCHEME_NAME);
	Technique* originalTechnique = material->getBestTechnique(lodIndex, rend);
	matMgr.setActiveScheme(curSchemeName);

	if (originalTechnique->getNumPasses() != 1) {
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
			"Can not generate G-Buffer materials for multi-pass objects",
			"GBufferSchemeHandler::inspectMaterial");
	}

	Pass* pass = originalTechnique->getPass(0);
	props.vertexColourType = pass->getVertexColourTracking();
	
	//TODO : Use renderable to indicate wether this has skinning.
	//Probably use same const cast that renderSingleObject uses.
	if (pass->hasVertexProgram())
	{
		props.isSkinned = pass->getVertexProgram()->isSkeletalAnimationIncluded();
	}
	else 
	{
		props.isSkinned = false;
	}

	for (unsigned short i=0; i<pass->getNumTextureUnitStates(); i++) 
	{
		TextureUnitState* tus = pass->getTextureUnitState(i);
		if (!checkNormalMap(tus, props))
		{
			props.regularTextures.push_back(tus);
		}
		
	}

	return props;
}

MaterialGenerator::Perm GBufferSchemeHandler::getPermutation(const MaterialProperties& props)
{
	MaterialGenerator::Perm perm = 0;
	switch (props.regularTextures.size())
	{
	case 0:
		//No texture, use vertex colors
		perm |= GBufferMaterialGenerator::GBP_NO_TEXTURES;
		//TODO

		if (props.normalMap != 0)
		{
			perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		}
		else
		{
			perm |= GBufferMaterialGenerator::GBP_NO_TEXCOORDS;
		}
		break;
	case 1:
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXTURE;
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		break;
	case 2:
		perm |= GBufferMaterialGenerator::GBP_TWO_TEXTURES;
		//TODO : When do we use two texcoords?
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		//perm |= GBufferMaterialGenerator::GBP_TWO_TEXCOORDS;
		break;
	case 3:
		perm |= GBufferMaterialGenerator::GBP_THREE_TEXTURES;
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		//perm |= GBufferMaterialGenerator::GBP_THREE_TEXCOORDS;
		break;
	default:
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
			"Can not generate G-Buffer materials for '>3 regular-texture' objects",
			"GBufferSchemeHandler::inspectMaterial");
	}

	if (props.isSkinned)
	{
		perm |= GBufferMaterialGenerator::GBP_SKINNED;
	}

	if (props.normalMap != 0)
	{
		perm |= GBufferMaterialGenerator::GBP_NORMAL_MAP;
	}

	return perm;
}

void GBufferSchemeHandler::fillPass(
	Pass* gBufferPass, Pass* originalPass, const MaterialProperties& props)
{

}



