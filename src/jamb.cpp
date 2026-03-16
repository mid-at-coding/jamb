#include "audioplayable.hpp"
#include "library.hpp"

int main()
{
	Library<AudioPlayable, AudioMetaData, AudioPlayable::acceptable> mylib("/home/emelia/Music/Human People - Butterflies Drink Turtle Tears/");
	printf("Press Enter to quit...");
	getchar();
}
