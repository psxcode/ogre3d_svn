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

const char* POINT_RENDERING_GLSL_PROGRAM = "													\n\
	#version 120																				\n\
	#extension GL_EXT_geometry_shader4 : enable													\n\
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
			EmitVertex();																		\n\
		}																						\n\
		EndPrimitive();																			\n\
		//New piece of geometry!  We just swizzle the x and y terms								\n\
		for(i=0; i< gl_VerticesIn; i++){														\n\
			gl_Position = gl_PositionIn[i];														\n\
			gl_Position.xy = gl_Position.yx;													\n\
			EmitVertex();																		\n\
		}																						\n\
		EndPrimitive();																			\n\
																								\n\
																								\n\
	}\n";

#include "ExampleApplication.h"

SceneNode* rotNode;

// Listener class for frame updates
class CelShadingListener : public ExampleFrameListener
{
protected:
public:
    CelShadingListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
	if( ExampleFrameListener::frameRenderingQueued(evt) == false )
		return false;

        rotNode->yaw(Degree(evt.timeSinceLastFrame * 30));
        return true;
    }
};

// Custom parameter bindings
#define CUSTOM_SHININESS 1
#define CUSTOM_DIFFUSE 2
#define CUSTOM_SPECULAR 3

class CelShadingApplication : public ExampleApplication
{
public:
    CelShadingApplication() { 
    }

    ~CelShadingApplication() {  }

protected:
    
	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new CelShadingListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }
    

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Check capabilities
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support geometry programs, so cannot "
                "run this demo. Sorry!", 
                "CelShading::createScene");
        }

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // Add light to the scene node
        rotNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        rotNode->createChildSceneNode(Vector3(20,40,50))->attachObject(l);

        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");

        mCamera->setPosition(20, 0, 100);
        mCamera->lookAt(0,0,0);


		MaterialPtr newMaterial = MaterialManager::getSingleton().create(
			"SampleGeometryShaderMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		Pass* pointPass = newMaterial->getTechnique(0)->getPass(0);

		HighLevelGpuProgramPtr vp = 
			HighLevelGpuProgramManager::getSingleton().createProgram(
			"SampleGeometryShaderMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			"glsl", GPT_GEOMETRY_PROGRAM);
		vp->setSource(POINT_RENDERING_GLSL_PROGRAM);
		vp->load();

		pointPass->setName("GeometryProgramPass");
		pointPass->setGeometryProgram(vp->getName());
		
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
    CelShadingApplication app;

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
