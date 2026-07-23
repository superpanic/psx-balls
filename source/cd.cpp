#include "cd.hh"
#include "psyqo/kernel.hh"
#include "psyqo/xprintf.h"

void CD::read(eastl::string filename) {
    psyqo::Kernel::assert(filename.size() <= MAX_FILENAME_LENGTH, "Filename too long");
	m_filename = filename;

	if(m_state == State::Idle) {
		m_cdrom.prepare();
		m_cdrom.reset([this](bool s) { onReset(s); });
		m_state = State::Resetting;
	} else {
		advance();
	}
}

bool CD::advance() {
	switch (m_state) {
		case State::FindingFile:
			printf("Finding file %s ...\n", m_filename.c_str());
			m_parser.getDirentry(m_filename, &m_entry, [this](bool s) { onFileFound(s); });
			break;
		case State::LoadingFile:
			printf("File ready at LBA=%d, size=%d. Implement sector read!\n", m_entry.LBA, m_entry.size);
			m_state = State::Ready;
			break;
		case State::InitializingParser:
			break;
		case State::Ready:
			return true;
			break;
		case State::Error:
			break;
		default:
			break;
	}
	return false;
}

void CD::onReset(bool success) {
	if(success) {
		printf("SUCCESS: CD-ROM reset\n");
		if(!m_parser.initialized()) {
			m_parser.initialize( [this](bool s) { onParserInit(s); } );
			m_state = State::InitializingParser;
		} else {
			m_state = State::FindingFile;
			advance();
		}
	} else {
		printf("ERROR: CD-ROM reset failed\n");
		m_state = State::Error;
	}
}

void CD::onParserInit(bool success) {
	if(success) {
		printf("SUCCESS: Parser initialized\n");
		m_state = State::FindingFile;
	} else {
		printf("ERROR: Parser initialization failed\n");
		m_state = State::Error;
	}
}

void CD::onFileFound(bool success) {
	printf("=== onFileFound callback === success=%d, type=%d, name='%s'\n", 
	       (int)success, (int)m_entry.type, m_entry.name.c_str());
		   
	if(success && m_entry.type == psyqo::ISO9660Parser::DirEntry::FILE) {
		printf("SUCCESS: File found: LBA=%d, size=%d, name=%s\n", m_entry.LBA, m_entry.size, m_entry.name.c_str());
		m_state = State::LoadingFile;
		advance();
	} else {
		printf("ERROR: File not found or invalid\n");
		printf("File type: %d, name: %s\n", m_entry.type, m_entry.name.c_str());
		m_state = State::Error;
	}
}
