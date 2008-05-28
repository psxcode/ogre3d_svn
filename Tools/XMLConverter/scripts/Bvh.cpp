
#include "Bvh.h"
#include <iostream>
#include <fstream>
#include <stack>
#include <strstream>
#include <sstream>
#include "OgrePrerequisites.h"


Bvh::Bvh(const std::string &filename)
{
	std::ifstream ifs;
	int ChannelSum;
	ifs.open(filename.c_str(), std::ios_base::in);
	if (ifs.bad())
	{
		std::cout << "Unable to load file " << filename << std::endl;
		exit(1);
	}

	std::stack<BVH_Node*> hierarchy;
	char line[256];

	std::getline(ifs,line);
	std::stringstream ss(line);
	std::string str;
	str << ss;
	if ( !strcmp(str.c_str(),"HIERARCHY"))
	{
		ChannelSum = ReadHierarchy(ifs,hierarchy,false,ss);
	}
	std::getline(ifs,line);
	str.clear();
	ss.str(line);
	str << ss;
	if ( !strcmp(str.c_str(),"MOTION"))
	{
		
		std::getline(ifs,line);
		str.clear();
		ss.str(line);
		str << ss;
		if ( !strcmp(str.c_str(),"Frames:"))
		{
			m_FrameNum << ss;
		}
		std::getline(ifs,line);
		str.clear();
		ss.str(line);
		str << ss;
		if ( !strcmp(str.c_str(),"Frame"))
		{
			str.clear();
			str << ss;
			if ( !strcmp(str.c_str(),"Time:")
			{
				m_FrameDuration << ss;
			}
		}

		m_Motion.resize(m_FrameNum);
		int FrameNum = 0;
		while ( !ifs.eof() )
		{
			
			std::getline(ifs,line);
			str.clear();
			ss.str(line);
			Ogre::Real channelValue;

			for ( int i = 0; i < ChannelSum; i++)
			{
			channelValue << ss;
			m_Motion[FrameNum].push_back(channelValue);	 
			}
		}
	}
}


int Bvh::ReadHierarchy(const ifstream& ifs, std::stack<BVH_Node*>& hierarchy, bool bLineReady, std::stringstream& preSs )
{

	static int ChannelSum = 0;
	char line[256];
	std::stringstream ss;
	if ( bLineReady == true )
	{
		ss = preSs;
	}else
	{
		std::getline(ifs,line);
		ss.str(line);
	}
	std::string str;
	str << ss;
	
	BVH_Node* pBN = new BVH_Node;
	if ( !strcmp(str.c_str(),"ROOT"))
	{
		str << ss;
		pBN->name = str;
		pBN->pParent NULL;
	}else if ( !strcmp(str.c_str(),"JOINT"))
	{
		str << ss;
		pBN->name = str;
		pBN->pParent = static_cast<BVH_Node*>(hierarchy.top());

	}else if ( !strcmp(str.c_str(),"End"))
	{
	
		pBN->name = static_cast<BVH_Node*>(hierarchy.top())->name + "_End";
		pBN->pParent = static_cast<BVH_Node*>(hierarchy.top());
	}

	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	str << ss;
	if ( !strcmp(str.c_str(),"{"))
	{
		hierarchy.push(pBN);
		m_Hierarchy.push_back(pBN);
	}
	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	str << ss;
	if ( !strcmp(str.c_str(),"OFFSET"))
	{
		pBN->offset.x << ss;
		pBN->offset.y << ss;
		pBN->offset.z << ss;
	}

	
	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	str << ss;

	if ( !strcmp(str.c_str(),"CHANNELS"))
	{
		int ChannelNum;
		ChannelNum << ss;
		if ( ChannelNum == 6)
		{
			pBN->pPChannel = new PositionChannel;
			ChannelSum += 3;
			pBN->pPChannel->Xposition = 0;
			pBN->pPChannel->Yposition = 0;
			pBN->pPChannel->Zposition = 0;
			pBN->pRChannel = new RotationChannel;
			ChannelSum += 3;
			pBN->pRChannel->Xrotation = 0;
			pBN->pRChannel->Yrotation = 0;
			pBN->pRChannel->Zrotation = 0;
		}else if ( ChannelNum == 3)
		{
			pBN->pPChannel = NULL;
			pBN->pRChannel = new RotationChannel;
			ChannelSum += 3;
			pBN->pRChannel->Xrotation = 0;
			pBN->pRChannel->Yrotation = 0;
			pBN->pRChannel->Zrotation = 0;
		}
		ReadHierarchy(ifs,hierarchy,false,ss);

	}else 
	    while( !strcmp(str.c_str(),"}"))
	{
		hierarchy.pop();
		std::getline(ifs,line);
		ss.clear();
		ss.str(line);
		str << ss;
	}
		ss.clear();
		ss.str(line);
		ReadHierarchy(ifs,hierarchy,true,ss);

   return ChannelSum;
}