#include<string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

extern char * conf_file;
extern uint8_t make_daemon;
void usage (char ** argv) {

  printf ("Usage:\n");
  printf ("%s [-f conf] [-d 0|1]\n", argv[0]);
  printf ("-f file: configuration file\n");
  printf ("-d 0|1: daemonize the proxy or not\n");
  exit (1);
}

void parseCommandLine (int argc, char ** argv){
  extern char *optarg;
  extern int optind;
  int op;

  while((op = getopt(argc, argv,"f:d:")) != -1) {
    switch (op) {
    case 'f':
      conf_file = (char *) malloc (1024 *sizeof (char));
      strcpy (conf_file, optarg);
      break;
    case 'd':
      make_daemon = atoi (optarg);
      break;
    default:
      usage (argv);
    }
  }
}
