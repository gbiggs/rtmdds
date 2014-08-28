#if !defined(DSPSFUNCSUB_H_)
#define DSPSFUNCSUB_H_

#include <vector>
#include <coil/Properties.h>

typedef std::vector<CORBA::Octet> octecSeq;

class DcpsFuncSub
{
	public:
		virtual ~DcpsFuncSub(void){}
		
		virtual void init(coil::Properties const& props,
							std::string const portName) = 0;
		
		virtual bool read(octecSeq* oct_seq) = 0;
		
		virtual bool isNew() = 0;
		
		virtual bool subscribeInterfaces(std::string const topic_name,
									coil::Properties const& props) = 0;
		
		virtual void unsubscribeInterfaces() = 0;
		
	protected:
		DcpsFuncSub(void){}
		
	private:
		DcpsFuncSub(const DcpsFuncSub&);
		void operator=(const DcpsFuncSub&);
};

#endif // !defined(DSPSFUNCSUB_H_)
