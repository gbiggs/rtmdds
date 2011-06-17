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

