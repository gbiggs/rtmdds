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

#include <ndds/ndds_cpp.h>
#include <string>

#include <iostream>

namespace RTC
{
    template<typename DataType, typename DataTypeWriter>
    class DDSPubPort
        : public DDSPubPortBase
    {
        public:
            DDSPubPort(std::string const name, std::string const type_name)
                : DDSPubPortBase(name), type_name_(type_name),
                participant_(0), topic_(0), writer_(0)
            {
                RTC_TRACE(("DDSPubPort()"));
            }

            ~DDSPubPort()
            {
                RTC_TRACE(("~DDSPubPort()"));
                if (participant_)
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
                DDSPubPortBase::init(props);
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
                if (!res)
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
                        qos.profile.url_profile.length(1);
                        qos.profile.url_profile[1] =
                            DDS_String_dup(props_["qos_file"].c_str());
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
                RTC_INFO(("DDSPubPort %s joining domain %d", getName(), domain));
                participant_ = fact->create_participant(domain,
                        DDS_PARTICIPANT_QOS_DEFAULT, 0, DDS_STATUS_MASK_NONE);
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
                if (!writer_)
                {
                    RTC_INFO(("Port is not connected; cannot write"));
                    return false;
                }

                DDS_ReturnCode_t res = type_writer_->write(value,
                        DDS_HANDLE_NIL);
                if (res != DDS_RETCODE_OK)
                {
                    RTC_ERROR(("Writer error %d", res));
                    return false;
                }
                RTC_DEBUG(("Successfully wrote data"));
                return true;
            }

            DDSPubPort<DataType, DataTypeWriter>& operator<<(DataType* value)
            {
                RTC_TRACE(("operator<<()"));
                write(value);
                return *this;
            }

        private:
            std::string topic_name_;
            std::string type_name_;

            DDSDomainParticipant* participant_;
            DDSTopic* topic_;
            DDSDataWriter* writer_;
            DataTypeWriter* type_writer_;

            ReturnCode_t publishInterfaces(ConnectorProfile& cp)
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
                // Create a writer
                writer_ = participant_->create_datawriter(topic_,
                        DDS_DATAWRITER_QOS_DEFAULT, 0,
                        DDS_STATUS_MASK_NONE);
                if (!writer_)
                {
                    RTC_ERROR(("Failed to create data writer"));
                    return RTC_ERROR;
                }
                type_writer_ = DataTypeWriter::narrow(writer_);
                if (!type_writer_)
                {
                    RTC_ERROR(("Failed to narrow writer"));
                    return RTC_ERROR;
                }
                return RTC_OK;
            }

            void unsubscribeInterfaces(ConnectorProfile const& cp)
            {
                RTC_TRACE(("unsubscribeInterfaces()"));
                if (participant_)
                {
                    if (writer_)
                    {
                        participant_->delete_datawriter(writer_);
                        writer_ = 0;
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

#endif // !defined(DDSPUBPORT_H_)

