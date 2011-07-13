// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionComponent.hpp"

#include "CActionDirector.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

ComponentBuilder < CActionDirector, CAction, LibCommon > CActionDirector_Builder;

/////////////////////////////////////////////////////////////////////////////////////

CActionDirector::CActionDirector(const std::string& name): CAction(name)
{
  m_options.add_option< OptionArrayT<std::string> >("ActionList", std::vector<std::string>())
      ->set_description("Names of the actions to execute in sequence");
}

void CActionDirector::execute()
{
  Option& actions_prop = option("ActionList");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  BOOST_FOREACH(const std::string& action_name, actions)
  {
    dynamic_cast<CAction&>(get_child(action_name)).execute();
  }
}

CActionDirector& CActionDirector::append(CAction& action)
{
  return append(action.as_ptr<CAction>());
}

CActionDirector& CActionDirector::append(const CAction::Ptr& action)
{
  Component::Ptr existing_child = get_child_ptr(action->name());
  if(is_null(existing_child))
  {
    if(action->has_parent())
    {
      CLink& action_link = create_component<CLink>(action->name());
      action_link.link_to(action);
    }
    else
    {
      add_component(action);
    }
  }
  else
  {
    // If a child with the given name existed, check that it corresponds to the supplied action
    CAction::Ptr existing_action = boost::dynamic_pointer_cast<CAction>(existing_child);

    if(is_null(existing_action))
      throw ValueExists(FromHere(), "A component named " + action->name() + " already exists in " + uri().string() + ", but it is not a CAction");

    if(existing_action != action)
      throw ValueExists(FromHere(), "An action named " + action->name() + " already exists in " + uri().string() + ", but it is different from the appended action");
  }

  Option& actions_prop = option("ActionList");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  actions.push_back(action->name());
  actions_prop.change_value(actions);

  return *this;
}


/////////////////////////////////////////////////////////////////////////////////////

CActionDirector& operator<<(CActionDirector& action_director, CAction& action)
{
  return action_director.append(action);
}

CActionDirector& operator<<(CActionDirector& action_director, const CAction::Ptr& action)
{
  return action_director.append(action);
}


/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////
