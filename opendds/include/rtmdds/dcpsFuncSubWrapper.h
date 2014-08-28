#if !defined(DSPSFUNCSUBWRAPPER_H_)
#define DSPSFUNCSUBWRAPPER_H_

#include <rtmdds/dcpsFuncSub.h>

class DcpsFuncSubWrapper
{
	public:
		// Constructor
		DcpsFuncSubWrapper();
		// Destructor
		~DcpsFuncSubWrapper();
		
		void init(coil::Properties const& props, std::string const portName);
		
		bool read(octecSeq* oct_seq);
		
		bool isNew();
		
		bool subscribeInterfaces(std::string const topic_name,
								coil::Properties const& props);
		
		void unsubscribeInterfaces();
		
	private:
		DcpsFuncSub* obj_;
};

#endif // !defined(DSPSFUNCSUBWRAPPER_H_)
