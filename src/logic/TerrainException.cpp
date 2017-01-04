#include "TerrainFile.hpp"

using namespace Tribalia::Logic;

terrain_file_exception::terrain_file_exception(std::string msg, 
	std::string file, int code)
    : std::runtime_error(msg)
    {
		this->file = file;
        this->code = code;

    }


