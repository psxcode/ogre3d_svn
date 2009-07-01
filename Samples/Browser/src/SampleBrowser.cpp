#include "SampleBrowser.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
#else
int main(int argc, char *argv[])
#endif
{
	try
	{
		OgreBites::SampleBrowser sb;
		sb.go();
	}
	catch (Ogre::Exception& e)
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA(NULL, e.what(), "An exception has occured!", MB_ICONERROR | MB_TASKMODAL);
		#else
		std::cerr << "An exception has occured: " << e.what() << std::endl;
		#endif
	}
	return 0;
}
