#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 5
#define ROCK_COUNT 3

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
#include <vector>
#include "Entity.h"

// Create Game state
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* rocks;
};

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

// define improtant matrix (but not for entities)
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

// texture
const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const float MILLISECONDS_IN_SECOND = 1000.0;

// sprite filepath
const char SPRITESHEET_FILEPATH[] = "george_0.png";
const char PLATFORM_FILEPATH[] = "platformPack_tile027.png";
const char ROCK_FILEPATH[] = "rock.png";
const char TEXT_FILEPATH[] = "font1.png";

// tick counter and accumulator
float previous_ticks = 0.0f;
float accumulator = 0.0f;

GameState state;

const int FONTBANK_SIZE = 16;

bool win = false;
bool lose = false;

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

// Draw the texts
void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void initialise()
{
    // initiallize SDL & display window
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Lunar Lander",
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

    // defines the location and orientation of the camera
    view_matrix = glm::mat4(1.0f);
    // defines the characteristics of camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    // set view matrix & projection matrix
    program.SetViewMatrix(view_matrix);
    program.SetProjectionMatrix(projection_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // Rock
    GLuint rock_texture_id = load_texture(ROCK_FILEPATH);
    state.rocks = new Entity[ROCK_COUNT];
    for (int i = 0; i < ROCK_COUNT; i++)
    {
        state.rocks[i].texture_id = rock_texture_id;
        state.rocks[i].set_position(glm::vec3(i*3.5 - 3.0f, -1.0f, 0.0f));
        state.rocks[i].set_width(0.35f);
        state.rocks[i].set_height(0.35f);
        state.rocks[i].update(0.0f, NULL, 0);
        state.rocks[i].type = ITEM;
    }

    // Platform stuff
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-2.5f, -2.35f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].set_width(0.4f);
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 1].type = PLATFORM;

    for (int i = 0; i < PLATFORM_COUNT - 2; i++)
    {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, 0);
        state.platforms[i].type = PLATFORM;
    }

    state.platforms[PLATFORM_COUNT - 2].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(2.5f, -2.5f, 0.0f));
    state.platforms[PLATFORM_COUNT - 2].set_width(0.4f);
    state.platforms[PLATFORM_COUNT - 2].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 2].type = PLATFORM;

    // Existing
    state.player = new Entity();
    state.player->set_position(glm::vec3(-4.0f,3.0f,0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -0.81f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    state.player->type = PLAYER;

    // Walking
    state.player->walking[state.player->LEFT] = new int[4]{ 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4]{ 3, 7, 11, 15 };
    state.player->walking[state.player->UP] = new int[4]{ 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN] = new int[4]{ 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->LEFT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index = 0;
    state.player->animation_time = 0.0f;
    state.player->animation_cols = 4;
    state.player->animation_rows = 4;
    state.player->set_height(0.9f);
    state.player->set_width(0.9f);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // if nothing pressed, there is only gravity
    state.player->set_acceleration(glm::vec3(0.0f, -0.4f, 0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
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

    // four directions of accelerations (gravity is still in count)
    if (key_state[SDL_SCANCODE_LEFT])
    {
        state.player->set_acceleration(glm::vec3(-1.0f, -0.4f, 0.0f));
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->set_acceleration(glm::vec3(1.0f, -0.4f, 0.0f));
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_UP])
    {
        state.player->set_acceleration(glm::vec3(0.0f, 0.6f, 0.0f));
        state.player->animation_indices = state.player->walking[state.player->UP];
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        state.player->set_acceleration(glm::vec3(0.0f, -1.4f, 0.0f));
        state.player->animation_indices = state.player->walking[state.player->DOWN];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;  // get the current number of ticks
    float delta_time = ticks - previous_ticks;       // the delta time is the difference from the last frame
    previous_ticks = ticks;                         // record current tick for next use

    // add accumulator to avoid missing collison
    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // Update player with platfroms
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        // Win if player touch the top surface of the platform
        if (state.player->collided_bottom && state.player->get_last_collision_entity() != NULL) {
            if (state.player->get_last_collision_entity()->type == PLATFORM) {
                win = true;
            }
        }
        // Lose if player touch the sides of the platform
        if ((state.player->collided_left || state.player->collided_right)
            && state.player->get_last_collision_entity() != NULL) {
            if (state.player->get_last_collision_entity()->type == PLATFORM) {
                lose = true;
            }

        }
        // Update player with rocks
        state.player->update(FIXED_TIMESTEP, state.rocks, ROCK_COUNT);
        // Lose if player touch the rocks
        if ((state.player->collided_bottom || state.player->collided_top
            || state.player->collided_right || state.player->collided_left)
            && state.player->get_last_collision_entity() != NULL) {
            if (state.player->get_last_collision_entity()->type == ITEM) {
                lose = true;
            }
        }

        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    state.player->render(&program);

    // render rocks and platforms
    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    for (int i = 0; i < ROCK_COUNT; i++) state.rocks[i].render(&program);

    if (win) {
        GLuint text_texture_id = load_texture(TEXT_FILEPATH);
        DrawText(&program, text_texture_id, "SUCCESS", 1.0f, 0.1f, glm::vec3(-3.3f, 1.0f, 0.0f));
    }

    if (lose) {
        GLuint text_texture_id = load_texture(TEXT_FILEPATH);
        DrawText(&program, text_texture_id, "GAME OVER", 0.8f, 0.03f, glm::vec3(-3.3f, 1.0f, 0.0f));
    }
    

    SDL_GL_SwapWindow(display_window);
}

void shutdown(){
    SDL_Quit(); 

    delete[] state.rocks;
    delete[] state.platforms;
    delete state.player;
}


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

