#pragma once

#define UPWND_EXCEPT( hr ) misc::HrException( __LINE__,__FILE__,(hr) )
#define UPWND_LAST_EXCEPT() misc::HrException( __LINE__,__FILE__,GetLastError() )
#define UPWND_NOGFX_EXCEPT() misc::NoGfxException( __LINE__,__FILE__ )
