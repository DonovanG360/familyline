#include "AssetObject.hpp"

using namespace Familyline::Graphics;

AssetObject Familyline::Graphics::create_asset_from_mesh(Mesh* m) {
    AssetObject o;
    o.mesh = m;
    return o;
}


AssetObject Familyline::Graphics::create_asset_from_material(std::vector<Material*>* m) {
    AssetObject o;
    o.materialvec = m;
    return o;
}

AssetObject Familyline::Graphics::create_asset_from_texture(Texture* t) {
    AssetObject o;
    o.texture = t;
    return o;
}

AssetObject Familyline::Graphics::create_asset_from_null() {
    AssetObject o;
    o.texture = nullptr;
    return o;
}


