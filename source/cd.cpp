#include "cd.hh"
#include "psyqo/kernel.hh"
#include "psyqo/xprintf.h"

void CD::request(const LoadRequest &request) {
	psyqo::Kernel::assert(request.filename[0] != '\0', "Filename is empty!");
	m_load_request_queue[m_queueCount++] = request;
	psyqo::Kernel::assert(m_queueCount <= CD::MAX_QUEUE_SIZE, "Load request queue overflow!");
	if(m_state == State::Idle && m_queueCount > 0) {
		read(m_load_request_queue[0].filename);
	}
}

void CD::read(eastl::string_view filename) {
	psyqo::Kernel::assert(filename.size() <= MAX_FILENAME_LENGTH, "Filename too long!");
	m_filename.assign(filename.data(), filename.size());

	if(m_firstRun) {
		m_cdrom.prepare();
		m_cdrom.reset([this](bool s) { onReset(s); });
		m_state = State::Resetting;
		m_firstRun = false;
	} else {
		findFile();
	}
}

void CD::removeRequest() {
	if(m_queueCount > 0) {
		for(unsigned i = 1; i < m_queueCount; ++i) {
			m_load_request_queue[i-1] = m_load_request_queue[i];
		}
		--m_queueCount;
	}
}

bool CD::advance() {
	switch (m_state) {
		case State::Idle:
			break;
		case State::Resetting:
			break;
		case State::InitializingParser:
			break;
		case State::Finding:
			printf("*");
			break;
		case State::Loading:
			printf("#");
			break;
		case State::Completed:
			break;
		case State::Error:
			printf("CD-ROM error");
			break;
		default:
			break;
	}
	return false;
}

// 1
// find the file in the CD-ROM directory, using the ISO9660 parser
// calls onFileFound() when the file is found or not found in the CD-ROM directory
void CD::findFile() {
	m_state = State::Finding;
	m_parser.getDirentry(m_load_request_queue[0].filename, &m_load_request_queue[0].dir_entry, [this](bool s) { 
		onFileFound(s); 
	});
}

// 2
// called when the file is found or not found in the CD-ROM directory
// if the file is found, it will start loading the file
// calling loadFile() if the file is found
void CD::onFileFound(bool success) {		   
	if(success && m_load_request_queue[0].dir_entry.type == psyqo::ISO9660Parser::DirEntry::FILE) {
		printf("SUCCESS: File found: LBA=%d, size=%d, name=%s\n", m_load_request_queue[0].dir_entry.LBA, m_load_request_queue[0].dir_entry.size, m_load_request_queue[0].dir_entry.name.c_str());
		loadFile();
	} else {
		printf("ERROR: File %s not found or invalid\n", m_filename.c_str());
		m_state = State::Error;
	}
}

// 3
// read the file from the CD-ROM using the LBA and sector count, 
// calling onFileLoaded() when done
void CD::loadFile() {
	uint32_t sectorCount = (m_load_request_queue[0].dir_entry.size + 2047) >> 11;  // (divide by 2048);
	printf("Loading file %s (LBA=%d, size=%d, sectors=%d)\n", m_load_request_queue[0].filename, m_load_request_queue[0].dir_entry.LBA, m_load_request_queue[0].dir_entry.size, sectorCount);
	m_state = State::Loading; // set before calling readSectors to avoid race condition if callback
	printf("Loading ");
	m_cdrom.readSectors(m_load_request_queue[0].dir_entry.LBA, sectorCount, m_file_buffer, [this](bool s) { 
		onFileLoaded(s); 
	});
}


// 4
// called when the file has finished loading from the CD-ROM
// if successful, it will call the callback associated with the load request
// and remove the request from the queue
// enter state completed if successful
void CD::onFileLoaded(bool success) {
	if(success) {
		printf("\nSUCCESS: File loaded\n");
		removeRequest(); // remove request before calling the callback, to avoid potential issues if the callback triggers another request
		m_state = State::Completed;
		if(m_load_request_queue[0].callback) {
			m_load_request_queue[0].callback(true, m_load_request_queue[0].dir_entry.size);
		}
	} else {
		printf("ERROR: File load failed\n");
		m_state = State::Error;
	}
}

void CD::onReset(bool success) {
	if(success) {
		printf("SUCCESS: CD-ROM reset\n");
		if(!m_parser.initialized()) {
			m_state = State::InitializingParser;
			m_parser.initialize( [this](bool s) { onParserInit(s); } );
		} else {
			findFile();
		}
	} else {
		printf("ERROR: CD-ROM reset failed\n");
		m_state = State::Error;
	}
}

void CD::onParserInit(bool success) {
	if(success) {
		printf("SUCCESS: Parser initialized\n");
		findFile();
	} else {
		printf("ERROR: Parser initialization failed\n");
		m_state = State::Error;
	}
}

uint8_t *CD::extractFile() {
	if(m_queueCount > 0) {
		findFile();
	} else {
		m_state = State::Idle;
	}
	return m_file_buffer;
}

