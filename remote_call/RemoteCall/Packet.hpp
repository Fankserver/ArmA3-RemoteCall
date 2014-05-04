#include <string>

namespace RemoteCall {
	class Packet {
	private:
		char identfier[2];
		unsigned char version;
		unsigned char spacer;
		unsigned char command;
		char *content;
		int contentLength;

	private:
		void _allocContent(size_t Size);

	public:
		Packet();
		~Packet();

		bool hasCommand();
		unsigned char getCommand();
		void setCommand(unsigned char Command);

		bool hasContent();
		const char* getContent();
		void setContent(const char Content);
		void setContent(const char *Content);
		void setContent(short int Content);

		void serialize(char *&Content, int *ContentLength);
	};
}