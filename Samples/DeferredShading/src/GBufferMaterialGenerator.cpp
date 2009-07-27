#include "GBufferMaterialGenerator.h"

#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreTechnique.h"

//This is the concrete implementation of the material generator.
class GBufferMaterialGeneratorImpl : public MaterialGenerator::Impl
{
public:
	GBufferMaterialGeneratorImpl(const Ogre::String& baseName) : mBaseName(baseName) {}
	
protected:
	Ogre::String mBaseName;
	virtual Ogre::GpuProgramPtr generateVertexShader(MaterialGenerator::Perm permutation);
	virtual Ogre::GpuProgramPtr generateFragmentShader(MaterialGenerator::Perm permutation);
	virtual Ogre::MaterialPtr generateTemplateMaterial(MaterialGenerator::Perm permutation);

};

GBufferMaterialGenerator::GBufferMaterialGenerator() {
	vsMask = VS_MASK;
	fsMask = FS_MASK;
	matMask = MAT_MASK;
	materialBaseName = "DeferredShading/GBuffer/";
	mImpl = new GBufferMaterialGeneratorImpl(materialBaseName);
}

Ogre::GpuProgramPtr GBufferMaterialGeneratorImpl::generateVertexShader(MaterialGenerator::Perm permutation)
{
	Ogre::StringStream ss;
	
	ss << "void DeferredVP(" << std::endl;
	ss << "	float4 iPosition : POSITION," << std::endl;
	ss << "	float3 iNormal   : NORMAL," << std::endl;

	Ogre::uint32 numTexCoords = (permutation & GBufferMaterialGenerator::GBP_TEXCOORD_MASK) >> 8;
	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	float2 iUV" << i << " : TEXCOORD" << i << ',' << std::endl;
	}

	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP)
	{
		ss << "	float3 iTangent : TANGENT0," << std::endl;
	}

	//TODO : Skinning inputs
	ss << std::endl;
	


	ss << "	out float4 oPosition : POSITION," << std::endl;
	ss << "	out float oDepth : TEXCOORD0," << std::endl;
	ss << "	out float3 oNormal : TEXCOORD1," << std::endl;
	int texCoordNum = 2;
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP) {
		ss << "	out float3 oTangent : TEXCOORD" << texCoordNum++ << ',' << std::endl;
		ss << "	out float3 oBiNormal : TEXCOORD" << texCoordNum++ << ',' << std::endl;
	}
	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	out float2 oUV" << i << " : TEXCOORD" << texCoordNum++ << ',' << std::endl;
	}

	ss << std::endl;

	ss << "	uniform float4x4 cWorldViewProj," << std::endl;
	ss << "	uniform float4x4 cWorldView" << std::endl;

	ss << "	)" << std::endl;
	
	
	ss << "{" << std::endl;
	ss << "	oPosition = mul(cWorldViewProj, iPosition);" << std::endl;
	ss << "	oNormal = mul(cWorldView, float4(iNormal,0)).xyz;" << std::endl;
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP)
	{
		ss << "	oTangent = mul(cWorldView, float4(iTangent,0)).xyz;" << std::endl;
		ss << "	oBiNormal = cross(oNormal, oTangent);" << std::endl;
	}
	ss << "	oDepth = oPosition.w;" << std::endl;

	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	oUV" << i << " = iUV" << i << ';' << std::endl;
	}

	ss << "}" << std::endl;
	
	Ogre::String programSource = ss.str();
	Ogre::String programName = mBaseName + "VP_" + Ogre::StringConverter::toString(permutation);

	//Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(programSource);

	// Create shader object
	Ogre::HighLevelGpuProgramPtr ptrProgram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
		programName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		"cg", Ogre::GPT_VERTEX_PROGRAM);
	ptrProgram->setSource(programSource);
	ptrProgram->setParameter("entry_point","DeferredVP");
	ptrProgram->setParameter("profiles","vs_1_1 arbvp1");

	const Ogre::GpuProgramParametersSharedPtr& params = ptrProgram->getDefaultParameters();
	params->setNamedAutoConstant("cWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("cWorldView", Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
	ptrProgram->load();

	return ptrProgram;
}

Ogre::GpuProgramPtr GBufferMaterialGeneratorImpl::generateFragmentShader(MaterialGenerator::Perm permutation)
{
	Ogre::StringStream ss;
	
	ss << "void DeferredFP(" << std::endl;
	ss << "	float1 iDepth : TEXCOORD0," << std::endl;
	ss << "	float3 iNormal   : TEXCOORD1," << std::endl;

	int texCoordNum = 2;
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP) {
		ss << "	float3 iTangent : TEXCOORD" << texCoordNum++ << ',' << std::endl;
		ss << "	float3 iBiNormal : TEXCOORD" << texCoordNum++ << ',' << std::endl;
	}

	Ogre::uint32 numTexCoords = (permutation & GBufferMaterialGenerator::GBP_TEXCOORD_MASK) >> 8;
	for (Ogre::uint32 i=0; i<numTexCoords; i++) {
		ss << "	float2 iUV" << i << " : TEXCOORD" << texCoordNum++ << ',' << std::endl;
	}

	ss << std::endl;

	ss << "	out float4 oColor0 : COLOR0," << std::endl;
	ss << "	out float4 oColor1 : COLOR1," << std::endl;

	ss << std::endl;

	int samplerNum = 0;
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP)
	{
		ss << "	uniform sampler sNormalMap : register(s" << samplerNum++ << ")," << std::endl;
	}
	Ogre::uint32 numTextures = permutation & GBufferMaterialGenerator::GBP_TEXTURE_MASK;
	for (Ogre::uint32 i=0; i<numTextures; i++) {
		ss << "	uniform sampler sTex" << i << " : register(s" << samplerNum++ << ")," << std::endl;
	}
	if (numTextures == 0)
	{
		ss << "	uniform float3 cDiffuseColour," << std::endl;
	}
	
	
	ss << "	uniform float cSpecularity" << std::endl;

	ss << "	)" << std::endl;
	
	
	ss << "{" << std::endl;

	if (numTexCoords > 0 && numTextures > 0) 
	{
		ss << "	oColor0.rgb = tex2D(sTex0, iUV0);" << std::endl;
	}
	else
	{
		ss << "	oColor0.rgb = cDiffuseColour;" << std::endl;
	}
	
	ss << "	oColor0.a = cSpecularity;" << std::endl;
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP) {
		ss << "	float3 texNormal = (tex2D(sNormalMap, iUV0)-0.5)*2;" << std::endl;
		ss << "	float3x3 normalRotation = float3x3(iTangent, iBiNormal, iNormal);" << std::endl;
		ss << "	oColor1.rgb = normalize(mul(texNormal, normalRotation));" << std::endl;
	} else {
		ss << "	oColor1.rgb = normalize(iNormal);" << std::endl;
	}

	ss << "	oColor1.a = iDepth;" << std::endl;
	ss << "}" << std::endl;
	
	Ogre::String programSource = ss.str();
	Ogre::String programName = mBaseName + "FP_" + Ogre::StringConverter::toString(permutation);

	//Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(programSource);

	// Create shader object
	Ogre::HighLevelGpuProgramPtr ptrProgram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
		programName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		"cg", Ogre::GPT_FRAGMENT_PROGRAM);
	ptrProgram->setSource(programSource);
	ptrProgram->setParameter("entry_point","DeferredFP");
	ptrProgram->setParameter("profiles","ps_2_0 arbfp1");

	const Ogre::GpuProgramParametersSharedPtr& params = ptrProgram->getDefaultParameters();
	params->setNamedAutoConstant("cSpecularity", Ogre::GpuProgramParameters::ACT_SURFACE_SHININESS);
	if (numTextures == 0) 
	{
		params->setNamedAutoConstant("cDiffuseColour", Ogre::GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
	}
	ptrProgram->load();
	return ptrProgram;
}

Ogre::MaterialPtr GBufferMaterialGeneratorImpl::generateTemplateMaterial(MaterialGenerator::Perm permutation)
{
	Ogre::String matName = mBaseName + "Mat_" + Ogre::StringConverter::toString(permutation);

	Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().create
		(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
	Ogre::Pass* pass = matPtr->getTechnique(0)->getPass(0);
	pass->setName(mBaseName + "Pass_" + Ogre::StringConverter::toString(permutation));
	pass->setLightingEnabled(false);
	if (permutation & GBufferMaterialGenerator::GBP_NORMAL_MAP)
	{
		pass->createTextureUnitState();
	}
	Ogre::uint32 numTextures = permutation & GBufferMaterialGenerator::GBP_TEXTURE_MASK;
	for (Ogre::uint32 i=0; i<numTextures; i++)
	{
		pass->createTextureUnitState();
	}

	return matPtr;
}