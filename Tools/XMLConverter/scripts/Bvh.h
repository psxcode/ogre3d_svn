/*
-----------------------------------------------------------------------------
2008-05-27 created
Lucas Westine
-----------------------------------------------------------------------------
*/

#ifndef __Bvh_H__
#define __Bvh_H__


#include "OgrePrerequisites.h"
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <strstream>




	/** Class for bvh fileformat , this class is independent of Ogre, which means it can be used by other 
	applications to deal with bvh mocap data
	@par
	<LI>Create a Bvh object and populate it using it's methods.</LI>
	*/

	class  Bvh
	{
		Bvh();
	public:
		typedef struct	
		{
			Ogre::Real Xposition;
			Ogre::Real Yposition;
			Ogre::Real Zposition;

		} PositionChannel;

		typedef struct  
		{
			Ogre::Real Xrotation;
			Ogre::Real Yrotation;
			Ogre::Real Zrotation;

		} RotationChannel;

		typedef struct BVH_Node_tag
		{
			std::string name;
			BVH_Node_tag* pParent;
			
			Ogre::Vector3 offset;
			PositionChannel*   pPChannel;
			RotationChannel*   pRChannel; 
		} BVH_Node;

		typedef std::vector<BVH_Node> Bvh_Hierarchy;
		typedef std::vector<std::vector<Ogre::Real> > Bvh_Motion ;

	public:
		Bvh(const std::string& filename);
		virtual ~Bvh();
		//* if bLineReady is true, no need to getline first, else getline first in ReadHierarchy
		// the return  value is the sum of Channels in this bvh
		int ReadHierarchy(const std::ifstream& ifs, std::stack<BVH_Node*>& hierarchy, bool bLineReady, std::stringstream& preSs);

		
	protected:
		Bvh_Hierarchy m_Hierarchy;
		int           m_FrameNum;
		Ogre::Real    m_FrameDuration;
		Bvh_Motion    m_Motion;

		


	};



#endif
