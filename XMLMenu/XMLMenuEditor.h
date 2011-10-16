#ifndef _XMLMenuEditor_h_
#define _XMLMenuEditor_h_

#include <CtrlLib/CtrlLib.h>

#include "XMLMenuInterface.h"

NAMESPACE_UPP

#define LAYOUTFILE <XMLMenu/XMLMenuEditor.lay>
#include <CtrlCore/lay.h>

////////////////////////////////////////////////////////////////////////////////
// bar editor class -- allow editing of a single menu/toolbar
class XMLBarEditor : public ParentCtrl
{
	friend class XMLBarsEditor;
	private:
		// toolbar being edited
		XMLToolBar *bar;
		
		// current item, if any
		XMLToolBarItem *curItem;
		
		// size of item editor
		Size itemSize;
	
		// item editor pane
		WithItemEditorLayout<ParentCtrl> itemPane;
		
		// tree ctrl containing bar structure
		TreeCtrl barTree;
		
		// vertical splitter dividing tree from item editor
		Splitter vertSplitter;
		
		// layouts control
		void Layout(void);
		
		// builds bar tree
		void buildTree(int root, XMLToolBar const *bar);
		
		// drag and drop support
				
		// dragging element
		void dragCb(void);
		
		// dropping between elements (inserts between)
		void dropInsertCb(int parent, int ii, PasteClip& d);
		
		// item selection callback
		void itemSelCb(void);
		
		// refresh current bar
		void RefreshBar(int treeRoot = 0, XMLToolBar *subBar = NULL);

	protected:
	
	public:
		typedef XMLBarEditor CLASSNAME;
		
		// constructor
		XMLBarEditor();
		
		// gets minimum size of bar editor
		Size GetMinSize(void);
		
		// sets bar being edited
		void SetBar(XMLToolBar *bar);
		
};

////////////////////////////////////////////////////////////////////////////////
class XMLBarsEditor : public ParentCtrl
{
	private:
	
		// local copy of toolbars being edited
		XMLToolBars toolBars;
	
		// MenuBars and ToolBars lists
		WithBarListLayout<ParentCtrl> barListPane;
		
		// horizontal splitter
		Splitter horzSplitter;
		
		// the bar editor
		XMLBarEditor barEditor;
		
		// size of selector
		Size selectorSize;

		// bar selection callback
		void barSelCb(void);
	
	protected:
	
	public:
	
		typedef XMLBarsEditor CLASSNAME;
	
		// constructor
		XMLBarsEditor();

		// gets minimum size of bar editor
		Size GetMinSize(void);

		// adjust layout on win changes
		void Layout(void);

		// set title
		void SetTitle(const char *s);
		
		// sets the local copy of toolbars
		void SetToolBars(XMLToolBars const &tb);
		
		// gets the local copy of toolbars
		XMLToolBars &GetToolBars(void) { barEditor.RefreshBar(); return toolBars; }
};

////////////////////////////////////////////////////////////////////////////////
class XMLMenuEditor : public WithMenuEditorLayout<TopWindow>
{
	private :
		// interface to access menu system data
		XMLMenuInterface *iFace;
		
		// left pane
		WithCmdBoxLayout<ParentCtrl> cmdPane;
		
		// the menus and bars editors
		XMLBarsEditor menusEditor, barsEditor;

		// right pane - contains tab ctrl
		TabCtrl tabCtrl;
		
		// vertical size of button area
		int buttonVertSize;
		
		// original sizes of cmd area and item editor area
		Size cmdSize, editorSize, tabCtrlSize;

		// minimum window width and height
		int minWidth, minHeight;
	
	protected :
	
		void cancelCb(void);
		void okCb(void);
		
		// adjust layout on win changes
		void Layout(void);
		
		// gets commands from iFace and fills the command list
		void GetCommands(void);
	
	public :
	
		typedef XMLMenuEditor CLASSNAME;
		
		XMLMenuEditor(XMLMenuInterface *iFace);
		
		~XMLMenuEditor();
		
};

END_UPP_NAMESPACE

#endif
