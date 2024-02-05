#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        this->pitch = 0;
        this->yaw = 0;
        
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        //return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

        //TODO
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            cameraTarget += cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            cameraTarget -= cameraFrontDirection * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            cameraTarget += cameraRightDirection * speed;
            break;
        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            cameraTarget -= cameraRightDirection * speed;
            break;
        case MOVE_UP:
            cameraPosition += cameraUpDirection * speed;
            cameraTarget += cameraUpDirection * speed;
            break;
        case MOVE_DOWN:
            cameraPosition -= cameraUpDirection * speed;
            cameraTarget -= cameraUpDirection * speed;
            break;
        default:
            break;
        }
    }
    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        /*glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(direction);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));*/


        ///ce urmeaza e testing:
        /*float rotationAngle = yaw;
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, cameraUpDirection);

        cameraFrontDirection = glm::mat3(rotationMatrix) * cameraFrontDirection;
        cameraRightDirection = glm::mat3(rotationMatrix) * cameraRightDirection;*/


        ///asta ar trebui sa fie bun
        this->pitch += pitch;
        this->yaw += yaw;

        if (this->pitch > 89.9f)
            this->pitch = 89.9f;
        
        if (this->pitch < -89.9f)
            this->pitch = -89.9f;

        glm::vec3 aux;
        aux.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        aux.y = sin(glm::radians(this->pitch));
        aux.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        
        cameraFrontDirection = glm::normalize(aux);
        cameraRightDirection = glm::normalize(
            glm::cross(this->cameraFrontDirection, this->cameraUpDirection));

    }
}
