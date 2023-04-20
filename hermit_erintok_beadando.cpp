#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>

#define WIDTH 960
#define HEIGHT 960

#define N 100

using namespace std;

GLuint vao1, vao2, vao3;
GLuint pointsvao;
GLuint vbo1, vbo2, vbo3; // Hermit curves vbos
GLuint pointsvbo;

GLfloat HermiteCurve[4 * N];
GLfloat HermiteCurve2[4 * N]; // masodik hermit gorbe
GLfloat HermiteCurve3[4 * N]; // harmadik gorbe

GLint dragged = -1; // tarolja h melyik pontot fogtuk meg

GLfloat t;
GLint i;

GLfloat step = 1.0f / (N - 1); // t in [0,1]

GLfloat Points[] = {
        -0.75f, 0.0f, 0.0f, // elso pontja
        -0.5f, 0.5f, 0.0f,  // elso pontja
        -0.25f, 0.0f, 0.0f, // elso es masodik kozos pontja
        -0.20, -0.5f, 0.0f,  // elso es masodik kozos pontja
        0.20f, 0.25f, 0.0f, // masodik es harmadik kozos pontja
        0.25f, 0.0f, 0.0f, // masodik es harmadik kozos pontja
        0.5f, -0.15f, 0.0f, // harmadik pontja
        0.55f, 0.3f, 0.0f // harmadik pontja
};
GLfloat Vectors[] = {
    // R1(x,y) = P2x - P1x, P2y - P1y
    Points[3*1] - Points[3*0], Points[3*1+1] - Points[3*0+1], 0.0f, // elso erintoje
    // R2(x,y) = P4x - P3x, P4y - P3y
    Points[3*3] - Points[3*2], Points[3*3+1] - Points[3*2+1], 0.0f, // elso es masodik kozos erintoja
    Points[3*5] - Points[3*4], Points[3*5+1] - Points[3*4+1], 0.0f, // masodik es harmadik kozos erintoje
    Points[3*7] - Points[3*6], Points[3*7+1] - Points[3*6+1], 0.0f // harmadik erintoje
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

    for (GLint i = 0; i < 8; i++)
        if (dist2_2d(p[i * 3], p[i * 3 + 1], xNorm, yNorm) < s) // itt szamoljuk a tav.negyzetet a pont ket koorinatajat(p[i*3], p[i*3+1] es a kurzor koordinataja kozott, es a sugarnegyzettel osszehasonl.)
            return i;

    return -1;
}
void calcFirst() {
    // elso gorbe ujraszamolasa, P1 pontbol indul a gorbe
    HermiteCurve[0] = Points[3*0];
    HermiteCurve[1] = Points[3*0+1];
    HermiteCurve[2] = 0.0f;
    for (i = 1; i < (N - 1); i++) {
        t = 0 + i * step;
        // P1x * (...) + P3x * (...) + R1x * (...) + R2x * (...)
        // P1 és P3 pontokon megy át a görbe, P2 és P4 csak az érintővektorok számolásához vannak
        HermiteCurve[i * 3] = Points[3*0] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3*2] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3*0] * (t * t * t - 2.0f * t * t + t) + Vectors[3*1] * (t * t * t - t * t);
        HermiteCurve[i * 3 + 1] = Points[3*0+1] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3*2+1] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3*0+1] * (t * t * t - 2.0f * t * t + t) + Vectors[3*1+1] * (t * t * t - t * t);
        HermiteCurve[i * 3 + 2] = 0.0f;
    }
    // P3 pontba vegzodik a gorbe
    HermiteCurve[(3 * N) - 3] = Points[3*2];
    HermiteCurve[(3 * N) - 2] = Points[3*2+1];
    HermiteCurve[(3 * N) - 1] = 0.0f;
}
void calcSecond() {
    // masodik gorbe ujraszamolasa, P3 pontbol indul a gorbe
    HermiteCurve2[0] = Points[3*2];
    HermiteCurve2[1] = Points[3*2+1];
    HermiteCurve2[2] = 0.0f;
    for (i = 1; i < (N - 1); i++) {
        t = 0 + i * step;
        // P3x * (...) + P5x * (...) + R2x * (...) + R3x * (...)
        // curve 2: P3 es P5 pontokon megy at, P4 es P6 az erintok szamolasahoz vannak
        HermiteCurve2[i * 3] = Points[3*2] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3*4] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3*1] * (t * t * t - 2.0f * t * t + t) + Vectors[3*2] * (t * t * t - t * t);
        HermiteCurve2[i * 3 + 1] = Points[3*2+1] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3*4+1] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3*1+1] * (t * t * t - 2.0f * t * t + t) + Vectors[3*2+1] * (t * t * t - t * t);
        HermiteCurve2[i * 3 + 2] = 0.0f;
    }
    // P5 pontba vegzodik a gorbe
    HermiteCurve2[(3 * N) - 3] = Points[3*4];
    HermiteCurve2[(3 * N) - 2] = Points[3*4+1];
    HermiteCurve2[(3 * N) - 1] = 0.0f;
}
void calcThird() {
    // masodik gorbe ujraszamolasa, P3 pontbol indul a gorbe
    HermiteCurve3[0] = Points[3 * 4];
    HermiteCurve3[1] = Points[3 * 4 + 1];
    HermiteCurve3[2] = 0.0f;
    for (i = 1; i < (N - 1); i++) {
        t = 0 + i * step;
        // P3x * (...) + P5x * (...) + R2x * (...) + R3x * (...)
        // curve 2: P3 es P5 pontokon megy at, P4 es P6 az erintok szamolasahoz vannak
        HermiteCurve3[i * 3] = Points[3 * 4] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3 * 6] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3 * 2] * (t * t * t - 2.0f * t * t + t) + Vectors[3 * 3] * (t * t * t - t * t);
        HermiteCurve3[i * 3 + 1] = Points[3 * 4 + 1] * (2.0f * t * t * t - 3.0f * t * t + 1) + Points[3 * 6 + 1] * (-2.0f * t * t * t + 3.0f * t * t) + Vectors[3 * 2 + 1] * (t * t * t - 2.0f * t * t + t) + Vectors[3 * 3 + 1] * (t * t * t - t * t);
        HermiteCurve3[i * 3 + 2] = 0.0f;
    }
    // P5 pontba vegzodik a gorbe
    HermiteCurve3[(3 * N) - 3] = Points[3 * 6];
    HermiteCurve3[(3 * N) - 2] = Points[3 * 6 + 1];
    HermiteCurve3[(3 * N) - 1] = 0.0f;
}

void cursorPosCallback(GLFWwindow* window, double x, double y) {
    // ha valamelyik pontot mozgatjuk, azaz valamelyiket drageljuk, 
    // az eger kurzor pontjaival felulirom az akt. pontom koordinatajat
    if (dragged >= 0) {
        //std::cout << "Dragged!, cursor coords: x:" << x << "y:" << y<< endl;
        GLfloat		xNorm = -1 + x / (WIDTH / 2);
        GLfloat		yNorm = -1 + (HEIGHT - y) / (HEIGHT / 2);

        Points[3 * dragged] = xNorm;  // x coord
        Points[3 * dragged + 1] = yNorm;  // y coord
        
        // elso vagy masodik pontot fogjuk meg -> csak az elso gorbe valtozik
        if (dragged == 0 || dragged == 1) {
            // elso erinto v ujraszamolasa P2 - P1
            Vectors[3*0] = Points[3*1] - Points[3*0];
            Vectors[3*0+1] = Points[3*1+1] - Points[3*0+1]; 

            // elso gorbe ujraszamolasa
            calcFirst();

            // 1. vbo frissitese
            glBindBuffer(GL_ARRAY_BUFFER, vbo1);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // harmadik vagy negyedik pontot fogjuk meg
        // azaz kozos pontot fogunk meg, tehat mindket gorbe valtozik!
        if (dragged == 2 || dragged == 3) {
            // masodik erinto v ujraszamolasa P4 - P3
            Vectors[3*1] = Points[3*3] - Points[3*2]; 
            Vectors[3*1+1] = Points[3*3+1] - Points[3*2+1];

            calcFirst();
            calcSecond();

            // 1. es 2. vbo frissitese
            glBindBuffer(GL_ARRAY_BUFFER, vbo1);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ARRAY_BUFFER, vbo2);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve2, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        // otodik vagy hatodik pontot fogjuk meg -> masodik es harmadik gorbe valtozik
        if (dragged == 4 || dragged == 5) {
            // harmadik erinto v ujraszamolasa
            Vectors[3*2] = Points[3*5] - Points[3*4];
            Vectors[3*2+1] = Points[3*5+1] - Points[3*4+1];

            calcSecond();
            calcThird();

            // 2. es 3. vbo frissitese
            glBindBuffer(GL_ARRAY_BUFFER, vbo2);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve2, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ARRAY_BUFFER, vbo3);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve3, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        // hetedik vagy nyolcadik pontot fogjuk meg -> harmadik gorbe valtozik
        if (dragged == 6 || dragged == 7) {
            // negyedik erinto v ujraszamolasa
            Vectors[3*3] = Points[3*7] - Points[3*6];
            Vectors[3*3+1] = Points[3*7+1] - Points[3*6+1];

            calcThird();
            // 3. vbo frissitese
            glBindBuffer(GL_ARRAY_BUFFER, vbo3);
            glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve3, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        // points vbo frissitese
        glBindBuffer(GL_ARRAY_BUFFER, pointsvbo);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), Points, GL_STATIC_DRAW);
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

    //hermit iv, pontok, erintok ugyanaz a vs
    const char* vertex_shader1 =
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
    // erintok
    const char* fragment_shader3 =
        "#version 410\n"
        "out vec4 frag_colour;"
        "void main () {"
        "  frag_colour = vec4(1.0, 0.0, 0.0, 1.0);"
        "}";

    GLuint vert_shader1, frag_shader1, frag_shader2, frag_shader3;
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

    window = glfwCreateWindow(WIDTH, HEIGHT, "HermiteProject", NULL, NULL);
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

    // elso es masodik gorbe kiszamolasa
    calcFirst();
    calcSecond();
    calcThird();

    // Curve 1 vbo
    glGenBuffers(1, &vbo1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve, GL_STATIC_DRAW);
    // Curve 2 vbo
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve2, GL_STATIC_DRAW);
    // Curve 3 vbo
    glGenBuffers(1, &vbo3);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glBufferData(GL_ARRAY_BUFFER, (3 * N) * sizeof(GLfloat), HermiteCurve3, GL_STATIC_DRAW);

    glGenBuffers(1, &pointsvbo);
    glBindBuffer(GL_ARRAY_BUFFER, pointsvbo);
    glBufferData(GL_ARRAY_BUFFER, (24) * sizeof(GLfloat), Points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao1); // Curve 1
    glGenVertexArrays(1, &pointsvao);
    glGenVertexArrays(1, &vao2); // Curve 2
    glGenVertexArrays(1, &vao3); // Curve 3

    glBindVertexArray(vao1);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindVertexArray(vao2);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindVertexArray(vao3);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
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

    frag_shader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader2, 1, &fragment_shader2, NULL);
    glCompileShader(frag_shader2);

    frag_shader3 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader3, 1, &fragment_shader3, NULL);
    glCompileShader(frag_shader3);

    shader_programme1 = glCreateProgram();
    glAttachShader(shader_programme1, frag_shader1);
    glAttachShader(shader_programme1, vert_shader1);
    glLinkProgram(shader_programme1);

    shader_programme2 = glCreateProgram();
    glAttachShader(shader_programme2, frag_shader2);
    glAttachShader(shader_programme2, vert_shader1);
    glLinkProgram(shader_programme2);

    shader_programme3 = glCreateProgram();
    glAttachShader(shader_programme3, frag_shader3);
    glAttachShader(shader_programme3, vert_shader1);
    glLinkProgram(shader_programme3);

    GLuint vert_shader, frag_shader;
    GLuint shader_programme;


    glPointSize(15.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // pontok
        glUseProgram(shader_programme2);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_POINTS, 0, 8);

        // hermite curve 1
        glUseProgram(shader_programme1);
        glBindVertexArray(vao1);
        glDrawArrays(GL_LINE_STRIP, 0, N);

        // hermite curve 2
        glUseProgram(shader_programme1);
        glBindVertexArray(vao2);
        glDrawArrays(GL_LINE_STRIP, 0, N);

        // hermite curve 3
        glUseProgram(shader_programme1);
        glBindVertexArray(vao3);
        glDrawArrays(GL_LINE_STRIP, 0, N);

        // erintok
        glUseProgram(shader_programme3);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_LINE_STRIP, 0, 2);

        // masodik erinto
        glUseProgram(shader_programme3);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_LINE_STRIP, 2, 2); // masodik kettot

        // harmadik erinto
        glUseProgram(shader_programme3);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_LINE_STRIP, 4, 2);

        // negyedik erinto
        glUseProgram(shader_programme3);
        glBindVertexArray(pointsvao);
        glDrawArrays(GL_LINE_STRIP, 6, 2);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
