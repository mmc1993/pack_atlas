#include "gl.h"

bool gl::Begin(uint w, uint h, const std::string & title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    assert(window != nullptr);
    glfwMakeContextCurrent(window);
    return GLEW_OK == glewInit();
}

uint gl::CreateRenderTarget()
{
    uint id;
    glGenFramebuffers(1, &id);
    return id;
}

void gl::UseProgram(uint id)
{ 
    glUseProgram(id);
}

uint gl::CreateProgram(const std::string & vfile, const std::string & ffile)
{
    std::ifstream is0(vfile);
    std::ifstream is1(ffile);
    std::noskipws(is0);
    std::noskipws(is1);
    std::string fs;
    std::string vs;
    std::copy(std::istream_iterator<char>(is0), 
              std::istream_iterator<char>(), 
              std::back_inserter(vs));
    std::copy(std::istream_iterator<char>(is1), 
              std::istream_iterator<char>(), 
              std::back_inserter(fs));
    return gl::CreateProgram(vs.c_str(), (uint)vs.size(), fs.c_str(), (uint)fs.size());
}

uint gl::CreateProgram(const char * vs, const size_t vlen, const char * fs, const size_t flen)
{
    static const auto Assert = [](uint id) 
    {
        iint ret;
        glGetShaderiv(id, GL_COMPILE_STATUS, &ret);
        assert(ret != 0);
        //char err[256];
        //glGetShaderInfoLog(id, sizeof(err), nullptr, err);
    };

    uint id = glCreateProgram();
    uint vid = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vid, 1, &vs, (iint *)&vlen);
    glCompileShader(vid); Assert(vid);
    glAttachShader(id, vid);
    glDeleteShader(vid);

    uint fid = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fid, 1, &fs, (iint *)&flen);
    glCompileShader(fid); Assert(fid);
    glAttachShader(id, fid);
    glDeleteShader(fid);

    glLinkProgram(id);

    iint ret;
    glGetProgramiv(id, GL_LINK_STATUS, &ret);
    //int size;
    //GLchar error[256] = { 0 };
    //glGetProgramInfoLog(id, sizeof(error), &size, error);
    assert(ret != 0);
    return id;
}

void gl::BindUniformVector(uint id, const char * const key, const glm::vec2 & val)
{ 
    glUniform2f(glGetUniformLocation(id, key), val.x, val.y);
}

void gl::BindUniformVector(uint id, const char * const key, const glm::vec3 & val)
{ 
    glUniform3f(glGetUniformLocation(id, key), val.x, val.y, val.z);
}

void gl::BindUniformVector(uint id, const char * const key, const glm::vec4 & val)
{ 
    glUniform4f(glGetUniformLocation(id, key), val.x, val.y, val.z, val.w);
}

void gl::BindUniformMatrix(uint id, const char * const key, const glm::mat3 & val)
{ 
    glUniformMatrix3fv(glGetUniformLocation(id, key), 1, GL_FALSE, &val[0][0]);
}

void gl::BindUniformMatrix(uint id, const char * const key, const glm::mat4 & val)
{ 
    glUniformMatrix4fv(glGetUniformLocation(id, key), 1, GL_FALSE, &val[0][0]);
}

void gl::BindUniformTex2D(uint id, const char * const key, const uint val, iint pos)
{ 
    glActiveTexture(GL_TEXTURE0 + pos); glBindTexture(GL_TEXTURE_2D,  val); glUniform1i(glGetUniformLocation(id, key), pos);
}

gl::Texture gl::CreateTexture(const std::string & url)
{
    gl::Texture texture;
    int w, h, fmt;
    if (auto data = stbi_load(url.c_str(), &w, &h, &fmt, 0))
    {
        switch (fmt)
        {
        case 1: fmt = GL_RED; break;
        case 3: fmt = GL_RGB; break;
        case 4: fmt = GL_RGBA; break;
        }
        texture = gl::CreateTexture((uint)w, (uint)h, (uint)fmt, (const char *)data);
        stbi_image_free(data);
    }
    return texture;
}

gl::Texture gl::CreateTexture(uint w, uint h, uint fmt, const char * buffer)
{
    gl::Texture texture;
    texture.mW = w;
    texture.mH = h;
    texture.mFmt = fmt;
    glGenTextures(1, &texture.mID);
    glBindTexture(GL_TEXTURE_2D, texture.mID);
    glTexImage2D( GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

gl::Mesh gl::CreateMesh(const std::vector<Mesh::Vertex> & points, const std::vector<uint> & indexs)
{
    gl::Mesh mesh;
    mesh.mVCount = points.size();
    mesh.mECount = indexs.size();

    glGenVertexArrays(1, &mesh.mVAO);
    glBindVertexArray(mesh.mVAO);

    glGenBuffers(1, &mesh.mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gl::Mesh::Vertex) * mesh.mVCount, points.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh.mECount, indexs.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(gl::Mesh::Vertex), (void *)offsetof(gl::Mesh::Vertex, v));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(gl::Mesh::Vertex), (void *)offsetof(gl::Mesh::Vertex, uv));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return mesh;
}

void gl::BindRenderTarget(uint type, uint target)
{
    assert((target != 0) || glCheckFramebufferStatus(type) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(type, target);
}

void gl::BindAttachmentRT(uint type, uint attachment, uint textureType, uint texture)
{
    glFramebufferTexture2D(type, attachment, textureType, texture, 0);
}

void gl::DeleteRenderTarget(uint & id)
{
    glDeleteFramebuffers(1, &id); id = 0;
}

void gl::DeleteTexture(uint & id)
{
    glDeleteTextures(1, &id); id = 0;
}

void gl::DeleteProgram(uint & id)
{
    glDeleteProgram(id); id = 0;
}

void gl::DeleteVAO(uint & id)
{
    glDeleteVertexArrays(1, &id); id = 0;
}

void gl::DeleteBuf(uint & id)
{
    glDeleteBuffers(1, &id); id = 0;
}

void gl::Ended()
{
    glfwTerminate();
}