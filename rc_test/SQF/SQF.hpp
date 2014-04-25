#include <vector>
#include <string>
#include <sstream>

#ifndef SQF_H
#define SQF_H

class SQF {
private:
	std::vector<std::string> arrayStack;

public:
	SQF();
	~SQF();
	void push(const char *String);
	void push(int Number);
	void push_array(const char *String);
	std::string toArray();
};

#endif
