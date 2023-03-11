/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| This is just a file holding some commonly-used "utility" functions to keep   |
| the main file a bit easier to read. You can might build up something like    |
| this as learn more GL. Note that you don't need much code here to do good GL.|
| If you have a big object-oriented engine then maybe you can ask yourself if  |
| it is really making life easier.                                             |
\******************************************************************************/
#include "gl_utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#define GL_LOG_FILE "gl.log"
#define MAX_SHADER_LENGTH 262144
#pragma warning(disable:4996)


bool parse_file_into_str(const char* file_name, char* shader_str, int max_len) {
    FILE* file = fopen(file_name, "r");
    if (!file) {
        std::cout << "ERROR: opening file for reading:" << file_name;
        return false;
    }
    size_t cnt = fread(shader_str, 1, max_len - 1, file);
    if ((int)cnt >= max_len - 1) { std::cout << "WARNING: file %s too big - truncated.\n"<<file_name; }
    if (ferror(file)) {
        std::cout << "ERROR: reading shader file %s\n"<<file_name;
        fclose(file);
        return false;
    }
    // append \0 to end of file string
    shader_str[cnt] = 0;
    fclose(file);
    return true;
}