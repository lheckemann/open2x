
#include <ctime>

/* For std::string */
#include <string>

/* You should include this file when you want to use SDL_Config. */
#include "SDL_config.h"

/* For autotest macros */
#include "SDL_config_auto_tests.h"

using namespace std;

#define PROFILE_LIB

CFG_String GenerateRandomEntryName()
 {
  CFG_String tmp;
  
  for (int i = 0; i < (rand() % 10) + 3; ++i)
   tmp += (rand() % (128 - 35)) + 35;

  return tmp;
 }

/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
  CFG_WRITE_HEADER(005)
  
  #ifdef PROFILE_LIB
  
  SDL_Init(SDL_INIT_TIMER);
  
  #endif

  
  /*
   The most important structure is CFG_File, we must create it in order to do anything.
   If you need to open and work on many config files sequentially, you can reuse one CFG_File -
   open file, do sth with it, maybe save, close and once again - open another file etc.
  */
  CFG_File config_syntax, config_comments;

  /*
   Open and parse file "test_001.cfg" and place results in "config" object.
   You don't need to call any library initialization functions before.
  */
  int result = CFG_OpenFile("test_005_syntax_stress.cfg", &config_syntax );

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
   
  result = CFG_OpenFile("test_005_comments_stress.cfg", &config_comments );

  /* CFG_OpenFile returns value, that allows you to check if everything went right, or do we have problems */
  if ( CFG_ERROR == result )
   {
    fprintf(stderr, "Error: %s", CFG_GetError());
    CFG_CloseFile(&config_syntax);
 	return 1;
   }
  else if ( CFG_CRITICAL_ERROR == result )
   {
    fprintf(stderr, "Critical error: %s", CFG_GetError());
    CFG_CloseFile(&config_syntax);
 	return 1;
   }

  /* Save current state to file */
  CFG_SaveFile("test_005_syntax_stress_generated.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);
  
  CFG_SelectFile(&config_comments);
  CFG_SaveFile("test_005_comments_stress_generated.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);
  

  /* Close selected file (we would pass to it 'config' variable instead of 0, if it wasn't selected) */
  CFG_CloseFile(&config_syntax);
  CFG_CloseFile(&config_comments);
  
  /* ------------------------------------------------------ Profile speed */
  
  srand(time(0));
  
  CFG_File config_speed;
  
  #ifdef PROFILE_LIB
  fprintf(stdout, "\n\n");
  int profile_all = 5;
  int total_generation_time = 0;
  int total_parsing_time = 0;
  int total_saving_time = 0;
  #else
  int profile_all = 1;
  #endif
  
  for (int k = 0; k < profile_all; ++k)
   {
    result = CFG_OpenFile(0, &config_speed);
    
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

    const int create_groups = 2000;
    const int create_booleans = 5;
    const int create_integers = 10;
    const int create_floats = 7;
    const int create_texts = 4;

  
    #ifdef PROFILE_LIB

    Uint32 time_start = SDL_GetTicks();

    #endif

    for (int i = 0; i < create_groups; ++i)
     {
      CFG_SelectGroup( GenerateRandomEntryName().c_str(), CFG_True );

      for (int j = 0; j < create_booleans; ++j)
       {
        CFG_WriteBool( GenerateRandomEntryName().c_str(), (rand() % 100) > 50 );
       }

      for (int j = 0; j < create_integers; ++j)
       {
        CFG_WriteInt(GenerateRandomEntryName().c_str(), (rand() % 20000 ) - 5000 );
       }

      for (int j = 0; j < create_floats; ++j)
       {
        CFG_WriteFloat(GenerateRandomEntryName().c_str(), ((rand() % 20000 ) - 5000.0f) / 123.0f );
       }

      for (int j = 0; j < create_texts; ++j)
       {
        CFG_WriteText(GenerateRandomEntryName().c_str(), GenerateRandomEntryName().c_str() );
       }
     }
   
    #ifdef PROFILE_LIB

    Uint32 time_end = SDL_GetTicks();

    Uint32 time_diff = time_end - time_start;

    fprintf(stdout, "\n test_005_speed_stress_generated.cfg >>> >>> Generated in: %i ms\n", time_diff);
    
    total_generation_time += time_diff;

    #endif


    #ifdef PROFILE_LIB

    time_start = SDL_GetTicks();

    #endif

    CFG_SaveFile("test_005_speed_stress_generated.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);

    #ifdef PROFILE_LIB

    time_end = SDL_GetTicks();
    time_diff = time_end - time_start;

    fprintf(stdout, "\n >>> >>> Saved in: %i ms\n\n", time_diff);
    total_saving_time += time_diff;

    #endif

    /* Closes currently selected file */
    CFG_CloseFile(0);
    
    #ifdef PROFILE_LIB

    time_start = SDL_GetTicks();

    #endif

    result = CFG_OpenFile("test_005_speed_stress_generated.cfg", &config_speed);

    #ifdef PROFILE_LIB

    time_end = SDL_GetTicks();
    time_diff = time_end - time_start;

    fprintf(stdout, "\n >>> >>> Parsed in: %i ms\n\n", time_diff);
    total_parsing_time += time_diff;

    #endif

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

   CFG_CloseFile(0);
   
   #ifdef PROFILE_LIB

   fprintf(stdout, "\n --------------------- \n");

   #endif

  }

  #ifdef PROFILE_LIB
  
  fprintf(stdout, "\n\n >>> >>> Total average time of generation: %i ms\n", total_generation_time / profile_all );
  
  fprintf(stdout, "\n\n >>> >>> Total average time of parsing: %i ms\n", total_parsing_time / profile_all );
  fprintf(stdout, "\n >>> >>> Total average time of saving: %i ms\n", total_saving_time / profile_all );

  SDL_Quit();

  #endif

  return 0;
 }
