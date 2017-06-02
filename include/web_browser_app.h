// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <wx/wx.h>

#if !wxUSE_WEBVIEW_WEBKIT && !wxUSE_WEBVIEW_IE
#error "A wxWebView backend is required by this sample"
#endif

#include <wx/artprov.h>
#include <wx/cmdline.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#include <wx/infobar.h>
#include <wx/notifmsg.h>
#include <wx/settings.h>
#include <wx/stc/stc.h>
#include <wx/webview.h>
#include <wx/webviewarchivehandler.h>
#include <wx/webviewfshandler.h>

#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#if defined( __WXMSW__ ) || defined( __WXOSX__ )
#include "../images/refresh.xpm"
#include "../images/stop.xpm"
#endif

#include "../images/wxlogo.xpm"

#include "config.h"

// We map menu items to their history items
WX_DECLARE_HASH_MAP( int, wxSharedPtr<wxWebViewHistoryItem>, wxIntegerHash, wxIntegerEqual, wxMenuHistoryMap );

class WebFrame;

class WebApp : public wxApp {
	wxString m_url;
	WebFrame *m_frame;
	config_t m_app_config;

  public:
	WebApp( );
	virtual ~WebApp( );
	WebApp( WebApp const & ) = default;
	WebApp( WebApp && ) = default;
	WebApp &operator=( WebApp const & ) = default;
	WebApp &operator=( WebApp && ) = default;

	bool OnInit( ) override;
	void OnInitCmdLine( wxCmdLineParser &parser ) override;
	bool OnCmdLineParsed( wxCmdLineParser &parser ) override;
}; // WebApp

class WebFrame : public wxFrame {
	wxTextCtrl *m_url;
	wxWebView *m_browser;

	wxToolBar *m_toolbar;
	wxToolBarToolBase *m_toolbar_back;
	wxToolBarToolBase *m_toolbar_forward;
	wxToolBarToolBase *m_toolbar_stop;
	wxToolBarToolBase *m_toolbar_reload;
	wxToolBarToolBase *m_toolbar_tools;

	wxToolBarToolBase *m_find_toolbar_done;
	wxToolBarToolBase *m_find_toolbar_next;
	wxToolBarToolBase *m_find_toolbar_previous;
	wxToolBarToolBase *m_find_toolbar_options;
	wxMenuItem *m_find_toolbar_wrap;
	wxMenuItem *m_find_toolbar_highlight;
	wxMenuItem *m_find_toolbar_matchcase;
	wxMenuItem *m_find_toolbar_wholeword;

	std::unique_ptr<wxMenu> m_tools_menu;
	wxMenu *m_tools_history_menu;
	wxMenuItem *m_tools_layout;
	wxMenuItem *m_tools_tiny;
	wxMenuItem *m_tools_small;
	wxMenuItem *m_tools_medium;
	wxMenuItem *m_tools_large;
	wxMenuItem *m_tools_largest;
	wxMenuItem *m_tools_handle_navigation;
	wxMenuItem *m_tools_handle_new_window;
	wxMenuItem *m_tools_enable_history;
	wxMenuItem *m_edit_cut;
	wxMenuItem *m_edit_copy;
	wxMenuItem *m_edit_paste;
	wxMenuItem *m_edit_undo;
	wxMenuItem *m_edit_redo;
	wxMenuItem *m_edit_mode;
	wxMenuItem *m_scroll_line_up;
	wxMenuItem *m_scroll_line_down;
	wxMenuItem *m_scroll_page_up;
	wxMenuItem *m_scroll_page_down;
	wxMenuItem *m_selection_clear;
	wxMenuItem *m_selection_delete;
	wxMenuItem *m_find;
	wxMenuItem *m_context_menu;

	wxInfoBar *m_info;
	// wxStaticText *m_info_text;
	wxTextCtrl *m_find_ctrl;
	wxToolBar *m_find_toolbar;

	wxMenuHistoryMap m_histMenuItems;
	wxString m_findText;
	int m_findFlags;
	int m_findCount;
	config_t const *m_app_config;

  public:
	WebFrame( wxString const &url, config_t const &app_config );
	virtual ~WebFrame( );
	WebFrame( WebFrame && ) = default;
	WebFrame &operator=( WebFrame && ) = default;
	WebFrame( WebFrame const & ) = default;
	WebFrame &operator=( WebFrame const & ) = default;

	void UpdateState( );
	void OnIdle( wxIdleEvent &evt );
	void OnUrl( wxCommandEvent &evt );
	void OnBack( wxCommandEvent &evt );
	void OnForward( wxCommandEvent &evt );
	void OnStop( wxCommandEvent &evt );
	void OnReload( wxCommandEvent &evt );
	void OnClearHistory( wxCommandEvent &evt );
	void OnEnableHistory( wxCommandEvent &evt );
	void OnNavigationRequest( wxWebViewEvent &evt );
	void OnNavigationComplete( wxWebViewEvent &evt );
	void OnDocumentLoaded( wxWebViewEvent &evt );
	void OnNewWindow( wxWebViewEvent &evt );
	void OnTitleChanged( wxWebViewEvent &evt );
	void OnViewSourceRequest( wxCommandEvent &evt );
	void OnViewTextRequest( wxCommandEvent &evt );
	void OnToolsClicked( wxCommandEvent &evt );
	void OnSetZoom( wxCommandEvent &evt );
	void OnError( wxWebViewEvent &evt );
	void OnPrint( wxCommandEvent &evt );
	void OnCut( wxCommandEvent &evt );
	void OnCopy( wxCommandEvent &evt );
	void OnPaste( wxCommandEvent &evt );
	void OnUndo( wxCommandEvent &evt );
	void OnRedo( wxCommandEvent &evt );
	void OnMode( wxCommandEvent &evt );
	void OnZoomLayout( wxCommandEvent &evt );
	void OnHistory( wxCommandEvent &evt );
	void OnScrollLineUp( wxCommandEvent & );
	void OnScrollLineDown( wxCommandEvent & );
	void OnScrollPageUp( wxCommandEvent & );
	void OnScrollPageDown( wxCommandEvent & );
	void OnRunScript( wxCommandEvent &evt );
	void OnClearSelection( wxCommandEvent &evt );
	void OnDeleteSelection( wxCommandEvent &evt );
	void OnSelectAll( wxCommandEvent &evt );
	void OnLoadScheme( wxCommandEvent &evt );
	void OnUseMemoryFS( wxCommandEvent &evt );
	void OnFind( wxCommandEvent &evt );
	void OnFindDone( wxCommandEvent &evt );
	void OnFindText( wxCommandEvent &evt );
	// void OnFindOptions( wxCommandEvent &evt );
	void OnEnableContextMenu( wxCommandEvent &evt );
}; // WebFrame

struct SourceViewDialog : wxDialog {
	SourceViewDialog( wxWindow *parent, wxString source );
	virtual ~SourceViewDialog( );
	SourceViewDialog( SourceViewDialog const & ) = default;
	SourceViewDialog &operator=( SourceViewDialog const & ) = default;
	SourceViewDialog( SourceViewDialog && ) = default;
	SourceViewDialog &operator=( SourceViewDialog && ) = default;
}; // SourceViewDialog

