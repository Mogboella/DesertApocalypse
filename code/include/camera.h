#ifndef CAMERA_H
#define CAMERA_H

#include "glm_compat.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

enum Camera_Movement
{
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    DOWN,
    UP
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 10.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
    public:
        vec3 position;
        vec3 front;
        vec3 up;
        vec3 right;
        vec3 wrldup;

        float Yaw;
        float Pitch;

        float movementSpeed;
        float mouseSens;
        float zoom;

        bool isTransitioning;
        vec3 startPosition;
        vec3 targetPosition;
        float startYaw;
        float targetYaw;
        float startPitch;
        float targetPitch;
        float transitionProgress;
        float transitionDuration;

        Camera(
            vec3 pos = vec3(0.0f, 0.0f, 0.0f),
            vec3 up = vec3(0.0f, 1.0f, 0.0f),
            float yaw = YAW,
            float pitch = PITCH) : front(vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSens(SENSITIVITY), zoom(ZOOM),
                                    isTransitioning(false), transitionProgress(0.0f), transitionDuration(2.0f)
        {
            position = pos;
            wrldup = up;
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }

        Camera(float X, float Y, float Z, float upX, float upY, float upZ, float yaw, float pitch) : front(vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSens(SENSITIVITY), zoom(ZOOM),
                                                                                                        isTransitioning(false), transitionProgress(0.0f), transitionDuration(2.0f)
        {
            position = vec3(X, Y, Z);
            wrldup = vec3(upX, upY, upZ);
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }

        mat4 GetViewMatrix() const
        {
            vec3 target = position + front;
            return lookAt(position, target, up);
        }

        void ProcessKeyboard(Camera_Movement direction, float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            if (direction == FRONT)
                position += front * velocity;
            if (direction == BACK)
                position -= front * velocity;
            if (direction == LEFT)
                position -= right * velocity;
            if (direction == RIGHT)
                position += right * velocity;
            if (direction == UP)
                position += up * velocity;
            if (direction == DOWN)
                position -= up * velocity;
        }

        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
        {
            xoffset *= mouseSens;
            yoffset *= mouseSens;

            Yaw += xoffset;
            Pitch += yoffset;

            if (constrainPitch)
            {
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }

            updateCameraVectors();
        }

        void ProcessMouseScroll(float yoffset)
        {
            float zoomSpeed = 3.0f;
            position += front * yoffset * zoomSpeed;

            if (zoom < 30.0f)
                zoom = 30.0f;
            if (zoom > 60.0f)
                zoom = 60.0f;
        }

        void SetPresetPosition(int preset)
        {
            vec3 targetPos;
            float targetYawVal;
            float targetPitchVal;

            switch (preset)
            {
            case 0:
                targetPos = vec3(-60.0f, 90.0f, 150.0f);
                targetYawVal = -40.0f;
                targetPitchVal = 20.0f;
                break;
            case 1:
                targetPos = vec3(100.0f, 0.0f, 250.0f);
                targetYawVal = -100.0f;
                targetPitchVal = 30.0f;
                break;
            case 2:
                targetPos = vec3(100.0f, 0.0f, 250.0f);
                targetYawVal = -100.0f;
                targetPitchVal = 15.0f;
                break;
            case 3:
                targetPos = vec3(100.0f, 0.0f, 400.0f);
                targetYawVal = -130.0f;
                targetPitchVal = 20.0f;
                break;
            default:
                return;
            }


            startPosition = position;
            startYaw = Yaw;
            startPitch = Pitch;
            targetPosition = targetPos;
            targetYaw = targetYawVal;
            targetPitch = targetPitchVal;
            transitionProgress = 0.0f;
            isTransitioning = true;
        }

        void UpdateTransition(float deltaTime)
        {
            if (!isTransitioning)
                return;

            transitionProgress += deltaTime / transitionDuration;

            if (transitionProgress >= 1.0f)
            {
                transitionProgress = 1.0f;
                position = targetPosition;
                Yaw = targetYaw;
                Pitch = targetPitch;
                isTransitioning = false;
                updateCameraVectors();
            }
            else
            {
                float t = transitionProgress;
                float easedT = t * t * (3.0f - 2.0f * t);
                position = startPosition + (targetPosition - startPosition) * easedT;

                float yawDiff = targetYaw - startYaw;
                while (yawDiff > 180.0f)
                    yawDiff -= 360.0f;
                while (yawDiff < -180.0f)
                    yawDiff += 360.0f;
                Yaw = startYaw + yawDiff * easedT;

                Pitch = startPitch + (targetPitch - startPitch) * easedT;

                updateCameraVectors();
            }
        }


        void updateCameraVectors()
        {
            vec3 fr;
            float yawRad = Yaw * ONE_DEG_IN_RAD;
            float pitchRad = Pitch * ONE_DEG_IN_RAD;

            fr.x = cos(yawRad) * cos(pitchRad);
            fr.y = sin(pitchRad);
            fr.z = sin(yawRad) * cos(pitchRad);

            front = normalize(fr);
            right = normalize(cross(front, wrldup));
            up = normalize(cross(right, front));
        }
};

#endif
