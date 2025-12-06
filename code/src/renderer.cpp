#include <cmath>
#include <map>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "scene_manager.h"
#include "mesh_loader.h"
#include "model_render.h"
#include "hierarchy_utils.h"
#include "glm_compat.h"
#include "shader_uniform.h"
#include "animate.h"
#include "transform_utils.h"
#include "animator.h"

using namespace std;
using namespace glm;

static map<size_t, Animator *> animators;

static float fogDensity = 0.01f;
static float fogStart = 200.0f;
static float fogEnd = 1000.0f;
static float fogHeight = 0.0f;
static float fogHeightRange = 60.0f;

static float heatDelta = 0.0f;

void renderScene(float delta, const mat4 &view, const mat4 &proj, GLuint shaderProgramID, const vec3 &cameraPos, float timeOfDay){
    if (meshes.empty())
        return;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);

    static ModelRenderer *renderer = nullptr;

    if (!renderer)
    {
        renderer = new ModelRenderer(shaderProgramID);
    }

    DayNightParams dayNight = calculateDayNightCycle(timeOfDay);

    renderer->setViewProjection(view, proj, cameraPos);
    renderer->setMaterial(dayNight.ambientColor.x, dayNight.ambientColor.y, dayNight.ambientColor.z, 0.35f, 32.0f);
    renderer->setLighting(dayNight.lightDir, dayNight.lightColor * dayNight.lightIntensity);

    heatDelta += delta;

    renderer->setFog(dayNight.fogColor, fogDensity, fogStart, fogEnd, fogHeight, fogHeightRange);
    renderer->setTime(heatDelta);
    renderer->setUseHeatShimmer(true);
    renderer->setHeatShimmerIntensity(1.5f);

    renderer->renderMeshes(meshes, meshTransforms, delta, view, proj);
};

void renderHierarchicalMeshes(float delta, const mat4 &view, const mat4 &proj, GLuint shaderProgramID, const vec3 &cameraPos, float timeOfDay){
    if (hierarchicalModels.empty())
    {
        return;
    }

    glUseProgram(shaderProgramID);

    static ModelRenderer *renderer = nullptr;
    if (!renderer)
    {
        renderer = new ModelRenderer(shaderProgramID);
    }

    DayNightParams dayNight = calculateDayNightCycle(timeOfDay);

    renderer->setViewProjection(view, proj, cameraPos);
    renderer->setMaterial(dayNight.ambientColor.x, dayNight.ambientColor.y, dayNight.ambientColor.z, 0.35f, 32.0f);
    renderer->setLighting(dayNight.lightDir, dayNight.lightColor * dayNight.lightIntensity);

    renderer->setFog(dayNight.fogColor, fogDensity, fogStart, fogEnd, fogHeight, fogHeightRange);

    heatDelta += delta;

    renderer->setTime(heatDelta);
    renderer->setUseHeatShimmer(true);
    renderer->setHeatShimmerIntensity(1.5f);

    for (size_t modelIdx = 0; modelIdx < hierarchicalModels.size(); modelIdx++)
    {
        HierarchicalModel &hmodel = hierarchicalModels[modelIdx];

        if (!hmodel.rootNode)
            continue;

        if (hmodel.useOrbitalMotion &&
            hmodel.orbitalParentIdx == (int)modelIdx &&
            hmodel.orbitalChildIdx >= 0 &&
            hmodel.orbitalChildIdx < (int)hierarchicalModels.size())
        {
            updateOrbitalMotion(
                hierarchicalModels[hmodel.orbitalParentIdx],
                hierarchicalModels[hmodel.orbitalChildIdx],
                delta,
                15.0f,
                0.5f,
                5.0f
            );
        }

        static map<size_t, float> animationTimes;

        if (animationTimes.find(modelIdx) == animationTimes.end())
        {
            animationTimes[modelIdx] = 0.0f;
        }
        animationTimes[modelIdx] += delta;
        float animTime = animationTimes[modelIdx];

        if (animators.find(modelIdx) == animators.end())
        {
            animators[modelIdx] = new Animator(&hmodel);
            if (hmodel.activeAnimation >= 0 &&
                hmodel.activeAnimation < (int)hmodel.animationClips.size())
            {
                animators[modelIdx]->setActiveAnimation(hmodel.activeAnimation);
            }
        }

        Animator *animator = animators[modelIdx];

        if (hmodel.activeAnimation >= 0 &&
            hmodel.activeAnimation != animator->getCurrentAnimation())
        {
            animator->setActiveAnimation(hmodel.activeAnimation);
        }

        if (hmodel.hasEmbeddedAnimation && hmodel.activeAnimation >= 0)
        {
            animator->updateAnimation(delta);
            hmodel.boneMatricesPerMesh = animator->getFinalBoneMatrices();
        }
        else if (hmodel.animated)
        {
            animateNodeRecursive(hmodel.rootNode, animationTimes[modelIdx], 0, hmodel.nodes);
        }

        if (!hmodel.hasEmbeddedAnimation)
        {
            static map<size_t, vec3> basePositions;
            if (basePositions.find(modelIdx) == basePositions.end())
            {
                basePositions[modelIdx] = hmodel.worldPosition;
            }

            vec3 pos = hmodel.worldPosition;
            pos.x += cos(animTime) * 0.5f;
            pos.y += sin(animTime * 2.0f) * 0.3f;
            pos.z -= 2.0f * delta;

            mat4 worldTransform = identity_mat4();
            worldTransform = translate(worldTransform, pos);
            worldTransform = rotate_x_deg(worldTransform, hmodel.worldRotation.x);
            worldTransform = rotate_y_deg(worldTransform, hmodel.worldRotation.y);
            worldTransform = rotate_z_deg(worldTransform, hmodel.worldRotation.z);
            worldTransform = scale(worldTransform, hmodel.worldScale);

            renderer->renderHierarchicalModel(hmodel, worldTransform, view, proj);
        }
        else
        {
            mat4 worldTransform = identity_mat4();
            worldTransform = translate(worldTransform, hmodel.worldPosition);
            worldTransform = rotate_x_deg(worldTransform, hmodel.worldRotation.x);
            worldTransform = rotate_y_deg(worldTransform, hmodel.worldRotation.y);
            worldTransform = rotate_z_deg(worldTransform, hmodel.worldRotation.z);
            worldTransform = scale(worldTransform, hmodel.worldScale);

            renderer->renderHierarchicalModel(hmodel, worldTransform, view, proj);
        }
    }
};

void updateDynamicLights(GLuint shaderProgramID, float time, const vec3 &cameraPos, bool enablePointLights, bool enableSpotLights)
{
    static ModelRenderer *renderer = nullptr;
    if (!renderer)
    {
        renderer = new ModelRenderer(shaderProgramID);
    }

    glUseProgram(shaderProgramID);

    vec3 leviathanPos = vec3(0.0f, 0.0f, 0.0f);
    if (!hierarchicalModels.empty() && hierarchicalModels.size() > 0)
    {
        leviathanPos = hierarchicalModels[0].worldPosition;
    }

    vec3 ship1Pos = vec3(100.0f, 30.0f, 35.0f);
    vec3 ship2Pos = vec3(50.0f, 20.0f, -10.0f);

    if (!hierarchicalModels.empty() && hierarchicalModels.size() > 0)
    {
        float leviathanHeight = 10.0f;
        float hoverRadius = 40.0f;
        float hoverHeight = leviathanPos.y + leviathanHeight + 25.0f;

        static float ship1OrbitAngle = 0.0f;
        ship1OrbitAngle = fmod(time * 0.2f, 2.0f * 3.14159f);

        ship1Pos = vec3(
            leviathanPos.x + cos(ship1OrbitAngle) * hoverRadius,
            hoverHeight,
            leviathanPos.z + sin(ship1OrbitAngle) * hoverRadius);

        float ship2OrbitAngle = ship1OrbitAngle + 3.14159f;
        ship2Pos = vec3(
            leviathanPos.x + cos(ship2OrbitAngle) * hoverRadius,
            hoverHeight,
            leviathanPos.z + sin(ship2OrbitAngle) * hoverRadius);
    }
    else
    {

        if (meshTransforms.size() > 3)
        {
            ship1Pos = meshTransforms[3].position;
        }
        if (meshTransforms.size() > 5)
        {
            ship2Pos = meshTransforms[5].position;
        }
    }

    vec3 toLeviathan1 = normalize(leviathanPos - ship1Pos);
    vec3 toLeviathan2 = normalize(leviathanPos - ship2Pos);

    vector<vec3> pointPositions;
    vector<vec3> pointColors;
    vector<float> pointIntensities;
    vector<float> pointConstants;
    vector<float> pointLinears;
    vector<float> pointQuadratics;
    vector<float> pointRanges;

    // Ship 1 laser: Position at leviathan where it hits
    if (enablePointLights)
    {
        float weaponIntensity = abs(sin(time * 4.0f * 3.14159f));
        float daylightMultiplier = 20.0f;


        vec3 laser1HitPos = leviathanPos + vec3(0.0f, 3.0f, 0.0f);
        pointPositions.push_back(laser1HitPos);
        pointColors.push_back(vec3(1.0f, 0.0f, 0.0f));
        pointIntensities.push_back(weaponIntensity * 30.0f * daylightMultiplier);
        pointConstants.push_back(1.0f);
        pointLinears.push_back(0.09f);
        pointQuadratics.push_back(0.032f);
        pointRanges.push_back(350.0f);
    }

    renderer->setPointLights(pointPositions, pointColors, pointIntensities,
                             pointConstants, pointLinears, pointQuadratics, pointRanges);


    vector<vec3> spotPositions;
    vector<vec3> spotDirections;
    vector<vec3> spotColors;
    vector<float> spotIntensities;
    vector<float> spotConstants;
    vector<float> spotLinears;
    vector<float> spotQuadratics;
    vector<float> spotCutoffs;
    vector<float> spotOuterCutoffs;
    vector<float> spotRanges;

    // Ship 2 spotlight: Position below ship, direction towards leviathan
    if (enableSpotLights)
    {
        vec3 spotlightPos = ship2Pos + vec3(0.0f, -2.0f, 0.0f);
        vec3 spotlightDir = normalize(leviathanPos - spotlightPos);

        spotPositions.push_back(spotlightPos);
        spotDirections.push_back(spotlightDir);
        spotColors.push_back(vec3(0.2f, 0.8f, 1.0f));
        spotIntensities.push_back(300.0f);
        spotConstants.push_back(1.0f);
        spotLinears.push_back(0.09f);
        spotQuadratics.push_back(0.032f);
        spotCutoffs.push_back(cos(radians(20.0f)));
        spotOuterCutoffs.push_back(cos(radians(35.0f)));
        spotRanges.push_back(500.0f);
    }

    renderer->setSpotLights(spotPositions, spotDirections, spotColors,
                            spotIntensities, spotConstants, spotLinears, spotQuadratics,
                            spotCutoffs, spotOuterCutoffs, spotRanges);
};

void updateOrbitalMotion(HierarchicalModel &parentModel,
                         HierarchicalModel &childModel,
                         float delta,
                         float orbitRadius,
                         float orbitSpeed,
                         float forwardSpeed)
{
    static map<size_t, float> orbitAngles;
    static map<size_t, vec3> parentBasePositions;

    size_t parentIdx = parentModel.modelIndex;
    size_t childIdx = childModel.modelIndex;

    if (parentBasePositions.find(parentIdx) == parentBasePositions.end())
    {
        parentBasePositions[parentIdx] = parentModel.worldPosition;
    }

    if (orbitAngles.find(childIdx) == orbitAngles.end())
    {
        orbitAngles[childIdx] = 0.0f;
    }

    vec3 &parentBasePos = parentBasePositions[parentIdx];
    parentBasePos.z += forwardSpeed * delta;
    parentModel.worldPosition = parentBasePos;

    orbitAngles[childIdx] += orbitSpeed * delta;
    float angle = orbitAngles[childIdx];

    vec3 orbitOffset = vec3(
        cos(angle) * orbitRadius,
        0.0f,
        sin(angle) * orbitRadius
    );

    childModel.worldPosition = parentModel.worldPosition + orbitOffset;

};

DayNightParams calculateDayNightCycle(float timeOfDay)
{
    DayNightParams params;

    // (0 = midnight, 0.5 = noon, 1 = midnight)
    float normalizedTime = fmod(timeOfDay, 24.0f) / 24.0f;

    float sunAngle = (normalizedTime - 0.25f) * 2.0f * 3.14159f;
    float sunHeight = sin(sunAngle);

    float sunX = cos(sunAngle) * 0.9f;
    float sunY = sunHeight;
    float sunZ = sin(sunAngle) * 0.2f;

    params.sunDir = normalize(vec3(sunX, sunY, sunZ));
    params.lightDir = -params.sunDir;

    float dayFactor = std::max(0.0f, std::min(1.0f, (sunHeight + 0.2f) / 0.4f));

    if (dayFactor > 0.1f)
    {
        float sunHeightFactor = std::max(0.0f, std::min(1.0f, (sunHeight - 0.2f) / 0.6f));
        params.sunColor = mix(
            vec3(1.0f, 0.6f, 0.3f),
            vec3(1.0f, 0.95f, 0.8f),
            sunHeightFactor);
        params.lightColor = params.sunColor * (0.8f + 0.4f * dayFactor);
    }
    else
    {
        params.sunColor = vec3(0.8f, 0.85f, 1.0f) * 0.6f;
        params.lightColor = params.sunColor * 0.5f;
    }

    params.lightIntensity = 0.4f + 0.6f * dayFactor;
    params.ambientColor = mix(
        vec3(0.15f, 0.15f, 0.25f),
        vec3(0.25f, 0.22f, 0.2f),
        dayFactor);

    params.fogColor = mix(
        vec3(0.15f, 0.15f, 0.25f),
        vec3(0.93f, 0.83f, 0.68f),
        dayFactor);

    params.skyHorizonColor = mix(
        vec3(0.25f, 0.25f, 0.35f),
        vec3(0.95f, 0.85f, 0.70f),
        dayFactor);

    params.skyZenithColor = mix(
        vec3(0.1f, 0.12f, 0.2f),
        vec3(0.40f, 0.60f, 0.85f),
        dayFactor);

    return params;
};
