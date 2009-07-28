#include "SamplePlugin.h"
#include "Test.h"

using namespace Ogre;
using namespace OgreBites;

SamplePlugin* sp;
SdkSample* tests[8];

extern "C" __declspec(dllexport) void dllStartPlugin()
{
	sp = OGRE_NEW SamplePlugin("Sample Test");

	tests[0] = new Test1;
	tests[1] = new Test2;
	tests[2] = new Test3;
	tests[3] = new Test4;
	tests[4] = new Test5;
	tests[5] = new Test6;
	tests[6] = new Test7;
	tests[7] = new Test8;

	for (int i = 0; i < 8; i++)
	{
		sp->addSample(tests[i]);
	}

	Root::getSingleton().installPlugin(sp);
}

extern "C" __declspec(dllexport) void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp); 

	for (int i = 0; i < 8; i++)
	{
		delete tests[i];
	}

	OGRE_DELETE sp;
}
