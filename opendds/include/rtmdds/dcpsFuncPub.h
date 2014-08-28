#if !defined(DSPSFUNCPUB_H_)
#define DSPSFUNCPUB_H_

#include <vector>
#include <coil/Properties.h>

typedef std::vector<CORBA::Octet> octecSeq;

class DcpsFuncPub
{
	public:
		virtual ~DcpsFuncPub(void){}
		
		virtual void init(coil::Properties const& props,
							std::string const portName) = 0;
		
		virtual bool write(octecSeq& oct_seq) = 0;
	
		virtual bool publishInterfaces(std::string const topic_name,
								coil::Properties const& props) = 0;
		
		virtual void unsubscribeInterfaces() = 0;
	
	protected:
		DcpsFuncPub(void){}
		
	private:
		DcpsFuncPub(const DcpsFuncPub&);
		void operator=(const DcpsFuncPub&);
};

#endif // !defined(DSPSFUNCPUB_H_)
