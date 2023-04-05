#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>

#define WIDTH 640
#define HEIGHT 640

#define N 100

using namespace std;

GLuint vao;
GLuint pointsvao;
GLuint vbo;
GLuint pointsvbo;

GLfloat HermiteCurve[4 * N];

GLint dragged = -1; // tarolja h melyik pontot fogtuk meg

GLfloat t;
GLint i;

GLfloat step = 3.0f / (N - 1); // t \in [-1,2]

GLfloat Points[] = {
        -0.75f, 0.0f, 0.0f,
        -0.5f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.80, 0.5f, 0.0f
};


// tavolsag negyzet kiszamolas
GLfloat dist2_2d(GLfloat P1x, GLfloat P1y, GLfloat P2x, GLfloat P2y) {
    GLfloat dx = P1x - P2x;
    GLfloat dy = P1y - P2y;
    return dx * dx + dy * dy;
}

// i -> melyik pontra mutatok ra
GLint getActivePoint(GLfloat* p, GLfloat sensitivity, GLfloat x, GLfloat y) {

    GLfloat		s = sensitivity * sensitivity; // sugar negyzet
    GLfloat		xNorm = -1 + x / (WIDTH / 2); // eger pozicio normalizalasa: 640x640-esbol [-1,1] rendszerbe
    GLfloat		yNorm = -1 + (HEIGHT - y) / (HEIGHT / 2);

    for (GLint i = 0; i < 4; i++)
        if (dist2_2d(p[i * 3], p[i * 3 + 1], xNorm, yNorm) < s) // itt szamoljuk a tav.negyzetet a pont ket koorinatajat(p[i*3], p[i*3+1] es a kurzor koordinataja kozott, es a sugarnegyzettel osszehasonl.)
            return i;

    return -1;
}
// 
void cursorPosCallback(GLFWwindow* window, double x, double y) {
    // ha valamelyik pontot mozgatjuk, azaz valamelyiket drageljuk, 
    // az eger kurzor pontjaival felulirom az akt. pontom koordinatajat
    if (dragged >= 0) {
        //std::cout << "Dragged!, cursor coords: x:" << x << "y:" << y<< endl;
        GLfloat		xNorm = -1 + x / (WIDTH / 2);
        GLfloat		yNorm = -1 + (HEIGHT - y) / (HEIGHT / 2);

        Points[3 * dragged] = xNorm;  // x coord
        Points[3 * dragged + 1] = yNorm;  // y coord

        HermiteCurve[0] = Points[0];
        HermiteCurve[1] = Points[1];
        HermiteCurve[2] = 0.0f;

        for (i = 1; i < (N - 1); i++) {
            t = -1 + i * step;
            HermiteCurve[i * 3] = Points[0] * (-1.0f / 6.0f * t * t * t + 1.0f / 2.0f * t * t - 1.0f / 3.0f * t) + Points[3] * (1.0f / 2.0f * t * t * t - t * t - 1.0f / 2.0f * t + 1) + Points[6] * (-1.0f / 2.0f * t * t * t + 1.0f / 2.0f * t * t + t) + Points[9] * (1.0f / 6.0f * t * t * t - 1.0f / 6.0f * t);
            HermiteCurve[i * 3 + 1] = Points[1] * (-1.0f / 6.0f * t * t * t + 1.0f / 2.0f * t * t - 1.0f / 3.0f * t) + Points[4] * (1.0f / 2.0f * t * t * t - t * t - 1.0f / 2.0f * t + 1) + Points[7] * (-1.0f / 2.0f * t * t * t + 1.0f / 2.0f * t * t + t) + Points[10] * (1.0f / 6.0f * t * t * t - 1.0f / 6.0f * t);
            HermiteCurve[i * 3 + 2] = 0.0f;

        }

        HermiteCurve[(3 * N) - 3] = Points[9];
        HermiteCurve[(3 * N) - 2] = Points[10];
        HermiteCurve[(3 * N) - 1] = 0.0f;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, pointsvbo);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), Points, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);



    }
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double	x, y;

        glfwGetCursorPos(window, &x, &y);
        dragged = getActivePoint(Points, 0.1f, x, y);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        dragged = -1;
}
int main() {
    GLFWwindow* window = NULL;
    const GLubyte* renderer;
    const GLubyte* version;

    //hermit iv
    const char* vertex_shader1 =
        "#version 410\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";
    // pontok
    const char* vertex_shader2 =
        "#version 410\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";
    // osszekotes
    const char* vertex_shader3 =
        "#version 410\n"
        "in vec3 vp;"
        "void main () {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";
    //hermit iv
    const char* fragment_shader1 =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0, 0.0, 1.0, 1.0);"
        "}";
    // pontok
    const char* fragment_shader2 =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0, 1.0, 0.0, 1.0);"
        "}";
    // osszekotes
    const char* fragment_shader3 =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0, 0.0, 0.0, 1.0);"
        "}";

    GLuint vert_shader1, frag_shader1, vert_shader2, frag_shader2, vert_shader3, frag_shader3;
    GLuint shader_programme1;
    GLuint shader_programme2;
    GLuint shader_programme3;

    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Drag&Drop", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // beallitjuk a fuggvenyunket mint callback fuggvenyt, ami kezeli az esemenyeket
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);


    HermiteCurve[0] = Points[0];
    HermiteCurve[1] = Points[1];
    HermiteCurve[2] = 0.0f;

    for (i = 1; i < (N - 1); i++) {
        t = -1 + i * step;
        HermiteCurve[i * 3] = Points[0] * (-1.0f / 6.0f * t * t * t + 1.0f / 2.0f * t * t - 1.0f / 3.0f * t) + Points[3] * (1.0f / 2.0f * t * t * t - t * t - 1.0f / 2.0f * t + 1) + Points[6] * (-1.0f / 2.0f * t * t * t + 1.0f / 2.0f * t * t + t) + Points[9] * (1.0f / 6.0f * t * t * t - 1.0f / 6.0f * t);
        HermiteCurve[i * 3 + 1] = Points[1] * (-1.0f / 6.0f * t * t * t + 1.0f / 2.0f * t * t - 1.0f / 3.0f * t) + Points[4] * (1.0f / 2.0f * t * t * t - t * t - 1.0f / 2.0f * t + 1) + Points[7] * (-1.0f / 2.0f * t * t * t + 1.0f / 2.0f * t * t + t) + Points[10] * (1.0f / 6.0f * t * t * t - 1.0f / 6.0f * t);
        HermiteCurve[i * 3 + 2] = 0.0f;

    }

    HermiteCurve[(3 * N) - 3] = Points[9];
    HermiteCurve[(3 * N) - 2] = Points[10];
    HermiteCurve[(3 * N) - 1] = 0.0f;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve, GL_STATIC_DRAW);

    glGenBuffers(1, &pointsvbo);
    glBindBuffer(GL_ARRAY_BUFFER, pointsvbo);
    glBufferData(GL_ARRAY_BUFFER, (12) * sizeof(GLfloat), Points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glGenVertexArrays(1, &pointsvao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    glBindVertexArray(pointsvao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, pointsvbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    vert_shader1 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader1, 1, &vertex_shader1, NULL);
    glCompileShader(vert_shader1);

    frag_shader1 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader1, 1, &fragment_shader1, NULL);
    glCompileShader(frag_shader1);

    vert_shader2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader2, 1, &vertex_shader2, NULL);
    glCompileShader(vert_shader2);

    frag_shader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader2, 1, &fragment_shader2, NULL);
    glCompileShader(frag_shader2);

    vert_shader3 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader3, 1, &vertex_shader3, NULL);
    glCompileShader(vert_shader3);

    frag_shader3 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader3, 1, &fragment_shader3, NULL);
    glCompileShader(frag_shader3);

    shader_programme1 = glCreateProgram();
    glAttachShader(shader_programme1, frag_shader1);
    glAttachShader(shader_programme1, vert_shader1);
    glLinkProgram(shader_programme1);

    shader_programme2 = glCreateProgram();
    glAttachShader(shader_programme2, frag_shader2);
    glAttachShader(shader_programme2, vert_shader2);
    glLinkProgram(shader_programme2);

    shader_programme3 = glCreateProgram();
    glAttachShader(shader_programme3, frag_shader3);
    glAttachShader(shader_programme3, vert_shader3);
    glLinkProgram(shader_programme3);

    GLuint vert_shader, frag_shader;
    GLuint shader_programme;

    
    glPointSize(15.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // pontok
        glUseProgram(shader_programme2);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_POINTS, 0, 4);

        // hermite
        glUseProgram(shader_programme1);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_STRIP, 0, N);

        // osszekotes
        glUseProgram(shader_programme3);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_LINE_STRIP, 0, 4);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
