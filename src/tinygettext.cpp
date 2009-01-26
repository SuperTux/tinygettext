//  $Id$
//
//  TinyGetText
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifndef SDL_iconv_string
#include <iconv.h>
#endif

#include "po_file_reader.hpp"
#include "tinygettext.hpp"
#include "findlocale.hpp"

//#define TRANSLATION_DEBUG

#define log_warning std::cout

namespace tinygettext {

std::string convert(const std::string& text,
                    const std::string& from_charset,
                    const std::string& to_charset)
{
#ifdef SDL_iconv_string
  if (from_charset == to_charset)
    return text;

  char *in = new char[text.length() + 1];
  strcpy(in, text.c_str());
  char *out = SDL_iconv_string(to_charset.c_str(), from_charset.c_str(), in, text.length() + 1);
  delete[] in; 
  if(out == 0)
  {
    log_warning << "Error: conversion from " << from_charset << " to " << to_charset << " failed" << std::endl;
    return "";
  }
  std::string ret(out);
  SDL_free(out);
  return ret;
#else
#ifndef SDL_iconv_open
#define SDL_iconv_open iconv_open
#define SDL_iconv iconv
#define SDL_iconv_close iconv_close
#define SDL_free free
#endif
#ifndef ICONV_CONST
#define ICONV_CONST
#endif

  iconv_t cd = SDL_iconv_open(to_charset.c_str(), from_charset.c_str());

  size_t in_len = text.length();
  size_t out_len = text.length()*3; // FIXME: cross fingers that this is enough

  char*  out_orig = new char[out_len];
  char*  in_orig  = new char[in_len+1];
  strcpy(in_orig, text.c_str());

  char* out = out_orig;
  ICONV_CONST char* in  = in_orig;
  size_t out_len_temp = out_len; // iconv is counting down the bytes it has
                                 // written from this...

  size_t retval = SDL_iconv(cd, &in, &in_len, &out, &out_len_temp);
  out_len -= out_len_temp; // see above
  if (retval == (size_t) -1)
    {
      log_warning << strerror(errno) << std::endl;
      log_warning << "Error: conversion from " << from_charset << " to " << to_charset << " went wrong: " << retval << std::endl;
      return "";
    }
  SDL_iconv_close(cd);

  std::string ret(out_orig, out_len);
  delete[] out_orig;
  delete[] in_orig;
  return ret;
#endif
}

bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(lhs.length() - rhs.length(), rhs.length(), rhs) == 0;
}

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

void read_po_file(Dictionary& dict_, std::istream& in)
{
  POFileReader reader(in, dict_);
}

} // namespace tinygettext

/* EOF */