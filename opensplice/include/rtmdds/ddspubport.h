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

#include <dds_dcps.h>
#include <string>

#include <typeinfo>
#include <iostream>

#include <typeinfo>

namespace RTC
{
    template<typename DataType, typename DataTypeWriter, typename DataTypeFunc>
    class DDSPubPort
        : public DDSPubPortBase
    {
        public:
            DDSPubPort(std::string const name, std::string const type_name)
                :	DDSPubPortBase(name), type_name_(type_name),
					partition_name_("rtmdds_data"),
					participant_(0), topic_(0), type_writer_(0)
            {
				publisher_ = DDS_OBJECT_NIL;
				topic_qos = DDS_OBJECT_NIL;
				pub_qos = DDS_OBJECT_NIL;
				dw_qos = DDS_OBJECT_NIL;
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSPubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
                if (participant_)
                {
                    DDS_DomainParticipantFactory fact(
                                DDS_DomainParticipantFactory_get_instance());
                    DDS_DomainParticipant_delete_contained_entities(participant_);
                    DDS_DomainParticipantFactory_delete_participant(fact, participant_);
                }
            }

            DDS_DomainParticipant get_participant() const
            {
                RTC_TRACE(("get_participant()"));
                return participant_;
            }

            void init(coil::Properties const& props)
            {
                RTC_TRACE(("init()"));
                DDSPubPortBase::init(props);
                DDS_DomainParticipantFactory fact(
                        DDS_DomainParticipantFactory_get_instance());

                DDS_DomainParticipantFactoryQos qos;
                DDS_ReturnCode_t res = DDS_DomainParticipantFactory_get_qos(fact, &qos);
                if (res != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Failed to get factory QoS (%d); cannot "
                                "set QoS properties", res));
                }

                int domain(DDS_DOMAIN_ID_DEFAULT);
                if (!coil::stringTo(domain, props_.getProperty("domain",
                                "0").c_str()))
                {
                    RTC_ERROR(("Invalid domain value"));
                    return;
                }
                RTC_INFO(("DDSPubPort %s joining domain %d", getName(), domain));
                participant_ = DDS_DomainParticipantFactory_create_participant(
						fact, domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
                if (!participant_)
                {
                    RTC_ERROR(("Failed to create domain participant"));
                    return;
                }
                RTC_PARANOID(("DDSPubPort %s initialised succesfully", getName()));
            }

            bool write(DataType& value)
            {
                RTC_TRACE(("write(DataType&)"));
                if (!type_writer_)
                {
                    RTC_INFO(("Port is not connected; cannot write"));
                    return false;
                }

				try
				{
					DDS_ReturnCode_t res = DataTypeFunc::write(type_writer_,
							&value,
							DDS_HANDLE_NIL);
					if (res != DDS_RETCODE_OK)
					{
						RTC_ERROR(("Writer error %d", res));
						return false;
					}
					RTC_DEBUG(("Successfully wrote data"));
					return true;
				}
				catch(std::bad_cast)
				{
					RTC_ERROR(("dynamic_cast error"));
					return false;
				}
            }

            DDSPubPort<DataType, DataTypeWriter, DataTypeFunc>& operator<<(DataType* value)
            {
                RTC_TRACE(("operator<<()"));
                write(*value);
                return *this;
            }

        protected:
            std::string topic_name_;
            std::string type_name_;
			std::string partition_name_;

            DDS_DomainParticipant participant_;
            DDS_Topic topic_;
            DataTypeWriter type_writer_;
			DDS_Publisher publisher_;
			DDS_TopicQos* topic_qos;
			DDS_PublisherQos* pub_qos;
			DDS_DataWriterQos* dw_qos;

            virtual ReturnCode_t publishInterfaces(ConnectorProfile& cp)
            {
                RTC_TRACE(("publishInterfaces()"));
                // TODO: Fix this
                // Currently can only write to one topic at a time
                if (topic_)
                {
                    RTC_ERROR(("DDSPubPort can only write to one topic at a "
                            "time"));
                    return RTC_ERROR;
                }

                // Make sure the port was initialised correctly
                if (!participant_)
                {
                    RTC_ERROR(("Port is not initialised"));
                    return RTC_ERROR;
                }

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
                // Get the QoS
				partition_name_ = props_.getProperty("partition_name", "rtmdds_data");
				std::string dur_kind_str = props_.getProperty("qos_durability_kind", "VOLATILE");
				std::string rel_kind_str = props_.getProperty("qos_reliability_kind", "BEST_EFFORT");
				std::string rel_time_str = props_.getProperty("qos_reliability_max_blocking_time", "100");
				DDS_DurabilityQosPolicyKind dur_kind(DDS_VOLATILE_DURABILITY_QOS);
				DDS_ReliabilityQosPolicyKind rel_kind(DDS_BEST_EFFORT_RELIABILITY_QOS);
				long rel_time(100);
				if (partition_name_.empty())
				{
					RTC_ERROR(("Invalid partition_name value"));
					return RTC_ERROR;
				}
				if (dur_kind_str == "VOLATILE")
				{
					dur_kind = DDS_VOLATILE_DURABILITY_QOS;
				}
				else if (dur_kind_str == "TRANSIENT_LOCAL")
				{
					dur_kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
				}
				else if (dur_kind_str == "TRANSIENT")
				{
					dur_kind = DDS_TRANSIENT_DURABILITY_QOS;
				}
				else if (dur_kind_str == "PERSISTENT")
				{
					dur_kind = DDS_PERSISTENT_DURABILITY_QOS;
				}
				else
				{
					RTC_ERROR(("Invalid qos_durability_kind value"));
					return RTC_ERROR;
				}
				if (rel_kind_str == "RELIABLE")
				{
					rel_kind = DDS_RELIABLE_RELIABILITY_QOS;
				}
				else if (rel_kind_str == "BEST_EFFORT")
				{
					rel_kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
				}
				else
				{
					RTC_ERROR(("Invalid qos_reliability_kind value"));
					return RTC_ERROR;
				}
				if (!coil::stringTo(rel_time, rel_time_str.c_str()))
				{
					RTC_ERROR(("Invalid qos_reliability_max_blocking_time value"));
					return RTC_ERROR;
				}
				DDS_Duration_t_s rel_time_st;
				rel_time_st.sec = rel_time / 1000;;
				rel_time_st.nanosec = (rel_time % 1000) * 1000000; // mili -> nano
                // Join the topic
                topic_qos = DDS_TopicQos__alloc();
                if (!topic_qos)
				{
                    RTC_ERROR(("Failed to create topic qos"));
				}
				DDS_DomainParticipant_get_default_topic_qos(participant_, topic_qos);
				topic_qos->durability.kind = dur_kind;
				topic_qos->reliability.kind = rel_kind;
				topic_qos->reliability.max_blocking_time = rel_time_st;
				
                RTC_INFO(("Creating and joining topic %s", topic_name_.c_str()));
                topic_ = DDS_DomainParticipant_create_topic(
                        participant_,
                        topic_name_.c_str(),
                        type_name_.c_str(), topic_qos, 0,
                        DDS_STATUS_MASK_NONE);
                if (!topic_)
                {
                    RTC_ERROR(("Failed to create topic %s", topic_name_.c_str()));
                    return RTC_ERROR;
                }
                // Create a publisher
				pub_qos = DDS_PublisherQos__alloc();
				if (!pub_qos) {
                    RTC_ERROR(("Failed to create publisher qos"));
                    return RTC_ERROR;
				}
				DDS_DomainParticipant_get_default_publisher_qos(participant_, pub_qos);
				pub_qos->partition.name._length = 1;
				pub_qos->partition.name._maximum = 1;
				pub_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
				pub_qos->partition.name._buffer[0] = DDS_string_alloc(partition_name_.length());
				strcpy(pub_qos->partition.name._buffer[0], partition_name_.c_str());

				publisher_ = DDS_DomainParticipant_create_publisher(
					participant_, pub_qos, NULL, DDS_STATUS_MASK_NONE);
				if (!publisher_) {
                    RTC_ERROR(("Failed to create publisher"));
                    return RTC_ERROR;
				}
                // Create a writer
                dw_qos = DDS_DataWriterQos__alloc();
				if (!dw_qos) {
                    RTC_ERROR(("Failed to create data writer qos"));
                    return RTC_ERROR;
				}
				DDS_Publisher_get_default_datawriter_qos(publisher_, dw_qos);
				dw_qos->durability.kind = dur_kind;
				dw_qos->reliability.kind = rel_kind;
				dw_qos->reliability.max_blocking_time = rel_time_st;
                
                type_writer_ = DDS_Publisher_create_datawriter(
                        publisher_,
                        topic_,
                        dw_qos, NULL,
                        DDS_STATUS_MASK_NONE);
                if (!type_writer_)
                {
                    RTC_ERROR(("Failed to create data writer"));
                    return RTC_ERROR;
                }
                return RTC_OK;
            }

            virtual void unsubscribeInterfaces(ConnectorProfile const& cp)
            {
                RTC_TRACE(("unsubscribeInterfaces()"));
                if (participant_)
                {
                    if (type_writer_)
                    {
                        DDS_Publisher_delete_datawriter(participant_, type_writer_);
                        type_writer_ = 0;
                    }
                    if (topic_)
                    {
                        DDS_DomainParticipant_delete_topic(participant_, topic_);
                        topic_ = 0;
                    }
                }
            }
    };
}; // namespace RTC

#endif // !defined(DDSPUBPORT_H_)

