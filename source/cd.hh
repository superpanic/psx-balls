#pragma once

#include <EASTL/string.h>
#include <EASTL/string_view.h>
#include "psyqo/cdrom-device.hh"
#include "psyqo/iso9660-parser.hh"

#define MAX_FILENAME_LENGTH (16)

typedef struct LoadRequest {
	char filename[MAX_FILENAME_LENGTH+1];
	uint8_t *buffer;
	size_t max_size;
	size_t loaded_size;
	eastl::function<void(bool, uint32_t size)> callback;
	void setFilename(eastl::string_view str) {
		size_t len = str.size() < MAX_FILENAME_LENGTH ? str.size() : MAX_FILENAME_LENGTH;
		for(size_t i = 0; i < len; ++i) filename[i] = str[i];
		filename[len] = '\0';
	}
} LoadRequest;

class CD {	
	public: 
		CD() : m_parser(&m_cdrom) {   }
		void request(const LoadRequest &request);
		void read(eastl::string_view filename);
		bool advance();
		bool isBusy() const { return m_state != State::Idle && m_state != State::Ready && m_state != State::Error; }
		bool isReady() const { return m_state == State::Ready; }
		bool hasError() const { return m_state == State::Error; }
		uint8_t *getFileBuffer() { return m_file_buffer; }

		// making internal ISO9660Parser and CDRomDevice available for debugging purposes
		const psyqo::ISO9660Parser::DirEntry &getEntry() const { return m_entry; }
		psyqo::CDRomDevice &getCDRom() { return m_cdrom; }

		constexpr static unsigned MAX_QUEUE_SIZE = 4;
		constexpr static size_t MAX_FILE_SIZE = 256 * 1024;   // e.g. 256 KB max per model

	private:
		enum class State {
			Idle,
			Resetting,
			InitializingParser,
			FindFile,
			Finding,
			LoadFile,
			Loading,
			Ready,
			Error
		};
		State m_state = State::Idle;

		LoadRequest m_loadRequestQueue[MAX_QUEUE_SIZE];
		unsigned m_loadRequestQueueCount = 0;

		alignas(4) uint8_t m_file_buffer[MAX_FILE_SIZE];
		psyqo::CDRomDevice m_cdrom;
		psyqo::ISO9660Parser m_parser;
		eastl::string m_filename;
		psyqo::ISO9660Parser::DirEntry m_entry;

		void onReset(bool success);
		void onParserInit(bool success);
		void onFileFound(bool success);
		void onFileLoaded(bool success);
};