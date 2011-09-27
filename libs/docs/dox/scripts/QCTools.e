/*******************************************************************************
* QC Tools for SlickEdit
* E-mail: mkrishna
*******************************************************************************
* Revision History
* Author : MK (Unless explicitly stated for a macro)
* V1.0 (09/21/2010)
 1. Moved from MKsTools v7.2 to QCTools v1.0
 2. Added HTML Markup support for TP macros
******************************************************************************/


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "slick.sh" 
#include 'tagsdb.sh'

/*******************************************************************************
 * Constants
 ******************************************************************************/
#define MAX_COL 80
#define MAX_COMMENT_COL 60
#define INDENT 2 
#define QCTOOLSMENUNAME "&QC Tools"
#define QCTOOLSVERSION "v1.0"
/*******************************************************************************
 * Icons for each macro -- Change if you dont like the defaults
 ******************************************************************************/
#define ProcessHWIOMacroIcon     'QCTools_ProcessHWIOMacro.ico'
#define AlignEqualsIcon          'QCTools_AlignEquals.ico'
#define InsertFnCommentIcon      'QCTools_FuncComment.ico'
#define InsertDoxyFnCommentIcon  'QCTools_DoxygenComment.ico'
#define InsertBoxCommentIcon     'QCTools_BoxComment.ico'  
#define InsertDividerCommentIcon 'QCTools_HorizLineComment.ico'
#define InsertParenCommentIcon   'QCTools_ParenComment.ico'
#define ProcessDALMacroIcon      'QCTools_ProcessDALMacro.ico'

/************************************
 * Globals
 *******************************************/


/* Functions */

/*
Menu Insertion Code - No Copyright
*/

// Find item or submenu for a menu resource. This function is not recursive (i.e.
// it will only search one level).
//
// Return true if item or submenu found and set found_index, found_pos.
// Menu items will have found_index.p_object=OI_MENU_ITEM.
// Submenus will have found_index.p_object=OI_MENU.
static boolean findByIndex(int menu_index, _str findCaption, int& found_index, int& found_pos)
{
   found_index = 0;
   found_pos = -1;
   int first_child = menu_index.p_child;
   int child = first_child;
   int i = 0;
   while( child > 0 ) {
      _str caption = child.p_caption;
      // Do not compare accelerator letter
      if( strieq(stranslate(findCaption,'','&'),stranslate(caption,'','&')) ) {
         // Found
         found_index = child;
         found_pos = i;
         return true;
      }
      ++i;
      child = child.p_next;
      if( child == first_child ) {
         // Done
         break;
      }
   }
   // Not found
   return false;
}

// Add a top-level menu 'menuCaption' to a menu by resource index. Menu is
// added after the menu with caption=afterMenuCaption. If caption is not
// found or "", then menu is added as last menu.
//
// Note: Some _menu_* apis (like _menu_insert) can operate on either a resource handle or
// an active menu handle. Do not get the two concepts confused when writing your code.
// For example, do not use find_index to get the resource index of a menu, then make
// changes to it and expect those changes to be reflected right away in the currently
// showing instance of that menu.
//
// Return index of added menu.
static int addTopLevelMenuByIndex(int menu_index, _str menuCaption, _str afterMenuCaption="")
{
   // 0-based position of new menu
   int inserted_pos = -1;

   if( afterMenuCaption != "" ) {
      int found_index = 0;
      int found_pos = -1;
      if( findByIndex(menu_index,afterMenuCaption,found_index,found_pos) ) {
         inserted_pos = found_pos + 1;
      }
   }
   // Note: If inserted_pos is <0, then new menu is inserted last
   _menu_insert(menu_index,inserted_pos,MF_SUBMENU,menuCaption,"","QCTools","","");
   return inserted_pos;
}

static _command void QCToolsi_MenuAdd() name_info(',')
{
   int MDIindex = find_index("_mdi_menu",oi2type(OI_MENU));
   if(MDIindex == 0) {
      message("Unable to get the index for the mdimenu");
      return;
   }

      // Add "Test" menu after "Project"
      int menu_pos = addTopLevelMenuByIndex(MDIindex,QCTOOLSMENUNAME);
      // Find "Test" menu we just added
      int QCToolsMenu_index = 0;
      int QCToolsMenu_pos = -1;
      findByIndex(MDIindex,QCTOOLSMENUNAME,QCToolsMenu_index,QCToolsMenu_pos);
       // Add a few menu items to "Test" menu
      _menu_insert(QCToolsMenu_index,-1,0,"Process &HWIO Macro","QCTools_ProcessHWIOMacro","","help nothing","Processes the word under cursor as a HWIO Macro");
      _menu_insert(QCToolsMenu_index,-1,0,"Process &DAL Macro","QCTools_ProcessDALMacro","","help nothing","Processes word under cursor as a DAL Macro");
      // Add a separator after last item
      _menu_insert(QCToolsMenu_index,-1,0,"-");
      _menu_insert(QCToolsMenu_index,-1,0,"Do&x Function Comment","QCTools_InsertDoxygenFunctionComment","","help nothing","Inserts a doxygen comment block for a function");
      //   Living Docs Contribution 
      _menu_insert(QCToolsMenu_index,-1,MF_SUBMENU,"&HTML Comment Blocks");
      // Find the submenu we just added
      int HTMLSubmenu_index = 0;
      int HTMLSubmenu_pos = -1;
      findByIndex(QCToolsMenu_index,"&HTML Comment Blocks",HTMLSubmenu_index,HTMLSubmenu_pos);
             _menu_insert(HTMLSubmenu_index,-1,0,"Ordered List","QCTools_InsertHtmlOrderedList","","help nothing","Inserts a html ordered list");
             _menu_insert(HTMLSubmenu_index,-1,0,"UnOrdered List","QCTools_InsertHtmlUnOrderedList","","help nothing","Inserts html unordered list");
             _menu_insert(HTMLSubmenu_index,-1,0,"Table","QCTools_InsertHtmlTable","","help nothing","Inserts a html table");

      // Add a separator
       _menu_insert(QCToolsMenu_index,-1,0,"-");
      _menu_insert(QCToolsMenu_index,-1,0,"&Match Parens","QCTools_InsertParenComment","","help nothing","Inserts a comment indicating a matching paren");
      _menu_insert(QCToolsMenu_index,-1,0,"&Box Comment","QCTools_InsertBoxComment","","help nothing","Inserts a comment block");
      _menu_insert(QCToolsMenu_index,-1,0,"Di&vider","QCTools_InsertDividerComment","","help nothing","Inserts a comment line");
      // Add a separator
      _menu_insert(QCToolsMenu_index,-1,0,"-");
      _menu_insert(QCToolsMenu_index,-1,0,"&Align Equals","QCTools_AlignEquals","","help nothing","Aligns a block of statements around the equal sign");
      // Add a separator
      _menu_insert(QCToolsMenu_index,-1,0,"-");
      _menu_insert(QCToolsMenu_index,-1,0,"A&bout","QCTools_About","","help nothing","What else? An About dialog");


   /*
  Example code for adding a submenu

// Add a submenu
   _menu_insert(Test_index,-1,MF_SUBMENU,"&Submenu");
   // Find the submenu we just added
   int Submenu_index = 0;
   int Submenu_pos = -1;
   findByIndex(Test_index,"Submenu",Submenu_index,Submenu_pos);
   // Add submenu items
   _menu_insert(Submenu_index,-1,0,"Item &4","nothing","","help nothing","Does nothing");
   _menu_insert(Submenu_index,-1,0,"Item &5","nothing","","help nothing","Does nothing");
   _menu_insert(Submenu_index,-1,0,"Item &6","nothing","","help nothing","Does nothing");
*/
   // If you want "my_mdi_menu" to become the permanent replacement, even after
   // you restart SE, then you also have to set def_mdi_menu.
   //def_mdi_menu = "my_mdi_menu";
//   _load_mdi_menu();
   _menu_mdi_update();

}

static _command void QCToolsi_MenuDel() name_info(',')
{
   int MDIindex = find_index("_mdi_menu",oi2type(OI_MENU));
   if(MDIindex == 0) {
      message("Unable to get the index for the mdimenu");
      return;
   }
   // Find "Test" menu we just added
   int Test_index = 0;
   int Test_pos = -1;
   boolean status=findByIndex(MDIindex,QCTOOLSMENUNAME,Test_index,Test_pos);
   if(status == true) {
      _menu_delete(MDIindex,Test_pos);
      _menu_mdi_update();
   }
}

/*

Function Name : definit
Description : Main Entry function to this macro
*/
definit()
{
   //does nothing..
}

/*

Function Name : definit
Description : Main Entry function to this macro
*/

defload()
{        
   wid = _tbIsVisible("_tbQCTools_form");
   if (wid) {
      tbClose(wid);
   }
/*   _tbAddForm("_tbQCTools_form",1,true,BBSIDE_TOP,true);
   dock("_tbQCTools_form",BBSIDE_TOP);
   */
   QCToolsi_MenuDel();
   QCToolsi_MenuAdd();
}


/***************************************************
 *                                                 *
 *             Utility Functions to follow         *
 *                                                 *
 ***************************************************/


/**
 * @brief Shows the about dialog box
 * 
 * @author mkrishna (8/1/2009)
 */
_command void QCTools_About() name_info(',')
{
   _message_box(stranslate(QCTOOLSMENUNAME,'','&'):+" Version : ":+QCTOOLSVERSION,stranslate(QCTOOLSMENUNAME,'','&'),MB_OK|MB_ICONINFORMATION);
}

/*----------------------------------------------------
     ALIGN EQUALS FUNCTION
 *----------------------------------------------------*/

//
// Author:     Joseph Van Valen
//
// Description:
// Implements three commands borrowed from other editors. The common thread
// between each of the commands is that they operate on selected text and use
// the selection filter techniques supplied by Visual SlickEdit.

static int gMaxEqCol;

/**
 * A selection filter that determines the column of the right most equal sign.
 *
 * The filter finds the column of the first equal sign of the line if available.
 * If there is an equal sign, and the column is greater than the current maximum,
 * then the column will become the next maximum.
 *
 * This is a helper function for the align_equals command.
 *
 * @return
 */
static _str find_max_eq_filter(s)
{
   _str inputStr = expand_tabs(s);
   int equalsPos = pos("=", inputStr, 1, "");
   if ( equalsPos > gMaxEqCol  ) {
      gMaxEqCol = equalsPos;
   }

   return s;
}

/**
 * This selection filter injects spaces in front of the first equal sign of
 * the line until the equal sign is in the same column as the right most first
 * equal sign of the selection. If the line does not contain an equal sign, then
 * no action is taken.
 *
 * @return The resulting string with the first equal sign shifted, if applicable
 */
static _str align_eq_filter(s)
{
   if (gMaxEqCol <= 0 || length(s) == 0) {
      return s;
   }

   _str expandedStr = expand_tabs(s);

   int equalsPos  = pos("=", expandedStr, 1, "");
   if (equalsPos == 0) {
      return s;
   }

   _str prefix    = substr(expandedStr, 1, equalsPos - 1);
   _str postfix   = substr(expandedStr,equalsPos);

   while (equalsPos < gMaxEqCol ) {
      prefix = prefix :+ ' ';
      ++equalsPos;
   }

   return prefix :+ postfix;
}


/**
 * Aligns the first equal sign of each line of a selection to the right-most
 * equal sign. Lines without equal signs are not affected.
 *
 * Requires a Line or Block selection to identify the lines to align       
 */
_command void QCTools_AlignEquals() name_info(','VSARG2_MARK|VSARG2_REQUIRES_EDITORCTL|VSARG2_REQUIRES_AB_SELECTION)
{
   message(_select_type());
   gMaxEqCol = 0;
   filter_selection(find_max_eq_filter);

   if (gMaxEqCol == 0) {
      // no equal signs
      return;
   }

   filter_selection(align_eq_filter);
   _free_selection("");
}


/*----------------------------------------------------
     Insert Doxygen Comment
 *----------------------------------------------------*/
/**
 * @brief Inserts Doxygen comment block to the function 
 * 
 * @author mkrishna
 */
_command void QCTools_InsertDoxygenFunctionComment() name_info(','VSARG2_EDITORCTL|VSARG2_REQUIRES_MDI_EDITORCTL|VSARG2_MARK|VSARG2_REQUIRES_EDITORCTL) 
{ 

   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return; 
   }

/* Identify the function name */
   currfnname = current_proc(false);
   if (currfnname =="") {
      save_pos(currpos);

      next_proc();
      currfnname = current_proc(false)
                   //_message_box(currfnname);
      restore_pos(currpos);
   } else {
      prev_proc();
   }

   int curr_fnc_context_id = tag_find_context(currfnname,true,true,false);
   _str return_type,args[];
   tag_get_detail2(VS_TAGDETAIL_context_type, curr_fnc_context_id, type_name);
   if (tag_tree_type_is_func(type_name)) 
   {
         tag_get_detail2(VS_TAGDETAIL_context_return, curr_fnc_context_id, return_type);
         tag_get_detail2(VS_TAGDETAIL_context_type,  curr_fnc_context_id, type_name);
         tag_get_detail2(VS_TAGDETAIL_context_flags, curr_fnc_context_id, tag_flags);
         tag_get_detail2(VS_TAGDETAIL_context_args,  curr_fnc_context_id, signature);
//         if (return_type != '') {
//           caption = return_type :+ ' ' :+ caption;
//         }
//         sticky_message(
//            'Func : ':+ currfnname :+' Args: ':+signature :+ ' Return: ':+return_type:+' Type : ':+type_name);// + ' Flags : ' :+ tag_flags);
         split(signature,',',args);
   }
   //Function Name:
   _insert_text("/* ============================================================================\r\n",false,"\r\n"); 
   _insert_text("**  Function : " :+ currfnname :+ "\r\n",false,"\r\n"); 
   _insert_text("** ============================================================================\r\n",false,"\r\n");
   _insert_text("*/\r\n",false,"\r\n"); 
   _insert_text("/**\r\n",false,"\r\n");
   _insert_text("    @brief\r\n",false,"\r\n");
   _insert_text("    Brief_Description\r\n",false,"\r\n");
   _insert_text("    \r\n",false,"\r\n");
   _insert_text("    @details\r\n",false,"\r\n");
   _insert_text("    Detailed_Description\r\n",false,"\r\n");
   _insert_text("    \r\n",false,"\r\n");
   
   _str argnames[];
   _str argtypes[];
   int count = 0;
   int maxlength = 0;
   int maxtypelength = 0;
   //Args Types
   for(i = 0;i< args._length();i++) {
      _str temp[];
      args[i] = strip(args[i]);
      split(args[i]," ",temp);
      argnames[i] = temp[temp._length()-1];
      if(pos('const',args[i]) != 0) {
         argtypes[i] = "in";
      }
      else 
      {
         if(pos('*',args[i]) != 0) 
         {
            argtypes[i] = "in,out";
         }
         else
         {
            argtypes[i] = "in";
         }
      }
      if(maxlength < argnames[i]._length())
      {
         maxlength = argnames[i]._length();
      }
      if(maxtypelength < argtypes[i]._length()) 
      {
         maxtypelength = argtypes[i]._length();
      }
   }
//   _message_box("Max Length : " :+ maxlength :+ " Max Type Length : " :+ maxtypelength);
//   _message_box("Arg Names Count = " :+ argnames._length() :+ "  Args Count = " :+ args._length());
//   _message_box("Args : " :+ argnames[2]);
   
   if(args._length() == 1 && argnames[0] == "void") 
   {
       _insert_text("   @param " :+ " None\r\n",false,"\r\n");
   }
   else
   {
      for(i = 0; i < args._length(); i++) 
      {
         _str paramtext = "    @param[" :+ argtypes[i];
         for(j=0;j<maxtypelength - argtypes[i]._length(); j++) {
            paramtext = paramtext :+ ' ';
         }
         paramtext  = paramtext :+ "]  " :+argnames[i];
         for(j=0;j<maxlength + 3 - argnames[i]._length(); j++) {
            paramtext = paramtext :+ ' ';
         }
         paramtext = paramtext :+ " Param_description";
         //      _message_box(paramtext);
         _insert_text(paramtext :+"\r\n",false,"\r\n");
      }
   }
   _insert_text("\r\n",false,"\r\n");
   _insert_text("    @dependencies\r\n",false,"\r\n");
   _insert_text("    Dependency_description_or_None\r\n",false,"\r\n");
   _insert_text("    \r\n",false,"\r\n");
   _insert_text("    @sideeffects\r\n",false,"\r\n");
   _insert_text("    Sideeffect_description_or_None\r\n",false,"\r\n");
   _insert_text("    \r\n",false,"\r\n");

   //Return Types
   if(return_type == '' || return_type == 'void' ) {
      _insert_text("    @return\r\n",false,"\r\n");
      _insert_text("    None\r\n",false,"\r\n");
   }
   else
   {
      _insert_text("    @return\r\n",false,"\r\n");
      _insert_text("    Return_description\r\n",false,"\r\n");
   }
   _insert_text("    \r\n",false,"\r\n");
   _insert_text("    @sa\r\n",false,"\r\n");
   _insert_text("    Other_Relevant_APIS_or_None\r\n",false,"\r\n");
   _insert_text("*/\r\n",false,"\r\n");

   

//   _insert_text(Function_Desc_Comment_Text,false,"\r\n");
   return;
} 

/* 
 
   Living Docs Contribution 

*/

 /*----------------------------------------------------
     Insert Html Ordered List
 *----------------------------------------------------*/
/**
 * @brief Inserts a HTML Ordered List
 * 
 * @author Living Docs
 */
_command void QCTools_InsertHtmlOrderedList()name_info(','VSARG2_EDITORCTL|VSARG2_MARK)
{
	_str htmlOrderdList = "<ol>";
	strappend(htmlOrderdList,"\n");
	strappend(htmlOrderdList,"   ")
	strappend(htmlOrderdList,"<li>")
	strappend(htmlOrderdList,"</li>")
	strappend(htmlOrderdList,"\n")
   strappend(htmlOrderdList,"   ")
   strappend(htmlOrderdList,"<li>")
	strappend(htmlOrderdList,"</li>")
	strappend(htmlOrderdList,"\n")
	strappend(htmlOrderdList,"</ol>")
   _insert_text(htmlOrderdList);
}  

/*----------------------------------------------------
     Insert Html UnOrdered List
 *----------------------------------------------------*/
/**
 * @brief Inserts a HTML UnOrdered List
 * 
 * @author Living Docs
 */
_command void QCTools_InsertHtmlUnOrderedList()name_info(','VSARG2_EDITORCTL|VSARG2_MARK)
{
	_str htmlUnOrderdList = "<ul>";
	strappend(htmlUnOrderdList,"\n");
	strappend(htmlUnOrderdList,"   ")
	strappend(htmlUnOrderdList,"<li>")
	strappend(htmlUnOrderdList,"</li>")
	strappend(htmlUnOrderdList,"\n")
 	strappend(htmlUnOrderdList,"   ")
   strappend(htmlUnOrderdList,"<li>")
	strappend(htmlUnOrderdList,"</li>")
	strappend(htmlUnOrderdList,"\n")
	strappend(htmlUnOrderdList,"</ul>")
   _insert_text(htmlUnOrderdList);
}  


/*----------------------------------------------------
     Insert Html Table
 *----------------------------------------------------*/
/**
 * @brief Inserts a HTML Table
 * 
 * @author Living Docs
 */
_command void QCTools_InsertHtmlTable()name_info(','VSARG2_EDITORCTL|VSARG2_MARK)
{
	_str htmlTable = "<table>";
	strappend(htmlTable,"\n");
	strappend(htmlTable,"      ")
	strappend(htmlTable,"<tr>")
 	strappend(htmlTable,"\n");
	strappend(htmlTable,"         ")
   strappend(htmlTable,"<td>");
	strappend(htmlTable,"</td>")
  	strappend(htmlTable,"\n");
 	strappend(htmlTable,"         ")
   strappend(htmlTable,"<td>");
	strappend(htmlTable,"</td>")
  	strappend(htmlTable,"\n");
   strappend(htmlTable,"      ")
	strappend(htmlTable,"</tr>")
	strappend(htmlTable,"\n")
 	strappend(htmlTable,"</table>")
   _insert_text(htmlTable);
}  


/*----------------------------------------------------
     Insert C Box Comment
 *----------------------------------------------------*/

/** 
* Inserts a comment template at the current position. 
* Command can also be hooked to a button, or to a keystroke. 
* 
* @author mkrishna 
*/ 

/* Todo: The block selection does not work */

_command void QCTools_InsertBoxComment() name_info(','VSARG2_EDITORCTL|VSARG2_REQUIRES_MDI_EDITORCTL|VSARG2_MARK|VSARG2_REQUIRES_EDITORCTL) 
{ 

   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return; 
   }
   p_col = 0;
/* Prepare the comment box */
   /* First comment line */
   _str CommentBox = "  /*";
   int currpos = 4;
   while (currpos < 80) {
      strappend(CommentBox,'-');
      currpos++;
   }
   strappend(CommentBox,"\n");

   /* Second Line Just indent */
   currpos = 0;
   while (currpos < INDENT * 2) {
      strappend(CommentBox,' ');
      currpos++;
   }
   strappend(CommentBox,"\n");

   /* Third Line */
   strappend(CommentBox,'  ');

   currpos = INDENT;
   while (currpos < 80-2) {
      strappend(CommentBox,'-');
      currpos++;
   }
   strappend(CommentBox,"*/\n");

   if ( _select_type() != "LINE" && _select_type() != "BLOCK" && _select_type() != "CHAR") {
      _insert_text(CommentBox);
   }//Not a line or block selection
   /* Optional : Move the cursor to the expected comment input column */
   p_line = p_line - 2;
   p_col = INDENT * 2 + 1;
} 

/*----------------------------------------------------
     Insert C Divider Comment
 *----------------------------------------------------*/

/** 
* Inserts a Divider comment seperating declarations from commands. 
* Command can also be hooked to a button, or to a keystroke. 
* 
* @author mkrishna 
*/ 

/* Todo: The block selection does not work */

_command void QCTools_InsertDividerComment() name_info(','VSARG2_EDITORCTL|VSARG2_REQUIRES_MDI_EDITORCTL|VSARG2_MARK|VSARG2_REQUIRES_EDITORCTL) 
{ 


   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return; 
   }
   p_col = 0;
   /* Prepare the comment box */
   /* First comment line */
   _str CommentBox = "  /*";
   int currpos = 4;
   while (currpos < 50) {
      strappend(CommentBox,'-');
      currpos++;
   }
   strappend(CommentBox,"*/\n");
   if ( _select_type() != "LINE" && _select_type() != "BLOCK" && _select_type() != "CHAR") {
      _insert_text(CommentBox);
   }//Not a line or block selection
   /* Optional : Move the cursor to the expected comment input column */
   p_line = p_line - 2;
   p_col = INDENT * 2 + 1;
} 


/*----------------------------------------------------
     Insert Paren Comment 
 *----------------------------------------------------*/

/** 
* Inserts a comment on the closing paren, that shows
* the statement that started the block.
* 
* @author mkrishna 
*/ 


static _str braceable_keywords[]={'if', 'while','do','for','{','switch','case','else'};



_command int QCTools_InsertParenComment() name_info(','VSARG2_REQUIRES_EDITORCTL|VSARG2_READ_ONLY)
{
//   execute('bind-to-key -r QCTools_insert_braces_comment 'event2index(A_F12));
   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return -1; 
   }

   prepend_str = "";
   save_pos(currpos);
   c_col = p_col;
   c_line = p_line;
   get_line(textline);
   if (pos("}",textline,1,'I') == 0) {
      message(nls("No Brace found to add comment to!!"));
      return -1;
   }
   /* Are we at the end of the position? */
   fnname = current_proc(false);
   if (fnname == "") {
      /* Must be a function */
      p_line--;
   }
   int result = end_proc();
   if (result != 0) {
      message(nls("Anticipated a function body. But didn't find so!"));
      return 0;
   }

   if (c_line == p_line && c_col >= p_col) {
      p_line--; /* We need to get into the function to get the fn name */
      _str currfnname = current_proc();
      p_line++;
      message("String is "currfnname);
      /* We are next to the function end */
      save_pos(end_brace_pos);
      if (p_col > pos('}',p_line,1,"Ir")) {
         commentline =" /* "currfnname" */";
         // Go to the original cursor position.
         restore_pos(end_brace_pos);
         get_line_raw(line);
         line = strip(line,'T');
         line = line" "commentline;
         replace_line(line);
      }
      return 0;
   } /* if(c_line == ... */
   else {
      /* Retract to the original position and do a brace analysis */
      restore_pos(currpos);
      linepresent = p_line;
      save_pos(end_brace_pos);
      end_brace_col = p_col;
      get_line_raw(line);
      p_col = pos('}',line,1,"Ir");
      status = _find_matching_paren(def_pmatch_max_diff,true);
      if (status) {
         message("Unable to find the leading brace");
         return 0;
      }
      skipped_first_brace = 0;
      /* Get the current line */
      while (p_line != 1) {
         get_line(strline);
         i = 0;
         while (i < braceable_keywords._length()) {
            position = pos(braceable_keywords[i],strline);

            if (position != 0) {
               if (braceable_keywords[i] == "{" && skipped_first_brace == 1) {
                  /* Missed a braceable condition :( */
                  message("No matching if, while, for or do statement found");
                  return 0;
               }
               if (braceable_keywords[i] == "else") {
                  /* Requirement : All if and else blocks will be guarded 
                                   with {} guards per coding standards
                   */
                  /* Find its matching if block */

                  position = pos("}",strline);
                  if (position == 0) {
                     p_line--;
                     get_line(templine);
                     position = pos("}",templine);
                     if (position == 0) {
                        message("If block of else pair is not guarded with {}");
                        return -1;
                     }
                  }
                  /* Got the position of the paren */
                  p_col = position;
                  status = _find_matching_paren(def_pmatch_max_diff,true);
                  if (status) {
                     message("Unable to find the leading brace for if");
                     return 0;
                  }
                  /* search for that 'if' pair of the 'else' under interest */
                  do {
                     p_line--;
                     get_line(strline);
                     position = pos("if",strline);
                  }while (p_line != -1 && pos("{",strline) == 0 && position == 0);
                  if (position == 0) {
                     message("Unable to find that elusive if");
                     return -1;
                  }
                  prepend_str = "Else of";
               } /* End of else analysis */
               p_col = position;
               if (_clex_find( 0, 'G' ) == CFG_KEYWORD) {

                  /* found the keyword    */
                  strline = strip(strline,'L');
                  if (prepend_str != "") {
                     strline = prepend_str" "strline;
                  }
                  len_str = length(strline);
                  position = MAX_COMMENT_COL - end_brace_col ;

                  if (position > len_str ) {
                     position = len_str;
                     appstr = " ";
                  } else {
                     appstr = "...";
                  }
                  if (position) {
                     commentline = substr(strline,1,position);// Need to see how big the comment can be
                  } else {
                     commentline = strline;
                  }
                  commentline =" /* "commentline" "appstr"*/";
                  // Go to the original cursor position.
                  restore_pos(end_brace_pos);
                  get_line_raw(line);
                  line = strip(line,'T');
                  line = line" "commentline;
                  replace_line(line);
                  return(1);

               }

            }
            i++;
         }  
         /* Search for possible function names */

         p_line--;
         skipped_first_brace = 1;
      }
      word=cur_word(col);
      _message_box("Word is "word);

      if (pos(' 'word' ',' with for foreach if elsif elseif switch while lock ')) {


         commentline = substr(strline,0,12);// Need to see how big the comment can be
         commentline =" /* "commentline" */";
         _message_box("The comment is "commentline);
      }
      return 0;

   } /* else of if(p_line = ......) */

}            

static _str HWIO_ARG_keywords[]={'BMSK','SHFT','RMSK','PHYS'};

/*----------------------------------------------------
     Process HWIO Macro
 *----------------------------------------------------*/
/**
 * @brief Process the text under cursor as a possible HWIO macro
 * 
 * @author mkrishna
 */

_command void QCTools_ProcessHWIOMacro() name_info(','VSARG2_EDITORCTL|VSARG2_READ_ONLY|VSARG2_REQUIRES_MDI_EDITORCTL|VSARG2_REQUIRES_TAGGING|VSARG2_REQUIRES_EDITORCTL)
{
   int i,status,curr_col;
   _str addr_word, word,tag,pretag;
   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return; 
   }
   curr_col = p_col;
   word =  cur_word(0);

   /* Check if we have a HWIO_ADDR with that name */
   //pretag = "HWIO_"word; 
   tag = "HWIO_"word"_ADDR";
   status = find_tag(tag,true);
   if (status == 0) {
      //push_tag(tag);
      return;
   }

   /* Not found..so it could be a shift define .. Get the previous word as well*/
   begin_word();
   prev_word();
   addr_word =  cur_word(0);
   i = 0;
   while (i < HWIO_ARG_keywords._length()) {
      tag = "HWIO_"addr_word"_"word"_"HWIO_ARG_keywords[i];
      status = find_tag(tag,true);
      if (status == 0)
         break;
      i++;
   }
   if (status == 0) {
      //push_tag(tag);
      return;
   }
   message( nls( 'no hwio macro found for 'tag ) ); 
   return;

}

/*----------------------------------------------------
     Process DAL Macro
 *----------------------------------------------------*/
/**
 * @brief Process the text under cursor as a possible DAL SYS macro
 * 
 * @author mkrishna
 */

_command void QCTools_ProcessDALMacro() name_info(','VSARG2_EDITORCTL|VSARG2_READ_ONLY|VSARG2_REQUIRES_MDI_EDITORCTL|VSARG2_REQUIRES_TAGGING|VSARG2_REQUIRES_EDITORCTL)
{
   int i,status,curr_col;
   _str addr_word, word,tag,pretag;
   if ( p_extension!='c' && p_extension != 'h' && p_extension != "cpp") {
      // Check to see if we are in C mode. If not, exit 
      message( nls( 'Command not allowed while not in c mode' ) ); 
      return; 
   }
   curr_col = p_col;
   word =  cur_word(0);

    /** Make Sure it starts with DAL */
   DalPrefix = substr(word,1,3,"");
   if(lowcase(DalPrefix) == "dal")
   {

      /* 1. Check if it is a DALxxx_YYYY */
      tag = substr(word,4,-1,"");
      status = find_tag(tag,true);
      if (status == 0) {
         //push_tag(tag);
         return;
      }
      /* Check if it is a DALSys Fn */
      DalSysPrefix = substr(word,1,6,"");
      if(lowcase(DalSysPrefix) == "dalsys")
      {
         tag = "_"word;
         status = find_tag(tag,true);
         message('after find tag')
         if (status == 0) {
         //   push_tag(tag);
            return;
         }
         
      }

   }


   message( nls( 'no DAL macro found for 'tag ) ); 
   return;

}
_str def_process=1;        // Leave process buffer active when exec commands.



/**
 * gets called when active buffer is switched
 *
 * @param oldbuffname
 *               name of buffer being switched from
 * @param flag   flag = 'Q' if file is being closed
 *               flag = 'W' if focus is being indicated
 *  
 * @author unknown 
 *  
 */ 

void _switchbuf_Sconsctruct(_str oldbuffname, _str flag)
{
if (p_mode_name == 'Plain Text') {
      _str filename = _mdi.p_child.p_buf_name;
      _str name     = strip_filename(filename,'P');
//      message( nls( 'filename is ' :+ name :+" and mode is ":+p_mode_name :+" Lang = " :+ p_LangId ) ); 
      if (lowcase(name) == 'sconstruct' || lowcase(name) ==  'sconscript') {
         select_mode('Python');
      }
   }
}
