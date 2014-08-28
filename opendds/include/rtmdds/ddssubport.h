/* rtmdss
 *
 * Header file for the DDS subscriber port.
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

#if !defined(DDSSUBPORT_H_)
#define DDSSUBPORT_H_

#include <rtmdds/ddssubportbase.h>
#include <rtmdds/dcpsFuncSubWrapper.h>

#include <rtm/idl/DataPortSkel.h>

#include <string>
#include <iostream>

namespace RTC
{
    template<typename DataType>
    class DDSSubPort
        : public DDSSubPortBase
    {
        public:
            DDSSubPort(std::string const name)
                : DDSSubPortBase(name), dcpsFunc_()
            {
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSSubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
            }

            void init(coil::Properties const& props)
            {
				RTC_TRACE(("init()"));
				DDSSubPortBase::init(props);
				dcpsFunc_.init(props, getName());
            }

            bool read(DataType* value)
            {
				std::vector<CORBA::Octet> data;
				if(!dcpsFunc_.read(&data))
				{
					return false;
				}
				// Convert sequence<octet> to CdrData
				int length = data.size();
				::OpenRTM::CdrData tmp(length);
				for (int i = 0; i < length; i++)
				{
					CORBA_SeqUtil::push_back(tmp, data[i]);
				}

				// Convert CdrData to cdrMemoryStream
				cdrMemoryStream cdr;
				bool littleEndian = true;
				cdr.setByteSwapFlag(littleEndian);
				cdr.put_octet_array(&(tmp[0]), tmp.length());
				
				// Convert cdrMemoryStream to DataType
				*value <<= cdr;
				return true;
            }
            
            bool isNew()
            {
				return dcpsFunc_.isNew();
			}

#if 0
            /// This version of read loans the memory space for the data from
            /// the middleware. return_loan() *must* be called once the data is
            /// no longer needed.
            bool read_with_loan(DataTypeSeq& values, DDS::SampleInfoSeq& info,
                    unsigned int max_samples=1)
            {
				return false;
            }

            void return_loan(DataTypeSeq& values, DDS_SampleInfoSeq& info)
            {
            }
#endif

            DDSSubPort<DataType>& operator>>(DataType* rhs)
            {
                RTC_TRACE(("operator>>()"));
                read(rhs);
                return *this;
            }

        protected:
			std::string topic_name_;
			DcpsFuncSubWrapper dcpsFunc_;
			
            virtual ReturnCode_t subscribeInterfaces(ConnectorProfile const& cp)
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
				if (!dcpsFunc_.subscribeInterfaces(topic_name_, props_))
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

#endif // !defined(DDSSUBPORT_H_)

