#include "streamproof.h"
#include "../../../render/render.h"
#include "../../../settings.h"
#include <windows.h>

namespace menu
{
	namespace streamproof
	{
		void run()
		{
			if (!render || !render->detail || !render->detail->window)
				return;

			if (settings::menu::streamproof)
			{
				SetWindowDisplayAffinity(render->detail->window, WDA_EXCLUDEFROMCAPTURE);
			}
			else
			{
				SetWindowDisplayAffinity(render->detail->window, WDA_NONE);
			}
		}
	}
}
