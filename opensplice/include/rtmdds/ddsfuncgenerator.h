// Macro for create DataTypeFunc class

#if !defined(DDSFUNCGENERATOR_H_)

#define CREATE_DATATYPEFUNC_HEADER(dataType)							\
class dataType ## Func													\
{																		\
	public:																\
		static DDS_ReturnCode_t (*write)(								\
					dataType ## DataWriter,								\
					const dataType*,									\
					const DDS_InstanceHandle_t);						\
																		\
		static DDS_sequence_ ## dataType* (*sequence__alloc)();		\
																		\
		static DDS_ReturnCode_t (*take)(								\
					dataType ## DataReader,								\
					DDS_sequence_ ## dataType*,							\
					DDS_SampleInfoSeq*,									\
					const DDS_long,									\
					const DDS_SampleStateMask,							\
					const DDS_ViewStateMask,							\
					const DDS_InstanceStateMask);						\
																		\
		static DDS_ReturnCode_t (*read)(								\
					dataType ## DataReader,								\
					DDS_sequence_ ## dataType*,							\
					DDS_SampleInfoSeq*,									\
					const DDS_long,									\
					const DDS_SampleStateMask,							\
					const DDS_ViewStateMask,							\
					const DDS_InstanceStateMask);						\
																		\
		static DDS_ReturnCode_t (*return_loan)(						\
					dataType ## DataReader,								\
					DDS_sequence_ ## dataType*,							\
					DDS_SampleInfoSeq*);								\
};

#define CREATE_DATATYPEFUNC_HEADER_WITH_SPACE(space, foo)				\
	CREATE_DATATYPEFUNC_HEADER(space ## _ ## foo)

#define CREATE_DATATYPEFUNC_SOURCE(dataType)							\
DDS_ReturnCode_t (*dataType ## Func::write)(							\
					dataType ## DataWriter, 							\
					const dataType*,									\
					const DDS_InstanceHandle_t)						\
	= dataType ## DataWriter_write;										\
																		\
DDS_sequence_ ## dataType* (*dataType ## Func::sequence__alloc)()		\
	= DDS_sequence_ ## dataType ## __alloc;								\
																		\
DDS_ReturnCode_t (*dataType ## Func::take)(								\
					dataType ## DataReader,								\
					DDS_sequence_## dataType*,							\
					DDS_SampleInfoSeq*,									\
					const DDS_long,									\
					const DDS_SampleStateMask,							\
					const DDS_ViewStateMask,							\
					const DDS_InstanceStateMask)						\
	= dataType ## DataReader_take;										\
																		\
DDS_ReturnCode_t (*dataType ## Func::read)(								\
					dataType ## DataReader,								\
					DDS_sequence_## dataType*,							\
					DDS_SampleInfoSeq*,									\
					const DDS_long,									\
					const DDS_SampleStateMask,							\
					const DDS_ViewStateMask,							\
					const DDS_InstanceStateMask)						\
	= dataType ## DataReader_read;										\
																		\
DDS_ReturnCode_t (*dataType ## Func::return_loan)(						\
					dataType ## DataReader,								\
					DDS_sequence_ ## dataType*,							\
					DDS_SampleInfoSeq*)									\
	= dataType ## DataReader_return_loan;

#define CREATE_DATATYPEFUNC_SOURCE_WITH_SPACE(space, foo)				\
	CREATE_DATATYPEFUNC_SOURCE(space ## _ ## foo)

#endif // !defined(DDSFUNCGENERATOR_H_)
