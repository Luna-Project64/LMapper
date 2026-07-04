#include "Win.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <Windows.h>

extern char gPluginConfigDir[MAX_PATH];

const char* kDefaultConfig = R"(enabled: [ 1 ]

controller:
# Button mapping
- from: A
  to: A
- from: X
  to: B

- from: DpadLeft
  to: DpadLeft
- from: DpadRight
  to: DpadRight
- from: DpadDown
  to: DpadDown
- from: DpadUp
  to: DpadUp
- from: Start
  to: Start

- from: R
  to: R
- from: L
  to: Z

#- from:
#    type: axis
#    axis: 120
#    comparer: More
#    offset: LeftTrigger
#  to: Z
- from: Back
  to: L
#- from:
#    type: axis
#    axis: 120
#    comparer: More
#    offset: RightTrigger
#  to: R

# CButtons Mapping
- from:
    type: axis
    axis: 16000
    offset: RightX
    comparer: More
  to: CRight
- from:
    type: axis
    axis: -16000
    offset: RightX
    comparer: Less
  to: CLeft
- from:
    type: axis
    axis: 16000
    offset: RightY
    comparer: More
  to: CUp
- from:
    type: axis
    axis: -16000
    offset: RightY
    comparer: Less
  to: CDown

# Stick Mapping
- type: bilinear
  fromX:
    center: 0
    max: 32000
    offset: LeftX
  toX:
    center: 0
    max: 85
    offset: X
  fromY:
    center: 0
    max: 32000
    offset: LeftY
  toY:
    center: 0
    max: 85
    offset: Y
  deadzone: 0.01)";

namespace Win
{
    std::string ConfigPath()
    {
		static char _strPath[4096] = {};
		if ('\0' != *_strPath)
			return _strPath;

        if (*gPluginConfigDir)
        {
            strncpy_s(_strPath, gPluginConfigDir, sizeof(_strPath));
		}
        else
        {
            SHGetFolderPath(NULL,
                CSIDL_APPDATA,
                NULL,
                0,
                _strPath);
        }

		PathAppend(_strPath, "LMapper");
		CreateDirectory(_strPath, nullptr); // can fail, ignore errors
		PathAppend(_strPath, "cfg.yaml");
		int fd = _open(_strPath, _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, 0666);
		if (-1 != fd)
		{
			_write(fd, kDefaultConfig, sizeof(kDefaultConfig) - 1);
			_close(fd);
		}

        return _strPath;
    }
}
