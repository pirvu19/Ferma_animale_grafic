#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 cameraFront = glm::normalize(cameraTarget - cameraPosition);

        if (direction == MOVE_FORWARD) {
            cameraPosition += cameraFront * speed;
            cameraTarget += cameraFront * speed;
        }

        if (direction == MOVE_BACKWARD) {
            cameraPosition -= cameraFront * speed;
            cameraTarget -= cameraFront * speed;
        }

        if (direction == MOVE_LEFT) {
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUpDirection));
            cameraPosition -= cameraRight * speed;
            cameraTarget -= cameraRight * speed;
        }

        if (direction == MOVE_RIGHT) {
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUpDirection));
            cameraPosition += cameraRight * speed;
            cameraTarget += cameraRight * speed;
        }

        if (direction == MOVE_UP) {
            cameraPosition += cameraUpDirection * speed;
            cameraTarget += cameraUpDirection * speed;
        }

        if (direction == MOVE_DOWN) {
            cameraPosition -= cameraUpDirection * speed;
            cameraTarget -= cameraUpDirection * speed;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitchOffset, float yawOffset) {
        // Actualizează yaw și pitch
        static float pitch = 0.0f;
        static float yaw = -90.0f; // Direcția inițială a camerei este spre -Z

        yaw += yawOffset;
        pitch += pitchOffset;

        // Constrânge pitch între -89° și 89°
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // Calculează noua direcție
        glm::vec3 direction;
        direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        direction = glm::normalize(direction);

        // Actualizează cameraPosition pe baza noii direcții
        float radius = glm::length(cameraPosition - cameraTarget);
        cameraPosition = cameraTarget - direction * radius;

        // Recalculează cameraUpDirection
        cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    }


}
