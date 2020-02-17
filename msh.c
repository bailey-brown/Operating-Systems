/*
Name: Bailey Brown
ID: 1001555076
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports 10 arguments

int main()
{

	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	pid_t pids[15];
	char history[15][MAX_COMMAND_SIZE];
	
	//We need a way to keep track of the current position we are in the history and pids array
	//We also need a variable to tell if we are over 15 commands yet 
	int history_count, hc, pid_count, pc = 0;

	while( 1 ) {
		// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

		// Parse input 
		char *token[MAX_NUM_ARGUMENTS];
		int   token_count = 0;                                 
				                                       
		// Pointer to point to the token
		// parsed by strsep
		char *arg_ptr;                                         
		
		//Save a copy of the command line since strsep
		// will end up moving the pointer head		                           
		char *working_str  = strdup( cmd_str );               

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;

		// Tokenize the input stringswith whitespace used as the delimiter
		while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
			  (token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 ) {
				token[token_count] = NULL;
			}

			token_count++;
		}	

		//If the user enters a blank line, we want to do nothing except start the loop over
		if(token[0] == 0) {
				continue;
		}
		
		//If the user anything besides a blank line, we want to store the command in the history array for further use
		cmd_str[strlen(cmd_str)-1] = '\0';
		strcpy(history[history_count], cmd_str);	
		history_count++;
		
		//The history array only stores the last 15 command and new commands are stored at the beginning of the array once we've reached that point
		//Setting hc to 1 tells us we have stored more than 15 commands
		if(history_count > 14) {
			history_count = 0;
			hc = 1;
		}

		//Entering !n, where n is a number between 1 and 15 will result in the shell re-running the nth command.
		//If n is not 0-14, a message will be displayed to the user and the loop will start over
		if(token[0][0] == '!') {
			char num[2] = {token[0][1], token[0][2]};
			int n = (atoi)(num); 

			if(n > history_count || n < 0) {
				printf("Command not in history\n");
				continue;
			}

			working_str  = strdup( history[n] );
			token_count = 0;
			while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
			  (token_count<MAX_NUM_ARGUMENTS))
				{
					token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
					if( strlen( token[token_count] ) == 0 ) {
						token[token_count] = NULL;
					}

					token_count++;
				}
		}

			

		//If the user enters "quit" or "exit" we will completely exit from the shell
		if(strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0) {
			free( working_root );
			exit(0);
		}

		//If the user enters "history" we will traverse through the history array and display up to the last 15 commands
		else if(strcmp(token[0], "history") == 0) {
			int i, n;
			if(hc) n = 15;
			else n = history_count;
			for(i = 0; i < n; i++) 
				printf("%d: %s\n", i, history[i]);
		}

		//"showpids" command will traverse through the pids array and display the pid for up to the last 15 processes that have spawned
		else if(strcmp(token[0], "showpids") == 0) {
			int i, n;
			if(pc) n = 15;
			else n = pid_count;
			for(i = 0; i < n; i++)
				printf("%d: %ld\n", i, (long)pids[i]);
		}

		//"cd" command will change the cwd to the directory entered after the command
		else if(strcmp(token[0], "cd") == 0) {
			strcat(history[history_count], token[1]);
			chdir(token[1]);
		}

		else {
			//All other commands will be executed by forking and using exec functions
			pid_t pid = fork( );

			// Everytime a new process is spawned we want to store the command in the pids array for further use
			pids[pid_count] = pid;
			pid_count++;

			//The pids array only stores the last 15 pids and new pids are stored at the beginning of the array once we've reached that point
			//Setting pc to 1 tells us we have stored more than 15 pids
			if(pid_count > 14) {
				pid_count = 0;
				pc = 1;
			}

			//If the pid is equal to 0, we are in the new(child) process
			if( pid == 0 ) {

				// Notice you can add as many NULLs on the end as you want
				int ret = execvp( token[0], token );  

				//If ret is -1 the command the user has entered is not supported so we want to display a message to the user and start the loop over
				if( ret == -1 ) {
					printf("%s : Command not found.\n", token[0]);
					exit(0);
				}
			}

			//If the pid is not 0, we are in the old(parent) process and need to wait
			else {
				int status;
				wait( & status );
			}
		}
	}

  return 0;
}


