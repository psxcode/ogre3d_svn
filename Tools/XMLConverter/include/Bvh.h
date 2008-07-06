/*
-----------------------------------------------------------------------------
2008-05-27 created
Lucas Westine
-----------------------------------------------------------------------------
*/

#ifndef __Bvh_H__
#define __Bvh_H__


#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <iostream>




	/** Class for bvh fileformat , this class is independent of Ogre, which means it can be used by other 
	applications to deal with bvh mocap data
	@par
	<LI>Create a Bvh object and populate it using it's methods.</LI>
	*/
	class  Bvh
	{
		Bvh();
	public:

		enum BoneMapping
		{
			NON_BONE_ID = -1
		};
		typedef struct	
		{
			int		   ChannelBlockIndex;
			Ogre::Real Xposition;
			Ogre::Real Yposition;
			Ogre::Real Zposition;

		} PositionChannel;

		typedef struct  
		{
			int		   ChannelBlockIndex;
			Ogre::Real Xrotation;
			Ogre::Real Yrotation;
			Ogre::Real Zrotation;

		} RotationChannel;

		typedef struct BVH_Node_tag
		{
			std::string name;
			BVH_Node_tag* pParent;
			int index;   //record the index in bvh hierarchy
			Ogre::Vector3 offset;
			PositionChannel*   pPChannel;
			RotationChannel*   pRChannel; 
		} BVH_Node;

		typedef std::vector<BVH_Node*> Bvh_Hierarchy;
		typedef std::vector<std::vector<Ogre::Real> > Bvh_Motion ;
		typedef std::vector<int>  ChannelMap;
		typedef struct  
		{
			int id;
			std::string name;
		} Bone_Element;
		typedef struct
		{
			Bone_Element* pBvh_part;
			Bone_Element* pSkeleton_part;
		} Bone_Mapping_Element;
		typedef std::vector<Bone_Mapping_Element*> Bone_Map;
		

	public:
		Bvh(const std::string& filename);
		virtual ~Bvh();
		int  FrameNum() const { return m_FrameNum; }
		Ogre::Real FrameDuration() const { return m_FrameDuration; }
		//* if bLineReady is true, no need to getline first, else getline first in ReadHierarchy
		// the return  value is the sum of Channels in this bvh
		int ReadHierarchy(std::ifstream& ifs, std::stack<BVH_Node*>& hierarchy, bool bLineReady, std::stringstream& preSs);
		const Bvh_Motion&   MotionData() const { return m_Motion; }    
		const ChannelMap&   GetChannelMap() const { return m_ChannelMap; }
		const Bvh_Hierarchy&  GetHierarchy() const { return m_Hierarchy; }
		const Bone_Map&  GetBoneMap() const { return m_BoneMap; }
		void  LoadBoneMap();
		void LogBoneHierarchy();
		bool IsBoneMapping() const { return m_bIsBoneMapping; }
		const std::string& GetAnimName(void) const { return m_AnimName; }
	protected:	
		Bvh_Hierarchy m_Hierarchy;
		int           m_FrameNum;
		Ogre::Real    m_FrameDuration;
		Bvh_Motion    m_Motion;
		ChannelMap    m_ChannelMap;
		std::string   m_AnimName;
		Bone_Map      m_BoneMap;
		bool          m_bIsBoneMapping;
		// because there end_effector in bvh, the m_Hierarchy.size() is larger than or equal to the joint num in bvh 
		int           m_BoneNum;   
		int ChannelSum;       //be used in recursive function ReadHierarchy(), so make it object domain
		int ChannelBlockIndex;//be used in recursive function ReadHierarchy(), so make if object domain
	};



#endif
