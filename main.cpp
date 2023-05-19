/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Virtual Camera - create and modify VIEW and PROJECTION matrices              |
| keyboard controls: W,S,A,D,left and right arrows                             |
\******************************************************************************/
#include "gl_utils.h"
#include "maths_funcs.h"
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"

using namespace std;

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 640;
GLFWwindow* g_window = NULL;

int main() {
    restart_gl_log();
    /*------------------------------start GL context------------------------------*/
    start_gl();

    /*------------------------------create geometry-------------------------------*/
    GLfloat face1[] = {
       -0.5f,  -0.5f, 0.5f, // 1
       -0.5f, 0.5f, 0.5f,   // 2
        0.5f, -0.5f, 0.5f,  // 3
        -0.5f, 0.5f, 0.5f,  // 4
        0.5f,  0.5f, 0.5f,  // 5
        0.5f, -0.5f, 0.5f,  // 6
    };
    GLfloat face1_colours[] = { 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0 };
    
    GLfloat face2[] = {
        0.5f, -0.5f, 0.5f,  // 1 (6 elsobol)
        0.5f,  0.5f, 0.5f,  // 2 (4 elsobol)
        0.5f, -0.5f, -0.5f,  // 3 (1, -0.5 z)
        0.5f,  0.5f, 0.5f, // 4 
        0.5f,  0.5f, -0.5f, // 5 (2, -0.5 z)
        0.5f, -0.5f, -0.5f, // 6
    };


    GLfloat face2_colours[] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    // hatso lap: elso lapot tukrozzuk!
    GLfloat face3[] = {
        -0.5f, -0.5f, -0.5f, // 1
        -0.5f, 0.5f, -0.5f,   // 2
        0.5f, -0.5f, -0.5f,  // 3
        -0.5f, 0.5f, -0.5f,  // 4
        0.5f,  0.5f, -0.5f,  // 5
        0.5f, -0.5f, -0.5f,  // 6
    };
    GLfloat face3_colours[] = { 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };

    GLfloat face4[] = {
        -0.5f, -0.5f, 0.5f,  // 1 (6 elsobol)
        -0.5f,  0.5f, 0.5f,  // 2 (4 elsobol)
        -0.5f, -0.5f, -0.5f,  // 3 (1, -0.5 z)
        -0.5f,  0.5f, 0.5f, // 4 
        -0.5f,  0.5f, -0.5f, // 5 (2, -0.5 z)
        -0.5f, -0.5f, -0.5f, // 6
    };
    GLfloat face4_colours[] = { 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0 };

    // teteje
    GLfloat face5[] = {
        -0.5f, 0.5f, 0.5f,  // 1
        -0.5f, 0.5f, -0.5f,  // 2
        0.5f,  0.5f, 0.5f,  // 3
       -0.5f, 0.5f, -0.5f,  // 4
        0.5f, 0.5f, -0.5f,  // 4
        0.5f,  0.5f, 0.5f,  // 3
    };
    GLfloat face5_colours[] = { 1.0, 0.5, 0.0, 1.0, 0.5, 0.0, 1.0, 0.5, 0.0, 1.0, 0.5, 0.0, 1.0, 0.5, 0.0, 1.0, 0.5, 0.0 };

    // alja
    GLfloat face6[] = {
        -0.5f, -0.5f, 0.5f,  // 1
        -0.5f, -0.5f, -0.5f,  // 2
        0.5f,  -0.5f, 0.5f,  // 3
       -0.5f, -0.5f, -0.5f,  // 4
        0.5f, -0.5f, -0.5f,  // 4
        0.5f,  -0.5f, 0.5f,  // 3
    };
    GLfloat face6_colours[] = { 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0 };

    //GLfloat rgbcolours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    

    GLuint face1_points_vbo;
    glGenBuffers(1, &face1_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face1_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face1, GL_STATIC_DRAW);

    GLuint face1_colour_vbo;
    glGenBuffers(1, &face1_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face1_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face1_colours, GL_STATIC_DRAW);

    GLuint face2_points_vbo;
    glGenBuffers(1, &face2_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face2_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face2, GL_STATIC_DRAW);

    GLuint face2_colour_vbo;
    glGenBuffers(1, &face2_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face2_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face2_colours, GL_STATIC_DRAW);

    GLuint face3_points_vbo;
    glGenBuffers(1, &face3_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face3_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face3, GL_STATIC_DRAW);

    GLuint face3_colour_vbo;
    glGenBuffers(1, &face3_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face3_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face3_colours, GL_STATIC_DRAW);

    GLuint face4_points_vbo;
    glGenBuffers(1, &face4_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face4_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face4, GL_STATIC_DRAW);

    GLuint face4_colour_vbo;
    glGenBuffers(1, &face4_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face4_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face4_colours, GL_STATIC_DRAW);

    GLuint face5_points_vbo;
    glGenBuffers(1, &face5_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face5_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face5, GL_STATIC_DRAW);

    GLuint face5_colour_vbo;
    glGenBuffers(1, &face5_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face5_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face5_colours, GL_STATIC_DRAW);

    GLuint face6_points_vbo;
    glGenBuffers(1, &face6_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face6_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face6, GL_STATIC_DRAW);

    GLuint face6_colour_vbo;
    glGenBuffers(1, &face6_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face6_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), face6_colours, GL_STATIC_DRAW);

    GLuint face1_vao;
    glGenVertexArrays(1, &face1_vao);
    glBindVertexArray(face1_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face1_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face1_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint face2_vao;
    glGenVertexArrays(1, &face2_vao);
    glBindVertexArray(face2_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face2_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face2_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint face3_vao;
    glGenVertexArrays(1, &face3_vao);
    glBindVertexArray(face3_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face3_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face3_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint face4_vao;
    glGenVertexArrays(1, &face4_vao);
    glBindVertexArray(face4_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face4_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face4_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint face5_vao;
    glGenVertexArrays(1, &face5_vao);
    glBindVertexArray(face5_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face5_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face5_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint face6_vao;
    glGenVertexArrays(1, &face6_vao);
    glBindVertexArray(face6_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face6_points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, face6_colour_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /*------------------------------create shaders--------------------------------*/
    char vertex_shader[1024 * 256];
    char fragment_shader[1024 * 256];
    parse_file_into_str("vs.glsl", vertex_shader, 1024 * 256);
    parse_file_into_str("fs.glsl", fragment_shader, 1024 * 256);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* p = (const GLchar*)vertex_shader;
    glShaderSource(vs, 1, &p, NULL);
    glCompileShader(vs);

    // check for compile errors
    int params = -1;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
        print_shader_info_log(vs);
        return 1; // or exit or something
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    p = (const GLchar*)fragment_shader;
    glShaderSource(fs, 1, &p, NULL);
    glCompileShader(fs);

    // check for compile errors
    glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
        print_shader_info_log(fs);
        return 1; // or exit or something
    }

    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);

    glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf(stderr, "ERROR: could not link shader programme GL index %i\n", shader_programme);
        print_programme_info_log(shader_programme);
        return false;
    }

    /*--------------------------create camera matrices----------------------------*/
    /* create PROJECTION MATRIX */
    // input variables
    float near = 0.1f;                                   // clipping plane
    float far = 100.0f;                                 // clipping plane
    float fov = 67.0f * ONE_DEG_IN_RAD;                 // convert 67 degrees to radians
    float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
    // matrix components
    float inverse_range = 1.0f / tan(fov * 0.5f);
    float Sx = inverse_range / aspect;
    float Sy = inverse_range;
    float Sz = -(far + near) / (far - near);
    float Pz = -(2.0f * far * near) / (far - near);
    GLfloat proj_mat[] = { Sx, 0.0f, 0.0f, 0.0f, 0.0f, Sy, 0.0f, 0.0f, 0.0f, 0.0f, Sz, -1.0f, 0.0f, 0.0f, Pz, 0.0f }; // PR matrix

    /* create VIEW MATRIX */
    float cam_speed = 1.0f;                 // 1 unit per second
    float cam_yaw_speed = 10.0f;                // 10 degrees per second
    float cam_pos[] = { 0.0f, 0.0f, 2.0f }; // don't start at zero, or we will be too close
    float cam_yaw = 0.0f;                 // y-rotation in degrees
    // a kamera mozgasanak ellentetet fogjuk csinalni a targyal, igy azt az erzetet keltve hogy mi mozgunk
    mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
    mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
    mat4 view_mat = R * T; // VIEW matrix

    // model az y tengelyes forgashoz. Model: transzformaciokat tartalmaz
    mat4 model = rotate_y_deg(identity_mat4(), glfwGetTime());

    /* get location numbers of matrices in shader programme */
    GLint view_mat_location = glGetUniformLocation(shader_programme, "view");
    GLint proj_mat_location = glGetUniformLocation(shader_programme, "proj");
    /* use program (make current in state machine) and set default matrix values*/
    glUseProgram(shader_programme);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, proj_mat);

    GLint moveMatrix_location = glGetUniformLocation(shader_programme, "moveMatrix");
    GLint model_location = glGetUniformLocation(shader_programme, "model");


    GLfloat moveMatrix[] = {
      1.0f,
      0.0f,
      0.0f,
      0.0f, // first column
            0.0f,
            1.0f,
            0.0f,
            0.0f, // second column
                    0.0f,
                    0.0f,
                    1.0f,
                    0.0f, // third column
                            0.0f,
                            0.5f,
                            0.0f,
                            1.0f  // fourth column
    };

    glUniformMatrix4fv(moveMatrix_location, 1, GL_FALSE, moveMatrix);
    // model uniform matrixa
    glUniformMatrix4fv(moveMatrix_location, 1, GL_FALSE, model.m);

    glUseProgram(shader_programme);

    /*------------------------------rendering loop--------------------------------*/
    /* some rendering defaults */
    // generalja le a hatuljat is!
    //glEnable(GL_CULL_FACE); // cull face
    //glCullFace(GL_BACK);    // cull back face
    //glFrontFace(GL_CW);     // GL_CCW for counter clock-wise
    
    // engedelyezi a melyseg ellenorzeset 
    glEnable(GL_DEPTH_TEST);

    float speed = 1.0f; // move at 1 unit per second
    float last_position = 0.0f; // pozicio meg skalazas

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    while (!glfwWindowShouldClose(g_window)) {
        static double previous_seconds = glfwGetTime();
        double current_seconds = glfwGetTime();
        double elapsed_seconds = current_seconds - previous_seconds;
        previous_seconds = current_seconds;

        _update_fps_counter(g_window);
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, g_gl_width, g_gl_height);

        // KIRAJZOLAS

        glUseProgram(shader_programme);
        // elso oldal kirajz elulso
        glBindVertexArray(face1_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // masodik oldal kirajz jobboldali
        glBindVertexArray(face2_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // harmadik oldal kirajzolas hatso
        glBindVertexArray(face3_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // negyedik oldal baloldali
        glBindVertexArray(face4_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // otodik oldal (felso)
        glBindVertexArray(face5_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // hatodik oldal (felso)
        glBindVertexArray(face6_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ---------

        // update other events like input handling
        glfwPollEvents();

        moveMatrix[13] = elapsed_seconds * speed + last_position;
        last_position = moveMatrix[13];
        if (fabs(last_position) > 1.0) { speed = -speed; }
        /*-----------------------------move camera here-------------------------------*/
        // control keys
        // a haromszog pont az ellentetet csinalja a mi pozicio valtasunknak
        bool cam_moved = false;
        if (glfwGetKey(g_window, GLFW_KEY_A)) {
            cam_pos[0] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_D)) {
            cam_pos[0] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_PAGE_UP)) {
            cam_pos[1] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_PAGE_DOWN)) {
            cam_pos[1] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_W)) {
            cam_pos[2] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_S)) {
            cam_pos[2] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_LEFT)) {
            cam_yaw += cam_yaw_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_RIGHT)) {
            cam_yaw -= cam_yaw_speed * elapsed_seconds;
            cam_moved = true;
        }
        /* update view matrix */
        if (cam_moved) {
            mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1],
                -cam_pos[2])); // cam translation
            mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);     //
            mat4 view_mat = R * T;
            glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);

        }
        // move matrix folyamatosan
        glUniformMatrix4fv(moveMatrix_location, 1, GL_FALSE, moveMatrix);

        // forgas modellje
        model = rotate_y_deg(identity_mat4(), glfwGetTime() * 100);
        glUniformMatrix4fv(model_location, 1, GL_FALSE, model.m);

        if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(g_window, 1); }
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers(g_window);
    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    return 0;
}