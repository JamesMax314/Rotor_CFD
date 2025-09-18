#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "vk_types.h"

namespace MeshGen {
    void generateMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, const std::string& filename, float gridSize, float maxHeight);
}
