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
#include "cmath"
#include <ctime>
#include <stdlib.h>

#define LOG(argument) std::cout << argument << '\n'

// window size & define colors
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const float BG_RED = 0.0f, BG_BLUE = 0.0f, BG_GREEN = 0.0f; // Completely Black
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
glm::mat4 model_matrix_player_1;
glm::mat4 model_matrix_player_2;
glm::mat4 model_matrix_ball;
glm::mat4 projection_matrix;

// translate & rotation & scale
float paddle_rotate = 90.0f;
float ball_rotate = 0.0f;

float previous_ticks = 0.0f;
bool going_right = true;
bool rotating_right = true;

// texture
const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

// sprite filepath
const char PLAYER_SPRITE_FILEPATH[] = "blue_neon_paddle.png";
const char BALL_SPRITE_FILEPATH[] = "neon_ball.png";

// texture
GLuint player_texture_id;
GLuint ball_id;

// Input & Movement
glm::vec3 player_1_position = glm::vec3(-2.0f, 0.0f, 0.0f);
glm::vec3 player_1_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_1_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_1_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_2_position = glm::vec3(2.0f, 0.0f, 0.0f);
glm::vec3 player_2_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_2_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_2_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(1.0f, 0.5f, 0.0f);

float player_speed = 1.0f;

float ball_speed = 2.0f;

// collisions
const float MINIMUM_COLLISION_DISTANCE = 0.1f;
bool ball_to_right = true;
bool ball_to_top = true;

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
    display_window = SDL_CreateWindow("Neon Pong",
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
    model_matrix_player_1 = glm::mat4(1.0f);
    model_matrix_player_2 = glm::mat4(1.0f);
    model_matrix_ball = glm::mat4(1.0f);

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
    ball_id = load_texture(BALL_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // reset the movement
    player_1_movement = glm::vec3(0.0f);
    player_2_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        // End the game (using switch)
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;
        
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                // Quit the game with a keystroke
                gameIsRunning = false;
                break;
            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W] && player_1_position.y <= 1.5f)
    {
        player_1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && player_1_position.y >= -1.5f)
    {
        player_1_movement.y = -1.0f;
    }

    if (key_state[SDL_SCANCODE_UP] && player_2_position.y <= 1.5f)
    {
        player_2_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && player_2_position.y >= -1.5f)
    {
        player_2_movement.y = -1.0f;
    }

    // player can't move faster diagonally (won't be used here)
    if (glm::length(player_1_movement) > 1.0f)
    {
        player_1_movement = glm::normalize(player_1_movement);
    }
    if (glm::length(player_2_movement) > 1.0f)
    {
        player_2_movement = glm::normalize(player_2_movement);
    }
}

bool check_ball_to_bar_collision(glm::vec3& position_ball, glm::vec3& position_bar)
{
    return abs(position_ball.x - position_bar.x * 2.0f) < 0.35f &&
        position_ball.y < position_bar.y * 2.0f + 0.8f && position_ball.y > position_bar.y * 2.0f - 0.8f;
}

bool check_ball_to_topbot_edge_collision(glm::vec3& position_ball)
{
    return ((position_ball.y < -3.35f) && !ball_to_top) || ((position_ball.y > 3.35f) && ball_to_top);
}

bool check_ball_to_leftright_edge_collision(glm::vec3& position_ball)
{
    if (position_ball.x < -5.0f || position_ball.x > 5.0f){ gameIsRunning = false; }
    return position_ball.x < -5.0f || position_ball.x > 5.0f;
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;  // get the current number of ticks
    float delta_time = ticks - previous_ticks;       // the delta time is the difference from the last frame
    previous_ticks = ticks;                         // record current tick for next use

    // reset model matrix
    model_matrix_player_1 = glm::mat4(1.0f);
    model_matrix_player_2 = glm::mat4(1.0f);
    model_matrix_ball = glm::mat4(1.0f);

    // new position of players
    player_1_position += player_1_movement * player_speed * delta_time;
    player_2_position += player_2_movement * player_speed * delta_time;

    // ball becomes faster after hit and gradually slows down
    if (ball_speed > 2.0f) {
        ball_speed -= 0.5f * delta_time;
    }

    // collision checks
    if (check_ball_to_bar_collision(ball_position, player_1_position) ||
        check_ball_to_bar_collision(ball_position, player_2_position)) {
        ball_movement.x = -ball_movement.x;
        ball_speed = 3.2f;
    }

    if (check_ball_to_topbot_edge_collision(ball_position)) {
        ball_movement.y = -ball_movement.y;
        ball_to_top = !ball_to_top;
    }

    check_ball_to_leftright_edge_collision(ball_position);

    // new position of the ball
    ball_position += ball_movement * ball_speed * delta_time;

    // ball rotate
    ball_rotate += 90.0 * delta_time;

    // initalize scale vector
    glm::vec3 scale_vector;
    // adjust model matrix
    scale_vector = glm::vec3(2.0f,2.0f,1.0f);
    model_matrix_player_1 = glm::scale(model_matrix_player_1, scale_vector);
    model_matrix_player_1 = glm::translate(model_matrix_player_1, player_1_position);

    model_matrix_player_2 = glm::scale(model_matrix_player_2, scale_vector);
    model_matrix_player_2 = glm::translate(model_matrix_player_2, player_2_position);
 
    model_matrix_ball = glm::translate(model_matrix_ball, ball_position);
    model_matrix_ball = glm::rotate(model_matrix_ball, glm::radians(ball_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
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
    draw_object(model_matrix_player_1, player_texture_id);
    draw_object(model_matrix_player_2, player_texture_id);
    draw_object(model_matrix_ball, ball_id);

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

