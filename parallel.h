#pragma once
#ifdef CODAL_I2C
#include "I2C.h"
#include "Pin.h"
#endif
#include "inttypes.h"
#include "pxt.h"


class Test
{
	public:
		Test();
		int playParallel(pxt::Action);
};
