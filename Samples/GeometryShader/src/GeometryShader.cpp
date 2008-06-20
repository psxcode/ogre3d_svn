/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    GeometryShader.cpp
Description: Demo of a Shader Model 4 geometry shader in action
-----------------------------------------------------------------------------
*/

const char* SWIZZLE_ASSEMBLY_GEOMETRY_PROGRAM = 
	"!!NVgp4.0																					\n\
	PRIMITIVE_IN TRIANGLES;																		\n\
	PRIMITIVE_OUT TRIANGLE_STRIP;																\n\
	VERTICES_OUT 6;																				\n\
	# cgc version 2.0.0015, build date May 15 2008												\n\
	# command line args: -profile gpu_gp														\n\
	# source file: gs_simple.cg																	\n\
	#vendor NVIDIA Corporation																	\n\
	#version 2.0.0.15																			\n\
	#profile gpu_gp																				\n\
	#program geometry_swizzle																	\n\
	#semantic geometry_swizzle.position : POSITION												\n\
	#var float4 position.__data[0] : $vin.VERTEX[0].POSITION : HPOS[0][32] : 0 : 1				\n\
	#var float4 position.__data[1] : $vin.VERTEX[1].POSITION : HPOS[1][32] : 0 : 1				\n\
	#var float4 position.__data[2] : $vin.VERTEX[2].POSITION : HPOS[2][32] : 0 : 1				\n\
	ATTRIB vertex_position = vertex.position;													\n\
	TEMP RC, HC;																				\n\
	MOV.F result.position, vertex[0].position;													\n\
	MOV.F result.color, {1, 0}.xyyx;															\n\
	EMIT;																						\n\
	MOV.F result.position, vertex[1].position;													\n\
	MOV.F result.color, {1, 0}.xyyx;															\n\
	EMIT;																						\n\
	MOV.F result.position, vertex[2].position;													\n\
	MOV.F result.color, {1, 0}.xyyx;															\n\
	EMIT;																						\n\
	ENDPRIM;																					\n\
	MOV.F result.position, vertex[0].position.yxzw;												\n\
	MOV.F result.color, {0, 1}.xxyy;															\n\
	EMIT;																						\n\
	MOV.F result.position, vertex[1].position.yxzw;												\n\
	MOV.F result.color, {0, 1}.xxyy;															\n\
	EMIT;																						\n\
	MOV.F result.position, vertex[2].position.yxzw;												\n\
	MOV.F result.color, {0, 1}.xxyy;															\n\
	EMIT;																						\n\
	ENDPRIM;																					\n\
	END																							\n\
	# 20 instructions, 0 R-regs																	\n";	

#include "ExampleApplication.h"

class GeometryShadingApplication : public ExampleApplication
{
public:
    GeometryShadingApplication() { 
    }

    ~GeometryShadingApplication() {  }

	MaterialPtr createGLSLProgramMaterial()
	{
		//Now using script interface for GLSL.
		return MaterialManager::getSingleton().getByName("Ogre/GPTest/SwizzleGLSL");
		//MaterialPtr newMaterial = MaterialManager::getSingleton().create(
		//	"GLSLGeometryShaderMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		//
		//Pass* swizzlePass = newMaterial->getTechnique(0)->getPass(0);
		//swizzlePass->setName("GeometryProgramPass");

		//HighLevelGpuProgramPtr vp = 
		//	HighLevelGpuProgramManager::getSingleton().createProgram(
		//	"PassThroughVertexShaderProgram", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		//	"glsl", GPT_VERTEX_PROGRAM);
		//vp->setSource(PASS_THROUGH_GLSL_VERTEX_PROGRAM);
		//vp->load();
		//swizzlePass->setVertexProgram(vp->getName());
		//
		//vp = HighLevelGpuProgramManager::getSingleton().createProgram(
		//	"GLSLSwizzleGeometryShaderProgram", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		//	"glsl", GPT_GEOMETRY_PROGRAM);
		//vp->setSource(SWIZZLE_GLSL_GEOMETRY_PROGRAM);
		//vp->load();
		//vp->setInputOperationType(RenderOperation::OT_TRIANGLE_LIST);
		//vp->setOutputOperationType(RenderOperation::OT_TRIANGLE_LIST);
		//vp->setMaxOutputVertices(6); //3 vertices per triangle, two triangles per input triangle
		//
		//swizzlePass->setGeometryProgram(vp->getName());

		//GpuProgramParametersSharedPtr geomParams = swizzlePass->getGeometryProgramParameters();
		//geomParams->setNamedConstant("origColor", ColourValue::Blue);
		//geomParams->setNamedConstant("cloneColor", ColourValue::Red);
		//return newMaterial;
	}

	MaterialPtr createASMProgramMaterial()
	{
		MaterialPtr newMaterial = MaterialManager::getSingleton().create(
			"ASMGeometryShaderMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		
		Pass* swizzlePass = newMaterial->getTechnique(0)->getPass(0);
		swizzlePass->setName("GeometryProgramPass");

		GpuProgramPtr vp = GpuProgramManager::getSingleton().createProgramFromString(
			"ASMSwizzleGeometryShaderProgram", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			SWIZZLE_ASSEMBLY_GEOMETRY_PROGRAM, GPT_GEOMETRY_PROGRAM, "nvgp4");
		vp->load();
		
		swizzlePass->setGeometryProgram(vp->getName());

		return newMaterial;
	}
protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Check capabilities
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support geometry programs, so cannot "
                "run this demo. Sorry!", 
                "GeometryShading::createScene");
        }
		
		int maxOutputVertices = caps->getGeometryProgramNumOutputVertices();
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << 
			"Num output vertices per geometry shader run : " << maxOutputVertices;

        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        mCamera->setPosition(20, 0, 100);
        mCamera->lookAt(0,0,0);
		
		MaterialPtr geomMaterial = createGLSLProgramMaterial();
		//MaterialPtr geomMaterial = createASMProgramMaterial();

		// Set all of the material's sub entities to use the new material
		for (unsigned int i=0; i<ent->getNumSubEntities(); i++)
		{
			ent->getSubEntity(i)->setMaterialName(geomMaterial->getName());
		}
        
        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        mWindow->getViewport(0)->setBackgroundColour(ColourValue::Green);
    }
};



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    GeometryShadingApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}

#ifdef __cplusplus
}
#endif
