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
Filename:    ParticleGS.cpp
Description: Demonstrates the use of the geometry shader and render to vertex
	buffer to create a particle system that is entirely calculated on the GPU.
	Partial implementation of ParticlesGS example from Microsoft's DirectX 10
	SDK : http://msdn.microsoft.com/en-us/library/bb205329(VS.85).aspx
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"
#include "ProceduralManualObject.h"
#include "OgreRenderToVertexBufferManager.h"

class ParticleGSApplication : public ExampleApplication
{
public:
    ParticleGSApplication() { 
    }

    ~ParticleGSApplication() {  }
protected:

	ProceduralManualObject* createProceduralParticleSystem()
	{
		ProceduralManualObject* particleSystem = static_cast<ProceduralManualObject*>
			(mSceneMgr->createMovableObject("ParticleGSEntity", ProceduralManualObjectFactory::FACTORY_TYPE_NAME));
		particleSystem->setMaterial("Ogre/ParticleGS/Display");

		//Generate the geometry that will seed the particle system
		ManualObject* particleSystemSeed = mSceneMgr->createManualObject("ParticleSeed");
		particleSystemSeed->begin("");
		particleSystemSeed->position(10,10,1);
		particleSystemSeed->position(10,20,1);
		particleSystemSeed->position(20,20,1);
		particleSystemSeed->position(20,10,1);
		particleSystemSeed->quad(0,1,2,3);
		particleSystemSeed->end();
		
		//Generate the RenderToBufferObject
		RenderToVertexBufferObjectSharedPtr r2vbObject = 
			RenderToVertexBufferManager::getSingleton().createObject();
		//r2vbObject->setRenderToBufferMaterialName("Ogre/ParticleGS/Generate");
		r2vbObject->setRenderToBufferMaterialName("Ogre/ParticleGS/Generate");
		r2vbObject->setOperationType(RenderOperation::OT_TRIANGLE_LIST);
		r2vbObject->setMaxVertexCount(1000);
		r2vbObject->setResetsEveryUpdate(false);
		VertexDeclaration* vertexDecl = r2vbObject->getVertexDeclaration();
		size_t offset = 0;
		offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
		offset += vertexDecl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE).getSize();


		//Bond the two together
		particleSystem->setRenderToVertexBufferObject(r2vbObject);
		particleSystem->setManualObject(particleSystemSeed);

		r2vbObject->update(mSceneMgr);
		return particleSystem;
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
                "ParticleGSApplication::createScene");
        }
		if (!caps->hasCapability(RSC_HWRENDER_TO_VERTEX_BUFFER))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support render to vertex buffer, "
				"so cannot run this demo. Sorry!", 
                "ParticleGSApplication::createScene");
        }

		Root::getSingleton().addMovableObjectFactory(new ProceduralManualObjectFactory);

		ProceduralManualObject* particleSystem = createProceduralParticleSystem();

		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(particleSystem);

		mCamera->setPosition(0,0,-20);
		mCamera->lookAt(0,0,0);
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
    ParticleGSApplication app;

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
