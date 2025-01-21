#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

//Batuhan Güzelyurt 31003 PA1

int main(int argc, char *argv[]) {
    int curDepth, maxDepth, lr;
    int num1, num2, result;

    
    //check if the given number of inputs are correct 
    if (argc != 4) {
        fprintf(stderr, "Usage: treePipe <current depth> <max depth> <left-right>\n");
        exit(1);
    }

    //convert to integers
    curDepth = atoi(argv[1]);
    maxDepth = atoi(argv[2]);
    lr = atoi(argv[3]);

    char indent[50];// string for indentation in output for readability
    //if root node
    if (curDepth == 0) {
        strcpy(indent, "> ");
        fprintf(stderr, "%sCurrent depth : %d, lr : %d\n", indent, curDepth, lr);
        fprintf(stderr, "Please enter num1 for the root : ");
        scanf("%d", &num1); // get num1 from user
    } else {
        //non-root nodes
        int indent_len = 3 * curDepth;
        for (int i = 0; i < indent_len; i++) {
            indent[i] = '-';
        }
        indent[indent_len] = '>';
        indent[indent_len + 1] = ' ';
        indent[indent_len + 2] = '\0';

        fprintf(stderr, "%sCurrent depth : %d, lr : %d\n", indent, curDepth, lr);

        // read num1 from stdin
        scanf("%d", &num1);
    }

    if (curDepth == maxDepth) {
        // leaf nodes
        num2 = 1;

        // execute left-right
        //pipes for parent-to-child and child-to-parent
        int p_to_c[2], c_to_p[2];
        if (pipe(p_to_c) == -1 || pipe(c_to_p) == -1) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid = fork();//fork child process
        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // child process

            // close unused ends of pipes
            close(p_to_c[1]); //close write end of parent to child
            close(c_to_p[0]); // close read end of child to parent

            // redirect stdin to read end of paretn to child pipe
            if (dup2(p_to_c[0], STDIN_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            // redirect stdout to write end of child-to parent pipe
            if (dup2(c_to_p[1], STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            // Close file descriptors 
            close(p_to_c[0]);
            close(c_to_p[1]);

            char *args[] = { (lr == 0) ? "./left" : "./right", NULL };
            execvp(args[0], args);

            perror("execvp failed");
            exit(1);
        } else {
            // parent process

            // close unused ends
            close(p_to_c[0]); //close read end of parent to child pipe
            close(c_to_p[1]);//close write end of child to parent pipe

            // write num1 and num2 to child
            dprintf(p_to_c[1], "%d\n%d\n", num1, num2);

            // close write end
            close(p_to_c[1]);

            // wait for child
            waitpid(pid, NULL, 0);

            // read result from c_to_p[0]
            char buffer[11];
            int n = read(c_to_p[0], buffer, 10);
            buffer[n] = '\0';
            result = atoi(buffer);

            // close read end
            close(c_to_p[0]);

            fprintf(stderr, "%sMy num1 is : %d\n", indent, num1);
            fprintf(stderr, "%sMy result is : %d\n", indent, result);

            if (curDepth == 0) {
                printf("The final result is : %d\n", result);
            } else {
                // send result to parent via stdout
                printf("%d\n", result);
            }
            exit(0);
        }
    } else {
        // non-leaf node

        // create left child
        int p_to_c_left[2], c_to_p_left[2];
        if (pipe(p_to_c_left) == -1 || pipe(c_to_p_left) == -1) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid_left = fork(); //fork left child process
        if (pid_left == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid_left == 0) {
            // left child process

            // close unused ends
            close(p_to_c_left[1]); //close write end to parent to child
            close(c_to_p_left[0]);//close read end of child to parent

            // redirect stdin
            if (dup2(p_to_c_left[0], STDIN_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            //Redirect stdout
            if (dup2(c_to_p_left[1], STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            // Close file descriptors
            close(p_to_c_left[0]);
            close(c_to_p_left[1]);

            // build arguments for execvp
            char curDepth_str[10], maxDepth_str[10], lr_str[10];
            sprintf(curDepth_str, "%d", curDepth + 1);
            sprintf(maxDepth_str, "%d", maxDepth);
            sprintf(lr_str, "%d", 0);

            char *args[] = {"./treePipe", curDepth_str, maxDepth_str, lr_str, NULL};
            execvp(args[0], args);

            perror("execvp failed");
            exit(1);
        } else {
            // parent process

            // close unused ends
            close(p_to_c_left[0]); // close read end of paretn to child
            close(c_to_p_left[1]);// close write end of child to paretn pipe

            // write num1 to left child
            dprintf(p_to_c_left[1], "%d\n", num1);

            //close write end
            close(p_to_c_left[1]);

            // wait for left child
            waitpid(pid_left, NULL, 0);

            // read result from c_to_p_left[0]
            char buffer[11];
            int n = read(c_to_p_left[0], buffer, 10);
            buffer[n] = '\0';
            num1 = atoi(buffer);

            //close read end
            close(c_to_p_left[0]);

            fprintf(stderr, "%sMy num1 is : %d\n", indent, num1);
        }

        // create right child
        //create pipes for right child
        int p_to_c_right[2], c_to_p_right[2];
        if (pipe(p_to_c_right) == -1 || pipe(c_to_p_right) == -1) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid_right = fork(); // fork the right child
        if (pid_right == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid_right == 0) {
            // right child process

            // close unused ends
            close(p_to_c_right[1]); // close write end of parent to child pipe
            close(c_to_p_right[0]);//close read end of child to parent pipe

            // rdirect stdin
            if (dup2(p_to_c_right[0], STDIN_FILENO) == -1) {//to read end
                perror("dup2 failed");
                exit(1);
            }

            // redirect stdout
            if (dup2(c_to_p_right[1], STDOUT_FILENO) == -1) {//to write end
                perror("dup2 failed");
                exit(1);
            }

            // close file descriptors
            close(p_to_c_right[0]);
            close(c_to_p_right[1]);

            // build arguments
            char curDepth_str[10], maxDepth_str[10], lr_str[10];
            sprintf(curDepth_str, "%d", curDepth + 1);
            sprintf(maxDepth_str, "%d", maxDepth);
            sprintf(lr_str, "%d", 1);

            char *args[] = {"./treePipe", curDepth_str, maxDepth_str, lr_str, NULL};
            execvp(args[0], args);

            perror("execvp failed");
            exit(1);
        } else {
            // parent process(after forking right child)

            // close unused ends
            close(p_to_c_right[0]); // close read end of parent to child pipe
            close(c_to_p_right[1]);//close write end of child to parent pipe

            // Write num1 (from left child) to right child
            dprintf(p_to_c_right[1], "%d\n", num1);

            // close write end
            close(p_to_c_right[1]);

            // Wait for right child to finish
            waitpid(pid_right, NULL, 0);

            // read result from c_to_p_right[0]
            char buffer[11];
            int n = read(c_to_p_right[0], buffer, 10);
            buffer[n] = '\0';
            num2 = atoi(buffer);

            // close read end
            close(c_to_p_right[0]);

            fprintf(stderr, "%sCurrent depth : %d, lr : %d, my num1 : %d, my num2 : %d\n", indent, curDepth, lr, num1, num2);
        }

        // execute left-right program
        int p_to_c[2], c_to_p[2]; // pipes for communucating with computation
        if (pipe(p_to_c) == -1 || pipe(c_to_p) == -1) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid = fork(); //fork a child process for computation
        if (pid == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // child process

            // close unused ends
            close(p_to_c[1]); // close write end of parent to child pipe
            close(c_to_p[0]); // close read end of child to parent pipe

            // redirect stdin
            if (dup2(p_to_c[0], STDIN_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            // Redirect stdout
            if (dup2(c_to_p[1], STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                exit(1);
            }

            // Close file descriptors
            close(p_to_c[0]);
            close(c_to_p[1]);

            char *args[] = { (lr == 0) ? "./left" : "./right", NULL };
            execvp(args[0], args);

            perror("execvp failed");
            exit(1);
        } else {
            // parent process

            // close unused ends
            close(p_to_c[0]);//close read end of parent to child
            close(c_to_p[1]);//close write end of child to parent

            // Write num1 and num2 to child
            dprintf(p_to_c[1], "%d\n%d\n", num1, num2);

            // close write end
            close(p_to_c[1]);

            // Wait for child To finish
            waitpid(pid, NULL, 0);

            // Read result from c_to_p[0]
            char buffer[11];
            int n = read(c_to_p[0], buffer, 10);
            buffer[n] = '\0';
            result = atoi(buffer);

            // close read end
            close(c_to_p[0]);

            fprintf(stderr, "%sMy result is : %d\n", indent, result);

            if (curDepth == 0) {
                printf("The final result is : %d\n", result);
            } else {
                // send result to parent via stdout
                printf("%d\n", result);
            }
            exit(0);
        }
    }

    return 0;
}
