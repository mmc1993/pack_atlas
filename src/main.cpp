#include "include.h"
#include "gl.h"
#include "atlas_packer.h"

uint WINDOW_W = 1280;
uint WINDOW_H = 720;

std::vector<gl::Texture> CollectImages(const std::string & folder)
{
    std::vector<gl::Texture> textures;
    for (auto & item : std::filesystem::directory_iterator(folder))
    {
        textures.push_back(gl::CreateTexture(item.path().string()));
    }
    return std::move(textures);
}

std::vector<AtlasPacker::Atlas> PackageAtlas(const std::vector<gl::Texture> & textures)
{
    std::vector<AtlasPacker::Item> items;
    for (const auto & texture : textures)
    {
        items.emplace_back((void *)&texture, texture.mW, texture.mH);
    }
    AtlasPacker packer;
    return packer.Package(items, 0, 1, 32);
}

std::vector<gl::Texture> RenderToTextures(const std::vector<AtlasPacker::Atlas> & atlass)
{
    glClearColor(0, 0, 0, 0);

    auto renderTarget = gl::CreateRenderTarget();

    gl::BindRenderTarget(GL_FRAMEBUFFER, renderTarget);

    auto mesh = gl::CreateMesh({ }, { 0, 1, 2, 0, 2, 3 });

    auto program = gl::CreateProgram("res/program/v.shader", 
                                     "res/program/f.shader");

    std::vector<gl::Texture> textures;
    for (auto & atlas : atlass)
    {
        auto & limit = AtlasPacker::LimitConfig[atlas.mLevel];
        auto texture = gl::CreateTexture(limit.w, limit.h, GL_RGBA, nullptr);
        gl::BindAttachmentRT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.mID);

        auto proj = glm::ortho(limit.w * -0.5f, limit.w * 0.5f,
                               limit.h * -0.5f, limit.h * 0.5f);

        auto view = glm::lookAt(glm::vec3(0, 0,  0), 
                                glm::vec3(0, 0, -1), 
                                glm::vec3(0, 1,  0));

        glViewport(0, 0, limit.w, limit.h);

        gl::UseProgram(program);
        gl::BindUniformMatrix(program, "matrix_proj", proj);
        gl::BindUniformMatrix(program, "matrix_view", view);

        glClear(GL_COLOR_BUFFER_BIT);
        for (auto & pack : atlas.mPacks)
        {
            auto texture = (gl::Texture *)pack.v;
            auto coord   = glm::vec3(pack.x - limit.w * 0.5f, 
                                     pack.y - limit.h * 0.5f, 0);
            auto model   = glm::translate(glm::mat4(1), coord);
            gl::BindUniformMatrix(program, "matrix_model", model);
            gl::BindUniformTex2D(program,  "texture_main", texture->mID, 0);

            std::vector<gl::Mesh::Vertex> points;
            points.push_back(gl::Mesh::Vertex{ { 0,           0 },  { 0.0f, 0.0f } });
            points.push_back(gl::Mesh::Vertex{ { texture->mW, 0 },  { 1.0f, 0.0f } });
            points.push_back(gl::Mesh::Vertex{ { texture->mW, texture->mH }, { 1.0f, 1.0f } });
            points.push_back(gl::Mesh::Vertex{ { 0,           texture->mH }, { 0.0f, 1.0f } });

            glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(gl::Mesh::Vertex) * points.size(), points.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            mesh.mVCount = points.size();
            glBindVertexArray(mesh.mVAO);
            glDrawElements(GL_TRIANGLES, mesh.mECount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
        textures.push_back(texture);
    }
    gl::BindRenderTarget(GL_FRAMEBUFFER, 0);
    gl::DeleteRenderTarget(renderTarget);
    gl::DeleteProgram(program);
    gl::DeleteVAO(mesh.mVAO);
    gl::DeleteBuf(mesh.mVBO);
    gl::DeleteBuf(mesh.mEBO);
    return std::move(textures);
}

void SaveToFile(const std::string & output, const std::vector<gl::Texture> & textures)
{
    for (auto i = 0; i != textures.size(); ++i)
    {
        auto & texture = textures.at(i);
        auto file = output + std::to_string(i) + ".png";
        glBindTexture(GL_TEXTURE_2D, texture.mID);
        std::unique_ptr<char []> data(new char[texture.mW * 4 * texture.mH]);
        glGetTexImage(GL_TEXTURE_2D, 0, texture.mFmt, GL_UNSIGNED_BYTE, data.get());
        stbi_write_png(file.c_str(), texture.mW, texture.mH, 4, data.get(), texture.mW * 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

int main(int argv, char **argc)
{
    auto input  = "./res/1/";
    auto output = "./output/";
    if (gl::Begin(WINDOW_W, WINDOW_H, "PackAtlas"))
    {
        auto window = glfwGetCurrentContext();
        std::cout << "Init: " << glfwGetTime() << std::endl;
        auto textures = CollectImages(input);
        std::cout << "Collect: " << glfwGetTime() << std::endl;
        auto atlass = PackageAtlas(textures);
        std::cout << "Package: " << glfwGetTime() << std::endl;
        auto pngs = RenderToTextures(atlass);
        std::cout << "Render: " << glfwGetTime() << std::endl;
        SaveToFile(output, pngs);
        std::cout << "Save: " << glfwGetTime() << std::endl;
        gl::Ended();
    }
    std::cin.get();
    return 0;
}