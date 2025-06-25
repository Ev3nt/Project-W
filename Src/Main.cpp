#include <iostream>
#include <raylib.h>
#include <fstream>
#include "BLPReader.h"

Texture2D LoadBlpTexture(const std::string& fileName) {
    Image image = {
        .mipmaps = 1
    };

    int channels;
    DataChunk data = LoadBLP(fileName, image.width, image.height, channels);

    image.data = data.data();
    image.format = channels == 4 ? PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 : PIXELFORMAT_UNCOMPRESSED_R8G8B8;
   
    return LoadTextureFromImage(image);
}

Texture2D LoadTextureEx(const std::string& fileName) {
    if (fileName.empty()) {
        throw std::runtime_error("Filename is empty");
    }

    Texture2D texture = LoadTexture(fileName.c_str());
    if (IsTextureValid(texture)) {
        return texture;
    }

    texture = LoadBlpTexture(fileName);
    if (IsTextureValid(texture)) {
        return texture;
    }

    throw std::runtime_error("Failed to load texture: Unsupported or corrupted format (" + fileName + ")");
}

int main() {
    try {
        InitWindow(800, 600, "3D Model");
        SetTargetFPS(160);

        Camera3D camera = {
            .position = { 10.0f, 10.0f, 10.0f },
            .target = { 0.0f, 0.0f, 0.0f },
            .up = { 0.0f, 1.0f, 0.0f },
            .fovy = 45.0f,
            .projection = CAMERA_PERSPECTIVE
        };

        Model model = LoadModel("resources/peon/peon.obj");
        Texture2D texture1 = LoadTextureEx("resources/peon/peon.blp");
        Texture2D texture2 = LoadTextureEx("resources/peon/gutz.blp");
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;
        model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;
        model.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = texture2;

        /*Model model = LoadModel("resources/Load-Multiplayer-Human.obj");
        Texture2D texture1 = LoadTextureEx("resources/Loading-BotLeft.blp");
        Texture2D texture2 = LoadTextureEx("resources/Loading-Human-BotRight.blp");
        Texture2D texture3 = LoadTextureEx("resources/Loading-Human-TopRight.blp");
        Texture2D texture4 = LoadTextureEx("resources/Loading-TopLeft.blp");
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;
        model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture2;
        model.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = texture3;
        model.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = texture4;*/

        int xCount = 1;
        int yCount = 1;

        while (!WindowShouldClose()) {
            UpdateCamera(&camera, CAMERA_ORBITAL);

            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);


            for (int i = 0; i < xCount; i++) {
                for (int j = 0; j < yCount; j++) {
                    Vector3 position = { 0.5f * i - (float(xCount) / 2 * 0.5f), 0.f, 0.5f * j - (float(yCount) / 2 * 0.5f) };
                    DrawModel(model, position, 0.05f, WHITE);
                }
            }
            DrawGrid(10, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
            EndDrawing();
        }

        UnloadModel(model);
        UnloadTexture(texture1);
        UnloadTexture(texture2);
        //UnloadTexture(texture3);
        //UnloadTexture(texture4);
        CloseWindow();
    }
    catch (const std::exception& exception) {
        std::cout << exception.what() << '\n';
    }

    return 0;
}