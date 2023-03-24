#pragma once
#include <stdint.h>

#define AAC_ADTS_SIZE   7

class AAC_ADTS
{
public:
	AAC_ADTS(void);
	virtual ~AAC_ADTS(void);

	void syncword (uint16_t ivalue);

	void ID(uint16_t ivalue);

	void layer(uint16_t ivalue);

	void protection_absent(uint16_t ivalue);

	void profile(uint16_t ivalue);

	void sample_rate_index(uint32_t ivalue);

	void private_bit(uint16_t ivalue);

	void channel_configuration(uint16_t ivalue);

	void original_copy(uint16_t ivalue);

	void home(uint16_t ivalue);

	void copyright_identification_bit(uint16_t ivalue);

	void copyright_identification_start(uint16_t ivalue);

	void aac_frame_length(uint16_t ivalue);

	void adts_buffer_fullness(uint16_t ivalue);

	void number_of_raw_data_blocks_in_frame(uint16_t ivalue);

	inline unsigned char * Data(void);

    double frame_duration();

private:
	unsigned char * m_Data;
    int m_sampleRateIndex;
    int m_profile;
};

inline unsigned char * AAC_ADTS::Data(void)
{
	return m_Data;
}