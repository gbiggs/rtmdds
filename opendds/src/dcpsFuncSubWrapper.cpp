#include <rtmdds/dcpsFuncSubImpl.h>
#include <rtmdds/dcpsFuncSubWrapper.h>

// Constructor
DcpsFuncSubWrapper::DcpsFuncSubWrapper()
{
	obj_ = new DcpsFuncSubImpl();
}

// Destructor
DcpsFuncSubWrapper::~DcpsFuncSubWrapper()
{
	delete obj_;
}

void DcpsFuncSubWrapper::init(coil::Properties const& props,
								std::string const portName)
{
	return obj_->init(props, portName);
}

bool DcpsFuncSubWrapper::read(octecSeq* oct_seq)
{
	return obj_->read(oct_seq);
}

bool DcpsFuncSubWrapper::isNew()
{
	return obj_->isNew();
}

bool DcpsFuncSubWrapper::subscribeInterfaces(std::string const topic_name,
										coil::Properties const& props)
{
	return obj_->subscribeInterfaces(topic_name, props);
}

void DcpsFuncSubWrapper::unsubscribeInterfaces()
{
	obj_->unsubscribeInterfaces();
}
