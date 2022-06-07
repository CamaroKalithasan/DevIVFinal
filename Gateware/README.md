```
******************* GETTING STARTED ********************
ALL PLATFORMS:

Just copy paste Gateware.h and LICENSE.md into your C++11 64bit console project.
(if you don't see Gateware.h this means you downloaded the main repo used 
for development. Instead go back to "Releases" on our GitLab page and download)

in your source file:
	#define GATEWARE_ENABLE_CORE
	#include "Gateware.h" 

This should compile on any standard C++11 compiler.
Gateware is designed to be Modular so by default it is quite basic.

Next Step is to open the included Docs(index.html) and see what is available. 
Once you spot a library you want to use just #define it's namespace:

#define GATEWARE_ENABLE_CORE // This is called white-lisitng
#define GATEWARE_ENABLE_SYSTEM // Enables valid implementations
#include "Gateware.h" 

Note: Most Classes will still compile, even without the #define but will not be functional. 
Many more complex libs rely on SYSTEM so that is a good one to have typically enabled.

You can also disable specific libraries in a group:
#define GATEWARE_ENABLE_CORE
#define GATEWARE_ENABLE_SYSTEM
#define GATEWARE_DISABLE_GWINDOW // This is called black-listing
#include "Gateware.h" 

Use them to shorten compile times and reduce binary size.

Library Basics:

All Gateware objects are considered "Proxies". 
Proxies are basically smart pointers that have strong & weak binding. 

// To make a valid library interface, declare and "Create(...)"
GW::SYSTEM::GWindow myWin;
if (+myWin.Create(0, 0, 500, 500, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED))
	std::cout << "Gateware window was created" << std::endl;

ALL Gateware routines return a GW:GReturn error/success code.
The '+' above is a shortcut to see if the return was a good one.

When you copy proxies they are considered to have "weak" binding.
This means they will only be valid for the lifetime of the right hand proxy you copied from. 
When you move proxies they are considered to have "strong" binding.
This means they stay valid for the lifetime of the object. 
You can check the validity of a Proxy using simple boolean logic. (ex: if (Proxy){} )

Tips:

All Created proxies can "Share()", "Mirror()" and "Relinquish()".
You can use these operations to more carefully control a proxies lifetime.

To "delete" a proxy just let it fall out of scope or "MyGProxy = nullptr;" 

Gateware arguments are heavily templated behind the scenes. 
If you want "intellisense" to see what they are use the "--->" operator on a Proxy. 
Just be sure to remove it after, they are not meant for actual use and call abort()

If you heavily use Gateware.h in multiple places consider placing it in a pre-compiled header to reduce compile times.

"GW::I" is an internal namespace, ignore it unless you want to become a Gateware developer/contributor.	

Some libraries outside the Core have a couple manual dependencies depending on platform. 
See the below section to resolve them. 

*********** PER PLATFORM SETUP INSTRUCTIONS ************

WIN:

How to setup your dev machine on Windows 10:

All first party libraries are auto-linked so not much to do here.

Requires Visual Studio and the Windows 10 SDK

if you want to use Vulkan:
	Go Here: https://vulkan.lunarg.com/sdk/home
	Install the latest SDK
	Link the SDK /Include/Vulkan & /Lib folders to your project.
else
	#define GATEWARE_DISABLE_GVULKANSURFACE // above include

MAC:

How to setup your dev machine on MacOS (High Sierra/Mojave/Catalina):

You need XCode if you don't already have it.

Most first party frameworks are auto-imported as needed.
But to make that work you must: (in your project)
	change the extensions of any source files that include 

	Gateware.h to support Objective-C++ (.cpp/.m -> .mm) 


XCode Project Settings: (Build Settings)	 
	

Apple Clang - Custom Compiler Flags: Other C++ Flags
		
	-fmodules -fcxx-modules
	
Apple Clang - Warnings - All languages
	
	Mismatched Return Type = No

if you want to use Vulkan:
	Go Here: https://vulkan.lunarg.com/sdk/home
	Download the latest SDK
	Extract the archive
	Navigate the terminal to the extracted folder and:
	./install_vulkan.py
	Link Vulkan to your project.
else
	#define GATEWARE_DISABLE_GVULKANSURFACE // above include


LINUX:

How to setup your dev machine on Linux: (Mint/Ubuntu/Debian)

We build & test using GCC + Codelite on Linux but other compilers & IDEs will probably be fine.

Terminal time:

sudo apt-get install cmake codelite libx11-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libpulse-dev

In your project you need: (depending on library)
	-std=c++11 -pthread -lX11 -lGL -lpulse

if you want to use Vulkan:
	Go Here: https://vulkan.lunarg.com/sdk/home
	Click on the Ubuntu Packages Tab under Linux.
	Click "Latest Supported Release"
	Click on the link that is compatible with your Debian Distro (Typically the top one).
	Copy the text inside the text box underneath the link.
	Paste it into your terminal and run.
	Link Vulkan to your project.
else
	#define GATEWARE_DISABLE_GVULKANSURFACE // above include

optional: (if you would rather use Clang)
sudo apt-get install llvm clang 
``` 
