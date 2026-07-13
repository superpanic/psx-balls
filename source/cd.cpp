#include "cd.hh"
#include "psyqo/xprintf.h"

bool CD::load(eastl::string filename) {
	switch (m_state) {
		case State::InitCDROM:
			if(!m_isPrepared) {
				m_cdrom.prepare();
				m_isPrepared = true;
			}
			if(!m_parser.initialized()) {
				m_parser.initialize( [this](bool s) { onParserInit(s); } );
				m_state = State::InitParser;
			} else {
				m_state = State::FindFile;
			}
			break;
		case State::InitParser:
			// wait for parser init
			break;
		case State::FindFile:
			break;
		case State::Loading:
			break;
		case State::Ready:
			return true;
			break;
		case State::Error:
			break;
	}
	return false;
}

bool CD::getIsPrepared() {
	return m_isPrepared;
}

void CD::onParserInit(bool success) {
	if(success) {
		m_state = State::FindFile;
	} else {
		m_state = State::Error;
	}
}

void CD::onFileFound(bool success) {

}
