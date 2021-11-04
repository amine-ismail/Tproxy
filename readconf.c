#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include "log.h"
#include "readconf.h"
#include "compat.h"

#define EXIT_FAILURE 1


typedef struct {
  char * key;
  char ** values;
  uint16_t nbVal;
} confLine;
 
static confLine * conf; 
static uint16_t nbConfLine=0;

/* ======================================== */

static int scanLine (char * line) {

  char * tmp=NULL;
  char * key=NULL;
  char ** values=NULL;
  char *ptr_tmp;
  char * ptr;
  int i=0;

  ptr_tmp = line;
  while (*ptr_tmp != '\0') { 
    ptr = (char *) strchrnul (ptr_tmp, ' ');
    if (ptr) {
      tmp = (char *) malloc ((strlen(ptr_tmp)-strlen(ptr) + 1)*sizeof (char));
      memcpy (tmp, ptr_tmp, strlen(ptr_tmp)-strlen(ptr));
      tmp[strlen(ptr_tmp)-strlen(ptr)]=0;
    
      switch (i) {
      case 0:
	key = tmp;
	break;
      default: 
	values = (char **) realloc (values, i * sizeof (char **));
	if (!values) {
	  //logError ("can't allocate value\n");
	  exit (EXIT_FAILURE);
	}
	values[i-1]=tmp;
	break;
      }
      i++;
    }
    while (*ptr == ' ' || *ptr == '\t') {
      ptr++;
    }
    ptr_tmp = ptr;
  }

  nbConfLine++;
  conf = realloc (conf, nbConfLine * sizeof (confLine));
  if (!conf) {
    //logError ("can't allocate conf\n");
    exit (EXIT_FAILURE);
  }
  (conf[nbConfLine - 1]).key = key;
  conf[nbConfLine - 1].values = values;
  conf[nbConfLine - 1].nbVal = i-1;

  return 0;
}

/* ======================================== */

int destroyConf () {

  int i,j;
  for (i=0; i < nbConfLine; i++) {
    free (conf[i].key);
    conf[i].key=NULL;
    for (j=0; j < conf[i].nbVal; j++) {
      free((conf[i].values)[j]);
      (conf[i].values)[j]=NULL;
    }
    free (conf[i].values);
    conf[i].values=NULL;
    conf[i].nbVal=0;
  }
  free(conf);
  conf=NULL;
  nbConfLine=0;
  return 0;
}



/* ======================================== */

int loadConf (const char * file) {
  char * line=NULL;
  int n=0, i;
  FILE *f;

  f = fopen(file, "r");
  if (f == NULL) {
    logWarning("ERROR: can not open configuration file %s: %s\n", 
	       file, strerror(errno));
    return (-1);
  }
  while (getline (&line, (size_t*) &n, f)>0) {
    i=0;
    while (line[i] == ' ' || line[i]=='\t') {
      i++;
    }
    if (line[i] == '#' || line[i] == '\n') {
      continue;
    }
    if (line[strlen(line)-1]=='\n'){
      line[strlen(line)-1]=0;
    }
    /* process the line here */
    scanLine (&(line[i]));
    /* here we should free the */
    free(line);
    line=NULL;
    n=0;
  }
  fclose (f);
  return 0;
}

/* ======================================== */

static char * getVal (char * key, uint16_t pos) {
  int i;
  for (i=0; i<nbConfLine; i++) {
    if (strcmp (key,conf[i].key) == 0) {
      if (pos < conf[i].nbVal) {
	return (conf[i].values)[pos];
      }
    }
  }
  return NULL;
}

/* ======================================== */

int read_int_pos (char * pname, int * val, uint16_t pos) {
  
  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n", pname);
    return -1;
  }
  *val = strtol (tmp_val, &err, 10);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting integer value\n", pname);
    return -1;
  }
  return 0;
}

/* ======================================== */

int read_int (char * pname, int * val) {
  return read_int_pos (pname, val, 0);
}

/* ======================================== */

int read_long_pos (char * pname, long * val, uint16_t pos) {
  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n", pname);
    return -1;
  }
  *val = strtol (tmp_val, &err, 10);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting integer value\n", pname);
    return -1;
  }
  return 0;
}

/* ======================================== */

int read_long (char * pname, long * val) {
  return read_long_pos (pname, val, 0);
}

/* ======================================== */

int read_longlong_pos (char * pname, long long * val, uint16_t pos) {
  
  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n", pname);
    return -1;
  }
  *val = strtoll (tmp_val, &err, 10);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting integer value\n", pname);
    return -1;
  }
  return 0;
 
}

/* ======================================== */

int read_longlong (char * pname, long long * val) {
  return read_longlong_pos (pname, val, 0);
}

/* ======================================== */

int read_double_pos (char * pname, double * val, uint16_t pos) {
  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n", pname);
    return -1;
  }
  *val = strtod (tmp_val, &err);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting double value\n", pname);
    return -1;
  }
  return 0;
}

/* ======================================== */

int read_double (char * pname, double * val) {
  return read_double_pos (pname, val, 0);
}

/* ======================================== */

int read_float_pos (char * pname, float * val, uint16_t pos) {
  
  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n",pname);
    return -1;
  }
  *val = strtof (tmp_val, &err);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting float value\n", pname);
    return -1;
  }
  return 0;
  
}

/* ======================================== */

int read_float (char * pname, float * val) {
  return read_float_pos (pname, val, 0);
}

/* ======================================== */

int read_long_double_pos (char * pname, long double * val, uint16_t pos) {

  char * tmp_val;
  char * err;
  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n", pname);
    return -1;
  }
  *val = strtold (tmp_val, &err);
  if (*err != '\0') {
    logError ("invalid value for keyword %s, expecting long double value\n", pname);
    return -1;
  }
  return 0;

}

/* ======================================== */

int read_long_double (char * pname, long double * val) {
  return read_long_double_pos (pname, val, 0);
}

/* ======================================== */

int read_string_pos (char * pname, char ** val, uint16_t pos) {

  char * tmp_val;

  tmp_val = getVal(pname, pos);
  if (tmp_val == NULL) {
    logError ("keyword %s not found in the configuration file\n");
    return -1;
  }
  *val = (char * ) malloc ((strlen(tmp_val) + 1) * sizeof (char));
  strcpy (*val, tmp_val);
  return 0;
}

/* ======================================== */

int read_string (char * pname, char ** val) {
  return read_string_pos (pname, val, 0);
}


/* ======================================== */

int processConf ( confParam * paramSet) {
  int i;

  for (i=0; paramSet[i].keyWord; i++) {
    
    switch (paramSet[i].type) {
    case t_int:
      if (read_int (paramSet[i].keyWord, (int *) (paramSet[i].param)) < 0) {
	*((int*)(paramSet[i].param)) = atoi (paramSet[i].defaultValue);
      }
      break;
    case t_long:
      if (read_long (paramSet[i].keyWord, (long *) (paramSet[i].param)) < 0) {
	*((long*)(paramSet[i].param)) = atol (paramSet[i].defaultValue);
      }
      break;
    case t_longlong:
      if (read_longlong (paramSet[i].keyWord, 
			 (long long *) (paramSet[i].param)) < 0) {
	*((long long*)(paramSet[i].param)) = atoll (paramSet[i].defaultValue);
      }
      break;
    case t_double:
      if (read_double (paramSet[i].keyWord, 
		       (double *) (paramSet[i].param)) < 0) {
	*((double*)(paramSet[i].param)) = strtod (paramSet[i].defaultValue, NULL);
      }
      break;
    case t_float:
      if (read_float (paramSet[i].keyWord, 
		      (float *) (paramSet[i].param)) < 0) {
	*((float*)(paramSet[i].param)) = strtof (paramSet[i].defaultValue, NULL);
      }
      break;
    case t_long_double:
      if (read_long_double (paramSet[i].keyWord, 
			    (long double *) (paramSet[i].param)) < 0) {
	*((long double*)(paramSet[i].param)) = strtold (paramSet[i].defaultValue, NULL);
      }
      break;
    case t_string:
      if (read_string (paramSet[i].keyWord, 
		       (char **) (paramSet[i].param)) < 0) {
	*((char **) (paramSet[i].param)) = (char *) malloc ((strlen (paramSet[i].defaultValue)+1)* sizeof (char));
	strcpy (*((char**)(paramSet[i].param)), paramSet[i].defaultValue);
      }
      break;
    default:
      break;
    }
  } 
  return 0;
}

/* ======================================= */

#ifdef TEST_CONF

void printConf () {

  int i,j;
  for (i=0; i < nbConfLine; i++) {
    printf ("key=%s n=%d values=", conf[i].key, conf[i].nbVal);
    fflush (stdout);
    for (j=0; j < conf[i].nbVal; j++) {
      
      printf ("|%d)%s ", j, (conf[i].values)[j]);
      fflush(stdout);
    }
    printf ("\n");
  }
}



/* ======================================= */

int main (int argc, char ** argv) {

  char * s;
  int a;
  long b;
  long long c;
  float d;

#if 0
  static confParam config_param_set [] =
    
    {"int", t_int, &a, "0",
     "long", t_long, &b, "0",
     "longlong", t_longlong, &c, "0",
     "float", t_float, &d, "3.41",
     "string", t_string, &s, "abc",
     NULL, t_undef, NULL, NULL};
#endif
  
  loadConf (argv[1]);


  read_int ("int", &a);
  read_long ("long", &b);
  read_longlong ("longlong", &c);
  read_float ("float", &d);
  read_string ("string", &s);

  printf ("a=%d, b=%ld, c=%lld, d=%f, s=%s\n", a,b,c,d,s); 
  
  printConf ();
  destroyConf ();
  return 0;
}

#endif
