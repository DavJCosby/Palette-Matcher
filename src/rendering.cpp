#include "internal/rendering.h"

#include "cy/cyTriMesh.h"
#include "glad/glad.h"
#include "lodepng.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

void build_programs(ShaderPrograms& programs) {
    cyGLSLProgram& mesh_prog = programs.mesh;
    cyGLSLProgram& shadow_prog = programs.shadow;
    mesh_prog.BuildFiles("./shaders/mesh.vert", "./shaders/mesh.frag");

    mesh_prog.Bind();
    mesh_prog.RegisterUniform(0, "MVP");
    mesh_prog.RegisterUniform(1, "MVP");
    mesh_prog.RegisterUniform(2, "BaseColor");
    mesh_prog.RegisterUniform(3, "SpecularColor");
    mesh_prog.RegisterUniform(4, "Shine");
    mesh_prog.RegisterUniform(5, "Ambient");
    mesh_prog.RegisterUniform(6, "ShadowMap");
    mesh_prog.RegisterUniform(7, "LightSpaceMatrix");
    mesh_prog.RegisterUniform(8, "LightPosition");
    mesh_prog.RegisterUniform(9, "LightConeAngle");

    programs.mesh.SetUniform("ShadowMap", 4); // shadow map is texture unit 4

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    shadow_prog.BuildFiles("./shaders/shadow.vert", "./shaders/shadow.frag");
    shadow_prog.Bind();
    shadow_prog.RegisterUniform(0, "MVP");
}

struct MeshData load_mesh(cyGLSLProgram& prog, char* path) {
    prog.Bind();
    cyTriMesh mesh;
    bool success = mesh.LoadFromFileObj(path);
    if (!success) {
        std::cout << "Failed to load obj file: '" << path << "'." << std::endl;
        exit(-1);
    }

    struct MeshData md = bind_mesh_vertex_attributes(prog, mesh);
    if (mesh.NM() > 0) { // hecka jank
        string diffuse_path = mesh.M(0).map_Kd.data;
        string specular_path = mesh.M(0).map_Ks.data;
        diffuse_path = "./assets/teapot/" + diffuse_path;
        specular_path = "./assets/teapot/" + specular_path;
        load_texture(prog, diffuse_path, (char*)"DiffuseTexture", 0);
        load_texture(prog, specular_path, (char*)"SpecularTexture", 1);
    }

    return md;
}

void bind_material_properties(cyGLSLProgram& prog, cyTriMesh& mesh) {
    if (mesh.NM() > 0) {
        //std::cout << "at least one material was detected." << std::endl;
        cyTriMesh::Mtl objMaterial = mesh.M(0);

        prog.SetUniform3("BaseColor", objMaterial.Kd);
        prog.SetUniform3("SpecularColor", objMaterial.Ks);
        prog.SetUniform("Shine", objMaterial.Ns);
        prog.SetUniform3("Ambient", objMaterial.Ka);
    } else {
        //std::cout << "Using default material settings." << std::endl;
        // loadTexture(prog, "./assets/blank.png", (char*)"diffuse_tex", 0);
        // loadTexture(prog, "./assets/blank.png", (char*)"specular_tex", 1);
        float kd_default[3] = {0.25, 0.25, 0.25};
        float ks_default[3] = {0.65, 0.65, 0.65};
        float ns_default = 50.0;
        float ka_default[3] = {0.21, 0.21, 0.21};

        prog.SetUniform3("BaseColor", kd_default);
        prog.SetUniform3("SpecularColor", ks_default);
        prog.SetUniform("Shine", ns_default);
        prog.SetUniform3("Ambient", ka_default);
    }
}

void cleanup_mesh_data(MeshData& meshData) {
    glDeleteBuffers(1, &meshData.VBO);
    glDeleteBuffers(1, &meshData.EBO);
    glDeleteVertexArrays(1, &meshData.VAO);
}

void read_texture_and_handle_error(
    vector<unsigned char>& img_data_out,
    unsigned& width_out,
    unsigned& height_out,
    const char* img_path
) {
    unsigned error =
        lodepng::decode(img_data_out, width_out, height_out, img_path);
    if (error) {
        std::cout << "Error loading texture: " << lodepng_error_text(error)
                  << std::endl;
        exit(-1);
    }
}

void load_texture(
    cyGLSLProgram& prog,
    string filepath,
    char* attrib_name,
    unsigned tex_id
) {
    prog.Bind();
    unsigned width, height;
    vector<unsigned char> diffuse_data;

    read_texture_and_handle_error(
        diffuse_data,
        width,
        height,
        (char*)filepath.c_str()
    );

    cyGLTexture2D texture;

    glActiveTexture(GL_TEXTURE0 + tex_id);

    texture.Initialize();
    texture.SetImage(diffuse_data.data(), 4, width, height);
    texture.BuildMipmaps();
    texture.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    texture.SetWrappingMode(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T);
    texture.Bind(tex_id);

    prog[attrib_name] = tex_id;
}

struct MeshData
bind_mesh_vertex_attributes(cyGLSLProgram& prog, cyTriMesh& mesh) {
    prog.Bind();
    mesh.ComputeNormals();

    // do some pre-processing
    vector<float> vertexData;
    vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh.NF(); i++) {
        for (int j = 0; j < 3; j++) {
            unsigned int vIndex = mesh.F(i).v[j];
            unsigned int tIndex = mesh.FT(i).v[j];
            unsigned int nIndex = mesh.FN(i).v[j];

            vertexData.push_back(mesh.V(vIndex).x);
            vertexData.push_back(mesh.V(vIndex).y);
            vertexData.push_back(mesh.V(vIndex).z);

            vertexData.push_back(mesh.VN(nIndex).x);
            vertexData.push_back(mesh.VN(nIndex).y);
            vertexData.push_back(mesh.VN(nIndex).z);

            vertexData.push_back(mesh.VT(tIndex).x);
            vertexData.push_back(mesh.VT(tIndex).y);

            indices.push_back(i * 3 + j);
        }
    }

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(float),
        vertexData.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW
    );

    // Position attribute
    GLuint pos_attrib = prog.AttribLocation("VertexPosition");
    glVertexAttribPointer(
        pos_attrib,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(pos_attrib);

    // Normal attribute
    GLuint norm_attrib = prog.AttribLocation("VertexNormal");
    glVertexAttribPointer(
        norm_attrib,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(norm_attrib);

    // Texture coordinate attribute
    GLuint txc_attrib = prog.AttribLocation("VertexTexCoord");
    glVertexAttribPointer(
        txc_attrib,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(6 * sizeof(float))
    );
    glEnableVertexAttribArray(txc_attrib);

    glBindVertexArray(0);

    return {VAO, VBO, EBO, mesh.NF(), mesh};
}

void draw_mesh(cyGLSLProgram& prog, struct MeshData& meshData) {
    glBindVertexArray(meshData.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
    glDrawElements(GL_TRIANGLES, meshData.numFaces * 3, GL_UNSIGNED_INT, 0);
}
