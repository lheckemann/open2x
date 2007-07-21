
/* For std::string */
#include <string>

/* You should include this file when you want to use SDL_Config. */
#include "SDL_config.h"

/* For autotest macros */
#include "SDL_config_auto_tests.h"

using namespace std;

#define GROUP_TO_BE_REMOVED "AAA"

/*

 This test generally should be compiled with SDL_Config build with CFG_REMOVE_GROUP_SPACES

*/

/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
  CFG_WRITE_HEADER(002)

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
  int result = CFG_OpenFile("test_002.cfg", &config );

  /* CFG_OpenFile returns value, that allows you to check if everything went right, or do we have problems */
  if ( CFG_ERROR == result )
   {
    fprintf(stderr, "Error: %s", CFG_GetError());
 	return 1;
   }
  else if ( CFG_CRITICAL_ERROR == result )
   {
    fprintf(stderr, "Critical error: %s", CFG_GetError());
 	return 1;
   }

  /*  ----------------------------------------- */

  /* It dumps out nice header to stderr */
  CFG_BEGIN_AUTO_TEST
  
  fprintf(stderr, "Number of groups: %i\n", CFG_GetGroupCount() );
  
  int i = 0;
  
  fprintf(stderr, "Next goes list of all groups obtained by iteration:\n\n ----------\n\n" );
  
  CFG_SetIterationDirection( CFG_ITERATE_REVERSE );

  for ( CFG_StartGroupIteration(); !CFG_IsLastGroup(); CFG_SelectNextGroup())
   {
    fprintf(stderr, "%s\n", CFG_GetSelectedGroupName() );
    ++i;
   }
   
  fprintf(stderr, "\n ----------\n");
   
  CFG_AUTO_TEST_INTEGER(i, CFG_GetGroupCount() )
  
  /*  ----------------------------------------- */

  int removed_group = 0;
  
  fprintf(stderr, "Next goes list of all groups obtained by iteration:\n\n ----------\n\n" );
  
  for ( CFG_StartGroupIteration(); !CFG_IsLastGroup(); CFG_SelectNextGroup())
   {
    fprintf(stderr, "%s\n", CFG_GetSelectedGroupName() );

    if (!strcmp(CFG_GetSelectedGroupName(), GROUP_TO_BE_REMOVED ) )
     {
      CFG_RemoveSelectedGroup();
      fprintf(stderr, "Removed currently selected group \""  GROUP_TO_BE_REMOVED "\".\n");
      removed_group = 1;
     }
   }
   
  fprintf(stderr, "\n ----------\n");
   
  CFG_AUTO_TEST_INTEGER( 1, removed_group )

  CFG_AUTO_TEST_INTEGER(i - 1, CFG_GetGroupCount() )

  /*  ----------------------------------------- */
  
  CFG_AUTO_TEST_INTEGER( CFG_OK, CFG_SelectGroup("xxx") );

  CFG_AUTO_TEST_INTEGER ( 2147483647, CFG_ReadInt("good_1", 0 ) ); /* 2147483647 */
  CFG_AUTO_TEST_INTEGER ( -2147483648, CFG_ReadInt("good_2", 0 ) ); /* -2147483648 */
  
  CFG_AUTO_TEST_INTEGER ( 0, CFG_ReadInt("bad_1", 0 ) ); /* 2147483648 -> text */
  CFG_AUTO_TEST_INTEGER ( 0, CFG_ReadInt("bad_2", 0 ) ); /* -2147483649 -> text */
  CFG_AUTO_TEST_INTEGER ( 0, CFG_ReadInt("bad_3", 0 ) ); /* 123456789123456789123456789123456789123456789123456 -> text */
  
  /* "2147483648" */
  CFG_AUTO_TEST_TEXT ( "2147483648", CFG_ReadText("bad_1", "0" ) );
  
  /* "-2147483649" */
  CFG_AUTO_TEST_TEXT ( "-2147483649", CFG_ReadText("bad_2", "0" ) );
  
  /* "123456789123456789123456789123456789123456789123456" */
  CFG_AUTO_TEST_TEXT ( "123456789123456789123456789123456789123456789123456", CFG_ReadText("bad_3", "0" ) );

  /*  ----------------------------------------- */
   
  /* Save current state to file */
  CFG_SaveFile("test_002_generated.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);

  /* Close selected file (we would pass to it 'config' variable instead of 0, if it wasn't selected) */
  CFG_CloseFile(0);

  CFG_AUTO_SUMMARY

  return 0;
 }
