// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include "Common/Option.hpp"

using namespace boost::assign;
using namespace CF::Common;

Option::Option(const std::string & name, const std::string & desc, boost::any def)
  : Property(def),
    m_default(def),
    m_name(name),
    m_description(desc)
{
  m_restricted_list += def;

  m_is_option = true;
}

////////////////////////////////////////////////////////////////////////////

Option::~Option()
{

}

////////////////////////////////////////////////////////////////////////////

void Option::configure_option ( XmlNode& node )
{
  this->configure(node); // update the value

  // call all process functors
  BOOST_FOREACH( Option::Trigger_t& process, m_triggers )
      process();
}

////////////////////////////////////////////////////////////////////////////

void Option::mark_basic()
{
  if(!has_tag("basic"))
    add_tag("basic");
}

////////////////////////////////////////////////////////////////////////////

void Option::change_value ( const boost::any& value )
{
//  cf_assert(/*m_restricted_list.size() == 1 ||*/
//            std::find(m_restricted_list.begin(), m_restricted_list.end(), value)
//            != m_restricted_list.end());

  Property::change_value(value);

  // call all process functors
  BOOST_FOREACH( Option::Trigger_t& process, m_triggers )
    process();
};
