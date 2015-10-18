#include "ChromiumBrowserExample.h"

using namespace Upp;

#define IMAGECLASS IMG
#define IMAGEFILE <ChromiumBrowserExample/ChromiumBrowserExample.iml>
#include <Draw/iml.h>

#include "files.brc"


/* Table of Javascript functions that are forwarded to native code */
const char * const Upp::ChromiumBrowserJSFunctions[]={
	"JSExample1",
	"JSExample2",
	/* DO NOT forget nullptr at the end of the table */
	nullptr
};


ChromiumBrowserExample::ChromiumBrowserExample()
{
	CtrlLayout(*this, "Embedded Chromium Example " + Browser.GetVersion());
	Sizeable().MaximizeBox();
	Icon(IMG::icon);
	
	Back.SetImage(IMG::back);
	Forward.SetImage(IMG::forward);
	Refresh.SetImage(IMG::refresh);
	Go.SetImage(IMG::go);
	Stop.SetImage(IMG::stop);
	
	Browser.WhenUrlChange		= THISBACK(OnUrlChange);
	Browser.WhenTakeFocus		= THISBACK(OnTakeFocus);
	Browser.WhenKeyboard		= STDBACK(::ShowKeyboard);
	Browser.WhenConsoleMessage	= THISBACK(OnConsoleMessage);
	Browser.WhenMessage			= THISBACK(OnMessage);
	
	Back.WhenAction				= callback(&Browser, &ChromiumBrowser::GoBack);
	Forward.WhenAction			= callback(&Browser, &ChromiumBrowser::GoForward);
	Refresh.WhenAction			= callback(&Browser, &ChromiumBrowser::RefreshPage);
	Url.WhenEnter				= THISBACK(OnBrowse);
	Go.WhenAction				= THISBACK(OnBrowse);
	Stop.WhenAction				= callback(&Browser, &ChromiumBrowser::Stop);
	JSTests.WhenAction			= THISBACK(OnJSTests);

	//Delayed maximization - workaround of layout problem
	SetTimeCallback(100, THISBACK1(Maximize, false));
}



void ChromiumBrowserExample::OnJSTests()
{
	Browser.ShowHTML(String(test_page, test_page_length));
}


void ChromiumBrowserExample::OnMessage(String name, const Vector<Value>& par)
{
	String tmp = "Native function executed by JS:&[* " + name + "(";
	for (int i = 0; i < par.GetCount(); i++){
		if (i > 0) tmp += ',';
		tmp += DeQtfLf(par[i].ToString());
	}
	tmp += ") ]&&After you press OK javascript function will be executed by native code";
	PromptOK(tmp);
	
	Browser.ExecuteJavaScript(Format("CallbackExample(%d);", (int)Random()));
}


GUI_APP_MAIN
{
	StdLogSetup(LOG_FILE | LOG_CERR | LOG_TIMESTAMP | LOG_APPEND);
	SetLanguage( SetLNGCharset( GetSystemLNG(), CHARSET_UTF8 ) );
	
	if (ChromiumBrowser::IsChildProcess()){
		ChromiumBrowser::ChildProcess();
	}else{
		ChromiumBrowserExample().Run();
	}
}


