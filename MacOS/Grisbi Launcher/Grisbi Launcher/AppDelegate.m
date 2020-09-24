//
//  AppDelegate.m
//  Grisbi Launcher
//
//  Created by Ludovic Rousseau on 19/09/2020.
//  Copyright © 2020 Grisbi. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSTask *task = [[NSTask alloc] init];
    NSBundle *main = [NSBundle mainBundle];
//    NSLog(@"bundle: %@", [main resourceURL]);
    NSURL *baseURL = [main resourceURL];

    // Get the Grisbi.app inside the launcher
    NSURL *url = [NSURL URLWithString:@"Grisbi.app" relativeToURL:baseURL];
    NSBundle *bundle = [NSBundle bundleWithURL:[url absoluteURL]];
//    NSLog(@"url: %@", [url absoluteURL]);
//    NSLog(@"executablePath: %@", [bundle executablePath]);
    [task setExecutableURL:[bundle executableURL]];

    //    NSArray *arguments = [NSArray arrayWithObjects:@"Argument1", @"Argument2", nil];
    //    [task setArguments:arguments];
    NSError *error;
    BOOL success = [task launchAndReturnError: &error];
    if (!success)
    {
        NSLog(@"Error: %@ %@", error, [error userInfo]);
    }

    // wait for the application to start
    sleep(1);

    NSArray *apps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in apps)
    {
//        NSLog(@"app: %@", app);
//        NSLog(@"Bundle: %@", [app bundleIdentifier]);
        if ([[app bundleIdentifier] isEqual: @"org.grisbi.Grisbi"])
        {
            // make it front and focus
            [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
        }
    }

    // Terminate the launcher
    [NSApp terminate:self];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
