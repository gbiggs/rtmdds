/* rtmdss
 *
 * Source file for the DDS subscriber port base class.
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

#include <rtmdds/ddssubport.h>

using namespace RTC;

DDSSubPortBase::DDSSubPortBase(std::string name)
    : PortBase(name.c_str())
{
    addProperty("port.port_type", "DDSSubPort");
}


DDSSubPortBase::~DDSSubPortBase()
{
}


void DDSSubPortBase::init(coil::Properties const& props)
{
    props_ << props;
}


ReturnCode_t DDSSubPortBase::publishInterfaces(ConnectorProfile& cp)
{
    return RTC_OK;
}

