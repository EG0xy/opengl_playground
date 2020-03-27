#ifndef CAMERA_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


struct Camera {
    glm::vec3 position  = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up        = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right;
    glm::vec3 world_up;
    
    f32 yaw   = -90.0f;
    f32 pitch = 0.0f;
    f32 fov   = 45.0f;
    
    f32 movement_speed    = 2.5f;
    f32 mouse_sensitivity = 0.1f;
};

void
update_camera(Camera *camera) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    direction.y = sin(glm::radians(camera->pitch));
    direction.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    camera->direction = glm::normalize(direction);
    
    camera->right = glm::normalize(glm::cross(camera->direction, camera->world_up));
    camera->up    = glm::normalize(glm::cross(camera->right, camera->direction));
}


glm::mat4
get_view_matrix(Camera *camera) {
    glm::mat4 view_matrix = glm::lookAt(camera->position, camera->position + camera->direction, camera->up);
    return view_matrix;
}

void
process_keyboard(Camera *camera, Camera_Movement direction, f32 dt) {
    f32 camera_speed = camera->movement_speed * dt;
    if (direction == FORWARD) {
        camera->position += camera_speed * camera->direction;
    }
    if (direction == BACKWARD) {
        camera->position -= camera_speed * camera->direction;
    }
    if (direction == LEFT) {
        camera->position -= glm::normalize(glm::cross(camera->direction, camera->up)) * camera_speed;
    }
    if (direction == RIGHT) {
        camera->position += glm::normalize(glm::cross(camera->direction, camera->up)) * camera_speed;
    }
}

void
process_mouse_movement(Camera *camera, f32 offset_x, f32 offset_y, b32 constrain_pitch = true) {
    offset_x *= camera->mouse_sensitivity;
    offset_y *= camera->mouse_sensitivity;
    
    camera->yaw += offset_x;
    camera->pitch += offset_y;
    
    if (camera->pitch >  89.0f)  camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)  camera->pitch = -89.0f;
    
    glm::vec3 direction;
    direction.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    direction.y = sin(glm::radians(camera->pitch));
    direction.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    camera->direction = glm::normalize(direction);
}

void
process_mouse_scroll(Camera *camera, f32 offset_y) {
    if (camera->fov >= 1.0f && camera->fov <= 45.0f)  camera->fov -= offset_y;
    else if (camera->fov <= 1.0f)  camera->fov = 1.0f;
    else if (camera->fov >= 45.0f)  camera->fov = 45.0f;
}


#define CAMERA_H
#endif//CAMERA_H
