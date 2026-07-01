#pragma once

#include <EASTL/string.h>
#include "psyqo/cdrom-device.hh"
#include "psyqo/iso9660-parser.hh"

class CD {
	public:
		bool load(eastl::string filename);
		bool getIsPrepared();
	private:
		enum class State {
			InitCDROM,
			InitParser,
			FindFile,
			Loading,
			Ready,
			Error
		};

		State m_state = State::InitCDROM;
		bool m_isPrepared = false;
		psyqo::CDRomDevice m_cdrom;
		psyqo::ISO9660Parser m_parser;

		void onParserInit(bool success);
		void onFileFound(bool success);

};