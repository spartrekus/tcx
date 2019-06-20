
//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////
#include <stdio.h>
#define PATH_MAX 2500
#if defined(__linux__) //linux
#define MYOS 1
#elif defined(_WIN32)
#define MYOS 2
#elif defined(_WIN64)
#define MYOS 3
#elif defined(__unix__) 
#define MYOS 4  // freebsd
#define PATH_MAX 2500
#else
#define MYOS 0
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>  
#include <time.h>



char *fbasename(char *name)
{
  char *base = name;
  while (*name)
    {
      if (*name++ == '/')
	{
	  base = name;
	}
    }
  return (base);
}







#define ESC "\033"
#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //clear the screen, move to (1,1)
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);
#define ansigotoyx(y,x)		printf(ESC "[%d;%dH", y, x);



int rows, cols ; 
int pansel = 1;
char pathbefore[PATH_MAX];

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"




void nsystem( char *mycmd )
{
   printf( "<SYSTEM>\n" );
   printf( " >> CMD:%s\n", mycmd );
   system( mycmd );
   printf( "</SYSTEM>\n");
}





char *strtimestampalt()
{
      long t;
      struct tm *ltime;
      time(&t);
      ltime=localtime(&t);
      char charo[50];  int fooi ; 
      fooi = snprintf( charo, 50 , "%04d%02d%02d%02d%02d%02d",
	1900 + ltime->tm_year, ltime->tm_mon +1 , ltime->tm_mday, 
	ltime->tm_hour, ltime->tm_min, ltime->tm_sec 
	);
    size_t siz = sizeof charo ; 
    char *r = malloc( sizeof charo );
    return r ? memcpy(r, charo, siz ) : NULL;
}










void nrunwith( char *cmdapp, char *filesource )
{
           char cmdi[PATH_MAX];
           strncpy( cmdi , "  " , PATH_MAX );
           strncat( cmdi , cmdapp , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " " , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " \"" , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi ,  filesource , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , "\" " , PATH_MAX - strlen( cmdi ) -1 );
           nsystem( cmdi ); 
}


void clear_screen_retro()
{
    int fooi;
    struct winsize w; // need ioctl and unistd 
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
    for ( fooi = 1 ; fooi <= w.ws_row ; fooi++ ) 
       printf( "\n" );
    home();
}


void clear_screen()
{
    int fooi;
    struct winsize w; // need ioctl and unistd 
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
    clrscr();
    home();
}


int fexist(char *a_option)
{
  char dir1[PATH_MAX]; 
  char *dir2;
  DIR *dip;
  strncpy( dir1 , "",  PATH_MAX  );
  strncpy( dir1 , a_option,  PATH_MAX  );

  struct stat st_buf; 
  int status; 
  int fileordir = 0 ; 

  status = stat ( dir1 , &st_buf);
  if (status != 0) {
    fileordir = 0;
  }

  // this is compatible to check if a file exists
  FILE *fp2check = fopen( dir1  ,"r");
  if( fp2check ) {
  // exists
  fileordir = 1; 
  fclose(fp2check);
  } 

  if (S_ISDIR (st_buf.st_mode)) {
    fileordir = 2; 
  }
return fileordir;
/////////////////////////////
}




///////////////////////////////////////////
void readfile( char *filesource )
{
   FILE *source; 
   int ch ; 
   source = fopen( filesource , "r");
   if ( source == NULL ) { printf( "File not found.\n" ); } else {
   while( ( ch = fgetc(source) ) != EOF )
   {
         printf( "%c", ch );
   }
   fclose(source);
   }
}




static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void enable_waiting_for_enter(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}





/////////////////////////
/////////////////////////
int  nexp_user_sel[5] ; 
int  nexp_user_scrolly[5] ;
char nexp_user_fileselection[PATH_MAX]; 
int  tc_det_dir_type = 0;
/////////////////////////
void printdir( int pyy, int fopxx, char *mydir , int panviewpr )
{
   int pxx = fopxx;
   if ( pxx == 0 )       pxx = 2;
   DIR *dirp; int posy = 0;  int posx, chr ; 
   int fooselection = 0;
   posy = 1; posx = cols/2;
   char cwd[PATH_MAX];
   struct dirent *dp;
   dirp = opendir( mydir  );
   int entrycounter = 0;
   fooselection = 0;
   while  ((dp = readdir( dirp )) != NULL ) 
   if ( posy <= rows-3 )
   {
        //printf("%s\n", KYEL);
        entrycounter++;
        if ( entrycounter <= nexp_user_scrolly[panviewpr] )
              continue;

        if (  dp->d_name[0] !=  '.' ) 
        if (  strcmp( dp->d_name, "." ) != 0 )
        if (  strcmp( dp->d_name, ".." ) != 0 )
        {
            posy++;  fooselection++;
            if ( dp->d_type == DT_DIR ) 
            {
                 //color_set( 3 , NULL );
                 //mvaddch( posy, posx++ , '/' );
                 //printf( "/" );
                 ansigotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
            }
            else if ( dp->d_type == 0 )
            {
               if ( tc_det_dir_type == 1 )
               if ( fexist( dp->d_name ) == 2 )
               {
                 //color_set( 3 , NULL );
                 //mvaddch( posy, posx++ , '/' );
                 //printf( "/" );
                 ansigotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
               }
            }

            ansigotoyx( posy, pxx-1 );
            if ( nexp_user_sel[ panviewpr ] == fooselection ) 
            {
                  if ( panviewpr == pansel )
                  {
                    strncpy( nexp_user_fileselection, dp->d_name , PATH_MAX );
                    //color_set( 4, NULL );
                    printf( ">" );
                  }
            }
            else 
                  printf( " " );

            ansigotoyx( posy, pxx );
            for ( chr = 0 ;  chr <= strlen(dp->d_name) ; chr++) 
            {
              if  ( dp->d_name[chr] == '\n' )
              {    //posx = cols/2;
              }
              else if  ( dp->d_name[chr] == '\0' )
              {    //posx = cols/2;
              }
              else
              {  
                 //mvaddch( posy, posx++ , dp->d_name[chr] );
                 printf( "%c", dp->d_name[chr] );
                 posx++;
              }
            }
        }
   }
   closedir( dirp );
   // color_set( 0, NULL ); attroff( A_REVERSE );
   //mvprintw( rows-1, cols/2, "[FILE: %s]", nexp_user_fileselection );
}

////////////////////////////////////////////////////////////////////
char *strcut( char *str , int myposstart, int myposend )
{     // copyleft, C function made by Spartrekus 
      char ptr[strlen(str)+1];
      int i,j=0;
      for(i=0; str[i]!='\0'; i++)
      {
        if ( ( str[i] != '\0' ) && ( str[i] != '\0') )
         if ( ( i >=  myposstart-1 ) && (  i <= myposend-1 ) )
           ptr[j++]=str[i];
      } 
      ptr[j]='\0';
      size_t siz = sizeof ptr ; 
      char *r = malloc( sizeof ptr );
      return r ? memcpy(r, ptr, siz ) : NULL;
}

char *strtimestamp()
{
      long t;
      struct tm *ltime;
      time(&t);
      ltime=localtime(&t);
      char charo[50];  int fooi ; 
      fooi = snprintf( charo, 50 , "%04d%02d%02d%02d%02d%02d",
	1900 + ltime->tm_year, ltime->tm_mon +1 , ltime->tm_mday, 
	ltime->tm_hour, ltime->tm_min, ltime->tm_sec 
	);
    size_t siz = sizeof charo ; 
    char *r = malloc( sizeof charo );
    return r ? memcpy(r, charo, siz ) : NULL;
}





char thefilename[PATH_MAX];
////////////////////////////////
void nlaunchscrot( ) 
{
       char cmdi[PATH_MAX];
       char cwdpath[PATH_MAX];
       FILE *fp;
       char figref[PATH_MAX];
       char lastfile[PATH_MAX];

       strncpy( figref, strtimestamp(), PATH_MAX );
       printf( " Entering sleep 3... before shot!\n" );
       system( " sleep 1 " );

       printf( "Cmd: scrot! \n" );

       strncpy( lastfile, "" , PATH_MAX );
       strncat( lastfile , figref , PATH_MAX - strlen( lastfile ) -1 );
       strncat( lastfile , "-" , PATH_MAX - strlen( lastfile ) -1 );
       strncat( lastfile , thefilename , PATH_MAX - strlen( lastfile ) -1 );
       strncat( lastfile , ".png" , PATH_MAX - strlen( lastfile ) -1 );

       strncpy( cmdi , "  export DISPLAY=:0 ; scrot -s  " , PATH_MAX );
       strncat( cmdi , " \"" , PATH_MAX - strlen( cmdi ) -1 );
       strncat( cmdi , lastfile , PATH_MAX - strlen( cmdi ) -1 );
       strncat( cmdi , "\" " , PATH_MAX - strlen( cmdi ) -1 );
       strncat( cmdi , " " , PATH_MAX - strlen( cmdi ) -1 );

       printf( "CMD:%s\n", cmdi );
       system( cmdi );

       printf( "cmd: %s\n", cmdi );

       chdir( getenv( "HOME" ) );
       //printf( "PATH: %s\n", getcwd( cwdpath, PATH_MAX), PATH_MAX );
       fp = fopen( ".clipboard", "wb+" );
       fputs( "!fig{"    , fp  ); 
              fputs( fbasename(  lastfile  ) , fp  ); 
              fputs( "}"        , fp  ); 
              fputs( "{}"        , fp  ); 
              fputs( "{fig:"        , fp  ); 
              fputs( figref   , fp  ); 
              fputs( "}"        , fp  ); 
              fputs( "\n" , fp  ); 
              fclose( fp );

       chdir( pathbefore );
       printf( "PATH: %s\n",  pathbefore );
}



char userstr[PATH_MAX];
/////////////////////////////////////
void strninput( char *mytitle, char *foostr )
{
      strncpy( userstr , "" , PATH_MAX );
      disable_waiting_for_enter();
      char strmsg[PATH_MAX];
      char charo[PATH_MAX];
      strncpy( strmsg, foostr , PATH_MAX );

      int foousergam = 0; int ch ;  int chr;

      while( foousergam == 0 ) 
      {

         home();
         for ( chr = 0 ;  chr <= cols-1 ; chr++) printf( " ");

         home();
         printf( "|Title: %s|", mytitle );

         ansigotoyx( rows, 0 );
         for ( chr = 0 ;  chr <= cols-1 ; chr++) printf( " ");
         ansigotoyx( rows, 0 );
         printf( ": %s", strmsg );

         ch = getchar();
         if ( ch == 10 )       foousergam = 1;

	 else if ( ch == 27 ) 
         {
              foousergam = 1;
	      strncpy( strmsg, ""  ,  PATH_MAX );
         }

	 else if ( ch == 2 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 4 ) 
           strncat( strmsg , strtimestamp() , PATH_MAX - strlen( strmsg ) -1 );

         else if ( ch == 127 ) 
            strncpy( strmsg, strcut( strmsg, 1 , strlen( strmsg ) -1 )  ,  PATH_MAX );

         else if ( ch == 8 ) 
            strncpy( strmsg, strcut( strmsg, 1 , strlen( strmsg ) -1 )  ,  PATH_MAX );

	 else if (
			(( ch >= 'a' ) && ( ch <= 'z' ) ) 
		        || (( ch >= 'A' ) && ( ch <= 'Z' ) ) 
		        || (( ch >= '1' ) && ( ch <= '9' ) ) 
		        || (( ch == '0' ) ) 
		        || (( ch == '~' ) ) 
		        || (( ch == '!' ) ) 
		        || (( ch == '&' ) ) 
		        || (( ch == '=' ) ) 
		        || (( ch == ':' ) ) 
		        || (( ch == ';' ) ) 
		        || (( ch == '<' ) ) 
		        || (( ch == '>' ) ) 
		        || (( ch == ' ' ) ) 
		        || (( ch == '|' ) ) 
		        || (( ch == '#' ) ) 
		        || (( ch == '?' ) ) 
		        || (( ch == '+' ) ) 
		        || (( ch == '/' ) ) 
		        || (( ch == '\\' ) ) 
		        || (( ch == '.' ) ) 
		        || (( ch == '$' ) ) 
		        || (( ch == '%' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == ',' ) ) 
		        || (( ch == '{' ) ) 
		        || (( ch == '}' ) ) 
		        || (( ch == '(' ) ) 
		        || (( ch == ')' ) ) 
		        || (( ch == ']' ) ) 
		        || (( ch == '[' ) ) 
		        || (( ch == '*' ) ) 
		        || (( ch == '"' ) ) 
		        || (( ch == '@' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == '_' ) ) 
		        || (( ch == '^' ) ) 
		        || (( ch == '\'' ) ) 
	             ) 
		  {
                        snprintf( charo, PATH_MAX , "%s%c",  strmsg, ch );
		        strncpy( strmsg,  charo ,  PATH_MAX );
		  }
     }
     strncpy( userstr, strmsg , PATH_MAX );
}







int main( int argc, char *argv[])
{

    strncpy(  thefilename, "", PATH_MAX );

    ////////////////////////////////////////////////////////
    char cwd[PATH_MAX];
    char stringmyinput[PATH_MAX];
    //char pathbefore[PATH_MAX];
    char pathpile[PATH_MAX];
    strncpy( pathbefore , getcwd( cwd, PATH_MAX ) , PATH_MAX );
    strncpy( pathpile , getcwd( cwd, PATH_MAX ) , PATH_MAX );
    char pathpan[5][PATH_MAX];
    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "" ) !=  0 )
    if ( fexist( argv[1] ) ==  2 )
    {
          chdir( argv[ 1 ] );
          strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    }



    ////////////////////////////////////////////////////////
    int viewpan[5];
    nexp_user_sel[1] = 1;
    nexp_user_sel[2] = 1;
    nexp_user_scrolly[1] = 0;
    nexp_user_scrolly[2] = 0;
    strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
    viewpan[ 1 ] = 1;
    viewpan[ 2 ] = 1;

    struct winsize w; // need ioctl and unistd 
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
    char string[PATH_MAX];
    ////////////////////////////////////////////////////////
    if ( argc == 2)
    if ( strcmp( argv[1] , "-s" ) ==  0 ) 
    {
       printf("Screen\n" );
       printf("Env HOME:  %s\n", getenv( "HOME" ));
       printf("Env PATH:  %s\n", getcwd( string, PATH_MAX ) );
       printf("Env TERM ROW:  %d\n", w.ws_row );
       printf("Env TERM COL:  %d\n", w.ws_col );
       return 0;
    }
    rows = w.ws_row ; 
    cols = w.ws_col ; 



    ////////////////////////////////////////////////////////
    if ( argc == 3)
    if ( strcmp( argv[1] , "-f" ) ==  0 ) 
    {
       printf("%syellow\n", KYEL);
       readfile( argv[ 2 ] );
       return 0;
    }
   
    ////////////////////////////////////////////////////////
    if ( argc == 2)
    {
       printf("%syellow\n", KYEL);
       printf( "PATH1:%s\n",  pathpan[ 1 ] );
       printf( "PATH2:%s\n", pathpan[ 2 ] );
    }
    int ch ; 
    int chr; 
    int gameover = 0;
    char charo[PATH_MAX];
    char strmsg[PATH_MAX];
    strncpy( strmsg, "", PATH_MAX );


    while ( gameover == 0 ) 
    {
        disable_waiting_for_enter();
        ch = getchar();
        printf( "%d\n" , ch );
        
        if ( ch == '[' )  
        {
           ch = getchar();
           if ( ch ==      66 )       nsystem( "   export DISPLAY=:0 ; xdotool  key Down " );
           else if ( ch == 68 )       nsystem( "  export DISPLAY=:0 ;  xdotool  key Left " );
           else if ( ch == 67 )       nsystem( "  export DISPLAY=:0 ;  xdotool  key Right " );
           else if ( ch == 65 )       nsystem( "  export DISPLAY=:0 ;  xdotool  key Up " );
        }


        else if ( ch == 23 )  //ctrl+w
        {
            //printf(  "  Write (take) screenshot... (scrot -s)\n" );
            //nlaunchscrot( );
            //printf( "\n");
            //printf( " => Entered (%s)\n", thefilename );
            //printf( "\n");
            printf(  "  Close app on display 0... (wmctrl)\n" );
            nsystem( "  export DISPLAY=:0 ; wmctrl -r :ACTIVE: -b toggle,maximized_vert,maximized_horz   " );
            ch = 0; 
        }

        else if ( ch == 20 )  //c+t
        {
              printf( "\n");
              strncpy( stringmyinput, thefilename, PATH_MAX );
              strninput( "", stringmyinput );
              printf( " => Passed (%s)\n", userstr );
              printf( "\n");
              strncpy( thefilename, userstr , PATH_MAX );
              printf( " => Entered (%s)\n", thefilename );
              printf( "\n");
              ch = 0; 
        }

        else if ( ch == 20 )  //c+t
        {
              printf( "\n");
              strninput( "", "" );
              printf( "\n");
              strncpy( thefilename, userstr , PATH_MAX );
              printf( " => Entered (%s)\n", thefilename );
              printf( "\n");
              ch = 0; 
        }

        else if ( ch == 20 )  //c+t
        {
           printf( "======\n" );
           printf( "=MENU=\n" );
           printf( "======\n" );
           printf( " 1: scrot title (%s)\n", thefilename );
           printf( " 2: scrot now!\n" );
           ch = getchar();
           if ( ch == '1' )
           {
              printf( "\n");
              strninput( "", "" );
              printf( "\n");
              strncpy( thefilename, userstr , PATH_MAX );
              printf( " => Entered (%s)\n", thefilename );
              printf( "\n");
              ch = 0; 
           }
           else if ( ch == '2' )
           {
              printf(  "  Write (take) screenshot... (scrot -s)\n" );
              //nsystem( "  export DISPLAY=:0 ; nconfig scrot " );
              nlaunchscrot( );
              printf( "\n");
              printf( " => Entered (%s)\n", thefilename );
              printf( "\n");
              ch = 0; 
           }
           ch = 0;
        }

        //else if ( ch == 20 )  //ctrl+t
        //    nsystem( "  export DISPLAY=:0 ; xdotool key ctrl+t " );

        else if ( ch == '/' )  
            nsystem( "  export DISPLAY=:0 ; xdotool key KP_Divide " );

        else if ( ch == 7 )  
            nsystem( "  export DISPLAY=:0 ; wmctrl -r :ACTIVE: -b toggle,maximized_vert,maximized_horz   " );

        else if ( ch == 24 )  //ctrl+x
            nsystem( "  export DISPLAY=:0 ; screen -d -m xterm -bg black -fg yellow " );

        else if ( ch == 2 )  //ctrl+b
            nsystem( "  export DISPLAY=:0 ; xdotool key ctrl+b " );

        else if ( ch == 15 )  //ctrl+o
            nsystem( "  export DISPLAY=:0 ; xdotool key ctrl+o " );

        else if ( ch == 4 )  
            nsystem( "  export DISPLAY=:0 ; xdotool key Next  " );

        else if ( ch == 21 )  
            nsystem( "  export DISPLAY=:0 ; xdotool key Prior " );


        else if ( ch == 9 )  
            nsystem( "  export DISPLAY=:0 ; xdotool key Tab " );

        else if ( ch == 39 )  
            nsystem( "  export DISPLAY=:0 ; xdotool key apostrophe " );

        else if ( ch == ':' )  
            nsystem( "  export DISPLAY=:0 ; xdotool key colon " );

        else if ( ch == 18 ) //ctrl+r for links 
            nsystem( "  export DISPLAY=:0 ; xdotool key ctrl+r " );

        else if ( ch == 27  ) nsystem( "  export DISPLAY=:0 ; xdotool  key escape ; xdotool key Escape " );

        else if ( ch == 10 )  nsystem( "  export DISPLAY=:0 ; xdotool  key Return " );

        else if ( ch == 32 )  nsystem( "  export DISPLAY=:0 ; xdotool  key space " );

        else if ( ch == 8 )            nsystem( "  export DISPLAY=:0 ; xdotool  key BackSpace " );

        else if ( ch == '+' )          nsystem( "  export DISPLAY=:0 ; xdotool  key plus " );
        else if ( ch == '-' )          nsystem( "  export DISPLAY=:0 ; xdotool  key minus " );

        else if ( ch == '@' )          nsystem( "  export DISPLAY=:0 ; xdotool  key at " );
        else if ( ch == '*' )          nsystem( "  export DISPLAY=:0 ; xdotool  key asterisk " );
        else if ( ch == ',' )          nsystem( "  export DISPLAY=:0 ; xdotool  key comma " );

        else if ( ch == '<' ) 
                   nsystem( "  export DISPLAY=:0 ;   xdotool key less " ); 
        else if ( ch == '>' ) 
                  nsystem( "  export DISPLAY=:0 ;   xdotool key greater " ); 


               else if ( ch == '.' ) 
                   nsystem( "  export DISPLAY=:0 ;    xdotool key period " ); 

               else if ( ch == '&' ) 
                   nsystem( "  export DISPLAY=:0 ;    xdotool key ampersand " ); 

               else if ( ch == '#' ) 
                   nsystem( "  export DISPLAY=:0 ;    xdotool key numbersign " ); 

        else if (
			(( ch >= 'a' ) && ( ch <= 'z' ) ) 
		        || (( ch >= 'A' ) && ( ch <= 'Z' ) ) 
		        || (( ch >= '1' ) && ( ch <= '9' ) ) 
		        || (( ch == '0' ) ) 
		        || (( ch == '~' ) ) 
		        || (( ch == '!' ) ) 
		        || (( ch == '&' ) ) 
		        || (( ch == '=' ) ) 
		        || (( ch == ':' ) ) 
		        || (( ch == ';' ) ) 
		        || (( ch == '<' ) ) 
		        || (( ch == '>' ) ) 
		        || (( ch == ' ' ) ) 
		        || (( ch == '|' ) ) 
		        || (( ch == '#' ) ) 
		        || (( ch == '?' ) ) 
		        || (( ch == '+' ) ) 
		        || (( ch == '/' ) ) 
		        || (( ch == '\\' ) ) 
		        || (( ch == '.' ) ) 
		        || (( ch == '$' ) ) 
		        || (( ch == '%' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == ',' ) ) 
		        || (( ch == '{' ) ) 
		        || (( ch == '}' ) ) 
		        || (( ch == '(' ) ) 
		        || (( ch == ')' ) ) 
		        || (( ch == ']' ) ) 
		        || (( ch == '[' ) ) 
		        || (( ch == '*' ) ) 
		        || (( ch == '"' ) ) 
		        || (( ch == '@' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == '_' ) ) 
		        || (( ch == '^' ) ) 
		        || (( ch == '\'' ) ) 
	             ) 
		  {
		        strncpy( charo,  ""   ,  PATH_MAX );
		        strncpy( strmsg,  "  export DISPLAY=:0 ; xdotool  key "  ,  PATH_MAX );
                        snprintf( charo, PATH_MAX , "%s%c",  strmsg, ch );
                        nsystem( charo );
		  }
    }

    enable_waiting_for_enter();
    printf( "\n" );
    printf( "Bye!\n" );
    return 0;
}



