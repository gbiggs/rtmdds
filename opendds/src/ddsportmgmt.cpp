/* rtmdss
 *
 * Source file for port management functions.
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

#include <rtmdds/ddsportmgmt.h>

#include <iostream>
#include <string>

bool RTC::addDDSSubPort(char const* name, DDSSubPortBase& port, RTObject_impl& rtc)
{
    std::string propkey("port.ddsport.");
    propkey += name;
    coil::Properties& props(rtc.getProperties());
    props.getNode(propkey) << props.getNode("port.ddsport.sub");

    bool result = rtc.addPort(port);
    if (!result)
    {
        std::cerr << "addDDSSubPort() failed\n";
        return result;
    }

    port.init(props.getNode(propkey));
    return result;
}


bool RTC::addDDSPubPort(char const* name, DDSPubPortBase& port, RTObject_impl& rtc)
{
    std::string propkey("port.ddsport.");
    propkey += name;
    coil::Properties& props(rtc.getProperties());
    props.getNode(propkey) << props.getNode("port.ddsport.pub");

    bool result = rtc.addPort(port);
    if (!result)
    {
        std::cerr << "addDDSPubPort() failed\n";
        return result;
    }

    port.init(props.getNode(propkey));
    return result;
}

