#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 18
#define LEVEL_HEIGHT 8

unsigned int LEVELA_DATA[] =
{
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,
    2, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

LevelA::~LevelA()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    delete    this->state.reds;
    delete    this->state.gates;
    delete    this->state.bullets;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeChunk(this->state.fire_sfx);
    Mix_FreeChunk(this->state.gate_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialise()
{
    state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/sand_tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);

    // Gates
    GLuint gate_texture_id = Utility::load_texture("assets/gate.png");
    state.gates = new Entity[this->GATE_COUNT];
    state.gates[0].set_entity_type(GATE);
    state.gates[0].set_position(glm::vec3(3.0f, -3.0f, 0.0f));
    state.gates[0].texture_id = gate_texture_id;

    state.gates[1].set_entity_type(GATE);
    state.gates[1].set_position(glm::vec3(9.0f, -5.0f, 0.0f));
    state.gates[1].texture_id = gate_texture_id;

     // Player Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.0f, -3.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 4.0f;
    state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->texture_id = Utility::load_texture("assets/pointer.png");

    // Player Walking
    /*
    state.player->walking[state.player->LEFT] = new int[4]{ 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4]{ 3, 7, 11, 15 };
    state.player->walking[state.player->UP] = new int[4]{ 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN] = new int[4]{ 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start looking left
    state.player->animation_frames = 4;
    state.player->animation_index = 0;
    state.player->animation_time = 0.0f;
    state.player->animation_cols = 4;
    state.player->animation_rows = 4;
    */
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);

    // Jumping
    state.player->jumping_power = 5.0f;

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("assets/enemy.png");
    state.enemies = new Entity[this->ENEMY_COUNT];
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].set_entity_type(MINION);
        state.enemies[i].set_ai_type(WALKER);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_position(glm::vec3(7.0f+i, -5.0f, 0.0f));
        state.enemies[i].speed = 0.5f;
        state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        state.enemies[i].set_team(BLUE);
        state.enemies[i].deactivate();
    }

    // Ally minions
    GLuint red_texture_id = Utility::load_texture("assets/red.png");
    state.reds = new Entity[this->RED_COUNT];
    for (int i = 0; i < RED_COUNT; i++) {
        state.reds[i].set_entity_type(MINION);
        state.reds[i].set_ai_type(WALKER);
        //state.reds[i].texture_id = red_texture_id;
        state.reds[i].set_position(glm::vec3(3.0f + i, -5.0f, 0.0f));
        state.reds[i].speed = -0.5f;
        state.reds[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        state.reds[i].set_team(RED);
        state.reds[i].deactivate();
    }

    // Bullets
    GLuint bullet_texture_id = Utility::load_texture("assets/bullet.png");
    state.bullets = new Entity[this->BULLET_COUNT];
    for (int i = 0; i < BULLET_COUNT; i++) {
        state.bullets[i].set_entity_type(BULLET);
        state.bullets[i].set_ai_type(WALKER);
        state.bullets[i].texture_id = bullet_texture_id;
        state.bullets[i].set_position(glm::vec3(3.0f + i, -5.0f, 0.0f));
        state.bullets[i].speed = -2.0f;
        state.bullets[i].set_acceleration(glm::vec3(0.0f, -0.01f, 0.0f));
        state.bullets[i].set_team(RED);
        state.bullets[i].deactivate();
    }

    /*
    state.enemies[0].set_entity_type(MINION);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(7.0f, -3.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    //GLuint enemy_texture_id = Utility::load_texture("assets/enemy.png");
    state.enemies[1].set_entity_type(MINION);
    state.enemies[1].set_ai_type(WALKER);
    state.enemies[1].set_ai_state(WALKING);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(4.0f, -3.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 1.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    */

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("assets/MGSVBGM.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME/2.0f);

    state.jump_sfx = Mix_LoadWAV("assets/equip.wav");
    state.fire_sfx = Mix_LoadWAV("assets/gun_shot.wav");
    state.gate_sfx = Mix_LoadWAV("assets/New item.wav");
    Mix_VolumeChunk(state.jump_sfx, MIX_MAX_VOLUME / 2.0f);
    Mix_VolumeChunk(state.fire_sfx, MIX_MAX_VOLUME / 3.0f);
    Mix_VolumeChunk(state.gate_sfx, MIX_MAX_VOLUME / 2.0f);
}

void LevelA::update(float delta_time)
{
    blue_spawn_time_accumulator += delta_time;
    bullet_time_accumulator += delta_time;

    // update pointer
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);

    // Update blue
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].update(delta_time, state.player, nullptr, 0, this->state.map);
        for (int j = 0; j < RED_COUNT; j++) {
            if (state.enemies[i].get_state() && state.enemies[i].check_collision(&state.reds[j])) {
                state.enemies[i].deactivate();
                state.reds[j].set_gated(true);
                state.reds[j].speed = -0.5f;
                state.reds[j].deactivate();
            }
        }
        for (int j = 0; j < BULLET_COUNT; j++) {
            if (state.enemies[i].get_state() && state.enemies[i].check_collision(&state.bullets[j])) {
                state.enemies[i].deactivate();
                state.bullets[j].deactivate();
            }
        }
        if (state.enemies[i].get_position().x < 2.0f) state.player->set_hp(0);
    }

    // Update red
    bool shot = false;
    for (int i = 0; i < RED_COUNT; i++) {
        state.reds[i].update(delta_time, state.player, nullptr, 0, this->state.map);
        // bullets!!!
        if (bullet_time_accumulator > 2) {
            shot = true;
            if (state.reds[i].get_state() && state.reds[i].get_ai_type() == SHOOTER) {
                for (int j = 0; j < BULLET_COUNT; j++) {
                    if (state.bullets[j].get_state() == false) {
                        Mix_PlayChannel(-1, state.fire_sfx, 0);
                        glm::vec3 position = state.reds[i].get_position();
                        state.bullets[j].set_position(position);
                        state.bullets[j].activate();
                        break;
                    }
                }
            }
        }
        // Gated!!!
        for (int j = 0; j < GATE_COUNT; j++) {
            if (state.reds[i].get_state() && state.reds[i].check_collision(&state.gates[j])
                && state.gates[j].get_state() && !state.reds[i].get_gated()) {
                Mix_PlayChannel(-1, state.gate_sfx, 0);
                state.reds[i].set_gated(true);
                GLuint red_texture_id = Utility::load_texture("assets/red_gated.png");
                state.reds[i].texture_id = red_texture_id;
                if (state.reds[i].get_ai_type() == WALKER) {
                    for (int k = 0; k < RED_COUNT; k++) {
                        if (state.reds[k].get_state() == false) {
                            GLuint red_texture_id = Utility::load_texture("assets/red_gated.png");
                            state.reds[k].texture_id = red_texture_id;
                            glm::vec3 position = state.reds[i].get_position();
                            state.reds[k].set_ai_type(WALKER);
                            state.reds[k].set_position(position + glm::vec3(0.5f, 0.0f, 0.0f));
                            state.reds[k].set_gated(true);
                            state.reds[k].activate();
                            break;
                        }
                    }
                }
                if (state.reds[i].get_ai_type() == RUNNER) {
                    GLuint red_texture_id = Utility::load_texture("assets/red_runner_gated.png");
                    state.reds[i].texture_id = red_texture_id;
                    state.reds[i].speed = -1.0f;
                }
                if (state.reds[i].get_ai_type() == SHOOTER) {
                    GLuint red_texture_id = Utility::load_texture("assets/red_shooter_gated.png");
                    state.reds[i].texture_id = red_texture_id;
                    state.reds[i].speed = -0.1f;
                }

            }
        }
        if (state.reds[i].get_state() && state.reds[i].get_position().x > 12.0f) state.next_scene_id = 2;
    }
    if (shot) bullet_time_accumulator = 0;

    // Spawn red walker
    if (spawn_soldier_1) {
        spawn_soldier_1 = false;
        for (int i = 0; i < RED_COUNT; i++) {
            if (state.reds[i].get_state() == false) {
                GLuint red_texture_id = Utility::load_texture("assets/red.png");
                state.reds[i].texture_id = red_texture_id;
                float y_position = state.player->get_position().y;
                state.reds[i].set_ai_type(WALKER);
                state.reds[i].set_position(glm::vec3(2.0f, y_position, 0.0f));
                state.reds[i].set_gated(false);
                state.reds[i].activate();
                break;
            }
        }
    }

    // Spawn red runner
    if (spawn_soldier_2) {
        spawn_soldier_2 = false;
        for (int i = 0; i < RED_COUNT; i++) {
            if (state.reds[i].get_state() == false) {
                GLuint red_texture_id = Utility::load_texture("assets/red_runner.png");
                state.reds[i].texture_id = red_texture_id;
                float y_position = state.player->get_position().y;
                state.reds[i].set_ai_type(RUNNER);
                state.reds[i].set_position(glm::vec3(2.0f, y_position, 0.0f));
                state.reds[i].set_gated(false);
                state.reds[i].activate();
                break;
            }
        }
    }

    // Spawn red shooter
    if (spawn_soldier_3) {
        spawn_soldier_3 = false;
        for (int i = 0; i < RED_COUNT; i++) {
            if (state.reds[i].get_state() == false) {
                GLuint red_texture_id = Utility::load_texture("assets/red_shooter.png");
                state.reds[i].texture_id = red_texture_id;
                float y_position = state.player->get_position().y;
                state.reds[i].set_ai_type(SHOOTER);
                state.reds[i].set_position(glm::vec3(2.0f, y_position, 0.0f));
                state.reds[i].set_gated(false);
                state.reds[i].activate();
                break;
            }
        }
    }

    // Spawn blue
    if (blue_spawn_time_accumulator > 1) {
        blue_spawn_time_accumulator = 0;
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (state.enemies[i].get_state() == false) {
                state.enemies[i].set_position(glm::vec3(12.0f, -6.0, 0.0f));
                state.enemies[i].activate();
                break;
            }
        }
    }

    // Update gates
    for (int i = 0; i < GATE_COUNT; i++) {
        state.gates[i].update(delta_time, state.player, nullptr, 0, this->state.map);
    }

    // Update bullets
    for (int i = 0; i < BULLET_COUNT; i++) {
        state.bullets[i].update(delta_time, state.player, nullptr, 0, this->state.map);
        if (state.bullets[i].get_position().x > 12.0f) state.bullets[i].deactivate();
    }

    // Update energy
    energy_time_accumulator += delta_time;
    if (energy_time_accumulator > 0.5f) {
        energy_time_accumulator = 0;
        energy += 5;
        if (energy > 200) energy = 200;
    }
}

void LevelA::render(ShaderProgram* program)
{
    this->state.map->render(program);
    for (int i = 0; i < GATE_COUNT; i++) {
        this->state.gates[i].render(program);
    }
    for (int i = 0; i < ENEMY_COUNT; i++) {
        this->state.enemies[i].render(program);
    }
    for (int i = 0; i < RED_COUNT; i++) {
        this->state.reds[i].render(program);
    }
    for (int i = 0; i < BULLET_COUNT; i++) {
        this->state.bullets[i].render(program);
    }
    this->state.player->render(program);
}