
#include "Server.h"

int main()
{
	Log::initializeLogFiles();

    Server s;
	int iResult = s.init();
	if (iResult != SUCCESS)
	{
		std::cerr << "Server initialization failed: " << getError() << std::endl;
		return iResult;
	}
	s.run();
	s.stop();
	return 0;
}

