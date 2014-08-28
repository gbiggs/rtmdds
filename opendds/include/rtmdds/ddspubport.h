/* rtmdss
 *
 * Header file for the DDS publisher port.
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

#if !defined(DDSPUBPORT_H_)
#define DDSPUBPORT_H_

#include <rtmdds/ddspubportbase.h>
#include <rtmdds/dcpsFuncPubWrapper.h>

#include <rtm/idl/DataPortSkel.h>

#include <string>
#include <iostream>

namespace RTC
{
    template<typename DataType>
    class DDSPubPort
        : public DDSPubPortBase
    {
        public:
            DDSPubPort(std::string const name)
                : DDSPubPortBase(name), dcpsFunc_()
            {
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSPubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
            }

            void init(coil::Properties const& props)
            {
				RTC_TRACE(("init()"));
				DDSPubPortBase::init(props);
				dcpsFunc_.init(props, getName());
            }

            bool write(DataType& value)
            {
				// Convert DataType to cdrMemoryStream
				cdrMemoryStream cdr;
				bool littleEndian = true;
				cdr.rewindPtrs();
				cdr.setByteSwapFlag(littleEndian);
				value >>= cdr;
				
				// Convert cdrMemoryStream to CdrData
#ifndef ORB_IS_RTORB
				::OpenRTM::CdrData tmp(cdr.bufSize(), cdr.bufSize(),
									   static_cast<CORBA::Octet*>(cdr.bufPtr()), 0);
#else // ORB_IS_RTORB
				OpenRTM_CdrData *cdrdata_tmp = new OpenRTM_CdrData();
				cdrdata_tmp->_buffer = 
				  (CORBA_octet *)RtORB_alloc(cdr.bufSize(), "DDSPubPort::write");
				memcpy(cdrdata_tmp->_buffer, cdr.bufPtr(), cdr.bufSize());
				cdrdata_tmp->_length = cdrdata_tmp->_maximum= cdr.bufSize();
				::OpenRTM::CdrData tmp(cdrdata_tmp);
#endif // ORB_IS_RTORB

				// Convert CdrData to sequence<octet>
				std::vector<CORBA::Octet> data;
				int length = tmp.length();
				for (int i = 0; i < length; i++)
				{
					data.push_back(tmp[i]);
				}
				return dcpsFunc_.write(data);
            }

            DDSPubPort<DataType>& operator<<(DataType* value)
            {
                RTC_TRACE(("operator<<()"));
                write(value);
                return *this;
            }

        protected:
			std::string topic_name_;
			DcpsFuncPubWrapper dcpsFunc_;
			
            virtual ReturnCode_t publishInterfaces(ConnectorProfile& cp)
            {
				// Check the connector profile for a topic name
				CORBA::Long ii = NVUtil::find_index(cp.properties,
						"ddsport.topic");
				// If no topic name was found, look for a default one in the
				// port's properties, and use the port's name as the default if
				// that isn't found
				if (ii < 0)
				{
					RTC_INFO(("ddsport.topic key not found"));
					std::string port_name = getName();
					std::string::size_type ii = port_name.find_last_of(".");
					if (ii != std::string::npos)
					{
						port_name = port_name.substr(ii + 1);
					}
					else
					{
						RTC_ERROR(("Error getting port name"));
						return RTC_ERROR;
					}
					topic_name_ = props_.getProperty("default_topic",
							port_name);
					// Put this topic name into the connector profile for any
					// other ports to use
					NVUtil::appendStringValue(cp.properties, "ddsport.topic",
							topic_name_.c_str());
				}
				else
				{
					char const* topic_name(0);
					if (!(cp.properties[ii].value >>= topic_name))
					{
						RTC_ERROR(("ddsport.topic is empty"));
						return RTC_ERROR;
					}
					topic_name_ = topic_name;
				}
				if (!dcpsFunc_.publishInterfaces(topic_name_, props_))
				{
					return RTC_ERROR;
				}
				return RTC_OK;
            }

            virtual void unsubscribeInterfaces(ConnectorProfile const& cp)
            {
				dcpsFunc_.unsubscribeInterfaces();
            }
    };
}; // namespace RTC

#endif // !defined(DDSPUBPORT_H_)

