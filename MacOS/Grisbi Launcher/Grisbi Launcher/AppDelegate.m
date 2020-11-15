//
//  AppDelegate.m
//  Grisbi Launcher
//
//  Created by Ludovic Rousseau on 19/09/2020.
//  Copyright © 2020 Grisbi. All rights reserved.
//

#import "AppDelegate.h"
#include <sys/types.h>
#include <sys/stat.h>

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSTask *task = [[NSTask alloc] init];
    NSBundle *main = [NSBundle mainBundle];
    NSURL *baseURL = [main resourceURL];
    NSError *error;
    NSArray *arguments;
    NSArray *apps;
    BOOL success;
    ssize_t s;
    int r, fd;
    char buf[3];

    // Get the Grisbi.app inside the launcher
    NSURL *url = [NSURL URLWithString:@"Grisbi.app" relativeToURL:baseURL];
    NSBundle *bundle = [NSBundle bundleWithURL:[url absoluteURL]];
    [task setExecutableURL:[bundle executableURL]];

    char temp_filename[] = "/tmp/grisbi.XXXXXXXXXX";
    if (NULL == mktemp(temp_filename))
    {
        NSLog(@"mktemp: %s", strerror(errno));
        goto end;
    }
    NSLog(@"Using: %s", temp_filename);

    r = mkfifo(temp_filename, S_IRUSR | S_IWUSR);
    if (r < 0)
    {
        NSLog(@"mkfifo: %s", strerror(errno));
        goto end;
    }

    arguments = [NSArray arrayWithObjects:[NSString stringWithFormat:@"--launcher=%s", temp_filename], nil];
    [task setArguments:arguments];
    success = [task launchAndReturnError: &error];
    if (!success)
    {
        NSLog(@"Error: %@ %@", error, [error userInfo]);
        goto end;
    }

    // wait for the sub-application to start
    fd = open(temp_filename, O_RDONLY);
    if (fd < 0)
    {
        NSLog(@"open: %s", strerror(errno));
        goto end;
    }
    s = read(fd, buf, sizeof buf);
    NSLog(@"read: [%ld] %s", s, buf);
    close(fd);

    unlink(temp_filename);

    // Make Gribi the front application
    apps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in apps)
    {
        if ([[app bundleIdentifier] isEqual: @"org.grisbi.Grisbi"])
        {
            // make it front and focus
            [app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
        }
    }

end:
    // Terminate the launcher
    [NSApp terminate:self];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
