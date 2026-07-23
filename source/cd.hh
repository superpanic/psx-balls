#pragma once

#include <EASTL/string.h>
#include "psyqo/cdrom-device.hh"
#include "psyqo/iso9660-parser.hh"

class CD {
	
	public: 
		CD() : m_parser(&m_cdrom) {   }
		void read(eastl::string filename);
		bool advance();

		bool isReady() const { return m_state == State::Ready; }
		bool hasError() const { return m_state == State::Error; }

		// making internal ISO9660Parser and CDRomDevice available for debugging purposes
		const psyqo::ISO9660Parser::DirEntry &getEntry() const { return m_entry; }
		psyqo::CDRomDevice &getCDRom() { return m_cdrom; }

	private:
		enum class State {
			Idle,
			Resetting,
			InitializingParser,
			FindingFile,
			LoadingFile,
			Ready,
			Error
		};

		constexpr static unsigned MAX_FILENAME_LENGTH = 32;

		State m_state = State::Idle;
		psyqo::CDRomDevice m_cdrom;
		psyqo::ISO9660Parser m_parser;

		eastl::string m_filename;
		psyqo::ISO9660Parser::DirEntry m_entry;

		void onReset(bool success);
		void onParserInit(bool success);
		void onFileFound(bool success);
		void onFileLoaded(bool success);
};