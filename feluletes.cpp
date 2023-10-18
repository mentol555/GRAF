#include "gl_utils.h"
#include "maths_funcs.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h> 
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gl_utils.h"
#define GL_LOG_FILE "gl.log"

using namespace std;

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 960;
int g_gl_height = 960;
GLFWwindow* g_window = NULL;

double bezier(int i, double u, int n) {
    // Binomiális együttható számítása
    double binomialCoefficient = 1.0;
    for (int j = 1; j <= i; j++) {
        binomialCoefficient *= (double)(n - j + 1) / j;
    }

    // Bézier-görbe pontjának kiszámítása
    double result = binomialCoefficient * pow(u, i) * pow(1 - u, n - i);

    return result;
}

int main() {
    const GLubyte* renderer;
    const GLubyte* version;
    GLuint vbo, vbo2, vbo3;
    GLuint vao, vao2, vao3;

    start_gl();

    float step = 0.2f;
    // x oszlopok szama
    // y sorok szama
    int x = 6, y = 5;
    int k = 0;
    int c = 0;

    GLfloat points[300] = {};
    GLfloat points2[300] = {};
    for (int j = 0; j < y; j++) {
        for (int i = 0; i < x; i++)
        {
            points[k * 3] = -0.5f + (i * step);
            points[k * 3 + 1] = -1.0f + (1.0f / y) + (j * 0.2f);
            points[k * 3 + 2] = 0.0f;
            k = k + 1;
        }
    }
    for (int j = 0; j <= (x * 3); j += 3) {
        for (int i = 0; i < (x * 3 * y); i += (x * 3)) {
            points2[c * 3] = points[i + j];
            points2[c * 3 + 1] = points[i + j + 1];
            points2[c * 3 + 2] = points[i + j + 2];
            c = c + 1;
        }
    }
    /*
    for (int i = 0; i < sizeof(points2) / 4; i += 3) {
        cout << "X= " << points2[i] << "Y= " << points2[i + 1] << "Z= " << points2[i + 2] << endl;
    }
    */




    const char* vertex_shader =
        "#version 410\n"
        "in vec3 vertex_position;"
        "uniform mat4 view, proj;"

        "void main() {"
        "   gl_Position = proj * view * vec4(vertex_position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 410\n"

        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0f, 0.5f, 0.0f, 1.0);"
        "}";
        

    GLuint vert_shader, frag_shader;

    GLuint shader_programme;


    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);

    GLfloat curve_points[3030] = {}; // nemtom miert 3030
    int curve_i = 0;

    for (float u = 0; u < 1; u = u + 0.1){
        for (float v = 0; v < 1; v = v + 0.01){

            k = 0;
            float Bezierx = 0;
            float Beziery = 0;
            float Bezierz = 0;

            for (int i = 0; i < x; i++) {
                for (int j = 0; j < y; j++) {
                    Bezierx += bezier(i, u, x) * bezier(j, v, y) * points[k]; //x koord
                    Beziery += bezier(i, u, x) * bezier(j, v, y) * points[k + 1]; //y koord
                    Bezierz += bezier(i, u, x) * bezier(j, v, y) * points[k + 2]; //z koord
                    cout << points[k] << " " << points[k + 1] << " " << points[k + 2] << endl;
                    k += 3;
                }
            }
            curve_points[curve_i] = Bezierx;
            curve_points[curve_i + 1] = Beziery;
            curve_points[curve_i + 2] = Bezierz;

            curve_i+=3;
        }

    }

    // SHADERS
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points2, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo3);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), curve_points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    glGenVertexArrays(1, &vao3);
    glBindVertexArray(vao3);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader, NULL);
    glCompileShader(vert_shader);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader, NULL);
    glCompileShader(frag_shader);
    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, frag_shader);
    glAttachShader(shader_programme, vert_shader);
    glLinkProgram(shader_programme);


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
    float cam_pos[] = { 0.0f, -1.5f, 1.0f }; // don't start at zero, or we will be too close
    float cam_yaw = 60.0f;                 //x-rotation in degrees
    // a kamera mozgasanak ellentetet fogjuk csinalni a targyal, igy azt az erzetet keltve hogy mi mozgunk
    mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
    mat4 R = rotate_x_deg(identity_mat4(), -cam_yaw);
    mat4 view_mat = R * T; // VIEW matrix

    /* get location numbers of matrices in shader programme */

    GLint view_mat_location = glGetUniformLocation(shader_programme, "view");
    GLint proj_mat_location = glGetUniformLocation(shader_programme, "proj");


    /* use program (make current in state machine) and set default matrix values*/
    glUseProgram(shader_programme);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, proj_mat);


    glUseProgram(shader_programme);

    glEnable(GL_DEPTH_TEST);

    glPointSize(15.0f);

    float speed = 1.0f; // move at 1 unit per second
    float last_position = 0.0f; // pozicio meg skalazas


    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    while (!glfwWindowShouldClose(g_window)) {

        static double previous_seconds = glfwGetTime();
        double current_seconds = glfwGetTime();
        double elapsed_seconds = current_seconds - previous_seconds;
        previous_seconds = current_seconds;

        // wipe the drawing surface clear
        glViewport(0, 0, g_gl_width, g_gl_height);

        _update_fps_counter(g_window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_programme);
        glViewport(0, 0, g_gl_width, g_gl_height);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, x * y);

        for (int i = 0; i <= y; i++) {
            glDrawArrays(GL_LINE_STRIP, i * x, x);
        }

        glBindVertexArray(vao2);
        glDrawArrays(GL_POINTS, 0, x * y);
        for (int i = 0; i <= x; i++) {
            glDrawArrays(GL_LINE_STRIP, i * y, y);
        }

        glBindVertexArray(vao3);
        glDrawArrays(GL_POINTS, 100, 100);
        


        // update other events like input handling
        glfwPollEvents();

        glUseProgram(shader_programme);

        if (fabs(last_position) > 1.0) { speed = -speed; }
        /*-----------------------------move camera here-------------------------------*/
        // control keys
        bool cam_moved = false;
        if (glfwGetKey(g_window, GLFW_KEY_D)) {
            cam_pos[0] -= cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_A)) {
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
            mat4 R = rotate_x_deg(identity_mat4(), -cam_yaw);     //
            mat4 view_mat = R * T;
            glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);

        }
        glUseProgram(shader_programme);


        glfwSwapBuffers(g_window);
    }


    glfwTerminate();
    return 0;
}
