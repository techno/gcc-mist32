// 2001-07-17 Benjamin Kosnik  <bkoz@redhat.com>

// Copyright (C) 2001, 2002, 2003 Free Software Foundation
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// 22.2.7.1.1 messages members

#include <locale>
#include <testsuite_hooks.h>

void test03()
{
  using namespace std;
  typedef std::messages<char>::catalog catalog;
  typedef std::messages<char>::string_type string_type;

  bool test = true;
  // This is defined through CXXFLAGS in scripts/testsuite_flags[.in].
  const char* dir = LOCALEDIR;

  // basic construction
  locale loc_c = locale::classic();
  locale loc_us = __gnu_test::try_named_locale("en_US");
  locale loc_fr = __gnu_test::try_named_locale("fr_FR");
  locale loc_de = __gnu_test::try_named_locale("de_DE");
  VERIFY( loc_c != loc_de );
  VERIFY( loc_us != loc_fr );
  VERIFY( loc_us != loc_de );
  VERIFY( loc_de != loc_fr );

  // cache the messages facets
  const messages<char>& mssg_c = use_facet<messages<char> >(loc_c); 
  const messages<char>& mssg_us = use_facet<messages<char> >(loc_us); 
  const messages<char>& mssg_fr = use_facet<messages<char> >(loc_fr); 
  const messages<char>& mssg_de = use_facet<messages<char> >(loc_de); 

  // catalog open(const string&, const locale&) const;
  // string_type get(catalog, int, int, const string_type& ) const; 
  // void close(catalog) const;

  // Check US (en_US) locale.
  catalog cat_us = mssg_fr.open("libstdc++", loc_c, dir);
  string s01 = mssg_us.get(cat_us, 0, 0, "please");
  string s02 = mssg_us.get(cat_us, 0, 0, "thank you");
  VERIFY ( s01 == "please" );
  VERIFY ( s02 == "thank you" );
  mssg_us.close(cat_us);
}

int main()
{
  test03();
  return 0;
}
