/* rtmdss
 *
 * Component header file.
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


#if !defined(RTC_H_)
#define RTC_H_

#include "test.h"
#include "testSupport.h"

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtmdds/ddssubport.h>


class SubscriberRTC
    : public RTC::DataFlowComponentBase
{
    public:
        SubscriberRTC(RTC::Manager* manager);
        ~SubscriberRTC();

        virtual RTC::ReturnCode_t onInitialize();
        virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

    private:
        TestDataType value_;
        RTC::DDSSubPort<TestDataType, TestDataTypeDataReader> port_;
};


extern "C"
{
    DLL_EXPORT void rtc_init(RTC::Manager* manager);
};

#endif // !defined(RTC_H_)

