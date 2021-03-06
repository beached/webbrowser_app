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

#include <regex>
#include <string>

#include "config.h"

config_t::~config_t( ) {}

bool config_t::is_valid_url( boost::string_view url ) const {
	if( url_validators.empty( ) ) {
		return true;
	}
	for( auto const & url_validator: url_validators ) {
		if( url_validator.is_regex ) {
			std::regex r{ url_validator.url };
			if( std::regex_match( url.to_string( ), r ) ) {
				return true;
			}
		} else {
			return url_validator.url == url;
		}
	}
	return false;
}

url_validation_t::url_validation_t( ) : daw::json::JsonLink<url_validation_t>{}, is_regex{false}, url{""} {
	link_json( );
}

url_validation_t::url_validation_t( url_validation_t const &other )
    : daw::json::JsonLink<url_validation_t>{}, is_regex{other.is_regex}, url{other.url} {

	link_json( );
}

url_validation_t::url_validation_t( url_validation_t &&other )
    : daw::json::JsonLink<url_validation_t>{}, is_regex{std::move( other.is_regex )}, url{std::move( other.url )} {

	link_json( );
}

url_validation_t &url_validation_t::operator=( url_validation_t const &rhs ) {
	using std::swap;
	url_validation_t tmp{rhs};
	swap( *this, tmp );
	return *this;
}

url_validation_t &url_validation_t::operator=( url_validation_t &&rhs ) {
	using std::swap;
	url_validation_t tmp{std::move( rhs )};
	swap( *this, tmp );
	return *this;
}

url_validation_t::~url_validation_t( ) {}

void url_validation_t::link_json( ) {
	this->link_boolean( "is_regex", is_regex );
	this->link_string( "url", url );
}

config_t::config_t( )
    : daw::json::JsonLink<config_t>{}
    , app_icon{}
    , app_title{}
    , home_url{}
    , enable_clipboard{true}
    , enable_command_line{true}
    , enable_debug_window{true}
    , enable_edit{true}
    , enable_navigation{true}
    , enable_printing{true}
    , enable_reload{true}
    , enable_search{true}
    , enable_select{true}
    , enable_title_change{true}
    , enable_toolbar{true}
    , enable_view_source{true}
    , enable_view_text{true}
    , enable_zoom{true}
    , url_validators{} {

	link_json( );
}

config_t::config_t( config_t const &other )
    : daw::json::JsonLink<config_t>{}
    , app_icon{other.app_icon}
    , app_title{other.app_title}
    , home_url{other.home_url}
    , enable_clipboard{other.enable_clipboard}
    , enable_command_line{other.enable_command_line}
    , enable_debug_window{other.enable_debug_window}
    , enable_edit{other.enable_edit}
    , enable_navigation{other.enable_navigation}
    , enable_printing{other.enable_printing}
    , enable_reload{other.enable_reload}
    , enable_search{other.enable_search}
    , enable_select{other.enable_select}
    , enable_title_change{other.enable_title_change}
    , enable_toolbar{other.enable_toolbar}
    , enable_view_source{other.enable_view_source}
    , enable_view_text{other.enable_view_text}
    , enable_zoom{other.enable_zoom}
    , url_validators{other.url_validators} {

	link_json( );
}

config_t::config_t( config_t &&other )
    : daw::json::JsonLink<config_t>{}
    , app_icon{std::move( other.app_icon )}
    , app_title{std::move( other.app_title )}
    , home_url{std::move( other.home_url )}
    , enable_clipboard{std::move( other.enable_clipboard )}
    , enable_command_line{std::move( other.enable_command_line )}
    , enable_debug_window{std::move( other.enable_debug_window )}
    , enable_edit{std::move( other.enable_edit )}
    , enable_navigation{std::move( other.enable_navigation )}
    , enable_printing{std::move( other.enable_printing )}
    , enable_reload{std::move( other.enable_reload )}
    , enable_search{std::move( other.enable_search )}
    , enable_select{std::move( other.enable_select )}
    , enable_title_change{std::move( other.enable_title_change )}
    , enable_toolbar{std::move( other.enable_toolbar )}
    , enable_view_source{std::move( other.enable_view_source )}
    , enable_view_text{std::move( other.enable_view_text )}
    , enable_zoom{std::move( other.enable_zoom )}
    , url_validators{std::move( other.url_validators )} {

	link_json( );
}

config_t &config_t::operator=( config_t const &rhs ) {
	app_icon = rhs.app_icon;
	app_title = rhs.app_title;
	home_url = rhs.home_url;
	url_validators = rhs.url_validators;
	enable_clipboard = rhs.enable_clipboard;
	enable_command_line = rhs.enable_command_line;
	enable_debug_window = rhs.enable_debug_window;
	enable_edit = rhs.enable_edit;
	enable_navigation = rhs.enable_navigation;
	enable_printing = rhs.enable_printing;
	enable_reload = rhs.enable_reload;
	enable_search = rhs.enable_search;
	enable_select = rhs.enable_select;
	enable_title_change = rhs.enable_title_change;
	enable_toolbar = rhs.enable_toolbar;
	enable_view_source = rhs.enable_view_source;
	enable_view_text = rhs.enable_view_text;
	enable_zoom = rhs.enable_zoom;
	url_validators = rhs.url_validators;
	return *this;
}

config_t &config_t::operator=( config_t &&rhs ) {
	app_icon = std::move( rhs.app_icon );
	app_title = std::move( rhs.app_title );
	home_url = std::move( rhs.home_url );
	url_validators = std::move( rhs.url_validators );
	enable_clipboard = std::move( rhs.enable_clipboard );
	enable_command_line = std::move( rhs.enable_command_line );
	enable_debug_window = std::move( rhs.enable_debug_window );
	enable_edit = std::move( rhs.enable_edit );
	enable_navigation = std::move( rhs.enable_navigation );
	enable_printing = std::move( rhs.enable_printing );
	enable_reload = std::move( rhs.enable_reload );
	enable_search = std::move( rhs.enable_search );
	enable_select = std::move( rhs.enable_select );
	enable_title_change = std::move( rhs.enable_title_change );
	enable_toolbar = std::move( rhs.enable_toolbar );
	enable_view_source = std::move( rhs.enable_view_source );
	enable_view_text = std::move( rhs.enable_view_text );
	enable_zoom = std::move( rhs.enable_zoom );
	url_validators = std::move( rhs.url_validators );
	return *this;
}

void config_t::link_json( ) {
	this->link_string( "app_icon", app_icon );
	this->link_string( "app_title", app_title );
	this->link_string( "home_url", home_url );
	this->link_boolean( "enable_clipboard", enable_clipboard );
	this->link_boolean( "enable_command_line", enable_command_line );
	this->link_boolean( "enable_debug_window", enable_debug_window );
	this->link_boolean( "enable_edit", enable_edit );
	this->link_boolean( "enable_navigation", enable_navigation );
	this->link_boolean( "enable_printing", enable_printing );
	this->link_boolean( "enable_reload", enable_reload );
	this->link_boolean( "enable_search", enable_search );
	this->link_boolean( "enable_select", enable_select );
	this->link_boolean( "enable_title_change", enable_title_change );
	this->link_boolean( "enable_toolbar", enable_toolbar );
	this->link_boolean( "enable_view_source", enable_view_source );
	this->link_boolean( "enable_view_text", enable_view_text );
	this->link_boolean( "enable_zoom", enable_zoom );
	this->link_array( "url_validators", url_validators );
}

char const *config_denied_exception::config_param_t::to_string( type t ) noexcept {
	static constexpr char const *const result[] = {
	    "Access denied, enable_clipboard feature is not enabled",
	    "Access denied, enable_command_line feature is not enabled.",
	    "Access denied, enable_debug_window feature is not enabled.",
	    "Access denied, enable_edit feature is not enabled",
	    "Access denied, enable_naviation feature is not enabled",
	    "Access denied, enable_printing feature is not enabled",
	    "Access denied, enable_reload feature is not enabled",
	    "Access denied, enable_search feature is not enabled",
	    "Access denied, enable_select feature is not enabled",
	    "Access denied, enable_title_change feature is not enabled",
	    "Access denied, enable_toolbar feature is not enabled",
	    "Access denied, enable_view_source feature is not enabled",
	    "Access denied, enable_view_text feature is not enabled",
	    "Access denied, enable_zoom feature is not enabled",
	};
	return result[static_cast<uint8_t>( t )];
}
config_denied_exception::config_param_t::~config_param_t( ){};

config_denied_exception::~config_denied_exception( ) {}

config_denied_exception::config_denied_exception( config_param_t::type ex_type )
    : std::runtime_error{config_param_t::to_string( ex_type )} {}
