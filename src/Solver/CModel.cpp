// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <boost/algorithm/string/find.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/Log.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CGroup.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CTime.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;

Common::ComponentBuilder < CModel, Component, LibSolver > CModel_Builder;

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const std::string& name  ) :
  Component ( name )
{
   mark_basic();

   // options

   std::string cwd = boost::filesystem::current_path().string();

   m_options.add_option< OptionURI >("WorkingDir", URI( cwd ) )
       ->set_description("Your working directory")
       ->mark_basic();

   m_options.add_option< OptionURI >("ResultsDir", URI( cwd ) )
       ->set_description("Directory to store the output files")
       ->mark_basic();

   m_options.add_option< OptionT<Uint> >("CPUs", 1u )
       ->set_description("Number of cpus to use in simulation")
       ->mark_basic();

   // properties

   m_properties["steady"] = bool(true);

   // signals
   SignalPtr sig_create_physics = regist_signal ( "create_physics" , "Create the physical model", "Create Physics" );
        sig_create_physics->signal    -> connect ( boost::bind ( &CModel::signal_create_physics, this, _1 ) );
        sig_create_physics->signature -> connect ( boost::bind ( &CModel::signature_create_physics, this, _1) );

  SignalPtr sig_create_domain = regist_signal ( "create_domain" , "Create the domain and load a mesh", "Create Domain" );
       sig_create_domain->signal    -> connect ( boost::bind ( &CModel::signal_create_domain, this, _1 ) );
       sig_create_domain->signature -> connect ( boost::bind ( &CModel::signature_create_domain, this, _1) );

   SignalPtr sig_create_solver = regist_signal ( "create_solver" , "Create the solver", "Create Solver" );
       sig_create_solver->signal    -> connect ( boost::bind ( &CModel::signal_create_solver, this, _1 ) );
       sig_create_solver->signature -> connect ( boost::bind ( &CModel::signature_create_solver, this, _1) );

   regist_signal ( "simulate" , "Simulates this model", "Simulate" )
       ->signal->connect ( boost::bind ( &CModel::signal_simulate, this, _1 ) );

   m_tools = create_static_component_ptr<CGroup>("tools");
   m_tools->mark_basic();
}

CModel::~CModel() {}

////////////////////////////////////////////////////////////////////////////////

void CModel::simulate ()
{
  CFinfo << "\n" << name() << ": start simulation" << CFendl;

  // call all (non-linear) iterative solvers to solve this dt step

  boost_foreach(CSolver& is, find_components<CSolver>(*this))
    is.solve();

  CFinfo << name() << ": end simulation\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

CTime& CModel::time()
{
  return find_component<CTime>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Physics::PhysModel& CModel::physics()
{
  return find_component<Physics::PhysModel>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::domain()
{
  return find_component<CDomain>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::solver()
{
  return find_component<CSolver>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CGroup& CModel::tools()
{
  return *m_tools;
}

////////////////////////////////////////////////////////////////////////////////

Physics::PhysModel& CModel::create_physics( const std::string& builder )
{
  std::string pm_name = CBuilder::extract_reduced_name(builder);

  Physics::PhysModel::Ptr pm =
      build_component_abstract_type_reduced<Physics::PhysModel>( builder , pm_name );

  add_component(pm);

  return *pm;
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::create_domain( const std::string& name )
{
  return *this->create_component_ptr<CDomain>( name );
}

////////////////////////////////////////////////////////////////////////////////

CTime& CModel::create_time( const std::string& name )
{
  return *this->create_component_ptr<CTime>( name );
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::create_solver( const std::string& builder)
{
  std::string solver_name = CBuilder::extract_reduced_name(builder);

  CSolver::Ptr solver =
      build_component_abstract_type<CSolver>( builder, solver_name );

  add_component(solver);

  return *solver;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_physics ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  CFactory::Ptr pm_factory = Core::instance().factories().get_factory<Physics::PhysModel>();
  std::vector<std::string> pms;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *pm_factory ) )
  {
    pms.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add( "builder", std::string() , "Choose solver", pms, " ; ");
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_physics ( Common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder = options.option<std::string>( "builder" );
  create_physics( builder );
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_domain ( Common::SignalArgs& node )
{
  // no signature parameters needed
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_domain ( Common::SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  CDomain& domain = create_domain("Domain"); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  CFactory::Ptr solver_factory = Core::instance().factories().get_factory<CSolver>();
  std::vector<boost::any> solvers;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *solver_factory ) )
  {
    solvers.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add_option< OptionT<std::string> >( "builder", std::string() )
      ->set_description("Choose solver")
      ->restricted_list() = solvers;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder_name = options.value<std::string>( "builder" );
  create_solver(builder_name);
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_simulate ( Common::SignalArgs& node )
{
  this->simulate(); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
