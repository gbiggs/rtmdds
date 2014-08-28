#if !defined(DSPSFUNCSUBIMPL_H_)
#define DSPSFUNCSUBIMPL_H_

#include <../build/src/dds_datatypeTypeSupportImpl.h>

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/DomainParticipantFactoryImpl.h>

#include <rtmdds/dcpsFuncSub.h>
#include <rtm/SystemLogger.h>

class DcpsFuncSubImpl : public DcpsFuncSub
{
	public:
		// Constructor
		DcpsFuncSubImpl();
		// Destructor
		virtual ~DcpsFuncSubImpl();
		
		virtual void init(coil::Properties const& props,
							std::string const portName);
		
		virtual bool read(octecSeq* oct_seq);
		
		virtual bool isNew();
		
		virtual bool subscribeInterfaces(std::string const topic_name,
									coil::Properties const& props);
		
		virtual void unsubscribeInterfaces();
		
	private:
		std::string type_name_;
		
        DDS::DomainParticipantFactory_var fact_;
		static DDS::DomainParticipant_var participant_;
		DDS::Topic_var topic_;
		DDS::Subscriber_var subscriber_;
		DDS::DataReader_var reader_;
		dds_datatype::DdsDataTypeDataReader_var type_reader_;
		
		mutable RTC::Logger rtclog;
};

#endif // !defined(DSPSFUNCSUBIMPL_H_)
