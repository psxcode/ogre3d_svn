/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __Camera_H__
#define __Camera_H__

// Default options
#include "OgrePrerequisites.h"

#include "OgreString.h"

// Matrices & Vectors
#include "OgreMatrix4.h"
#include "OgreVector3.h"
#include "OgrePlane.h"
#include "OgreQuaternion.h"


namespace Ogre {

    /** Specifies perspective (realistic) or orthographic (architectural) projection.
    */
    enum ProjectionType
    {
        PT_ORTHOGRAPHIC,
        PT_PERSPECTIVE
    };

    /** The broad type of detail this camera will render.
    */
    enum SceneDetailLevel
    {
        SDL_POINTSONLY,
        SDL_WIREFRAME,
        SDL_FLATSHADE,
        SDL_SMOOTHSHADE,
        SDL_TEXTURED
    };

    /** Worldspace clipping planes.
    */
    enum FrustumPlane
    {
        FRUSTUM_PLANE_NEAR   = 0,
        FRUSTUM_PLANE_FAR    = 1,
        FRUSTUM_PLANE_LEFT   = 2,
        FRUSTUM_PLANE_RIGHT  = 3,
        FRUSTUM_PLANE_TOP    = 4,
        FRUSTUM_PLANE_BOTTOM = 5
    };

    /** A viewpoint from which the scene will be rendered.
        @remarks
            OGRE renders scenes from a camera viewpoint into a buffer of
            some sort, normally a window or a texture (a subclass of
            RenderTarget). OGRE cameras support both perspective projection (the default,
            meaning objects get smaller the further away they are) and
            orthographic projection (blueprint-style, no decrease in size
            with distance). Each camera carries with it a style of rendering,
            e.g. full textured, flat shaded, wireframe), field of view,
            rendering distances etc, allowing you to use OGRE to create
            complex multi-window views if required. In addition, more than
            one camera can point at a single render target if required,
            each rendering to a subset of the target, allowing split screen
            and picture-in-picture views.
        @par
            Cameras maintain their own aspect ratios, field of view, and frustrum,
            and project co-ordinates into a space measured from -1 to 1 in x and y,
            and 0 to 1 in z. At render time, the camera will be rendering to a
            Viewport which will translate these parametric co-ordinates into real screen
            co-ordinates. Obviously it is advisable that the viewport has the same
            aspect ratio as the camera to avoid distortion (unless you want it!).
        @par
            Note that a Camera can be attached to a SceneNode, using the method
            SceneNode::attachCamera. If this is done the Camera will combine it's own
            position/orientation settings with it's parent SceneNode. 
            This is useful for implementing more complex Camera / object
            relationships i.e. having a camera attached to a world object.
    */
    class _OgreExport Camera
    {
    protected:
        /// Camera name
        String mName;
        /// Scene manager responsible for the scene
        SceneManager *mSceneMgr;

        /// Camera orientation, quaternion style
        Quaternion mOrientation;

        /// Camera position - default (0,0,0)
        Vector3 mPosition;

        /// Stored versions of parent orientation / position
        Quaternion mLastParentOrientation;
        Vector3 mLastParentPosition;

        /// Derived positions of parent orientation / position
        Quaternion mDerivedOrientation;
        Vector3 mDerivedPosition;

        /// Camera y-direction field-of-view (default 45)
        Real mFOVy;
        /// Far clip distance - default 10000
        Real mFarDist;
        /// Near clip distance - default 100
        Real mNearDist;
        /// x/y viewport ratio - default 1.3333
        Real mAspect;
        /// Whether to yaw around a fixed axis.
        bool mYawFixed;
        /// Fixed axis to yaw around
        Vector3 mYawFixedAxis;

        /// The 6 main clipping planes
        Plane mFrustumPlanes[6];

        /// Orthographic or perspective?
        ProjectionType mProjType;
        /// Rendering type
        SceneDetailLevel mSceneDetail;


        /// Pre-calced projection matrix
        Matrix4 mProjMatrix;
        /// Pre-calced view matrix
        Matrix4 mViewMatrix;
        /// Something's changed in the frustrum shape?
        bool mRecalcFrustum;
        /// Something re the view pos has changed
        bool mRecalcView;

        /// Poniter to scene node attached to
        SceneNode* mSceneNode;

        /** Temp coefficient values calculated from a frustum change,
            used when establishing the frustum planes when the view changes
        */
        Real mCoeffL[2], mCoeffR[2], mCoeffB[2], mCoeffT[2];


        // Internal functions for calcs
        void updateFrustum(void);
        void updateView(void);
        bool isViewOutOfDate(void);
        bool isFrustumOutOfDate(void);

        /// Stored number of visible faces in the last render
        unsigned int mVisFacesLastRender;

    public:
        /** Standard constructor.
        */
        Camera(String name, SceneManager* sm);

        /** Standard destructor.
        */
        virtual ~Camera();

        /** Returns true if this Camera is attached to a SceneNode, and thus may have its
            transformations altered by the node.
        */
        bool isAttached(void) const;

        /** Returns a pointer to the SceneManager this camera is rendering through.
        */
        SceneManager* getSceneManager(void) const;

        /** Returns a pointer to the SceneNode to which this Camera is attached, if any.
        */
        SceneNode* getAttachedSceneNode(void) const;

        /** Internal method for notification of attachments. Not to be used by outside programs.
        */
        void _notifyAttached( SceneNode* attachedTo );


        /** Gets the camera's name.
        */
        const String& getName(void) const;

        /** Sets the type of projection to use (orthographic or perspective). Default is perspective.
        */
        void setProjectionType(ProjectionType pt);

        /** Retrieves info on the type of projection used (orthographic or perspective).
        */
        ProjectionType getProjectionType(void) const;

        /** Sets the level of rendering detail required from this camera.
            @remarks
                Each camera is set to render at full detail by default, that is
                with full texturing, lighting etc. This method lets you change
                that behaviour, allowing you to make the camera just render a
                wireframe view, for example.
        */
        void setDetailLevel(SceneDetailLevel sd);

        /** Retrieves the level of detail that the camera will render.
        */
        SceneDetailLevel getDetailLevel(void) const;

        /** Sets the camera's position.
        */
        void setPosition(Real x, Real y, Real z);

        /** Sets the camera's position.
        */
        void setPosition(const Vector3& vec);

        /** Retrieves the camera's position.
        */
        const Vector3& getPosition(void) const;

        /** Moves the camera's position by the vector offset provided along world axes.
        */
        void move(const Vector3& vec);

        /** Moves the camera's position by the vector offset provided along it's own axes (relative to orientation).
        */
        void moveRelative(const Vector3& vec);

        /** Sets the camera's direction vector.
            @remarks
                Note that the 'up' vector for the camera will automatically be recalculated based on the
                current 'up' vector (i.e. the roll will remain the same).
        */
        void setDirection(Real x, Real y, Real z);

        /** Sets the camera's direction vector.
        */
        void setDirection(const Vector3& vec);

        /* Gets the camera's direction.
        */
        Vector3 getDirection(void) const;


        /** Points the camera at a location in worldspace.
            @remarks
                This is a helper method to automatically generate the
                direction vector for the camera, based on it's current position
                and the supplied look-at point.
            @param
                targetPoint A vector specifying the look at point.
        */
        void lookAt( const Vector3& targetPoint );
        /** Points the camera at a location in worldspace.
            @remarks
                This is a helper method to automatically generate the
                direction vector for the camera, based on it's current position
                and the supplied look-at point.
            @param
                x
            @param
                y
            @param
                z Co-ordinates of the point to look at.
        */
        void lookAt(Real x, Real y, Real z);

        /** Rolls the camera anticlockwise, in degrees, around its local z axis.
        */
        void roll(Real degrees);

        /** Rotates the camera anticlockwise around it's local y axis.
        */
        void yaw(Real degrees);

        /** Pitches the camera up/down anticlockwise around it's local z axis.
        */
        void pitch(Real degrees);

        /** Rotate the camera around an arbitrary axis.
        */
        void rotate(const Vector3& axis, Real degrees);

        /** Rotate the camera around an aritrary axis using a Quarternion.
        */
        void rotate(const Quaternion& q);

        /** Tells the camera whether to yaw around it's own local Y axis or a fixed axis of choice.
            @remarks
                This method allows you to change the yaw behaviour of the camera - by default, the camera
                yaws around it's own local Y axis. This is often what you want - for example a flying camera
                - but sometimes this produces unwanted effects. For example, if you're making a first-person
                shooter, you really don't want the yaw axis to reflect the local camera Y, because this would
                mean a different yaw axis if the player is looking upwards rather than when they are looking
                straight ahead. You can change this behaviour by setting the yaw to a fixed axis (say, the world Y).
            @param
                useFixed If true, the axis passed in the second parameter will always be the yaw axis no
                matter what the camera orientation. If false, the camera returns to it's default behaviour.
            @param
                fixedAxis The axis to use if the first parameter is true.
        */
        void setFixedYawAxis( bool useFixed, const Vector3& fixedAxis = Vector3::UNIT_Y );

        /** Sets the Y-dimension Field Of View (FOV) of the camera.
            @remarks
                Field Of View (FOV) is the angle made between the camera's position, and the left & right edges
                of the 'screen' onto which the scene is projected. High values (90+) result in a wide-angle,
                fish-eye kind of view, low values (30-) in a stretched, telescopic kind of view. Typical values
                are between 45 and 60.
            @par
                This value represents the HORIZONTAL field-of-view. The vertical field of view is calculated from
                this depending on the dimensions of the viewport (they will only be the same if the viewport is square).
            @note
                Setting the FOV overrides the value supplied for Camera::setNearClipPlane.
         */
        void setFOVy(Real fovy);

        /** Retrieves the cameras Y-dimension Field Of View (FOV).
        */
        Real getFOVy(void) const;

        /** Sets the position of the near clipping plane.
            @remarks
                The position of the near clipping plane is the distance from the cameras position to the screen
                on which the world is projected. The near plane distance, combined with the field-of-view and the
                aspect ratio, determines the size of the viewport through which the world is viewed (in world
                co-ordinates). Note that this world viewport is different to a screen viewport, which has it's
                dimensions expressed in pixels. The cameras viewport should have the same aspect ratio as the
                screen viewport it renders into to avoid distortion.
            @param
                near The distance to the near clipping plane from the camera in world coordinates.
         */
        void setNearClipDistance(Real nearDist);

        /** Sets the position of the near clipping plane.
        */
        Real getNearClipDistance(void) const;

        /** Sets the distance to the far clipping plane.
            @remarks
                The view frustrum is a pyramid created from the camera position and the edges of the viewport.
                This frustrum does not extend to infinity - it is cropped near to the camera and there is a far
                plane beyond which nothing is displayed. This method sets the distance for the far plane. Different
                applications need different values: e.g. a flight sim needs a much further far clipping plane than
                a first-person shooter. An important point here is that the larger the gap between near and far
                clipping planes, the lower the accuracy of the Z-buffer used to depth-cue pixels. This is because the
                Z-range is limited to the size of the Z buffer (16 or 32-bit) and the max values must be spread over
                the gap between near and far clip planes. The bigger the range, the more the Z values will
                be approximated which can cause artifacts when lots of objects are close together in the Z-plane. So
                make sure you clip as close to the camera as you can - don't set a huge value for the sake of
                it.
            @param
                far The distance to the far clipping plane from the camera in world coordinates.
        */
        void setFarClipDistance(Real farDist);

        /** Retrieves the distance from the camera to the far clipping plane.
        */
        Real getFarClipDistance(void) const;

        /** Sets the aspect ratio for the camera viewport.
            @remarks
                The ratio between the x and y dimensions of the rectangular area visible through the camera
                is known as aspect ratio: aspect = width / height .
            @par
                The default for most fullscreen windows is 1.3333 - this is also assumed by Ogre unless you
                use this method to state otherwise.
        */
        void setAspectRatio(Real ratio);

        /** Retreives the current aspect ratio.
        */
        Real getAspectRatio(void) const;

        /** Gets the projection matrix for this camera. Mainly for use by OGRE internally.
        */
        const Matrix4& getProjectionMatrix(void);

        /** Gets the view matrix for this camera. Mainly for use by OGRE internally.
        */
        const Matrix4& getViewMatrix(void);

        /** Retrieves a specified plane of the frustum.
            @remarks
                Gets a reference to one of the planes which make up the camera frustum, e.g. for clipping purposes.
        */
        const Plane& getFrustumPlane( FrustumPlane plane );

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding box to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy = 0);

        /** Tests whether the given container is visible in the Frustum.
            @param
                bound Bounding sphere to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the sphere was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const Sphere& bound, FrustumPlane* culledBy = 0);

        /** Tests whether the given vertex is visible in the Frustum.
            @param
                vert Vertex to be checked
            @param
                culledBy Optional pointer to an int which will be filled by the plane number which culled
                the box if the result was false;
            @returns
                If the box was visible, true is returned.
            @par
                Otherwise, false is returned.
        */
        bool isVisible(const Vector3& vert, FrustumPlane* culledBy = 0);

        /** Returns the camera's current orientation.
        */
        const Quaternion& getOrientation(void) const;

        /** Sets the camera's orientation.
        */
        void setOrientation(const Quaternion& q);

        /** Tells the Camera to contact the SceneManager to render from it's viewpoint.
        */
        void _renderScene(Viewport *vp);

        /** Function for outputting to a stream.
        */
        friend std::ostream& operator<<(std::ostream& o, Camera& c);

        /** Internal method to notify camera of the visible faces in the last render.
        */
        void _notifyRenderedFaces(unsigned int numfaces);

        /** Internal method to retrieve the number of visible faces in the last render.
        */
        unsigned int _getNumRenderedFaces(void) const;
    };

} // namespace Ogre
#endif
