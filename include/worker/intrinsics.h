#pragma once

#include "Runtime/Intrinsics.h"

namespace intrinsics {
    Intrinsics::Module& getIntrinsicModule_faasm()                                                \
	{                                                                                              \
		static Intrinsics::Module module;                                                          \
		return module;                                                                             \
	}

}
