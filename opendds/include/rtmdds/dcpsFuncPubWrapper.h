#if !defined(DSPSFUNCPUBWRAPPER_H_)
#define DSPSFUNCPUBWRAPPER_H_

#include <rtmdds/dcpsFuncPub.h>

class DcpsFuncPubWrapper
{
	public:
		// Constructor
		DcpsFuncPubWrapper();
		// Destructor
		~DcpsFuncPubWrapper();
		
		void init(coil::Properties const& props, std::string const portName);
		
		bool write(octecSeq& oct_seq);
		
		bool publishInterfaces(std::string const topic_name,
								coil::Properties const& props);
		
		void unsubscribeInterfaces();
		
	private:
		DcpsFuncPub* obj_;
};

#endif // !defined(DSPSFUNCPUBWRAPPER_H_)
