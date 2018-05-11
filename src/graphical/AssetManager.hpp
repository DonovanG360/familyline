/*
    Asset management class

    Copyright (C) 2016, 2018 Arthur M
*/

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "AssetFile.hpp"

#if __has_include(<optional>)
/* Things to make it compile in Debian 8's GCC compiler */
#include <optional>
#define s_optional std::optional
#define make_optional std::make_optional

#else

#include <experimental/optional>
#define s_optional std::experimental::optional
#define make_optional std::experimental::make_optional

#endif

#include <unordered_map>
#include "AssetObject.hpp"


namespace Familyline::Graphics {

    
    enum AssetType {
	AMesh,
	AMaterial,
	ATexture
    };

    /* A pointer to the asset
       Binds an entry in the asset file to the object itself
    */
       
    struct AssetPointer {
	AssetItem* item;
	AssetType type;
	s_optional<AssetObject> object;
    };


    class AssetManager {

    private:
	AssetFile af;

	std::unordered_map<std::string, AssetPointer> assetlist; 
	AssetObject LoadAsset(AssetType type, const char* path);
    public:
	
	void Create();
	
	AssetObject GetAsset(const char* name);
	
	static AssetManager* GetInstance() {
	    static AssetManager* am = nullptr;
	    if (!am)    am = new AssetManager;
	    return am;
	}


    };


}




#endif /* end of include guard: ASSETMANAGER_H */
