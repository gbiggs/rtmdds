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


#include "pub_rtc.h"

#include <rtmdds/ddsportmgmt.h>
#include <stdio.h>


PublisherRTC::PublisherRTC(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
    port_("PublisherPort", TestDataTypeTypeSupport::get_type_name()),
    count_(0)
{
}


PublisherRTC::~PublisherRTC()
{
}


RTC::ReturnCode_t PublisherRTC::onInitialize()
{
    // Register the port with the component
    addDDSPubPort("PublisherPort", port_, *this);
    // Register the data type to be used with the port
    if (TestDataTypeTypeSupport::register_type(port_.get_participant(),
                TestDataTypeTypeSupport::get_type_name()) != DDS_RETCODE_OK)
    {
        std::cerr << "PublisherComp: Failed to register data type\n";
        return RTC::RTC_ERROR;
    }
    return RTC::RTC_OK;
}


RTC::ReturnCode_t PublisherRTC::onExecute(RTC::UniqueId ec_id)
{
    TestDataType* tdt = TestDataTypeTypeSupport::create_data();
    if (!tdt)
    {
        std::cerr << "PublisherComp: failed to create data instance\n";
        return RTC::RTC_ERROR;
    }
    tdt->int_value = count_++;
    snprintf(tdt->str_value, 256, "String value #%d", tdt->int_value);
    port_.write(*tdt);
    std::cout << "Wrote value " << tdt->str_value << '\n';
    TestDataTypeTypeSupport::delete_data(tdt);
    return RTC::RTC_OK;
}


static const char* spec[] =
{
    "implementation_id", "PubComp",
    "type_name",         "pubcomp",
    "description",       "DDS publisher component",
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
        manager->registerFactory(profile, RTC::Create<PublisherRTC>,
                RTC::Delete<PublisherRTC>);
    }
};

