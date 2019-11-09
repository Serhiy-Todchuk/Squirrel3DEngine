//
//  AppDelegate.m
//  SimpleTest
//
//  Created by Serhiy Todchuk on 17.10.12.
//  Copyright (c) 2012 Serhiy Todchuk. All rights reserved.
//

#import "AppDelegate.h"
#import <Auxiliary/Application.h>

using namespace Squirrel;
using namespace Auxiliary;

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	Application::Create();
	
	if(!Application::GetApplication()->init())
	{
		[NSApp terminate:self];
		return;
	}
    
    NSTimer * renderTimer = [NSTimer timerWithTimeInterval:(NSTimeInterval)0
                                           target:self
                                         selector:@selector(process:)
                                         userInfo:nil
                                          repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                 forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                 forMode:NSEventTrackingRunLoopMode]; //Ensure timer fires during resize
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	if(Application::GetApplication())
		Application::GetApplication()->onBecomeActive();
}

- (void)applicationWillResignActive:(NSNotification *)aNotification
{
	
}

- (void)process:(id)sender
{
	if(Application::GetApplication())
		Application::GetApplication()->process();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	if(Application::GetApplication())
		delete Application::GetApplication();
}

@end
