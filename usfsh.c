
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF_LEN 128
#define MAX_ARGS 10

 
void print_prompt(void) {

	int rv;
	// setting it to rv you can check output values
	rv = write(1, "$ ", 2);        
	if (rv < 0) {
		printf("write() failed. \n");
		exit(-1);
	}
}


void read_command_line(char *buf) {

	int rv;
	// -1 to leave room for the null terminating character
	rv = read(0, buf, (MAX_BUF_LEN-1));      
	rv--;
	
	if (rv < 0) {
		printf("read( ) failed. \n");
		exit(-1);
	}
	// put null terminating character at rv because rv will give us the number of characters given and can be used as index for next open spot in buf
	buf[rv] = '\0';           								

}

int parse_command_line(char *buf, char** args) {

	int i = 0;
	args[i] = strtok(buf, " ");
	i++;

	while ((args[i] = strtok(NULL, " ")) != NULL) {
		i++;
	}
	
	return i;
}

void pipe_redirection(char** args, int fd_out) {

	pid_t id;
	id = fork();

	if(id == 0) {
		// we are in the child
		// close the stdout in child;
		close(1); 
		// replace stdout child w/ 'out'
		dup(fd_out);
		// close 'out' of pipe
		close(fd_out);
		if (execvp(args[0], args) < 0) {
			
            printf("Invalid command: %s\n", args[0]);
            exit(0);
        }
	}
	else {
		// we are in the parent
		close(fd_out);
		id = wait(NULL);
	}
}

void pipe_command(char** args, char** arg_store) {

	pid_t id_one;
	pid_t id_two;
	int counter;
	int fildes[2];

	pipe(fildes);

	if((id_one = fork()) == 0) {
		// we are in the child
		// close the write of child
		close(1);
		// putting the write end of the pipe into stdout index
		dup(fildes[1]);
		close(fildes[0]);
		if (execvp(args[0], args) < 0) {
			printf("execvp failed...\n");
            exit(1);
		}
	} 
	else {
		
		id_two = fork();
		if (id_two == 0) {
			// we are in the second child
			// need to close the stdin for 2nd child
			close(0);
			// put the read end of the pipe into stdin index
			dup(fildes[0]);
			// close stdout(write) of pipe
			close(fildes[1]);
			if (execvp(arg_store[0], arg_store) < 0) {
				
				printf("Invalid command: %s\n", arg_store[0]);
                exit(0);
			}
		}
		else {
			// we are in the parent
			// close the read and write of the pipe
			close(fildes[0]);
			close(fildes[1]);
			// wait for child
			id_one = wait(NULL);
			id_two = wait(NULL);
		}
	}
}

void execute_program(char** args) {

	pid_t id;
	id = fork();

	if (id == 0) {
		// we are in the child
		if (execvp(args[0], args) < 0) {
			
            printf("Invalid command: %s\n", args[0]);
            exit(0);
        }
	}
	else {
		// we are in the parent
		id = wait(NULL);
	}
}

void execute_command(char** args, int num_args) {

	char* arg_store[MAX_ARGS];
	int i;
	int k = 0;
	
	//printf("%s\n", args[0]);


	if (strcmp(args[0], "exit") == 0) {
		printf("Exit shell\n");
		exit(-1);
	}
	else if (strcmp(args[0], "cd") == 0) {
		
		if (args[1] == NULL) {
			chdir("/home/pi");
		} else {
			chdir(args[1]);
		}

	} 
	else {
		
		for (i = 0; i < num_args; i++) {

			if (strcmp(args[i], "|") == 0) {
				args[i] = NULL;
				i++;
				while (i < num_args) {
					arg_store[k] = args[i];
					k++;
					i++;
				}
				arg_store[k] = NULL;
				pipe_command(args, arg_store);
				return;
			}

			else if (strcmp(args[i], ">") == 0) {

				int fd_out = open(args[i + 1], O_CREAT | O_WRONLY, 0644);
				if (fd_out < 0) {
					printf("cannot open file: %s\n", args[i + 1]);
                    return;
				}
				args[i] = NULL;
				pipe_redirection(args, fd_out);
				return;
			}
		}
		execute_program(args);
	}
}


void clear(char *buf, char* args[]) {
    memset(buf, 0, MAX_BUF_LEN);
    memset(args, 0, MAX_ARGS);
}
	


int main (int argc, char **argv) {
	
	int num_args;
	char buf[MAX_BUF_LEN];
	char *args[MAX_ARGS];

	while (true) {

		// need to clear the memory of the buffer and the parsed args before each prompt
		clear(buf, args);

		// allow user to write input
		print_prompt();
		//printf("here");

		// read in the input
		read_command_line(buf);
		//printf("here2");

		//printf("buf = %s\n", buf);

		num_args = parse_command_line(buf, args);

		execute_command(args, num_args);

	}

	return 0;

}