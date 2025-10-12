#pragma once

#define UPWND_EXCEPT( hr ) engine::core::HrException( __LINE__,__FILE__,(hr) )
#define UPWND_LAST_EXCEPT() engine::core::HrException( __LINE__,__FILE__,GetLastError() )
#define UPWND_NOGFX_EXCEPT() engine::core::NoGfxException( __LINE__,__FILE__ )
