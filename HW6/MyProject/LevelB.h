#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 100;
    int RED_COUNT = 100;
    int GATE_COUNT = 2;
    int BULLET_COUNT = 20;

    ~LevelB();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};