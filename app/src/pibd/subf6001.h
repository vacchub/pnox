#ifndef _SUBF6001_H
#define _SUBF6001_H

struct bittrex {
    char    code[32];
    double  last;
    double  volume;
};

int proc_settbl(struct bittrex *);
int get_web_data();
int data_parse(char *);

#endif
