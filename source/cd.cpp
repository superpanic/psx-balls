#include "cd.hh"
#include "psyqo/kernel.hh"
#include "psyqo/xprintf.h"

void CD::read(eastl::string filename) {
    psyqo::Kernel::assert(filename.size() <= MAX_FILENAME_LENGTH, "Filename too long!");
	m_filename = filename;

	if(m_state == State::Idle) {
		m_cdrom.prepare();
		m_cdrom.reset([this](bool s) { onReset(s); });
		m_state = State::Resetting;
	}
}

bool CD::advance() {
	switch (m_state) {
		case State::Idle:
			printf("Use read(filename) to start reading a file.\n");
			break;
		case State::Resetting:
			break;
		case State::InitializingParser:
			break;
		case State::FindingFile: {
				printf("Finding file %s ...\n", m_filename.c_str());
				m_parser.getDirentry(m_filename, &m_entry, [this](bool s) { onFileFound(s); });
			}
			break;
		case State::LoadingFile: {
				uint32_t sectorCount = (m_entry.size + 2047) >> 11;  // (divide by 2048);
				printf("Loading file %s ... (LBA=%d, size=%d, sectors=%d)\n", m_filename.c_str(), m_entry.LBA, m_entry.size, sectorCount);
				m_cdrom.readSectors(m_entry.LBA, sectorCount, m_file_buffer, [this](bool s) { onFileLoaded(s); });
				printf("File ready at LBA=%d, size=%d. Implement sector read!\n", m_entry.LBA, m_entry.size);
			}
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
	if(success && m_entry.type == psyqo::ISO9660Parser::DirEntry::FILE) {
		printf("SUCCESS: File found: LBA=%d, size=%d, name=%s\n", m_entry.LBA, m_entry.size, m_entry.name.c_str());
		m_state = State::LoadingFile;
	} else {
		printf("ERROR: File %s not found or invalid\n", m_filename.c_str());
		m_state = State::Error;
	}
}

void CD::onFileLoaded(bool success) {
	if(success) {
		printf("SUCCESS: File loaded\n");
		printf("File data (first 16 bytes): ");
		for (unsigned i = 0; i < 16 && i < m_entry.size; i++) {
			printf("%02X ", m_file_buffer[i]);
		}
		printf("\n");
		m_state = State::Ready;
	} else {
		printf("ERROR: File load failed\n");
		m_state = State::Error;
	}
}