/***************************************************************************
octreenode.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation; either version 2 of the    * 
*   License, or (at your option) any later version.                       *
*                                                                         *
***************************************************************************/

#ifndef OCTREENODE_H
#define OCTREENODE_H

#include <OgreSceneNode.h>

#include <OgreOctreeSceneManager.h>

namespace Ogre
{

/** Specialized SceneNode that is customized for working within an Octree. Each node
* maintains it's own bounding box, rather than merging it with all the children.
*
*/

class OctreeNode : public SceneNode
{
public:
    /** Standard constructor */
    OctreeNode( SceneManager* creator );
    /** Standard constructor */
    OctreeNode( SceneManager* creator, const String& name );
    /** Standard destructor */
    ~OctreeNode();

    /** Overridden from Node to remove any reference to octants */
    Node * removeChild( unsigned short index );
    
    /** Overridden from Node to remove any reference to octants */
    Node * removeChild( const String & name );

    /** Returns the Octree in which this OctreeNode resides
    */
    Octree * getOctant()
    {
        return mOctant;
    };

    /** Sets the Octree in which this OctreeNode resides
    */
    void setOctant( Octree *o )
    {
        mOctant = o;
    };

    /** Determines if the center of this node is within the given box
    */
    bool _isIn( AxisAlignedBox &box );

    /** Adds all the attached scenenodes to the render queue
    */
    virtual void _addToRenderQueue( Camera* cam, RenderQueue * );

    /** Sets up the LegacyRenderOperation for rendering this scene node as geometry.
    @remarks
    This will render the scenenode as a bounding box.
    */
    virtual void getRenderOperation( RenderOperation& rend );

    /** Returns the local bounding box of this OctreeNode.
    @remarks
    This is used to render the bounding box, rather then the global.
    */
    AxisAlignedBox & _getLocalAABB()
    {
        return mLocalAABB;
    };




protected:

    /** Internal method for updating the bounds for this OctreeNode.
    @remarks
    This method determines the bounds solely from the attached objects, not
    any children. If the node has changed it's bounds, it is removed from its
    current octree, and reinserted into the tree.
    */
    void _updateBounds( void );

    void _removeNodeAndChildren( );

    ///local bounding box
    AxisAlignedBox mLocalAABB;

    ///Octree this node is attached to.
    Octree *mOctant;

    ///preallocated corners for rendering
    Real mCorners[ 24 ];
    ///shared colors for rendering
    static unsigned long mColors[ 8 ];
    ///shared indexes for rendering
    static unsigned short mIndexes[ 24 ];


};

}


#endif
