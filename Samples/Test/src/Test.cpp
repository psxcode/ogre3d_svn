#include "SamplePlugin.h"
#include "Test.h"

using namespace Ogre;
using namespace OgreBites;

SamplePlugin* sp;
Test1* t1;
Test2* t2;

extern "C" __declspec(dllexport) void dllStartPlugin()
{
	sp = OGRE_NEW SamplePlugin("Sample Test");   // create sample plugin

	// create and add samples
	t1 = new Test1;
	t2 = new Test2;
	sp->addSample(t2);
	sp->addSample(t1);

	Root::getSingleton().installPlugin(sp);   // install sample plugin
}

extern "C" __declspec(dllexport) void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp);   // uninstall sample plugin  

	// destroy samples
	delete t1;
	delete t2;

	OGRE_DELETE sp;   // destroy sample plugin  
}
