#include "TestLeptonica.h"

void TestLeptonica::onQuit()
{
	Break();
}

TestLeptonica::TestLeptonica()
{
	CtrlLayout(*this, "Leptonica library example suite");
	
	// sets PixRaster in PixRasterCtrl
	pixRasterCtrl.SetPixRaster(&pixRaster);
	pixRasterCtrl.ShowThumbnails(true);
	
	// connects button handlers
	LineRemovalButton <<= THISBACK(onLineRemoval);
	PageLayoutButton <<= THISBACK(onPageLayout);
	BaseLineButton <<= THISBACK(onBaseLine);
	QuitButton <<= THISBACK(onQuit);
}

GUI_APP_MAIN
{
	TestLeptonica testLept;

	testLept.Sizeable().Zoomable();
	testLept.Run();
}

