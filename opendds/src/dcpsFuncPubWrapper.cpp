#include <rtmdds/dcpsFuncPubImpl.h>
#include <rtmdds/dcpsFuncPubWrapper.h>

// Constructor
DcpsFuncPubWrapper::DcpsFuncPubWrapper()
{
	obj_ = new DcpsFuncPubImpl();
}

// Destructor
DcpsFuncPubWrapper::~DcpsFuncPubWrapper()
{
	delete obj_;
}

void DcpsFuncPubWrapper::init(coil::Properties const& props,
								std::string const portName)
{
	return obj_->init(props, portName);
}

bool DcpsFuncPubWrapper::write(octecSeq& oct_seq)
{
	return obj_->write(oct_seq);
}

bool DcpsFuncPubWrapper::publishInterfaces(std::string const topic_name,
										coil::Properties const& props)
{
	return obj_->publishInterfaces(topic_name, props);
}

void DcpsFuncPubWrapper::unsubscribeInterfaces()
{
	obj_->unsubscribeInterfaces();
}
