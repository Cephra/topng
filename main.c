#include "main.h"

void cleanup() {
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    
    // cleanup write pointers
    if (png_w_ptr != NULL) {
        png_destroy_write_struct(&png_w_ptr, &info_w_ptr);
    }

    // cleanup read pointers
    if (png_r_ptr != NULL) {
        png_destroy_read_struct(&png_r_ptr, &info_r_ptr, &end_r_ptr);
    }

    if (row != NULL) {
        free(row);
        row = NULL;
    }
}

char *png2str(char *filename) {
    png_byte sig[8];
    png_uint_32 width, height; 
    int bit_depth, color_type;
    char *str;

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("%s: can't open %s: %s\n", proc, filename, strerror(errno));
        cleanup();
        return NULL;
    }
    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8)) {
        printf("invalid file\n");
        cleanup();
        return NULL;
    }

    png_r_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_r_ptr) {
        cleanup();
        return NULL;
    }

    info_r_ptr = png_create_info_struct(png_r_ptr);
    if (!info_r_ptr) {
        cleanup();
        return NULL;
    }

    end_r_ptr = png_create_info_struct(png_r_ptr);
    if (!end_r_ptr) {
        cleanup();
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_r_ptr))) {
        cleanup();
        return NULL;
    }

    png_init_io(png_r_ptr, fp);
    png_set_sig_bytes(png_r_ptr, 8);
    png_read_info(png_r_ptr, info_r_ptr);

    png_get_IHDR(png_r_ptr, info_r_ptr, 
            &width, &height, &bit_depth, &color_type,
            NULL, NULL, NULL);
    str = malloc(width*height*sizeof(png_byte));
    
    // read the message
    for (int h = 0; h < height; h++) {
        png_read_row(png_r_ptr, (png_bytep)(str+h*width), NULL);
    }

    cleanup();
    return str;
}

void str2png(char *filename, char *txt) {
    int txtpos = 0, txtlen = strlen(txt)+1, wh = round(sqrt(txtlen));
    printf("%d bytes -> %d*%d\n", txtlen, wh, wh);

    // open the file and abort on error
    fp = fopen(filename, "wb");
    if (!fp) {
        printf("%s: can't open %s: %s\n", proc, filename, strerror(errno));
        cleanup();
        return;
    }

    png_w_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_w_ptr == NULL) {
        cleanup();
        return;
    }

    info_w_ptr = png_create_info_struct(png_w_ptr);
    if (info_w_ptr == NULL) {
        cleanup();
        return;
    }

    if (setjmp(png_jmpbuf(png_w_ptr))) {
        cleanup();
        return;
    }
    png_init_io(png_w_ptr, fp);

    // write header
    png_set_IHDR(png_w_ptr, info_w_ptr, wh, wh, 8, 
            PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, 
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_w_ptr, info_w_ptr);

    row = malloc(wh*sizeof(png_byte));
    for (int h = 0; h < wh; h++) {
        memset(row, 0xFF, wh*sizeof(png_byte));
        for (int w = 0; w < wh; w++) {
            if (txtpos < txtlen) {
                row[w] = txt[txtpos];
                txtpos++;
            }
        }
        png_write_row(png_w_ptr, (png_bytep)row);
    }
    png_write_end(png_w_ptr, NULL);

    // closing file
    cleanup();
    return;
}

void printhelp() {
    printf("usage: %s -rw -o filename\n", proc);
}

int main(int argc, char *argv[]) {
    char *buf, chunk[BUF_SIZE];
    char *msg, *fname = NULL;
    char c;
    proc = argv[0];
    size_t bufs = 1;
    unsigned char rw;

    opterr = 0;

    while ((c = getopt(argc, argv, "rwo:")) != -1) {
        switch (c) {
        case 'r':
            if (rw == 0) rw = 1;
            break;
        case 'w':
            if (rw == 0) rw = 2;
            break;
        case 'o':
            fname = optarg;
            break;
        case '?':
            if (optopt == 'c')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option '%c'.\n", optopt);
            else 
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
            break;
        default:
            abort();
        }
    }

    if (fname == NULL) {
        fprintf(stderr, "No output file specified.\n");
        return EXIT_FAILURE;
    }

    if (rw == 1) {
        msg = png2str(fname);
        printf("%s\n", msg);
        free(msg);
    } else if (rw == 2) {
        // read from stdin
        buf = malloc(sizeof(char)*BUF_SIZE);
        buf[0] = 0;
        while (fgets(chunk, BUF_SIZE, stdin)) {
            bufs += strlen(chunk);
            buf = realloc(buf, bufs);
            strcat(buf,chunk);
        }

        str2png(fname, buf);
        free(buf);
    } else {
        printhelp();
    }

    return 0;
}
