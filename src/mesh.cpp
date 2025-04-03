#include "glad/glad.h"
#include <cy/cyCore.h>
#include <cy/cyGL.h>

#include "internal/mesh.h"

Mesh::Mesh(MeshData mesh_data, bool casts_shadow) :
    mesh_data(mesh_data),
    casts_shadow(casts_shadow) {}

void Mesh::draw() {
    glBindVertexArray(mesh_data.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_data.EBO);
    glDrawElements(GL_TRIANGLES, mesh_data.numFaces * 3, GL_UNSIGNED_INT, 0);
}

void Mesh::bindMaterialProperties(cyGLSLProgram& program) {
    if (mesh_data.mesh.NM() > 0) {
        cyTriMesh::Mtl objMaterial = mesh_data.mesh.M(0);

        program.SetUniform3("BaseColor", objMaterial.Kd);
        program.SetUniform3("SpecularColor", objMaterial.Ks);
        program.SetUniform("Shine", objMaterial.Ns);
        program.SetUniform3("Ambient", objMaterial.Ka);
    } else {
        float kd_default[3] = {0.15, 0.15, 0.45};
        float ks_default[3] = {0.65, 0.65, 0.65};
        float ns_default = 90.0;
        float ka_default[3] = {0.21, 0.21, 0.21};

        program.SetUniform3("BaseColor", kd_default);
        program.SetUniform3("SpecularColor", ks_default);
        program.SetUniform("Shine", ns_default);
        program.SetUniform3("Ambient", ka_default);
    }
}

void Mesh::cleanup() {
    glDeleteBuffers(1, &this->mesh_data.VBO);
    glDeleteBuffers(1, &this->mesh_data.EBO);
    glDeleteVertexArrays(1, &this->mesh_data.VAO);
}
