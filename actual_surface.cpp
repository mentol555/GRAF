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
#include <iomanip>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gl_utils.h"
#define GL_LOG_FILE "gl.log"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <locale.h>




//Test
// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//Test

using namespace std;

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 960;
int g_gl_height = 960;

int currentPoint = 0;
int secondcurrentPoint = 0;
bool grid = true;
bool triangles = true;

// selected axis x = 0, y = 1, z = 2
char selectedAxis = 0;

float step = 0.2f;

int oszlop = 6, sor = 6;

// FÉNYFORRÁS, SZINE ÉS INTENZITÁSA
vec3 lightDirection(-1.0f, 1.0f, 1.0f);
GLfloat lightDirectionArray[3] = { -1.0f, 1.0f, 1.0f };

vec3 lightColor(1.0f, 1.0f, 0.0f);
GLfloat lightColorArray[3] = { 1.0f, 1.0f, 0.0f };

float lightIntensity = 1.0f;
bool lightSourceMove = false;

///

GLint dragged = -1;
int k = 0;
int c = 0;

// csak találomra írtam, a z-t változtatom vele, hogy initialba ne egy sík legyen a felület
const float gorbulet = 0.05f;

// PONT tömbbök
GLfloat points[300] = {};
GLfloat points2[300] = {};
GLfloat points_color[300] = {};


// vbo - points vbo
// vbo2 - points 2 vbo
// vbo 3, 4 - surface egyik, masik iranyba
// vbo triange
// vbo lightSource
GLuint vbo, vbo2, vbo3, vbo4, vbo_triangle, vbo_lightSource;
GLuint points_color_vbo, surface_color_vbo, triangle_color_vbo, lightSource_color_vbo;
GLuint vao, vao2, vao3, vao4, vao5, vao6;

GLfloat curve_points[5000] = {}; // egyik iranyba a curve-k
GLfloat curve_points2[5000] = {}; // masik iranyba a curve-k

GLfloat surface_color[5000] = {};

GLfloat surface_triangle[500000] = {};
GLfloat surface_triangle_colors[500000] = {};

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

// ahany alappont oszlopunk van, ketszer annyi felulet oszlopunk legyen
float oszlopEgyseg = 1 / ((float)(oszlop - 1) * 2);
// ahany alappont sorunk van, ketszer annyi felulet sorunk legyen
float sorEgyseg = 1 / ((float)(sor - 1) * 2);

float epsilon = 0.0001;

void bezierGenerator(GLfloat curve_points[5000], GLfloat surface_color[5000], GLfloat points[]) {
    int k = 0;
    int curve_i = 0;
    for (float u = 0; u + epsilon < oszlopEgyseg * (oszlop * 2 - 1); u += oszlopEgyseg) {
        for (float v = 0; v + epsilon < sorEgyseg * (sor * 2 - 1); v += sorEgyseg) {
            k = 0;
            float Bezierx = 0;
            float Beziery = 0;
            float Bezierz = 0;

            for (int j = 0; j < sor; j++) {
                for (int i = 0; i < oszlop; i++) {
                    Bezierx += bezier(i, u, oszlop - 1) * bezier(j, v, sor - 1) * points[k * 3]; // x koord
                    Beziery += bezier(i, u, oszlop - 1) * bezier(j, v, sor - 1) * points[k * 3 + 1]; // y koord
                    Bezierz += bezier(i, u, oszlop - 1) * bezier(j, v, sor - 1) * points[k * 3 + 2]; // z koord
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

vec3 calculateNormal(vec3 vector1, vec3 vector2, vec3 vector3) {
    vec3 newvec1 = vector3 - vector1;
    vec3 newvec2 = vector2 - vector1;
    return normalise(cross(newvec1, newvec2));
}

float calculateLightAngle(vec3 lightDirection, vec3 normal) {
    return std::acos(dot(normalise(lightDirection),(normalise(normal))));
}

vec3 calculateColorIntensity(vec3 lightColor, float lightIntensity, float lightAngle) {
    float intensity = max(0.0f, std::cos(lightAngle));
    return lightColor * (intensity * lightIntensity);
}

void triangulization(GLfloat curve_points[5000]) {
    int j = 0;
    int jobb = 1;
    int temp = 0;
    int count = 0;
    if (sor == 4) {
        for (int i = 0; i < 246; i += 6) {

            if (i == 36 || i == 78 || i == 120 || i == 162 || i == 204 || i == 246) {
                j += 1;
            }
            else {
                // alsó háromszögek

                surface_triangle[i * 3] = curve_points[j * 3];
                surface_triangle[i * 3 + 1] = curve_points[j * 3 + 1];
                surface_triangle[i * 3 + 2] = curve_points[j * 3 + 2];
                vec3 vector1(surface_triangle[i*3], surface_triangle[i*3+1], surface_triangle[i*3+2]);

                surface_triangle[i * 3 + 3] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 4] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 5] = curve_points[j * 3 + (1 * 3) + 2];
                vec3 vector2(surface_triangle[i*3+3], surface_triangle[i*3+4], surface_triangle[i*3+5]);

                surface_triangle[i * 3 + 6] = curve_points[j * 3 + (7 * 3)];
                surface_triangle[i * 3 + 7] = curve_points[j * 3 + (7 * 3) + 1];
                surface_triangle[i * 3 + 8] = curve_points[j * 3 + (7 * 3) + 2];
                vec3 vector3(surface_triangle[i*3+6], surface_triangle[i*3+7], surface_triangle[i*3+8]);

                vec3 normalised = calculateNormal(vector1, vector2, vector3);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle = calculateLightAngle(lightDirection, normalised);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity = calculateColorIntensity(lightColor, lightIntensity, lightAngle);
                surface_triangle_colors[i * 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 1] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 2] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 4] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 5] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 6] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 7] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 8] = colorIntensity.v[2];


                // felső háromszögek

                surface_triangle[i * 3 + 9] = curve_points[j * 3 + (7 * 3)];
                surface_triangle[i * 3 + 10] = curve_points[j * 3 + (7 * 3) + 1];
                surface_triangle[i * 3 + 11] = curve_points[j * 3 + (7 * 3) + 2];
                vec3 vector4(surface_triangle[i*3+9], surface_triangle[i * 3 + 10], surface_triangle[i * 3 + 11]);

                surface_triangle[i * 3 + 12] = curve_points[j * 3 + (8 * 3)];
                surface_triangle[i * 3 + 13] = curve_points[j * 3 + (8 * 3) + 1];
                surface_triangle[i * 3 + 14] = curve_points[j * 3 + (8 * 3) + 2];
                vec3 vector5(surface_triangle[i * 3 + 12], surface_triangle[i * 3 + 13], surface_triangle[i * 3 + 14]);

                surface_triangle[i * 3 + 15] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 16] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 17] = curve_points[j * 3 + (1 * 3) + 2];
                vec3 vector6(surface_triangle[i * 3 + 15], surface_triangle[i * 3 + 16], surface_triangle[i * 3 + 17]);

                vec3 normalised2 = calculateNormal(vector5, vector4, vector6);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle2 = calculateLightAngle(lightDirection, normalised2);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity2 = calculateColorIntensity(lightColor, lightIntensity, lightAngle2);
                surface_triangle_colors[i * 3 + 9] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 10] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 11] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 12] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 13] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 14] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 15] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 16] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 17] = colorIntensity2.v[2];

                j += 1;

            }
        }
    }
    if (sor == 5) {
        for (int i = 0; i < 426; i += 6) {

            if (i == 48 || i == 102 || i == 156 || i == 210 || i == 264 || i == 318 || i == 372 || i == 426) {
                j += 1;
            }
            else {
                surface_triangle[i * 3] = curve_points[j * 3];
                surface_triangle[i * 3 + 1] = curve_points[j * 3 + 1];
                surface_triangle[i * 3 + 2] = curve_points[j * 3 + 2];
                vec3 vector1(surface_triangle[i * 3], surface_triangle[i * 3 + 1], surface_triangle[i * 3 + 2]);

                surface_triangle[i * 3 + 3] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 4] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 5] = curve_points[j * 3 + (1 * 3) + 2];
                vec3 vector2(surface_triangle[i * 3 + 3], surface_triangle[i * 3 + 4], surface_triangle[i * 3 + 5]);

                surface_triangle[i * 3 + 6] = curve_points[j * 3 + (9 * 3)];
                surface_triangle[i * 3 + 7] = curve_points[j * 3 + (9 * 3) + 1];
                surface_triangle[i * 3 + 8] = curve_points[j * 3 + (9 * 3) + 2];
                vec3 vector3(surface_triangle[i * 3 + 6], surface_triangle[i * 3 + 7], surface_triangle[i * 3 + 8]);

                vec3 normalised = calculateNormal(vector1, vector2, vector3);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle = calculateLightAngle(lightDirection, normalised);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity = calculateColorIntensity(lightColor, lightIntensity, lightAngle);
                surface_triangle_colors[i * 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 1] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 2] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 4] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 5] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 6] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 7] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 8] = colorIntensity.v[2];


                // alsó háromszögek

                surface_triangle[i * 3 + 9] = curve_points[j * 3 + (9 * 3)];
                surface_triangle[i * 3 + 10] = curve_points[j * 3 + (9 * 3) + 1];
                surface_triangle[i * 3 + 11] = curve_points[j * 3 + (9 * 3) + 2];
                vec3 vector4(surface_triangle[i * 3 + 9], surface_triangle[i * 3 + 10], surface_triangle[i * 3 + 11]);

                surface_triangle[i * 3 + 12] = curve_points[j * 3 + (10 * 3)];
                surface_triangle[i * 3 + 13] = curve_points[j * 3 + (10 * 3) + 1];
                surface_triangle[i * 3 + 14] = curve_points[j * 3 + (10 * 3) + 2];
                vec3 vector5(surface_triangle[i * 3 + 12], surface_triangle[i * 3 + 13], surface_triangle[i * 3 + 14]);

                surface_triangle[i * 3 + 15] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 16] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 17] = curve_points[j * 3 + (1 * 3) + 2];
                vec3 vector6(surface_triangle[i * 3 + 15], surface_triangle[i * 3 + 16], surface_triangle[i * 3 + 17]);

                vec3 normalised2 = calculateNormal(vector5, vector4, vector6);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle2 = calculateLightAngle(lightDirection, normalised2);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity2 = calculateColorIntensity(lightColor, lightIntensity, lightAngle2);
                surface_triangle_colors[i * 3 + 9] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 10] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 11] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 12] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 13] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 14] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 15] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 16] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 17] = colorIntensity2.v[2];

                j += 1;

            }
        }
    }
    if (sor == 6) {

        for (int i = 0; i < 654; i += 6) {

            if (i == 60 || i == 126 || i == 192 || i == 258 || i == 324 || i == 390 || i == 456 || i == 522 || i == 588 || i == 654) {
                j += 1;
            }
            else {
                surface_triangle[i * 3] = curve_points[j * 3];
                surface_triangle[i * 3 + 1] = curve_points[j * 3 + 1];
                surface_triangle[i * 3 + 2] = curve_points[j * 3 + 2];
                vec3 vector1(surface_triangle[i * 3], surface_triangle[i * 3 + 1], surface_triangle[i * 3 + 2]);

                surface_triangle[i * 3 + 3] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 4] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 5] = curve_points[j * 3 + (1 * 3) + 2];
                vec3 vector2(surface_triangle[i * 3 + 3], surface_triangle[i * 3 + 4], surface_triangle[i * 3 + 5]);

                surface_triangle[i * 3 + 6] = curve_points[j * 3 + (11 * 3)];
                surface_triangle[i * 3 + 7] = curve_points[j * 3 + (11 * 3) + 1];
                surface_triangle[i * 3 + 8] = curve_points[j * 3 + (11 * 3) + 2];
                vec3 vector3(surface_triangle[i * 3 + 6], surface_triangle[i * 3 + 7], surface_triangle[i * 3 + 8]);

                vec3 normalised = calculateNormal(vector1, vector2, vector3);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle = calculateLightAngle(lightDirection, normalised);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity = calculateColorIntensity(lightColor, lightIntensity, lightAngle);
                surface_triangle_colors[i * 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 1] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 2] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 3] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 4] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 5] = colorIntensity.v[2];
                surface_triangle_colors[i * 3 + 6] = colorIntensity.v[0];
                surface_triangle_colors[i * 3 + 7] = colorIntensity.v[1];
                surface_triangle_colors[i * 3 + 8] = colorIntensity.v[2];

                // alsó háromszögek

                surface_triangle[i * 3 + 9] = curve_points[j * 3 + (11 * 3)];
                surface_triangle[i * 3 + 10] = curve_points[j * 3 + (11 * 3) + 1];
                surface_triangle[i * 3 + 11] = curve_points[j * 3 + (11 * 3) + 2];
                vec3 vector4(surface_triangle[i * 3 + 9], surface_triangle[i * 3 + 10], surface_triangle[i * 3 + 11]);

                surface_triangle[i * 3 + 12] = curve_points[j * 3 + (12 * 3)];
                surface_triangle[i * 3 + 13] = curve_points[j * 3 + (12 * 3) + 1];
                surface_triangle[i * 3 + 14] = curve_points[j * 3 + (12 * 3) + 2];
                vec3 vector5(surface_triangle[i * 3 + 12], surface_triangle[i * 3 + 13], surface_triangle[i * 3 + 14]);

                surface_triangle[i * 3 + 15] = curve_points[j * 3 + (1 * 3)];
                surface_triangle[i * 3 + 16] = curve_points[j * 3 + (1 * 3) + 1];
                surface_triangle[i * 3 + 17] = curve_points[j * 3 + (1 * 3) + 2];

                vec3 vector6(surface_triangle[i * 3 + 15], surface_triangle[i * 3 + 16], surface_triangle[i * 3 + 17]);

                vec3 normalised2 = calculateNormal(vector5, vector4, vector6);

                // Fényforrás és normálvektor közötti szög számolása
                float lightAngle2 = calculateLightAngle(lightDirection, normalised2);
                // Szín intenzitásának kiszámolása
                vec3 colorIntensity2 = calculateColorIntensity(lightColor, lightIntensity, lightAngle2);
                surface_triangle_colors[i * 3 + 9] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 10] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 11] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 12] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 13] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 14] = colorIntensity2.v[2];
                surface_triangle_colors[i * 3 + 15] = colorIntensity2.v[0];
                surface_triangle_colors[i * 3 + 16] = colorIntensity2.v[1];
                surface_triangle_colors[i * 3 + 17] = colorIntensity2.v[2];
                j += 1;

            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glBufferData(GL_ARRAY_BUFFER, 50000 * sizeof(GLfloat), surface_triangle, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, triangle_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 50000 * sizeof(GLfloat), surface_triangle_colors, GL_STATIC_DRAW);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (lightSourceMove) {
        lightDirectionArray[0 + selectedAxis] += 0.015 * yoffset;
        lightDirection = { lightDirectionArray[0], lightDirectionArray[1], lightDirectionArray[2] };

        glBindBuffer(GL_ARRAY_BUFFER, vbo_lightSource);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), lightDirectionArray, GL_STATIC_DRAW);
    }

    if (!lightSourceMove) {
        points[currentPoint * 3 + selectedAxis] += 0.03f * yoffset;
        points2[secondcurrentPoint * 3 + selectedAxis] += 0.03f * yoffset;

        // pontok újraszámolása
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sor * oszlop * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo2);
        glBufferData(GL_ARRAY_BUFFER, oszlop * sor * 3 * sizeof(GLfloat), points2, GL_STATIC_DRAW);

        // bezier felület újraszámolása
        bezierGenerator(curve_points, surface_color, points);
        glBindBuffer(GL_ARRAY_BUFFER, vbo3);
        glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), curve_points, GL_STATIC_DRAW);

        // bezier felület újraszámolása
        bezierGenerator(curve_points2, surface_color, points2);
        glBindBuffer(GL_ARRAY_BUFFER, vbo4);
        glBufferData(GL_ARRAY_BUFFER, 3030 * sizeof(GLfloat), curve_points2, GL_STATIC_DRAW);
    }

    // háromszögelés újraszámolása
    triangulization(curve_points);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightSourceMove = !lightSourceMove;
    }
    else {
        if (!lightSourceMove) {
            if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS) {
                if (currentPoint < sor * oszlop) {
                    currentPoint++;
                }
                else {
                    currentPoint = 0;
                }
                int i = currentPoint;
                for (int j = 0; j < oszlop * sor; j++)
                {
                    if ((points[i * 3] == points2[j * 3]) && (points[i * 3 + 1] == points2[j * 3 + 1]) && (points[i * 3 + 2] == points2[j * 3 + 2]))
                    {
                        secondcurrentPoint = j;
                        break;
                    }
                }
            }
        }
        //cout << endl << currentPoint << " " << secondcurrentPoint;
        if (key == GLFW_KEY_X && action == GLFW_PRESS) {
            selectedAxis = 0;
        }
        if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            selectedAxis = 1;
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            selectedAxis = 2;
        }
    }
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int main() {


    // Create application window
//ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example", WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);




    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            static int sor_oszlop = 5;

            ImGui::Begin("Hello Grafika!");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("Choose all parameters wisely summoner!\n You won't have the possibility to do it later!!");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);



            ImGui::SliderInt("Sor X Oszlop", &sor_oszlop, 4, 6);
            ImGui::Checkbox("Show grid:", &grid);
            ImGui::Checkbox("Show triangles:", &triangles);

            oszlop = sor_oszlop;
            sor = sor_oszlop;
            oszlopEgyseg = 1 / ((float)(oszlop - 1) * 2);
            sorEgyseg = 1 / ((float)(sor - 1) * 2);


            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

         /*   if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);*/


            //

            //

            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    //



    const GLubyte* renderer;
    const GLubyte* version;

    start_gl();

    // PONT GENERÁLÁSOK

    for (int j = 0; j < sor; j++) {
        for (int i = 0; i < oszlop; i++)
        {
            points_color[k * 3] = 1.0f;
            points_color[k * 3 + 1] = 0.5f;
            points_color[k * 3 + 2] = 0.0f;

            points[k * 3] = -0.5f + (i * step);
            points[k * 3 + 1] = 0.0f + (1.0f / sor) + (j * 0.2f);
            points[k * 3 + 2] = i < oszlop / 2 ? gorbulet * i : -gorbulet * i;
            if (j < sor / 2)
                points[k * 3 + 2] += gorbulet * j;
            else
                points[k * 3 + 2] -= gorbulet * j;

            k = k + 1;
        }
    }

    for (int j = 0; j <= (oszlop * 3); j += 3) {
        for (int i = 0; i < (oszlop * 3 * sor); i += (oszlop * 3)) {
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
    triangulization(curve_points);


    // SHADERS

    // fényforrás
    glGenBuffers(1, &vbo_lightSource);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_lightSource);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), lightDirectionArray, GL_STATIC_DRAW);

    glGenBuffers(1, &lightSource_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lightSource_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), lightColorArray, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao6);
    glBindVertexArray(vao6);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_lightSource);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, lightSource_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // háromszögelés
    glGenBuffers(1, &vbo_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glBufferData(GL_ARRAY_BUFFER, 50000 * sizeof(GLfloat), surface_triangle, GL_STATIC_DRAW);

    glGenBuffers(1, &triangle_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 50000 * sizeof(GLfloat), surface_triangle_colors, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao5);
    glBindVertexArray(vao5);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // points 1
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, oszlop * sor * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glGenBuffers(1, &points_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, oszlop * sor * 3 * sizeof(GLfloat), points_color, GL_STATIC_DRAW);

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
    glBufferData(GL_ARRAY_BUFFER, oszlop * sor * 3 * sizeof(GLfloat), points2, GL_STATIC_DRAW);

    glGenBuffers(1, &points_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, oszlop * sor * 3 * sizeof(GLfloat), points_color, GL_STATIC_DRAW);

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
    float mynear = 0.1f;                                   // clipping plane
    float myfar = 100.0f;                                 // clipping plane
    float fov = 67.0f * ONE_DEG_IN_RAD;                 // convert 67 degrees to radians
    float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
    // matrix components
    float inverse_range = 1.0f / tan(fov * 0.5f);
    float Sx = inverse_range / aspect;
    float Sy = inverse_range;
    float Sz = -(myfar + mynear) / (myfar - mynear);
    float Pz = -(2.0f * myfar * mynear) / (myfar - mynear);
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

        glfwSetKeyCallback(g_window, key_callback);
        glfwSetScrollCallback(g_window, scroll_callback);

        static double previous_seconds = glfwGetTime();
        double current_seconds = glfwGetTime();
        double elapsed_seconds = current_seconds - previous_seconds;
        previous_seconds = current_seconds;

        // wipe the drawing surface clear
        glViewport(0, 0, g_gl_width, g_gl_height);

        _update_fps_counter(g_window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // ----- axis ------//
        /*
        glUseProgram(shader_programme_axis);
        // tengelyek kirajz
        glBindVertexArray(axes_vao);
        glDrawArrays(GL_LINES, 0, 6);
        */
        // --------------- //

        glUseProgram(shader_programme);

        glPointSize(10.0f);

        // POINTS 1
        glBindVertexArray(vao);
        //glDrawArrays(GL_POINTS, 0, x * y);
        if (grid) {
            for (int i = 0; i <= sor; i++) {
                glDrawArrays(GL_LINE_STRIP, i * oszlop, oszlop);
            }

            glDrawArrays(GL_POINTS, currentPoint, 1);

            // POINTS 2
            glBindVertexArray(vao2);
            //glDrawArrays(GL_POINTS, 0, x * y);
            for (int i = 0; i <= oszlop; i++) {
                glDrawArrays(GL_LINE_STRIP, i * sor, sor);
            }


        }
        glPointSize(3.0f);
        // BEZIER surface egyik iranyba kirajzolas
        glUseProgram(shader_programme);
        glBindVertexArray(vao3);
        //glDrawArrays(GL_POINTS, 0, 1010);
        for (int i = 0; i < (1 / oszlopEgyseg) + 1; i++) {
            glDrawArrays(GL_LINE_STRIP, i * (1 / oszlopEgyseg + 1), 1 / oszlopEgyseg + 1);
        }


        // BEZIER surface masik iranyba kirajzolas
        glUseProgram(shader_programme);
        glBindVertexArray(vao4);
        if (sor == 4) {
            glDrawArrays(GL_POINTS, 0, 49);
        }
        if (sor == 5) {
            glDrawArrays(GL_POINTS, 0, 81);
        }
        if (sor == 6) {
            glDrawArrays(GL_POINTS, 0, 121);
        }
        for (int i = 0; i < (1 / oszlopEgyseg) + 1; i++) {
            glDrawArrays(GL_LINE_STRIP, i * (1 / oszlopEgyseg + 1), 1 / oszlopEgyseg + 1);
        }

        // háromszögek
        glBindVertexArray(vao5);
        if (triangles) {
            glDrawArrays(GL_TRIANGLES, 0, 700);
        }

        // fényforrás
        glBindVertexArray(vao6);
        glPointSize(20.0f);
        glDrawArrays(GL_POINTS, 0, 1);


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
        if (glfwGetKey(g_window, GLFW_KEY_UP)) {
            cam_pos[1] += cam_speed * elapsed_seconds;
            cam_moved = true;
        }
        if (glfwGetKey(g_window, GLFW_KEY_DOWN)) {
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
