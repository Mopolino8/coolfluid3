// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/CDiscretization.hpp"

namespace CF {
namespace Solver {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CDiscretization::CDiscretization ( const CName& name  ) :
  CMethod ( name )
{
  BUILD_COMPONENT;
  properties()["brief"]=std::string("Discretization Method component");
  properties()["description"]=std::string("Handles the discretization of the PDE's");
}

////////////////////////////////////////////////////////////////////////////////

CDiscretization::~CDiscretization()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
