#include <coil/stringutil.h>
#include <rtmdds/dcpsFuncSubImpl.h>

// Init participant
DDS::DomainParticipant_var DcpsFuncSubImpl::participant_(0);

// Constructor
DcpsFuncSubImpl::DcpsFuncSubImpl()
 :	fact_(0), topic_(0),
	subscriber_(0), reader_(0), rtclog("dcpsSub")
{
	dds_datatype::DdsDataTypeTypeSupport_var ddtTs
		= new dds_datatype::DdsDataTypeTypeSupportImpl();
	type_name_ = ddtTs->get_type_name();
}

// Destructor
DcpsFuncSubImpl::~DcpsFuncSubImpl()
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
void DcpsFuncSubImpl::init(coil::Properties const& props,
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
		RTC_INFO(("DDSSubPort %s joining domain %d", portName.c_str(), domain));
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
	RTC_PARANOID(("DDSSubPort %s initialised succesfully", portName.c_str()));
}

bool DcpsFuncSubImpl::read(octecSeq* oct_seq)
{
	RTC_TRACE(("read(DataType*)"));
	if (!reader_)
	{
		RTC_INFO(("Port is not connected; cannot read"));
		return false;
	}

	// Attempt to take
	// Use 'take' function and set the sample state to ANY_SAMPLE_STATE.
	// (Don't use 'take_next_sample')
	// Because the sample state becomes OLD by 'isNew' function.
	DDS::SampleInfoSeq info(1);
	dds_datatype::DdsDataTypeSeq value(1);
	DDS::ReturnCode_t ret = type_reader_->take(
								value,
								info,
								1,
								DDS::ANY_SAMPLE_STATE,
								DDS::ANY_VIEW_STATE,
								DDS::ANY_INSTANCE_STATE);
	if (ret == DDS::RETCODE_NO_DATA)
	{
		RTC_INFO(("No data available"));
		return false;
	}
	else if (ret != DDS::RETCODE_OK)
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
	
	// Convert DdsDataType to sequence<octet>
	unsigned long length = value[0].data.length();
	for (unsigned int i = 0; i < length; i++)
	{
		oct_seq->push_back(value[0].data[i]);
	}
	return true;
}

bool DcpsFuncSubImpl::isNew()
{
	RTC_TRACE(("isNew()"));
	if (!reader_)
	{
		RTC_INFO(("Port is not connected; cannot read"));
		return false;
	}

	DDS::SampleInfoSeq info(1);
	// Attempt to read
	dds_datatype::DdsDataTypeSeq value(1);
	DDS::ReturnCode_t ret = type_reader_->read(
								value,
								info,
								1,
								DDS::NOT_READ_SAMPLE_STATE,
								DDS::ANY_VIEW_STATE,
								DDS::ANY_INSTANCE_STATE);
	if (ret == DDS::RETCODE_NO_DATA)
	{
		RTC_INFO(("No data available"));
		return false;
	}
	else if (ret != DDS::RETCODE_OK)
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
//	type_reader_->return_loan(value, info);
	return true;
}

bool DcpsFuncSubImpl::subscribeInterfaces(std::string const topic_name,
									coil::Properties const& props)
{
	RTC_TRACE(("subscribeInterfaces()"));
	// TODO: Fix this
	// Currently can only read from one topic at a time
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
	// Create a subscriber
	DDS::SubscriberQos sub_qos;
	participant_->get_default_subscriber_qos(sub_qos);
	sub_qos.partition.name.length(1);
	sub_qos.partition.name[0] = partition_name.c_str();
	
	subscriber_ = participant_->create_subscriber(
					sub_qos,
					0,
					::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!subscriber_)
	{
		RTC_ERROR(("Failed to create subscriber"));
		return false;
	}
	// Create a reader
	DDS::DataReaderQos dr_qos;
	subscriber_->get_default_datareader_qos(dr_qos);
	dr_qos.durability.kind = dur_kind;
	dr_qos.reliability.kind = rel_kind;
	dr_qos.reliability.max_blocking_time = rel_time_st;
	
	reader_ = subscriber_->create_datareader(
					topic_,
					dr_qos,
					0,
					::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
	if (!reader_)
	{
		RTC_ERROR(("Failed to create data reader"));
		return false;
	}
	type_reader_ = dds_datatype::DdsDataTypeDataReader::_narrow(reader_);
	if (!type_reader_)
	{
		RTC_ERROR(("Failed to narrow data reader"));
		return false;
	}
	return true;
}

void DcpsFuncSubImpl::unsubscribeInterfaces()
{
	RTC_TRACE(("unsubscribeInterfaces()"));
	if (participant_)
	{
		if (reader_)
		{
			subscriber_->delete_datareader(reader_);
			reader_ = 0;
		}
		if (subscriber_)
		{
			participant_->delete_subscriber(subscriber_);
			subscriber_ = 0;
		}
		if (topic_)
		{
			participant_->delete_topic(topic_);
			topic_ = 0;
		}
	}
}
