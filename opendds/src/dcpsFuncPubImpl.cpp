#include <coil/stringutil.h>
#include <rtmdds/dcpsFuncPubImpl.h>

// Init participant
DDS::DomainParticipant_var DcpsFuncPubImpl::participant_(0);

// Constructor
DcpsFuncPubImpl::DcpsFuncPubImpl()
 :	fact_(0), topic_(0),
	publisher_(0), writer_(0), rtclog("dcpsPub")
{
	dds_datatype::DdsDataTypeTypeSupport_var ddtTs
		= new dds_datatype::DdsDataTypeTypeSupportImpl();
	type_name_ = ddtTs->get_type_name();
}

// Destructor
DcpsFuncPubImpl::~DcpsFuncPubImpl()
{
	if (participant_)
	{
		participant_->delete_contained_entities();
		if (fact_)
		{
			fact_->delete_participant(participant_);
			TheServiceParticipant->shutdown();
		}
	}
}

//--------------------------------------------------
//**************** Public Functions ****************
//--------------------------------------------------
void DcpsFuncPubImpl::init(coil::Properties const& props,
							std::string const portName)
{
	// Get the host and port of DCPSInfoRepo service
	std::string address = props.getProperty("dcpsir_ip_address");
	std::string port_num = props.getProperty("dcpsir_port_number");
	if (address.empty())
	{
		RTC_ERROR(("Invalid dcpsir_ip_address value"));
		return;
	}
	if (port_num.empty())
	{
		RTC_ERROR(("Invalid dcpsir_port_number value"));
		return;
	}
	
	std::vector<std::string> args;
	args.push_back("-DCPSInfoRepo");
	args.push_back(address + ":" + port_num);
	// args.push_back("-ORBDebugLevel");
	// args.push_back("10");
	// args.push_back("-DCPSDebugLevel");
	// args.push_back("10");
	// args.push_back("-ORBLogFile");
	// args.push_back(portName + ".log");
	char** argv = coil::toArgv(args);
	int argc(args.size());
	
	// Create DomainParticipantFactory
	fact_ = TheParticipantFactoryWithArgs(argc, argv);
	
	DDS::DomainParticipantFactoryQos qos;
	DDS::ReturnCode_t res = fact_->get_qos(qos);
	if (res != DDS::RETCODE_OK)
	{
		RTC_ERROR(("Failed to get factory QoS (%d); cannot "
					"set QoS properties", res));
	}

	if (!participant_)
	{
		int domain(0);
		if (!coil::stringTo(domain, props.getProperty("domain",
						"0").c_str()))
		{
			RTC_ERROR(("Invalid domain value"));
			return;
		}
		RTC_INFO(("DDSPubPort %s joining domain %d", portName.c_str(), domain));
		participant_ = fact_->create_participant(
				domain,
				PARTICIPANT_QOS_DEFAULT,
				0,
				::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
		if (!participant_)
		{
			RTC_ERROR(("Failed to create domain participant"));
			return;
		}
	}
	
	// Register the Type
	dds_datatype::DdsDataTypeTypeSupport_var ddtTs
		= new dds_datatype::DdsDataTypeTypeSupportImpl();
	if (DDS::RETCODE_OK !=
			ddtTs->register_type(participant_, type_name_.c_str()))
	{
		RTC_ERROR(("Failed to register data type"));
		return;
	}
	RTC_PARANOID(("DDSPubPort %s initialised succesfully", portName.c_str()));
}

bool DcpsFuncPubImpl::write(octecSeq& oct_seq)
{
	if (!writer_)
	{
		RTC_INFO(("Port is not connected; cannot write"));
		return false;
	}
	
	// Convert sequence<octet> to DdsDataType
	dds_datatype::DdsDataType value;
	unsigned long length = oct_seq.size();
	value.data.length(length);
	for (unsigned int i = 0; i < length; i++)
	{
		value.data[i] = oct_seq[i];
	}
	// Write
	DDS::ReturnCode_t res = type_writer_->write(value,
			DDS::HANDLE_NIL);
	if (res != DDS::RETCODE_OK)
	{
		RTC_ERROR(("Writer error %d", res));
		return false;
	}
	return true;
}

bool DcpsFuncPubImpl::publishInterfaces(std::string const topic_name,
									coil::Properties const& props)
{
	RTC_TRACE(("publishInterfaces()"));
	// TODO: Fix this
	// Currently can only write to one topic at a time
	if (topic_)
	{
		RTC_ERROR(("DDSPubPort can only write to one topic at a "
				"time"));
		return false;
	}

	// Make sure the port was initialised correctly
	if (!participant_)
	{
		RTC_ERROR(("Port is not initialised"));
		return false;
	}

	// Get the QoS
	std::string partition_name = props.getProperty("partition_name", "rtmdds_data");
	std::string dur_kind_str = props.getProperty("qos_durability_kind", "VOLATILE");
	std::string rel_kind_str = props.getProperty("qos_reliability_kind", "BEST_EFFORT");
	std::string rel_time_str = props.getProperty("qos_reliability_max_blocking_time", "100");
	DDS::DurabilityQosPolicyKind dur_kind(DDS::VOLATILE_DURABILITY_QOS);
	DDS::ReliabilityQosPolicyKind rel_kind(DDS::BEST_EFFORT_RELIABILITY_QOS);
	long rel_time(100);
	if (partition_name.empty())
	{
		RTC_ERROR(("Invalid partition_name value"));
		return false;
	}
	if (dur_kind_str == "VOLATILE")
	{
		dur_kind = DDS::VOLATILE_DURABILITY_QOS;
	}
	else if (dur_kind_str == "TRANSIENT_LOCAL")
	{
		dur_kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
	}
	else if (dur_kind_str == "TRANSIENT")
	{
		dur_kind = DDS::TRANSIENT_DURABILITY_QOS;
	}
	else if (dur_kind_str == "PERSISTENT")
	{
		dur_kind = DDS::PERSISTENT_DURABILITY_QOS;
	}
	else
	{
		RTC_ERROR(("Invalid qos_durability_kind value"));
		return false;
	}
	if (rel_kind_str == "RELIABLE")
	{
		rel_kind = DDS::RELIABLE_RELIABILITY_QOS;
	}
	else if (rel_kind_str == "BEST_EFFORT")
	{
		rel_kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
	}
	else
	{
		RTC_ERROR(("Invalid qos_reliability_kind value"));
		return false;
	}
	if (!coil::stringTo(rel_time, rel_time_str.c_str()))
	{
		RTC_ERROR(("Invalid qos_reliability_max_blocking_time value"));
		return false;
	}
	DDS::Duration_t rel_time_st;
	rel_time_st.sec = rel_time / 1000;
	rel_time_st.nanosec = (rel_time % 1000) * 1000000; // mili -> nano
	// Join the topic
	DDS::TopicQos topic_qos;
	participant_->get_default_topic_qos(topic_qos);
	topic_qos.durability.kind = dur_kind;
	topic_qos.reliability.kind = rel_kind;
	topic_qos.reliability.max_blocking_time = rel_time_st;
	
	RTC_INFO(("Creating and joining topic %s", topic_name.c_str()));
	topic_ = participant_->create_topic(
					topic_name.c_str(),
					type_name_.c_str(),
					topic_qos,
					0,
					::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!topic_)
	{
		RTC_ERROR(("Failed to create topic %s", topic_name.c_str()));
		return false;
	}
	// Create a publisher
	DDS::PublisherQos pub_qos;
	participant_->get_default_publisher_qos(pub_qos);
	pub_qos.partition.name.length(1);
	pub_qos.partition.name[0] = partition_name.c_str();
	
	publisher_ = participant_->create_publisher(
					pub_qos,
					0,
					::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!publisher_)
	{
		RTC_ERROR(("Failed to create publisher"));
		return false;
	}
	// Create a writer
	DDS::DataWriterQos dw_qos;
	publisher_->get_default_datawriter_qos(dw_qos);
	dw_qos.durability.kind = dur_kind;
	dw_qos.reliability.kind = rel_kind;
	dw_qos.reliability.max_blocking_time = rel_time_st;
                
	writer_ = publisher_->create_datawriter(
					topic_,
					dw_qos,
					0,
					::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!writer_)
	{
		RTC_ERROR(("Failed to create data writer"));
		return false;
	}
	type_writer_ = dds_datatype::DdsDataTypeDataWriter::_narrow(writer_);
	if (!type_writer_)
	{
		RTC_ERROR(("Failed to narrow writer"));
		return false;
	}
	return true;
}

void DcpsFuncPubImpl::unsubscribeInterfaces()
{
	RTC_TRACE(("unsubscribeInterfaces()"));
	if (participant_)
	{
		if (writer_)
		{
			publisher_->delete_datawriter(writer_);
			writer_ = 0;
		}
		if (publisher_)
		{
			participant_->delete_publisher(publisher_);
			publisher_ = 0;
		}
		if (topic_)
		{
			participant_->delete_topic(topic_);
			topic_ = 0;
		}
	}
}
