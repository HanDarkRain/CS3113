#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Entity.h"
#include "Map.h"

struct GameState
{
    Map* map;
    Entity* player;
    Entity* enemies;
    Entity* reds;
    Entity* gates;
    Entity* bullets;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
    Mix_Chunk* fire_sfx;
    Mix_Chunk* gate_sfx;

    int next_scene_id;
};

class Scene {
public:
    int energy = 100;
    bool spawn_soldier_1 = false;
    bool spawn_soldier_2 = false;
    bool spawn_soldier_3 = false;

    float bullet_time_accumulator = 0;
    float blue_spawn_time_accumulator = 0;
    float energy_time_accumulator = 0;

    GameState state;

    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram* program) = 0;

    GameState const get_state() const { return this->state; }
};