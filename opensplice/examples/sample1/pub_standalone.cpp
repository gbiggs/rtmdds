/* rtmdss
 *
 * Source file for launching the component in stand-alone mode.
 *
 * Copyright 2011 Geoffrey Biggs geoffrey.biggs@aist.go.jp
 *     RT-Synthesis Research Group
 *     Intelligent Systems Research Institute,
 *     National Institute of Advanced Industrial Science and Technology (AIST),
 *     Japan
 *     All rights reserved.
 *
 * This file is part of rtmdds.
 *
 * rtmdss is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * rtmdss is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with rtmdss. If not, see
 * <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <rtm/Manager.h>
#include <string>
#include <stdlib.h>
#include <rtm/NVUtil.h>

#include "pub_rtc.h"

void ModuleInit(RTC::Manager* manager)
{
    rtc_init(manager);
    RTC::RtcBase* comp;
    comp = manager->createComponent("PubComp");

    if (comp == NULL)
    {
        std::cerr << "PubComp component creation failed." << std::endl;
        abort();
    }
    
  // for Debug
  std::cout << "Creating a component: \"PublisherRTC\"....";
  std::cout << "succeed." << std::endl;

  RTC::ComponentProfile_var prof;
  prof = comp->get_component_profile();
  std::cout << "=================================================" << std::endl;
  std::cout << " Component Profile" << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;
  std::cout << "InstanceID:     " << prof->instance_name << std::endl;
  std::cout << "Implementation: " << prof->type_name << std::endl;
  std::cout << "Description:    " << prof->description << std::endl;
  std::cout << "Version:        " << prof->version << std::endl;
  std::cout << "Maker:          " << prof->vendor << std::endl;
  std::cout << "Category:       " << prof->category << std::endl;
  std::cout << "  Other properties   " << std::endl;
  NVUtil::dump(prof->properties);
  std::cout << "=================================================" << std::endl;

  RTC::PortServiceList* portlist;
  portlist = comp->get_ports();

  for (CORBA::ULong i(0), n(portlist->length()); i < n; ++i)
    {
      RTC::PortService_ptr port;
      port = (*portlist)[i];
      std::cout << "================================================="
		<< std::endl;
      std::cout << "Port" << i << " (name): ";
      std::cout << port->get_port_profile()->name << std::endl;
      std::cout << "-------------------------------------------------"
		<< std::endl;    
      RTC::PortInterfaceProfileList iflist;
      iflist = port->get_port_profile()->interfaces;

      for (CORBA::ULong i(0), n(iflist.length()); i < n; ++i)
	{
	  std::cout << "I/F name: ";
	  std::cout << iflist[i].instance_name << std::endl;
	  std::cout << "I/F type: ";
	  std::cout << iflist[i].type_name << std::endl;
	  const char* pol;
	  pol = iflist[i].polarity == 0 ? "PROVIDED" : "REQUIRED";
	  std::cout << "Polarity: " << pol << std::endl;
	}
      std::cout << "- properties -" << std::endl;
      NVUtil::dump(port->get_port_profile()->properties);
      std::cout << "-------------------------------------------------" << std::endl;
    }
}


int main(int argc, char** argv)
{
    RTC::Manager *manager;
    manager = RTC::Manager::init(argc, argv);
    manager->init(argc, argv);
    manager->setModuleInitProc(ModuleInit);
    manager->activateManager();
    manager->runManager();

    return 0;
}

