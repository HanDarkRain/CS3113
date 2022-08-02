#pragma once
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

enum EntityType { PLATFORM, PLAYER, ENEMY };
enum AIState { WALKING, IDLE, ATTACKING, JUMPING, WANDERING, HI_JUMPING};
enum AIType { WALKER, GUARD, JUMPER };

class Entity
{
private:
    bool is_active = true;
    int* animation_right = NULL; // move to the right
    int* animation_left = NULL; // move to the left
    int* animation_up = NULL; // move upwards
    int* animation_down = NULL; // move downwards

    // Three basic vectors
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    float width = 1;
    float height = 1;

    EntityType entity_type;
    AIType ai_type;
    AIState ai_state;

    Entity* last_collision_entity = NULL;   // Last entity it collide with

public:
    // Static attributes
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT = 0,
        RIGHT = 1,
        UP = 2,
        DOWN = 3;

    // Existing
    GLuint texture_id;
    glm::mat4 model_matrix;

    // Translating
    float speed;
    glm::vec3 movement;
    float drag;

    // Animating
    int** walking = new int* [4]{ animation_left, animation_right, animation_up, animation_down };
    int* animation_indices = NULL;
    int animation_frames = 0;
    int animation_index = 0;
    float animation_time = 0.0f;
    int animation_cols = 0;
    int animation_rows = 0;

    // Jumping
    bool is_jumping = false;
    float jumping_power = 5.2f;

    // found
    int found = 0;

    // Shooting
    bool is_shooting = false;

    // Colliding
    bool collided_top = false;
    bool collided_bottom = false;
    bool collided_left = false;
    bool collided_right = false;

    // AI methods
    void activate_ai(Entity* player);
    void ai_jumper(Entity* player);
    void ai_guard(Entity* player);
    void ai_walker();

    // Methods
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Entity* shootable_entities, int shootable_entity_count);
    void render(ShaderProgram* program);

    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    Entity* const const check_shooting(Entity* collidable_entities, int collidable_entity_count);
    // int const const check_about_to_fall(Entity* collidable_entities, int collidable_entity_count);
    bool const check_collision(Entity* other) const;

    void activate() { is_active = true; };
    void deactivate() { is_active = false; };

    // Getters
    bool       const get_active()        const { return is_active; };
    glm::vec3 const get_position()     const { return position; };
    glm::vec3 const get_movement()     const { return movement; };
    glm::vec3 const get_velocity()     const { return velocity; };
    glm::vec3 const get_acceleration() const { return acceleration; };
    int       const get_width()        const { return width; };
    int       const get_height()       const { return height; };
    EntityType const get_entity_type()  const { return entity_type; };
    AIType     const get_ai_type()      const { return ai_type; };
    AIState    const get_ai_state()     const { return ai_state; };
    Entity* const get_last_collision_entity() const { return last_collision_entity; };

    // Setters
    void const set_position(glm::vec3 new_position) { position = new_position; };
    void const set_original_position(glm::vec3 new_position) { position = new_position; };
    void const set_movement(glm::vec3 new_movement) { movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration; };
    void const set_width(float new_width) { width = new_width; };
    void const set_height(float new_height) { height = new_height; };
    void const set_entity_type(EntityType new_entity_type) { entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { ai_state = new_state; };
};