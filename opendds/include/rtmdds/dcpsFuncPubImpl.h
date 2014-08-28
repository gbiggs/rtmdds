#if !defined(DSPSFUNCPUBIMPL_H_)
#define DSPSFUNCPUBIMPL_H_

#include <../build/src/dds_datatypeTypeSupportImpl.h>

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/DomainParticipantFactoryImpl.h>

#include <rtmdds/dcpsFuncPub.h>
#include <rtm/SystemLogger.h>

class DcpsFuncPubImpl : public DcpsFuncPub
{
	public:
		// Constructor
		DcpsFuncPubImpl();
		// Destructor
		virtual ~DcpsFuncPubImpl();
		
		virtual void init(coil::Properties const& props,
							std::string const portName);
		
		virtual bool write(octecSeq& oct_seq);
	
		virtual bool publishInterfaces(std::string const topic_name,
									coil::Properties const& props);
		
		virtual void unsubscribeInterfaces();
		
	private:
		std::string type_name_;
		
        DDS::DomainParticipantFactory_var fact_;
		static DDS::DomainParticipant_var participant_;
		DDS::Topic_var topic_;
		DDS::Publisher_var publisher_;
		DDS::DataWriter_var writer_;
		dds_datatype::DdsDataTypeDataWriter_var type_writer_;
		
		mutable RTC::Logger rtclog;
};

#endif // !defined(DSPSFUNCPUBIMPL_H_)
