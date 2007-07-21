
#include <string>

#include "SDL_config.h"

#include "SDL_config_auto_tests.h"

using namespace std;

/*

  Test 003 - what it does:

   1. Opens file "test_003.cfg"
   2. Reads two variables from global group, and two variables from group "Foobar".
   3. Changes their values
   4. Reads value that doesn't exist
   5. Creates new group with few new entries.
   6. Saves new state to file "test003_generated.ini" (to prove that extensions are irrelevant)
   
*/

/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
  CFG_WRITE_HEADER(003)

  CFG_File config;

  int result = CFG_OpenFile("test_003.cfg", &config );
   
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

  CFG_BEGIN_AUTO_TEST

  Sint32 value1 = CFG_ReadInt("xyz", 0);
  fprintf(stderr, "Integer value read from global group, entry *xyz*: %i\n", value1 );

  bool value2 = CFG_ReadBool("Koshmaar_is_stupid", false);
  fprintf(stderr, "Bool value read from global group, entry *Koshmaar_is_stupid*: %i\n", value2 );

/*  ----------------------------------------- */

  if ( CFG_OK == CFG_SelectGroup("Foobar", 0) )
   {
    float value3 = CFG_ReadFloat("PI", 66.6f);
    fprintf(stderr, "Float value read from [Foobar] group, *PI* entry: %f\n", value3 );

    CFG_String value4 = CFG_ReadText("what_is_the_best_configuration_parsing_library_in_whole_wide_world?",
                                     "error");
    fprintf(stderr, "Text value read from [Foobar] group, *what_is_the_best_configuration_parsing_library_in_whole"
                    "_wide_world?* entry: %s\n\n", value4.c_str() );


    CFG_AUTO_TEST_INTEGER(value1, -99999);
    CFG_AUTO_TEST_BOOL(value2, true);

    CFG_AUTO_TEST_FLOAT(value3, -3.14);
    CFG_AUTO_TEST_TEXT(value4.c_str(), "definitely not SDL_Config");

/*  ----------------------------------------- Write integers */

    fprintf(stderr, "Now we're going to write some values\n\n");
    
    if (CFG_SelectGroup(0, 0) != CFG_OK)
     fprintf(stderr, "CFG_SelectGroup(0, 0) != CFG_OK");

    CFG_WriteInt("xyz", 123456);
    CFG_WriteInt("creation", 777); /* this should create new entry */

    value1 = CFG_ReadInt("xyz", 0);
    fprintf(stderr, "Integer value read from global group, entry *xyz*: %i\n", value1 );
    
    Sint32 value5 = CFG_ReadInt("creation", 0);
    fprintf(stderr, "Integer value read from global group, entry *creation*: %i\n", value5 );
    
    CFG_AUTO_TEST_INTEGER(value1, 123456);
    CFG_AUTO_TEST_INTEGER(value5, 777);
    
/*  ----------------------------------------- Write bool, float and text */
    
    CFG_WriteBool("Koshmaar_is_stupid", false);
    
    value2 = CFG_ReadBool("Koshmaar_is_stupid", false);
    fprintf(stderr, "Bool value read from global group, entry *Koshmaar_is_stupid*: %i\n", value2 );
    
    CFG_AUTO_TEST_BOOL(value2, false);


    if ( CFG_OK != CFG_SelectGroup("Foobar", 0) )
     CFG_LOG_OTHER_ERROR("Couldn't select group 'Foobar'")

    CFG_WriteFloat("PI", 3.14f);
     
    value3 = CFG_ReadFloat("PI", 66.6f);
    fprintf(stderr, "Float value read from [Foobar] group, *PI* entry: %f\n", value3 );
    
    CFG_AUTO_TEST_FLOAT(value3, 3.14);
    
    
    CFG_WriteText("what_is_the_best_configuration_parsing_library_in_whole_wide_world?", "SDL_Config");

    value4 = CFG_ReadText("what_is_the_best_configuration_parsing_library_in_whole_wide_world?",
                                     "error");
    fprintf(stderr, "Text value read from [Foobar] group, *what_is_the_best_configuration_parsing_library_in_whole"
                    "_wide_world?* entry: %s\n\n", value4.c_str() );

    CFG_AUTO_TEST_TEXT(value4.c_str(), "SDL_Config");


   }
    else { CFG_LOG_OTHER_ERROR("Couldn't open group \"Foobar\"") }
    
/*  ----------------------------------------- Create additional group and write to it some values  */

  /* such group not exists, so it will be created */
  if ( CFG_GROUP_CREATED != CFG_SelectGroup("Newly_created_group", CFG_True))
   CFG_LOG_OTHER_ERROR("Couldn't create new group.")
  
  CFG_WriteFloat("Float", 55.55f);
  CFG_WriteBool("false", true);
  CFG_WriteInt("rudy", 102);
  CFG_WriteText("123456", "xyz");

  if ( CFG_OK != CFG_SaveFile("test_003_generated.ini", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES) )
    CFG_LOG_OTHER_ERROR("Couldn't save file.")
    
  CFG_SaveFile("test_003_compressed.ini", CFG_SORT_ORIGINAL, CFG_COMPRESS_OUTPUT);

  CFG_CloseFile(0);

  CFG_AUTO_SUMMARY

  return 0;
 }
