#include "ObjectPathManager.hpp"
#include "DebugPlot.hpp"

int maxpathID = 0;
using namespace Tribalia::Logic;

static std::vector<glm::vec3> ConvertTo3DPath(std::vector<glm::vec2>* path)
{
    std::vector<glm::vec3> v3;
    for (const auto v : *path) {
	v3.push_back(glm::vec3(v.x, 512, v.y));
    }
    return v3;
}


/* 	Adds a path from object 'o' to the path manager 
	Returns true if added successfully, or false if there's already a path 
	there for the same object
 */
bool ObjectPathManager::AddPath(LocatableObject* o, 
				std::vector<glm::vec2>* path)
{
	for (auto& ref : _pathrefs) {
		if (ref.lc->GetObjectID() == o->GetObjectID()) {
		    /* If we had some object following a path and set a new
		       path for that object, interrupt the path for this one 
		       and make it follow the new path

		       This is the default behavior of all RTS games
		    */
		    ref.interrupted = true;
		}
	}

	std::remove_if(_pathrefs.begin(), _pathrefs.end(), [](ObjectPathRef& r)
		       { return r.interrupted; });


	auto p3dpath = ConvertTo3DPath(path);
	auto dbg_plot = DebugPlotter::pinterface->AddPath(p3dpath, glm::vec3(0, 0, 1));
	_pathrefs.emplace_back(maxpathID++, o, new std::vector<glm::vec2>(*path),
			       dbg_plot);	
	return true;
}

/* 	Removes a path from object 'oid'. 
 	Returns true if path removed, or false if path didn't exist there */
bool ObjectPathManager::RemovePath(long oid)
{
	for (auto it = _pathrefs.begin(); it != _pathrefs.end(); it++) {
		if (it->lc->GetObjectID() == oid) {
		    DebugPlotter::pinterface->RemovePath(it->dbg_path_plot);
		    _pathrefs.erase(it);
		    return true;
		}
	}

	return false;
}

/* Update the paths
 * Also, removes the completed paths
 * */
void ObjectPathManager::UpdatePaths()
{
	/* Store a vector of 'completed path' iterators.
	 * Nuke them in iteration end */
	std::vector<std::vector<ObjectPathRef>::iterator> compl_its;

	for (auto it = _pathrefs.begin(); it != _pathrefs.end(); it++) {
		if (it->path_point == it->path->back()) {
			/* Path completed. Remove the iterator */
			compl_its.push_back(it);
		}

		auto px = it->path_point.x;
		auto pz = it->path_point.y;

		it->lc->SetX(px);
		it->lc->SetY(_terr->GetHeightFromPoint(px, pz));
		it->lc->SetZ(pz);

		if (it->path_ptr < it->path->size()-1)
			it->path_point = (*it->path)[++(it->path_ptr)];
	}

	/* Delete the reserved iterators */
	for (auto& it : compl_its) {
	    DebugPlotter::pinterface->RemovePath(it->dbg_path_plot);
	    _pathrefs.erase(it);
	}
}

void ObjectPathManager::SetTerrain(Terrain* t) { _terr = t; }

