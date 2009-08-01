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

#include "LightMaterialGenerator.h"

#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"

#include "OgrePass.h"
#include "OgreTechnique.h"

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "DLight.h"

using namespace Ogre;

//CG
class LightMaterialGeneratorCG : public MaterialGenerator::Impl
{
public:
	typedef MaterialGenerator::Perm Perm;
	LightMaterialGeneratorCG(const String &baseName):
	    mBaseName(baseName) 
	{

	}
	virtual ~LightMaterialGeneratorCG()
	{

	}

	virtual GpuProgramPtr generateVertexShader(Perm permutation)
	{
        String programName = "DeferredShading/post/";

		if (permutation & LightMaterialGenerator::MI_QUAD)
		{
			programName += "vs";
		}
		else
		{
			programName += "LightMaterial_vs";
		}

		GpuProgramPtr ptr = HighLevelGpuProgramManager::getSingleton().getByName(programName);
		assert(ptr->getLanguage()=="cg");
		return ptr;
	}

	virtual GpuProgramPtr generateFragmentShader(Perm permutation)
	{
		/// Create shader
		if (mMasterSource.empty())
		{
			DataStreamPtr ptrMasterSource = ResourceGroupManager::getSingleton().openResource(
				 "DeferredShading/post/LightMaterial_ps.cg"
				, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			assert(ptrMasterSource.isNull()==false);
			mMasterSource = ptrMasterSource->getAsString();
		}

		assert(mMasterSource.empty()==false);

		// Create name
		String name = mBaseName+StringConverter::toString(permutation)+"_ps";		

		// Create shader object
		HighLevelGpuProgramPtr ptrProgram = HighLevelGpuProgramManager::getSingleton().createProgram(
			name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"cg", GPT_FRAGMENT_PROGRAM);
		ptrProgram->setSource(mMasterSource);
		ptrProgram->setParameter("entry_point","main");
	    ptrProgram->setParameter("profiles","ps_2_0 arbfp1");
		// set up the preprocessor defines
		// Important to do this before any call to get parameters, i.e. before the program gets loaded
		ptrProgram->setParameter("compile_arguments", getPPDefines(permutation));

		setUpBaseParameters(ptrProgram->getDefaultParameters());

		return GpuProgramPtr(ptrProgram);
	}

	virtual MaterialPtr generateTemplateMaterial(Perm permutation)
	{
        if(permutation & LightMaterialGenerator::MI_QUAD)
		{   
			return MaterialManager::getSingleton().getByName("DeferredShading/LightMaterialQuad");
		}
		else
		{
			return MaterialManager::getSingleton().getByName("DeferredShading/LightMaterial");
		}
	}

	protected:
		String mBaseName;
        String mMasterSource;
		// Utility method
		String getPPDefines(Perm permutation)
		{
			String strPPD;
            if (permutation & LightMaterialGenerator::MI_QUAD)
            {
                strPPD += "-DIS_QUAD ";
            }
			if (permutation & LightMaterialGenerator::MI_SPECULAR)
			{
				strPPD += "-DIS_SPECULAR ";
			}
			if (permutation & LightMaterialGenerator::MI_ATTENUATED)
			{
				strPPD += "-DIS_ATTENUATED ";
			}
			if (permutation & LightMaterialGenerator::MI_SPOTLIGHT)
			{
				strPPD += "-DIS_SPOTLIGHT";
			}
			return strPPD;
		}

		void setUpBaseParameters(const GpuProgramParametersSharedPtr& params)
		{
			assert(params.isNull()==false);

            if (params->_findNamedConstantDefinition("vpWidth"))
			{
                params->setNamedAutoConstant("vpWidth", GpuProgramParameters::ACT_VIEWPORT_WIDTH);
			}
            if (params->_findNamedConstantDefinition("vpHeight"))
			{
                params->setNamedAutoConstant("vpHeight", GpuProgramParameters::ACT_VIEWPORT_HEIGHT);
			}
			if (params->_findNamedConstantDefinition("worldView"))
			{
				params->setNamedAutoConstant("worldView", GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
			}
			if (params->_findNamedConstantDefinition("invProj"))
			{
				params->setNamedAutoConstant("invProj", GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX);
			}
            if (params->_findNamedConstantDefinition("flip"))
			{
				params->setNamedAutoConstant("flip", GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING);
			}
			if (params->_findNamedConstantDefinition("lightDiffuseColor"))
			{
				params->setNamedAutoConstant("lightDiffuseColor", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR);
			}
			if (params->_findNamedConstantDefinition("lightSpecularColor"))
			{
				params->setNamedAutoConstant("lightSpecularColor", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR);
			}			
			if(params->_findNamedConstantDefinition("lightFalloff"))
			{
				params->setNamedAutoConstant("lightFalloff", GpuProgramParameters::ACT_LIGHT_ATTENUATION);
			}
			if(params->_findNamedConstantDefinition("lightPos"))
			{
				params->setNamedAutoConstant("lightPos", GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE);
			}
			if(params->_findNamedConstantDefinition("lightDir"))
			{
				params->setNamedAutoConstant("lightDir", GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE);
			}
			if(params->_findNamedConstantDefinition("spotParams"))
			{
				params->setNamedAutoConstant("spotParams", GpuProgramParameters::ACT_SPOTLIGHT_PARAMS);
			}

		}
};

LightMaterialGenerator::LightMaterialGenerator()
{
	bitNames.push_back("Quad");		  // MI_QUAD
	bitNames.push_back("Attenuated"); // MI_ATTENUATED
	bitNames.push_back("Specular");   // MI_SPECULAR
	bitNames.push_back("Spotlight");   // MI_SPOTLIGHT

	vsMask = 0x00000001;
	fsMask = 0x0000000F;
	matMask = 0x00000001;
	
	materialBaseName = "DeferredShading/LightMaterial/";
    mImpl = new LightMaterialGeneratorCG("DeferredShading/LightMaterial/");
}

LightMaterialGenerator::~LightMaterialGenerator()
{

}