/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
\******************************************************************************/
#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <stdarg.h>


bool parse_file_into_str(const char* file_name, char* shader_str, int max_len);

#endif