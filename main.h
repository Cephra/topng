#ifndef _main_h
#define _main_h

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#define PNG_DEBUG 3
#include <png.h>

#define BUF_SIZE 1024

const char *proc;

FILE *fp;
char *row;
png_structp png_w_ptr;
png_infop info_w_ptr;

png_structp png_r_ptr;
png_infop info_r_ptr;
png_infop end_r_ptr;

void cleanup();
char *png2str();
void str2png();

#endif
