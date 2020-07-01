//
//  daemon.c
//  bullinServer
//
//  Created by Ernie on 2020-06-28.
//  Copyright © 2020 Ernie. All rights reserved.


// run the process directly in the back ground http://fibrevillage.com/sysadmin/253-how-to-put-running-process-to-background-on-linux

// code template: https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
// https://github.com/pasce/daemon-skeleton-linux-c
// for debugging: http://www.enderunix.org/docs/eng/daemon.php log into a file, since it's a background process we can not see the log, we need to log it into a file

#include "daemon.h"

/*
 * daemonize.c
 * This example daemonizes a process, writes a few log messages,
 * sleeps 20 seconds and terminates afterwards.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

/*  for calling it in the main function
 
 int main()
 {
     skeleton_daemon();

     while (1)
     {
         //TODO: Insert daemon code here.
         syslog (LOG_NOTICE, "First daemon started.");
         sleep (20);
         break;
     }

     syslog (LOG_NOTICE, "First daemon terminated.");
     closelog();

     return EXIT_SUCCESS;
 }

 */
