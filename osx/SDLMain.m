/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#include "SDL2/SDL.h"
#include "SDLMain.h"
#include "NSFileManagerDirectoryLocations.h"
#include "ApplicationSupportBridge.h"
#include <sys/param.h> /* for MAXPATHLEN */
#include <unistd.h>

/* Define these interop methods for calling into Objective-C from C++ at the top, */
/* so they can be referenced from anywhere in this file. */
const char ** GetCaseFilePathsOSX(unsigned int *pCaseFileCount);
const char ** GetSaveFilePathsForCaseOSX(const char *pCaseUuid, unsigned int *pSaveFileCount);
const char * GetVersionStringOSX(const char *pPropertyListFilePath);
char * GetPropertyListXMLForVersionStringOSX(const char *pPropertyListFilePath, const char *pVersionString, unsigned long *pVersionStringLength);

/* For some reaon, Apple removed setAppleMenu from the headers in 10.4,
 but the method still is there and works. To avoid warnings, we declare
 it ourselves here. */
@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end

/* Use this flag to determine whether we use SDLMain.nib or not */
#define		SDL_USE_NIB_FILE	0

/* Use this flag to determine whether we use CPS (docking) or not */
#define		SDL_USE_CPS		1
#ifdef SDL_USE_CPS
/* Portions of CPS.h */
typedef struct CPSProcessSerNum
{
	UInt32		lo;
	UInt32		hi;
} CPSProcessSerNum;

extern OSErr	CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern OSErr 	CPSEnableForegroundOperation( CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr	CPSSetFrontProcess( CPSProcessSerNum *psn);

#endif /* SDL_USE_CPS */

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;
static BOOL   gCalledAppMainline = FALSE;

static NSString *getApplicationName(void)
{
    const NSDictionary *dict;
    NSString *appName = 0;

    /* Determine the application name */
    dict = (const NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
    if (dict)
        appName = [dict objectForKey: @"CFBundleName"];

    if (![appName length])
        appName = [[NSProcessInfo processInfo] processName];

    return appName;
}

#if SDL_USE_NIB_FILE
/* A helper category for NSString */
@interface NSString (ReplaceSubString)
- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString;
@end
#endif

@interface NSApplication (SDLApplication)
@end

@implementation NSApplication (SDLApplication)
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

/* Set the working directory to the .app's resource directory */
- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    if (shouldChdir)
    {
        chdir([[[NSBundle mainBundle] bundlePath] UTF8String]);
        chdir("Contents/Resources");
    }
}

#if SDL_USE_NIB_FILE

/* Fix menu to contain the real app name instead of "SDL App" */
- (void)fixMenu:(NSMenu *)aMenu withAppName:(NSString *)appName
{
    NSRange aRange;
    NSEnumerator *enumerator;
    NSMenuItem *menuItem;

    aRange = [[aMenu title] rangeOfString:@"SDL App"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:appName]];

    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"SDL App"];
        if (aRange.length != 0)
            [menuItem setTitle: [[menuItem title] stringByReplacingRange:aRange with:appName]];
        if ([menuItem hasSubmenu])
            [self fixMenu:[menuItem submenu] withAppName:appName];
    }
}

#else

static void setApplicationMenu(void)
{
    /* warning: this code is very odd */
    NSMenu *appleMenu;
    NSMenuItem *menuItem;
    NSString *title;
    NSString *appName;

    appName = getApplicationName();
    appleMenu = [[NSMenu alloc] initWithTitle:@""];

    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];


    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appleMenu];
    [[NSApp mainMenu] addItem:menuItem];

    /* Tell the application object that this is now the application menu */
    [NSApp setAppleMenu:appleMenu];

    /* Finally give up our references to the objects */
    [appleMenu release];
    [menuItem release];
}

/* Create a window menu */
static void setupWindowMenu(void)
{
    NSMenu      *windowMenu;
    NSMenuItem  *windowMenuItem;
    NSMenuItem  *menuItem;

    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];

    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];

    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];

    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}

/* Replacement for NSApplicationMain */
static void CustomApplicationMain (int argc, char **argv)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    SDLMain				*sdlMain;

    /* Ensure the application object is initialised */
    [NSApplication sharedApplication];

#ifdef SDL_USE_CPS
    {
        CPSProcessSerNum PSN;
        /* Tell the dock about us */
        if (!CPSGetCurrentProcess(&PSN))
            if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
                if (!CPSSetFrontProcess(&PSN))
                    [NSApplication sharedApplication];
    }
#endif /* SDL_USE_CPS */

    /* Set up the menubar */
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    setApplicationMenu();
    setupWindowMenu();

    /* Create SDLMain and make it the app delegate */
    sdlMain = [[SDLMain alloc] init];
    [NSApp setDelegate:sdlMain];

    /* Start the main event loop */
    [NSApp run];

    [sdlMain release];
    [pool release];
}

#endif


/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    const char *temparg;
    size_t arglen;
    char *arg;
    char **newargv;

    if (!gFinderLaunch)  /* MacOS is passing command line args. */
        return FALSE;

    if (gCalledAppMainline)  /* app has started, ignore this document. */
        return FALSE;

    temparg = [filename UTF8String];
    arglen = SDL_strlen(temparg) + 1;
    arg = (char *) SDL_malloc(arglen);
    if (arg == NULL)
        return FALSE;

    newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
    if (newargv == NULL)
    {
        SDL_free(arg);
        return FALSE;
    }
    gArgv = newargv;

    SDL_strlcpy(arg, temparg, arglen);
    gArgv[gArgc++] = arg;
    gArgv[gArgc] = NULL;
    return TRUE;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    /* Set the working directory to the .app's parent directory */
    [self setupWorkingDirectory:gFinderLaunch];

#if SDL_USE_NIB_FILE
    /* Set the main menu to contain the real app name instead of "SDL App" */
    [self fixMenu:[NSApp mainMenu] withAppName:getApplicationName()];
#endif

    /* Hand off to main application code */
    gCalledAppMainline = TRUE;
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    exit(status);
}
@end


@implementation NSString (ReplaceSubString)

- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString
{
    unsigned int bufferSize;
    unsigned int selfLen = [self length];
    unsigned int aStringLen = [aString length];
    unichar *buffer;
    NSRange localRange;
    NSString *result;

    bufferSize = selfLen + aStringLen - aRange.length;
    buffer = (unichar *)NSAllocateMemoryPages(bufferSize*sizeof(unichar));

    /* Get first part into buffer */
    localRange.location = 0;
    localRange.length = aRange.location;
    [self getCharacters:buffer range:localRange];

    /* Get middle part into buffer */
    localRange.location = 0;
    localRange.length = aStringLen;
    [aString getCharacters:(buffer+aRange.location) range:localRange];

    /* Get last part into buffer */
    localRange.location = aRange.location + aRange.length;
    localRange.length = selfLen - localRange.location;
    [self getCharacters:(buffer+aRange.location+aStringLen) range:localRange];

    /* Build output string */
    result = [NSString stringWithCharacters:buffer length:bufferSize];

    NSDeallocateMemoryPages(buffer, bufferSize);

    return result;
}

@end



#ifdef main
#  undef main
#endif


/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

    /* Store the function pointers to our interop functions, first off. */
    pfnGetCaseFilePathsOSX = GetCaseFilePathsOSX;
    pfnGetSaveFilePathsForCaseOSX = GetSaveFilePathsForCaseOSX;
    pfnGetVersionStringOSX = GetVersionStringOSX;
    pfnGetPropertyListXMLForVersionStringOSX = GetPropertyListXMLForVersionStringOSX;

    /* Create our Application Support folders if they don't exist yet and store the paths */
    NSString *pStrLocalApplicationSupportPath = [[NSFileManager defaultManager] localApplicationSupportDirectory];
    NSString *pStrUserApplicationSupportPath = [[NSFileManager defaultManager] userApplicationSupportDirectory];

    /* Next, create the folders that the executable will need during execution if they don't already exist. */
    NSMutableString *pStrLocalGameApplicationSupportPath = [[NSMutableString alloc] init];
    NSMutableString *pStrCasesPath = [[NSMutableString alloc] init];
    NSMutableString *pStrUserGameApplicationSupportPath = [[NSMutableString alloc] init];
    NSMutableString *pStrDialogSeenListsPath = [[NSMutableString alloc] init];
    NSMutableString *pStrSavesPath = [[NSMutableString alloc] init];

    [pStrLocalGameApplicationSupportPath setString:pStrLocalApplicationSupportPath];
    [pStrLocalGameApplicationSupportPath appendString:[NSString stringWithUTF8String:"/My Little Investigations/"]];
    [pStrCasesPath setString:pStrLocalGameApplicationSupportPath];
    [pStrCasesPath appendString:[NSString stringWithUTF8String:"Cases/"]];
    [pStrUserGameApplicationSupportPath setString:pStrUserApplicationSupportPath];
    [pStrUserGameApplicationSupportPath appendString:[NSString stringWithUTF8String:"/My Little Investigations/"]];
    [pStrDialogSeenListsPath setString:pStrUserGameApplicationSupportPath];
    [pStrDialogSeenListsPath appendString:[NSString stringWithUTF8String:"DialogSeenLists/"]];
    [pStrSavesPath setString:pStrUserGameApplicationSupportPath];
    [pStrSavesPath appendString:[NSString stringWithUTF8String:"Saves/"]];

	NSError *error = nil;

	[[NSFileManager defaultManager]
		createDirectoryAtPath:pStrDialogSeenListsPath
		withIntermediateDirectories:YES
		attributes:nil
		error:&error];

	[[NSFileManager defaultManager]
		createDirectoryAtPath:pStrSavesPath
		withIntermediateDirectories:YES
		attributes:nil
		error:&error];

    pLocalApplicationSupportPath = [pStrLocalGameApplicationSupportPath UTF8String];
    pCasesPath = [pStrCasesPath UTF8String];
    pUserApplicationSupportPath = [pStrUserGameApplicationSupportPath UTF8String];
    pDialogSeenListsPath = [pStrDialogSeenListsPath UTF8String];
    pSavesPath = [pStrSavesPath UTF8String];

    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }

#if SDL_USE_NIB_FILE
    NSApplicationMain (argc, argv);
#else
    CustomApplicationMain (argc, argv);
#endif

    [pPool release];
    return 0;
}

const char ** GetCaseFilePathsOSX(unsigned int *pCaseFileCount)
{
	NSError *error = nil;

    NSArray *pCaseFileList =
        [[NSFileManager defaultManager]
            contentsOfDirectoryAtPath:[NSString stringWithUTF8String:pCasesPath]
            error:&error];

    unsigned int caseFileCount = [pCaseFileList count];
    const char **ppCaseFileList = (const char **)malloc((size_t)(caseFileCount * sizeof(const char *)));

    unsigned int caseFileIndex = 0;

    for (id object in pCaseFileList)
    {
        NSString *pStrCaseFileName = (NSString *)object;
        NSMutableString *pStrCaseFilePath = [[NSMutableString alloc] init];

        [pStrCaseFilePath setString:[NSString stringWithUTF8String:pCasesPath]];
        [pStrCaseFilePath appendString:pStrCaseFileName];

        ppCaseFileList[caseFileIndex++] = [pStrCaseFilePath UTF8String];
    }

    *pCaseFileCount = caseFileCount;
    return ppCaseFileList;
}

const char ** GetSaveFilePathsForCaseOSX(const char *pCaseUuid, unsigned int *pSaveFileCount)
{
	NSError *error = nil;

    NSMutableString *pStrCaseSavesFilePath = [[NSMutableString alloc] init];
    [pStrCaseSavesFilePath setString:[NSString stringWithUTF8String:pSavesPath]];
    [pStrCaseSavesFilePath appendString:[NSString stringWithUTF8String:pCaseUuid]];
    [pStrCaseSavesFilePath appendString:[NSString stringWithUTF8String:"/"]];

    [[NSFileManager defaultManager]
        createDirectoryAtPath:pStrCaseSavesFilePath
        withIntermediateDirectories:YES
        attributes:nil
        error:&error];

    NSArray *pSaveFileList =
        [[NSFileManager defaultManager]
            contentsOfDirectoryAtPath:pStrCaseSavesFilePath
            error:&error];

    unsigned int saveFileCount = [pSaveFileList count];
    const char **ppSaveFilePathList = (const char **)malloc((size_t)(saveFileCount * sizeof(const char *)));

    unsigned int saveFileIndex = 0;

    for (id object in pSaveFileList)
    {
        NSString *pStrSaveFileName = (NSString *)object;
        NSMutableString *pStrSaveFilePath = [[NSMutableString alloc] init];

        [pStrSaveFilePath setString:pStrCaseSavesFilePath];
        [pStrSaveFilePath appendString:pStrSaveFileName];

        ppSaveFilePathList[saveFileIndex++] = [pStrSaveFilePath UTF8String];
    }

    [pStrCaseSavesFilePath release];

    *pSaveFileCount = saveFileCount;
    return ppSaveFilePathList;
}

const char * GetVersionStringOSX(const char *pPropertyListFilePath)
{
    NSString *pErrorDesc = nil;
    NSPropertyListFormat format;
    NSString *pProperyListPath = [NSString stringWithUTF8String:pPropertyListFilePath];

    if (![[NSFileManager defaultManager] fileExistsAtPath:pProperyListPath])
    {
        return "";
    }

    NSData *pPropertyListXML = [[NSFileManager defaultManager] contentsAtPath:pProperyListPath];
    NSDictionary *pPropertyListDictionary =
        (NSDictionary *)[NSPropertyListSerialization propertyListFromData:pPropertyListXML
            mutabilityOption:NSPropertyListMutableContainersAndLeaves
            format:&format
            errorDescription:&pErrorDesc];

    if (pPropertyListDictionary == NULL)
    {
        return "";
    }

    NSString *pVersionString = [pPropertyListDictionary objectForKey:@"VersionString"];
    return [pVersionString UTF8String];
}

char * GetPropertyListXMLForVersionStringOSX(const char *pPropertyListFilePath, const char *pVersionString, unsigned long *pVersionStringLength)
{
    *pVersionStringLength = 0;

    NSString *pErrorDesc = nil;
    NSPropertyListFormat format;
    NSString *pProperyListPath = [NSString stringWithUTF8String:pPropertyListFilePath];

    if (![[NSFileManager defaultManager] fileExistsAtPath:pProperyListPath])
    {
        return NULL;
    }

    NSData *pPropertyListXML = [[NSFileManager defaultManager] contentsAtPath:pProperyListPath];
    NSDictionary *pPropertyListDictionary =
        (NSDictionary *)[NSPropertyListSerialization propertyListFromData:pPropertyListXML
            mutabilityOption:NSPropertyListMutableContainersAndLeaves
            format:&format
            errorDescription:&pErrorDesc];

    if (pPropertyListDictionary == NULL)
    {
        return NULL;
    }

    NSMutableDictionary *pPropertyListDictionaryMutable = [pPropertyListDictionary mutableCopy];

    [pPropertyListDictionaryMutable setObject:[NSString stringWithUTF8String:pVersionString] forKey:@"VersionString"];

    NSData *pData = [NSPropertyListSerialization
        dataFromPropertyList:(id)pPropertyListDictionaryMutable
        format:NSPropertyListXMLFormat_v1_0
        errorDescription:&pErrorDesc];

    if (pData == NULL)
    {
        return NULL;
    }

    unsigned long dataLength = [pData length];

    char *pCharData = (char *)malloc(dataLength * sizeof(char));
    [pData getBytes:(void *)pCharData length:dataLength];

    *pVersionStringLength = dataLength;
    return pCharData;
}
