#ifndef __Test_H__
#define __Test_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class Test1 : public SdkSample
{
public:

	Test1()
	{
		mInfo["Title"] = "Grapefruit";
		mInfo["Thumbnail"] = "thumb_test1.jpg";
		mInfo["Category"] = "Warm Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0.8, 0.4, 0.3));
	}
};

class Test2 : public SdkSample
{
public:

	Test2()
	{
		mInfo["Title"] = "Orange";
		mInfo["Thumbnail"] = "thumb_test2.jpg";
		mInfo["Category"] = "Warm Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(1, 0.7, 0.3));
	}
};

class Test3 : public SdkSample
{
public:

	Test3()
	{
		mInfo["Title"] = "Maroon";
		mInfo["Thumbnail"] = "thumb_test3.jpg";
		mInfo["Category"] = "Warm Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0.5, 0.1, 0.1));
	}
};

class Test4 : public SdkSample
{
public:

	Test4()
	{
		mInfo["Title"] = "Salmon";
		mInfo["Thumbnail"] = "thumb_test4.jpg";
		mInfo["Category"] = "Warm Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(1, 0.8, 0.7));
	}
};

class Test5 : public SdkSample
{
public:

	Test5()
	{
		mInfo["Title"] = "Azure";
		mInfo["Thumbnail"] = "thumb_test5.jpg";
		mInfo["Category"] = "Cool Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0, 0.5, 1));
	}
};

class Test6 : public SdkSample
{
public:

	Test6()
	{
		mInfo["Title"] = "Light Purple";
		mInfo["Thumbnail"] = "thumb_test6.jpg";
		mInfo["Category"] = "Cool Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0.7, 0.7, 1));
	}
};

class Test7 : public SdkSample
{
public:

	Test7()
	{
		mInfo["Title"] = "Green";
		mInfo["Thumbnail"] = "thumb_test7.jpg";
		mInfo["Category"] = "Cool Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0, 0.5, 0.2));
	}
};

class Test8 : public SdkSample
{
public:

	Test8()
	{
		mInfo["Title"] = "Turquoise";
		mInfo["Thumbnail"] = "thumb_test8.jpg";
		mInfo["Category"] = "Cool Colours";
	}

protected:

	void setupScene()
	{     
		mViewport->setBackgroundColour(ColourValue(0.2, 0.7, 0.7));
	}
};

#endif
