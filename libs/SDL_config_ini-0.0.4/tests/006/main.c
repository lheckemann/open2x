
/*

 This test program should compile on Visual Studio, and probably won't compile on GCC / MinGW.
 It will be finished / debugger later. Compile at your own risk :-)
 
 Currently it compiles and runs without any problems (?), but there are some strange errors when anything else is uncommented.
 Try uncommenting CFG_WRITE_HEADER(006); and see what happens. If you know what causes those errors and how they can be
 fixed, please write to me.

*/


/* You should include this file when you want to use SDL_Config. */

#include "stdlib.h"

#include "SDL_config.h"

#include "SDL_config_auto_tests.h"


/*  ----------------------------------------- */

int result;

/* Check if everything went right, or do we have problems */
void CheckErrors(void)
 {
  if ( CFG_ERROR == result )
   {
    fprintf(stderr, "Error: %s", CFG_GetError());
 	exit (1);
   }
  else if ( CFG_CRITICAL_ERROR == result )
   {
    fprintf(stderr, "Critical error: %s", CFG_GetError());
 	exit (1);
   }
  else
   fprintf(stderr, "\nAll OK.\n\n");
 }


/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
 
  /* CFG_WRITE_HEADER(006); Next line can't compile with thise :-/ */
  
    
  CFG_File config;
    
  /*
   Open and parse file "test_001.cfg".
   There's no need to create any special objects or call initialization functions before.
  */

  int result = CFG_OpenFile("test_001.cfg", &config );
    
  /*
   Now, we are reading integer entry and outputting its value in the next line.
   101 will be assigned if no entry called "screen_size_x" exists.
   */
  
    
  int value1 = CFG_ReadInt("screen_size_x", 101);
  fprintf(stderr, "Integer value read from global group, entry *screen_size_x*: %i\n", value1 );

  CheckErrors();
  

  /* Selects "Video" group. */
  if ( CFG_OK == CFG_SelectGroup("Video", 0) )
   {
    /* read integers */
   /* Sint32 value2 = CFG_ReadInt("screen_size_x", 101);
    fprintf(stderr, "Integer value read from [Video] group, *screen_size_x* entry: %i\n", value2 );

    Sint32 value3 = CFG_ReadInt("screen_size_y", 101);
    fprintf(stderr, "Integer value read from [Video] group, entry *screen_size_y* entry: %i\n", value3 );

    Sint32 value4 = CFG_ReadInt("color_depth", 32);
    fprintf(stderr, "Integer value read from [Video] group, *color_depth* entry: %i\n", value4 );*/

    /* This entry doesn't exist. */
    /*Sint32 value5 = CFG_ReadInt("dasdad", 101);
    fprintf(stderr, "Integer value read from [Video] group, *dasdad* entry: %i\n", value5 );*/

    /*
     This entry also doesn't exist in Video group, but it exists in global group -
     so it'll get value from global group.
     */
    /*Sint32 value6 = CFG_ReadInt("bad_integer", 101);
    fprintf(stderr, "Integer value read from [Video] group, *bad_integer* entry: %i\n", value6 );*/

    /* Read bool */
    int value7 = CFG_ReadBool("fullscreen", 0);
    fprintf(stderr, "Bool value read from [Video] group, *fullscreen* entry: %i\n\n", value7 );

    /*float value8;
    const char * title;*/


    /* Select group Game, it should exist */
    if ( CFG_OK == CFG_SelectGroup("Game", 0) )
     {
      /*value8 = CFG_ReadFloat("version", 10.0);
      fprintf(stderr, "Float value read from [Game] group, *version* entry: %f\n", value8 );

      title = CFG_ReadText("title", "error");
      fprintf(stderr, "Text value read from [Game] group, *title* entry: %s\n\n", title );*/

     } else CFG_LOG_OTHER_ERROR("Couldn't open group \"Game\" (without creating it).")

    /*  ----------------------------------------- */
    /*                  AUTO TEST                 */
    /*  ----------------------------------------- */

    /* Those are auto tests that compare results of what should be, with what we got. */
    /*CFG_AUTO_TEST_INTEGER(value1, 101);
    CFG_AUTO_TEST_INTEGER(value2, 640);
    CFG_AUTO_TEST_INTEGER(value3, 480);
    CFG_AUTO_TEST_INTEGER(value4, 32);
    CFG_AUTO_TEST_INTEGER(value5, 101);
    CFG_AUTO_TEST_INTEGER(value6, 1234);
    CFG_AUTO_TEST_BOOL(value7, 1);

    CFG_AUTO_TEST_FLOAT(value8, 0.01);
    CFG_AUTO_TEST_TEXT(title, "Jump n' RotN");*/

   }
    else CFG_LOG_OTHER_ERROR("Couldn't open group \"Video\" (without creating it).")

  /* Close selected file (we would pass to it 'config' variable if it wasn't selected)*/
  CFG_CloseFile(0);

  CFG_AUTO_SUMMARY

  return 0;
 }
