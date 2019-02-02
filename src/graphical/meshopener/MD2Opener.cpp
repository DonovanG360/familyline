#include "MD2Opener.hpp"
#include "../DeformAnimator.hpp"

#include <vector>
#include <array>

using namespace familyline::graphics;

/*  MD2 vertex
    Each vertex is stored in a one-byte form, together with a multiplication
    factor in the frame information */
struct md2_vertex {
    unsigned char v[3];
    unsigned char normal;
};

/* MD2 triangle */
struct md2_triangle
{
  unsigned short vertex[3];   /* vertex indices of the triangle */
  unsigned short st[3];       /* tex. coord. indices */
};

/* Texture coords */
struct md2_texcoords
{
  short s;
  short t;
};

struct md2_frame {
    float scaleX, scaleY, scaleZ;
    float transX, transY, transZ;
    char name[16];
}; //__attribute__((packed));

#include "anorms.h"

constexpr int MD2Magic = 0x32504449; // IDP2

// Default framerate (in frames per second) for md2 animations
// (since they do not specify this in the file)
constexpr int framerate = 30;

VertexData create_vertex_data(FILE* file, int offset_tris,
			      unsigned int num_tris,
			      std::vector<glm::vec3>& vertices,
			      std::vector<glm::vec3>& normals,
			      std::vector<glm::vec2>& texcoords)
{
    fseek(file, offset_tris, SEEK_SET);

    std::vector<struct md2_triangle> tris(num_tris);
    fread((void*)tris.data(), sizeof(md2_triangle), num_tris, file);

    VertexData vd;

    // TODO: change this to use indexed rendering
    for (const auto& t : tris) {
	for (auto i = 0; i < 3; i++) {
	    vd.position.push_back(vertices[t.vertex[i]]);
	    vd.normals.push_back(normals[t.vertex[i]]);
	    vd.texcoords.push_back(texcoords[t.st[i]]);
	}
    }

    return vd;
}

std::vector<glm::vec3> decode_frame(FILE* file, int offset_frames,
				    unsigned num_vertices,
				    unsigned frame_size, unsigned frame_num)
{


    fseek(file, offset_frames + (frame_size * frame_num), SEEK_SET);

    md2_frame frame;
    fread((struct md2_frame*)&frame, sizeof(frame), 1, file);

    std::vector<md2_vertex> vs(num_vertices);
    fread((void*)vs.data(), sizeof(struct md2_vertex),
	  num_vertices, file);

    std::vector<glm::vec3> vertices;
    vertices.reserve(num_vertices);

    for (const auto& v : vs) {
	vertices.push_back(
	    glm::vec3(
		(v.v[0] * frame.scaleX) + frame.transX,
		(v.v[1] * frame.scaleY) + frame.transY,
		(v.v[2] * frame.scaleZ) + frame.transZ));
    }

    return vertices;
}

std::vector<glm::vec3> decode_normals(FILE* file, int offset_frames,
				      unsigned num_vertices) {


    fseek(file, offset_frames, SEEK_SET);

    md2_frame frame;
    fread((struct md2_frame*)&frame, sizeof(frame), 1, file);

    std::vector<md2_vertex> vs(num_vertices);
    fread((void*)vs.data(), sizeof(struct md2_vertex),
	  num_vertices, file);

    std::vector<glm::vec3> normals;
    normals.reserve(num_vertices);

    for (const auto& v : vs) {
	normals.push_back(
	    glm::vec3(anorms[int(v.normal)][0],
		      anorms[int(v.normal)][1],
		      anorms[int(v.normal)][2]));
    }

    return normals;
}


std::vector<glm::vec2> decode_texcoords(FILE* file, int offset_st,
					unsigned num_st,
					unsigned skinwidth,
					unsigned skinheight) {


    fseek(file, offset_st, SEEK_SET);
    std::vector<glm::vec2> texcoords;

    std::vector<md2_texcoords> texes(num_st);
    fread((void*)texes.data(), sizeof(struct md2_texcoords),
	  num_st, file);

    for (const auto& tex : texes) {
	texcoords.push_back(
	    glm::vec2(
		float(tex.s) / skinwidth,
		float(tex.t) / skinheight));
    }

    return texcoords;
}




std::vector<Mesh*> MD2Opener::OpenSpecialized(const char* file)
{
    FILE* fMD2 = fopen(file, "rb");
    if (!fMD2) {
	throw mesh_exception("Failure to open mesh", errno, file);
    }

    rewind(fMD2);
    
    md2_header_t header = {};
    fread((md2_header_t*)&header, sizeof(header), 1, fMD2);

    if (header.ident != MD2Magic) {
	throw mesh_exception("Invalid MD2 header", -1, file);
    }

    /// TODO: check header numerical limits

    auto normals = decode_normals(fMD2, header.offset_frames,
				  unsigned(header.num_vertices));
    auto texcoords = decode_texcoords(fMD2, header.offset_st,
				      header.num_st, header.skinwidth,
				      header.skinheight);

    std::vector<VertexDataGroup> frames;
    frames.reserve(header.num_frames);

    for (auto i = 0; i < header.num_frames; i++) {
	auto vertices = decode_frame(fMD2, header.offset_frames, header.num_vertices,
				     header.framesize, i);

	VertexDataGroup vdg;
	vdg.push_back(create_vertex_data(fMD2, header.offset_tris,
					 header.num_tris,
					 vertices, normals, texcoords));

	frames.push_back(vdg);

    }


    std::map<std::string, std::vector<VertexDataGroup>> framedata = {
	{"default", frames}
    };

    DeformAnimator* da = new DeformAnimator{framedata, framerate};
    da->runAnimation("default"); //Set the animator internat pointer
    
    std::vector<VertexInfo> vinfo = {
	VertexInfo{0, 0, "forward", new glm::mat4(1.0)}
    };

    return {new Mesh{da, vinfo}};
}
