#pragma once
#include <filesystem>

template <typename CursorType, typename Identifier>
class Playable{
protected:
	std::filesystem::path location;
	CursorType cursor;
	Identifier identifier;
public:
	Playable(const CursorType &cursor, const std::filesystem::path &loc, const Identifier &id) 
		: location(loc), cursor(cursor), identifier(id) {};
	virtual void play() {}
	virtual void seek(const CursorType& location) {}
	virtual void pause() {}

	virtual CursorType getLocation() { return cursor; }
	virtual Identifier getIdentifier() { return identifier; }
};


