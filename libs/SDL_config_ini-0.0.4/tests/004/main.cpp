
/*

 Short description of test program: loads file "test_001.cfg" (1) and saves it to "test_004.gz" (2),
                                    which is extracted, and file that was saved there  is once again loaded (3),
                                    and saved to "test004_generated.cfg" (4).
 

 This test program was created with help of this tutorial: http://www.kekkai.org/roger/sdl/rwops/rwops.html

 It uses one type of SDL_RWops stream from SDL_RWlib (http://burningsmell.org/SDL_rwlib/) which allows it to
 load / save config file from / to .gz file. Three files that contain source of that stream are included with this test:

  - SDL_rwz.h
  - SDL_rwz.c
  - SDL_rwlib_internal.h
  
 They were modified by me slightly, since I wasn't able to compile them on VisualC++. 
 I really encourage you to download whole library, since there are implemented many other useful streams.

 Apart from that library, if you want to compile this test program and/or use SDL_RWlib in your own project,
 you'll need also to install developement version of zlib (http://www.zlib.net/).
 

 The most serious drawback of .gz files, is thay they can contain only one file. If you are interested in .zip files, then I
 have good news for you: there is Zziplib library (http://zziplib.sourceforge.net), which allows you to operate on .zip files.
 
 What's even better, author of Zziplib was so kind that he already created necessary SDL_RWops stream type
 for reading from .zip files (writing is not supported). See: http://zziplib.sourceforge.net/zzip-sdl-rwops.html

 Unfortunately, I wasn't able to install and compile it, so it's not tested.
 However, I have tested reading and writing to FILE stream (created using SDL_RWFromFile internally in SDL_config.c,
 when you're using plain CFG_OpenFile and CFG_SaveFile) and to .gz stream (SDL_RWFromGZ in this test),
 and they both work well. So, I would be very surprised if it didn't work. In theory, just change SDL_RWFromGZ
 to SDL_RWFromZZIP and it *should* work fine.
 
 If it works fine, that's great - mail to me that it's working. If not, also mail me that there's bug somewhere,
 so I will try to find it :-)

 -----------------------------------------
 
 There's one little problem with file generated to test_004.gz - if you will extract it and open in text editor, you will see
 that line endings have been saved in other way than with '\n'. Compare file from inside test_004.gz with
 test_004_generated.cfg to see what I mean. Problem is related with that you can't pass only "w" to write
 to file in non-binary way. I don't know how to fix it by myself, sot if you know what to do it (or know any good
 workaround) - mail me.

*/

#include <cstdlib>

/* You should include this file when you want to use SDL_Config. */
#include "SDL_config.h"

/* Included for auto tests */
#include "SDL_config_auto_tests.h"

/* For Gzip stream */
#include "SDL_rwz.h"

/*  ----------------------------------------- */

int result;

/* Check if everything went right, or do we have problems */
void CheckErrors()
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
   fprintf(stderr, "\nSo far, all OK.\n");
 }

using namespace std;

/*  ----------------------------------------- */

int main(int argc, char **argv)
 {
  CFG_WRITE_HEADER(004)

  CFG_File config;
  
  /* This is the RWops structure we'll be using */
  SDL_RWops * source;

  /* (1) */
  result = CFG_OpenFile("test_001.cfg", &config);

  fprintf(stderr, "\n\n");
  
  CheckErrors();

  /* Create new gz stream for writing */
  source = SDL_RWFromGZ("test_004.gz", "wb");
  
  if (source == 0)
   {
    fprintf(stderr, "Error: Couldn't open gz stream for writing: %s", SDL_GetError());
    return 1;
   }
   
  /* (2) */
  result = CFG_SaveFile_RW(source, CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);
  
  CheckErrors();
  
  /* We must close stream in order to flush it and generate .gz file */
  SDL_RWclose(source); 
  
  CFG_CloseFile(0);

/*  ----------------------------------------- */
  
  source = SDL_RWFromGZ("test_004.gz", "rb");

  /*
   Open and parses file "test_001.cfg" from decompressed stream.
   (3)
  */
  result = CFG_OpenFile_RW(source, &config );
  
  CheckErrors();
  
  /* (4) */
  result = CFG_SaveFile("test_004_generated.cfg", CFG_SORT_ORIGINAL, CFG_SPACE_ENTRIES);
  CFG_SaveFile("test_004_compressed.cfg", CFG_SORT_ORIGINAL, CFG_COMPRESS_OUTPUT);
  
  CheckErrors();
  
  SDL_RWclose(source);

  /* Close selected file (we would pass to it 'config' variable if it wasn't selected)*/
  CFG_CloseFile(0);
  
  fprintf(stderr, "\nAll tests passed.");

  return 0;
 }
