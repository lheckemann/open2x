
/* For std::string */
#include <string>

/* You should include this file when you want to use SDL_Config. */
#include "SDL_config.h"

/* For autotest macros */
#include "SDL_config_auto_tests.h"

using namespace std;

#define OUTPUT stdout

/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
  CFG_WRITE_HEADER(001)
  
  /*
   The most important structure is CFG_File, we must create it in order to do anything.
   If you need to open and work on many config files sequentially, you can reuse one CFG_File -
   open file, do sth with it, maybe save, close and once again - open another file etc.
  */
  CFG_File config;
  
  /*
   Open and parse file "test_001.cfg" and place results in "config" object.
   You don't need to call any library initialization functions before.
  */
  int result = CFG_OpenFile("test_001.cfg", &config );

  /* CFG_OpenFile returns value, that allows you to check if everything went right, or do we have problems */
  if ( CFG_ERROR == result )
   {
    printf("Error: %s", CFG_GetError());
 	return 1;
   }
  else if ( CFG_CRITICAL_ERROR == result )
   {
    printf("Critical error: %s", CFG_GetError());
 	return 1;
   }

  /* It dumps out nice header to stderr */
  CFG_BEGIN_AUTO_TEST
  
  /*
   Since there aren't any other config files opened at present, we don't need to select this config
   file for further use. But if they were, you should just call CFG_SelectFile(&config);
  
   Now, we are reading integer entry and outputting its value in the next line.
   101 will be assigned if no entry called "screen_size_x" exists in currently selected group.
   
   Since we haven't explicitly opened any groups, global group is selected by default.
   */
  Sint32 value1 = CFG_ReadInt("screen_size_x", 101);
  printf("Integer value read from global group, entry *screen_size_x*: %i\n", value1 );
  
/*  ----------------------------------------- */
  
  /*
   Now we select "Video" group.
   Passing 0 as second argument means: if there's no such group, don't create it.
  */
  if ( CFG_OK == CFG_SelectGroup("Video", 0) )
   {
    /* Now, read integers */
    Sint32 value2 = CFG_ReadInt("screen_size_x", 101);
    printf("Integer value read from [Video] group, *screen_size_x* entry: %i\n", value2 );

    Sint32 value3 = CFG_ReadInt("screen_size_y", 101);
    printf("Integer value read from [Video] group, entry *screen_size_y* entry: %i\n", value3 );
    
    Sint32 value4 = CFG_ReadInt("color_depth", 31);
    printf("Integer value read from [Video] group, *color_depth* entry: %i\n", value4 );

    /*
     This entry doesn't exist, so it'll be created with value passed as second argument - 101.
     OTOH, if it existed, CFG_ReadInt would just return value of that existing entry.
    */
    Sint32 value5 = CFG_ReadInt("xxxxxxxxxxxxx", 101);
    printf("Integer value read from [Video] group, *xxxxxxxxxxxxx* entry: %i\n", value5 );
    
    /*
     This entry also doesn't exist in Video group, but it exists in global group -
     so it'll get value from global group.
     Remember, first we look in selected group, then in global group, and if it doesn't exist anywhere - create entry.
    */
    Sint32 value6 = CFG_ReadInt("bad_integer", 101);
    printf("Integer value read from [Video] group, *bad_integer* entry: %i\n", value6 );
    
    /* Read bool */
    bool value7 = CFG_ReadBool("fullscreen", false);
    printf("Bool value read from [Video] group, *fullscreen* entry: %i\n\n", value7 );

/*  ----------------------------------------- */
    
    float value8;
    CFG_String title;

    /* Select group Game, it should exist */
    if ( CFG_OK == CFG_SelectGroup("Game", 0) )
     {
      value8 = CFG_ReadFloat("version", 10.0);
      printf("Float value read from [Game] group, *version* entry: %f\n", value8 );
      
      title = CFG_ReadText("title", "error");
      printf("Text value read from [Game] group, *title* entry: %s\n\n", title.c_str() );

     } else CFG_LOG_OTHER_ERROR("Couldn't open group \"Game\" (without creating it).")
    
    /*  ----------------------------------------- */
    /*                  AUTO TEST                 */
    /*  ----------------------------------------- */
    
    /* Those are auto tests that compare results of what should be, with what we got. */
    CFG_AUTO_TEST_INTEGER(value1, 101);
    CFG_AUTO_TEST_INTEGER(value2, 640);
    CFG_AUTO_TEST_INTEGER(value3, 480);
    CFG_AUTO_TEST_INTEGER(value4, 32);
    CFG_AUTO_TEST_INTEGER(value5, 101);
    CFG_AUTO_TEST_INTEGER(value6, 1234);
    CFG_AUTO_TEST_BOOL(value7, true);
    
    CFG_AUTO_TEST_FLOAT(value8, 0.01);
    CFG_AUTO_TEST_TEXT(title.c_str(), "Jump n' RotN");

   }
    else CFG_LOG_OTHER_ERROR("Couldn't open group \"Video\" (without creating it).")

/*  ----------------------------------------- */
    
  /* Remove group Game, testing returned value */
  CFG_AUTO_TEST_INTEGER(CFG_RemoveGroup("Game"), CFG_OK);
  
  /* Test once more" */
  CFG_AUTO_TEST_BOOL( CFG_GroupExists("Game"), CFG_False);
  
  
  /* Select group Video, testing returned value */
  CFG_AUTO_TEST_INTEGER(CFG_SelectGroup("Video", 0), CFG_OK);
  
  /* Test once more" */
  CFG_AUTO_TEST_BOOL( CFG_GroupExists("Video"), CFG_True);

/*  ----------------------------------------- */

  /* Remove "fullscreen" entry from group Video */
  CFG_AUTO_TEST_INTEGER(CFG_RemoveBoolEntry("fullscreen"), CFG_OK);
  
  /* Remove "fgfdgfdg" entry from group Video, which doesn't exist - so we should get CFG_ERROR */
  CFG_AUTO_TEST_INTEGER(CFG_RemoveIntEntry("fgfdgfdg"), CFG_ERROR);
  
  /* Try to remove "screen_size_y" entry from group Video,
  which exists but have different type - so it won't be removed */
  CFG_AUTO_TEST_INTEGER(CFG_RemoveFloatEntry("screen_size_y"), CFG_ERROR);

/*  ----------------------------------------- */
  
  /* Check if "fullscreen" exists */ 
  CFG_AUTO_TEST_BOOL( CFG_BoolEntryExists("fullscreen"), CFG_False);

  /* Check if "good_integer" exists */
  CFG_AUTO_TEST_BOOL( CFG_IntEntryExists("good_integer"), CFG_False);

  /* Check if "bad_integer" exists */
  CFG_AUTO_TEST_BOOL( CFG_TextEntryExists("bad_integer"), CFG_True);
  
/*  ----------------------------------------- */

  /* Save current state to file */
  CFG_SaveFile("test_001_final.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);
  CFG_SaveFile("test_001_compressed.cfg", CFG_SORT_ORIGINAL, CFG_COMPRESS_OUTPUT);
   
  /* Close selected file (we would pass to it 'config' variable instead of 0, if it wasn't selected) */
  CFG_CloseFile(0);

  CFG_AUTO_SUMMARY
  
  return 0;
 }
