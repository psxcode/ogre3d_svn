#ifndef __SdkCameraMan_H__
#define __SdkCameraMan_H__

#include "Ogre.h"

namespace OgreBites
{
	enum CameraStyle   // enumerator values for different styles of camera movement
	{
		CS_FREELOOK,
		CS_ORBIT,
		CS_MANUAL
	};

	/*=============================================================================
	| Utility class for controlling the camera in samples.
	=============================================================================*/
	class SdkCameraMan
    {
    public:

		SdkCameraMan(Ogre::Camera* cam)
		{
			mTarget = 0;
			mOrbiting = false;
			mZooming = false;
			mCamera = 0;
			mTopSpeed = 150;
			mGoingForward = false;
			mGoingBack = false;
			mGoingLeft = false;
			mGoingRight = false;
			mVelocity = Ogre::Vector3::ZERO;

			// create our camera's main control scene node and set our camera and style
			mCamNode = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
			setCamera(cam);
			setStyle(CS_FREELOOK);
		}

		virtual ~SdkCameraMan() {}

		/*-----------------------------------------------------------------------------
		| Swaps the camera on our camera man for another camera.
		-----------------------------------------------------------------------------*/
		virtual void setCamera(Ogre::Camera* cam)
		{
			if (mCamera) mCamNode->detachObject(mCamera);
			mCamera = cam;
			if (mCamera) mCamNode->attachObject(mCamera);
		}

		virtual Ogre::Camera* getCamera()
		{
			return mCamera;
		}

		virtual Ogre::SceneNode* getCameraNode()
		{
			return mCamNode;
		}

		/*-----------------------------------------------------------------------------
		| Sets the target we will revolve around. Only applies for orbit style.
		-----------------------------------------------------------------------------*/
		virtual void setTarget(Ogre::Node* target)
		{
			if (mStyle == CS_ORBIT)
			{
				mTarget->removeChild(mCamNode);
				mTarget = target ? target : mCamNode->getCreator()->getRootSceneNode();
				mTarget->addChild(mCamNode);
			}
		}

		virtual Ogre::Node* getTarget()
		{
			return mTarget;
		}

		/*-----------------------------------------------------------------------------
		| Sets the spatial offset from the target. Only applies for orbit style.
		-----------------------------------------------------------------------------*/
		virtual void setTargetOffset(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
		{
			if (mStyle == CS_ORBIT)
			{
				mCamNode->setPosition(0, 0, 0);
				mCamNode->yaw(yaw, Ogre::Node::TS_PARENT);
				mCamNode->pitch(-pitch, Ogre::Node::TS_LOCAL);
				mCamNode->translate(0, 0, dist, Ogre::Node::TS_LOCAL);
			}
		}

		/*-----------------------------------------------------------------------------
		| Sets the camera's top speed. Only applies for free-look style.
		-----------------------------------------------------------------------------*/
		virtual void setTopSpeed(Ogre::Real topSpeed)
		{
			mTopSpeed = topSpeed;
		}

		virtual Ogre::Real getTopSpeed()
		{
			return mTopSpeed;
		}

		/*-----------------------------------------------------------------------------
		| Sets the movement style of our camera man.
		-----------------------------------------------------------------------------*/
		virtual void setStyle(CameraStyle style)
		{
			mStyle = style;

			// reset the camera's target if using orbit style
			if (mStyle == CS_ORBIT) setTargetOffset(Ogre::Degree(0), Ogre::Degree(15), 150);
			else
			{
				if (mTarget)   // unparent the camera node if not using orbit style
				{
					mTarget->removeChild(mCamNode);
					mCamNode->getCreator()->getRootSceneNode()->addChild(mCamNode);
				}

				// reset camera position and set the target to the root
				mTarget = mCamNode->getParent();
				mCamNode->setPosition(Ogre::Vector3::ZERO);
				mCamNode->setOrientation(Ogre::Quaternion::IDENTITY);
			}
		}

		virtual CameraStyle getStyle()
		{
			return mStyle;
		}

		/*-----------------------------------------------------------------------------
		| Manually stops the camera when in free-look mode.
		-----------------------------------------------------------------------------*/
		virtual void manualStop()
		{
			if (mStyle == CS_FREELOOK)
			{
				mGoingForward = false;
				mGoingBack = false;
				mGoingLeft = false;
				mGoingRight = false;
				mVelocity = Ogre::Vector3::ZERO;
			}
		}

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			if (mStyle == CS_FREELOOK)
			{
				// build our acceleration vector based on keyboard input composite
				Ogre::Vector3 accel = Ogre::Vector3::ZERO;
				if (mGoingForward) accel -= mCamNode->getOrientation().zAxis();
				if (mGoingBack) accel += mCamNode->getOrientation().zAxis();
				if (mGoingRight) accel += mCamNode->getOrientation().xAxis();
				if (mGoingLeft) accel -= mCamNode->getOrientation().xAxis();

				// if accelerating, try to reach top speed in a certain time
				if (accel.squaredLength() != 0)
				{
					accel.normalise();
					mVelocity += accel * mTopSpeed * evt.timeSinceLastFrame * 10;
				}
				// if not accelerating, try to stop in a certain time
				else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

				// keep camera velocity below top speed and above zero
				if (mVelocity.squaredLength() > mTopSpeed * mTopSpeed)
				{
					mVelocity.normalise();
					mVelocity *= mTopSpeed;
				}
				else if (mVelocity.squaredLength() < 0.1) mVelocity = Ogre::Vector3::ZERO;

				if (mVelocity != Ogre::Vector3::ZERO)
					mCamNode->translate(mVelocity * evt.timeSinceLastFrame);
			}

			return true;
		}

		/*-----------------------------------------------------------------------------
		| Processes key presses for free-look style movement.
		-----------------------------------------------------------------------------*/
		virtual void injectKeyDown(const OIS::KeyEvent& evt)
		{
			if (mStyle == CS_FREELOOK)
			{
				if (evt.key == OIS::KC_W) mGoingForward = true;
				else if (evt.key == OIS::KC_S) mGoingBack = true;
				else if (evt.key == OIS::KC_A) mGoingLeft = true;
				else if (evt.key == OIS::KC_D) mGoingRight = true;
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes key releases for free-look style movement.
		-----------------------------------------------------------------------------*/
		virtual void injectKeyUp(const OIS::KeyEvent& evt)
		{
			if (mStyle == CS_FREELOOK)
			{
				if (evt.key == OIS::KC_W) mGoingForward = false;
				else if (evt.key == OIS::KC_S) mGoingBack = false;
				else if (evt.key == OIS::KC_A) mGoingLeft = false;
				else if (evt.key == OIS::KC_D) mGoingRight = false;
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes mouse movement differently for each style.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseMove(const OIS::MouseEvent& evt)
		{
			if (mStyle == CS_ORBIT)
			{
				if (mOrbiting)   // yaw around the target, and pitch locally
				{
					Ogre::Degree yaw(-evt.state.X.rel * 0.25f);
					Ogre::Degree pitch(-evt.state.Y.rel * 0.25f);

					Ogre::Real dist = mCamNode->getPosition().length();

					mCamNode->translate(0, 0, -dist, Ogre::Node::TS_LOCAL);
					
					mCamNode->yaw(yaw, Ogre::Node::TS_PARENT);
					mCamNode->pitch(pitch);

					mCamNode->translate(0, 0, dist, Ogre::Node::TS_LOCAL);

					// don't let the camera go over the top or around the bottom of the target
					if (mCamNode->getOrientation().yAxis().y < 0)
					{
						if (mCamNode->getOrientation().zAxis().y > 0) mCamNode->setPosition(0, dist, 0);
						else mCamNode->setPosition(0, -dist, 0);
						mCamNode->lookAt(Ogre::Vector3::ZERO, Ogre::Node::TS_PARENT);
					}
				}
				else if (mZooming)  // move the camera toward or away from the target
				{
					// the further the camera is, the faster it moves
					mCamNode->translate(0, 0, evt.state.Y.rel * 0.004f * mCamNode->getPosition().length(),
						Ogre::Node::TS_LOCAL);
				}
				else if (evt.state.Z.rel != 0)  // move the camera toward or away from the target
				{
					// the further the camera is, the faster it moves
					mCamNode->translate(0, 0, -evt.state.Z.rel * 0.0008f * mCamNode->getPosition().length(),
						Ogre::Node::TS_LOCAL);
				}
			}
			else if (mStyle == CS_FREELOOK)
			{
				Ogre::Degree yaw(-evt.state.X.rel * 0.15f);
				Ogre::Degree pitch(-evt.state.Y.rel * 0.15f);

				mCamNode->yaw(yaw, Ogre::Node::TS_PARENT);
				mCamNode->pitch(pitch);

				if (mCamNode->getOrientation().yAxis().y < 0)
				{
					if (mCamNode->getOrientation().zAxis().y > 0)
						mCamNode->lookAt(mCamNode->getPosition() + Ogre::Vector3::NEGATIVE_UNIT_Y, Ogre::Node::TS_PARENT);
					else mCamNode->lookAt(mCamNode->getPosition() + Ogre::Vector3::UNIT_Y, Ogre::Node::TS_PARENT);
				}
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes mouse presses. Only applies for orbit style.
		| Left button is for orbiting, and right button is for zooming.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mStyle == CS_ORBIT)
			{
				if (id == OIS::MB_Left) mOrbiting = true;
				else if (id == OIS::MB_Right) mZooming = true;
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes mouse releases. Only applies for orbit style.
		| Left button is for orbiting, and right button is for zooming.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseUp(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mStyle == CS_ORBIT)
			{
				if (id == OIS::MB_Left) mOrbiting = false;
				else if (id == OIS::MB_Right) mZooming = false;
			}
		}

    protected:

		Ogre::Camera* mCamera;
		Ogre::SceneNode* mCamNode;
		Ogre::String mName;
		CameraStyle mStyle;
		Ogre::Node* mTarget;
		bool mOrbiting;
		bool mZooming;
		Ogre::Real mTopSpeed;
		Ogre::Vector3 mVelocity;
		bool mGoingForward;
		bool mGoingBack;
		bool mGoingLeft;
		bool mGoingRight;
    };
}

#endif
