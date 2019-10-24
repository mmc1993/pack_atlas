#pragma once

#include "include.h"

namespace gl {
    struct Mesh {
        struct Vertex {
            glm::vec2 v;
            glm::vec2 uv;
        };
        uint mVAO;
        uint mVBO;
        uint mEBO;
        uint mVCount;
        uint mECount;
    };

    struct Texture {
        uint mW;
        uint mH;
        uint mID;
        uint mFmt;
        Texture() : mID(0) {}
    };

    bool Begin(uint w, uint h, const std::string & title);

    uint CreateRenderTarget();

    void UseProgram(uint id);

    uint CreateProgram(const std::string & vfile,
                       const std::string & ffile);
    uint CreateProgram(const char * vs, const size_t vlen,
                       const char * fs, const size_t flen);

    void BindUniformVector(uint id, const char * const key, const glm::vec2 & val);
    void BindUniformVector(uint id, const char * const key, const glm::vec3 & val);
    void BindUniformVector(uint id, const char * const key, const glm::vec4 & val);
    void BindUniformMatrix(uint id, const char * const key, const glm::mat3 & val);
    void BindUniformMatrix(uint id, const char * const key, const glm::mat4 & val);
    void BindUniformTex2D(uint id, const char * const key, const uint val, iint pos);

    Texture CreateTexture(const std::string & url);
    Texture CreateTexture(uint w, uint h, uint fmt,
                          const char * buffer = nullptr);

    Mesh CreateMesh(const std::vector<Mesh::Vertex> & points,
                    const std::vector<uint>         & indexs);

    void BindRenderTarget(uint type, uint target);

    void BindAttachmentRT(uint type, uint attachment, uint textureType, uint texture);

    void DeleteRenderTarget(uint & id);
    void DeleteTexture(uint & id);
    void DeleteProgram(uint & id);
    void DeleteVAO(uint & id);
    void DeleteBuf(uint & id);
    void Ended();

    const char * const GL_PROGRAM_SPRITE_V = "res/program/sprite.vs";
    const char * const GL_PROGRAM_SPRITE_F = "res/program/sprite.fs";
}