#include "GBufferMaterialGenerator.h"

#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreTechnique.h"

class GBufferMaterialGeneratorImpl : public MaterialGenerator::Impl
{
	virtual Ogre::GpuProgramPtr generateVertexShader(MaterialGenerator::Perm permutation);
	virtual Ogre::GpuProgramPtr generateFragmentShader(MaterialGenerator::Perm permutation);
	virtual Ogre::MaterialPtr generateTemplateMaterial(MaterialGenerator::Perm permutation);
};

GBufferMaterialGenerator::GBufferMaterialGenerator() {
	mImpl = new GBufferMaterialGeneratorImpl;
	vsMask = VS_MASK;
	fsMask = FS_MASK;
	matMask = MAT_MASK;
	materialBaseName = "DeferredShading/GBuffer/";
}

Ogre::GpuProgramPtr GBufferMaterialGeneratorImpl::generateVertexShader(MaterialGenerator::Perm permutation)
{
	Ogre::StringStream ss;
	
	ss << "void DefferedVP(" << std::endl;
	ss << "	float4 iPosition : POSITION," << std::endl;
	ss << "	float3 iNormal   : NORMAL," << std::endl;

	Ogre::uint32 numTexCoords = (permutation & GBufferMaterialGenerator::GBP_TEXCOORD_MASK) >> 16;
	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	float2 iUV" << i << " : TEXCOORD" << i << ',' << std::endl;
	}

	//TODO : Skinning inputs

	ss << "	out float4 oPosition : POSITION," << std::endl;
	ss << "	out float oDepth : TEXCOORD0," << std::endl;
	ss << "	out float3 oNormal : TEXCOORD1," << std::endl;

	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	float2 oUV" << i << " : TEXCOORD" << i+2 << ',' << std::endl;
	}

	ss << "	uniform float4x4 cWorldViewProj," << std::endl;
	ss << "	uniform float4x4 cWorldView" << std::endl;

	ss << "	)" << std::endl;
	
	
	ss << "{" << std::endl;
	ss << "	oPosition = mul(cWorldViewProj, iPosition);" << std::endl;
	ss << "	oNormal = mul(cWorldView, iNormal).xyz;" << std::endl;
	ss << "	oDepth = oPosition.w;" << std::endl;

	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	oUV" << i << " = iUV" << i << ';' << std::endl;
	}

	ss << "}" << std::endl;
	
	Ogre::String programSource = ss.str();
	Ogre::String programName = "DefferedShading/GBuffer/VP_" + Ogre::StringConverter::toString(permutation);

	// Create shader object
	Ogre::HighLevelGpuProgramPtr ptrProgram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
		programName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		"cg", Ogre::GPT_VERTEX_PROGRAM);
	ptrProgram->setSource(programSource);
	ptrProgram->setParameter("entry_point","DefferedVP");
	ptrProgram->setParameter("profiles","vs_1_1 arbvp1");

	const Ogre::GpuProgramParametersSharedPtr& params = ptrProgram->getDefaultParameters();
	params->setNamedAutoConstant("cWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("cWorldView", Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
	
	return ptrProgram;
}

Ogre::GpuProgramPtr GBufferMaterialGeneratorImpl::generateFragmentShader(MaterialGenerator::Perm permutation)
{
	Ogre::StringStream ss;
	
	ss << "void DefferedFP(" << std::endl;
	ss << "	float1 iDepth : TEXCOORD0," << std::endl;
	ss << "	float3 iNormal   : TEXCOORD1," << std::endl;

	Ogre::uint32 numTexCoords = (permutation & GBufferMaterialGenerator::GBP_TEXCOORD_MASK) >> 16;
	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	float2 iUV" << i << " : TEXCOORD" << i+2 << ',' << std::endl;
	}

	ss << "	out float4 oColor0 : COLOR0," << std::endl;
	ss << "	out float4 oColor1 : COLOR1," << std::endl;

	Ogre::uint32 numTextures = permutation & GBufferMaterialGenerator::GBP_TEXTURE_MASK;
	for (Ogre::uint32 i=0; i<numTextures; i++) {
		ss << "	uniform sampler sTex" << i << " : register(s" << i << ")," << std::endl;
	}
	
	ss << "	uniform float cSpecularity" << std::endl;

	ss << "	)" << std::endl;
	
	
	ss << "{" << std::endl;

	if (numTexCoords > 0 && numTextures > 0) 
	{
		ss << "	oColor0.rgb = tex2D(sTex0, iUV0);" << std::endl;
	}
	
	ss << "	oColor1.a = cSpecularity;" << std::endl;
	ss << "	oColor1.rgb = iNormal;" << std::endl;
	ss << "	oColor1.a = iDepth;" << std::endl;

	ss << "}" << std::endl;
	
	Ogre::String programSource = ss.str();
	Ogre::String programName = "DefferedShading/GBuffer/FP_" + Ogre::StringConverter::toString(permutation);

	// Create shader object
	Ogre::HighLevelGpuProgramPtr ptrProgram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
		programName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		"cg", Ogre::GPT_FRAGMENT_PROGRAM);
	ptrProgram->setSource(programSource);
	ptrProgram->setParameter("entry_point","DefferedFP");
	ptrProgram->setParameter("profiles","ps_2_0 arbfp1");

	const Ogre::GpuProgramParametersSharedPtr& params = ptrProgram->getDefaultParameters();
	params->setNamedAutoConstant("cSpecularity", Ogre::GpuProgramParameters::ACT_SURFACE_SHININESS);
	
	return ptrProgram;
}

Ogre::MaterialPtr GBufferMaterialGeneratorImpl::generateTemplateMaterial(MaterialGenerator::Perm permutation)
{
	Ogre::String matName = "DefferedShading/GBuffer/Mat_" + Ogre::StringConverter::toString(permutation);

	Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().create
		(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
	Ogre::Pass* pass = matPtr->getTechnique(0)->getPass(0);

	Ogre::uint32 numTextures = permutation & GBufferMaterialGenerator::GBP_TEXTURE_MASK;
	for (Ogre::uint32 i=0; i<numTextures; i++)
	{
		pass->createTextureUnitState();
	}

	return matPtr;
}