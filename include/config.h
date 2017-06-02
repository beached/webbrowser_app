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

#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <string>
#include <vector>

#include <daw/json/daw_json_link.h>

struct url_validation_t : public daw::json::JsonLink<url_validation_t> {
	bool is_regex;
	std::string url;

	url_validation_t( );
	url_validation_t( url_validation_t const &other );
	url_validation_t( url_validation_t &&other );
	url_validation_t &operator=( url_validation_t const &rhs );
	url_validation_t &operator=( url_validation_t &&rhs );
	~url_validation_t( );

  private:
	void link_json( );
}; // url_validation_t

struct config_t : public daw::json::JsonLink<config_t> {
	std::string app_icon;
	std::string app_title;
	std::string home_url;
	std::vector<url_validation_t> url_validators;
	bool enable_clipboard;
	bool enable_command_line;
	bool enable_debug_window;
	bool enable_edit;
	bool enable_navigation;
	bool enable_printing;
	bool enable_reload;
	bool enable_search;
	bool enable_select;
	bool enable_title_change;
	bool enable_toolbar;
	bool enable_view_source;
	bool enable_view_text;
	bool enable_zoom;

	bool is_valid_url( boost::string_view url ) const;

	config_t( );
	config_t( config_t const &other );
	config_t( config_t &&other );
	config_t &operator=( config_t const &rhs );
	config_t &operator=( config_t &&rhs );
	~config_t( );

  private:
	void link_json( );
};

struct config_denied_exception : public std::runtime_error {
	struct config_param_t final {
		enum class type : uint8_t {
			enable_clipboard,
			enable_command_line,
			enable_debug_window,
			enable_edit,
			enable_navigation,
			enable_printing,
			enable_reload,
			enable_search,
			enable_select,
			enable_title_change,
			enable_toolbar,
			enable_view_source,
			enable_view_text,
			enable_zoom,
		};
		static char const *to_string( type t ) noexcept;

		config_param_t( ) = default;
		config_param_t( config_param_t const & ) = default;
		config_param_t( config_param_t && ) = default;
		config_param_t &operator=( config_param_t const & ) = default;
		config_param_t &operator=( config_param_t && ) = default;
		~config_param_t( );
	}; // config_param_t

	~config_denied_exception( );
	config_denied_exception( config_denied_exception const & ) = default;
	config_denied_exception( config_denied_exception && ) = default;
	config_denied_exception &operator=( config_denied_exception const & ) = default;
	config_denied_exception &operator=( config_denied_exception && ) = default;

	explicit config_denied_exception( config_param_t::type ex_type );
}; // config_denied_exception
using config_denied_exception_kind = config_denied_exception::config_param_t::type;

