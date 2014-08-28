/* rtmdss
 *
 * Component source file.
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


#include "sub_rtc.h"

#include <rtmdds/ddsportmgmt.h>


SubscriberRTC::SubscriberRTC(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
    port_("SubscriberPort")
{
}


SubscriberRTC::~SubscriberRTC()
{
}


RTC::ReturnCode_t SubscriberRTC::onInitialize()
{
    // Register the port with the component
    addDDSSubPort("SubscriberPort", port_, *this);
    return RTC::RTC_OK;
}


RTC::ReturnCode_t SubscriberRTC::onExecute(RTC::UniqueId ec_id)
{
#ifdef EXEC_IS_NEW
    if (port_.isNew())
    {
        std::cout << "isNew: true\n";
    }
    else
    {
        std::cout << "isNew: false\n";
    }
#endif
    TestDataType tdt;
    if (port_.read(&tdt))
    {
        std::cout << "Received value: " << tdt.int_value << ':' <<
            tdt.str_value << '\n';
    }
    else
    {
        std::cout << "No data received\n";
    }
    return RTC::RTC_OK;
}


static const char* spec[] =
{
    "implementation_id", "SubComp",
    "type_name",         "subcomp",
    "description",       "DDS subscriber component",
    "version",           "1.0",
    "vendor",            "Geoffrey Biggs, AIST",
    "category",          "Example",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

extern "C"
{
    void rtc_init(RTC::Manager* manager)
    {
        coil::Properties profile(spec);
        manager->registerFactory(profile, RTC::Create<SubscriberRTC>,
                RTC::Delete<SubscriberRTC>);
    }
};

