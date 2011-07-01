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

#include <ndds/ndds_cpp.h>
#include <string>

#include <iostream>

namespace RTC
{
    template<typename DataType, typename DataTypeSeq, typename DataTypeReader>
    class DDSSubPort
        : public DDSSubPortBase
    {
        public:
            DDSSubPort(std::string const name, std::string const type_name)
                : DDSSubPortBase(name), type_name_(type_name),
                participant_(0), topic_(0), reader_(0)
            {
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSSubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
                if (!participant_)
                {
                    participant_->delete_contained_entities();
                    DDSDomainParticipantFactory::get_instance()->
                        delete_participant(participant_);
                }
            }

            DDSDomainParticipant* get_participant() const
            {
                RTC_TRACE(("get_participant()"));
                return participant_;
            }

            void init(coil::Properties const& props)
            {
                RTC_TRACE(("init()"));
                DDSSubPortBase::init(props);
                DDSDomainParticipantFactory* fact(
                        DDSDomainParticipantFactory::get_instance());

                // Enable DDS verbose mode if the verbose property is true
                if (coil::toBool(props_.getProperty("verbose", "NO"), "YES",
                            "NO", false))
                {
                    NDDSConfigLogger::get_instance()->set_verbosity(
                            NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                    NDDSConfigLogger::get_instance()->set_print_format(
                            NDDS_CONFIG_LOG_PRINT_FORMAT_MAXIMAL);
                }

                DDS_DomainParticipantFactoryQos qos;
                DDS_ReturnCode_t res = fact->get_qos(qos);
                if (res != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Failed to get factory QoS (%d); cannot "
                                "set QoS properties", res));
                }
                else
                {
                    // Set QoS XML file locations
                    if (props_.hasKey("qos_file"))
                    {
                        // Format for this parameter is the same as the URL
                        // Groups in RTI DDS:
                        // [URL1 | URL2 | URL3 | ... | URLn]
                        // Only one URL will be loaded; the remainder are
                        // considered alternates for fault tolerance.
                        char const* prof[1];
                        prof[0] = props_["qos_file"].c_str();
                        qos.profile.url_profile.from_array(prof, 1);
                    }
                    if (coil::toBool(props_.getProperty("ignore_user_profile",
                                    "NO"), "YES", "NO", false))
                    {
                        qos.profile.ignore_user_profile = DDS_BOOLEAN_TRUE;
                    }
                    else
                    {
                        qos.profile.ignore_user_profile = DDS_BOOLEAN_FALSE;
                    }
                    if (coil::toBool(props_.getProperty("ignore_env_profile",
                                    "NO"), "YES", "NO", false))
                    {
                        qos.profile.ignore_environment_profile = DDS_BOOLEAN_TRUE;
                    }
                    else
                    {
                        qos.profile.ignore_environment_profile = DDS_BOOLEAN_FALSE;
                    }
                    if (coil::toBool(props_.getProperty("ignore_resource_profile",
                                    "NO"), "YES", "NO", false))
                    {
                        qos.profile.ignore_resource_profile = DDS_BOOLEAN_TRUE;
                    }
                    else
                    {
                        qos.profile.ignore_resource_profile = DDS_BOOLEAN_FALSE;
                    }
                    fact->set_qos(qos);
                }

                int domain(0);
                if (!coil::stringTo(domain, props_.getProperty("domain",
                                "0").c_str()))
                {
                    RTC_ERROR(("Invalid domain value"));
                    return;
                }
                RTC_INFO(("DDSSubPort %s joining domain %d", getName(), domain));
                participant_ = fact->create_participant(domain,
                        DDS_PARTICIPANT_QOS_DEFAULT, 0, DDS_STATUS_MASK_NONE);
                if (!participant_)
                {
                    RTC_ERROR(("Failed to create domain participant"));
                    return;
                }
                RTC_PARANOID(("DDSSubPort %s initialised succesfully", getName()));
            }

            bool read(DataType* value)
            {
                RTC_TRACE(("read(DataType*)"));
                if (!reader_)
                {
                    RTC_INFO(("Port is not connected; cannot read"));
                    return false;
                }

                DDS_SampleInfo info;
                // Attempt to take the next sample
                DDS_ReturnCode_t ret = type_reader_->take_next_sample(*value, info);
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
                if (!info.valid_data)
                {
                    RTC_INFO(("Out-of-band data read"));
                    return false;
                }

                RTC_DEBUG(("Successfully read data"));
                return true;
            }

            /// This version of read loans the memory space for the data from
            /// the middleware. return_loan() *must* be called once the data is
            /// no longer needed.
            bool read_with_loan(DataTypeSeq& values, DDS_SampleInfoSeq& info,
                    unsigned int max_samples=1)
            {
                RTC_TRACE(("read_with_loan()"));
                if (!reader_)
                {
                    RTC_INFO(("Port is not connected; cannot read"));
                    return false;
                }

                DDS_ReturnCode_t ret = type_reader_->take(values, info,
                        max_samples, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE,
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
                if (!info[0].valid_data)
                {
                    RTC_INFO(("Out-of-band data read"));
                    return false;
                }

                RTC_DEBUG(("Successfully read data"));
                return true;
            }

            void return_loan(DataTypeSeq& values, DDS_SampleInfoSeq& info)
            {
                type_reader_->return_loan(values, info);
            }

            DDSSubPort<DataType, DataTypeSeq, DataTypeReader>& operator>>(DataType* rhs)
            {
                RTC_TRACE(("operator>>()"));
                read(rhs);
                return *this;
            }

        private:
            std::string topic_name_;
            std::string type_name_;

            DDSDomainParticipant* participant_;
            DDSTopic* topic_;
            DDSDataReader* reader_;
            DataTypeReader* type_reader_;

            ReturnCode_t subscribeInterfaces(ConnectorProfile const& cp)
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
                // Join the topic
                RTC_INFO(("Creating and joining topic %s", topic_name_.c_str()));
                topic_ = participant_->create_topic(topic_name_.c_str(),
                        type_name_.c_str(), DDS_TOPIC_QOS_DEFAULT, 0,
                        DDS_STATUS_MASK_NONE);
                if (!topic_)
                {
                    RTC_ERROR(("Failed to create topic %s", topic_name_.c_str()));
                    return RTC_ERROR;
                }
                // Create a reader
                reader_ = participant_->create_datareader(topic_,
                        DDS_DATAREADER_QOS_DEFAULT, 0,
                        DDS_STATUS_MASK_NONE);
                if (!reader_)
                {
                    RTC_ERROR(("Failed to create data reader"));
                    return RTC_ERROR;
                }
                type_reader_ = DataTypeReader::narrow(reader_);
                if (!type_reader_)
                {
                    RTC_ERROR(("Failed to narrow data reader"));
                    return RTC_ERROR;
                }
                return RTC_OK;
            }

            void unsubscribeInterfaces(ConnectorProfile const& cp)
            {
                RTC_TRACE(("unsubscribeInterfaces()"));
                if (participant_)
                {
                    if (reader_)
                    {
                        participant_->delete_datareader(reader_);
                        reader_ = 0;
                    }
                    if (topic_)
                    {
                        participant_->delete_topic(topic_);
                        topic_ = 0;
                    }
                }
            }
    };
}; // namespace RTC

#endif // !defined(DDSSUBPORT_H_)

