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


#include "pub_multi_rtc.h"

#include <rtmdds/ddsportmgmt.h>
#include <stdio.h>

// Create TestDataTypeFunc class source
CREATE_DATATYPEFUNC_SOURCE(TestDataType);

PublisherRTC::PublisherRTC(RTC::Manager* manager)
    :	RTC::DataFlowComponentBase(manager),
		testDataTypeTS_(TestDataTypeTypeSupport__alloc()),
		port1_("PublisherPort1",
			TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)),
		port2_("PublisherPort2",
			TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)),
		port3_("PublisherPort3",
			TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)),
		count1_(1), count2_(2), count3_(3)
{
}


PublisherRTC::~PublisherRTC()
{
}


RTC::ReturnCode_t PublisherRTC::onInitialize()
{
    // Register the port with the component
    addDDSPubPort("PublisherPort1", port1_, *this);
    addDDSPubPort("PublisherPort2", port2_, *this);
    addDDSPubPort("PublisherPort3", port3_, *this);
    // Register the data type to be used with the port
	if (!testDataTypeTS_)
	{
		std::cerr << "Allocating TypeSupport failed!!\n";
        return RTC::RTC_ERROR;
	};
    if (TestDataTypeTypeSupport_register_type(testDataTypeTS_, port1_.get_participant(),
                TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)) != DDS_RETCODE_OK)
    {
        std::cerr << "PublisherComp: Failed to register data type\n";
        return RTC::RTC_ERROR;
    }
    if (TestDataTypeTypeSupport_register_type(testDataTypeTS_, port2_.get_participant(),
                TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)) != DDS_RETCODE_OK)
    {
        std::cerr << "PublisherComp: Failed to register data type\n";
        return RTC::RTC_ERROR;
    }
    if (TestDataTypeTypeSupport_register_type(testDataTypeTS_, port3_.get_participant(),
                TestDataTypeTypeSupport_get_type_name(testDataTypeTS_)) != DDS_RETCODE_OK)
    {
        std::cerr << "PublisherComp: Failed to register data type\n";
        return RTC::RTC_ERROR;
    }
    return RTC::RTC_OK;
}


RTC::ReturnCode_t PublisherRTC::onExecute(RTC::UniqueId ec_id)
{
    TestDataType* tdt = TestDataType__alloc();
    if (!tdt)
    {
        std::cerr << "PublisherComp: failed to create data instance\n";
        return RTC::RTC_ERROR;
    }
    tdt->int_value = count1_;
    port1_.write(*tdt);
    tdt->int_value = count2_;
    port2_.write(*tdt);
    tdt->int_value = count3_;
    port3_.write(*tdt);
    std::cout << "port1: " << count1_ << ", ";
    std::cout << "port2: " << count2_ << ", ";
    std::cout << "port3: " << count3_ << std::endl;
    count1_ += 10;
    count2_ += 10;
    count3_ += 10;
    DDS_free(tdt);
    return RTC::RTC_OK;
}


static const char* spec[] =
{
    "implementation_id", "PubMultiComp",
    "type_name",         "pubmulticomp",
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

