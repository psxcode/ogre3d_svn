#include "SamplePlugin.h"
#include "Dot3Bump.h"

using namespace Ogre;
using namespace OgreBites;

SamplePlugin* sp;
Sample* s;

extern "C" __declspec(dllexport) void dllStartPlugin()
{
	s = new Dot3BumpSample;
	sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
	sp->addSample(s);
	Root::getSingleton().installPlugin(sp);
}

extern "C" __declspec(dllexport) void dllStopPlugin()
{
	Root::getSingleton().uninstallPlugin(sp); 
	OGRE_DELETE sp;
	delete s;
}
