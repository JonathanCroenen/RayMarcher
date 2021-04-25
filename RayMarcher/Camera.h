#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UPWARD,
    DOWNWARD
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float MOVESPEED = 3.0f;
const float SENSITIVITY = 0.02f;
const float ZOOM = 45.0f;

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float Zoom;
    float Sensitivity;
    float MovementSpeed;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(MOVESPEED), Zoom(ZOOM), Sensitivity(SENSITIVITY) {

        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) Position += velocity * Front;
        if (direction == BACKWARD) Position -= velocity * Front;
        if (direction == RIGHT) Position += velocity * Right;
        if (direction == LEFT) Position -= velocity * Right;
        if (direction == UPWARD) Position += velocity * Up;
        if (direction == DOWNWARD) Position -= velocity * Up;
    }

    void ProcessMouseMovement(double xoffset, double yoffset, GLboolean constrainPitch = true) {
        Yaw += xoffset * Sensitivity;
        Pitch += yoffset * Sensitivity;

        if (constrainPitch) {
            if (Pitch > 89.9f) Pitch = 89.9f;
            else if (Pitch < -89.9f) Pitch = -89.9f;
        }
        updateVectors();
    }

private:
    void updateVectors() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        direction.y = sin(glm::radians(Pitch));
        direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(direction);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif