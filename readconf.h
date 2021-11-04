#ifndef _READ_CONF_H_
#define _READ_CONF_H_

typedef enum {
  t_int, t_long, t_longlong, t_double, 
  t_float, t_long_double, t_string, t_undef
} params_type;

typedef struct {
  char * keyWord;
  params_type type;
  void * param;
  char * defaultValue;
} confParam;


int loadConf (const char * file);
int destroyConf ();
int read_int_pos (char * pname, int * val, uint16_t pos);
int read_int (char * pname, int * val);
int read_long_pos (char * pname, long * val, uint16_t pos);
int read_long (char * pname, long * val);
int read_longlong_pos (char * pname, long long * val, uint16_t pos);
int read_longlong (char * pname, long long * val);
int read_double_pos (char * pname, double * val, uint16_t pos);
int read_double (char * pname, double * val);
int read_float_pos (char * pname, float * val, uint16_t pos);
int read_float (char * pname, float * val);
int read_long_double_pos (char * pname, long double * val, uint16_t pos);
int read_long_double (char * pname, long double * val);
int read_string_pos (char * pname, char ** val, uint16_t pos);
int read_string (char * pname, char ** val);
int processConf ( confParam * paramSet);
#endif
