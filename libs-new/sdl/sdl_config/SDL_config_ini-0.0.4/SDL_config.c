
/*

    _____ ____  __       ______            _____
   / ___// __ \/ /      / ____/___  ____  / __(_)___ _
   \__ \/ / / / /      / /   / __ \/ __ \/ /_/ / __  /
  ___/ / /_/ / /___   / /___/ /_/ / / / / __/ / /_/ /
 /____/_____/_____/   \____/\____/_/ /_/_/ /_/\__, /
                                             /____/


 Library for reading and writing configuration files in an easy, cross-platform way.

 - Author: Hubert "Koshmaar" Rutkowski
 - Contact: koshmaar@poczta.onet.pl
 - Version: 0.4
 - Homepage: http://sdl-cfg.sourceforge.net/
 - License: GNU LGPL

*/

#include "SDL_config.h"

#ifdef __cplusplus

 using namespace std;

 #include <fstream>
 #include <string>
 #include <map>
 #include <vector>
 #include <sstream>

 #include <stdlib.h>

 #include "SDL_config_types.h"

#endif

#ifndef __cplusplus

 #error What, you want to compile file full of std::string and std::map without C++?

#endif

/*

 TODO: wrap this code into handy CFG_FOUND_ERROR macro:

		CFG_String errorMsg = "Found error in line ";
		errorMsg += current_line;
		errorMsg += " of file \"";
        errorMsg += CFG_Internal_filename;
        errorMsg += "\", error: couldn't classify this line: \n";
        errorMsg += buffer;

        CFG_DEBUG("! Error occured: %s\n", errorMsg.c_str())
        // maybe also CFG_SetError(errorMsg.c_str()) ?


*/

/* ------------------------------------------------------- start globals */

/* ---> Globals used for loading: */

/* Currently selected file */
static CFG_File * CFG_Internal_selected_file = 0;

/* DEPRECATED: Name of file we are currently parsing
static CFG_String CFG_Internal_filename;*/

/* Line number we are currently parsing */
static Uint32 CFG_Internal_line;

/* DEPRECATED: Number of files that were opened and not closed
static Uint32 CFG_Internal_files_open = 0;*/

/* Value of last found pre comment */
static CFG_String CFG_Internal_pre_comment;

/* Type of last found pre comment ( 0 - no comment, 1 - // ; # and one line C like, 2 - multiline C like */
static int CFG_Internal_pre_comment_type;

/* Equals 1 if we're inside C like comment */
static int CFG_Internal_c_like_comment;

/* Equals 1 if last line was empty */
static int CFG_Internal_last_line_empty;

/*
 It doesn't have any CFG_Internal_ prefix since it was made global long after number of places where it is used,
 became so high that it would take too much time and be too annoying to change this variable's name.
 
 It's used only in CFG_Internal_ParseLine.
*/
static CFG_String parsed;


/* ---> Globals used for saving: */

/* Flags that were passed to CFG_SaveFile */
static int CFG_Internal_save_file_flags;

/* Equals to 1 if it's not the first entry in group, used for saving entries */
static int CFG_Internal_not_first_entry;


/* Character wasn't found in string if std :: string :: find() returned this. */
#define CFG_NPOS (static_cast< basic_string <char> :: size_type> (-1))

#define CFG_NO_COMMENT 0
#define CFG_STD_COMMENT 1
#define CFG_C_LIKE_COMMENT 2

/*
   Hmmm, appareantly it might not be needed when using std::wstring, as I thought before...
   simply, in SaveXXX we would be saving twice as many chars as needed
*/
#define CFG_CHAR_SIZE (sizeof( CFG_Char ))


/* ------------------------------------------------------- end globals */

/* ------------------------------------------------------- start declarations of internal functions */

/*
  Parses buffer, creates new groups, entries etc.
*/

void CFG_Internal_ParseLine(CFG_Internal_File * internal_file, const CFG_Char * buffer, int line_length);

/* ------------------------------------------------------- end declarations of internal functions */

/* ------------------------------------------------------- start CFG_LinkedVersion */

float CFG_LinkedVersion(void)
 {
  return 0.3f;
 }

/* ------------------------------------------------------- end CFG_LinkedVersion */

/* ------------------------------------------------------- start CFG_Internal_string_equals */

/* Portable case-insensitive string compare function

  Used to compare bools in CFG_Internal_ParseLine.
  
*/

int CFG_Internal_StringEquals(const char * s1, const char * s2,  unsigned int n)
 {
  if (n == 0)
   return 0;

  while ((n-- != 0) && (tolower(*(unsigned char *) s1) == tolower(*(unsigned char *) s2)))
   {
    if ( (n == 0) || (*s1 == '\0') || (*s2 == '\0') )
     return 0;
    s1++;
    s2++;
   }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
 }

/* ------------------------------------------------------- end CFG_Internal_string_equals */

/* ------------------------------------------------------- start CFG_Internal_isspace */

/*
 You may wonder why I'm not using isspace() from C-stdlib - I wanted to eliminate function call overhead
 (note CFG_INLINE), since isspace() would be called for all characters in file - and that could be painful.
*/

CFG_INLINE int CFG_Internal_isspace ( int ch )
 {
  return (unsigned int)(ch - 9) < 5u  ||  ch == ' ';
 }
 
/* ------------------------------------------------------- end CFG_Internal_isspace */

/* ------------------------------------------------------- start CFG_Internal_IsComment */

/*
 Returns

  1 if //, ; or # comment starts at *ptr
  2 if /* comment starts at *ptr
  0 when neither of them starts

*/

int CFG_Internal_IsComment(const CFG_Char * ptr)
 {
  if ( ( ((*ptr) == CFG_COMMENT_3) || ((*ptr) == CFG_COMMENT_2) ||
       ( ((*ptr) == CFG_COMMENT_1) && ( *(ptr + 1) != 0 ) && ( *(ptr + 1) == CFG_COMMENT_1 ))) )
   return 1;

  if ( ((*ptr) == CFG_COMMENT_C1) && ( *(ptr + 1) != 0 ) && ( *(ptr + 1) == CFG_COMMENT_C2 ) )
   return 2;

  return 0;
 }

/* ------------------------------------------------------- end CFG_Internal_IsComment */

/* ------------------------------------------------------- start CFG_Internal_Assert_Null */

void CFG_Internal_Assert_Null(const char * function, const char * file, int line)
 {
  fprintf(stderr, "Application which uses SDL_Config library (version: %3.1f) has created"
                  " an errorneus situation: pointer to selected file is equal to NULL (0) at %i line"
                  " of file %s, so that function %s can't continue.", CFG_LinkedVersion(), line, file, function);
 }

/* ------------------------------------------------------- end CFG_Internal_Assert_Null */

/* ------------------------------------------------------- start CFG_Internal_RWfind */

/*

 This function performs search for char xxx and returns its index (or -1 if it wasn't found).
 To be more efficient, it loops and reads buffer_size bytes from stream and if xxx is there, it rewinds
 to the state from before all read opererations.
 If error occured, it returns -2.
    
*/

int CFG_Internal_RWfind( SDL_RWops * source, char xxx, int * last_line_length)
 {
  const int buffer_size = 32;
  
  char buffer [buffer_size];
  
  int read; /* number of bytes from last read */
  int read_all = 0; /* number of bytes that were read up to now */
  char * buffer_ptr;
  
  int offset_before_read;
  int offset_after_read;
  
  do
   {
    offset_before_read = SDL_RWtell(source);
    SDL_RWread(source, buffer, 1, buffer_size );
    
    offset_after_read = SDL_RWtell(source);
    read = offset_after_read - offset_before_read;

    if (read < 0)
     {
      CFG_LOG_CRITICAL_ERROR(002);
      CFG_SetError("Couldn't read even one block from stream.");
      return -2;
     }
    
    buffer_ptr = buffer;

    while ( (buffer_ptr - buffer) < read )
     {
      if ( (*buffer_ptr) == xxx)
       {
        SDL_RWseek(source, -(read_all + read ), SEEK_CUR);
        return ( buffer_ptr - buffer + read_all + 1);
       }
       
      ++buffer_ptr;
     }
    
   read_all += read;
    
  } while ( read == buffer_size );

  /*
   This means:

   - that we didn't find newline in read lines
   - this may be file which consists of one line
   - it's last line of file
   - whole file is smaller than buffer_size
   
   In all cases, in last_line_length we return length (instead of normal function returned value).
   */
  *last_line_length = read_all + 1; /* + 1 for terminating zero */
  
  /* Rewind to the state before search */
  SDL_RWseek(source, -read_all, SEEK_CUR);
  return -1;
 }

/* ------------------------------------------------------- end CFG_Internal_RWfind */


/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                Start file open operations              */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_Internal_OpenFile */

int CFG_Internal_OpenFile( SDL_RWops * source, CFG_File * file, CFG_String_Arg filename )
 {
  /* Check to make sure we can seek in this stream */
  int position = SDL_RWtell(source);
  
  if ( position < 0 )
   {
    CFG_SetError( "Can't seek in stream." );
    return CFG_CRITICAL_ERROR;
   }

/* ------------------------------------------------- Initialization */

  CFG_Internal_line = 1;
  CFG_Internal_pre_comment.erase();
  CFG_Internal_c_like_comment = 0;
  CFG_Internal_pre_comment_type = CFG_NO_COMMENT;
  CFG_Internal_last_line_empty = 0;

  file->internal_file = new CFG_Internal_File;

  if (file->internal_file == 0)
   {
    CFG_SetError("Couldn't allocate memory for file.");
	return CFG_CRITICAL_ERROR;
   }

  /* select global group */
  file->internal_file->selected_group = &file->internal_file->global_group;  
  file->internal_file->global_group.next_entry_order = 0;

  file->internal_file->next_group_order = 0;
  file->internal_file->iteration_direction = CFG_ITERATE_NORMAL;
  file->internal_file->filename = filename;

/* -------------------------------------------------------  */

  CFG_DEBUG("\nSDL_Config: parsing file: %s.\n\n", filename);

  int line_length = 0;
  int last_line_length;
  CFG_Char * line = 0;

  #ifdef CFG_PROFILE_LIB

  Uint32 time_start = SDL_GetTicks();

  #endif

  do
   {

    last_line_length = 1;
    int length = CFG_Internal_RWfind(source, '\n', &last_line_length );
    
    if ( length == -2 )
     {
      /* error type should be already set from inside SDL_Rwfind */
      return CFG_CRITICAL_ERROR;
     }
    else if ( length == -1 )
     {
      length = last_line_length;
      last_line_length = 0; /* It was last line  */
     }
    else if ( length == 0 )
     {
      /* Totally empty line? */
      continue;
     }
     
    if (line_length < length)
     {
      if (line != 0)
       free(line);
       
       line = (CFG_Char*) malloc( length );
       
       if (line == 0)
        {
         CFG_SetError("Couldn't allocate memory for one line.");
         return CFG_CRITICAL_ERROR;
        }
     }
     
    line_length = length;

    int read = SDL_RWread(source, line, 1, length );
    
    if ((read < 0)/* || (read != length - 1)*/ )
     {
      if (line != 0)
       free(line);
       
      CFG_SetError("Error reading from stream.");
      return CFG_CRITICAL_ERROR;
     }
     
    line[length-1] = 0;
     
    CFG_Internal_ParseLine(file->internal_file, line, length );
	++CFG_Internal_line;
	
   }
    while ( last_line_length > 0 );
   
  if (line)
   free (line);
   
/* -------------------------------------------------------  */
   
  #ifdef CFG_PROFILE_LIB

  Uint32 time_end = SDL_GetTicks();

  Uint32 time_diff = time_end - time_start;

  fprintf(stdout, "\n %s >>> >>> Parsed in: %i ms\n",
          file->internal_file->filename.c_str(), time_diff);

  #endif

  CFG_Internal_selected_file = file;

  /* once again select global group, which could have been overwritten with other loaded group */
  file->internal_file->selected_group = &file->internal_file->global_group;
  
  CFG_Internal_pre_comment.erase();
  parsed.erase();
  
  return CFG_OK;
 }

/* ------------------------------------------------------- end CFG_Internal_OpenFile */

/* ------------------------------------------------------- start CFG_OpenFile_RW */

int CFG_OpenFile_RW( SDL_RWops * source, CFG_File * file )
 {
  return CFG_Internal_OpenFile( source, file, "SDL_RWops stream" );
 }

/* ------------------------------------------------------- end CFG_OpenFile */

/* ------------------------------------------------------- start CFG_OpenFile_RW */

int CFG_OpenFile( CFG_String_Arg filename, CFG_File * file )
 {
  if (filename == 0)
   {
    file->internal_file = new CFG_Internal_File;

    if (file->internal_file == 0)
     {
      CFG_SetError("Couldn't allocate memory for file.");
      return CFG_CRITICAL_ERROR;
     }

    /* select global group */
    file->internal_file->selected_group = &file->internal_file->global_group;
    file->internal_file->global_group.next_entry_order = 0;

    file->internal_file->next_group_order = 0;
    file->internal_file->iteration_direction = CFG_ITERATE_NORMAL;

    /*++CFG_Internal_files_open;

     if this config file is alone, select it
    if ( CFG_Internal_files_open == 1)*/
     CFG_Internal_selected_file = file;

    return CFG_OK;
   }

  /* construct SDL_RWops from filename and pass it to CFG_Internal_OpenFile( */
  SDL_RWops * src = SDL_RWFromFile(filename, "r");
  
  if (src == 0)
   {
    CFG_SetError("Couldn't open file.");
    return CFG_ERROR;
   }

  return CFG_Internal_OpenFile(src, file, filename);
 }
  
/* ------------------------------------------------------- end CFG_OpenFile */

/* ------------------------------------------------------- start CFG_Internal_FindChar */

int CFG_Internal_FindChar(const CFG_Char * buffer, char xxx)
 {
  const CFG_Char * ptr = buffer;
  
  while (*ptr)
   {
    if ( (*ptr) == xxx)
     return (ptr - buffer);
     
    ++ptr;
   }

  return CFG_NPOS;
 }

/* ------------------------------------------------------- end CFG_Internal_FindChar */

/* ------------------------------------------------------- start CFG_Internal_ParseLine */

void CFG_Internal_ParseLine(CFG_Internal_File * internal_file, const CFG_Char * buffer, int line_length)
 {
  /*
   First, remove all spaces that aren't in: comments, strings
  */

  parsed.erase();
  parsed.reserve(line_length);
  
  const CFG_Char * ptr = buffer;
  
  /* if it's 0, then we're inside comment or string so don't skip spaces */
  int skipSpaces = 1;
  
  /* if it's 1, we're inside comment */
  int isComment = 0;
  
  /* if it's 1, we're inside string: " ... " */
  int isString = 0;
  
  /* position of last found */
  int lastDblQuote = CFG_NPOS;
  
  /* If we know that we're inside multiline C like comment, then there's nothing to remove from inside */
  if (CFG_Internal_c_like_comment == 1)
   {
    parsed = buffer;
   }
  else while (*ptr)
   {
	while (CFG_Internal_isspace(*ptr) && (*ptr) && (skipSpaces == 1) ) /* remove spaces */
	 {
	  ++ptr;
	 }
	 
	if ((*ptr) == 0 ) break;
	 
	while (CFG_Internal_isspace(*ptr) && (*ptr) && (skipSpaces == 0) ) /* copy spaces */
	 {
      parsed.push_back(*ptr);
	  ++ptr;
	 }

	if ((*ptr) == 0 ) break;

	while ((!CFG_Internal_isspace(*ptr)) && (*ptr )) /* copy text */
	 {
	  parsed.push_back(*ptr);
	  
	  if ( CFG_Internal_IsComment(ptr) && (isString == 0) )
	   {
    	isComment = 1;
    	skipSpaces = 0;
	   }
	   
	  if ( ((*ptr) == CFG_OPERATOR_DBL_QUOTE) && ( isComment == 0 ) &&
           ((ptr - 1) >= buffer ))
	   {
        if ( ( *(ptr - 1) != CFG_OPERATOR_ESCAPE_SEQ) )
         {
          /*
           we're not inside comment, and found quotation that isn't escape sequence - clearly, we've found
           end of string
          */
          isString = !isString;
          skipSpaces = !isString;
		
          /* get position in line of this CFG_OPERATOR_DBL_QUOTE */
          lastDblQuote = parsed.size(); //*ptr - buffer;*/
	     }
        else /* Remove \" escape sequence */
         {
          parsed.erase( parsed.size() - 2, 1);
         }
      }
	  
	  ++ptr;
	 }
   }

 /* ------------------------------------------------------- Analyze */

  /* Cache current line number as text */
  char current_line[33];
  sprintf(current_line, "%d", CFG_Internal_line);
  //itoa( CFG_Internal_line, current_line, 10);
   
   
  if (parsed.size() > 0)
   {

	/* Parse further, first check if it is full line comment */

    int is_full_line_comment = CFG_Internal_IsComment(&parsed[0]);
	
	if ( is_full_line_comment != 0 )
	 {
      if ( is_full_line_comment == 1)
       {
        /*
          Found //, ; or # comment from beginning to the end of line
          Save it, since it may be pre-comment
        */
         if (CFG_Internal_c_like_comment == 1)
          {
           CFG_Internal_pre_comment += parsed;
          }
         else
          {
           if ( (CFG_Internal_pre_comment_type == CFG_STD_COMMENT) && (CFG_Internal_last_line_empty == 0) )
            {
             CFG_Internal_pre_comment += '\n';
             CFG_Internal_pre_comment += parsed;
            }
           else
            {
             CFG_Internal_pre_comment = parsed;
             CFG_Internal_pre_comment_type = CFG_STD_COMMENT;
            }
          }
          
        CFG_Internal_last_line_empty = 0;
        return;
       }
      else /* It's also full line comment, but it's a special one - it's C like comment */
       {
        int endCommentPos = parsed.find(CFG_COMMENT_C1, 3 );

        if ( (endCommentPos != CFG_NPOS) && (parsed[endCommentPos - 1] == CFG_COMMENT_C2) )
         {
          /* found comment from beginning to the end of line
          CFG_DEBUG("Found new comment from beginning to the end of line: %s\n\n",
                     parsed.substr(0, endCommentPos + 1 ).c_str() )*/
                     
          /* Save it, since it may be pre-comment */
          CFG_Internal_pre_comment = parsed.substr(0, endCommentPos + 1 );
          CFG_Internal_pre_comment_type = CFG_STD_COMMENT;
          CFG_Internal_last_line_empty = 0;
          return;
         }
        else
         {

          if (CFG_Internal_c_like_comment == 1)
           {
            CFG_DEBUG("Warning: nested C like comments: %s\n\n", parsed.c_str() )
            CFG_Internal_pre_comment += parsed;
            CFG_Internal_pre_comment += '\n';
            CFG_Internal_c_like_comment = 0;
           }
          else
           {
            /*CFG_DEBUG("Found start of new multiline comment: %s\n", parsed.c_str() )*/
            CFG_Internal_c_like_comment = 1;
            
            /*
             buffer instead of parsed since there might be some whitespaces at begginning of line that
             were removed by "tokenizer"
             */
            CFG_Internal_pre_comment = buffer;
            CFG_Internal_pre_comment += '\n';
            CFG_Internal_pre_comment_type = CFG_C_LIKE_COMMENT;
           }
           
          CFG_Internal_last_line_empty = 0;
          return;
         }
        
       }
     }
    else if ( CFG_Internal_c_like_comment == 1 ) /* Is it end of multiline comment? */
     {
      int endCommentPos = parsed.find(CFG_COMMENT_C1);

      if ( (endCommentPos > 0) && (parsed[endCommentPos - 1] == CFG_COMMENT_C2) )
       {
        /* Yes, it's end of multiline comment */
        CFG_Internal_c_like_comment = 0;
        
        CFG_Internal_pre_comment += parsed.substr(0, endCommentPos + 1);
        
        /*CFG_DEBUG("Found end of multiline comment: %s\n", parsed.substr(0, endCommentPos + 1).c_str() )*/
       }
      else /* We're still inside mutliline comment, it's not end yet */
       {
        CFG_Internal_pre_comment += parsed;
        CFG_Internal_pre_comment += '\n';
        /*CFG_DEBUG("Found next part of multiline comment: %s\n", parsed.c_str() )*/
       }

     }
    else if (parsed[0] == CFG_OPERATOR_GROUP_START )
	 {
	   /* ------------------------------------------------------- probably it's a new group */
	  
	  int secondBracePos = parsed.find( CFG_OPERATOR_GROUP_END, 1 );
	  
      /* if not, then it's not a group nor entry, so report error */
      if (secondBracePos == CFG_NPOS)
       {
		CFG_String errorMsg = "Found error in line ";
		errorMsg += current_line;
		errorMsg += " of file \"";
        errorMsg += internal_file->filename; // CFG_Internal_filename;
        errorMsg += "\", error: couldn't classify this line: \n";
        errorMsg += buffer;
  
        CFG_DEBUG("! Error occured: %s\n", errorMsg.c_str())
        /* maybe also CFG_SetError(errorMsg.c_str()) ? */
        return;
       }
       
      #ifdef CFG_REMOVE_GROUP_SPACES
       CFG_String groupName = parsed.substr(1, secondBracePos - 1);
      #else
       int firstBracePos = CFG_Internal_find_char(buffer, CFG_OPERATOR_GROUP_START);
       int secondBracePosEx = CFG_Internal_find_char(buffer + firstBracePos, CFG_OPERATOR_GROUP_END);
       CFG_String groupName ( buffer + firstBracePos + 1, secondBracePosEx - 1);
      #endif

      CFG_Internal_Group group_body;
      group_body.order = ++internal_file->next_group_order;
      
      int is_post_comment = CFG_Internal_IsComment(&parsed[secondBracePos + 1]);
      
      if ( is_post_comment == 1)
       group_body.post_comment = parsed.substr(secondBracePos + 1);
      else
      if (( is_post_comment == 2) && ( parsed.size() > secondBracePos + 3 ))
       {
        int endCommentPos = parsed.find(CFG_COMMENT_C1, secondBracePos + 3 );
        
        if ( (endCommentPos != CFG_NPOS) && (parsed[endCommentPos - 1] == CFG_COMMENT_C2) )
         group_body.post_comment = parsed.substr(secondBracePos + 1);
        else
         {
          CFG_DEBUG("! Error occured: unterminated C-like post comment found in next group definition: %s\n",
                     parsed.substr(secondBracePos + 1).c_str())
         }
       }
       
      group_body.next_entry_order = 0;
      
      #ifdef CFG_DEBUG_LIB

       fprintf(stderr,"\n ------------------\n\nFound new group: %s\n", groupName.c_str() );
       
       if (CFG_Internal_pre_comment.size() > 0)
        {
         fprintf(stderr, "    Pre - comment: %s\n", CFG_Internal_pre_comment.c_str());
        }

       if (group_body.post_comment.size() > 0)
        {
         fprintf(stderr, "    Post - comment: %s\n", group_body.post_comment.c_str());
        }
        
       fprintf(stderr, "\n");

      #endif
      
      /* Assign current pre comment to this entry */
      if (CFG_Internal_pre_comment.size() > 0)
       {
        group_body.pre_comment = CFG_Internal_pre_comment;
        CFG_Internal_pre_comment.erase();
        group_body.pre_comment_type = CFG_Internal_pre_comment_type;
       }
       
      CFG_Internal_pre_comment_type = CFG_NO_COMMENT;

     CFG_Internal_GroupMapItor iterator = internal_file->groups.insert( std :: make_pair(groupName, group_body)).first;
      
      internal_file->selected_group = &( (*iterator).second );
     }
	else  /* -------------------------------------------------------  */
	 {
	  /* it may be new entry, though we must first check for CFG_OPERATOR_EQUALS character */
	  int equalPos = parsed.find(CFG_OPERATOR_EQUALS, 1 );
	  
	  /*
       Determine first char where we should start seeking for comments. If it's string, we should start
       seeking after last CFG_OPERATOR_DBL_QUOTE char, otherwise we could misinterpret chars from inside string.
       
       Note
       Be careful, there was bug here without subtracting 1 from lastDblQuote which arised on this line:

         text = "test" ; comment
         
       Maybe we should also subtract 1 from lastDblQuote in if?

       */
	  int commentPotentialPos = equalPos;

      if (lastDblQuote > commentPotentialPos)
       commentPotentialPos = lastDblQuote - 1;
	  
	  /*
	   Watch out for CFG_COMMENT_1, it's "double-comment", but even finding one char of it before CFG_OPERATOR_EQUALS
	  means it's not entry.
	  Of course, we are assuming that nobody will choose for CFG_COMMENT_1 such stupid chars as ie.
	   'a', ' ', '[' etc.
	  */
	  int commentPos1 = parsed.find(CFG_COMMENT_1, commentPotentialPos );
	  
	  int commentPos2 = parsed.find(CFG_COMMENT_2, commentPotentialPos );
	  int commentPos3 = parsed.find(CFG_COMMENT_3, commentPotentialPos );
	  int commentPos4 = parsed.find(CFG_COMMENT_C1, commentPotentialPos );
	  
	  /* We must check whether there is next char forming those double char comments // and /* */

	  if ( (commentPos1 != CFG_NPOS) && (parsed.size() > commentPos1) )
	   {
        if (parsed[commentPos1 + 1] != CFG_COMMENT_1)
         commentPos1 = CFG_NPOS;
       }
	  else if (parsed.size() > commentPos1)
	   {
		/* only one CFG_COMMENT_1 was found at end of line
		   what now, set error? it's non critical */
       }

	  if ( (commentPos4 != CFG_NPOS) && (parsed.size() > commentPos4) )
	   {
        if (parsed[commentPos4 + 1] != CFG_COMMENT_C2)
         commentPos4 = CFG_NPOS;
       }
	  else if (parsed.size() > commentPos4)
	   {
		/* only one part of /* was found at end of line
		   what now, set error? it's non critical */
       }


	  /*
       We must set them to the last char, since CFG_NPOS == -1, so it would be classified before, what would be wrong
	   since there is no comment of that type.
      */
	  if (commentPos1 == CFG_NPOS)
       commentPos1 = parsed.size();
       
      if (commentPos2 == CFG_NPOS)
       commentPos2 = parsed.size();
       
      if (commentPos3 == CFG_NPOS)
       commentPos3 = parsed.size();
       
      if (commentPos4 == CFG_NPOS)
       commentPos4 = parsed.size();

	  /*
	   Are those comments behind or after CFG_OPERATOR_EQUALS?
       If they are before, then it's not a group nor entry, so report error.
	  */
      if ( (equalPos == CFG_NPOS) ||
	   ((equalPos != CFG_NPOS) && ( ( commentPos1 < equalPos ) || ( commentPos2 < equalPos ) ||
                                    ( commentPos3 < equalPos ) || ( commentPos4 < equalPos ) )))
       {
        #ifdef CFG_DEBUG_LIB

		CFG_String errorMsg = "Found error in line ";
		errorMsg += current_line;
		errorMsg += " of file \"";
        errorMsg += internal_file->filename; //CFG_Internal_filename;
        errorMsg += "\", error: couldn't classify this line: \n";
        errorMsg += buffer;

        CFG_DEBUG("! Error occured: %s\n", errorMsg.c_str())
        
        #endif
        return;
       }
       
	  /*
       Now we must find out which comment goes first, since there can be comment inside comment ie.
       key = value // comment 1 # comment 2 ; comment 3
      */
      int commentPos = parsed.size();
      int commentType = 0;

	  if ( (commentPos1 < commentPos) && (commentPos1 != CFG_NPOS) )
	   {
        commentPos = commentPos1;
        commentType = 1;
	   }

      if ( (commentPos2 < commentPos) && (commentPos2 != CFG_NPOS) )
	   {
        commentPos = commentPos2;
        commentType = 2;
	   }
       
	  if ( (commentPos3 < commentPos) && (commentPos3 != CFG_NPOS) )
	   {
        commentPos = commentPos3;
        commentType = 3;
	   }

	  if ( (commentPos4 < commentPos) && (commentPos4 != CFG_NPOS) )
	   {
        commentPos = commentPos4;
        commentType = 4;
	   }

      CFG_String comment;
      
	  /* if there is any comment, we must take its value and copy to new string */
	  if (commentType != 0)
	   {
        if (commentType != 4)
         comment = parsed.substr(commentPos);
        else
         {
          int endCommentPos = parsed.find(CFG_COMMENT_C1, commentPos + 3 );

          if ( (endCommentPos != CFG_NPOS) && (parsed[endCommentPos - 1] == CFG_COMMENT_C2) )
           comment = parsed.substr(commentPos, endCommentPos - commentPos + 1 );
          else
           CFG_DEBUG("! Error occured: unterminated C-like comment found: %s\n\n", parsed.c_str())
         }

        
	   }

 /* ------------------------------------------------------- Analyze once more */
 
	  /* Determine type of value:

		 1. If first and last characters of value are CFG_OPERATOR_DBL_QUOTE, then it's text
		 2. If there is exactly one dot inside, and all other characters are digits, then it's float
		 3. If value equals CFG_KEYWORD_FALSE_1 or CFG_KEYWORD_TRUE_1 (or others, case insensitive), then it's bool
		 4. If there are only digits (0..9) inside, then it's integer
		 5. UNSUPPORTED NOW: If there are also other characters, then it's text
		 6. If else, report error.

	    */
	   
	  int type = 0;
	  int string_avoid_quotes = 0;
	  
	  /* 1. Text */
      /*
	  fprintf(stderr, "commentPos: %i,  lastDblQuote: %i\nparsed[commentPos-1] = %c \nparsed[lastDblQuote-1] = %c\n\n",
              commentPos, lastDblQuote, parsed[commentPos-1], parsed[lastDblQuote-1]);*/
	  
	  if ( (parsed[equalPos + 1] == CFG_OPERATOR_DBL_QUOTE ) && (parsed[commentPos - 1] == CFG_OPERATOR_DBL_QUOTE) )
	   {
    	type = CFG_TEXT;
    	
    	/*
		 We must also change string_avoid_quotes value to avoid substringing value with CFG_OPERATOR_DBL_QUOTE
		 at beginning and end of it.
		*/
		
		string_avoid_quotes = 1;
	   }
	  else
	   {
        /* 2. Float */
        
        int dotPosition = parsed.find('.', equalPos + 1);
        int minusPos = 0;
        
        if (parsed[equalPos + 1] == '-')
         minusPos = equalPos + 1;
        
        if (dotPosition != CFG_NPOS )
		 {
		  int i = equalPos + 1;
		  
		  for (; i < dotPosition; ++i)
		   if ( (!isdigit(parsed[i])) && (minusPos != i) )
			{
			 /* we've found non-digit and it's not minus, so definitely it's not float */
             type = -1;
             break;
			}
			
		  if (type == 0)
		   {
            i = dotPosition + 1;
		  
            for (; i < commentPos; ++i)
             if ( (!isdigit(parsed[i])) && (minusPos != i) )
              {
               /* we've found non-digit and it's not minus, so definitely it's not float */
               type = -1;
               break;
              }

            if (type != -1)
             {
              type = CFG_FLOAT;
             }
           }

         }
		else if ((dotPosition == CFG_NPOS) && ( type == 0 )) /* Since there aren't any dots in bools */
		 {
          /* 3. Bool */
          
          /* First compare sizes of bools and value, to avoid potentially slow char-by-char compare */
          
          int value_size = commentPos - equalPos - 1;
          
          int bools_sizes[6] = {sizeof(CFG_KEYWORD_TRUE_1),  sizeof(CFG_KEYWORD_TRUE_2),  sizeof(CFG_KEYWORD_TRUE_3),
                                sizeof(CFG_KEYWORD_FALSE_1), sizeof(CFG_KEYWORD_FALSE_2), sizeof(CFG_KEYWORD_FALSE_3)};

          char * bools_values[6] = { CFG_KEYWORD_TRUE_1,  CFG_KEYWORD_TRUE_2,  CFG_KEYWORD_TRUE_3,
		                             CFG_KEYWORD_FALSE_1, CFG_KEYWORD_FALSE_2, CFG_KEYWORD_FALSE_3 };

		  const char * value_pointer = (parsed.c_str() + equalPos + 1);
		  
		  int i = 0;
          for (; (i < 6) && (type != CFG_BOOL); ++i)
		   {
			if ( (value_size == (bools_sizes[i] - 1)) &&
                 (!CFG_Internal_StringEquals(value_pointer, bools_values[i], value_size)) )
			 type = CFG_BOOL;
           }

         }

        if ( type <= 0 ) /* Nothing changed, it's not boolean */
		 {
		  /* 4. Integer */

		  type = 0;
		  
          int minusPos = 0;
          int i = equalPos + 1;

          if (parsed[i] == '-')
           minusPos = i;

		  for (; i < commentPos; ++i)
		   if ( (!isdigit(parsed[i])) && (minusPos != i) )
			{
			 /*
              We've found non-digit and it's not minus, so definitely it's not integer
              it may be string without quotes around it
             */
             type = -1;
             break;
			}
          /*

           Max values for 32 bit signed integers (inclusive):
          
           -2147483648  size = 11
           2147483647   size = 10
           
           If our value is longer than one of them, than it can't be stored in Sint32, so change type to text.
           Also, check for exact value.

          */
          
          if (type != -1)
           {
            CFG_String integer_value = parsed.substr(equalPos + 1, commentPos - equalPos - 1);

            int integer_value_size = integer_value.size();

            if ( ( (parsed[equalPos + 1] == '-') && (integer_value_size <= 11) ) ||
                 ( (parsed[equalPos + 1] != '-') && (integer_value_size <= 10) ) )
             {
              istringstream stream( integer_value );
              Sint32 integer;
              
              if (stream >> integer)
               type = CFG_INTEGER;
              else
               {
                type = CFG_TEXT;
                CFG_MAX_INTEGER_WARNING
               }
            }
            else
            {
             type = CFG_TEXT;
             CFG_MAX_INTEGER_WARNING
            }
           }
          else
           type = CFG_TEXT;
         }
       }

 /* ------------------------------------------------------- Finally */
	  
	 if (type <= 0)
      {
       #ifdef CFG_DEBUG_LIB
       
   		CFG_String errorMsg = "Found error in line ";
		errorMsg += current_line;
		errorMsg += " of file \"";
        errorMsg += internal_file->filename; // CFG_Internal_filename;
        errorMsg += "\", error: couldn't classify type of this entry's value, in this line: \n";
        errorMsg += buffer;

        CFG_DEBUG("! Error occured: %s\n\n", errorMsg.c_str())
        
       #endif
       return;
      }

    #ifdef CFG_DEBUG_LIB
    
     switch (type)
      {
       case CFG_BOOL:
        fprintf(stderr, "Found new boolean entry:\n");
       break;
       
       case CFG_INTEGER:
        fprintf(stderr, "Found new integer entry:\n");
       break;
       
       case CFG_FLOAT:
        fprintf(stderr, "Found new float entry:\n");
       break;
       
       case CFG_TEXT:
        fprintf(stderr, "Found new text entry:\n");
       break;

       default:
        fprintf(stderr, "Error: unrecognized entry type: %i\n", type);

      }
    
     fprintf(stderr, "    Key: %s\n    Value: %s\n", parsed.substr(0, equalPos).c_str(),
      parsed.substr(equalPos + string_avoid_quotes + 1, commentPos - equalPos - 1 - 2 * string_avoid_quotes).c_str() );
     
      if (CFG_Internal_pre_comment.size() > 0)
        fprintf(stderr, "    Pre - comment: %s\n", CFG_Internal_pre_comment.c_str());

      if (commentType != 0)
        fprintf(stderr, "    Post - comment: %s\n", comment.c_str());

       fprintf(stderr, "\n");

     #endif
      
     CFG_Internal_Entry entry_body;
      
     entry_body.value = parsed.substr(equalPos + string_avoid_quotes + 1,
                                      commentPos - equalPos - 1 - 2 * string_avoid_quotes);
     entry_body.type = type;
     entry_body.post_comment = comment;
     entry_body.order = ++internal_file->selected_group->next_entry_order;
      
     if (CFG_Internal_pre_comment.size() > 0)
      {
       entry_body.pre_comment = CFG_Internal_pre_comment;
       entry_body.pre_comment_type = CFG_Internal_pre_comment_type;
       CFG_Internal_pre_comment.erase();
      }
     else
      entry_body.pre_comment_type = CFG_NO_COMMENT; /* No pre comment */
       
     CFG_Internal_pre_comment_type = CFG_NO_COMMENT;

     internal_file->selected_group->entries.insert( make_pair( parsed.substr(0, equalPos), entry_body) );
     /*return;*/
    }

   }
  else if (CFG_Internal_c_like_comment == 1)
   CFG_Internal_pre_comment += '\n';

  /* Else: don't do anything, we've found empty line */
  CFG_Internal_last_line_empty = 1;

 }

/* ------------------------------------------------------- end CFG_Internal_ParseLine */

/* -------------------------------------------------------*/
/*                  End file open operations              */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                  Start file operations                 */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_CloseFile */

int CFG_CloseFile( CFG_File * file )
 {
  if (file)
   {
    delete file->internal_file;
    CFG_Internal_selected_file = 0;

    return CFG_OK;
   }
  else
   {
    /*
     Close currently selected file,
     but first make sure that there is something selected.
    */

    if (CFG_Internal_selected_file == 0)
     {
      CFG_SetError("No file currently selected, ignored request to close currently selected file.");
      return CFG_ERROR;
     }
     
    delete CFG_Internal_selected_file->internal_file;
    CFG_Internal_selected_file = 0;
    
    return CFG_OK;
   }
 }

/* ------------------------------------------------------- end CFG_CloseFile */

/* ------------------------------------------------------- start CFG_ClearFile */

int CFG_ClearFile( CFG_File * file )
 {
  if (file)
   {
    file->internal_file->groups.clear();
    file->internal_file->global_group.entries.clear();
    file->internal_file->global_group.next_entry_order = 0;
    file->internal_file->next_group_order = 0;
    file->internal_file->selected_group = &file->internal_file->global_group;

    return CFG_OK;
   }
  else
   {
    /*
     Clear currently selected file,
     but first make sure that there is something selected.
    */

    if (CFG_Internal_selected_file == 0)
     {
      CFG_SetError("No file currently selected, ignored request to clear currently selected file.");
      return CFG_ERROR;
     }
     
    CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;

    internal_file->groups.clear();
    internal_file->global_group.entries.clear();
    internal_file->global_group.next_entry_order = 0;
    internal_file->next_group_order = 0;
    internal_file->selected_group = &CFG_Internal_selected_file->internal_file->global_group;

    return CFG_OK;
   }
 }

/* ------------------------------------------------------- end CFG_ClearFile */

/* ------------------------------------------------------- start CFG_SelectFile */

void CFG_SelectFile(CFG_File * file)
 {
  CFG_Internal_selected_file = file;
  CFG_ASSERT_NULL
 }

/* ------------------------------------------------------- end CFG_SelectFile */

/* ------------------------------------------------------- start CFG_GetSelectedFile */

CFG_File * CFG_GetSelectedFile( void )
 {
  CFG_ASSERT_NULL
  return CFG_Internal_selected_file;
 }

/* ------------------------------------------------------- end CFG_GetSelectedFile */

/* ------------------------------------------------------- start CFG_GetSelectedFileName */

CFG_String_Arg CFG_GetSelectedFileName( void )
 {
  CFG_ASSERT_NULL
  return CFG_Internal_selected_file->internal_file->filename.c_str();
 }

/* ------------------------------------------------------- end CFG_GetSelectedFileName */

/* -------------------------------------------------------*/
/*                    End file operations                 */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                   Start group operations               */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_SelectGroup */

int CFG_SelectGroup(CFG_String_Arg group, CFG_Bool create)
 {
  CFG_ASSERT_NULL
  CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;

  if (group)
   {
    CFG_Internal_GroupMapItor iterator = internal_file->groups.find(group);
    
    if ( iterator == internal_file->groups.end() && (create == 1) )
     {
      /* create and select new group */
      CFG_Internal_Group group_body;
      group_body.order = ++internal_file->next_group_order;
      group_body.next_entry_order = 0;

      iterator = internal_file->groups.insert( std :: make_pair(group, group_body)).first;
      internal_file->selected_group = &( (*iterator).second );
      
      /* This isn't error, but we have to do it */
      CFG_SetError("You tried to select group that doesn't exist - new group was created.");
      return CFG_GROUP_CREATED;
    }
   else if ( iterator == internal_file->groups.end() && (create == 0) )
    {
     CFG_SetError("You tried to select group that doesn't exist - new group wasn't created.");
     return CFG_ERROR;
    }
    
    internal_file->selected_group = &( (*iterator).second );
    return CFG_OK;
   }
  else /* 0 was passed, we have to select global group */
   {
    internal_file->selected_group = &(internal_file->global_group);
    return CFG_OK;
   }
 }

/* ------------------------------------------------------- end CFG_SelectGroup */

/* ------------------------------------------------------- start CFG_GetSelectedGroupName */

CFG_String_Arg CFG_GetSelectedGroupName( void )
 {
  CFG_ASSERT_NULL
  CFG_Internal_Group * selected_group = CFG_Internal_selected_file->internal_file->selected_group;
  CFG_Internal_Group * global_group = &CFG_Internal_selected_file->internal_file->global_group;
  
  /* If global group was selected, return null (0). */
  if (global_group == selected_group)
   return 0;

  /*
   Otherwise, we have to do sth bad, evil and horrible - manually search in map for group's name.
   Of course, it's very inefficient... :-/
   */
   
  for ( CFG_Internal_GroupMapConstItor iterator = CFG_Internal_selected_file->internal_file->groups.begin();
        iterator != CFG_Internal_selected_file->internal_file->groups.end(); ++iterator )
   {
    if ( &iterator->second == selected_group )
     {
      return iterator->first.c_str();
     }
   }

  CFG_LOG_CRITICAL_ERROR(003);
  return 0;
 }

/* ------------------------------------------------------- end CFG_GetSelectedGroupName */

/* ------------------------------------------------------- start CFG_GetGroupCount */

Uint32 CFG_GetGroupCount( void )
 {
  CFG_ASSERT_NULL
  return CFG_Internal_selected_file->internal_file->groups.size();
 }

/* ------------------------------------------------------- end CFG_GetGroupCount */

/* -------------------------------------------------------*/
/*                    End group operations                */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                   Start entry reading                  */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_Internal_FindInGlobal */

#define CFG_INTERNAL_FOUND 1
#define CFG_INTERNAL_DEFAULT 0
#define CFG_INTERNAL_CREATE (-1)

CFG_Internal_EntryMapItor CFG_Internal_FindInGlobal(CFG_String_Arg key, int type, int * result )
 {
  CFG_Internal_Group * global_group = &CFG_Internal_selected_file->internal_file->global_group;
  CFG_Internal_EntryMapItor iterator = global_group->entries.find(key);

  if ( (iterator != global_group->entries.end()) && ((*iterator).second.type == type ) )
   {
    *result = CFG_INTERNAL_FOUND;
    return iterator;
   }
  else if ((iterator != global_group->entries.end()) && ((*iterator).second.type != type ) )
   {
    *result = CFG_INTERNAL_DEFAULT;
    return iterator;
   }
  else
   {
    *result = CFG_INTERNAL_CREATE;
    return iterator;
   }
 }

/* ------------------------------------------------------- end CFG_Internal_FindInGlobal */

/* ------------------------------------------------------- start CFG_Internal_ReadValue */

 /*
   Returns (through *result):

     CFG_INTERNAL_DEFAULT - use default value when such entry exists in selected group //juz raczej niepotrzebne
     CFG_INTERNAL_CREATE - use default value and create new entry in selected group
     CFG_INTERNAL_FOUND - use value from iterator
     
 */
     
CFG_Internal_EntryMapItor CFG_Internal_ReadValue(CFG_String_Arg key, int type, int * result,
                                                 CFG_Internal_Group ** create_here)
 {
  CFG_Internal_Group * selected_group = CFG_Internal_selected_file->internal_file->selected_group;
  CFG_Internal_Group * global_group = &CFG_Internal_selected_file->internal_file->global_group;

  *create_here = global_group;

  if (global_group == selected_group)
   {
    return CFG_Internal_FindInGlobal(key, type, result );
   }
  else
   {
    CFG_Internal_EntryMapItor iterator = selected_group->entries.find(key);
    
    if ( (iterator != selected_group->entries.end()) && ((*iterator).second.type == type ) )
     {
      *result = CFG_INTERNAL_FOUND;
      return iterator;
     }
    else if ((iterator != selected_group->entries.end()) && ((*iterator).second.type != type ) )
     {
      return CFG_Internal_FindInGlobal(key, type, result );
     }
    else
     {
      *create_here = selected_group;
      return CFG_Internal_FindInGlobal(key, type, result );
     }
   }
 }

/* ------------------------------------------------------- end CFG_Internal_ReadValue */

/* ------------------------------------------------------- start CFG_ReadBool */

CFG_Bool CFG_ReadBool(CFG_String_Arg key, CFG_Bool defaultVal)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_Group * create_here;
  
  CFG_Internal_EntryMapItor iterator = CFG_Internal_ReadValue(key, CFG_BOOL, &result, &create_here);

  if ( result == CFG_INTERNAL_DEFAULT )
   return defaultVal;

  if ( result == CFG_INTERNAL_FOUND )
   {
    char * bools_values[6] = { CFG_KEYWORD_TRUE_1,  CFG_KEYWORD_TRUE_2,  CFG_KEYWORD_TRUE_3,
                               CFG_KEYWORD_FALSE_1, CFG_KEYWORD_FALSE_2, CFG_KEYWORD_FALSE_3 };

    const char * value_pointer = (*iterator).second.value.c_str();

    int i = 0;
    for (; i < 3; ++i)
     {
      if ( !strcasecmp(value_pointer, bools_values[i]) )
       return CFG_True;
     }
     
    for (i = 3; i < 6; ++i)
     {
      if ( !strcasecmp(value_pointer, bools_values[i]) )
       return CFG_False;
     }

    /*
     if none of them matched, then it's very strange situation, but we should cope with it by
     returning default value and logging this information:
    */
    
    CFG_LOG_CRITICAL_ERROR(001)
    
    return defaultVal;
   }

  /* not found, so create new entry in selected group */
  CFG_Internal_Entry entry_body;
   
  if (defaultVal)
   entry_body.value = CFG_KEYWORD_TRUE_1;
  else
   entry_body.value = CFG_KEYWORD_FALSE_1;

  entry_body.type = CFG_BOOL;
  entry_body.order = ++create_here->next_entry_order;

  create_here->entries.insert(make_pair( key, entry_body) );

  return defaultVal;
 }

/* ------------------------------------------------------- end CFG_ReadBool */

/* ------------------------------------------------------- start CFG_ReadInt */

Sint32 CFG_ReadInt(CFG_String_Arg key, Sint32 defaultVal)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_Group * create_here;
  
  CFG_Internal_EntryMapItor iterator = CFG_Internal_ReadValue(key, CFG_INTEGER, &result, &create_here);

  if ( result == CFG_INTERNAL_DEFAULT )
   return defaultVal;
   
  if ( result == CFG_INTERNAL_FOUND )
   return atoi( (*iterator).second.value.c_str() );
   
  /* not found, so create new entry in selected group */
  CFG_Internal_Entry entry_body;

  char tmp[33];
  sprintf(tmp, "%d", defaultVal);
  //itoa(defaultVal, tmp, 10);

  entry_body.value = tmp;
  entry_body.type = CFG_INTEGER;
  entry_body.order = ++create_here->next_entry_order;
   
  create_here->entries.insert(make_pair( key, entry_body) );

  return defaultVal;
 }

/* ------------------------------------------------------- end CFG_ReadInt */

/* ------------------------------------------------------- start CFG_ReadFloat */

float CFG_ReadFloat(CFG_String_Arg key, float defaultVal)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_Group * create_here;
  
  CFG_Internal_EntryMapItor iterator = CFG_Internal_ReadValue(key, CFG_FLOAT, &result, &create_here);

  if ( result == CFG_INTERNAL_DEFAULT )
   return defaultVal;

  if ( result == CFG_INTERNAL_FOUND )
   return ( static_cast<float>(atof( (*iterator).second.value.c_str() )));

  /* not found, so create new entry in selected group */
  CFG_Internal_Entry entry_body;

  char tmp[33];
  sprintf(tmp, "%f", defaultVal);
   
  entry_body.value = tmp;
  entry_body.type = CFG_FLOAT;
  entry_body.order = ++create_here->next_entry_order;

  create_here->entries.insert(make_pair( key, entry_body) );

  return defaultVal;
 }

/* ------------------------------------------------------- end CFG_ReadFloat */

/* ------------------------------------------------------- start CFG_ReadText */

CFG_String_Arg CFG_ReadText(CFG_String_Arg key, CFG_String_Arg defaultVal)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_Group * create_here;
  
  CFG_Internal_EntryMapItor iterator = CFG_Internal_ReadValue(key, CFG_TEXT, &result, &create_here);

  if ( result == CFG_INTERNAL_DEFAULT )
   return defaultVal;

  if ( result == CFG_INTERNAL_FOUND )
   return (*iterator).second.value.c_str();

  /* not found, so create new entry in selected group */
  CFG_Internal_Entry entry_body;

  entry_body.value = defaultVal;
  entry_body.type = CFG_TEXT;
  entry_body.order = ++create_here->next_entry_order;

  create_here->entries.insert(make_pair( key, entry_body) );

  return defaultVal;
 }

/* ------------------------------------------------------- end CFG_ReadText */

/* -------------------------------------------------------*/
/*                    End entry reading                   */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                    Start entry writing                 */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_Internal_WriteValue */

 /*
   Returns:

     CFG_ENTRY_CREATED if new entry should be created
     CFG_CRITICAL_ERROR if critical error
     CFG_OK just change value
 */

CFG_Internal_EntryMapItor CFG_Internal_WriteValue(CFG_String_Arg key, int type, int * result)
 {
  CFG_Internal_Group * selected_group = CFG_Internal_selected_file->internal_file->selected_group;

  CFG_Internal_EntryMapItor iterator = selected_group->entries.find(key);

  if (iterator == selected_group->entries.end())
   {
    /* create new entry */
    *result = CFG_ENTRY_CREATED;
    CFG_SetError("You tried to write to entry that don't exists - it was created.");
    return iterator;
   }
  else /* such entry exists */
   {
    if ((*iterator).second.type != type)
     {
      /* set error */
      *result = CFG_ERROR;
      CFG_SetError("Entry type mismatch occured during writing.");
      return iterator;
     }
     
    *result = CFG_OK;
    return iterator;
   }
 }

/* ------------------------------------------------------- end CFG_Internal_WriteValue */

/* ------------------------------------------------------- start CFG_WriteBool */

int CFG_WriteBool(CFG_String_Arg key, CFG_Bool value)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_EntryMapItor iterator = CFG_Internal_WriteValue(key, CFG_BOOL, &result);

  if (result == CFG_OK)
   {
    if (value)
     (*iterator).second.value = CFG_KEYWORD_TRUE_1;
    else
     (*iterator).second.value = CFG_KEYWORD_FALSE_1;

    return CFG_OK;
   }

  if (result == CFG_ENTRY_CREATED)
   {
    CFG_Internal_Entry entry_body;

    if (value)
     entry_body.value = CFG_KEYWORD_TRUE_1;
    else
     entry_body.value = CFG_KEYWORD_FALSE_1;

    entry_body.type = CFG_BOOL;
    entry_body.order = ++CFG_Internal_selected_file->internal_file->selected_group->next_entry_order;

    CFG_Internal_selected_file->internal_file->selected_group->entries.insert(make_pair( key, entry_body) );

    return CFG_ENTRY_CREATED;
   }
   
  CFG_SetError("Found entry differs in type.");
  return CFG_ERROR;
 }

/* ------------------------------------------------------- end CFG_WriteBool */

/* ------------------------------------------------------- start CFG_WriteInt */

int CFG_WriteInt(CFG_String_Arg key, Sint32 value)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_EntryMapItor iterator = CFG_Internal_WriteValue(key, CFG_INTEGER, &result);

  if (result == CFG_OK)
   {
    char tmp[33];
    sprintf(tmp, "%d", value);
    //itoa(value, tmp, 10);

    (*iterator).second.value = tmp;
    return CFG_OK;
   }

  if (result == CFG_ENTRY_CREATED)
   {
    CFG_Internal_Entry entry_body;

    char tmp[33];
    sprintf(tmp, "%d", value);
    //itoa(value, tmp, 10);

    entry_body.value = tmp;
    entry_body.type = CFG_INTEGER;
    entry_body.order = ++CFG_Internal_selected_file->internal_file->selected_group->next_entry_order;

    CFG_Internal_selected_file->internal_file->selected_group->entries.insert(make_pair( key, entry_body) );

    return CFG_ENTRY_CREATED;
   }

   
  CFG_SetError("Found entry differs in type.");
  return CFG_ERROR;
 }

/* ------------------------------------------------------- end CFG_WriteInt */

/* ------------------------------------------------------- start CFG_WriteFloat */

int CFG_WriteFloat(CFG_String_Arg key, float value)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_EntryMapItor iterator = CFG_Internal_WriteValue(key, CFG_FLOAT, &result);

  if (result == CFG_OK)
   {
    char tmp[33];
    sprintf(tmp, "%f", value);

    (*iterator).second.value = tmp;
    return CFG_OK;
   }

  if (result == CFG_ENTRY_CREATED)
   {
    CFG_Internal_Entry entry_body;

    char tmp[33];
    sprintf(tmp, "%f", value);

    entry_body.value = tmp;
    entry_body.type = CFG_FLOAT;
    entry_body.order = ++CFG_Internal_selected_file->internal_file->selected_group->next_entry_order;

    CFG_Internal_selected_file->internal_file->selected_group->entries.insert(make_pair( key, entry_body) );

    return CFG_ENTRY_CREATED;
   }
   
  CFG_SetError("Found entry differs in type.");
  return CFG_ERROR;
 }

/* ------------------------------------------------------- end CFG_WriteFloat */

/* ------------------------------------------------------- start CFG_WriteText */

int CFG_WriteText(CFG_String_Arg key, CFG_String_Arg value)
 {
  CFG_ASSERT_NULL
  int result;
  CFG_Internal_EntryMapItor iterator = CFG_Internal_WriteValue(key, CFG_TEXT, &result);

  if (result == CFG_OK)
   {
    (*iterator).second.value = value;
    return CFG_OK;
   }

  if (result == CFG_ENTRY_CREATED)
   {
    CFG_Internal_Entry entry_body;
    
    entry_body.value = value;
    entry_body.type = CFG_TEXT;
    entry_body.order = ++CFG_Internal_selected_file->internal_file->selected_group->next_entry_order;

    CFG_Internal_selected_file->internal_file->selected_group->entries.insert(make_pair( key, entry_body) );

    return CFG_ENTRY_CREATED;
   }

  CFG_SetError("Found entry differs in type.");
  return CFG_ERROR;
 }

/* ------------------------------------------------------- end CFG_WriteText */

/* -------------------------------------------------------*/
/*                     End entry writing                  */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                       Start saving                     */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_Internal_insert_prefix */

void CFG_Internal_InsertPrefix(CFG_String & comment, int prefix_size)
 {
  CFG_String prefix = "  ";

  /* Little optimization - instead of adding 4x ' ', add only 2x here and 2x in initialization */
  for (int i = 0; i < prefix_size - 2; ++i)
   prefix += ' ';

  comment.insert(0, prefix);

  int pos = comment.find('\n');

  while (pos != CFG_NPOS)
   {
    comment.insert(pos + 1, prefix);
    pos = comment.find('\n', pos + prefix.size());
   }
 };

/* ------------------------------------------------------- end CFG_Internal_insert_prefix */

/* ------------------------------------------------------- start CFG_Internal_SaveEntry */

void CFG_Internal_SaveEntry( SDL_RWops * destination, CFG_Internal_Entry * entry, CFG_Internal_String_Arg name,
                             int prefix_size, int type)
 {
  
  CFG_Char prefix[] = { ' ', ' ', ' ', ' ', '\n' };
  
  int pre_comment_allowed = ( (entry->pre_comment.size() > 0)
                               && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS ))
                               && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS_ENTRIES )) );
  
  if ( pre_comment_allowed )
   {
    if ( (CFG_Internal_save_file_flags & CFG_SPACE_ENTRIES ) && ( CFG_Internal_not_first_entry == 1 ) &&
         !(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT ) )
     {
      /* Dump newline so that 2 entries (this one with pre comment) won't be glued together */
      SDL_RWwrite( destination, prefix + 4, 1, 1);
     }
     
     if ( (entry->pre_comment_type != CFG_C_LIKE_COMMENT) && !(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT) )
      {
       /* It may be bunch of comments glued together ie.
       
       // comment 1\n // comment 2\n // comment 3
       
       We must insert prefix after every found newline (\n) to copy of that string, and
       this is done by CFG_Internal_insert_prefix() function.
       
       */
       CFG_String tmp = entry->pre_comment;
       CFG_Internal_InsertPrefix(tmp, prefix_size);

       SDL_RWwrite( destination, tmp.c_str(), tmp.size(), 1);
      }
     else
      {
       SDL_RWwrite( destination, entry->pre_comment.c_str(), entry->pre_comment.size(), 1);
      }

    /* Add newline after pre comment, since it wasn't appended by it */
    SDL_RWwrite( destination, prefix + 4, 1, 1);
   }

  /* Add 2 or 4 whitespace prefix before entry's name */
  if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
   SDL_RWwrite( destination, prefix, prefix_size, 1);
  
  /* Save entry's name */
  SDL_RWwrite( destination, name.c_str(), name.size(), 1);
  
 /* ------------------------------------------------------- */
  
  CFG_Char middle[] = { ' ', CFG_OPERATOR_EQUALS, ' ', CFG_OPERATOR_DBL_QUOTE };
  
  if (CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT)
   {
    middle[0] = CFG_OPERATOR_EQUALS;
    middle[1] = CFG_OPERATOR_DBL_QUOTE;
    
    if (type == CFG_TEXT) /* Write '="' */
     SDL_RWwrite( destination, middle, 2, 1);
    else /* Write '=' */
     SDL_RWwrite( destination, middle, 1, 1);
   }
  else
   {
    if (type == CFG_TEXT) /* Write ' = "' */
     SDL_RWwrite( destination, middle, 4, 1);
    else /* Write ' = ' */
     SDL_RWwrite( destination, middle, 3, 1);
   }

  /* Save entry' value */
  if (type != CFG_TEXT)
   SDL_RWwrite( destination, entry->value.c_str(), entry->value.size(), 1);
  else
   {
    /* Temporary replace all " with \" */
    CFG_String tmp = entry->value;
    
    for (int i = 0; i < tmp.size(); ++i)
     if (tmp[i] == CFG_OPERATOR_DBL_QUOTE)
      {
       tmp.insert(i, 1, CFG_OPERATOR_ESCAPE_SEQ );
       ++i;
      }
    
    SDL_RWwrite( destination, tmp.c_str(), tmp.size(), 1);
   }
  
 /* ------------------------------------------------------- */
  
  int post_comment_allowed = ( (entry->post_comment.size() > 0)
                                && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS ))
                                && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS_ENTRIES )) );
  
  if (type == CFG_TEXT)
   {
    middle[0] = CFG_OPERATOR_DBL_QUOTE;
    
    if ( post_comment_allowed )
     {
      /* Write '" ' */
      middle[1] = ' ';
      
      if (CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT)
       SDL_RWwrite( destination, middle, 1, 1);
      else
       SDL_RWwrite( destination, middle, 1, 2);

      /* Write post comment */
      SDL_RWwrite( destination, entry->post_comment.c_str(), entry->post_comment.size(), 1);
      
      /* Write \n */
      middle[0] = '\n';
      SDL_RWwrite( destination, middle, 1, 1);
     }
    else
     {
      middle[1] = '\n';
      SDL_RWwrite( destination, middle, 1, 2);
     }

    CFG_Internal_not_first_entry = 1;
    return;
   }

  /* It's not text */
  
  if (post_comment_allowed)
   {
    middle[0] = ' ';

    if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
     SDL_RWwrite( destination, middle, 1, 1); /* One space to space them ;-) */

    SDL_RWwrite( destination, entry->post_comment.c_str(), entry->post_comment.size(), 1);
   }

   middle[0] = '\n';
   SDL_RWwrite( destination, middle, 1, 1);
   CFG_Internal_not_first_entry = 1;
 }
 
/* ------------------------------------------------------- end CFG_Internal_SaveEntry */

  namespace std
   {

    template <>
    struct less< CFG_Internal_Entry  >
      {
       bool operator()( const CFG_Internal_Entry * _first, const CFG_Internal_Entry * _second )
        {
         return (_first->order < _second->order);
        }
      };
   }

 typedef std :: map < CFG_Internal_Entry * , CFG_String *, less< CFG_Internal_Entry > >
         CFG_Internal_OriginalFileEntryMap;

 typedef CFG_Internal_OriginalFileEntryMap :: iterator CFG_Internal_OriginalFileEntryMapItor;
 typedef CFG_Internal_OriginalFileEntryMap :: const_iterator CFG_Internal_OriginalFileEntryMapConstItor;


/* ------------------------------------------------------- start CFG_Internal_SaveGroup */

void CFG_Internal_SaveGroup(CFG_Internal_Group * group, SDL_RWops * destination,
                            CFG_Internal_String_Arg group_name, int is_global, int mode)
 {
  CFG_Char group_start[5] = { '\n', '\n', ' ', ' ', CFG_OPERATOR_GROUP_START };
  
  if (is_global == 0)
   {
   /* It's not global group */
    if ( (group->pre_comment.size() > 0) && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS ))
                                         && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS_GROUPS )))
     {
      if (CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT)
       {
        SDL_RWwrite( destination, group->pre_comment.c_str(), 1, group->pre_comment.size() );
        
        /* Write "\n[" */
        group_start[2] = CFG_OPERATOR_GROUP_START;
        SDL_RWwrite( destination, group_start + 1, 1, 2  );
        group_start[2] = ' '; // probably not needed
       }
      else
       {
        SDL_RWwrite( destination, group_start, 1, 2 );

        if ( (group->pre_comment_type != CFG_C_LIKE_COMMENT) )
         {
          /* It may be bunch of comments glued together etc. */
          CFG_String tmp = group->pre_comment;
          CFG_Internal_InsertPrefix(tmp, 2);

          SDL_RWwrite( destination, tmp.c_str(), 1, tmp.size() );
         }
         
        /* Write "\n  [" */
        SDL_RWwrite( destination, group_start + 1, 1, 4  );
       }
     }
    else
     {
      if (CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT)
       SDL_RWwrite( destination, group_start + 4, 1, 1);
      else
       SDL_RWwrite( destination, group_start, 5, 1);
     }

    /* Write group name */
    SDL_RWwrite( destination, group_name.c_str(), 1, group_name.size());
    
    group_start[0] = CFG_OPERATOR_GROUP_END;
    
    if ( (group->post_comment.size() > 0) && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS ))
                                          && (!(CFG_Internal_save_file_flags & CFG_NO_COMMENTS_GROUPS )) )
     {
      if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
       {
        group_start[1] = ' ';
        SDL_RWwrite( destination, group_start, 1, 2);
       }
      else
       SDL_RWwrite( destination, group_start, 1, 1);
       
      SDL_RWwrite( destination, group->post_comment.c_str(), group->post_comment.size(), 1);
      
      group_start[0] = group_start[1] = '\n';
      if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
       SDL_RWwrite( destination, group_start, 1, 2);
      else
       SDL_RWwrite( destination, group_start, 1, 1);
     }
    else
     {
      group_start[1] = group_start[2] = '\n';

      if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
       SDL_RWwrite( destination, group_start, 1, 3);
      else
       SDL_RWwrite( destination, group_start, 1, 2);
     }
   }
  else if (!(CFG_Internal_save_file_flags & CFG_COMPRESS_OUTPUT))
   SDL_RWwrite( destination, group_start, 1, 1); /* dump one line before first entry in global group */

 /* ------------------------------------------------------- Entries */
  
  /* four spaces of prefix for all entries found in group */
  int prefix_size = 4;

  /* but if it's global, only two spaces */
  if (is_global == 1)
   prefix_size = 2;
   
  CFG_Internal_not_first_entry = 0;
  
  if ( mode == CFG_SORT_CONTENT)
   {
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
      CFG_Internal_SaveEntry( destination, &iterator->second, iterator->first, prefix_size, iterator->second.type);
   }
  else if (mode == CFG_SORT_TYPE_CONTENT )
   {
   
    /* first save booleans */
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
     if (iterator->second.type == CFG_BOOL)
      {
       CFG_Internal_SaveEntry( destination, &iterator->second, iterator->first, prefix_size, CFG_BOOL);
      }

    /* now integer */
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
     if (iterator->second.type == CFG_INTEGER)
      {
       CFG_Internal_SaveEntry( destination, &iterator->second, iterator->first, prefix_size, CFG_INTEGER);
      }
   
    /* float */
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
     if (iterator->second.type == CFG_FLOAT)
      {
       CFG_Internal_SaveEntry( destination, &iterator->second, iterator->first, prefix_size, CFG_FLOAT);
      }
    
    /* and last but not least, text */
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
     if (iterator->second.type == CFG_TEXT)
      {
       CFG_Internal_SaveEntry( destination, &iterator->second, iterator->first, prefix_size, CFG_TEXT);
      }
      
   }
  else if (mode == CFG_SORT_ORIGINAL)
   {
    CFG_Internal_OriginalFileEntryMap originalFileMap;
    
    for (CFG_Internal_EntryMapItor iterator = group->entries.begin(); iterator != group->entries.end(); ++iterator)
      originalFileMap.insert(  make_pair < CFG_Internal_Entry * , CFG_String * >
                             ( &iterator->second, const_cast < string *> (&iterator->first) ));
      
    for (CFG_Internal_OriginalFileEntryMapItor iterator = originalFileMap.begin();
         iterator != originalFileMap.end(); ++iterator)
     {
      CFG_Internal_SaveEntry( destination, iterator->first, *iterator->second, prefix_size, iterator->first->type);
     }
   }
 }
 
/* ------------------------------------------------------- end CFG_Internal_SaveGroup */

  namespace std
   {

    template <>
    struct less< CFG_Internal_Group  >
      {
       bool operator()( const CFG_Internal_Group * _first, const CFG_Internal_Group * _second )
        {
         return (_first->order < _second->order);
        }
      };
   }

 typedef std :: map < CFG_Internal_Group * , CFG_String *, less< CFG_Internal_Group > >
         CFG_Internal_OriginalFileGroupMap;

 typedef CFG_Internal_OriginalFileGroupMap :: iterator CFG_Internal_OriginalFileGroupMapItor;
 typedef CFG_Internal_OriginalFileGroupMap :: const_iterator CFG_Internal_OriginalFileGroupMapConstItor;


/* ------------------------------------------------------- start CFG_Internal_SaveFile */

int CFG_Internal_SaveFile( SDL_RWops * destination, int mode, int flags, CFG_String_Arg filename )
 {
  /*CFG_Internal_selected_file->internal_file->filename.c_str()*/
  CFG_DEBUG("\nSDL_Config: saving to file: %s.\n\n", filename ); 
  
  if ( (mode != CFG_SORT_ORIGINAL ) && (mode != CFG_SORT_CONTENT ) && (mode !=  CFG_SORT_TYPE_CONTENT ) )
   {
    CFG_SetError("Unknown sorting type value at mode parameter.");
    return CFG_CRITICAL_ERROR;
   }

  CFG_Internal_save_file_flags = flags;

  #ifdef CFG_PROFILE_LIB
   Uint32 time_start = SDL_GetTicks();
  #endif

  /* global entry must be written first and without normal group header */
  CFG_Internal_SaveGroup( &CFG_Internal_selected_file->internal_file->global_group, destination, "", 1, mode);

  /* now save to file all other groups */
  if ( (mode == CFG_SORT_TYPE_CONTENT) || (mode == CFG_SORT_CONTENT) )
   {
    for (CFG_Internal_GroupMapItor iterator = CFG_Internal_selected_file->internal_file->groups.begin();
         iterator != CFG_Internal_selected_file->internal_file->groups.end(); ++iterator)
     {
      CFG_Internal_SaveGroup( &(iterator->second), destination, iterator->first, 0, mode);
     }
   }
  else if (mode == CFG_SORT_ORIGINAL)
   {
    /* sort them by order */
    CFG_Internal_OriginalFileGroupMap originalFileMap;

    for (CFG_Internal_GroupMapItor iterator = CFG_Internal_selected_file->internal_file->groups.begin();
         iterator != CFG_Internal_selected_file->internal_file->groups.end(); ++iterator)

         originalFileMap.insert(  make_pair < CFG_Internal_Group * , CFG_String * >
                               ( &iterator->second, const_cast < string *> (&iterator->first) ));

    for (CFG_Internal_OriginalFileGroupMapItor iterator = originalFileMap.begin();
         iterator != originalFileMap.end(); ++iterator)
     {
      CFG_Internal_SaveGroup( iterator->first, destination, *iterator->second, 0, mode);
     }

   }
   
  #ifdef CFG_PROFILE_LIB
   Uint32 time_end = SDL_GetTicks();

   Uint32 time_diff = time_end - time_start;

   fprintf(stdout, "\n %s >>> >>> Saved in: %i ms\n",
           CFG_Internal_selected_file->internal_file->filename.c_str(), time_diff);
  #endif

  return CFG_OK;
 }

/* ------------------------------------------------------- end CFG_Internal_SaveFile */

/* ------------------------------------------------------- start CFG_SaveFile_RW */

int CFG_SaveFile_RW( SDL_RWops * destination, int mode, int flags )
 {
  CFG_ASSERT_NULL
  return CFG_Internal_SaveFile( destination, mode, flags, "SDL_RWops stream" );
 }

/* ------------------------------------------------------- end CFG_SaveFile_RW */

/* ------------------------------------------------------- start CFG_SaveFile */

int CFG_SaveFile( CFG_String_Arg filename, int mode, int flags )
 {
  CFG_ASSERT_NULL
  /* construct SDL_RWops from filename and pass it to CFG_Internal_SaveFile */
  SDL_RWops * dest = SDL_RWFromFile(filename, "w");

  if (dest == 0)
   {
    CFG_SetError("Failed to open file for saving operation.");
    return CFG_ERROR;
   }

  int result = CFG_Internal_SaveFile( dest, mode, flags,
               ((filename) ? (filename) : (CFG_Internal_selected_file->internal_file->filename.c_str())) );

  SDL_RWclose(dest);
  return result;
 }

/* ------------------------------------------------------- end CFG_SaveFile */

/* -------------------------------------------------------*/
/*                        End saving                      */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                     Start removing                     */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_RemoveGroup */

int CFG_RemoveGroup(CFG_String_Arg group)
 {
  CFG_ASSERT_NULL
  CFG_Internal_Group * global_group = &CFG_Internal_selected_file->internal_file->global_group;

  if (group == 0)
   {
    /* Clean global group */
    global_group->entries.clear();
    return CFG_OK;
   }
   
  CFG_Internal_GroupMapItor iterator = CFG_Internal_selected_file->internal_file->groups.find(group);
  
  if (iterator == CFG_Internal_selected_file->internal_file->groups.end())
   {
    CFG_SetError("Request to remove group that doesn't exist.");
    return CFG_ERROR;
   }

  iterator->second.entries.clear();
  
  if ( &(iterator->second) == global_group)
   {
    global_group->entries.clear();
    return CFG_OK;
   }
  else
   {
    if (&(iterator->second) == CFG_Internal_selected_file->internal_file->selected_group)
     CFG_Internal_selected_file->internal_file->selected_group = global_group;
     
    CFG_Internal_selected_file->internal_file->groups.erase(iterator);
    
    return CFG_OK;
   }

 }

/* ------------------------------------------------------- end CFG_RemoveGroup */

/* ------------------------------------------------------- start CFG_Internal_RemoveEntry */

int CFG_Internal_RemoveEntry(CFG_String_Arg entry, int type)
 {
  CFG_Internal_Group * selected_group = CFG_Internal_selected_file->internal_file->selected_group;
  CFG_Internal_EntryMapItor iterator = selected_group->entries.find(entry);

  if ( iterator == selected_group->entries.end())
   {
    CFG_SetError("Couldn't find entry, which was requested for removal.");
    return CFG_ERROR;
   }

  if (iterator->second.type == type)
   {
    selected_group->entries.erase(iterator);
    return CFG_OK;
   }
  else
   {
    CFG_SetError("Type of found entry differs from expected.");
    return CFG_ERROR;
   }
 }

/* ------------------------------------------------------- end CFG_Internal_RemoveEntry */

/* ------------------------------------------------------- start CFG_RemoveBoolEntry */

int CFG_RemoveBoolEntry(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_RemoveEntry(entry, CFG_BOOL);
 }

/* ------------------------------------------------------- end CFG_RemoveBoolEntry */

/* ------------------------------------------------------- start CFG_RemoveIntEntry */

int CFG_RemoveIntEntry(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_RemoveEntry(entry, CFG_INTEGER);
 }

/* ------------------------------------------------------- end CFG_RemoveIntEntry */

/* ------------------------------------------------------- start CFG_RemoveFloatEntry */

int CFG_RemoveFloatEntry(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_RemoveEntry(entry, CFG_FLOAT);
 }

/* ------------------------------------------------------- end CFG_RemoveFloatEntry */

/* ------------------------------------------------------- start CFG_RemoveTextEntry */

int CFG_RemoveTextEntry(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_RemoveEntry(entry, CFG_TEXT);
 }

/* ------------------------------------------------------- end CFG_RemoveTextEntry */

/* -------------------------------------------------------*/
/*                       End removing                     */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                Start existance checking                */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_Internal_EntryExists */

CFG_Bool CFG_Internal_EntryExists(CFG_String_Arg entry, int type)
 {
  if (entry == 0)
   return CFG_False;

  CFG_Internal_Group * selected_group = CFG_Internal_selected_file->internal_file->selected_group;

  CFG_Internal_EntryMapItor iterator = selected_group->entries.find(entry);

  if ( iterator == selected_group->entries.end())
   {
    return CFG_False; /* Entry not exists */
   }

  if (iterator->second.type == type)
   {
    return CFG_True; /* Entry exists and has the same type */
   }
  else
   {
    return CFG_False; /* Entry exists but has different type */
   }
 }

/* ------------------------------------------------------- end CFG_Internal_EntryExists */

/* ------------------------------------------------------- start CFG_BoolEntryExists */

CFG_Bool CFG_BoolEntryExists(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_EntryExists(entry, CFG_BOOL );
 }

/* ------------------------------------------------------- end CFG_BoolEntryExists */

/* ------------------------------------------------------- start CFG_IntEntryExists */

CFG_Bool CFG_IntEntryExists(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_EntryExists(entry, CFG_INTEGER );
 }

/* ------------------------------------------------------- end CFG_ EntryExists */

/* ------------------------------------------------------- start CFG_FloatEntryExists */

CFG_Bool CFG_FloatEntryExists(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_EntryExists(entry, CFG_FLOAT );
 }

/* ------------------------------------------------------- end CFG_FloatEntryExists */

/* ------------------------------------------------------- start CFG_TextEntryExists */

CFG_Bool CFG_TextEntryExists(CFG_String_Arg entry)
 {
  CFG_ASSERT_NULL
  return CFG_Internal_EntryExists(entry, CFG_TEXT );
 }

/* ------------------------------------------------------- end CFG_TextEntryExists */

/* ------------------------------------------------------- start CFG_GroupExists */

CFG_Bool CFG_GroupExists(CFG_String_Arg group)
 {
  CFG_ASSERT_NULL
  
  if (group == 0)
   return CFG_False;
   
  return ( CFG_Internal_selected_file->internal_file->groups.find(group) !=
           CFG_Internal_selected_file->internal_file->groups.end() );
 }

/* ------------------------------------------------------- end CFG_GroupExists */

/* -------------------------------------------------------*/
/*                  End existance checking                */
/* -------------------------------------------------------*/

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */

/* -------------------------------------------------------*/
/*                   Start group iteration                */
/* -------------------------------------------------------*/

/* ------------------------------------------------------- start CFG_StartGroupIteration */

void CFG_StartGroupIteration( void )
 {
  CFG_ASSERT_NULL
  CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;

  if ( internal_file->iteration_direction == CFG_ITERATE_NORMAL )
   {
    internal_file->group_iterator = internal_file->groups.begin();
    internal_file->selected_group = &internal_file->group_iterator->second;
   }
  else if ( internal_file->iteration_direction == CFG_ITERATE_REVERSE)
   {
    internal_file->reverse_group_iterator = internal_file->groups.rbegin();
    internal_file->selected_group = &internal_file->reverse_group_iterator->second;
   }
  else
   {
    CFG_LOG_CRITICAL_ERROR(004)
   }
 }

/* ------------------------------------------------------- end CFG_StartGroupIteration */

/* ------------------------------------------------------- start CFG_SelectNextGroup */

void CFG_SelectNextGroup( void )
 {
  CFG_ASSERT_NULL
  CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;
  
  if ( internal_file->iteration_direction == CFG_ITERATE_NORMAL )
   {
    if ( internal_file->group_iterator != internal_file->groups.end() )
     {
      ++internal_file->group_iterator;
      internal_file->selected_group = &internal_file->group_iterator->second;
     }
   }
  else if ( internal_file->iteration_direction == CFG_ITERATE_REVERSE )
   {
    if ( internal_file->reverse_group_iterator != internal_file->groups.rend() )
     {
      ++internal_file->reverse_group_iterator;
      internal_file->selected_group = &internal_file->reverse_group_iterator->second;
     }
   }
  else
   {
    CFG_LOG_CRITICAL_ERROR(004)
   }
 }

/* ------------------------------------------------------- end CFG_SelectNextGroup */

/* ------------------------------------------------------- start CFG_IsLastGroup */

CFG_Bool CFG_IsLastGroup( void )
 {
  CFG_ASSERT_NULL
  CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;

  if ( internal_file->iteration_direction == CFG_ITERATE_NORMAL )
   {
    return (CFG_Internal_selected_file->internal_file->group_iterator ==
            CFG_Internal_selected_file->internal_file->groups.end() );
   }
  else if ( internal_file->iteration_direction == CFG_ITERATE_REVERSE )
   {
    return (CFG_Internal_selected_file->internal_file->reverse_group_iterator ==
            CFG_Internal_selected_file->internal_file->groups.rend() );
   }
  else
   {
    CFG_LOG_CRITICAL_ERROR(004)
    return CFG_False;
   }
 }

/* ------------------------------------------------------- end CFG_IsLastGroup */

/* ------------------------------------------------------- start CFG_RemoveSelectedGroup */

void CFG_RemoveSelectedGroup(void)
 {
  CFG_ASSERT_NULL
  CFG_Internal_File * internal_file = CFG_Internal_selected_file->internal_file;

  if ( internal_file->iteration_direction == CFG_ITERATE_NORMAL )
   {
    internal_file->groups.erase( internal_file->group_iterator );
   }
  else if ( internal_file->iteration_direction == CFG_ITERATE_REVERSE )
   {
    /* For explanation, see: http://www.gamedev.net/community/forums/topic.asp?topic_id=346196 */
    CFG_Internal_GroupMapItor normal_iterator = internal_file->reverse_group_iterator.base();
    --normal_iterator;

    internal_file->groups.erase( normal_iterator );
   }
  else
   {
    CFG_LOG_CRITICAL_ERROR(004)
   }
 }

/* ------------------------------------------------------- end CFG_RemoveSelectedGroup */

/* ------------------------------------------------------- start CFG_SetIterationDirection */

int CFG_SetIterationDirection( int direction )
 {
  CFG_ASSERT_NULL
  int tmp = CFG_Internal_selected_file->internal_file->iteration_direction;
  
  if ( direction != 0 )
   CFG_Internal_selected_file->internal_file->iteration_direction = direction;

  return tmp;
 }

/* ------------------------------------------------------- end CFG_SetIterationDirection */

/* -------------------------------------------------------*/
/*                    End group iteration                 */
/* -------------------------------------------------------*/
