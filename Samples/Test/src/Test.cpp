#include "SamplePlugin.h"
#include "Test.h"

using namespace Ogre;
using namespace OgreBites;

SamplePlugin* sp;
Test1* t1;
Test2* t2;
Test3* t3;
Test4* t4;
Test5* t5;

extern "C" __declspec(dllexport) void dllStartPlugin()
{
	sp = OGRE_NEW SamplePlugin("Sample Test");   // create sample plugin

	// create and add samples
	t1 = new Test1;
	sp->addSample(t1);
	t2 = new Test2;
	sp->addSample(t2);
	t3 = new Test3;
	sp->addSample(t3);
	t4 = new Test4;
	sp->addSample(t4);
	t5 = new Test5;
	sp->addSample(t5);

	Root::getSingleton().installPlugin(sp);   // install sample plugin
}

extern "C" __declspec(dllexport) void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp);   // uninstall sample plugin  

	// destroy samples
	delete t1;
	delete t2;
	delete t3;
	delete t4;
	delete t5;

	OGRE_DELETE sp;   // destroy sample plugin  
}
