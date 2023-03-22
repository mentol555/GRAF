#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int main() {

  GLFWwindow* window = NULL;
  const GLubyte* renderer;
  const GLubyte* version;
  
  GLuint VBO[2];
  GLuint VAO[2];

  GLfloat points1[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
  GLfloat points2[] = { 0.5f, 0.5f, 0.0f, 1.0f,  0.5f, 0.0f, 0.75f,  1.0f, 0.0f };
  
  const char* vertex_shader =
    "#version 330\n"
    "in vec3 vp;"
    "void main () {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";
    
  const char* fragment_shader =
    "#version 330\n"
    "out vec4 frag_colour;"
    "void main () {"
    "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

  GLuint vert_shader, frag_shader;

  GLuint shader_programme;

  if ( !glfwInit() ) {
    fprintf( stderr, "ERROR: could not start GLFW3\n" );
    return 1;
  }

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

  window = glfwCreateWindow( 640, 480, "Hello Triangles", NULL, NULL );
  if ( !window ) {
    fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent( window );

  glewExperimental = GL_TRUE;
  glewInit();

  renderer = glGetString( GL_RENDERER );
  version  = glGetString( GL_VERSION );
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );

  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );

  glGenBuffers( numVBOs, VBO );
    
  glBindBuffer( GL_ARRAY_BUFFER, VBO[0] );
  glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points1, GL_STATIC_DRAW );
  
  glBindBuffer( GL_ARRAY_BUFFER, VBO[1] );
  glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points2, GL_STATIC_DRAW );
  
  glGenVertexArrays(numVAOs, VAO);
  
  glBindVertexArray( VAO[0] );
  glEnableVertexAttribArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, VBO[0] );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  
  glBindVertexArray( VAO[1] );
  glEnableVertexAttribArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, VBO[1] );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

  vert_shader = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vert_shader, 1, &vertex_shader, NULL );
  glCompileShader( vert_shader );
  
  frag_shader = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( frag_shader, 1, &fragment_shader, NULL );
  glCompileShader( frag_shader );
  
  shader_programme = glCreateProgram();
  glAttachShader( shader_programme, frag_shader );
  glAttachShader( shader_programme, vert_shader );
  glLinkProgram( shader_programme );

  while ( !glfwWindowShouldClose( window ) ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram( shader_programme );
    glBindVertexArray( VAO[0] );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glBindVertexArray( VAO[1] );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glfwPollEvents();
    glfwSwapBuffers( window );
  }

  glfwTerminate();
  return 0;
}
