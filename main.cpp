/// Gra Jak Statki Na Niebie
#include <cmath>
#include <cstdio>
// To please the RNGods
#include <SFML/Graphics.hpp>
#include <ctime>
#include <fstream>
#include <iostream>
#pragma once

using namespace std;

const static int SWIMMING_FRAME_COUNT = 4;
const static string SWIMMING_FILE[SWIMMING_FRAME_COUNT] = {
    "assets/swimming1.png", "assets/swimming2.png", "assets/swimming3.png",
    "assets/swimming4.png"};
static sf::Texture SWIMMING_TEXTURE[SWIMMING_FRAME_COUNT];

const static int SHOOT_FRAME_COUNT = 8;
const static string SHOOT_FILE[SHOOT_FRAME_COUNT] = {
    "assets/shoot2.png", "assets/shoot3.png", "assets/shoot4.png",
    "assets/shoot5.png", "assets/shoot6.png", "assets/shoot7.png",
    "assets/shoot8.png", "assets/shoot1.png"};
static sf::Texture SHOOT_TEXTURE[SHOOT_FRAME_COUNT];

const static int BARREL_EXPLOSION_FRAME_COUNT = 11;
const static string BARREL_EXPLOSION_FILE[BARREL_EXPLOSION_FRAME_COUNT] = {
    "assets/barrelexplosion1.png", "assets/barrelexplosion2.png",
    "assets/barrelexplosion3.png", "assets/barrelexplosion4.png",
    "assets/barrelexplosion5.png", "assets/barrelexplosion6.png",
    "assets/barrelexplosion7.png", "assets/barrelexplosion8.png",
    "assets/barrelexplosion9.png", "assets/barrelexplosion10.png",
    "assets/barrelexplosion11.png"};
static sf::Texture BARREL_EXPLOSION_TEXTURE[BARREL_EXPLOSION_FRAME_COUNT];

const static int BARREL_SWIMMING_FRAME_COUNT = 4;
const static string BARREL_SWIMMING_FILE[BARREL_SWIMMING_FRAME_COUNT] = {
    "assets/barrelswimming1.png", "assets/barrelswimming2.png",
    "assets/barrelswimming3.png", "assets/barrelswimming4.png"};
static sf::Texture BARREL_SWIMMING_TEXTURE[BARREL_SWIMMING_FRAME_COUNT];

const static string BALL_TEXTURE_FILE = "assets/ball.png";
static sf::Texture BALL_TEXTURE;

const static string ENEMY_TEXTURE_FILE = "assets/enemy.png";
static sf::Texture ENEMY_TEXTURE;

const static int PLAYER_FRAME_COUNT = 3;
const static string PLAYER_TEXTURE_FILE[PLAYER_FRAME_COUNT] = {
    "assets/ship1.png", "assets/ship2.png", "assets/ship3.png"};
static sf::Texture PLAYER_TEXTURE[PLAYER_FRAME_COUNT];

const static string FONT_FILE = "assets/Treamd.ttf";
static sf::Font FONT;

const float DEG_TO_RADS = 3.14159265 / 180;

template <typename T> T clamp(T v, T lo, T hi) {
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

template <typename T> T wrap(T v, T limit) {
    T r = v;
    if (r < 0)
        r = limit + r;
    while (r >= limit)
        r -= limit;
    return r;
}

template <typename T> string to_string(T value) {
    string buffer;
    sprintf((char *)buffer.c_str(), "%i", value);
    return buffer.c_str();
}

template <typename T> T mix(T t1, T t2, float amount) {
    return (t1 * (1.0 - amount)) + (t2 * amount);
}

sf::Color mix(sf::Color c1, sf::Color c2, float amount) {
    return sf::Color(mix(c1.r, c2.r, amount), mix(c1.g, c2.g, amount),
                     mix(c1.b, c2.b, amount), mix(c1.a, c2.a, amount));
}

void init_textures(const int count, sf::Texture texture[],
                   const string file[]) {
    for (int i = 0; i < count; i++)
        texture[i].loadFromFile(file[i]);
}

void init_assets() {
    init_textures(SWIMMING_FRAME_COUNT, SWIMMING_TEXTURE, SWIMMING_FILE);
    init_textures(SHOOT_FRAME_COUNT, SHOOT_TEXTURE, SHOOT_FILE);
    init_textures(BARREL_EXPLOSION_FRAME_COUNT, BARREL_EXPLOSION_TEXTURE,
                  BARREL_EXPLOSION_FILE);
    init_textures(BARREL_SWIMMING_FRAME_COUNT, BARREL_SWIMMING_TEXTURE,
                  BARREL_SWIMMING_FILE);
    BALL_TEXTURE.loadFromFile(BALL_TEXTURE_FILE);
    ENEMY_TEXTURE.loadFromFile(ENEMY_TEXTURE_FILE);
    init_textures(PLAYER_FRAME_COUNT, PLAYER_TEXTURE, PLAYER_TEXTURE_FILE);
    FONT.loadFromFile(FONT_FILE);
}

class Timer {
  private:
    float counter;
    float limit;

  public:
    Timer(float limit) : counter(0), limit(limit) {}
    void add(float delta) { counter = clamp(counter + delta, 0.0f, limit); }
    float get_counter() { return counter; }
    void reset() { counter = 0; }
    bool isLimit() { return counter >= limit; }
    bool didWrap(float delta) {
        add(delta);
        bool result = isLimit();
        if (result)
            reset();
        return result;
    }
};

class Entity {
  protected:
    int hp;
    sf::Sprite sprite;

  private:
    sf::Texture texture;
    sf::Vector2f velocity;

  public:
    Entity(sf::Texture &texture, int hp, int pos_x, int pos_y, float vel_x,
           float vel_y, float rot)
        : hp(hp), texture(texture), velocity(vel_x, vel_y) {
        sf::Vector2u dims = texture.getSize();
        sprite.setTexture(texture);
        sprite.setOrigin((float)dims.x / 2, (float)dims.y / 2);
        sprite.setPosition(pos_x, pos_y);
        sprite.setRotation(rot);
    }
    sf::Vector2f getPosition() { return sprite.getPosition(); }
    float getRotation() { return sprite.getRotation(); }
    bool isDead() { return hp <= 0; }
    void damage() { hp--; }
    int getHP() { return hp; }
    void setHP(int hp_) { hp = hp_; }
    void draw(sf::RenderWindow &window) { window.draw(sprite); }
    void update(sf::Vector2f movement, float delta) {
        float x = (velocity.x + movement.x) * delta;
        float y = (velocity.y + movement.y) * delta;
        sprite.move(x, y);
    }
};

class ShipEntity : public Entity {
    const static int cannons_amount = 6;
    sf::Sprite cannons[cannons_amount];
    sf::Sprite wave;
    int frame_counter = 0;
    Timer cannon_texture_timer;
    Timer cannon_timer;
    Timer spawnBarrel_timer;
    int wave_frame_counter = 0;
    Timer wave_texture_timer;

  public:
    float wave_scale = 1;
    ShipEntity(sf::Texture &tex, int hp, int pos_x, int pos_y, float vel_x,
               float vel_y, float rot)
        : Entity(tex, hp, pos_x, pos_y, vel_x, vel_y, rot),
          cannon_texture_timer(0.06), cannon_timer(1.5), spawnBarrel_timer(7.0),
          wave_texture_timer(0.3) {
        const int FRONT = -15, MIDDLE = 5, BACK = 35, DISTANCE = 40,
                  OFFSET_X = 12, OFFSET_Y = 4;
        cannons[0].setOrigin(FRONT, DISTANCE);
        cannons[1].setOrigin(MIDDLE, DISTANCE);
        cannons[2].setOrigin(BACK, DISTANCE);
        cannons[3].setOrigin(FRONT + OFFSET_X, DISTANCE + OFFSET_Y);
        cannons[4].setOrigin(3 * MIDDLE + OFFSET_X, DISTANCE + OFFSET_Y);
        cannons[5].setOrigin(BACK + OFFSET_X, DISTANCE + OFFSET_Y);
    }
    void update(sf::Vector2f movement, float delta) {
        Entity::update(movement, delta);

        if (cannon_texture_timer.didWrap(delta))
            frame_counter = clamp(frame_counter + 1, 0, SHOOT_FRAME_COUNT - 1);
        for (int i = 0; i < cannons_amount; i++)
            cannons[i].setTexture(SHOOT_TEXTURE[frame_counter]);
        cannon_timer.add(delta);
        spawnBarrel_timer.add(delta);

        if (wave_texture_timer.didWrap(delta))
            wave_frame_counter =
                (wave_frame_counter + 1) % SWIMMING_FRAME_COUNT;
        wave.setTexture(SWIMMING_TEXTURE[wave_frame_counter]);
    }
    void draw(sf::RenderWindow &window) {
        wave.setOrigin(-70 / wave_scale, 24);
        wave.setScale(wave_scale, wave_scale);
        wave.setPosition(sprite.getPosition());
        wave.setRotation(sprite.getRotation());
        window.draw(wave);

        for (int i = 0; i < cannons_amount; i++) {
            cannons[i].setPosition(sprite.getPosition());
            float rot;
            if (i < 3)
                rot = 0;
            else
                rot = 180;
            cannons[i].setRotation(wrap(sprite.getRotation() + rot, 360.0f));
            window.draw(cannons[i]);
        Entity::draw(window);
        }
    }
    bool canShoot() { return cannon_timer.isLimit(); }

    bool canPutBarrel() { return spawnBarrel_timer.isLimit(); }

    void restartCannon() {
        cannon_timer.reset();
        frame_counter = 0;
    }

    void restartPutBarrel() { spawnBarrel_timer.reset(); }
};

class BallEntity : public Entity {
  public:
    BallEntity(int pos_x, int pos_y, float vel_x, float vel_y)
        : Entity(BALL_TEXTURE, 1, pos_x, pos_y, vel_x, vel_y, 0) {}
};

class BarrelEntity : public Entity {
    int frame_counter = 0;
    Timer barrel_texture_timer;
    Timer barrel_explosion_texture_timer;
    bool animation = true;

  public:
    BarrelEntity(int pos_x, int pos_y, float vel_x, float vel_y)
        : Entity(BARREL_SWIMMING_TEXTURE[0], 1, pos_x, pos_y, vel_x, vel_y, 0),
          barrel_texture_timer(0.3), barrel_explosion_texture_timer(0.1) {}

    void update(sf::Vector2f movement, float delta) {
        Entity::update(movement, delta);
        barrel_texture_timer.add(delta);
        barrel_explosion_texture_timer.add(delta);
        if (barrel_texture_timer.isLimit() && animation == true) {
            frame_counter = (frame_counter + 1) % BARREL_SWIMMING_FRAME_COUNT;
            barrel_texture_timer.reset();
            sprite.setTexture(BARREL_SWIMMING_TEXTURE[frame_counter]);
        }
        if (barrel_explosion_texture_timer.isLimit() && animation == false) {
            frame_counter =
                clamp(frame_counter + 1, 0, BARREL_EXPLOSION_FRAME_COUNT - 1);
            barrel_explosion_texture_timer.reset();
            sprite.setTexture(BARREL_EXPLOSION_TEXTURE[frame_counter]);
        }
    }
    void closeanimation() {
        if(animation == true) frame_counter = 0;
        animation = false;
    }
    bool getanimation() { return animation; }
    int getFrameCounter() { return frame_counter; }
};

class EnemyEntity : public ShipEntity {
    const static int MAX_HP = 7;

  public:
    EnemyEntity(int pos_x, int pos_y, float vel_x, float vel_y, float rot)
        : ShipEntity(ENEMY_TEXTURE, MAX_HP, pos_x, pos_y, vel_x, vel_y, rot) {}
    void draw(sf::RenderWindow &window) {
        ShipEntity::draw(window);
        const int HP_BAR_SIZE = 10;
        sf::RectangleShape hp_bar(sf::Vector2f(HP_BAR_SIZE * hp, HP_BAR_SIZE));
        hp_bar.setPosition(sprite.getPosition() +
                           sf::Vector2f(-HP_BAR_SIZE / 2 * MAX_HP, 80));
        hp_bar.setFillColor(sf::Color::Red);
        hp_bar.setOutlineColor(sf::Color::Black);
        hp_bar.setOutlineThickness(1.0);
        window.draw(hp_bar);
    }
};

class PlayerShipEntity : public ShipEntity {
    const static int MAX_HP = 25;

  public:
    PlayerShipEntity(int pos_x, int pos_y)
        : ShipEntity(PLAYER_TEXTURE[2], MAX_HP, pos_x, pos_y, 0, 0, 0) {}
    void update(float rotation, float delta) {
        sprite.setRotation(rotation);
        ShipEntity::update(sf::Vector2f(), delta);
    }
    void draw(sf::RenderWindow &window) {
        ShipEntity::draw(window);
        const int HP_BAR_SIZE = 10;
        sf::RectangleShape hp_bar(sf::Vector2f(HP_BAR_SIZE / 2 * hp, HP_BAR_SIZE));
        hp_bar.setPosition(sprite.getPosition() +
                           sf::Vector2f(-HP_BAR_SIZE / 4 * MAX_HP, 80));
        hp_bar.setFillColor(sf::Color::Green);
        hp_bar.setOutlineColor(sf::Color::Black);
        hp_bar.setOutlineThickness(1.0);
        window.draw(hp_bar);
    }
};

class World {
    float spawnEnemy_timer = 0;
    float spawnEnemy_timer_limit = 2.0;
    float boost_timer = 0;
    float boost_timer_limit = 0.3;
    float boost_load_timer = 0;
    float boost_load_timer_limit = 3.0;
    float break_timer = 0;
    float break_timer_limit = 0.3;
    float break_load_timer = 0;
    float break_load_timer_limit = 3.0;

    float credits_timer = 0;
    float credits_timer_limit = 7.0;

    float points_timer = 0;
    float points_timer_limit = 1.0;

    const static int MAX_BALLS = 200;
    const static int MAX_ENEMIES = 11;
    const static int MAX_BARRELS = 50;
    const float MAX_SPEED = 500;
    const float BALL_SPEED = 300;
    const float ENEMY_SPEED = 55;
    const float BARREL_SPEED = 20;
    const int WIDTH;
    const int HEIGHT;

    int TIME = 0;
    int max_enemies = 3;

    BallEntity *balls[MAX_BALLS];
    EnemyEntity *enemies[MAX_ENEMIES];
    BarrelEntity *barrels[MAX_BARRELS];
    int numBalls;
    int numEnemies;
    int numBarrels;
    PlayerShipEntity player;
    long long int points = 0;
    long int kills = 0;
    float speed;

    sf::RenderWindow window;

    bool keys[sf::Keyboard::KeyCount];

    float rotation_rads() { return player.getRotation() * DEG_TO_RADS; }

    bool outsideView(sf::Vector2f entity_pos) {
        if (entity_pos.x < -100 || entity_pos.x > WIDTH + 100 ||
            entity_pos.y < -100 || entity_pos.y > HEIGHT + 100)
            return true;
        return false;
    }
    bool doCollide(sf::Vector2f v1, sf::Vector2f v2, float limit) {
        float x = v1.x - v2.x;
        float y = v1.y - v2.y;
        float dist = x * x + y * y;
        return dist < limit * limit;
    }
    void processEvent(sf::Event event) {
        if (event.type == sf::Event::Closed ||
            (event.type == sf::Event::KeyPressed &&
             event.key.code == sf::Keyboard::Escape))
            window.close();
        if (event.type == sf::Event::KeyPressed)
            keys[event.key.code] = true;
        if (event.type == sf::Event::KeyReleased)
            keys[event.key.code] = false;
    }
    void checkCollision() {
        for (int i = numEnemies - 1; i >= 0; i--) {
            for (int j = numBalls - 1; j >= 0; j--) {
                if (doCollide(enemies[i]->getPosition(),
                              balls[j]->getPosition(), 40)) {
                    enemies[i]->damage();
                    balls[j]->damage();
                }
            }
            if (doCollide(enemies[i]->getPosition(), player.getPosition(),
                          100)) {
                enemies[i]->damage();
                player.damage();
            }
        }
        for (int j = numBalls - 1; j >= 0; j--) {
            if (doCollide(player.getPosition(), balls[j]->getPosition(), 40)) {
                player.damage();
                balls[j]->damage();
            }
        }
        for (int i = numBarrels - 1; i >= 0; i--) {
            if (doCollide(player.getPosition(), barrels[i]->getPosition(),
                          100)) {
                barrels[i]->closeanimation();
            }
            if (barrels[i]->getFrameCounter() == 0 &&
                barrels[i]->getanimation() == false) {
                player.damage();
                player.damage();
                player.damage();
            }
            if (barrels[i]->getFrameCounter() == 10 &&
                barrels[i]->getanimation() == false)
                barrels[i]->damage();
        }
    }
    void clearEntities() {
        for (int i = numBalls - 1; i >= 0; i--)
            delete balls[i];
        numBalls = 0;
        for (int i = numEnemies - 1; i >= 0; i--)
            delete enemies[i];
        numEnemies = 0;
        for (int i = numBarrels - 1; i >= 0; i--)
            delete barrels[i];
        numBarrels = 0;
    }
    void spawnEnemy(int pos_x, int pos_y, float vel_x, float vel_y,
                    float rotation) {
        int en = max_enemies - 1;
        if (numEnemies < MAX_ENEMIES && numEnemies < max_enemies) {
            enemies[numEnemies++] =
                new EnemyEntity(pos_x, pos_y, vel_x, vel_y, rotation);
            if (50 * en * (1 + en) <= points)
                max_enemies++;
        }
    }
    void spawnBarrel(int pos_x, int pos_y, float vel_x, float vel_y) {
        if (numBarrels < MAX_BARRELS)
            barrels[numBarrels++] =
                new BarrelEntity(pos_x, pos_y, vel_x, vel_y);
    }
    void spawnBall(int pos_x, int pos_y, float vel_x, float vel_y) {
        if (numBalls < MAX_BALLS)
            balls[numBalls++] = new BallEntity(pos_x, pos_y, vel_x, vel_y);
    }

    void fireCannons(float px, float py, float sin, float cos) {
        const int FRONT = -20, MIDDLE = 10, BACK = 40, DISTANCE = 55;
        // Left side
        spawnBall(px + cos * FRONT + sin * -DISTANCE,
                  py + sin * FRONT + cos * DISTANCE, -sin * BALL_SPEED,
                  cos * BALL_SPEED);
        spawnBall(px + cos * MIDDLE + sin * -DISTANCE,
                  py + sin * MIDDLE + cos * DISTANCE, -sin * BALL_SPEED,
                  cos * BALL_SPEED);
        spawnBall(px + cos * BACK + sin * -DISTANCE,
                  py + sin * BACK + cos * DISTANCE, -sin * BALL_SPEED,
                  cos * BALL_SPEED);
        // Right side
        spawnBall(px + cos * FRONT + sin * DISTANCE,
                  py + sin * FRONT + cos * -DISTANCE, sin * BALL_SPEED,
                  -cos * BALL_SPEED);
        spawnBall(px + cos * MIDDLE + sin * DISTANCE,
                  py + sin * MIDDLE + cos * -DISTANCE, sin * BALL_SPEED,
                  -cos * BALL_SPEED);
        spawnBall(px + cos * BACK + sin * DISTANCE,
                  py + sin * BACK + cos * -DISTANCE, sin * BALL_SPEED,
                  -cos * BALL_SPEED);
    }
    void putBarrel(float px, float py, float sin, float cos) {
        const int BACK = 71;
        spawnBarrel(px + cos * BACK, py + sin * BACK, cos * BARREL_SPEED,
                    sin * BARREL_SPEED);
    }

    bool canCoup_de_Burst() { return boost_timer < boost_timer_limit; }
    void restartCoup_de_Burst() { boost_timer = 0; }
    void restartCoup_de_Burst_Load() { boost_load_timer = 0; }

    bool canHalt() { return break_timer < break_timer_limit; }
    void restartHalt() { break_timer = 0; }
    void restartHalt_Load() { break_load_timer = 0; }

    bool canSpawnEnemy() { return spawnEnemy_timer >= spawnEnemy_timer_limit; }
    void restartSpawnerEnemy() { spawnEnemy_timer = 0; }

    bool canGivePoints() { return points_timer >= points_timer_limit; }
    void restartGivingPoints() { points_timer = 0; }

  public:
    World(const int width, const int height)
        : WIDTH(width), HEIGHT(height), numBalls(0), numEnemies(0),
          numBarrels(0), player(width / 2, height / 2), points(0), speed(0),
          window(sf::VideoMode(width, height, 32), "Jak Statki Na Niebie") {
        window.setVerticalSyncEnabled(true);
        for (int i = 0; i < sf::Keyboard::KeyCount; i++)
            keys[i] = false;
    }
    bool isPlayerDead() { return player.isDead(); }
    long long int getPoints() { return points; }
    long int getKills() { return kills; }
    int getTime() { return TIME; }
    void saveToFile(string filename) {
        ofstream file;
        file.open(filename);

        file << player.getHP() << endl << points;

        file.close();
    }
    void loadFromFile(string filename) {
        ifstream file;
        file.open(filename);

        int hp_;
        long long int points_;
        file >> hp_ >> points_;
        player.setHP(hp_);
        points = points_;
        clearEntities();
        file.close();
    }
    void WindowClose() { window.close(); }
    bool isWindowOpen() { return window.isOpen(); }
    void update(float delta) {
        sf::Event event;
        while (window.pollEvent(event))
            processEvent(event);

        if (keys[sf::Keyboard::F7])
            saveToFile("test1.sav");
        if (keys[sf::Keyboard::F8])
            loadFromFile("test1.sav");
        if (keys[sf::Keyboard::F9])
            points = 1000;

        credits_timer = clamp(credits_timer + delta, 0.0f, credits_timer_limit);

        points_timer = clamp(points_timer + delta, 0.0f, points_timer_limit);
        if(canGivePoints()) {
            TIME++;
            points += 3;
            restartGivingPoints();
        }

        float px = player.getPosition().x;
        float py = player.getPosition().y;
        float sin_rot = sin(rotation_rads());
        float cos_rot = cos(rotation_rads());
        float rot_step = 60 * delta;
        float player_speed = 0;
        bool left = keys[sf::Keyboard::Left] || keys[sf::Keyboard::A];
        bool up = keys[sf::Keyboard::Up] || keys[sf::Keyboard::W];
        bool down = keys[sf::Keyboard::Down] || keys[sf::Keyboard::S];
        bool right = keys[sf::Keyboard::Right] || keys[sf::Keyboard::D];

        // if(player.canShoot()) cout << "PRESS SPACE TO WIN" << endl;
        if (keys[sf::Keyboard::Space] && player.canShoot()) {
            fireCannons(px, py, sin_rot, cos_rot);
            player.restartCannon();
        }

        if (up && canCoup_de_Burst()) {
            boost_timer = clamp(boost_timer + delta, 0.0f, boost_timer_limit);
            player_speed = 500;
            player.wave_scale = 2;
        } else if (down && canHalt()) {
            break_timer = clamp(break_timer + delta, 0.0f, break_timer_limit);
            player_speed = 0;
            player.wave_scale = 0;
        } else {
            boost_load_timer =
                clamp(boost_load_timer + delta, 0.0f, boost_load_timer_limit);
            break_load_timer =
                clamp(break_load_timer + delta, 0.0f, break_load_timer_limit);
            if (boost_load_timer >= boost_load_timer_limit) {
                restartCoup_de_Burst();
                restartCoup_de_Burst_Load();
            }
            if (break_load_timer >= break_load_timer_limit) {
                restartHalt();
                restartHalt_Load();
            }
            player_speed = 50;
            player.wave_scale = 1;
        }

        if (left && !right)
            rot_step = -rot_step;
        else if (right && !left)
            rot_step = rot_step;
        else
            rot_step = 0;

        player_speed = clamp(player_speed, 0.0f, MAX_SPEED);
        sf::Vector2f move_step =
            sf::Vector2f(cos_rot * player_speed, sin_rot * player_speed);
        float rotation = wrap(player.getRotation() + rot_step, 360.0f);

        // Spawn these mother-duckers
        spawnEnemy_timer =
            clamp(spawnEnemy_timer + delta, 0.0f, spawnEnemy_timer_limit);
        if (canSpawnEnemy()) {
            int dir = rand() % 4;
            int x, y;
            int where = rand() % 10;
            switch (dir) {
            case 1:
                x = WIDTH / 10 * where;
                y = -100;
                break;
            case 3:
                x = WIDTH + 100;
                y = HEIGHT / 10 * where;
                break;
            case 2:
                x = WIDTH / 10 * where;
                y = HEIGHT + 100;
                break;
            default:
            case 0:
                x = -100;
                y = HEIGHT / 10 * where;
                break;
            }
            float dist_x = WIDTH - 2 * x;
            float dist_y = HEIGHT - 2 * y;
            float dist = sqrt(dist_x * dist_x + dist_y * dist_y);
            float sin_e = dist_y / dist;
            float cos_e = dist_x / dist;
            float Enemy_rot = 1 / DEG_TO_RADS;
            if (dir == 0 || (where < 5 && (dir == 1 || dir == 2)))
                Enemy_rot = Enemy_rot * asin(sin_e) - 180;
            else
                Enemy_rot *= asin(-sin_e);
            spawnEnemy(x, y, cos_e * ENEMY_SPEED, sin_e * ENEMY_SPEED,
                       Enemy_rot);
            restartSpawnerEnemy();
        }

        checkCollision();
        for (int i = numBalls - 1; i >= 0; i--) {
            balls[i]->update(move_step, delta);
            if (balls[i]->isDead() || outsideView(balls[i]->getPosition())) {
                *balls[i] = *balls[numBalls - 1];
                delete balls[numBalls - 1];
                numBalls--;
            }
        }
        for (int i = numBarrels - 1; i >= 0; i--) {
            barrels[i]->update(move_step, delta);
            if (barrels[i]->isDead() ||
                outsideView(barrels[i]->getPosition())) {
                *barrels[i] = *barrels[numBarrels - 1];
                delete barrels[numBarrels - 1];
                numBarrels--;
            }
        }
        for (int i = numEnemies - 1; i >= 0; i--) {
            enemies[i]->update(move_step, delta);
            if (enemies[i]->isDead()) {
                points += 11;
                kills++;
            }
            float rot_e = enemies[i]->getRotation() * 3.14159265 / 180;
            float px_e = enemies[i]->getPosition().x;
            float py_e = enemies[i]->getPosition().y;
            float sin_rot_e = sin(rot_e);
            float cos_rot_e = cos(rot_e);
            if (enemies[i]->canShoot()) {
                fireCannons(px_e, py_e, sin_rot_e, cos_rot_e);
                enemies[i]->restartCannon();
            }
            if (enemies[i]->canPutBarrel()) {
                putBarrel(px_e, py_e, sin_rot_e, cos_rot_e);
                enemies[i]->restartPutBarrel();
            }
            if (enemies[i]->isDead() ||
                outsideView(enemies[i]->getPosition())) {
                *enemies[i] = *enemies[numEnemies - 1];
                delete enemies[numEnemies - 1];
                numEnemies--;
            }
        }
        player.update(rotation, delta);
    }
    string show_time(int temp) {
        int tab[3];
        string time;
        tab[0] = temp%60;
        temp = temp/60;
        tab[1] = temp%60;
        temp = temp/60;
        tab[2] = temp;
        time = to_string(tab[2]) + ":";// + to_string(tab[1]) + ":" + to_string(tab[0]);
        if(tab[1] < 10) time += "0" + to_string(tab[1]) + ":";
        else time += to_string(tab[1]) + ":";
        if(tab[0] < 10) time += "0" + to_string(tab[0]);
        else time += to_string(tab[0]);

        return time;
    }
    void draw() {
        window.clear(sf::Color(0, 128, 183));
        for (int i = numBalls - 1; i >= 0; i--)
            balls[i]->draw(window);
        for (int i = numEnemies - 1; i >= 0; i--)
            enemies[i]->draw(window);
        for (int i = numBarrels - 1; i >= 0; i--)
            barrels[i]->draw(window);
        player.draw(window);

        sf::Text points_text("SCORE:  " + to_string(points), FONT);
        points_text.setFillColor(sf::Color::Yellow);
        points_text.setPosition(10, 0);
        window.draw(points_text);

        sf::Text time_text("SURVIVAL TIME:  " + show_time(TIME), FONT);
        time_text.setFillColor(sf::Color(255, 104, 4));
        time_text.setPosition(WIDTH / 2 - 170, 0);
        window.draw(time_text);

        sf::Text kills_text("KILLS:  " + to_string(kills), FONT);
        kills_text.setFillColor(sf::Color::Black);
        kills_text.setPosition(WIDTH - 180, 0);
        window.draw(kills_text);

        if (credits_timer < credits_timer_limit) {
            sf::RectangleShape intro_bg(sf::Vector2f(WIDTH, HEIGHT));
            intro_bg.setFillColor(sf::Color(
                0, 0, 0, 1 - (credits_timer * 255 / credits_timer_limit)));
            window.draw(intro_bg);

            sf::Text credits_text("        SHIPS by\n Admexter & Shadlock", FONT);
            credits_text.setFillColor(sf::Color::White);
            credits_text.setStyle(sf::Text::Bold);
            credits_text.setCharacterSize(60);
            credits_text.setPosition(WIDTH / 4, HEIGHT / 4 - 60);
            window.draw(credits_text);

            sf::Text instr_text("LEFT or A and RIGHT or steer\n           SPACE to shoot\n           UP or W to boost", FONT);
            instr_text.setFillColor(sf::Color::White);
            //instr_text.setStyle(sf::Text::Bold);
            instr_text.setCharacterSize(40);
            instr_text.setPosition(WIDTH / 4, HEIGHT / 4 * 3);
            window.draw(instr_text);
        }

        window.display();
    }
    ~World() { clearEntities(); }
};

bool rank_sorter(pair<string, long long int> p1,
                 pair<string, long long int> p2) {
    return p1.second > p2.second;
}

void ranking(long long int points, long int kills, string Time) {
    vector<pair<string, long long int>> ranking;
    ifstream ifile;
    ifile.open("ranking.rnk");
    if (ifile.is_open()) {
        for (int i = 0; i < 10; i++) {
            string name, score_str;
            long long int score;
            getline(ifile, name);
            getline(ifile, score_str);
            score = atoll(score_str.c_str());
            ranking.push_back(pair<string, long long int>(name, score));
        }
    }
    ifile.close();

    cout << "NICKNAME: ";
    string name;
    getline(cin, name);
    cout << "Dead man tell no tales." << endl
         << name << " gained score: " << points << endl
         <<"got kills: " << kills << endl
         <<"and survived: " + Time << endl
         << "Game over" << endl
         << endl;

    ranking.push_back(pair<string, long long int>(name, points));

    sort(ranking.begin(), ranking.end(), rank_sorter);

    // Print scores to terminal
    int i = 1;
    for (pair<string, long long int> score : ranking) {
        cout << i++ << ". " << score.first << ": " << score.second << endl;
    }

    cout << "Press any letter then enter to end, because c++ sucks..." << endl
         << endl;
    string c;
    cin >> c;

    ofstream ofile;
    ofile.open("ranking.rnk");
    // Save to file
    for (int i = 0; i < 10; i++) {
        ofile << ranking[i].first << endl << ranking[i].second << endl;
    }
    ofile.close();
}

int main() {
    srand(time(NULL));
    init_assets();
    const int width = 1100, height = 750;
    World zaWarudo(width, height);
    sf::Clock timer = sf::Clock();
    while (zaWarudo.isWindowOpen()) {
        float delta = timer.restart().asSeconds();
        zaWarudo.update(delta);
        zaWarudo.draw();

        if (zaWarudo.isPlayerDead()) {
            zaWarudo.WindowClose();
        }
        sf::sleep(sf::seconds(clamp(0.016 - delta, 0., 0.016)));
    }
    ranking(zaWarudo.getPoints(), zaWarudo.getKills(), zaWarudo.show_time(zaWarudo.getTime()));
    return 0;
}

// koniec
