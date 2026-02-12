#pragma once
#include <filesystem>

template <typename CursorType>
class Playable{
	std::filesystem::path location;
	
public:
	virtual void play();
	virtual void seek(const CursorType& location);
	virtual void pause();
	virtual CursorType getLocation();
	virtual std::string_view &getName();
};

class AudioPlayable : Playable<int> {

};
