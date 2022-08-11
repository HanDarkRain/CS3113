#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Effects.h"

/**
 CONSTANTS
 */
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

/**
 VARIABLES
 */
Scene* current_scene;
Menu* menu;
LevelA* levelA;
LevelB* levelB;
LevelC* levelC;

Effects* effects;

Scene* levels[4];

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

bool is_colliding_bottom = false;

int hp = 3;

bool lose = false;
bool win = false;

void switch_to_scene(Scene* scene)
{
    current_scene = scene;
    current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Cute Metal Gear",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    menu = new Menu();
    levelA = new LevelA();
    levelB = new LevelB();
    levelC = new LevelC();

    levels[0] = menu;
    levels[1] = levelA;
    levels[2] = levelB;
    levels[3] = levelC;
    switch_to_scene(levels[0]);

    effects = new Effects(projection_matrix, view_matrix);
    effects->start(SHRINK, 2.0f);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    current_scene->state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                if (current_scene->state.player->collided_bottom)
                {
                    current_scene->state.player->is_jumping = true;
                    Mix_PlayChannel(-1, current_scene->state.jump_sfx, 0);
                }
                break;

            case SDLK_RETURN:
                // Start game
                if (current_scene == menu) {
                    switch_to_scene(levelA);
                    current_scene->state.player->set_hp(3);
                    }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        current_scene->state.player->movement.x = -1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        current_scene->state.player->movement.x = 1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->RIGHT];
    }

    if (glm::length(current_scene->state.player->movement) > 1.0f)
    {
        current_scene->state.player->movement = glm::normalize(current_scene->state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    // glm::length(<#const vec<L, T, Q> & v#>)

        while (delta_time >= FIXED_TIMESTEP) {
            current_scene->update(FIXED_TIMESTEP);
            hp = current_scene->state.player->get_hp();
            if (hp == 0) {
                lose = true;
            }
            effects->update(FIXED_TIMESTEP);

            if (is_colliding_bottom == false && current_scene->state.player->collided_bottom) effects->start(SHAKE, 1.0f);

            is_colliding_bottom = current_scene->state.player->collided_bottom;

            delta_time -= FIXED_TIMESTEP;
        }

    accumulator = delta_time;

    // Prevent the camera from showing anything outside of the "edge" of the level
    view_matrix = glm::mat4(1.0f);

    if (current_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        view_matrix = glm::translate(view_matrix, glm::vec3(-current_scene->state.player->get_position().x, 3.75, 0));
    }
    else {
        view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
    }

    if (current_scene == levelA && current_scene->state.player->get_position().y < -10.0f) {
        switch_to_scene(levelB);
        current_scene->state.player->set_hp(hp);
    }

    if (current_scene == levelB && current_scene->state.player->get_position().y < -10.0f) {
        switch_to_scene(levelC);
        current_scene->state.player->set_hp(hp);
    }

    if (current_scene == levelC && current_scene->state.player->get_position().y < -10.0f) {
        win = true;
    }

    view_matrix = glm::translate(view_matrix, effects->view_offset);
}

void render()
{
    program.SetViewMatrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program.programID);
    current_scene->render(&program);
    effects->render();

    if (win) {
        GLuint text_texture_id = Utility::load_texture("assets/font1.png");
        Utility::draw_text(&program, text_texture_id, "You Win", 1.0f, 0.1f, glm::vec3(11.5f, -3.5f, 0.0f));
    }
    if (lose) {
        current_scene->state.player->deactivate();
        GLuint text_texture_id = Utility::load_texture("assets/font1.png");
        Utility::draw_text(&program, text_texture_id, "You Lose", 0.8f, 0.03f, glm::vec3(2.3f, -3.5f, 0.0f));
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();

    delete menu;
    delete levelA;
    delete levelB;
    delete effects;
}

/**
 DRIVER GAME LOOP
 */
int main(int argc, char* argv[])
{
    initialise();

    while (game_is_running)
    {
        process_input();
        update();

        if (current_scene->state.next_scene_id >= 0) switch_to_scene(levels[current_scene->state.next_scene_id]);

        render();
    }

    shutdown();
    return 0;
}