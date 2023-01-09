#include <raylib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "../raygui.h"
#include "GameMap.h"
#include "visualizeNN.h"

class TrainAI {
    public:
        TrainAI();
        ~TrainAI();
        void run();
    private:
        void setCar();
        void checkCar();
        void render();
        void start();
        void update(double deltaTime);
        // const int screenWidth;
        int screenWidth;
        int screenHeight;
        int fps;
        bool server;
        float epsilon;
        int generation;
        std::vector<Car> cars;
        std::vector<double> previousMove;
        GameMapE* map;
        Car* car;
        nlohmann::json mapData;

};

TrainAI::TrainAI() {
    screenWidth = 1980;
    screenHeight = 1024;
    server = false;
    epsilon = 0;
    generation = 0;
    fps = 60;

    map = new GameMapE();
    car = new Car(map, 3, {200, 200});

    std::ifstream f("maps/example.json");
    mapData = nlohmann::json::parse(f);
    f.close();
}

TrainAI::~TrainAI() {
    delete car;
    delete map;
}

void TrainAI::setCar() {
    int value2 = mapData["spawn"]["lenght"].get<int>();
    int value = rand() % value2;
    car->restartLocation(mapData["direction"][std::to_string(value)].get<float>(), mapData["spawn"][std::to_string(value)]["firstcheckpoint"].get<float>(), map->spawns.at(value), epsilon);
}

void TrainAI::checkCar() {
    if (!car->alive) {
        setCar();
        generation++;
        if (generation >= 2000) {
            epsilon -= 0.1;
            if (epsilon < 0) {
                epsilon = 0;
            }
            generation = 0;
        }
    }
}

void TrainAI::render() {
    const Color backgroundColor = BLACK;

    BeginDrawing();
    ClearBackground(backgroundColor);

    map->draw();
    car->draw(false);
    DrawText("ja", 600, 540, 10, BLACK);
    DrawText(TextFormat("%i", generation), 10, 40, 20, WHITE);
    DrawText(TextFormat("%f", epsilon), 10, 60, 20, WHITE);
    DrawText(TextFormat("%i", car->currentPoints), 10, 80, 20, WHITE);
    VisualiseNN visualiseNetwork = VisualiseNN(car);
    visualiseNetwork.DrawNeuralNetwork();
    DrawFPS(10,10);
    EndDrawing();

    checkCar();
}

void TrainAI::start() {
    map->setMap(mapData);
    setCar();
}

void TrainAI::update(double deltaTime) {
    std::ifstream f("NeuralNetworks/NN.json");
    nlohmann::json networkData = nlohmann::json::parse(f);
    f.close();
    if (IsKeyPressed(KEY_LEFT_CONTROL)) {
        car->neuralNetwork.saveNeuralNetwork();
    }
    if (IsKeyPressed(KEY_RIGHT_CONTROL)) {
        car->neuralNetwork.loadNeuralNetwork(networkData);
        setCar();
    }
    
    car->update(1.0f/60.0f);
}   

void TrainAI::run() {
    srand(time(NULL));
    
    InitWindow(screenWidth, screenHeight, "train Ai");
    SetTargetFPS(fps);
    start();
    while (!WindowShouldClose()){
        update(float(1.0/60.0));
        render();
    }
    
    CloseWindow();
}