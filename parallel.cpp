#include "parallel.h"

Test::Test()
{

}

/*Test::playParallel(pxt::Action a)
{
#ifdef CODAL_I2C
    runInParallel(a);
#else
    runInParallel(a,b);
#endif

return 1;
}*/

namespace parall
{
	static Test* xTest =  new Test();
	
	//%
	int startParallel(Action a)
	{
		runInParallel(a);
		return 1;
	}
}
