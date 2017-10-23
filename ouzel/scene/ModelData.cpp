// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "ModelData.hpp"
#include "core/Engine.hpp"
#include "utils/Utils.hpp"

namespace ouzel
{
    namespace scene
    {
        bool ModelData::init(const std::string& filename, bool mipmaps)
        {
            std::vector<uint8_t> data;

            if (!sharedEngine->getFileSystem()->readFile(filename, data))
            {
                return false;
            }

            return true;
        }

        bool ModelData::init(const std::vector<uint16_t> indices,
                             const std::vector<graphics::VertexPCT>& vertices,
                             const std::shared_ptr<graphics::Material>& newMaterial)
        {
            indexBuffer = std::make_shared<graphics::Buffer>();
            indexBuffer->init(graphics::Buffer::Usage::INDEX, indices.data(), static_cast<uint32_t>(getVectorSize(indices)));

            vertexBuffer = std::make_shared<graphics::Buffer>();
            vertexBuffer->init(graphics::Buffer::Usage::VERTEX, vertices.data(), static_cast<uint32_t>(getVectorSize(vertices)));

            meshBuffer = std::make_shared<graphics::MeshBuffer>();
            meshBuffer->init(sizeof(uint16_t), indexBuffer, graphics::VertexPCT::ATTRIBUTES, vertexBuffer);

            material = newMaterial;

            return true;
        }
    } // namespace scene
} // namespace ouzel
