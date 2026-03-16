#pragma once
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <taglib/fileref.h>
#include <taglib/tfile.h>
#include "playable.hpp"
#include "ext/miniaudio.h"

// https://stackoverflow.com/a/19195373 "how to combine hashes"
template <class T>
static inline void hash_combine(std::size_t & s, const T & v)
{
  std::hash<T> h;
  s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

class AudioMetaData {
#define WSTRING_FIELDS\
	X(album)\
	X(title)\
	X(artist)\
	X(genre)\
	X(comment)

#define UNSIGNED_FIELDS\
	X(year)\
	X(track)

#define X(name) std::wstring name;
	WSTRING_FIELDS
#undef X

#define X(name) unsigned int name;
	UNSIGNED_FIELDS
#undef X
public:
	AudioMetaData(const TagLib::FileRef &ref)
	{
#define X(name) name = ref.tag()->name().toWString();
		WSTRING_FIELDS
#undef X
#define X(name) name = ref.tag()->name();
		UNSIGNED_FIELDS
#undef X
	}

	bool operator==(const AudioMetaData &other) const noexcept
	{
#define X(name) if(name != other.name) { return false; }
		UNSIGNED_FIELDS
		WSTRING_FIELDS
#undef X
		return true;
	}

#define X(name) std::wstring get_##name() const noexcept { return name; }
	WSTRING_FIELDS
#undef X

#define X(name) unsigned int get_##name() const noexcept { return name; }
	UNSIGNED_FIELDS
#undef X

};

template<>
struct std::hash<AudioMetaData>
{
	std::size_t operator()(const AudioMetaData& amd) const noexcept
	{
		std::size_t res = 0;
#define X(name) hash_combine(res, amd.get_##name());
		WSTRING_FIELDS
		UNSIGNED_FIELDS
#undef X
		return res;
	}
};

#undef WSTRING_FIELDS
#undef UNSIGNED_FIELDS

class Ma_Engine {
	ma_engine engine;
public:
	Ma_Engine()
	{
		ma_result res = ma_engine_init(NULL, &engine);
		if(res != MA_SUCCESS){
			throw std::runtime_error("Could not initialize miniaudio engine");
		}
	}

	ma_engine *get() { return &engine; };

	~Ma_Engine(){
		ma_engine_uninit(&engine);
	}
};

class Ma_Sound {
	ma_sound sound;
public:
	Ma_Sound(const Ma_Sound &other) = delete;
	Ma_Sound operator=(const Ma_Sound &other) = delete;

	Ma_Sound(const std::string &soundFile, Ma_Engine &engine)
	{
		ma_result result;
		result = ma_sound_init_from_file(engine.get(), soundFile.c_str(), 0, NULL, NULL, &sound);
		if(result != MA_SUCCESS){
			throw std::runtime_error("Could not initialize miniaudio sound");
		}
	}

	~Ma_Sound()
	{
		ma_sound_uninit(&sound);
	}

	ma_sound *get(){ return &sound; };
};

class AudioPlayable : public Playable<std::chrono::milliseconds, AudioMetaData> {
	static inline Ma_Engine engine;
	Ma_Sound sound;
	std::string path;
	TagLib::FileRef fileref;
public:
	static bool acceptable(const std::filesystem::directory_entry &dir)
	{
		std::string ext = dir.path().extension();
		if(ext == ".mp3" || ext == ".flac" || ext == ".wav")
		{
			return true;
		}
		return false;
	}

	AudioPlayable(const std::string &path)
		: fileref(path.c_str()), 
		sound(path, engine),
		Playable<std::chrono::milliseconds, AudioMetaData>(std::chrono::milliseconds(0), path, TagLib::FileRef(path.c_str()))
	{
		this->path = path;
	}

	void play() override
	{
		ma_sound_start(sound.get());
	}

	void pause() override
	{
		ma_sound_stop(sound.get());
	}

	void seek(const std::chrono::milliseconds &ms) override
	{
		ma_uint32 sampleRate;
		ma_result result = ma_sound_get_data_format(sound.get(), NULL, NULL, &sampleRate, NULL, 0);
		if(result != MA_SUCCESS){
			throw std::runtime_error("Could not get sample rate");
		}
		ma_sound_seek_to_pcm_frame(sound.get(), (ma_uint64)(sampleRate * std::chrono::duration<double>(ms).count()));
	}

	std::chrono::milliseconds getLocation() override
	{
		return std::chrono::milliseconds(ma_sound_get_time_in_milliseconds(sound.get()));
	}

	AudioMetaData getIdentifier() override
	{
		return AudioMetaData(fileref);
	}
};
