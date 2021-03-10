// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <common/shader.hpp>

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "HW1", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr,
                "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint firstProgramID = LoadShaders("TransformShader.vertexshader", "FragmentShader1.fragmentshader");
    GLuint secondProgramID = LoadShaders("TransformShader.vertexshader", "FragmentShader2.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID1 = glGetUniformLocation(firstProgramID, "MVP");
    GLuint MatrixID2 = glGetUniformLocation(secondProgramID, "MVP");

    const GLuint ProgramIDs[] = {firstProgramID, secondProgramID};
    const GLuint MatrixIDs[] = {MatrixID1, MatrixID2};

    static const GLfloat g_vertex_buffer_data[] = {
            -0.8f, -0.7f, 0.0f,
            0.8f, -0.8f, 0.0f,
            0.0f, 0.6f, 0.0f,
            -0.8f, 0.7f, 0.0f,
            0.8f, 0.8f, 0.0f,
            0.0f, -0.6f, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Projection matrix : 45 degrees Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View;
    glm::mat4 MVP;

    const float radius = 3.0f;

    do {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        float camera_x = sin(glfwGetTime()) * radius;
        float camera_z = cos(glfwGetTime()) * radius;

        // Calculate View matrix and MVP matrix
        View = glm::lookAt(
                glm::vec3(camera_x, 0, camera_z),
                glm::vec3(0, 0, 0), // and looks at the origin
                glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
        );

        MVP = Projection * View * Model;

        for (size_t i = 0; i < 2; ++i) {
            // Use our shader
            glUseProgram(ProgramIDs[i]);

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDs[i], 1, GL_FALSE, &MVP[0][0]);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                    0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void *) 0            // array buffer offset
            );

            // Draw the triangle !
            glDrawArrays(GL_TRIANGLES, 3 * i, 3); // 3 indices starting at 0 -> 1 triangle

            glDisableVertexAttribArray(0);
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteProgram(firstProgramID);
    glDeleteProgram(secondProgramID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}