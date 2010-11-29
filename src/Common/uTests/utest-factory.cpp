// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for component factory"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Common/uTests/DummyComponents.hpp"

#include "Common/CFactories.hpp"
#include "Common/CBuilder.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

struct CFactoryFixture
{
  /// common setup for each test case
  CFactoryFixture() {}

  /// common tear-down for each test case
  ~CFactoryFixture() {}
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( CFactoryTest, CFactoryFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get_factory )
{
  CFactories::Ptr factories = Core::instance().root()->get_child_type< CFactories >("Factories");

  BOOST_CHECK( factories->get_factory< CAbstract >() != nullptr );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( component_builder )
{
  ComponentBuilder< CConcrete1, CAbstract, LibCommon > cc1;
  ComponentBuilder< CConcrete2, CAbstract, LibCommon > cc2;

  CFactories::Ptr factories = Core::instance().root()->get_child_type< CFactories >("Factories");

  CFactoryT<CAbstract>::Ptr cabstract_factory = factories->get_factory< CAbstract >();
  BOOST_CHECK( cabstract_factory != nullptr );
  BOOST_CHECK_EQUAL( cabstract_factory->factory_type_name() , std::string("CAbstract") );

  CBuilder::Ptr cconcrete1_builder = cabstract_factory->get_child_type< CBuilder >( "CF.Common.CConcrete1" );
  BOOST_CHECK( cconcrete1_builder != nullptr );
  BOOST_CHECK_EQUAL( cconcrete1_builder->builder_concrete_type_name() , std::string("CConcrete1") );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
