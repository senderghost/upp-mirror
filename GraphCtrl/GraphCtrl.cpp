#include "GraphCtrl.h"

#include <plugin/jpg/jpg.h>
#include <plugin/tif/tif.h>

#define IMAGECLASS GraphCtrlImg
#define IMAGEFILE <GraphCtrl/GraphCtrl.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP
namespace GraphDraw_ns {

enum { DUMMY_KEY = K_TAB | K_9 | K_M | K_CTRL_Z }; // default values are set by  GraphCtrl_Keys::Reset();   ==> so static init set dummy values 

dword GraphCtrl_Keys::K_UNDO        = DUMMY_KEY;
dword GraphCtrl_Keys::K_REDO        = DUMMY_KEY;

dword GraphCtrl_Keys::K_AXIS_SELECT = DUMMY_KEY;
dword GraphCtrl_Keys::K_ZOOM        = DUMMY_KEY;
dword GraphCtrl_Keys::K_SCROLL      = DUMMY_KEY;

dword GraphCtrl_Keys::K_AXIS_ZOOM   = DUMMY_KEY;
dword GraphCtrl_Keys::K_AXIS_SCROLL = DUMMY_KEY;

dword GraphCtrl_Keys::K_ELEMENT_FLOAT_MOVERESIZE = DUMMY_KEY;
dword GraphCtrl_Keys::K_ELEMENT_FLOAT_MOVE       = DUMMY_KEY;

dword GraphCtrl_Keys::K_KBD_LEFT_SCROLL  = DUMMY_KEY;
dword GraphCtrl_Keys::K_KBD_RIGHT_SCROLL = DUMMY_KEY;
dword GraphCtrl_Keys::K_KBD_UP_SCROLL    = DUMMY_KEY;
dword GraphCtrl_Keys::K_KBD_DOWN_SCROLL  = DUMMY_KEY;

INITBLOCK
{
	GraphCtrl_Keys::Reset();
}




void SaveImageToFile(String fileName, const Image& img)
{
	if (GetFileExt(fileName) == ".png") {
		PNGEncoder encoder;
		encoder.SaveFile(fileName, img);
	} else if (GetFileExt(fileName) == ".jpg") {
		JPGEncoder encoder(90);
		encoder.SaveFile(fileName, img);
	} else if (GetFileExt(fileName) == ".tif") {
		TIFEncoder encoder;
		encoder.SaveFile(fileName, img);
	} else {
		Exclamation(Format(t_("File format \"%s\" not found"), GetFileExt(fileName)));
	}
}
	

};
END_UPP_NAMESPACE


