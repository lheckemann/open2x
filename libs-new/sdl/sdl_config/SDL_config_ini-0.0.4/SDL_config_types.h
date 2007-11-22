
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

/* ------------------------------------------------------- */
/*            Ssssh, secret, don't tell anybody!           */
/* ------------------------------------------------------- */

/*

 File: Structures in this file for internal use of library only.

 This documentation shouldn't be mixed with normal library API, it is documentation of *internal*
 SDL_Config structures, and as such, it's not for normal library user.
 
 
 This file should be included only once (hence no compilation guards).

*/

/*

 They are used internally to distinguish between those four entry types.

*/

#define CFG_BOOL 1
#define CFG_INTEGER 2
#define CFG_FLOAT 3
#define CFG_TEXT 4

/* ------------------------------------------------------- CFG_Internal_Entry */

/*

 Structure: CFG_Internal_Entry

 Encapsulates single entry with its value. Sizeof ( CFG_Internal_Entry ) with padding = 20.

*/


typedef struct CFG_Internal_Entry
 {
  /*
   Variable: value
   
   String which is holding unconverted value.
  
  */

  CFG_String value;

  /*

  Variable: order
  
  Used to sort entries during save operation in order as in file they were originally read from.
  
  */
  
  Uint32 order;
  
  /*
   Variable: pre_comment

   String which is holding comment that was found before entry definition, ie.

   // pre comment
   entry = value

  */

  CFG_String pre_comment;

  
  /*
   Variable: post_comment

   String which is holding comment that was found after entry definition, ie. entry = value // post comment

  */

  CFG_String post_comment;
  
/*

   Variable: pre_comment_type

   Type of pre comment:

    0 - no comment
    1 - //, ; or #
    2 - C like

  */

  Uint8 pre_comment_type;

  
/*

   Variable: post_comment_type

   Currently for padding, maybe in later versions it will contain type of post comment (now it's not needed).

  */

 Uint8 post_comment_type;
 
 
/*

  Variable: type

  Depending on "type"'s value, "value" is treated and converted differently.

  Possible values for "type" are:

   - 0: reserved
   - CFG_BOOL: bool
   - CFG_INT: integer
   - CFG_FLOAT: float
   - CFG_TEXT: text

  */

  Uint8 type;


/* Used for padding */

  Uint8 reserved;

 };

/* ------------------------------------------------------- CFG_Internal_Group */


typedef std :: map < CFG_String, CFG_Internal_Entry > CFG_Internal_EntryMap;

typedef CFG_Internal_EntryMap :: iterator CFG_Internal_EntryMapItor;

/*

 Structure: CFG_Internal_Group
 
 Keeps entries of all types in single text-based map. Sizeof ( CFG_Internal_Group ) with padding = 32.

*/

typedef struct CFG_Internal_Group
 {
  CFG_Internal_EntryMap entries;
  
  /* Variable: order
  
   Used to sort groups during save operation in order as in file they were originally read from.

   */
  Uint32 order;
  
  /* Variable: next_entry_order
  
   Next created entry will have such order.
  
  */
  
  Uint32 next_entry_order;
  
  /*
   Variable: post_comment

   String which is holding comment that was found after group definition, ie. [group] // post comment

  */

  CFG_String post_comment;

  /*
   Variable: pre_comment

   String which is holding comment that was found before group definition, ie.
   // pre comment
   [group]

  */

  CFG_String pre_comment;
  
 /*

   Variable: pre_comment_type

   Type of pre comment:

    0 - no comment
    1 - //, ; or #
    2 - C like

  */

  Uint8 pre_comment_type;
  
  /* Used for padding */

  Uint8 reserved1;
  Uint8 reserved2;
  Uint8 reserved3;

 };


/* ------------------------------------------------------- CFG_Internal_File */

/*


*/

typedef std :: map < CFG_String, CFG_Internal_Group> CFG_Internal_GroupMap;

typedef CFG_Internal_GroupMap :: iterator CFG_Internal_GroupMapItor;
typedef CFG_Internal_GroupMap :: const_iterator CFG_Internal_GroupMapConstItor;
typedef CFG_Internal_GroupMap :: reverse_iterator CFG_Internal_GroupMapReverseItor;

/*

 Structure: CFG_Internal_File

*/

typedef struct CFG_Internal_File
 {
  /* Variable: groups
  
   Map which contains all groups.
  
  */

  CFG_Internal_GroupMap groups;
  
  /* Variable: selected_group
  
   Selected group. Always should point to vaild group.
  
  */
  
  CFG_Internal_Group * selected_group;
  
  /* Variable: group_iterator, reverse_group_iterator
    
  After every change in functions that *iterate*, it's dereferenced and its value is assigned to <selected_group>.
  
  */
  
  CFG_Internal_GroupMapItor group_iterator;
  CFG_Internal_GroupMapReverseItor reverse_group_iterator;
  
  /* Variable: iteration_direction
  
  1 means we're iterating to front group_iterator)
  -1 means we're reverse iterating (reverse_group_iterator)

  */
  
  int iteration_direction;
  
  /* Variable: global_group

   Global group is not stored directly in map, because we would have to reserve some kind of ID for global
   group name. Instead, we store it in object, treating it differently also in other ways in other places.
  */
  CFG_Internal_Group global_group;
  
  /* Variable: next_group_order

   Next created group will have such order.

  */
  
  Uint32 next_group_order;
  
  /* Variable: filename
  
  Name of hard-drive file, which was used to create this CFG_Internal_File.
  
  */
  
  CFG_String filename;
  
 };
