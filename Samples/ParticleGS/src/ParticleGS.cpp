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
#include "RandomTools.h"

class ParticleGSApplication : public ExampleApplication
{
public:
    ParticleGSApplication() { 
    }

    ~ParticleGSApplication() {  }
protected:

	virtual void chooseSceneManager(void)
    {
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("DefaultSceneManager", "ExampleSMInstance");
    }

	ProceduralManualObject* createProceduralParticleSystem()
	{
		ProceduralManualObject* particleSystem = static_cast<ProceduralManualObject*>
			(mSceneMgr->createMovableObject("ParticleGSEntity", ProceduralManualObjectFactory::FACTORY_TYPE_NAME));
		particleSystem->setMaterial("Ogre/ParticleGS/Display");

		//Generate the geometry that will seed the particle system
		ManualObject* particleSystemSeed = mSceneMgr->createManualObject("ParticleSeed");
		//This needs to be the initial launcher particle
		particleSystemSeed->begin("Ogre/ParticleGS/Display", RenderOperation::OT_POINT_LIST);
		particleSystemSeed->position(0,0,0); //Position
		particleSystemSeed->textureCoord(0); //Timer
		particleSystemSeed->textureCoord(0); //Type
		particleSystemSeed->textureCoord(0,0,0); //Velocity
		particleSystemSeed->end();
		//Generate the RenderToBufferObject
		RenderToVertexBufferObjectSharedPtr r2vbObject = 
			RenderToVertexBufferManager::getSingleton().createObject();
		r2vbObject->setRenderToBufferMaterialName("Ogre/ParticleGS/Generate");
		
		//Apply the random texture
		TexturePtr randomTexture = RandomTools::generateRandomVelocityTexture();
		r2vbObject->getRenderToBufferMaterial()->getTechnique(0)->getPass(0)->
			getTextureUnitState("RandomTexture")->setTextureName(
			randomTexture->getName(), randomTexture->getTextureType());

		r2vbObject->setOperationType(RenderOperation::OT_POINT_LIST);
		r2vbObject->setMaxVertexCount(1000);
		r2vbObject->setResetsEveryUpdate(true);
		VertexDeclaration* vertexDecl = r2vbObject->getVertexDeclaration();
		size_t offset = 0;
		offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize(); //Position
		offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 0).getSize(); //Timer
		offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 1).getSize(); //Type
		offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_TEXTURE_COORDINATES, 2).getSize(); //Velocity
		
		//Bind the two together
		particleSystem->setRenderToVertexBufferObject(r2vbObject);
		particleSystem->setManualObject(particleSystemSeed);

		//Set bounds
		AxisAlignedBox aabb;
		aabb.setMinimum(-100,-100,-100);
		aabb.setMaximum(100,100,100);
		particleSystem->setBoundingBox(aabb);
		
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
		//mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(particleSystem->getManualObject());
		mCamera->setPosition(0,0,-20);
		mCamera->lookAt(0,1,1);
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
