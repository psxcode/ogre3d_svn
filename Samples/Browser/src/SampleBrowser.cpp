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
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
	[pool release];
	return retVal;
#else
	try
	{
		OgreBites::SampleBrowser sb;
		sb.go();
	}
	catch (Ogre::Exception& e)
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_ICONERROR | MB_TASKMODAL);
		#else
		std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
		#endif
	}
#endif
	return 0;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   ifdef __OBJC__
@interface AppDelegate : NSObject <UIApplicationDelegate>
{
}

- (void)go;

@end

@implementation AppDelegate

- (void)go {
	try
	{
		OgreBites::SampleBrowser sb;
		sb.go();
	}
	catch (Ogre::Exception& e)
	{
		std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
	}

}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	// Hide the status bar
	[[UIApplication sharedApplication] setStatusBarHidden:YES];

	// Create a window
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	// Create an image view
	UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]];
	[window addSubview:imageView];

	// Create an indeterminate status indicator
	UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
	[indicator setFrame:CGRectMake(150, 280, 20, 20)];
	[indicator startAnimating];
	[window addSubview:indicator];

	// Display our window
	[window makeKeyAndVisible];

	// Clean up
	[imageView release];
	[indicator release];

	[NSThread detachNewThreadSelector:@selector(go) toTarget:self withObject:nil];
}

- (void)applicationWillTerminate:(UIApplication *)application {
	Root::getSingleton().queueEndRendering();
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    // Pause FrameListeners and rendering
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    // Resume FrameListeners and rendering
//}

- (void)dealloc {
	[super dealloc];
}

@end
#   endif

#endif
