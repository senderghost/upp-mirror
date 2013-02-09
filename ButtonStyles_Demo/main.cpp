#include <CtrlLib/CtrlLib.h>
#include <Controls4U/Controls4U.h>
#include <ButtonStyles/ButtonStyles.h>


using namespace Upp;
using namespace ButtonStyles;




// Accesseur permettant de recuperer le style par defaut en fonction du type passé
template<class STYLE>
inline STYLE  GetBFInitStyle() { return STYLE(); };

template<>
inline Button::Style  GetBFInitStyle<Button::Style>() { return Button::StyleNormal(); };

template<>
inline ButtonOption::Style  GetBFInitStyle<ButtonOption::Style>() { return ButtonOption::StyleDefault(); };



int subbuttonDeflate = 6;
int subbuttonBorderWidth = 3;

template <class STYLE >
STYLE MakeStyledButton_WithAppStyle(int style, int SubStyle, Image icon=Null)
{
	// in this case we use APP_STYLE_NUM  as FLAGS
	// --- NOT INTENDED TO BE USED THIS WAY ---
	//
	int col = 43;
	int alpha = 47;
	int v = alpha*col/255;
	STYLE s =  GetBFInitStyle<STYLE>();
	s.look[0] = MakeButtonLook( style, White, GrayColor(v),  alpha, subbuttonDeflate, subbuttonBorderWidth, icon );
	s.look[1] = MakeButtonLook( style, White, GrayColor(v),  alpha, subbuttonDeflate, subbuttonBorderWidth, icon );
	s.look[2] = MakeButtonLook( style, White, GrayColor(255),  255, subbuttonDeflate, subbuttonBorderWidth, icon );
	s.look[3] = MakeButtonLook( style, White, GrayColor(v),  alpha, subbuttonDeflate, subbuttonBorderWidth, icon );
	s.textcolor[0] = Black;
	s.textcolor[1] = Black;
	s.textcolor[2] = Black;
	s.textcolor[3] = Black;
	return s;
}

template <class BUTTON, int APP_STYLE_NUM, int APP_SUBSTYLE_NUM=0>
class FStyledButton : public BUTTON {
	public:
		typedef FStyledButton<BUTTON, APP_STYLE_NUM, APP_SUBSTYLE_NUM>  CLASSNAME;
		
		FStyledButton() {
			tStyle = MakeStyledButton_WithAppStyle<typename BUTTON::Style>(APP_STYLE_NUM, APP_SUBSTYLE_NUM);
			BUTTON::SetStyle(tStyle);
			}
//		void SetIcon(Image icon) { tStyle = MakeStyledButton_WithAppStyle<typename BUTTON::Style>(APP_STYLE_NUM, APP_SUBSTYLE_NUM, icon); BUTTON::SetModify(); }
//		void SetFlags(int flags)  { tStyle = MakeStyledButton_WithAppStyle<typename BUTTON::Style>(flags, APP_SUBSTYLE_NUM); BUTTON::SetModify(); };
		void SetIcon(Image icon) { tStyle = MakeStyledButton_WithAppStyle<typename BUTTON::Style>(APP_STYLE_NUM, APP_SUBSTYLE_NUM, icon); }
		void SetFlags(int flags)  { tStyle = MakeStyledButton_WithAppStyle<typename BUTTON::Style>(flags, APP_SUBSTYLE_NUM); };
		
	private:
		typename BUTTON::Style tStyle;
};

template <int APP_STYLE_NUM>
class MyButton :  public FStyledButton<Button, APP_STYLE_NUM> {
};


// =============================
//     C H A M E L E O N   END
// =============================


#define _CM_ ,
#define LAYOUTFILE <AlphaButtonTest/AlphaButtonTest.lay>
#include <CtrlCore/lay.h>
#undef _CM_

#define IMAGECLASS AlphaButtonImg
#define IMAGEFILE <AlphaButtonTest/AlphaButtonTest.iml>
#include <Draw/iml_header.h>


#define IMAGECLASS AlphaButtonImg
#define IMAGEFILE <AlphaButtonTest/AlphaButtonTest.iml>
#include <Draw/iml_source.h>



class AlphaButtonTestDlg : public WithAlphaButtonTestLayout<TopWindow> {
	typedef AlphaButtonTestDlg CLASSNAME;

	void ToggleIcon();
	void ToggleText();
	bool isIconSet;
	bool isTextSet;
	void updateCustomFlags(int flag, Option* ctrl);
	void updateBorderWidth();
	void updateDeflate();
	
	void refreshCustoms();
	int customFlags;

public:
	AlphaButtonTestDlg();
};

AlphaButtonTestDlg::AlphaButtonTestDlg()
: isIconSet(false)
, isTextSet(false)
, customFlags( BUTTON_STYLE_3g )
{

	CtrlLayout(*this, "");
	backImg.Set( AlphaButtonImg::BACKGROUND_S);
	bToggleIcon <<= THISBACK(ToggleIcon);
	bToggleText <<= THISBACK(ToggleText);
	
	opt_RightEndRound = 1;
	opt_LeftEndRound = 1;
	opt_ContainsSubbuton = 1;
	opt_SubbuttonRightRound = 1;
	opt_SubbuttonLeftRound = 1;
	
	opt_RightEndRound <<= THISBACK2(updateCustomFlags, ButtonLook::RIGHT_END_ROUND, &opt_RightEndRound);
	opt_LeftEndRound <<= THISBACK2(updateCustomFlags, ButtonLook::LEFT_END_ROUND, &opt_LeftEndRound);
	opt_ContainsSubbuton <<= THISBACK2(updateCustomFlags, ButtonLook::CONTAINS_SUBBUTON, &opt_ContainsSubbuton);
	opt_SubbuttonIsFull <<= THISBACK2(updateCustomFlags, ButtonLook::SUBBUTTON_IS_FULL, &opt_SubbuttonIsFull);
	opt_SubbuttonRightRound <<= THISBACK2(updateCustomFlags, ButtonLook::SUBBUTTON_RIGHT_END_ROUND, &opt_SubbuttonRightRound);
	opt_SubbuttonLeftRound <<= THISBACK2(updateCustomFlags, ButtonLook::SUBBUTTON_LEFT_END_ROUND, &opt_SubbuttonLeftRound);
	opt_SubItemOnRight <<= THISBACK2(updateCustomFlags, ButtonLook::SUB_ITEM_RIGHT, &opt_SubItemOnRight);
	opt_SubItemOnLeft <<= THISBACK2(updateCustomFlags, ButtonLook::SUB_ITEM_LEFT, &opt_SubItemOnLeft);
	
	sliderWidth.SetData(subbuttonBorderWidth);
	sliderWidth.Range(10);
	sliderWidth <<= THISBACK(updateBorderWidth);
	
	sliderDeflate.SetData(subbuttonDeflate);
	sliderWidth.Range(50);
	sliderDeflate <<= THISBACK(updateDeflate);
	
	refreshCustoms();
}


void AlphaButtonTestDlg::updateBorderWidth()
{
	subbuttonBorderWidth = ~sliderWidth;
	RLOG("updateBorderWidth() = " << subbuttonBorderWidth);
	refreshCustoms();
}
	
void AlphaButtonTestDlg::updateDeflate()
{
	subbuttonDeflate = ~sliderDeflate;
	RLOG("updateDeflate() = " << subbuttonDeflate);
	refreshCustoms();
}


void AlphaButtonTestDlg::updateCustomFlags(int flag, Option* ctrl)
{
	RLOG("updateCustomFlags()");
	customFlags = 0;
	customFlags |= (int)~opt_RightEndRound * ButtonLook::RIGHT_END_ROUND;
	customFlags |= (int)~opt_LeftEndRound * ButtonLook::LEFT_END_ROUND;
	customFlags |= (int)~opt_ContainsSubbuton * ButtonLook::CONTAINS_SUBBUTON;
	customFlags |= (int)~opt_SubbuttonIsFull * ButtonLook::SUBBUTTON_IS_FULL;
	customFlags |= (int)~opt_SubbuttonRightRound * ButtonLook::SUBBUTTON_RIGHT_END_ROUND;
	customFlags |= (int)~opt_SubbuttonLeftRound * ButtonLook::SUBBUTTON_LEFT_END_ROUND;
	customFlags |= (int)~opt_SubItemOnRight * ButtonLook::SUB_ITEM_RIGHT;
	customFlags |= (int)~opt_SubItemOnLeft * ButtonLook::SUB_ITEM_LEFT;
	refreshCustoms();
}

void AlphaButtonTestDlg::refreshCustoms()
{
	RLOG("refreshCustoms()");
	cust2.SetFlags(customFlags);
	cust5.SetFlags(customFlags);
	cust4.SetFlags(customFlags);
	cust3.SetFlags(customFlags);
	cust1.SetFlags(customFlags);
	Refresh();
}


void AlphaButtonTestDlg::ToggleIcon() {
	if (!isIconSet)
	{
		c27.SetIcon( AlphaButtonImg::UPP );
		c1.SetIcon( AlphaButtonImg::UPP );
		c3.SetIcon( AlphaButtonImg::UPP );
		c2.SetIcon( AlphaButtonImg::UPP );
		c4.SetIcon( AlphaButtonImg::UPP );
		c5.SetIcon( AlphaButtonImg::UPP );
		c6.SetIcon( AlphaButtonImg::UPP );
		c7.SetIcon( AlphaButtonImg::UPP );
		c8.SetIcon( AlphaButtonImg::UPP );
		c9.SetIcon( AlphaButtonImg::UPP );
		c10.SetIcon( AlphaButtonImg::UPP );
		c12.SetIcon( AlphaButtonImg::UPP );
		c13.SetIcon( AlphaButtonImg::UPP );
		c14.SetIcon( AlphaButtonImg::UPP );
		c15.SetIcon( AlphaButtonImg::UPP );
		c16.SetIcon( AlphaButtonImg::UPP );
		c17.SetIcon( AlphaButtonImg::UPP );
		c18.SetIcon( AlphaButtonImg::UPP  );
		c26.SetIcon( AlphaButtonImg::UPP );
		c20.SetIcon( AlphaButtonImg::UPP );
		c21.SetIcon( AlphaButtonImg::UPP );
		c22.SetIcon( AlphaButtonImg::UPP );
		c23.SetIcon( AlphaButtonImg::UPP );
		c24.SetIcon( AlphaButtonImg::UPP );
		c25.SetIcon( AlphaButtonImg::UPP );
		c19.SetIcon( AlphaButtonImg::UPP );
		c28.SetIcon( AlphaButtonImg::UPP );
	}
	else
	{
		c27.SetIcon( Null );
		c1.SetIcon( Null );
		c3.SetIcon( Null );
		c2.SetIcon( Null );
		c4.SetIcon( Null );
		c5.SetIcon( Null );
		c6.SetIcon( Null );
		c7.SetIcon( Null );
		c8.SetIcon( Null );
		c9.SetIcon( Null );
		c10.SetIcon( Null );
		c12.SetIcon( Null );
		c13.SetIcon( Null );
		c14.SetIcon( Null );
		c15.SetIcon( Null );
		c16.SetIcon( Null );
		c17.SetIcon( Null );
		c18.SetIcon( Null  );
		c26.SetIcon( Null );
		c20.SetIcon( Null );
		c21.SetIcon( Null );
		c22.SetIcon( Null );
		c23.SetIcon( Null );
		c24.SetIcon( Null );
		c25.SetIcon( Null );
		c19.SetIcon( Null );
		c28.SetIcon( Null );
	}
	isIconSet = ! isIconSet;
	Refresh();
}

void AlphaButtonTestDlg::ToggleText() {
	if (!isTextSet)
	{
		c27.SetLabel("Text");
		c1.SetLabel("Text");
		c3.SetLabel("Text");
		c2.SetLabel("Text");
		c4.SetLabel("Text");
		c5.SetLabel("Text");
		c6.SetLabel("Text");
		c7.SetLabel("Text");
		c8.SetLabel("Text");
		c9.SetLabel("Text");
		c10.SetLabel("Text");
		c12.SetLabel("Text");
		c13.SetLabel("Text");
		c14.SetLabel("Text");
		c15.SetLabel("Text");
		c16.SetLabel("Text");
		c17.SetLabel("Text");
		c18.SetLabel("Text");
		c26.SetLabel("Text");
		c20.SetLabel("Text");
		c21.SetLabel("Text");
		c22.SetLabel("Text");
		c23.SetLabel("Text");
		c24.SetLabel("Text");
		c25.SetLabel("Text");
		c19.SetLabel("Text");
		c28.SetLabel("Text");
	}
	else
	{
		c27.SetLabel("");
		c1.SetLabel("");
		c3.SetLabel("");
		c2.SetLabel("");
		c4.SetLabel("");
		c5.SetLabel("");
		c6.SetLabel("");
		c7.SetLabel("");
		c8.SetLabel("");
		c9.SetLabel("");
		c10.SetLabel("");
		c12.SetLabel("");
		c13.SetLabel("");
		c14.SetLabel("");
		c15.SetLabel("");
		c16.SetLabel("");
		c17.SetLabel("");
		c18.SetLabel("");
		c26.SetLabel("");
		c20.SetLabel("");
		c21.SetLabel("");
		c22.SetLabel("");
		c23.SetLabel("");
		c24.SetLabel("");
		c25.SetLabel("");
		c19.SetLabel("");
		c28.SetLabel("");
	}
	isTextSet = ! isTextSet;
	Refresh();
}

GUI_APP_MAIN
{
	AlphaButtonTestDlg().Run();
}
