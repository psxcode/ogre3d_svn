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

const char* PASS_THROUGH_GLSL_VERTEX_PROGRAM = "												\n\
	void main()																					\n\
	{																							\n\
		//Transform the vertex (ModelViewProj matrix)											\n\
		gl_Position = ftransform();																\n\
	}\n";

const char* SWIZZLE_GLSL_GEOMETRY_PROGRAM = "													\n\
	#version 120																				\n\
	#extension GL_EXT_geometry_shader4 : enable													\n\
																								\n\
	uniform vec4 origColor;																		\n\
	uniform vec4 cloneColor;																	\n\
																								\n\
	void main(void)																				\n\
	{																							\n\
																								\n\
		//increment variable																	\n\
		int i;																					\n\
																								\n\
		/////////////////////////////////////////////////////////////							\n\
		//This example has two parts															\n\
		//	step a) draw the primitive pushed down the pipeline									\n\
		//		 there are gl_Vertices # of vertices											\n\
		//		 put the vertex value into gl_Position											\n\
		//		 use EmitVertex => 'create' a new vertex										\n\
		// 		use EndPrimitive to signal that you are done creating a primitive!				\n\
		//	step b) create a new piece of geometry (I.E. WHY WE ARE USING A GEOMETRY SHADER!)	\n\
		//		I just do the same loop, but swizzle the x and y values							\n\
		//	result => the line we want to draw, and the same line, but along the other axis		\n\
																								\n\
		//Pass-thru!																			\n\
		for(i=0; i< gl_VerticesIn; i++){														\n\
			gl_Position = gl_PositionIn[i];														\n\
			gl_FrontColor = origColor;															\n\
			EmitVertex();																		\n\
		}																						\n\
		EndPrimitive();																			\n\
		//New piece of geometry!  We just swizzle the x and y terms								\n\
		for(i=0; i< gl_VerticesIn; i++){														\n\
			gl_Position = gl_PositionIn[i];														\n\
			gl_Position.xy = gl_Position.yx;													\n\
			gl_FrontColor = cloneColor;															\n\
			EmitVertex();																		\n\
		}																						\n\
		EndPrimitive();																			\n\
																								\n\
																								\n\
	}\n";

#include "ExampleApplication.h"

class GeometryShadingApplication : public ExampleApplication
{
public:
    GeometryShadingApplication() { 
    }

    ~GeometryShadingApplication() {  }

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
			"Num output vertices per geometry shader run : " << maxOutputVertices << "\n";

        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");

        mCamera->setPosition(20, 0, 100);
        mCamera->lookAt(0,0,0);


		MaterialPtr newMaterial = MaterialManager::getSingleton().create(
			"SampleGeometryShaderMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		
		Pass* swizzlePass = newMaterial->getTechnique(0)->getPass(0);
		swizzlePass->setName("GeometryProgramPass");

		HighLevelGpuProgramPtr vp = 
			HighLevelGpuProgramManager::getSingleton().createProgram(
			"PassThroughVertexShaderProgram", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"glsl", GPT_VERTEX_PROGRAM);
		vp->setSource(PASS_THROUGH_GLSL_VERTEX_PROGRAM);
		vp->load();
		swizzlePass->setVertexProgram(vp->getName());
		
		vp = HighLevelGpuProgramManager::getSingleton().createProgram(
			"SwizzleGeometryShaderProgram", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"glsl", GPT_GEOMETRY_PROGRAM);
		vp->setSource(SWIZZLE_GLSL_GEOMETRY_PROGRAM);
		vp->load();
		vp->setInputOperationType(RenderOperation::OT_TRIANGLE_LIST);
		vp->setOutputOperationType(RenderOperation::OT_TRIANGLE_LIST);
		vp->setMaxOutputVertices(6); //3 vertices per triangle, two triangles per input triangle
		
		swizzlePass->setGeometryProgram(vp->getName());

		GpuProgramParametersSharedPtr geomParams = swizzlePass->getGeometryProgramParameters();
		geomParams->setNamedConstant("origColor", ColourValue::Blue);
		geomParams->setNamedConstant("cloneColor", ColourValue::Red);
		
		// Set all of the material's sub entities to use the new material
		for (unsigned int i=0; i<ent->getNumSubEntities(); i++)
		{
			ent->getSubEntity(i)->setMaterialName(newMaterial->getName());
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
