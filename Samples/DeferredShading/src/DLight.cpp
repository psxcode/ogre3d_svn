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
#include "DLight.h"

#include "OgreHardwareBufferManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreLight.h"
#include "GeomUtils.h"

using namespace Ogre;
//-----------------------------------------------------------------------
DLight::DLight(MaterialGenerator *sys, Ogre::Light* parentLight):
	bIgnoreWorld(false), mGenerator(sys),mPermutation(0), mParentLight(parentLight)
{
	// Set up geometry
	// Allocate render operation
	mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = 0;
	mRenderOp.vertexData = 0;
	mRenderOp.useIndexes = true;

	updateFromParent();
}
//-----------------------------------------------------------------------
DLight::~DLight()
{
	// need to release IndexData and vertexData created for renderable
    delete mRenderOp.indexData;
    delete mRenderOp.vertexData;
}
//-----------------------------------------------------------------------
void DLight::setAttenuation(float c, float b, float a)
{
	// Set Attenuation parameter to shader
	setCustomParameter(3, Vector4(c, b, a, 0));

	/// There is attenuation? Set material accordingly
	if(c != 1.0f || b != 0.0f || a != 0.0f)
		mPermutation |= MI_ATTENUATED;
	else
		mPermutation &= ~MI_ATTENUATED;

	// Calculate radius from Attenuation
	int threshold_level = 15;// difference of 10-15 levels deemed unnoticeable
	float threshold = 1.0f/((float)threshold_level/256.0f); 

	// Use quadratic formula to determine outer radius
	c = c-threshold;
	float d=sqrt(b*b-4*a*c);
	float x=(-2*c)/(b+d);

	rebuildGeometry(x);
}
//-----------------------------------------------------------------------
void DLight::setDiffuseColour(const ColourValue &col)
{
	setCustomParameter(1, Vector4(col.r, col.g, col.b, col.a));
}
//-----------------------------------------------------------------------
void DLight::setSpecularColour(const ColourValue &col)
{
	setCustomParameter(2, Vector4(col.r, col.g, col.b, col.a));
	/// There is a specular component? Set material accordingly
	
	if(col.r != 0.0f || col.g != 0.0f || col.b != 0.0f)
		mPermutation |= MI_SPECULAR;
	else
		mPermutation &= ~MI_SPECULAR;
		
}
//-----------------------------------------------------------------------
void DLight::rebuildGeometry(float radius)
{
	// Scale node to radius
	
	if(radius > 10000.0f)
	{
		createRectangle2D();
		mPermutation |= MI_QUAD;
	}
	else
	{
		/// XXX some more intelligent expression for rings and segments
		createSphere(radius, 5, 5);
		mPermutation &= ~MI_QUAD;
	}	
}
//-----------------------------------------------------------------------
void DLight::createRectangle2D()
{
	/// XXX this RenderOp should really be re-used between DLight objects,
	/// not generated every time
	delete mRenderOp.vertexData; 
	delete mRenderOp.indexData; 

	mRenderOp.vertexData = new VertexData();
    mRenderOp.indexData = 0;

	GeomUtils::createQuad(mRenderOp.vertexData);

    mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP; 
    mRenderOp.useIndexes = false; 

	// Set bounding
    setBoundingBox(AxisAlignedBox(-10000,-10000,-10000,10000,10000,10000));
	mRadius = 15000;
	bIgnoreWorld = true;
}
//-----------------------------------------------------------------------
void DLight::createSphere(const float radius, const int nRings, const int nSegments)
{
	delete mRenderOp.vertexData; 
	delete mRenderOp.indexData;
	mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
	mRenderOp.indexData = new IndexData();
	mRenderOp.vertexData = new VertexData();
	mRenderOp.useIndexes = true;

	GeomUtils::createSphere(mRenderOp.vertexData, mRenderOp.indexData
		, radius
		, nRings, nSegments
		, false // no normals
		, false // no texture coordinates
		);

	// Set bounding box and sphere
	setBoundingBox( AxisAlignedBox( Vector3(-radius, -radius, -radius), Vector3(radius, radius, radius) ) );
	mRadius = radius;
	bIgnoreWorld = false;
}								 
//-----------------------------------------------------------------------
Real DLight::getBoundingRadius(void) const
{
	return mRadius;
}
//-----------------------------------------------------------------------
Real DLight::getSquaredViewDepth(const Camera* cam) const
{
	if(bIgnoreWorld)
	{
		return 0.0f;
	}
	else
	{
		Vector3 dist = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		return dist.squaredLength();
	}
}
//-----------------------------------------------------------------------
const MaterialPtr& DLight::getMaterial(void) const
{
	return mGenerator->getMaterial(mPermutation);
}
//-----------------------------------------------------------------------
void DLight::getWorldTransforms(Matrix4* xform) const
{
	xform->makeTransform(mParentLight->getDerivedPosition(),
		Vector3::UNIT_SCALE, Quaternion::IDENTITY);
}
//-----------------------------------------------------------------------
void DLight::updateFromParent()
{
	//TODO : Don't do this unless something changed
	setAttenuation(mParentLight->getAttenuationConstant(), 
		mParentLight->getAttenuationLinear(), mParentLight->getAttenuationQuadric());	
	setDiffuseColour(mParentLight->getDiffuseColour());
	setSpecularColour(mParentLight->getSpecularColour());
}