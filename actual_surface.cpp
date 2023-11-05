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

float step = 0.2f;
// x oszlopok szama
// y sorok szama
int x = 5, y = 5;

GLfloat curve_points[5000] = {}; // egyik iranyba a curve-k
GLfloat curve_points2[5000] = {}; // masik iranyba a curve-k

GLfloat surface_color[5000] = {};

GLFWwindow* g_window = NULL;

// bezier képlet

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


// BEZIER generálás egy irányba.
// Megkapja a curve_points-ot, amibe lesznek mentve egyik irányba a görbe pontjai
// surface color annyi hogy legyen színe, most statikusan egyszínű de lehet variálni

void bezierGenerator(GLfloat curve_points[5000], GLfloat surface_color[5000], GLfloat points[]) {
    int k = 0;
    int curve_i = 0;

    for (float u = 0; u < 1; u += 0.11) {
        for (float v = 0; v < 1; v += 0.01) {

            k = 0;
            float Bezierx = 0;
            float Beziery = 0;
            float Bezierz = 0;

            for (int j = 0; j < y; j++) {
                for (int i = 0; i < x; i++) {
                    Bezierx += bezier(i, u, x - 1) * bezier(j, v, y - 1) * points[k * 3]; // x koord
                    Beziery += bezier(i, u, x - 1) * bezier(j, v, y - 1) * points[k * 3 + 1]; // y koord
                    Bezierz += bezier(i, u, x - 1) * bezier(j, v, y - 1) * points[k * 3 + 2]; // z koord
                    k++;
                }
            }

            curve_points[curve_i * 3] = Bezierx;
            curve_points[curve_i * 3 + 1] = Beziery;
            curve_points[curve_i * 3 + 2] = Bezierz;

            surface_color[curve_i * 3] = 0.5f;
            surface_color[curve_i * 3 + 1] = 0.0f;
            surface_color[curve_i * 3 + 2] = 1.0f;

            curve_i++;
        }
    }
}

int main() {
    const GLubyte* renderer;
    const GLubyte* version;

    // vbo - points vbo
    // vbo2 - points 2 vbo
    // vbo 3, 4 - surface egyik, masik iranyba
    GLuint vbo, vbo2, vbo3, vbo4;
    GLuint points_color_vbo, surface_color_vbo;
    GLuint vao, vao2, vao3, vao4;

    start_gl();

    int k = 0;
    int c = 0;

    // csak találomra írtam, a z-t változtatom vele, hogy initialba ne egy sík legyen a felület
    const float gorbulet = 0.05f;

    // PONT generálások

    GLfloat points[300] = {};
    GLfloat points2[300] = {};
    GLfloat points_color[300] = {};
    for (int j = 0; j < y; j++) {
        for (int i = 0; i < x; i++)
        {
            points_color[k * 3] = 1.0f;
            points_color[k * 3 + 1] = 0.5f;
            points_color[k * 3 + 2] = 0.0f;

            points[k * 3] = -0.5f + (i * step);
            points[k * 3 + 1] = 0.0f + (1.0f / y) + (j * 0.2f);
            points[k * 3 + 2] = i < x / 2 ? gorbulet * i : -gorbulet * i;
            if (j < y / 2)
                points[k * 3 + 2] += gorbulet * j;
            else
                points[k * 3 + 2] -= gorbulet * j;

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

    // VS FS SHADEREK 

    const char* vertex_shader =
        "#version 410\n"
        "layout(location = 0) in vec3 vertex_position;"
        "layout(location = 1) in vec3 vertex_color;"
        "out vec3 color;"

        "uniform mat4 view, proj;"

        "void main() {"
        "   color = vertex_color;"
        "   gl_Position = proj * view * vec4(vertex_position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 410\n"
        "in vec3 color;"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(color, 1.0);"
        "}";


    GLuint vert_shader, frag_shader;

    GLuint shader_programme;

    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);
    
    // BEZIER FELÜLET KISZÁMOLÁSA, CURVE_POINTS-ba tároljuk ezeket
    bezierGenerator(curve_points, surface_color, points);
    bezierGenerator(curve_points2, surface_color, points2);

    // SHADERS

    // points 1
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glGenBuffers(1, &points_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points_color, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // points 2
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points2, GL_STATIC_DRAW);

    glGenBuffers(1, &points_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, x * y * 3 * sizeof(GLfloat), points_color, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // surface - egyik irany
    glGenBuffers(1, &vbo3);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), curve_points, GL_STATIC_DRAW);

    glGenBuffers(1, &surface_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, surface_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), surface_color, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao3);
    glBindVertexArray(vao3);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, surface_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // surface - masik irany 

    glGenBuffers(1, &vbo4);
    glBindBuffer(GL_ARRAY_BUFFER, vbo4);
    glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), curve_points2, GL_STATIC_DRAW);

    glGenBuffers(1, &surface_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, surface_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), surface_color, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao4);
    glBindVertexArray(vao4);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo4);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, surface_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // -----------------------//

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

    // ----- axis ------- // 

    GLuint vert_shader_axis;
    GLuint shader_programme_axis;

    const char* vertex_shader_axis =
        "#version 410\n"
        "layout(location = 0) in vec3 vertex_position;"
        "layout(location = 1) in vec3 vertex_color;"

        "out vec3 color;"
        "uniform mat4 view, proj;"

        "void main() {"
        "   color = vertex_color;"
        "   gl_Position = proj * view * vec4(vertex_position, 1.0);"
        "}";

    GLfloat axes_vertices[] = {
        -0.6f, 0.0f, 0.0f,  // Start of x-axis
        0.6f, 0.0f, 0.0f,  // End of x-axis
        -0.6f, 0.0f, 0.0f,  // Start of y-axis
        -0.2f, 1.0f, 0.0f,  // End of y-axis
        -0.6f, 0.0f, 0.0f,  // Start of z-axis
        -0.4f, 0.0f, 1.0f   // End of z-axis
    };

    GLfloat axes_colors[] = {
        0.0f, 1.0f, 0.0f,  // Green (x-axis)
        0.0f, 1.0f, 0.0f,  // Green (x-axis)
        1.0f, 0.0f, 0.0f,  // Red (y-axis)
        1.0f, 0.0f, 0.0f,  // Red (y-axis)
        0.0f, 0.0f, 1.0f,  // Blue (z-axis)
        0.0f, 0.0f, 1.0f   // Blue (z-axis)
    };

    GLuint axes_vbo, axes_color_vbo, axes_vao;
    glGenBuffers(1, &axes_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axes_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), axes_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &axes_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axes_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_colors), axes_colors, GL_STATIC_DRAW);

    glGenVertexArrays(1, &axes_vao);
    glBindVertexArray(axes_vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, axes_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, axes_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    vert_shader_axis = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader_axis, 1, &vertex_shader_axis, NULL);
    glCompileShader(vert_shader_axis);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader, NULL);
    glCompileShader(frag_shader);
    shader_programme_axis = glCreateProgram();
    glAttachShader(shader_programme_axis, frag_shader);
    glAttachShader(shader_programme_axis, vert_shader_axis);
    glLinkProgram(shader_programme_axis);


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
    float cam_speed = 1.0f;
    float cam_yaw_speed = 45.0f;
    float cam_pos[] = { 0.0f, -1.0f, 1.0f }; // don't start at zero, or we will be too close
    float cam_yaw = 60.0f;
    // a kamera mozgasanak ellentetet fogjuk csinalni a targyal, igy azt az erzetet keltve hogy mi mozgunk
    mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
    mat4 R = rotate_x_deg(identity_mat4(), -cam_yaw);
    mat4 view_mat = R * T; // VIEW matrix

    // forgas
    float rotation = 0.0f;

    /* get location numbers of matrices in shader programme */

    GLint view_mat_location = glGetUniformLocation(shader_programme, "view");
    GLint proj_mat_location = glGetUniformLocation(shader_programme, "proj");


    /* use program (make current in state machine) and set default matrix values*/
    glUseProgram(shader_programme);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, proj_mat);


    // ------ axis view proj --------- //

    /* Get location numbers of matrices in shader program for the axes */
    GLint axes_view_mat_location = glGetUniformLocation(shader_programme_axis, "view");
    GLint axes_proj_mat_location = glGetUniformLocation(shader_programme_axis, "proj");

    glUseProgram(shader_programme_axis);
    glUniformMatrix4fv(axes_view_mat_location, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(axes_proj_mat_location, 1, GL_FALSE, proj_mat);
    // --------- //


    glEnable(GL_DEPTH_TEST);

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


        // ----- axis ------//
        glUseProgram(shader_programme_axis);
        // tengelyek kirajz
        glBindVertexArray(axes_vao);
        glDrawArrays(GL_LINES, 0, 6);

        // --------------- //
        
        glUseProgram(shader_programme);

        glPointSize(5.0f);

        // POINTS 1
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, x * y);
        for (int i = 0; i <= y; i++) {
            glDrawArrays(GL_LINE_STRIP, i * x, x);
        }

        // POINTS 2

        glBindVertexArray(vao2);
        glDrawArrays(GL_POINTS, 0, x * y);
        for (int i = 0; i <= x; i++) {
            glDrawArrays(GL_LINE_STRIP, i * y, y);
        }

        glPointSize(3.0f);

        // BEZIER surface egyik iranyba kirajzolas
        glUseProgram(shader_programme);
        glBindVertexArray(vao3);
        glDrawArrays(GL_POINTS, 0, 1010);

        // BEZIER surface masik iranyba kirajzolas
        glUseProgram(shader_programme);
        glBindVertexArray(vao4);
        glDrawArrays(GL_POINTS, 0, 1010);



        // update other events like input handling
        glfwPollEvents();

        glUseProgram(shader_programme);

        if (fabs(last_position) > 1.0) { speed = -speed; }
        /*-----------------------------move camera here-------------------------------*/
        // control keys
        bool cam_moved = false;
        bool cam_rotated = false;

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


        // Inside your main loop:
        if (glfwGetKey(g_window, GLFW_KEY_LEFT)) {
            rotation = cam_yaw_speed * elapsed_seconds;
            cam_rotated = true;
            cam_moved = true;
        }

        if (glfwGetKey(g_window, GLFW_KEY_RIGHT)) {
            rotation = -cam_yaw_speed * elapsed_seconds;
            cam_rotated = true;
            cam_moved = true;
        }
        
        /* update view matrix */
        if (cam_moved) {
            mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2])); // cam translation
            
            if (cam_rotated) {
                R = rotate_z_deg(R, rotation); // Rotate around x-axis
            }
            mat4 view_mat = R * T;
            glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);
        }

        

        glUseProgram(shader_programme);

        glfwSwapBuffers(g_window);
    }


    glfwTerminate();
    return 0;
}
