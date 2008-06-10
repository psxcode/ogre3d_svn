
#include "Bvh.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <stack>
//#include <strstream>  this is for char* stringstream

#include "OgrePrerequisites.h"

Bvh::~Bvh()
{
}

Bvh::Bvh(const std::string &filename):m_AnimName(filename),m_BoneNum(0)
{
	std::ifstream ifs;
	int ChannelSum;
	ifs.open(filename.c_str(), std::ios_base::in);
	if (ifs.bad())
	{
		std::cout << "Unable to load file " << filename << std::endl;
		exit(1);
	}

	LoadBoneMap();

	std::stack<BVH_Node*> hierarchy;
	std::string line;

	std::getline(ifs,line);
	std::stringstream ss(line);
	std::string str;
	ss >> str;
	if ( !strcmp(str.c_str(),"HIERARCHY"))
	{
		ChannelSum = ReadHierarchy(ifs,hierarchy,false,ss);
	}
	//because "MOTION" has already been read in ReadHierarchy()
	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	ss >> str;
	if ( !strcmp(str.c_str(),"Frames:"))
	{
		
		ss >> m_FrameNum ;
	
		std::getline(ifs,line);
		ss.clear();
		ss.str(line);
		ss >> str;
		if ( !strcmp(str.c_str(),"Frame"))
		{
			str.clear();
			ss >> str;
			if ( !strcmp(str.c_str(),"Time:") )
			{
				ss >> m_FrameDuration ;
			}
		}

		m_Motion.resize(m_FrameNum);
		int FrameNum = 0;
		while ( !ifs.eof() )
		{
			std::getline(ifs,line);
			if ( !strcmp(line.c_str(),""))
				break;
			ss.clear();
			ss.str(line);
			Ogre::Real channelValue;

			for ( int i = 0; i < ChannelSum; i++)
			{
			ss >> channelValue ;
			m_Motion[FrameNum].push_back(channelValue);	 
			}
			FrameNum++;
		}
	}
	ifs.close();
}


void Bvh::LoadBoneMap()
{

	std::ifstream iBoneMap;

	iBoneMap.open("boneMap.txt", std::ios_base::in);
	if (iBoneMap.is_open() == false )
	{
		std::cout << "Unable to load BoneMap file " << std::endl;
		m_bIsBoneMapping = false;
	}
	else
		m_bIsBoneMapping = true;

	std::string line;
	std::stringstream ss;
	std::string str;
	char token;
	if (m_bIsBoneMapping == true)
	{
		while (!iBoneMap.eof())
		{
		
		std::getline(iBoneMap,line);
		if ( strcmp(line.c_str(),"") == 0 ) //at the end of file there is a an additional blank line 
		{
			break;
		}
		ss.clear();
		ss.str(line);
		Bone_Mapping_Element* pElem = new Bone_Mapping_Element;
		Bone_Element* pBvhElem = new Bone_Element;
		Bone_Element* pSkeletonElem = new Bone_Element;
		ss >> str;  // bvh_bone
		ss >> str; // id
		ss >> token; //=
		//ss >> token; //"
		ss >> pBvhElem->id; // id value
		//ss >> token; //"
		ss >> str; // name
		ss >> token; // =
		//ss >> token; // "
		ss >> pBvhElem->name; // name value
		//ss >> token; // "
		ss >> str; // skeleton_bone
		ss >> str; // id
		ss >> token; // =
		//ss >> token; // "
		ss >> pSkeletonElem->id; // id value
		//ss >> token; // "
		ss >> str;  // name
		ss >> token; // =
		//ss >> token; // "
		ss >> pSkeletonElem->name; // name value
		//ss >> token; // "
	
		pElem->pBvh_part = pBvhElem;
		pElem->pSkeleton_part = pSkeletonElem;
		m_BoneMap.push_back(pElem);
		}

	}

}

int Bvh::ReadHierarchy(std::ifstream& ifs, std::stack<BVH_Node*>& hierarchy, bool bLineReady, std::stringstream& preSs)
{

	static int ChannelSum = 0;
	static int ChannelBlockIndex = 0;
	std::string line;
	std::stringstream ss;
	if ( bLineReady == true )
	{
		ss.str(preSs.str());
	}else
	{
		std::getline(ifs,line);
		ss.str(line);
	}
	std::string str;
	ss >> str;
	
	BVH_Node* pBN = new BVH_Node;
	if ( !strcmp(str.c_str(),"ROOT"))
	{
		ss >> str;
		pBN->name = str;
		pBN->pParent = NULL;
		m_BoneNum++;
	}else if ( !strcmp(str.c_str(),"JOINT"))
	{
		ss >> str;
		pBN->name = str;
		pBN->pParent = static_cast<BVH_Node*>(hierarchy.top());
		m_BoneNum++;

	}else if ( !strcmp(str.c_str(),"End"))
	{
	
		pBN->name = static_cast<BVH_Node*>(hierarchy.top())->name + "_End";
		pBN->pParent = static_cast<BVH_Node*>(hierarchy.top());
	}

	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	ss >> str;
	if ( !strcmp(str.c_str(),"{"))
	{
		hierarchy.push(pBN);
		m_Hierarchy.push_back(pBN);
	}
	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	ss >> str;
	if ( !strcmp(str.c_str(),"OFFSET"))
	{
		ss >> pBN->offset.x;
		ss >> pBN->offset.y;
		ss >> pBN->offset.z;
	}

	
	std::getline(ifs,line);
	ss.clear();
	ss.str(line);
	ss >> str;

	if ( !strcmp(str.c_str(),"CHANNELS"))
	{
		int ChannelNum;
		ss >> ChannelNum;
		if ( ChannelNum == 6)
		{
			m_ChannelMap.push_back(ChannelSum);
			
			pBN->pPChannel = new PositionChannel;
			pBN->pPChannel->ChannelBlockIndex = ChannelBlockIndex;
			ChannelBlockIndex++;
			ChannelSum += 3;
			pBN->pPChannel->Xposition = 0;
			pBN->pPChannel->Yposition = 0;
			pBN->pPChannel->Zposition = 0;

			m_ChannelMap.push_back(ChannelSum);
           
			pBN->pRChannel = new RotationChannel; 
			pBN->pRChannel->ChannelBlockIndex = ChannelBlockIndex;
			ChannelBlockIndex++;
			ChannelSum += 3;
			pBN->pRChannel->Xrotation = 0;
			pBN->pRChannel->Yrotation = 0;
			pBN->pRChannel->Zrotation = 0;
		}else if ( ChannelNum == 3)
		{
			m_ChannelMap.push_back(ChannelSum);
			
			pBN->pPChannel = NULL;
			pBN->pRChannel = new RotationChannel;
			pBN->pRChannel->ChannelBlockIndex = ChannelBlockIndex;
			ChannelBlockIndex++;
			ChannelSum += 3;
			pBN->pRChannel->Xrotation = 0;
			pBN->pRChannel->Yrotation = 0;
			pBN->pRChannel->Zrotation = 0;
		}
		return ReadHierarchy(ifs,hierarchy,false,ss);

	}else {
	    while( !strcmp(str.c_str(),"}"))
	{
		hierarchy.pop();
		std::getline(ifs,line);
		ss.clear();
		ss.str(line);
		ss >> str;
	}
		if ( !strcmp(str.c_str(),"JOINT"))
		{
			ss.clear();
			ss.str(line);
			return ReadHierarchy(ifs,hierarchy,true,ss);
		}
		else// "MOTION" has been read here
        return ChannelSum;
	}
}

void Bvh::LogBoneHierarchy()
{
	std::ofstream ofs;
	
	ofs.open(std::string(m_AnimName+"_bonetable").c_str(), std::ios_base::out);
	if (ofs.bad())
	{
		std::cout << "Unable to create file " << m_AnimName << std::endl;
		exit(1);
	}

	for ( Bvh_Hierarchy::size_type i = 0; i < m_Hierarchy.size(); i++)
	{
		ofs<<"bvh_bone id = "<< i << " name = "<<m_Hierarchy[i]->name<<std::endl;
	}
	ofs.close();

	//this is for initializing BoneMap.txt, the actual bone matching work is up to users,
	// it is planned to design automatically bone matching method instead of manual one after all
	// the motion graph has be implemented 

#ifdef BONE_MAP
	ofs.open("BoneMap1.txt",std::ios_base::out);
	if (ofs.bad())
	{
		std::cout << "Unable to create file BoneMap.txt" << std::endl;
		exit(1);
	}
	for ( Bvh_Hierarchy::size_type i = 0; i < m_Hierarchy.size(); i++)
	{
		ofs<<"bvh_bone id = "<< i << " name = "<<m_Hierarchy[i]->name;
		ofs<<"\t \t skeleton_bone id = "" name ="""<<std::endl;
	}

#endif


}