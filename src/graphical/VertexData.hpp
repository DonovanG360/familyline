/***
    Raw vertex data

    Copyright (C) 2018 Arthur Mendes.

***/

#ifndef VERTEXDATA_HPP
#define VERTEXDATA_HPP

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <functional>

#include "ShaderProgram.hpp"

namespace familyline::graphics {

    /**
     * Contains base vertex data
     */
    struct VertexData {
	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	/// The indices
	///
	/// Each one of them corresponds to an unique set of vertices, normals and
	/// texcoords
	/// i.e, the index 3 means that the vertex is made of the position at index 2,
	/// normal at index 2 and texcoord at index 2 (the index start at 1)
	std::vector<unsigned int> indices;
    };

    /**
     * Contains metainformation about the vertex set
     */
    struct VertexInfo {

	/**
	 * When you retrieve the vertex list from an animation, it
	 * will come as a list of VertexDatas
	 * \see VertexDataGroup
	 *
	 * Each VertexData corresponds to a group of vertices that
	 * use a unique combination of material and shader, and are
	 * grouped together
	 *
	 * The index is a reference to that list
	 */
	int index = 0;

	int materialID = 0;   ///< Material ID for this vertex set
	int shaderID = 0;     ///< Shader ID for this vertex set
    };
    
}

#endif /* end of include guard: VERTEXDATA_HPP */
