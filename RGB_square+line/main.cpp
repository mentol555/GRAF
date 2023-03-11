#include <GL/glew.h>    /* include GLEW and new version of GL on Windows */
#include <GLFW/glfw3.h> /* GLFW helper library */
#include <stdio.h>
#include "gl_utils.h"
#include <iostream>

int main() {
    GLFWwindow* window = NULL;
    const GLubyte* renderer;
    const GLubyte* version;
    GLuint vao;
    GLuint vbo;
    GLuint color_vbo;

    /* geometry to use. these are 3 xyz points (9 floats total) to make a triangle */



    GLfloat points[] = {
       -0.5f,  -0.5f, 0.0f,
       -0.5f, 0.5f, 0.0f,   
        0.5f, -0.5f, 0.0f,   
        0.5f,  0.5f, 0.0f,   
        -0.5f, 0.5f, 0.0f,   
        0.5f, -0.5f, 0.0f,     
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    GLfloat szakasz[] = { 
        -0.7f, -0.5f, 0.0f,
        0.7f, 0.5f, 0.0f
    };


    /* start GL context and O/S window using the GLFW helper library */
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    /* Version 4.1 Core is a good default that should run on just about everything. Adjust later to suit project requirements. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    /* start GLEW extension handler */
    glewExperimental = GL_TRUE;
    glewInit();

    /* get version info */
    renderer = glGetString(GL_RENDERER); /* get renderer string */
    version = glGetString(GL_VERSION);  /* version as a string */
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    /* tell GL to only draw onto a pixel if the shape is closer to the viewer
    than anything already drawn at that pixel */
    glEnable(GL_DEPTH_TEST); /* enable depth-testing */
    /* with LESS depth-testing interprets a smaller depth value as meaning "closer" */
    glDepthFunc(GL_LESS);

    /* a vertex buffer object (VBO) is created here. this stores an array of
    data on the graphics adapter's memory. in our case - the vertex points */
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), colors, GL_STATIC_DRAW);



    /* the vertex array object (VAO) is a little descriptor that defines which
    data from vertex buffer objects should be used as input variables to vertex
    shaders. in our case - use our only VBO, and say 'every three floats is a
    variable' */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    /* "attribute #0 should be enabled when this vao is bound" */
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //0 a koordinatak -- 6-onkent ugralunk

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //1 a szin -- 6-onkent ugralunk (utolso arg: 3-tol)

    // Szakaszhoz
    GLuint vao2;
    GLuint vbo2;

    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), szakasz, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // shader rész
    char vertex_shader[1024 * 256];
    char fragment_shader[1024 * 256];
    parse_file_into_str("vs.glsl", vertex_shader, 1024 * 256);
    parse_file_into_str("fs.glsl", fragment_shader, 1024 * 256);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* p = (const GLchar*)vertex_shader;
    glShaderSource(vs, 1, &p, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    p = (const GLchar*)fragment_shader;
    glShaderSource(fs, 1, &p, NULL);
    glCompileShader(fs);

    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    /* this loop clears the drawing surface, then draws the geometry described
        by the VAO onto the drawing surface. we 'poll events' to see if the window
    was closed, etc. finally, we 'swap the buffers' which displays our drawing
        surface onto the view area. we use a double-buffering system which means
        that we have a 'currently displayed' surface, and 'currently being drawn'
        surface. hence the 'swap' idea. in a single-buffering system we would see
        stuff being drawn one-after-the-other */
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        /* wipe the drawing surface clear */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_programme);
        // kirajzoljuk a szakaszt
        glBindVertexArray(vao2);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        // kirajzoljuk a negyzetet
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /* update other events like input handling */
        glfwPollEvents();
        /* put the stuff we've been drawing onto the display */
        glfwSwapBuffers(window);
    }

    /* close GL context and any other GLFW resources */
    glfwTerminate();
    return 0;
}
