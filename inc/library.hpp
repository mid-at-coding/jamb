#pragma once
#include <vector>
#include <unordered_map>
#include <string_view>
#include <filesystem>
#include <optional>

template <typename LibMetaData, 
	  typename AssociativeContainer = std::unordered_map<std::string_view, LibMetaData>,
	  typename NameList = std::vector<std::string_view>>
class Library{
private:
	AssociativeContainer metadata;
	NameList playables;
public:
	Library(const std::filesystem::path &location);

	size_t add_dir(const std::filesystem::path &dir); // returns the number of playable objects that have been added,
		                 			  // including those that have already been added
	
	NameList &search(const std::string_view &name, const bool fuzzy);
	NameList &search(const std::filesystem::path &dir);

	AssociativeContainer     search_metadata  (const NameList &names);
	std::vector<LibMetaData> search_metadata_v(const NameList &names);

	NameList             &getNameList(){ return playables; }
	AssociativeContainer &getMetaData(){ return metadata;  }

	std::optional<LibMetaData> getSingle(const std::string_view &name); // if this does not return a value, either there are multiple
									    // items that name could mean, or there are zero
};
