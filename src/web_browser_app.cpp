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

#include <boost/filesystem/path.hpp>
#include <wx/stdpaths.h>

#include "config.h"
#include "web_browser_app.h"

void WebApp::OnInitCmdLine( wxCmdLineParser &parser ) {
	if( !m_app_config.enable_command_line && parser.GetParamCount( ) > 0 ) {
		throw config_denied_exception{config_denied_exception_kind::enable_command_line};
	}
	wxApp::OnInitCmdLine( parser );
	parser.AddParam( "URL to open", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );
}

bool WebApp::OnCmdLineParsed( wxCmdLineParser &parser ) {
	if( !wxApp::OnCmdLineParsed( parser ) ) {
		return false;
	}

	if( parser.GetParamCount( ) ) {
		m_url = parser.GetParam( 0 );
	}

	return true;
}

void WebFrame::OnScrollLineUp( wxCommandEvent & ) {
	m_browser->LineUp( );
}

void WebFrame::OnScrollLineDown( wxCommandEvent & ) {
	m_browser->LineDown( );
}

void WebFrame::OnScrollPageUp( wxCommandEvent & ) {
	m_browser->PageUp( );
}

void WebFrame::OnScrollPageDown( wxCommandEvent & ) {
	m_browser->PageDown( );
}

SourceViewDialog::~SourceViewDialog( ) {}

wxIMPLEMENT_APP( WebApp );

namespace {
	auto get_exec_path( ) {
		static auto const p_str = wxStandardPaths::Get( ).GetExecutablePath( );
		return boost::filesystem::path{p_str.mb_str( wxConvUTF8 ).data( )};
	}
	auto get_config_file( ) {
		return get_exec_path( ).replace_extension( ".config" ).string( );
	}
} // namespace

bool WebApp::OnInit( ) {
	if( !wxApp::OnInit( ) ) {
		return false;
	}

	{
		static auto const conf_file = get_config_file( );
		if( !boost::filesystem::exists( conf_file ) ) {
			m_app_config.to_file( conf_file, false );
		} else {
			m_app_config = daw::json::from_file<config_t>( conf_file );
		}
	}

	if( m_app_config.home_url.empty( ) ) {
		m_app_config.home_url = "http://localhost";
	}
	m_frame = new WebFrame{m_app_config.home_url, m_app_config};
	m_frame->Show( );

	return true;
}

WebApp::WebApp( ) : m_url{}, m_frame{}, m_app_config{} {}

WebFrame::WebFrame( wxString const &url, config_t const &app_config )
    : wxFrame{nullptr, wxID_ANY, app_config.app_title.c_str( )}, m_app_config{&app_config} {

	if( boost::filesystem::exists( m_app_config->app_icon ) &&
	    boost::filesystem::is_regular_file( m_app_config->app_icon ) ) {
		wxString const icon_path{m_app_config->app_icon};
		wxBitmap const bmp{wxImage{icon_path}};
		wxIcon icon;
		icon.CopyFromBitmap( bmp );
		SetIcon( icon );
	} else {
		wxLogMessage( "%s", "Error: invalid app_icon path in config; path='" + m_app_config->app_icon + "'" );
	}
	wxFrame::SetTitle( m_app_config->app_title.c_str( ) );

	auto topsizer = std::make_unique<wxBoxSizer>( wxVERTICAL );

	// Create the toolbar
	if( m_app_config->enable_toolbar ) {
		m_toolbar = wxFrame::CreateToolBar( wxTB_TEXT );
		m_toolbar->SetToolBitmapSize( wxSize{32, 32} );

		auto back = wxArtProvider::GetBitmap( wxART_GO_BACK, wxART_TOOLBAR );
		auto forward = wxArtProvider::GetBitmap( wxART_GO_FORWARD, wxART_TOOLBAR );
#ifdef __WXGTK__
		auto stop = wxArtProvider::GetBitmap( "gtk-stop", wxART_TOOLBAR );
#else
		auto stop = wxBitmap( stop_xpm );
#endif
#ifdef __WXGTK__
		auto refresh = wxArtProvider::GetBitmap( "gtk-refresh", wxART_TOOLBAR );
#else
		auto refresh = wxBitmap( refresh_xpm );
#endif

		m_toolbar_back = m_toolbar->AddTool( wxID_ANY, _( "Back" ), back );
		m_toolbar_forward = m_toolbar->AddTool( wxID_ANY, _( "Forward" ), forward );
		m_toolbar_stop = m_toolbar->AddTool( wxID_ANY, _( "Stop" ), stop );
		m_toolbar_reload = m_toolbar->AddTool( wxID_ANY, _( "Reload" ), refresh );
		m_url = new wxTextCtrl{m_toolbar, wxID_ANY, wxT( "" ), wxDefaultPosition, wxSize{400, -1}, wxTE_PROCESS_ENTER};
		m_toolbar->AddControl( m_url, _( "URL" ) );
		m_toolbar_tools = m_toolbar->AddTool( wxID_ANY, _( "Menu" ), wxBitmap( wxlogo_xpm ) );

		m_toolbar->Realize( );
		// Set find values.
		m_findFlags = wxWEBVIEW_FIND_DEFAULT;
		m_findText = wxEmptyString;
		m_findCount = 0;

		// Create panel for find toolbar.
		auto panel = new wxPanel{this};
		topsizer->Add( panel, wxSizerFlags( ).Expand( ) );

		// Create sizer for panel.
		auto panel_sizer = new wxBoxSizer{wxVERTICAL};
		panel->SetSizer( panel_sizer );

		// Create the find toolbar.
		m_find_toolbar = new wxToolBar{panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		                               wxTB_HORIZONTAL | wxTB_TEXT | wxTB_HORZ_LAYOUT};
		m_find_toolbar->Hide( );
		panel_sizer->Add( m_find_toolbar, wxSizerFlags( ).Expand( ) );

		// Create find control.
		m_find_ctrl = new wxTextCtrl{m_find_toolbar,    wxID_ANY,        wxEmptyString,
		                             wxDefaultPosition, wxSize{140, -1}, wxTE_PROCESS_ENTER};

		// Find options menu
		auto findmenu = std::make_unique<wxMenu>( );
		m_find_toolbar_wrap = findmenu->AppendCheckItem( wxID_ANY, "Wrap" );
		m_find_toolbar_matchcase = findmenu->AppendCheckItem( wxID_ANY, "Match Case" );
		m_find_toolbar_wholeword = findmenu->AppendCheckItem( wxID_ANY, "Entire Word" );
		m_find_toolbar_highlight = findmenu->AppendCheckItem( wxID_ANY, "Highlight" );
		// Add find toolbar tools.
		m_find_toolbar->SetToolSeparation( 7 );
		m_find_toolbar_done =
		    m_find_toolbar->AddTool( wxID_ANY, "Close", wxArtProvider::GetBitmap( wxART_CROSS_MARK ) );
		m_find_toolbar->AddSeparator( );
		m_find_toolbar->AddControl( m_find_ctrl, "Find" );
		m_find_toolbar->AddSeparator( );
		m_find_toolbar_next = m_find_toolbar->AddTool(
		    wxID_ANY, "Next", wxArtProvider::GetBitmap( wxART_GO_DOWN, wxART_TOOLBAR, wxSize{16, 16} ) );
		m_find_toolbar_previous = m_find_toolbar->AddTool(
		    wxID_ANY, "Previous", wxArtProvider::GetBitmap( wxART_GO_UP, wxART_TOOLBAR, wxSize{16, 16} ) );
		m_find_toolbar->AddSeparator( );
		m_find_toolbar_options = m_find_toolbar->AddTool(
		    wxID_ANY, "Options", wxArtProvider::GetBitmap( wxART_PLUS, wxART_TOOLBAR, wxSize{16, 16} ), "",
		    wxITEM_DROPDOWN );
		m_find_toolbar_options->SetDropdownMenu( findmenu.release( ) );
		m_find_toolbar->Realize( );
	}

	// Create the info panel
	m_info = new wxInfoBar{this};
	topsizer->Add( m_info, wxSizerFlags( ).Expand( ) );

	// Create the webview
	m_browser = wxWebView::New( this, wxID_ANY, url );

	topsizer->Add( m_browser, wxSizerFlags( ).Expand( ).Proportion( 1 ) );

	// We register the wxfs:// protocol for testing purposes
	m_browser->RegisterHandler( wxSharedPtr<wxWebViewHandler>( new wxWebViewArchiveHandler{"wxfs"} ) );
	// And the memory: file system
	m_browser->RegisterHandler( wxSharedPtr<wxWebViewHandler>( new wxWebViewFSHandler{"memory"} ) );

	SetSizer( topsizer.release( ) );

	// Set a more sensible size for web browsing
	SetSize( wxSize{800, 600} );

	// Create a log window
	if( m_app_config->enable_debug_window ) {
		new wxLogWindow{this, _( "Logging" ), true, false};
	}

	// Create the Tools menu
	m_tools_menu = std::make_unique<wxMenu>( );
	wxMenuItem *print = m_tools_menu->Append( wxID_ANY, _( "Print" ) );
	wxMenuItem *viewSource = m_tools_menu->Append( wxID_ANY, _( "View Source" ) );
	wxMenuItem *viewText = m_tools_menu->Append( wxID_ANY, _( "View Text" ) );
	m_tools_menu->AppendSeparator( );
	m_tools_layout = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Use Layout Zoom" ) );
	m_tools_tiny = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Tiny" ) );
	m_tools_small = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Small" ) );
	m_tools_medium = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Medium" ) );
	m_tools_large = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Large" ) );
	m_tools_largest = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Largest" ) );
	m_tools_menu->AppendSeparator( );
	m_tools_handle_navigation = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Handle Navigation" ) );
	m_tools_handle_new_window = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Handle New Windows" ) );
	m_tools_menu->AppendSeparator( );

	// Find
	m_find = m_tools_menu->Append( wxID_ANY, _( "Find" ) );
	m_tools_menu->AppendSeparator( );

	// History menu
	m_tools_history_menu = new wxMenu{};
	wxMenuItem *clearhist = m_tools_history_menu->Append( wxID_ANY, _( "Clear History" ) );
	m_tools_enable_history = m_tools_history_menu->AppendCheckItem( wxID_ANY, _( "Enable History" ) );
	m_tools_history_menu->AppendSeparator( );

	m_tools_menu->AppendSubMenu( m_tools_history_menu, "History" );

	// Create an editing menu
	wxMenu *editmenu = new wxMenu{};
	m_edit_cut = editmenu->Append( wxID_ANY, _( "Cut" ) );
	m_edit_copy = editmenu->Append( wxID_ANY, _( "Copy" ) );
	m_edit_paste = editmenu->Append( wxID_ANY, _( "Paste" ) );
	editmenu->AppendSeparator( );
	m_edit_undo = editmenu->Append( wxID_ANY, _( "Undo" ) );
	m_edit_redo = editmenu->Append( wxID_ANY, _( "Redo" ) );
	editmenu->AppendSeparator( );
	m_edit_mode = editmenu->AppendCheckItem( wxID_ANY, _( "Edit Mode" ) );

	m_tools_menu->AppendSeparator( );
	m_tools_menu->AppendSubMenu( editmenu, "Edit" );

	auto scroll_menu = std::make_unique<wxMenu>( );
	m_scroll_line_up = scroll_menu->Append( wxID_ANY, "Line &up" );
	m_scroll_line_down = scroll_menu->Append( wxID_ANY, "Line &down" );
	m_scroll_page_up = scroll_menu->Append( wxID_ANY, "Page u&p" );
	m_scroll_page_down = scroll_menu->Append( wxID_ANY, "Page d&own" );
	m_tools_menu->AppendSubMenu( scroll_menu.release( ), "Scroll" );

	auto *script = m_tools_menu->Append( wxID_ANY, _( "Run Script" ) );

	// Selection menu
	auto selection = std::make_unique<wxMenu>( );
	m_selection_clear = selection->Append( wxID_ANY, _( "Clear Selection" ) );
	m_selection_delete = selection->Append( wxID_ANY, _( "Delete Selection" ) );
	auto *selectall = selection->Append( wxID_ANY, _( "Select All" ) );

	editmenu->AppendSubMenu( selection.release( ), "Selection" );

	auto *loadscheme = m_tools_menu->Append( wxID_ANY, _( "Custom Scheme Example" ) );
	auto *usememoryfs = m_tools_menu->Append( wxID_ANY, _( "Memory File System Example" ) );

	m_context_menu = m_tools_menu->AppendCheckItem( wxID_ANY, _( "Enable Context Menu" ) );

	// By default we want to handle navigation and new windows
	m_tools_handle_navigation->Check( );
	m_tools_handle_new_window->Check( );
	m_tools_enable_history->Check( );

	if( !m_browser->CanSetZoomType( wxWEBVIEW_ZOOM_TYPE_LAYOUT ) ) {
		m_tools_layout->Enable( false );
	}

	// Connect the toolbar events
	if( m_app_config->enable_toolbar ) {
		Connect( m_toolbar_back->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnBack ), nullptr, this );
		Connect( m_toolbar_forward->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnForward ), nullptr, this );
		Connect( m_toolbar_stop->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnStop ), nullptr, this );
		Connect( m_toolbar_reload->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnReload ), nullptr, this );
		Connect( m_toolbar_tools->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnToolsClicked ), nullptr,
		         this );

		Connect( m_url->GetId( ), wxEVT_TEXT_ENTER, wxCommandEventHandler( WebFrame::OnUrl ), nullptr, this );

		// Connect find toolbar events.
		Connect( m_find_toolbar_done->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnFindDone ), nullptr,
		         this );
		Connect( m_find_toolbar_next->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnFindText ), nullptr,
		         this );
		Connect( m_find_toolbar_previous->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnFindText ), nullptr,
		         this );

		// Connect find control events.
		Connect( m_find_ctrl->GetId( ), wxEVT_TEXT, wxCommandEventHandler( WebFrame::OnFindText ), nullptr, this );
		Connect( m_find_ctrl->GetId( ), wxEVT_TEXT_ENTER, wxCommandEventHandler( WebFrame::OnFindText ), nullptr,
		         this );
	}
	// Connect the webview events
	Connect( m_browser->GetId( ), wxEVT_WEBVIEW_NAVIGATING, wxWebViewEventHandler( WebFrame::OnNavigationRequest ),
	         nullptr, this );
	Connect( m_browser->GetId( ), wxEVT_WEBVIEW_NAVIGATED, wxWebViewEventHandler( WebFrame::OnNavigationComplete ),
	         nullptr, this );
	Connect( m_browser->GetId( ), wxEVT_WEBVIEW_LOADED, wxWebViewEventHandler( WebFrame::OnDocumentLoaded ), nullptr,
	         this );
	Connect( m_browser->GetId( ), wxEVT_WEBVIEW_ERROR, wxWebViewEventHandler( WebFrame::OnError ), nullptr, this );
	Connect( m_browser->GetId( ), wxEVT_WEBVIEW_NEWWINDOW, wxWebViewEventHandler( WebFrame::OnNewWindow ), nullptr,
	         this );
	if( m_app_config->enable_title_change ) {
		Connect( m_browser->GetId( ), wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEventHandler( WebFrame::OnTitleChanged ),
		         nullptr, this );
	}

	if( m_app_config->enable_toolbar ) {
		// Connect the menu events
		Connect( viewSource->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnViewSourceRequest ), nullptr,
		         this );
		Connect( viewText->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnViewTextRequest ), nullptr, this );
		Connect( print->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnPrint ), nullptr, this );
		Connect( m_tools_layout->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnZoomLayout ), nullptr, this );
		Connect( m_tools_tiny->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSetZoom ), nullptr, this );
		Connect( m_tools_small->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSetZoom ), nullptr, this );
		Connect( m_tools_medium->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSetZoom ), nullptr, this );
		Connect( m_tools_large->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSetZoom ), nullptr, this );
		Connect( m_tools_largest->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSetZoom ), nullptr, this );
		Connect( clearhist->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnClearHistory ), nullptr, this );
		Connect( m_tools_enable_history->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnEnableHistory ),
		         nullptr, this );
		Connect( m_edit_cut->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnCut ), nullptr, this );
		Connect( m_edit_copy->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnCopy ), nullptr, this );
		Connect( m_edit_paste->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnPaste ), nullptr, this );
		Connect( m_edit_undo->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnUndo ), nullptr, this );
		Connect( m_edit_redo->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnRedo ), nullptr, this );
		Connect( m_edit_mode->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnMode ), nullptr, this );
		Connect( m_scroll_line_up->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnScrollLineUp ), nullptr,
		         this );
		Connect( m_scroll_line_down->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnScrollLineDown ), nullptr,
		         this );
		Connect( m_scroll_page_up->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnScrollPageUp ), nullptr,
		         this );
		Connect( m_scroll_page_down->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnScrollPageDown ), nullptr,
		         this );
		Connect( script->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnRunScript ), nullptr, this );
		Connect( m_selection_clear->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnClearSelection ), nullptr,
		         this );
		Connect( m_selection_delete->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnDeleteSelection ),
		         nullptr, this );
		Connect( selectall->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnSelectAll ), nullptr, this );
		Connect( loadscheme->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnLoadScheme ), nullptr, this );
		Connect( usememoryfs->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnUseMemoryFS ), nullptr, this );
		Connect( m_find->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnFind ), nullptr, this );
		Connect( m_context_menu->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnEnableContextMenu ), nullptr,
		         this );
	}
	// Connect the idle events
	Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler( WebFrame::OnIdle ), nullptr, this );
}

WebFrame::~WebFrame( ) {}

void WebFrame::UpdateState( ) {
	if( m_app_config->enable_toolbar ) {
		m_toolbar->EnableTool( m_toolbar_back->GetId( ), m_browser->CanGoBack( ) );
		m_toolbar->EnableTool( m_toolbar_forward->GetId( ), m_browser->CanGoForward( ) );

		if( m_browser->IsBusy( ) ) {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), true );
		} else {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), false );
		}
		m_url->SetValue( m_browser->GetCurrentURL( ) );
	}
	if( m_app_config->enable_title_change ) {
		SetTitle( m_browser->GetCurrentTitle( ) );
	}
}

void WebFrame::OnIdle( wxIdleEvent &WXUNUSED( evt ) ) {
	if( m_browser->IsBusy( ) ) {
		wxSetCursor( wxCURSOR_ARROWWAIT );
		if( m_app_config->enable_toolbar ) {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), true );
		}
	} else {
		wxSetCursor( wxNullCursor );
		if( m_app_config->enable_toolbar ) {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), false );
		}
	}
}

void WebFrame::OnUrl( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->is_valid_url( m_url->GetValue( ).mb_str( wxConvUTF8 ).data( ) ) ) {
		return;
	}
	m_browser->LoadURL( m_url->GetValue( ) );
	m_browser->SetFocus( );
	UpdateState( );
}

void WebFrame::OnBack( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_navigation ) {
		return;
	}
	m_browser->GoBack( );
	UpdateState( );
}

void WebFrame::OnForward( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_navigation ) {
		return;
	}
	m_browser->GoForward( );
	UpdateState( );
}

void WebFrame::OnStop( wxCommandEvent &WXUNUSED( evt ) ) {
	m_browser->Stop( );
	UpdateState( );
}

void WebFrame::OnReload( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_reload ) {
		return;
	}
	m_browser->Reload( );
	UpdateState( );
}

void WebFrame::OnClearHistory( wxCommandEvent &WXUNUSED( evt ) ) {
	m_browser->ClearHistory( );
	UpdateState( );
}

void WebFrame::OnEnableHistory( wxCommandEvent &WXUNUSED( evt ) ) {
	m_browser->EnableHistory( m_tools_enable_history->IsChecked( ) );
	UpdateState( );
}

void WebFrame::OnCut( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_clipboard || !m_app_config->enable_edit ) {
		return;
	}
	m_browser->Cut( );
}

void WebFrame::OnCopy( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_clipboard ) {
		return;
	}
	m_browser->Copy( );
}

void WebFrame::OnPaste( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_clipboard || !m_app_config->enable_edit ) {
		return;
	}
	m_browser->Paste( );
}

void WebFrame::OnUndo( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_edit ) {
		return;
	}
	m_browser->Undo( );
}

void WebFrame::OnRedo( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_edit ) {
		return;
	}
	m_browser->Redo( );
}

void WebFrame::OnMode( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_edit ) {
		return;
	}
	m_browser->SetEditable( m_edit_mode->IsChecked( ) );
}

void WebFrame::OnLoadScheme( wxCommandEvent &WXUNUSED( evt ) ) {
	wxFileName helpfile{"../help/doc.zip"};
	helpfile.MakeAbsolute( );
	wxString path = helpfile.GetFullPath( );
	// Under MSW we need to flip the slashes
	path.Replace( "\\", "/" );
	path = "wxfs:///" + path + ";protocol=zip/doc.htm";
	m_browser->LoadURL( path );
}

void WebFrame::OnUseMemoryFS( wxCommandEvent &WXUNUSED( evt ) ) {
	m_browser->LoadURL( "memory:page1.htm" );
}

void WebFrame::OnEnableContextMenu( wxCommandEvent &evt ) {
	m_browser->EnableContextMenu( evt.IsChecked( ) );
}

void WebFrame::OnFind( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_search ) {
		return;
	}
	auto value = m_browser->GetSelectedText( );
	if( value.Len( ) > 150 ) {
		value.Truncate( 150 );
	}
	m_find_ctrl->SetValue( value );
	if( m_app_config->enable_toolbar ) {
		if( !m_find_toolbar->IsShown( ) ) {
			m_find_toolbar->Show( true );
			SendSizeEvent( );
		}
		m_find_ctrl->SelectAll( );
	}
}

void WebFrame::OnFindDone( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_search ) {
		return;
	}
	m_browser->Find( "" );
	if( m_app_config->enable_toolbar ) {
		m_find_toolbar->Show( false );
	}
	SendSizeEvent( );
}

void WebFrame::OnFindText( wxCommandEvent &evt ) {
	if( !m_app_config->enable_search ) {
		return;
	}
	int flags = 0;

	if( m_app_config->enable_toolbar ) {
		if( m_find_toolbar_wrap->IsChecked( ) ) {
			flags |= wxWEBVIEW_FIND_WRAP;
		}

		if( m_find_toolbar_wholeword->IsChecked( ) ) {
			flags |= wxWEBVIEW_FIND_ENTIRE_WORD;
		}

		if( m_find_toolbar_matchcase->IsChecked( ) ) {
			flags |= wxWEBVIEW_FIND_MATCH_CASE;
		}

		if( m_find_toolbar_highlight->IsChecked( ) ) {
			flags |= wxWEBVIEW_FIND_HIGHLIGHT_RESULT;
		}

		if( m_find_toolbar_previous->GetId( ) == evt.GetId( ) ) {
			flags |= wxWEBVIEW_FIND_BACKWARDS;
		}
	}

	wxString find_text = m_find_ctrl->GetValue( );
	auto count = m_browser->Find( find_text, flags );

	if( m_findText != find_text ) {
		m_findCount = static_cast<int>( count );
		m_findText = find_text;
	}

	if( count != wxNOT_FOUND || find_text.IsEmpty( ) ) {
		m_find_ctrl->SetBackgroundColour( *wxWHITE );
	} else {
		m_find_ctrl->SetBackgroundColour( wxColour{255, 101, 101} );
	}

	m_find_ctrl->Refresh( );

	// Log the result, note that count is zero indexed.
	if( count != m_findCount ) {
		count++;
	}
	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "Searching for:%s  current match:%i/%i", m_findText.c_str( ), count, m_findCount );
	}
}

/**
 * Callback invoked when there is a request to load a new page (for instance
 * when the user clicks a link)
 */
void WebFrame::OnNavigationRequest( wxWebViewEvent &evt ) {
	if( false && !m_app_config->enable_navigation ) {
		evt.Veto( );
		if( m_app_config->enable_toolbar ) {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), false );
		}
		return;
	}
	if( m_info->IsShown( ) ) {
		m_info->Dismiss( );
	}

	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "%s", "Navigation request to '" + evt.GetURL( ) + "' (target='" + evt.GetTarget( ) + "')" );
	}

	wxASSERT( m_browser->IsBusy( ) );

	// If we don't want to handle navigation then veto the event and navigation
	// will not take place, we also need to stop the loading animation
	if( !m_tools_handle_navigation->IsChecked( ) ) {
		evt.Veto( );
		if( m_app_config->enable_toolbar ) {
			m_toolbar->EnableTool( m_toolbar_stop->GetId( ), false );
		}
	} else {
		UpdateState( );
	}
}

void WebFrame::OnNavigationComplete( wxWebViewEvent &evt ) {
	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "%s", "Navigation complete; url='" + evt.GetURL( ) + "'" );
	}
	UpdateState( );
}

void WebFrame::OnDocumentLoaded( wxWebViewEvent &evt ) {
	// Only notify if the document is the main frame, not a subframe
	if( evt.GetURL( ) == m_browser->GetCurrentURL( ) ) {
		if( m_app_config->enable_debug_window ) {
			wxLogMessage( "%s", "Document loaded; url='" + evt.GetURL( ) + "'" );
		}
	}
	UpdateState( );
}

void WebFrame::OnNewWindow( wxWebViewEvent &evt ) {
	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "%s", "New window; url='" + evt.GetURL( ) + "'" );
	}
	// If we handle new window events then just load them in this window as we
	// are a single window browser
	if( m_tools_handle_new_window->IsChecked( ) ) {
		m_browser->LoadURL( evt.GetURL( ) );
	}

	UpdateState( );
}

void WebFrame::OnTitleChanged( wxWebViewEvent &evt ) {
	if( !m_app_config->enable_title_change ) {
		return;
	}
	SetTitle( evt.GetString( ) );
	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "%s", "Title changed; title='" + evt.GetString( ) + "'" );
	}
}

void WebFrame::OnViewSourceRequest( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_view_source ) {
		return;
	}
	SourceViewDialog dlg( this, m_browser->GetPageSource( ) );
	dlg.ShowModal( );
}

void WebFrame::OnViewTextRequest( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_view_text ) {
		return;
	}
	wxDialog textViewDialog{
	    this, wxID_ANY, "Page Text", wxDefaultPosition, wxSize{700, 500}, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER};
	auto text = std::make_unique<wxStyledTextCtrl>( &textViewDialog, wxID_ANY );
	text->SetText( m_browser->GetPageText( ) );
	auto sizer = std::make_unique<wxBoxSizer>( wxVERTICAL );
	sizer->Add( text.release( ), 1, wxEXPAND );
	SetSizer( sizer.release( ) );
	textViewDialog.ShowModal( );
}

void WebFrame::OnToolsClicked( wxCommandEvent &WXUNUSED( evt ) ) {
	if( m_browser->GetCurrentURL( ) == "" ) {
		return;
	}

	m_tools_tiny->Check( false );
	m_tools_small->Check( false );
	m_tools_medium->Check( false );
	m_tools_large->Check( false );
	m_tools_largest->Check( false );

	auto zoom = m_browser->GetZoom( );
	switch( zoom ) {
	case wxWEBVIEW_ZOOM_TINY:
		m_tools_tiny->Check( );
		break;
	case wxWEBVIEW_ZOOM_SMALL:
		m_tools_small->Check( );
		break;
	case wxWEBVIEW_ZOOM_MEDIUM:
		m_tools_medium->Check( );
		break;
	case wxWEBVIEW_ZOOM_LARGE:
		m_tools_large->Check( );
		break;
	case wxWEBVIEW_ZOOM_LARGEST:
		m_tools_largest->Check( );
		break;
	}

	m_edit_cut->Enable( m_browser->CanCut( ) );
	m_edit_copy->Enable( m_browser->CanCopy( ) );
	m_edit_paste->Enable( m_browser->CanPaste( ) );

	m_edit_undo->Enable( m_browser->CanUndo( ) );
	m_edit_redo->Enable( m_browser->CanRedo( ) );

	m_selection_clear->Enable( m_browser->HasSelection( ) );
	m_selection_delete->Enable( m_browser->HasSelection( ) );

	m_context_menu->Check( m_browser->IsContextMenuEnabled( ) );

	// Firstly we clear the existing menu items, then we add the current ones
	wxMenuHistoryMap::const_iterator it;
	for( it = m_histMenuItems.begin( ); it != m_histMenuItems.end( ); ++it ) {
		m_tools_history_menu->Destroy( it->first );
	}
	m_histMenuItems.clear( );

	wxVector<wxSharedPtr<wxWebViewHistoryItem>> back = m_browser->GetBackwardHistory( );
	wxVector<wxSharedPtr<wxWebViewHistoryItem>> forward = m_browser->GetForwardHistory( );

	wxMenuItem *item;

	unsigned int i;
	for( i = 0; i < back.size( ); i++ ) {
		item = m_tools_history_menu->AppendRadioItem( wxID_ANY, back[i]->GetTitle( ) );
		m_histMenuItems[item->GetId( )] = back[i];
		Connect( item->GetId( ), wxEVT_MENU, wxCommandEventHandler( WebFrame::OnHistory ), nullptr, this );
	}

	wxString title = m_browser->GetCurrentTitle( );
	if( title.empty( ) ) {
		title = "(untitled)";
	}

	item = m_tools_history_menu->AppendRadioItem( wxID_ANY, title );
	item->Check( );

	// No need to connect the current item
	m_histMenuItems[item->GetId( )] = wxSharedPtr<wxWebViewHistoryItem>{
	    new wxWebViewHistoryItem{m_browser->GetCurrentURL( ), m_browser->GetCurrentTitle( )}};

	for( i = 0; i < forward.size( ); i++ ) {
		item = m_tools_history_menu->AppendRadioItem( wxID_ANY, forward[i]->GetTitle( ) );
		m_histMenuItems[item->GetId( )] = forward[i];
		Connect( item->GetId( ), wxEVT_TOOL, wxCommandEventHandler( WebFrame::OnHistory ), nullptr, this );
	}

	wxPoint position = ScreenToClient( wxGetMousePosition( ) );
	PopupMenu( m_tools_menu.get( ), position.x, position.y );
}

void WebFrame::OnSetZoom( wxCommandEvent &evt ) {
	if( !m_app_config->enable_zoom ) {
		return;
	}
	if( evt.GetId( ) == m_tools_tiny->GetId( ) ) {
		m_browser->SetZoom( wxWEBVIEW_ZOOM_TINY );
	} else if( evt.GetId( ) == m_tools_small->GetId( ) ) {
		m_browser->SetZoom( wxWEBVIEW_ZOOM_SMALL );
	} else if( evt.GetId( ) == m_tools_medium->GetId( ) ) {
		m_browser->SetZoom( wxWEBVIEW_ZOOM_MEDIUM );
	} else if( evt.GetId( ) == m_tools_large->GetId( ) ) {
		m_browser->SetZoom( wxWEBVIEW_ZOOM_LARGE );
	} else if( evt.GetId( ) == m_tools_largest->GetId( ) ) {
		m_browser->SetZoom( wxWEBVIEW_ZOOM_LARGEST );
	} else {
		wxFAIL_MSG( "Unknown event id" );
	}
}

void WebFrame::OnZoomLayout( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_zoom ) {
		return;
	}
	if( m_tools_layout->IsChecked( ) ) {
		m_browser->SetZoomType( wxWEBVIEW_ZOOM_TYPE_LAYOUT );
	} else {
		m_browser->SetZoomType( wxWEBVIEW_ZOOM_TYPE_TEXT );
	}
}

void WebFrame::OnHistory( wxCommandEvent &evt ) {
	m_browser->LoadHistoryItem( m_histMenuItems[evt.GetId( )] );
}

void WebFrame::OnRunScript( wxCommandEvent &WXUNUSED( evt ) ) {
	wxTextEntryDialog dialog{this, "Enter JavaScript to run.", wxGetTextFromUserPromptStr, "",
	                         wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE};
	if( dialog.ShowModal( ) == wxID_OK ) {
		m_browser->RunScript( dialog.GetValue( ) );
	}
}

void WebFrame::OnClearSelection( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_select ) {
		return;
	}
	m_browser->ClearSelection( );
}

void WebFrame::OnDeleteSelection( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_select ) {
		return;
	}
	m_browser->DeleteSelection( );
}

void WebFrame::OnSelectAll( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_select ) {
		return;
	}
	m_browser->SelectAll( );
}

/**
 * Callback invoked when a loading error occurs
 */
void WebFrame::OnError( wxWebViewEvent &evt ) {
#define WX_ERROR_CASE( type )                                                                                          \
	case type:                                                                                                         \
		category = #type;                                                                                              \
		break;

	wxString category;
	switch( evt.GetInt( ) ) {
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_CONNECTION );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_CERTIFICATE );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_AUTH );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_SECURITY );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_NOT_FOUND );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_REQUEST );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_USER_CANCELLED );
		WX_ERROR_CASE( wxWEBVIEW_NAV_ERR_OTHER );
	default:
		throw std::logic_error( "Unknown event type in OnError" );
	}

	if( m_app_config->enable_debug_window ) {
		wxLogMessage( "%s", "Error; url='" + evt.GetURL( ) + "', error='" + category + " (" + evt.GetString( ) + ")'" );
	}

	// Show the info bar with an error
	m_info->ShowMessage( _( "An error occurred loading " ) + evt.GetURL( ) + "\n" + "'" + category + "'",
	                     wxICON_ERROR );

	UpdateState( );
}

void WebFrame::OnPrint( wxCommandEvent &WXUNUSED( evt ) ) {
	if( !m_app_config->enable_printing ) {
		return;
	}
	m_browser->Print( );
}

WebApp::~WebApp( ) {}

SourceViewDialog::SourceViewDialog( wxWindow *parent, wxString source )
    : wxDialog{parent,           wxID_ANY,
               "Source Code",    wxDefaultPosition,
               wxSize{700, 500}, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER} {

	auto text = std::make_unique<wxStyledTextCtrl>( this, wxID_ANY );
	text->SetMarginWidth( 1, 30 );
	text->SetMarginType( 1, wxSTC_MARGIN_NUMBER );
	text->SetText( source );

	text->StyleClearAll( );
	text->SetLexer( wxSTC_LEX_HTML );
	text->StyleSetForeground( wxSTC_H_DOUBLESTRING, wxColour{255, 0, 0} );
	text->StyleSetForeground( wxSTC_H_SINGLESTRING, wxColour{255, 0, 0} );
	text->StyleSetForeground( wxSTC_H_ENTITY, wxColour{255, 0, 0} );
	text->StyleSetForeground( wxSTC_H_TAG, wxColour{0, 150, 0} );
	text->StyleSetForeground( wxSTC_H_TAGUNKNOWN, wxColour{0, 150, 0} );
	text->StyleSetForeground( wxSTC_H_ATTRIBUTE, wxColour{0, 0, 150} );
	text->StyleSetForeground( wxSTC_H_ATTRIBUTEUNKNOWN, wxColour{0, 0, 150} );
	text->StyleSetForeground( wxSTC_H_COMMENT, wxColour{150, 150, 150} );

	auto sizer = std::make_unique<wxBoxSizer>( wxVERTICAL );
	sizer->Add( text.release( ), 1, wxEXPAND );
	SetSizer( sizer.release( ) );
}
