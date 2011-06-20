// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Physics& p)
{
  for (Uint i=0; i<p.size(); ++i)
  {
    os << i << " : ";
    if (p.must_compute(i) == false) os << p.var(i);
    os << "\n";
  }
  os << std::flush;
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////
