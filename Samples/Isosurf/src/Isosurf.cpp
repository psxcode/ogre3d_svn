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
Filename:    IsoSurf.cpp
Description: Demonstrates the use of the geometry shader to tessellate an 
	isosurface using marching tetrahedrons. Partial implementation of cg 
	Isosurf sample from NVIDIA's OpenGL SDK 10 : 
	http://developer.download.nvidia.com/SDK/10/opengl/samples.html
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"

const String GLSL_MATERIAL_NAME = "Ogre/GPTest/SwizzleGLSL";
const String ASM_MATERIAL_NAME = "Ogre/GPTest/SwizzleASM";
const String CG_MATERIAL_NAME = "Ogre/GPTest/SwizzleCG";

class IsoSurfApplication : public ExampleApplication
{
public:
    IsoSurfApplication() { 
    }

    ~IsoSurfApplication() {  }
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
                "IsoSurfApplication::createScene");
        }
		
		int maxOutputVertices = caps->getGeometryProgramNumOutputVertices();
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << 
			"Num output vertices per geometry shader run : " << maxOutputVertices;

        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        mCamera->setPosition(20, 0, 100);
        mCamera->lookAt(0,0,0);
		
		String materialName = GLSL_MATERIAL_NAME;
		//String materialName = ASM_MATERIAL_NAME;
		//String materialName = CG_MATERIAL_NAME;

		// Set all of the material's sub entities to use the new material
		for (unsigned int i=0; i<ent->getNumSubEntities(); i++)
		{
			ent->getSubEntity(i)->setMaterialName(materialName);
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
    IsoSurfApplication app;

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
