#include <glad/gl.h>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>
#include <algorithm>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "shader_utils.h"
#include "skybox.h"
#include "camera.h"
#include "callbacks.h"
#include "terrain_manager.h"
#include "texture_loader.h"
#include "renderer.h"
#include "animate.h"
#include "scene_manager.h"
#include "glm_compat.h"

using namespace std;
using namespace glm;

int width = 1920, height = 1080;

Camera camera(100.0f, 10.0f, 130.0f,
              0.0f, 1.0f, 0.0f,
              180.0f,
              -15.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow *window = glfwCreateWindow(width, height, "Desert Colony", nullptr, nullptr);

    if (!window)
    {
        cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    gCamera = &camera;
    gLastX = width / 2.0f;
    gLastY = height / 2.0f;

    Skybox skybox;
    skybox.init("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    GLuint terrainProgram = CompileShaders("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");
    GLuint meshProgram = CompileShaders("assets/shaders/desert.vert", "assets/shaders/desert.frag");

    if (terrainProgram == 0)
    {
        cerr << "ERROR: Terrain shader failed to compile!\n";
        return -1;
    }

    if (meshProgram == 0)
    {
        cerr << "ERROR: Mesh shader failed to compile!\n";
        return -1;
    }

    TerrainManager terrainManager;
    if (!terrainManager.initialize(256, 1000.0f, 50.0f,
                                    "assets/textures/dune_heightmap-1.jpg",
                                    "assets/textures/sand_dark.jpg"))
    {
        cerr << "Failed to initialize terrain!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto sampleH = [&](float x, float z) -> float
    {
        return terrainManager.getHeightAt(x, z);
    };

    setupScene(meshProgram);

    // Add Hierarchical Meshes
    addHierarchicalMesh("assets/models/ice-worm/source/AnimatedWorm/AnimatedWorm.fbx"); // 0
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 1
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 2
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 3
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 4
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 5
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 6
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 7
    addHierarchicalMesh("assets/models/gas_mask/scene.gltf"); // 8
    addHierarchicalMesh("assets/models/mechanical_girl.glb"); // 9
    addHierarchicalMesh("assets/models/worm_monster/scene.gltf"); // 10
    addHierarchicalMesh("assets/models/worm_monster/scene.gltf"); // 11
    addHierarchicalMesh("assets/models/worm_monster/scene.gltf"); // 12
    addHierarchicalMesh("assets/models/worm_monster/scene.gltf"); // 13
    addHierarchicalMesh("assets/models/worm_monster/scene.gltf"); // 14

    // Set fighter moving around small worms
    setOrbitalMotion(10, 1);
    setOrbitalMotion(11, 3);
    setOrbitalMotion(12, 4);
    setOrbitalMotion(13, 7);
    setOrbitalMotion(14, 9);

    // Add regular meshes
    addMesh("assets/models/ruined_city.glb");                                 // 0
    addMesh("assets/models/cactus.glb");                                      // 1
    addMesh("assets/models/cherry_tree.glb");                                 // 2
    addMesh("assets/models/a_road_trip_in_2016_-_whale_class_transport.glb"); // 3
    addMesh("assets/models/cactus.glb");                                      // 4
    addMesh("assets/models/v-19_torrent_-_star_wars_-_clone_wars.glb");       // 5
    addMesh("assets/models/ruined_city2.glb");                                // 6

    // ZONE 1: City ruins cluster
    float city1X = 0.0f;
    float city1Z = 0.0f;
    float city1Y = sampleH(city1X, city1Z);

    float city2X = city1X - 25.0f;
    float city2Z = city1Z - 10.0f;
    float city2Y = sampleH(city2X, city2Z);

    setMeshTransform(0, vec3(city1X, city1Y, city1Z), vec3(0, 45, 0), vec3(7, 7, 7));              // ruined_city - CENTER
    setMeshTransform(6, vec3(city2X, city2Y, city2Z), vec3(0, -30, 0), vec3(6, 6, 6));             // ruined_skyscrapers
    setMeshTransform(2, vec3(city1X, city1Y + 60.0f, city1Z), vec3(0, 0, 0), vec3(5, 5, 5), true); // cherry_tree floating over first city

    // ZONE 2: Vegetation
    setMeshTransform(1, vec3(city1X + 50.0f, sampleH(city1X + 50.0f, city1Z - 30.0f), city1Z - 30.0f), vec3(0, 0, 0), vec3(4, 4, 4));
    setMeshTransform(4, vec3(city1X + 70.0f, sampleH(city1X + 50.0f, city1Z - 30.0f), city1Z - 30.0f), vec3(0, 0, 0), vec3(4, 4, 4));

    // ZONE 3: Combat ships
    float wormBurrowX = city1X + 120.0f;
    float wormBurrowZ = city1Z + 90.0f;

    // Ship 1 - whale transport
    float ship1X = city1X + 100.0f;
    float ship1Z = city1Z + 35.0f;
    vec3 toBurrow1 = vec3(wormBurrowX - ship1X, 0.0f, wormBurrowZ - ship1Z);
    float yaw1 = atan2(toBurrow1.x, toBurrow1.z) * 180.0f / 3.14159f;
    setMeshTransform(3, vec3(ship1X, sampleH(ship1X, ship1Z) + 30.0f, ship1Z), vec3(-10, yaw1, 0), vec3(4, 4, 4), true); // whale transport

    // Ship 2 - torrent
    float ship2X = city1X + 50.0f;
    float ship2Z = city1Z + 5.0f;
    vec3 toBurrow2 = vec3(wormBurrowX - ship2X, 0.0f, wormBurrowZ - ship2Z);
    float yaw2 = atan2(toBurrow2.x, toBurrow2.z) * 180.0f / 3.14159f;
    setMeshTransform(5, vec3(ship2X, sampleH(ship2X, ship2Z) + 20.0f, ship2Z), vec3(-8, yaw2, 0), vec3(3, 3, 3), true); // torrent

    float degree_x = 90;
    float degree_y = 0;
    float degree_z = 90;

    // Mother Worm
    setHierarchicalMeshTransform(0,
                                    vec3(wormBurrowX - 20.0f, sampleH(wormBurrowX - 20.0f, wormBurrowZ) + 0.5f, wormBurrowZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(0.025f, 0.025f, 0.025f), true);
    setHierarchicalActiveAnimation(0, 8);

    // Fighter & Worm Pairs
    float pair1BaseX = city1X + 50.0f;
    float pair1BaseZ = city1Z + 50.0f;
    setHierarchicalMeshTransform(1,
                                    vec3(pair1BaseX + 5.0f, sampleH(pair1BaseX + 5.0f, pair1BaseZ) + 0.5f, pair1BaseZ),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(1, 1);
    setHierarchicalMeshTransform(10,
                                    vec3(pair1BaseX, sampleH(pair1BaseX, pair1BaseZ) + 0.5f, pair1BaseZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(5.0f, 5.0f, 5.0f), true);
    setHierarchicalActiveAnimation(10, 2);

    float pair2BaseX = city1X + 80.0f;
    float pair2BaseZ = city1Z + 60.0f;
    setHierarchicalMeshTransform(3,
                                    vec3(pair2BaseX + 5.0f, sampleH(pair2BaseX + 5.0f, pair2BaseZ) + 0.5f, pair2BaseZ),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(3, 1);
    setHierarchicalMeshTransform(11,
                                    vec3(pair2BaseX, sampleH(pair2BaseX, pair2BaseZ) + 0.5f, pair2BaseZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(5.0f, 5.0f, 5.0f), true);
    setHierarchicalActiveAnimation(11, 5);

    float pair3BaseX = city1X + 100.0f;
    float pair3BaseZ = city1Z + 270.0f;
    setHierarchicalMeshTransform(4,
                                    vec3(pair3BaseX + 5.0f, sampleH(pair3BaseX + 5.0f, pair3BaseZ) + 0.5f, pair3BaseZ),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(4, 1);
    setHierarchicalMeshTransform(12,
                                    vec3(pair3BaseX, sampleH(pair3BaseX, pair3BaseZ) + 0.5f, pair3BaseZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(5.0f, 5.0f, 5.0f), true);
    setHierarchicalActiveAnimation(12, 1);

    float pair4BaseX = city1X + 130.0f;
    float pair4BaseZ = city1Z + 300.0f;
    setHierarchicalMeshTransform(7,
                                    vec3(pair4BaseX + 5.0f, sampleH(pair4BaseX + 5.0f, pair4BaseZ) + 0.5f, pair4BaseZ),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(7, 2);
    setHierarchicalMeshTransform(13,
                                    vec3(pair4BaseX, sampleH(pair4BaseX, pair4BaseZ) + 0.5f, pair4BaseZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(5.0f, 5.0f, 5.0f), true);
    setHierarchicalActiveAnimation(13, 2);

    float pair5BaseX = city1X + 150.0f;
    float pair5BaseZ = city1Z + 150.0f;
    setHierarchicalMeshTransform(9,
                                    vec3(pair5BaseX + 5.0f, sampleH(pair5BaseX + 5.0f, pair5BaseZ) + 0.5f, pair5BaseZ),
                                    vec3(-90, 0, 0), vec3(200.0f, 200.0f, 200.0f), true);
    setHierarchicalActiveAnimation(9, 3);
    setHierarchicalMeshTransform(14, // Worm 14 (child - will orbit Fighter 9)
                                    vec3(pair5BaseX, sampleH(pair5BaseX, pair5BaseZ) + 0.5f, pair5BaseZ),
                                    vec3(degree_x, degree_y, degree_z), vec3(2.0f, 2.0f, 2.0f), true);
    setHierarchicalActiveAnimation(14, 2);

    // Additional random worms
    setHierarchicalMeshTransform(2,
                                    vec3(city1X - 150.0f, sampleH(city1X + 30.0f, city1Z + 40.0f) + 0.5f, city1Z + 40.0f),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(2, 6);

    setHierarchicalMeshTransform(5,
                                    vec3(city1X - 120.0f, sampleH(city1X + 60.0f, city1Z + 80.0f) + 0.5f, city1Z + 80.0f),
                                    vec3(-90, 0, 0), vec3(20.0f, 20.0f, 20.0f), true);
    setHierarchicalActiveAnimation(5, 5);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    lastFrame = (float)glfwGetTime();

    static float timeOfDay = 12.0f;
    static float targetTimeOfDay = 12.0f;
    static bool isTransitioning = false;
    static float transitionProgress = 0.0f;
    static float startTimeOfDay = 12.0f;

    const float dayTime = 12.0f;
    const float nightTime = 0.0f;
    const float transitionDuration = 2.0f;

    static bool pointLightsEnabled = true;
    static bool spotLightsEnabled = true;
    static bool togglePointLightsKeyPressed = false;
    static bool toggleSpotLightsKeyPressed = false;


    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        // Toggle day/night on 'N' key press
        static bool nKeyPressed = false;
        int nKeyState = glfwGetKey(window, GLFW_KEY_N);

        if (nKeyState == GLFW_PRESS)
        {
            if (!nKeyPressed && !isTransitioning)
            {
                startTimeOfDay = timeOfDay;

                if (timeOfDay >= 6.0f && timeOfDay < 18.0f)
                {

                    targetTimeOfDay = nightTime;
                }
                else
                {
                    targetTimeOfDay = dayTime;
                }

                isTransitioning = true;
                transitionProgress = 0.0f;
                nKeyPressed = true;
            }
        }
        else
        {
            nKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            if (!togglePointLightsKeyPressed)
            {
                pointLightsEnabled = !pointLightsEnabled;
                togglePointLightsKeyPressed = true;
            }
        }
        else
        {
            togglePointLightsKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            if (!toggleSpotLightsKeyPressed)
            {
                spotLightsEnabled = !spotLightsEnabled;
                toggleSpotLightsKeyPressed = true;
            }
        }
        else
        {
            toggleSpotLightsKeyPressed = false;
        }

        if (isTransitioning)
        {
            transitionProgress += deltaTime / transitionDuration;

            if (transitionProgress >= 1.0f)
            {
                transitionProgress = 1.0f;
                timeOfDay = targetTimeOfDay;
                isTransitioning = false;
            }
            else
            {
                float easedProgress = transitionProgress;
                timeOfDay = startTimeOfDay + (targetTimeOfDay - startTimeOfDay) * easedProgress;
            }
        }

        processInput(window, camera, deltaTime);

        float terrainHeight = terrainManager.getHeightAt(camera.position.x, camera.position.z);
        float cameraHeightAboveTerrain = 3.0f;
        camera.position.y = terrainHeight + cameraHeightAboveTerrain;

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        if (fbHeight == 0)
            fbHeight = 1;

        glViewport(0, 0, fbWidth, fbHeight);

        DayNightParams dayNight = calculateDayNightCycle(timeOfDay);
        glClearColor(dayNight.skyZenithColor.x, dayNight.skyZenithColor.y, dayNight.skyZenithColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.setHorizonColor(dayNight.skyHorizonColor);
        skybox.setZenithColor(dayNight.skyZenithColor);

        if (dayNight.sunDir.y > -0.2f)
        {
            skybox.setSun1(dayNight.sunDir, dayNight.sunColor);
            skybox.setSunSize(500.0f);
        }
        else
        {
            vec3 moonDir = vec3(dayNight.sunDir.x, -dayNight.sunDir.y * 0.3f, dayNight.sunDir.z);
            skybox.setSun1(moonDir, dayNight.sunColor * 0.5f);
            skybox.setSunSize(1200.0f);
        }

        static float walkingTime = 0.0f;
        static vec3 lastCameraPos = camera.position;
        vec3 horizontalMovement = vec3(camera.position.x - lastCameraPos.x, 0.0f, camera.position.z - lastCameraPos.z);
        bool isMoving = length(horizontalMovement) > 0.001f;

        vec3 viewPosition = camera.position;
        if (isMoving)
        {
            walkingTime += deltaTime;

            float bobAmount = 0.15f;
            float bobSpeed = 8.0f;

            float verticalBob = sin(walkingTime * bobSpeed) * bobAmount;
            float horizontalSway = cos(walkingTime * bobSpeed * 0.5f) * 0.05f;

            viewPosition.y += verticalBob;
            viewPosition += camera.right * horizontalSway;
        }
        else
        {
            walkingTime = 0.0f;
        }

        lastCameraPos = camera.position;

        // Calculate matrices with bob offset
        vec3 target = viewPosition + camera.front;
        mat4 view = lookAt(viewPosition, target, camera.up);
        mat4 proj = perspective(glm::radians(camera.zoom), (float)fbWidth / (float)fbHeight, 0.1f, 5000.0f);


        skybox.render(view, proj);

        if (!hierarchicalModels.empty() && hierarchicalModels.size() > 0)
        {
            static float leviathanSpawnTime = 0.0f;
            static bool leviathanHasSpawned = false;

            float spawnDelay = 2.0f;
            float burrowDuration = 3.0f;

            leviathanSpawnTime += deltaTime;

            if (leviathanSpawnTime >= spawnDelay && !leviathanHasSpawned)
            {
                HierarchicalModel &leviathan = hierarchicalModels[0];

                float finalX = wormBurrowX - 20.0f;
                float finalZ = wormBurrowZ;
                float finalY = sampleH(finalX, finalZ) + 0.5f;

                float startY = finalY - 15.0f;

                float elapsed = leviathanSpawnTime - spawnDelay;
                float progress = std::min(1.0f, elapsed / burrowDuration);

                float easedProgress = 1.0f - pow(1.0f - progress, 3.0f);

                float currentY = startY + (finalY - startY) * easedProgress;

                leviathan.worldPosition = vec3(finalX, currentY, finalZ);

                if (progress >= 1.0f)
                {
                    leviathanHasSpawned = true;
                }
            }
        }

        // Update ship positions to hover over leviathan
        if (!hierarchicalModels.empty() && hierarchicalModels.size() > 0)
        {

            vec3 leviathanPos = hierarchicalModels[0].worldPosition;
            float leviathanHeight = 10.0f;

            float hoverRadius = 40.0f;
            float hoverHeight = leviathanPos.y + leviathanHeight + 25.0f;

            static float ship1OrbitAngle = 0.0f;
            ship1OrbitAngle += deltaTime * 0.2f;
            vec3 ship1HoverPos = vec3(
                leviathanPos.x + cos(ship1OrbitAngle) * hoverRadius,
                hoverHeight,
                leviathanPos.z + sin(ship1OrbitAngle) * hoverRadius);

            float ship2OrbitAngle = ship1OrbitAngle + 3.14159f;
            vec3 ship2HoverPos = vec3(
                leviathanPos.x + cos(ship2OrbitAngle) * hoverRadius,
                hoverHeight,
                leviathanPos.z + sin(ship2OrbitAngle) * hoverRadius);

            vec3 toLeviathan1 = normalize(leviathanPos - ship1HoverPos);
            vec3 toLeviathan2 = normalize(leviathanPos - ship2HoverPos);
            float yaw1 = atan2(toLeviathan1.x, toLeviathan1.z) * 180.0f / 3.14159f;
            float yaw2 = atan2(toLeviathan2.x, toLeviathan2.z) * 180.0f / 3.14159f;

            setMeshTransform(3, ship1HoverPos, vec3(-10, yaw1, 0), vec3(4, 4, 4), true); // Ship 1
            setMeshTransform(5, ship2HoverPos, vec3(-8, yaw2, 0), vec3(3, 3, 3), true);  // Ship 2
        }

        renderScene(deltaTime, view, proj, meshProgram, camera.position, timeOfDay);

        static float lightTime = 0.0f;
        lightTime += deltaTime;
        updateDynamicLights(meshProgram, lightTime, camera.position, pointLightsEnabled, spotLightsEnabled);

        renderHierarchicalMeshes(deltaTime, view, proj, meshProgram, camera.position, timeOfDay);

        mat4 model = identity_mat4();
        terrainManager.render(terrainProgram, model, view, proj, camera.position, timeOfDay, 0.000016f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    terrainManager.cleanup();
    cleanupScene();
    skybox.cleanup();
    glDeleteProgram(meshProgram);
    glDeleteProgram(terrainProgram);
    glfwTerminate();

    return 0;

}
