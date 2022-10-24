// Ouzel by Elviss Strazdins

#include <cstdlib>
#include "SystemMacOS.hpp"
#include "EngineMacOS.hpp"
#include "../../platform/foundation/AutoreleasePool.hpp"
#include "../../utils/Log.hpp"

ouzel::core::macos::System* systemPointer;

@interface AppDelegate: NSObject<NSApplicationDelegate>
@end

@implementation AppDelegate

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification
{
    if (systemPointer) systemPointer->start();
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification
{
    if (ouzel::engine) ouzel::engine->start();
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification
{
    if (ouzel::engine) ouzel::engine->exit();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)sender
{
    return YES;
}

- (BOOL)application:(__unused NSApplication*)sender openFile:(NSString*)filename
{
    if (ouzel::engine)
    {
        auto event = std::make_unique<ouzel::SystemEvent>();
        event->type = ouzel::Event::Type::openFile;
        event->filename = [filename cStringUsingEncoding:NSUTF8StringEncoding];
        ouzel::engine->getEventDispatcher().postEvent(std::move(event));
    }

    return YES;
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification
{
    if (ouzel::engine) ouzel::engine->resume();
}

- (void)applicationDidResignActive:(__unused NSNotification*)notification
{
    if (ouzel::engine) ouzel::engine->pause();
}
@end

int main(int argc, char* argv[])
{
    try
    {
        ouzel::core::macos::System system{argc, argv};
        systemPointer = &system;
        system.run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        ouzel::log(ouzel::Log::Level::error) << e.what();
        return EXIT_FAILURE;
    }
}

namespace ouzel::core::macos
{
    namespace
    {
        std::vector<std::string> parseArgs(int argc, char* argv[])
        {
            std::vector<std::string> result;
            for (int i = 0; i < argc; ++i)
                result.push_back(argv[i]);
            return result;
        }
    }

    System::System(int argc, char* argv[]):
        core::System{parseArgs(argc, argv)}
    {
    }

    void System::run()
    {
        ouzel::platform::foundation::AutoreleasePool autoreleasePool;

        [NSApplication sharedApplication];
        NSApp.activationPolicy = NSApplicationActivationPolicyRegular;

        [NSApp activateIgnoringOtherApps:YES];
        NSApp.delegate = [[[AppDelegate alloc] init] autorelease];

        NSMenu* mainMenu = [[[NSMenu alloc] initWithTitle:@"Main Menu"] autorelease];

        // Apple menu
        NSMenuItem* mainMenuItem = [mainMenu addItemWithTitle:@"Apple"
                                                       action:nil
                                                keyEquivalent:@""];

        NSMenu* applicationMenu = [[[NSMenu alloc] init] autorelease];
        mainMenuItem.submenu = applicationMenu;

        NSString* bundleName = NSBundle.mainBundle.infoDictionary[@"CFBundleDisplayName"];
        if (!bundleName)
            bundleName = NSBundle.mainBundle.infoDictionary[@"CFBundleName"];

        NSMenuItem* aboutItem = [applicationMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), bundleName]
                                                           action:@selector(orderFrontStandardAboutPanel:)
                                                    keyEquivalent:@""];
        aboutItem.target = NSApp;

        [applicationMenu addItem:[NSMenuItem separatorItem]];

        NSMenuItem* servicesItem = [applicationMenu addItemWithTitle:NSLocalizedString(@"Services", nil)
                                                              action:nil
                                                       keyEquivalent:@""];

        NSMenu* servicesMenu = [[[NSMenu alloc] init] autorelease];
        servicesItem.submenu = servicesMenu;
        NSApp.servicesMenu = servicesMenu;

        [applicationMenu addItem:[NSMenuItem separatorItem]];

        NSMenuItem* hideItem = [applicationMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", nil), bundleName]
                                                          action:@selector(hide:)
                                                   keyEquivalent:@"h"];
        hideItem.target = NSApp;

        NSMenuItem* hideOthersItem = [applicationMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
                                                                action:@selector(hideOtherApplications:)
                                                         keyEquivalent:@"h"];
        hideOthersItem.keyEquivalentModifierMask = NSEventModifierFlagOption | NSEventModifierFlagCommand;
        hideOthersItem.target = NSApp;

        NSMenuItem* showAllItem = [applicationMenu addItemWithTitle:NSLocalizedString(@"Show All", nil)
                                                             action:@selector(unhideAllApplications:)
                                                      keyEquivalent:@""];
        showAllItem.target = NSApp;

        [applicationMenu addItem:[NSMenuItem separatorItem]];

        NSMenuItem* quitItem = [applicationMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), bundleName]
                                                          action:@selector(terminate:)
                                                   keyEquivalent:@"q"];
        quitItem.target = NSApp;

        // View menu
        NSMenuItem* viewItem = [mainMenu addItemWithTitle:NSLocalizedString(@"View", nil)
                                                   action:nil
                                            keyEquivalent:@""];

        NSMenu* viewMenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"View", nil)] autorelease];
        viewItem.submenu = viewMenu;

        // Window menu
        NSMenuItem* windowsItem = [mainMenu addItemWithTitle:NSLocalizedString(@"Window", nil)
                                                      action:nil
                                               keyEquivalent:@""];

        NSMenu* windowsMenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", nil)] autorelease];
        [windowsMenu addItemWithTitle:NSLocalizedString(@"Minimize", nil) action:@selector(performMiniaturize:) keyEquivalent:@"m"];
        [windowsMenu addItemWithTitle:NSLocalizedString(@"Zoom", nil) action:@selector(performZoom:) keyEquivalent:@""];

        windowsItem.submenu = windowsMenu;
        NSApp.windowsMenu = windowsMenu;

        NSApp.mainMenu = mainMenu;

        [NSApp run];
    }

    void System::start()
    {
        engine = std::make_unique<Engine>(getArgs());
    }
}
