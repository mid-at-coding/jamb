#pragma once
#include "audioplayable.hpp"
#include <taglib/fileref.h>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <filesystem>
#include <optional>

template <typename Playable, typename Identifier, bool (*acceptable)(const std::filesystem::directory_entry &),
	  typename AssociativeContainer = std::unordered_map<Identifier, Playable>,
	  typename NameList = std::vector<Identifier>>
class Library{
private:
	AssociativeContainer metadata;
	NameList playables;
public:
	size_t add_dir(const std::filesystem::path &dir) // returns the number of playable objects that have been added,
		                 			 // including those that have already been added
	{
		size_t res = 0;
		for(auto const & dir_entry : std::filesystem::directory_iterator(dir)){
			if(!acceptable(dir_entry)){
				continue;
			}
			AudioMetaData meta (TagLib::FileRef(dir_entry.path().c_str()));
			metadata.emplace(
				meta ,
				dir_entry.path());
			playables.push_back(meta);
			res++;
		}
		return res;
	}

	Library(const std::filesystem::path &location)
	{
		add_dir(location);
	}

	Library(const std::vector<std::filesystem::path> &locations)
	{
		for(const auto &location : locations){
			add_dir(locations);
		}
	}

	
	NameList &search(const std::string_view &name, const bool fuzzy);
	NameList &search(const std::filesystem::path &dir);

	AssociativeContainer  get_metadata     (const NameList &names);
	std::vector<Playable> search_metadata_v(const NameList &names);

	NameList             &getNameList(){ return playables; }
	AssociativeContainer &getMetaData(){ return metadata;  }

	std::optional<Identifier> getSingle(const std::string_view &name); // if this does not return a value, either there are multiple
									    // items that name could mean, or there are zero
};
