#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL.h>
#include <SDL_opengl.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp" 
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

// window size & define colors
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const float BG_RED = 0.2588f, BG_BLUE = 0.522f, BG_GREEN = 0.9569f; // This is actually Google's blue
const float BG_OPACITY = 1.0f;
const float TRIANGLE_RED = 0.8588f, TRIANGLE_BLUE = 0.2667f, TRIANGLE_GREEN = 0.2157f;  // Google's red, DID NOT use
const float TRIANGLE_OPACITY = 1.0f;

// set camera
const int VIEWPORT_X = 0;
const int VIEWPORT_Y = 0;
const int VIEWPORT_WIDTH = WINDOW_WIDTH;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// define textures
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// set displayWindow & the bool
SDL_Window* display_window;
bool gameIsRunning = true;

// define ShaderProgram
ShaderProgram program;

// define improtant matrix
glm::mat4 view_matrix;
glm::mat4 model_matrix;
glm::mat4 model_matrix_mango_1;
glm::mat4 model_matrix_mango_2;
glm::mat4 model_matrix_mango_3;
glm::mat4 model_matrix_cloud;
glm::mat4 projection_matrix;

// translate & rotation & scale
float SR_x = 0.0f;
float SR_y = -0.3f;
float SR_rotate = 0.0f;
float mango_rotate = 0.0f;
float mango_1_x = -2.2f;
float mango_2_x = 0.0f;
float mango_3_x = 2.2f;
float mango_1_y = 1.0f;
float mango_2_y = 2.0f;
float mango_3_y = 1.0f;
float cloud_x = 4.0f;
float cloud_y = 2.8f;
float cloud_scale_x = 1.2f;
float cloud_scale_y = 1.0f;
float previous_ticks = 0.0f;
bool going_right = true;
bool rotating_right = true;
bool cloud_going_down = true;

// texture
const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

// sprite filepath
const char PLAYER_SPRITE_FILEPATH[] = "SR.png";
const char MANGO_SPRITE_FILEPATH[] = "mango.png";
const char CLOUD_SPRITE_FILEPATH[] = "cloud.png";

// texture
GLuint player_texture_id;
GLuint mango_id_1;
GLuint mango_id_2;
GLuint mango_id_3;
GLuint cloud_id;

GLuint load_texture(const char* filepath)
{
    // load the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // generate and binda texture ID to image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // set texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // release memory and return
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    // initiallize SDL & display window
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Mango and Rain",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // define camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    // load shader
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    // initalize matrix
    model_matrix = glm::mat4(1.0f);
    model_matrix_mango_1 = glm::mat4(1.0f);
    model_matrix_mango_2 = glm::mat4(1.0f);
    model_matrix_mango_3 = glm::mat4(1.0f);
    model_matrix_cloud = glm::mat4(1.0f);

    // defines the location and orientation of the camera
    view_matrix = glm::mat4(1.0f);
    // defines the characteristics of camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    // set view matrix & projection matrix
    program.SetViewMatrix(view_matrix);
    program.SetProjectionMatrix(projection_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // load texture
    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    mango_id_1 = load_texture(MANGO_SPRITE_FILEPATH);
    mango_id_2 = load_texture(MANGO_SPRITE_FILEPATH);
    mango_id_3 = load_texture(MANGO_SPRITE_FILEPATH);
    cloud_id = load_texture(CLOUD_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            gameIsRunning = false;
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;  // get the current number of ticks
    float delta_time = ticks - previous_ticks;       // the delta time is the difference from the last frame
    previous_ticks = ticks;                         // record current tick for next use

    // reset model matrix
    model_matrix = glm::mat4(1.0f);
    model_matrix_mango_1 = glm::mat4(1.0f);
    model_matrix_mango_2 = glm::mat4(1.0f);
    model_matrix_mango_3 = glm::mat4(1.0f);
    model_matrix_cloud = glm::mat4(1.0f);

    // SR translate
    if (going_right && SR_x >= 0.2f) {
        going_right = false;
    }
    else if (!going_right && SR_x <= -0.2f) {
        going_right = true;
    }
    if (going_right) {
        SR_x += 0.1f * delta_time;
    }
    else {
        SR_x -= 0.1f * delta_time;
    }
    // SR rotate
    if (rotating_right && SR_rotate <= -7.0f) {
        rotating_right = false;
    }
    else if (!rotating_right && SR_rotate >= 7.0f) {
        rotating_right = true;
    }
    if (rotating_right) {
        SR_rotate -= 3.5 * delta_time;
    }
    else {
        SR_rotate += 3.5 * delta_time;
    }

    // mango rotate
    mango_rotate += 90.0 * delta_time;

    // initalize scale vector
    glm::vec3 scale_vector;

    // adjust model matrix
    scale_vector = glm::vec3(6.0f,6.0f,1.0f);
    model_matrix = glm::scale(model_matrix, scale_vector);
    model_matrix = glm::translate(model_matrix, glm::vec3(SR_x, SR_y, 0.0f));
    model_matrix = glm::rotate(model_matrix, glm::radians(SR_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_mango_1 = glm::translate(model_matrix_mango_1, glm::vec3(mango_1_x, mango_1_y, 0.0f));
    model_matrix_mango_1 = glm::rotate(model_matrix_mango_1, glm::radians(mango_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    scale_vector = glm::vec3(1.0f, 1.5f, 1.0f);
    model_matrix_mango_1 = glm::scale(model_matrix_mango_1, scale_vector);
    model_matrix_mango_2 = glm::translate(model_matrix_mango_2, glm::vec3(mango_2_x, mango_2_y, 0.0f));
    model_matrix_mango_2 = glm::rotate(model_matrix_mango_2, glm::radians(mango_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_mango_2 = glm::scale(model_matrix_mango_2, scale_vector);
    model_matrix_mango_3 = glm::translate(model_matrix_mango_3, glm::vec3(mango_3_x, mango_3_y, 0.0f));
    model_matrix_mango_3 = glm::rotate(model_matrix_mango_3, glm::radians(mango_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_mango_3 = glm::scale(model_matrix_mango_3, scale_vector);
    // cloud scale & translate
    if (cloud_going_down && cloud_y <= 2.6f) {
        cloud_going_down = false;
    }
    else if (!cloud_going_down && cloud_y >= 3.0f) {
        cloud_going_down = true;
    }
    if (cloud_going_down) {
        cloud_y -= 0.2f * delta_time;
        cloud_scale_x -= 0.05f * delta_time;
        cloud_scale_y -= 0.05f * delta_time;
    }
    else {
        cloud_y += 0.2f * delta_time;
        cloud_scale_x += 0.05f * delta_time;
        cloud_scale_y += 0.05f * delta_time;
    }
    // adjust model matrix
    model_matrix_cloud = glm::translate(model_matrix_cloud, glm::vec3(cloud_x, cloud_y, 0.0f));
    scale_vector = glm::vec3(cloud_scale_x, cloud_scale_y, 1.0f);
    model_matrix_cloud = glm::scale(model_matrix_cloud, scale_vector);
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    // draw texture
    draw_object(model_matrix, player_texture_id);
    draw_object(model_matrix_mango_1, mango_id_1);
    draw_object(model_matrix_mango_2, mango_id_2);
    draw_object(model_matrix_mango_3, mango_id_3);
    draw_object(model_matrix_cloud, cloud_id);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    // swap the frame
    SDL_GL_SwapWindow(display_window);
}

void shutdown(){SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (gameIsRunning)
    {
        process_input();

        update();

        render();
    }

    shutdown();
    return 0;
}

