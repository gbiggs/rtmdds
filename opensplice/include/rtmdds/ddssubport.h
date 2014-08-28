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

#include <os_defs.h>
#include <dds_dcps.h>
#include <string>

#include <iostream>

namespace RTC
{
    template<typename DataType, typename DataTypeSeq,
			typename DataTypeReader, typename DataTypeFunc>
    class DDSSubPort
        : public DDSSubPortBase
    {
        public:
            DDSSubPort(std::string const name, std::string const type_name)
                :	DDSSubPortBase(name), type_name_(type_name),
					partition_name_("rtmdds_data"),
					participant_(0), topic_(0), type_reader_(0)
            {
				subscriber_ = DDS_OBJECT_NIL;
				topic_qos = DDS_OBJECT_NIL;
				sub_qos = DDS_OBJECT_NIL;
				dr_qos = DDS_OBJECT_NIL;
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSSubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
                if (!participant_)
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
                DDSSubPortBase::init(props);
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
                RTC_INFO(("DDSSubPort %s joining domain %d", getName(), domain));
                participant_ = DDS_DomainParticipantFactory_create_participant(
						fact, domain, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
                if (!participant_)
                {
                    RTC_ERROR(("Failed to create domain participant"));
                    return;
                }
                RTC_PARANOID(("DDSSubPort %s initialised succesfully", getName()));
            }

            bool read(DataType* value)
            {
				bool retCode = false;
				
                RTC_TRACE(("read(DataType*)"));
                if (!type_reader_)
                {
                    RTC_INFO(("Port is not connected; cannot read"));
                    return false;
                }

                DataTypeSeq *msgSeq = DataTypeFunc::sequence__alloc();
                if (!msgSeq)
                {
                    RTC_ERROR(("Error in DDS_sequence_SPACE_NamedMessage__alloc: Creation failed: invalid handle"));
                    return false;
				}
                DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
                if (!infoSeq)
                {
                    RTC_ERROR(("Error in DDS_SampleInfoSeq__alloc: Creation failed: invalid handle"));
                    return false;
				}
                // Attempt to take
				// Use 'take' function and set the sample state to ANY_SAMPLE_STATE.
				// (Don't use 'take_next_sample')
				// Because the sample state becomes OLD by 'isNew' function.
                DDS_ReturnCode_t ret = DataTypeFunc::take(
                                            type_reader_,
                                            msgSeq,
                                            infoSeq,
                                            1,
//                                            DDS_NOT_READ_SAMPLE_STATE,
                                            DDS_ANY_SAMPLE_STATE,
                                            DDS_ANY_VIEW_STATE,
                                            DDS_ANY_INSTANCE_STATE);
                if (ret == DDS_RETCODE_NO_DATA)
                {
                    RTC_INFO(("No data available"));
                }
                else if (ret != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Read error %d", ret));
                }
                else if (!infoSeq->_buffer->valid_data)
                {
                    RTC_INFO(("Out-of-band data read"));
                }
                else
                {
					*value = msgSeq->_buffer[0];
					RTC_DEBUG(("Successfully read data"));
					retCode = true;
				}
				DDS_free(msgSeq);
				DDS_free(infoSeq);
                return retCode;
            }

            bool isNew()
            {
				bool retCode = false;
				
                RTC_TRACE(("isNew()"));
                if (!type_reader_)
                {
                    RTC_INFO(("Port is not connected; cannot read"));
                    return false;
                }

                DataTypeSeq *msgSeq = DataTypeFunc::sequence__alloc();
                if (!msgSeq)
                {
                    RTC_ERROR(("Error in DDS_sequence_SPACE_NamedMessage__alloc: Creation failed: invalid handle"));
                    return false;
				}
                DDS_SampleInfoSeq *infoSeq = DDS_SampleInfoSeq__alloc();
                if (!infoSeq)
                {
                    RTC_ERROR(("Error in DDS_SampleInfoSeq__alloc: Creation failed: invalid handle"));
                    return false;
				}
                // Attempt to read
                DDS_ReturnCode_t ret = DataTypeFunc::read(
                                            type_reader_,
                                            msgSeq,
                                            infoSeq,
                                            1,
                                            DDS_NOT_READ_SAMPLE_STATE,
                                            DDS_ANY_VIEW_STATE,
                                            DDS_ANY_INSTANCE_STATE);
                if (ret == DDS_RETCODE_NO_DATA)
                {
                    RTC_INFO(("No data available"));
                }
                else if (ret != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Read error %d", ret));
                }
                else if (!infoSeq->_buffer->valid_data)
                {
                    RTC_INFO(("Out-of-band data read"));
                }
                else
                {
					RTC_DEBUG(("Successfully read data"));
					retCode = true;
				}
				DDS_free(msgSeq);
				DDS_free(infoSeq);
                return retCode;
            }

            /// This version of read loans the memory space for the data from
            /// the middleware. return_loan() *must* be called once the data is
            /// no longer needed.
            bool read_with_loan(DataTypeSeq& values, DDS_SampleInfoSeq& info,
                    unsigned int max_samples=1)
            {
                RTC_TRACE(("read_with_loan()"));
                if (!type_reader_)
                {
                    RTC_INFO(("Port is not connected; cannot read"));
                    return false;
                }

                DDS_ReturnCode_t ret = DataTypeFunc::take(
						type_reader_,
						&values,
						&info,
                        max_samples,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                if (ret == DDS_RETCODE_NO_DATA)
                {
                    RTC_INFO(("No data available"));
                    return false;
                }
                else if (ret != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Read error %d", ret));
                    return false;
                }
                if (!info._buffer->valid_data)
                {
                    RTC_INFO(("Out-of-band data read"));
                    return false;
                }

                RTC_DEBUG(("Successfully read data"));
                return true;
            }

            void return_loan(DataTypeSeq& values, DDS_SampleInfoSeq& info)
            {
                DataTypeFunc::return_loan(type_reader_, &values, &info);
            }

            DDSSubPort<DataType, DataTypeSeq, DataTypeReader, DataTypeFunc>& operator>>(DataType* rhs)
            {
                RTC_TRACE(("operator>>()"));
                read(rhs);
                return *this;
            }

        protected:
            std::string topic_name_;
            std::string type_name_;
			std::string partition_name_;

            DDS_DomainParticipant participant_;
            DDS_Topic topic_;
            DataTypeReader type_reader_;
			DDS_Subscriber subscriber_;
			DDS_TopicQos* topic_qos;
			DDS_SubscriberQos* sub_qos;
			DDS_DataReaderQos* dr_qos;

            virtual ReturnCode_t subscribeInterfaces(ConnectorProfile const& cp)
            {
                RTC_TRACE(("publishInterfaces()"));
                // TODO: Fix this
                // Currently can only read from one topic at a time
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
                // Create a subscriber
				sub_qos = DDS_SubscriberQos__alloc();
				if (!sub_qos) {
                    RTC_ERROR(("Failed to create subscriber qos"));
                    return RTC_ERROR;
				}
				DDS_DomainParticipant_get_default_subscriber_qos(participant_, sub_qos);
				sub_qos->partition.name._length = 1;
				sub_qos->partition.name._maximum = 1;
				sub_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
				sub_qos->partition.name._buffer[0] = DDS_string_alloc(partition_name_.length());
				strcpy(sub_qos->partition.name._buffer[0], partition_name_.c_str());

				subscriber_ = DDS_DomainParticipant_create_subscriber(
					participant_, sub_qos, NULL, DDS_STATUS_MASK_NONE);
				if (!subscriber_) {
                    RTC_ERROR(("Failed to create subscriber"));
                    return RTC_ERROR;
				}
                // Create a reader
                dr_qos = DDS_DataReaderQos__alloc();
				if (!dr_qos) {
                    RTC_ERROR(("Failed to create data reader qos"));
                    return RTC_ERROR;
				}
				DDS_Subscriber_get_default_datareader_qos(subscriber_, dr_qos);
				dr_qos->durability.kind = dur_kind;
				dr_qos->reliability.kind = rel_kind;
				dr_qos->reliability.max_blocking_time = rel_time_st;
                
                type_reader_ = DDS_Subscriber_create_datareader(
                        subscriber_,
                        topic_,
                        dr_qos, NULL,
                        DDS_STATUS_MASK_NONE);
                if (!type_reader_)
                {
                    RTC_ERROR(("Failed to create data reader"));
                    return RTC_ERROR;
                }
                return RTC_OK;
            }

            virtual void unsubscribeInterfaces(ConnectorProfile const& cp)
            {
                RTC_TRACE(("unsubscribeInterfaces()"));
                if (participant_)
                {
                    if (type_reader_)
                    {
                        DDS_Subscriber_delete_datareader(participant_, type_reader_);
                        type_reader_ = 0;
                    }
					if (subscriber_) {
						DDS_DomainParticipant_delete_subscriber(participant_, subscriber_);
						subscriber_ = 0;
					}
					if (sub_qos) {
						DDS_free(sub_qos);
						sub_qos = 0;
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

#endif // !defined(DDSSUBPORT_H_)

